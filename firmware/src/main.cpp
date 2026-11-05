#include "daisy_seed.h"
#include "daisysp.h"
#include "daisy_core.h"
#include "messages.h"
#include "trem.h"
#include "rpiComsManager.h"

using namespace daisy;
using namespace daisysp;

DaisySeed         hw;
rpiComsManager rpiComsManager;

//Effects Chain initialization:
Trem tremolo;


// //Chorus => To change to Chorus class...
// Chorus  ch;
// //Chorus Params
// bool  chorusEffectOn;
// float chorusWet;
// float del;
// float lfo;



// static void initChorus(float sampleRate)
// {
//     ch.Init(sampleRate);

//     chorusEffectOn = true;
//     chorusWet      = 0.6f;

//     ch.SetLfoFreq(1.2f, 0.7f);
//     ch.SetLfoDepth(0.43f, 0.43f);
//     ch.SetDelayMs(10.0f, 25.0f);
//     ch.SetPan(0.0f, 1.0f);
// }

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

    //Create rpiComsManager & Init
    rpiComsManager.InitI2CTarget();

    //Init Effects
    tremolo.Init(sample_rate);

    // initChorus(sample_rate);

    hw.StartAudio(AudioCallback);
    while(1)
    {
        //Handle Coms with RPi (Process Message Queue):

        // //wait 1 ms
        System::Delay(1);
    }
}