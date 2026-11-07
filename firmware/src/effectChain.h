#pragma once
#include "effect.h"
#include "messages.h"
#include "rpiComsManager.h"
#include "trem.h"
#include <cstddef>

struct node{
    node* next;
    uint8_t effect_idx;
};

class EffectChain{
public: 
    Trem tremolo;
    Effect* effects[6] = {&tremolo};
    node pedal_order = {nullptr,0};
    //Linked list of pedals?
    void Init(float sample_rate);
    void ProcessBlock(daisy::AudioHandle::InputBuffer in,
                                    daisy::AudioHandle::OutputBuffer out,
                                    size_t                    size);
    void ProcessMessage(GenericMsg msg, rpiComsManager* rpiComsManager);
};

