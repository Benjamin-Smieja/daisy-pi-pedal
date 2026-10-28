
class EffectParam{
    public:
        enum ParamValType{
            BOOL,
            FLOAT,
            INT,
            // CATEGORICAL
        };


        EffectParam();
        ~EffectParam();
        void Init(float value, ParamValType valueType, float min_val, float max_val);
        float GetVal();
        float SetVal(float val);
    private:
        enum ParamValType val_type_;
        float min_val_, max_val_;
        float val_;
        const char* name_; 

};