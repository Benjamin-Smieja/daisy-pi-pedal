#include "daisy_seed.h"
#include "daisysp.h"
#include "daisy_core.h"
#include "messages.h"
#include "stm32h7xx_hal_gpio.h"
#include "stm32h7xx_hal_i2c.h"
#include "trem.h"

using namespace daisy;
using namespace daisysp;

DaisySeed         hw;
I2C_HandleTypeDef hi2c1;

#define I2C_TARGET_ADDRESS 0x40
constexpr uint8_t kMaxRxBytes = 32;
uint8_t          rx_data_buffer[kMaxRxBytes];
volatile bool    is_new_data_ready = false;

MessageRingBuffer ringBuffer; 
volatile uint8_t pi_effect= 0;

//Effects Chain initialization:
Trem tremolo;


Chorus  ch;
//Chorus Params
bool  chorusEffectOn;
float chorusWet;
float del;
float lfo;

void InitI2CTarget()
{
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
        
        //Read in type... 
        pi_effect         = rx_data_buffer[0];
        is_new_data_ready = true;

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


static void initChorus(float sampleRate)
{
    ch.Init(sampleRate);

    chorusEffectOn = true;
    chorusWet      = 0.6f;

    ch.SetLfoFreq(1.2f, 0.7f);
    ch.SetLfoDepth(0.43f, 0.43f);
    ch.SetDelayMs(10.0f, 25.0f);
    ch.SetPan(0.0f, 1.0f);
}

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    tremolo.ProcessBlock(in,out,size);
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4); // Keeping your preferred block size
    float sample_rate = hw.AudioSampleRate();

    InitI2CTarget();

    tremolo.Init(sample_rate);

    // initChorus(sample_rate);

    //Configure and initialize button
    // GPIO button1;
    // button1.Init(hw.GetPin(22), 1000);
    // button1.Init(seed::D15, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

    hw.StartAudio(AudioCallback);
    while(1)
    {
        //Handle Coms with RPi (Process Message Queue):

        

        if(is_new_data_ready){
            if(pi_effect < 33){
                // hw.SetLed(true);
                // chorusEffectOn = true;
                // tremEffectOn   = false;
            }else if (pi_effect < 66){
                // hw.SetLed(false);
                // chorusEffectOn = false;
                // tremEffectOn   = true;
            }else{
                // hw.SetLed(true);
                // chorusEffectOn = false;
                // tremEffectOn   = false;
            }

            is_new_data_ready = false;
        }
        //  //Debounce the button
        // button1.Debounce();
        // //If the button is pressed, turn the LED on
        // bool isGrounded = !button1.Read();
        // hw.SetLed(isGrounded);

        // chorusEffectOn = isGrounded;
        // tremEffectOn   = !isGrounded;

        // //wait 1 ms
        System::Delay(1);
    }
}