#include "trem.h"
#include "effect_param.h"


void Trem::Init(float sample_rate){
    trem_.Init(sample_rate);

    params_[Trem::kFreq].Init(6.0f,EffectParam::FLOAT,0.f,300.f);
    params_[Trem::kDepth].Init(0.8f,EffectParam::FLOAT,0.f,1.f);
    params_[Trem::kWaveForm].Init(1.0f, EffectParam::INT, 0.f, 10.f); //TODO: Check bounds for waveform 

    is_on_ = false; // Enabled by default
}

void Trem::UpdateParams(){
    trem_.SetFreq(params_[Trem::kFreq].GetVal());
    trem_.SetDepth(params_[Trem::kDepth].GetVal());
    trem_.SetWaveform(static_cast<int>(params_[Trem::kWaveForm].GetVal()));
}