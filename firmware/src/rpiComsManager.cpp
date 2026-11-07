#include "rpiComsManager.h"
#include "daisy_core.h"
#include "daisy_seed.h"
#include "daisysp.h"
#include "messages.h"
#include "stm32h7xx_hal_i2c.h"
#include <cstdint>
#include <tuple>

I2C_HandleTypeDef hi2c1;
constexpr uint8_t I2C_TARGET_ADDRESS = 0x40;
constexpr uint8_t kMaxRxBytes = 32;
uint8_t rx_data_buffer[kMaxRxBytes];
// uint8_t tx_data_buffer[kMaxRxBytes]{0};
bool tx_in_progress = false;
uint8_t noTxDataCode = 0;
uint8_t rx_receive_bytes = 0;
uint8_t tx_send_bytes = 0;

extern daisy::DaisySeed hw;

bool is_error = false;

MessageRingBuffer ringBuffer;
MessageRingBuffer txRingBuffer;

extern "C" void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c,
                                     uint8_t TransferDirection,
                                     uint16_t AddrMatchCode) {
  if (hi2c->Instance == I2C1) {
    // When master writes to slave, STM32 HAL reports TransferDirection ==
    // I2C_DIRECTION_TRANSMIT (0)
    if (TransferDirection == I2C_DIRECTION_TRANSMIT) {
      // Always read header first
      if (rx_receive_bytes == 0) {
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, &rx_receive_bytes, 1,
                                     I2C_LAST_FRAME);
      } else if (rx_receive_bytes > 0) {
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, rx_data_buffer, rx_receive_bytes,
                                     I2C_LAST_FRAME);
      }

    } else if (TransferDirection == I2C_DIRECTION_RECEIVE) {

      if (txRingBuffer.hasMsg() && tx_in_progress == false) {
        tx_in_progress = true;
        tx_send_bytes = txRingBuffer.getHeadSize();
        // send header
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, &tx_send_bytes, 1, I2C_LAST_FRAME);
      } else if (txRingBuffer.hasMsg() && tx_in_progress == true) {
        // send payload
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, txRingBuffer.getPointerToHead(),
                                      txRingBuffer.getHeadSize(),
                                      I2C_LAST_FRAME);
        tx_send_bytes = 0; // reset to 0, so next time we send header first
      }

    } else {
      HAL_I2C_Slave_Seq_Transmit_IT(hi2c, &noTxDataCode, 1, I2C_LAST_FRAME);
    }
  }
}

// Handle receive callback, parsing message type and updating buffers according
extern "C" void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance == I2C1) {
    // Check & Queue...
    if (rx_receive_bytes > 0 && rx_data_buffer[0] != 0) {
      ringBuffer.push(rx_data_buffer, rx_receive_bytes);

      rx_receive_bytes = 0;
      rx_data_buffer[0] = 0; // Reset MsgType to sentinel value

    } else if (rx_receive_bytes > 0 && rx_data_buffer[0] == 0) {
      // Header received, wait for package.
    }

    HAL_I2C_EnableListen_IT(hi2c);
  }
}

extern "C" void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance == I2C1) {
    HAL_I2C_EnableListen_IT(hi2c);
  }
}

extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance == I2C1) {
    HAL_I2C_EnableListen_IT(hi2c);
    hw.SetLed(true);
  }
}

extern "C" void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance == I2C1) {
    // clear the tx buffer & num bytes
    if (tx_in_progress == true && tx_send_bytes == 0) {
      tx_in_progress = false;
      txRingBuffer.popNoCpy();
      // payload sent, prep next message
      //  Todo: Check case where there is only 1 element.
    }

    HAL_I2C_EnableListen_IT(hi2c);
  }
}

extern "C" void I2C1_EV_IRQHandler(void) { HAL_I2C_EV_IRQHandler(&hi2c1); }

extern "C" void I2C1_ER_IRQHandler(void) { HAL_I2C_ER_IRQHandler(&hi2c1); }

void rpiComsManager::InitI2CTarget() {
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_I2C1_CLK_ENABLE();

  // Bitwise OR configures both PB8 (SCL) and PB9 (SDA) together
  GPIO_InitTypeDef gpio_init = {0};
  gpio_init.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  gpio_init.Mode = GPIO_MODE_AF_OD;
  gpio_init.Pull = GPIO_PULLUP;
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  gpio_init.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &gpio_init);

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10707DBC;
  hi2c1.Init.OwnAddress1 = (I2C_TARGET_ADDRESS << 1);
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  HAL_I2C_Init(&hi2c1);

  // Set priority to 5 (a lower priority than audio, protecting the audio
  // thread)
  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
  HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);

  HAL_I2C_EnableListen_IT(&hi2c1);
}

GenericMsg rpiComsManager::PopMessage() { return ringBuffer.pop(); }

bool rpiComsManager::hasMsg() { return ringBuffer.hasMsg(); }

void rpiComsManager::SendEffectDesc(EffectDescMsg msg) {
  txRingBuffer.push(&msg, sizeof(EffectDescMsg));
}

void rpiComsManager::SendParamDesc(ParamDescMsg msg) {
  txRingBuffer.push(&msg, sizeof(ParamDescMsg));
}
void rpiComsManager::SendDescribeDone() {
  DescribeDoneMsg msg{static_cast<uint8_t>(MsgType::DescribeDone)};
  txRingBuffer.push(&msg, sizeof(DescribeDoneMsg));
}