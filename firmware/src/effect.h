#include "effect_param.h"

class Effect{
    public:
        Effect();
        ~Effect();
        virtual float ProcessBlock();
        virtual void Init(float sample_rate);
        virtual void UpdateParams();
        // void AddParams(EffectParam* params);
        

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