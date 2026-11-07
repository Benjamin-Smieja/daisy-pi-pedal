#pragma once
#include "messages.h"

#define kMaxParams 8

struct param {
  const uint8_t effect_id;
  const uint8_t param_id;
  const char *name;
  const float min, max, def;
};

class PedalState {
public:
  PedalState();
  ~PedalState();
  param params[kMaxParams];
  float paramValues[kMaxParams];
};