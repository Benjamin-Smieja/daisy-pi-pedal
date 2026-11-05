#include "trem.h"
#include "messages.h"
#include <cstdint>

const EffectParamDescriptor Trem::kParams_[kNumParams]{
    {"Freq", EffectParamType::Float, 0.1f, 300.f, 6.0f},
    {"Depth", EffectParamType::Float, 0.f, 1.f, 0.6f},
    {"waveform", EffectParamType::Enum, 0.f, 8.f, 0.f}
    // Todo: Check bounds for waveform
};

void Trem::Init(float sample_rate) {
  trem_.Init(sample_rate);
  is_on_ = true; // Enabled by default
  for(int paramNum = 0; paramNum < kNumParams; paramNum++){
    param_vals_[paramNum] = kParams_[paramNum].def;
    SetParam(paramNum,kParams_[paramNum].def);
  }
}

void Trem::SetParam(uint8_t param_id, float val) {
  switch (param_id) {
    case Trem::kFreq:
      trem_.SetFreq(param_vals_[Trem::kFreq]);
      break;
    case Trem::kDepth:
      trem_.SetDepth(param_vals_[Trem::kDepth]);
      break;
    case Trem::kWaveForm:
      trem_.SetWaveform(static_cast<int>(param_vals_[Trem::kWaveForm]));
      break;
    default:
    //Did not match one of the Param_Ids....
    break;
  }
}

void Trem::ProcessBlock(daisy::AudioHandle::InputBuffer in,
                        daisy::AudioHandle::OutputBuffer out, size_t size) {
  for (size_t i = 0; i < size; i++) {

    if (is_on_) {
      out[0][i] = trem_.Process(in[0][i]);
    } else {
      // bypass
      out[0][i] = in[0][i];
    }
    // Mono Bass => Silence the R channel
    out[1][i] = 0.f;
  }
}