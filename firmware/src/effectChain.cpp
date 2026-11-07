#include "effectChain.h"
#include "effect.h"
#include "messages.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <sys/types.h>

void EffectChain::Init(float sample_rate) {
  // Iterate and init effects
  node *pedal_idx_node = &pedal_order;
  do {
    effects[pedal_idx_node->effect_idx]->Init(sample_rate);
    pedal_idx_node = pedal_idx_node->next;
  } while (pedal_idx_node != nullptr);
}

void EffectChain::ProcessBlock(daisy::AudioHandle::InputBuffer in,
                               daisy::AudioHandle::OutputBuffer out,
                               size_t size) {
  // Iterate and process effects
  node *pedal_idx_node = &pedal_order;
  do {
    effects[pedal_idx_node->effect_idx]->ProcessBlock(in, out, size);
    pedal_idx_node = pedal_idx_node->next;
  } while (pedal_idx_node != nullptr);
}
void EffectChain::ProcessMessage(GenericMsg msg,
                                 rpiComsManager *rpiComsManager) {
  // Cast Msg and process accordingly
  switch (static_cast<MsgType>(msg.msg_type)) {
  case MsgType::DescibeRequest: {
    // Todo Cpy effect Name
    // Iterate through Effects, and Iterate through Params for each effect....
    //  Build EffectDescMsg & send to rpi
    EffectDescMsg effectDescMsg{static_cast<uint8_t>(MsgType::EffectDesc),
                                effects[0]->GetEffectId(),
                                effects[0]->GetNumParams(), "Tremolo"};
    rpiComsManager->SendEffectDesc(effectDescMsg);

    EffectParamDescriptor effectParamDescriptor = effects[0]->GetParamDesc(0);

    char paramName[16];
    strcpy(paramName, effectParamDescriptor.name);

    ParamDescMsg paramDescMsg{
        static_cast<uint8_t>(MsgType::ParamDesc),
        effects[0]->GetEffectId(),
        0,
        static_cast<uint8_t>(effectParamDescriptor.param_type),
        effectParamDescriptor.min,
        effectParamDescriptor.max,
        effectParamDescriptor.def,
    };

    strncpy(paramDescMsg.name, effectParamDescriptor.name, 15);
    paramDescMsg.name[15] = '\0';

    rpiComsManager->SendParamDesc(paramDescMsg);
    rpiComsManager->SendDescribeDone();
    break;
  }
  case MsgType::EffectDesc: {
    // This is should only be sent to the pi, never received!
    break;
  }

  case MsgType::ParamDesc: {
    // This is should only be sent to the pi, never received!
    break;
  }

  case MsgType::EnumLabel: {
    break;
  }

  case MsgType::DescribeDone: {
    // This is should only be sent to the pi, never received!
    break;
  }

  case MsgType::SetParam: {
    SetParamMsg p;
    memcpy(&msg.payload, &p, sizeof(SetParamMsg));
    effects[p.effect_id]->SetParam(p.param_id, p.val);
    break;
  }
  case MsgType::SetEffectOn: {
    break;
  }
  default:
    break;
  }
}