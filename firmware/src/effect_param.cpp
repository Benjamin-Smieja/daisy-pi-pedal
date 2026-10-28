#include "effect_param.h"

void EffectParam::Init(float value, ParamValType valueType, float min_val, float max_val){
    val_type_ = valueType;
    min_val_ = min_val;
    max_val_ = max_val;

    // TODO: Use a setter to check for INT
    if(valueType == INT){
        val_ = value + 0.01;
    }else{
        val_ = value;
    }
}
        
float EffectParam::GetVal(){
    return val_;
}

float EffectParam::SetVal(float val){
   if(val_type_ == INT){
        val_ = val + 0.01; //Prevent floating point issue during conversion to int
    }else{
        val_ = val;
    }
}
