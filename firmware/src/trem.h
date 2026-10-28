#include "Effects/tremolo.h"
#include "daisysp.h"
#include "effect_param.h"

#include "effect.h"
class Trem : Effect{
    enum params{
        kFreq = 0,
        kDepth,
        kWaveForm,
        kNumParams
    };

    public:
        void Init(float sample_rate) override;
        void UpdateParams() override;
    private:
        daisysp::Tremolo trem_;
        EffectParam params_[Trem::kNumParams]; 
};