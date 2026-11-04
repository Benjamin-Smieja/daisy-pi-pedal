#pragma once
#include "daisy_seed.h"
#include "messages.h"
#include <cmath>


struct EffectParamDescriptor{
    const char* name;
    const EffectParamType param_type;
    const float min,max,def;

    float clamp(float f){
        if(!isfinite(f)){
            // Guard against NaN - May be worth logging this issue
            return min;
        }

        if(f < min){
            return min;
        }

        if(f > max ){
            return max;
        }

        return f;
    }

    float setClampedVal(float f, float &toSet){
        switch (param_type) {
            case EffectParamType::Float:
                toSet = clamp(f);
            break;
            case EffectParamType::Bool:
                toSet = clamp(f);
            break;
            case EffectParamType::Enum:
                toSet = clamp(f) +0.001;
            break;
            case EffectParamType::Int:
                toSet = clamp(f) +0.001;
            break;
            default:
            //Problem
            break;
        }
    }
};


class Effect{
    public:
        Effect(){};
        ~Effect(){};
        virtual void ProcessBlock(daisy::AudioHandle::InputBuffer in,
                                    daisy::AudioHandle::OutputBuffer out,
                                    size_t                    size) = 0;
        virtual void Init(float sample_rate) = 0;
        virtual void UpdateParams() = 0;
        
        const int GetEffectId(){
            return effect_id_;
        }

        const bool GetIsOn(){
            return is_on_;
        }

    protected:
        int effect_id_;
        bool is_on_;
};