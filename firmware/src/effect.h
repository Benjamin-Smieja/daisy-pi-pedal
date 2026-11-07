#pragma once
#include "daisy_seed.h"
#include "messages.h"
#include <cmath>
#include <cstdint>
#include <sys/types.h>

struct EffectParamDescriptor {
  const char *name;
  const EffectParamType param_type;
  const float min, max, def;

  float clamp(float f) const {
    if (!isfinite(f)) {
      // Guard against NaN - May be worth logging this issue
      return min;
    }

    if (f < min) {
      return min;
    }

    if (f > max) {
      return max;
    }

    return f;
  }

  void setClampedVal(float f, float &toSet) const {
    switch (param_type) {
    case EffectParamType::Float:
      toSet = clamp(f);
      break;
    case EffectParamType::Bool:
      toSet = clamp(f);
      break;
    case EffectParamType::Enum:
      toSet = clamp(f) + 0.001;
      break;
    case EffectParamType::Int:
      toSet = clamp(f) + 0.001;
      break;
    default:
      // Todo: Flag issue where type was not set properly...
      toSet = f;
      break;
    }
  }
};

class Effect {
public:
  Effect(uint8_t id, uint8_t num_params, const char *name)
      : effect_id_(id), num_params_(num_params), is_on_(false), name_(name) {};
  ~Effect() {};
  virtual void ProcessBlock(daisy::AudioHandle::InputBuffer in,
                            daisy::AudioHandle::OutputBuffer out, size_t size) = 0;
  virtual void Init(float sample_rate) = 0;
  virtual void SetParam(uint8_t param_idx, float val) = 0;
  virtual EffectParamDescriptor GetParamDesc(uint8_t param_idx) = 0; 

  const uint8_t GetEffectId() { return effect_id_; }
  const bool GetIsOn() { return is_on_; }
  const uint8_t GetNumParams() { return num_params_; }
  const char *GetName() { return name_; }

protected:
  const uint8_t effect_id_;
  uint8_t num_params_;
  bool is_on_;
  const char *name_;
};