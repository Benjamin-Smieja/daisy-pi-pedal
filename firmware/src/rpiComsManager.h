#pragma once

#include "messages.h"
class rpiComsManager{
    public:
        void InitI2CTarget();
        GenericMsg PopMessage();
        bool hasMsg();
        void SendEffectDesc(EffectDescMsg msg);
        void SendParamDesc(ParamDescMsg msg);
        void SendDescribeDone();

};  