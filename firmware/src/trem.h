#pragma once
#include "Effects/tremolo.h"
#include "effect.h"

class Trem : public Effect {
public:
    enum params{
        kFreq = 0,
        kDepth,
        kWaveForm,
        kNumParams
    };

    void Init(float sample_rate) override;
    void UpdateParams() override;
    void ProcessBlock(daisy::AudioHandle::InputBuffer in,
                                    daisy::AudioHandle::OutputBuffer out,
                                    size_t                    size) override;
    
     
        
private:
    daisysp::Tremolo trem_;
    const static EffectParamDescriptor kParams_[kNumParams]; 
    float param_vals_[kNumParams];
};