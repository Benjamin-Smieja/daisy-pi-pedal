#include "rpiComsManager.h"
#include "messages.h"


I2C_HandleTypeDef hi2c1;
constexpr uint8_t I2C_TARGET_ADDRESS = 0x40;
constexpr uint8_t kMaxRxBytes = 32;
uint8_t          rx_data_buffer[kMaxRxBytes];
MessageRingBuffer ringBuffer; 


extern "C" void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c,
                                     uint8_t            TransferDirection,
                                     uint16_t           AddrMatchCode)
{
    if(hi2c->Instance == I2C1)
    {
        // When master writes to slave, STM32 HAL reports TransferDirection == I2C_DIRECTION_TRANSMIT (0)
        if(TransferDirection == I2C_DIRECTION_TRANSMIT)
        {
            HAL_I2C_Slave_Seq_Receive_IT(
                hi2c, rx_data_buffer, kMaxRxBytes, I2C_LAST_FRAME);
        }
    }
}

// Handle receive callback, parsing message type and updating buffers according
extern "C" void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        //Check & Queue...
        ringBuffer.push(rx_data_buffer,kMaxRxBytes);
        HAL_I2C_EnableListen_IT(hi2c);
    }
}

extern "C" void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        HAL_I2C_EnableListen_IT(hi2c);
    }
}

extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        if(hi2c->ErrorCode == HAL_I2C_ERROR_AF){
            uint8_t len = hi2c->pBuffPtr - rx_data_buffer;
            //Check & Queue
            ringBuffer.push(rx_data_buffer, len);
        }else{
            //drop the data
        }
        HAL_I2C_EnableListen_IT(hi2c);
    }
}

extern "C" void I2C1_EV_IRQHandler(void)
{ HAL_I2C_EV_IRQHandler(&hi2c1); }

extern "C" void I2C1_ER_IRQHandler(void)
{ HAL_I2C_ER_IRQHandler(&hi2c1); }


void rpiComsManager::InitI2CTarget(){
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    //Bitwise OR configures both PB8 (SCL) and PB9 (SDA) together
    GPIO_InitTypeDef gpio_init = {0};
    gpio_init.Pin              = GPIO_PIN_8 | GPIO_PIN_9;
    gpio_init.Mode             = GPIO_MODE_AF_OD;
    gpio_init.Pull             = GPIO_PULLUP;
    gpio_init.Speed            = GPIO_SPEED_FREQ_LOW;
    gpio_init.Alternate        = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio_init);

    hi2c1.Instance             = I2C1;
    hi2c1.Init.Timing          = 0x10707DBC;
    hi2c1.Init.OwnAddress1     = (I2C_TARGET_ADDRESS << 1);
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2     = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&hi2c1);

    //Set priority to 5 (a lower priority than audio, protecting the audio thread)
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);

    HAL_I2C_EnableListen_IT(&hi2c1);
}