#include "daisy_seed.h"
#include "daisysp.h"
#include "daisy_core.h"
#include "messages.h"
#include "trem.h"
#include "rpiComsManager.h"
#include "effectChain.h"

using namespace daisy;
using namespace daisysp;

DaisySeed         hw;
rpiComsManager rpiComsManager;

//Effects Chain initialization:
EffectChain effectChain;

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    effectChain.ProcessBlock(in,out,size);
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(64); // Keeping your preferred block size
    float sample_rate = hw.AudioSampleRate();

    //Create rpiComsManager & Init
    rpiComsManager.InitI2CTarget();

    //Init Effects
    effectChain.Init(sample_rate);

    // initChorus(sample_rate);

    hw.StartAudio(AudioCallback);
    while(1)
    {
        //Handle Coms with RPi (Process Message Queue):
        if(rpiComsManager.hasMsg()){
            effectChain.ProcessMessage(rpiComsManager.PopMessage(), &rpiComsManager);
        }

        // //wait 1 ms
        System::Delay(1);
    }
}