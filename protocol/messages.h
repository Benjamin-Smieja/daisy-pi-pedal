#pragma once
// #include <cstdint>
// #include <stdint.h>

#include <atomic>
#include <cstddef>
#include <cstring>
constexpr uint8_t kNameLen = 16;

enum class MsgType : uint8_t{
    DescibeRequest = 1,
    EffectDescMsg, ParamDesc, EnumLabel, DescribeDone,SetParam,SetEffectOn
};

enum class EffectParamType : uint8_t { Bool, Float, Int, Enum};

struct __attribute__((packed)) EffectDescMsg{
    uint8_t msg_type;
    int effect_id;
    int num_params;
    char name[kNameLen];

};

struct __attribute__((packed)) ParamDescMsg{
    uint8_t msg_type;
    uint8_t effect_id; 
    uint8_t param_id;
    uint8_t param_type;
    float min, max, def;
    char name[kNameLen];
};

struct __attribute__((packed)) SetEffectOn{
    uint8_t msg_type;
    uint8_t effect_id;
    bool setOn;
};

struct __attribute__((packed)) SetParam{
    uint8_t msg_type = static_cast<uint8_t>(MsgType::SetParam);
    uint8_t effect_id;
    uint8_t param_id;
    float val;
};

constexpr size_t kMaxMsgSize = sizeof(ParamDescMsg);

struct __attribute__((packed)) GenericMsg{
    uint8_t msg_type;
    uint8_t payload[kMaxMsgSize - 1];
};

constexpr uint8_t kMaxNumMsg = 8;

struct MessageRingBuffer{
    std::atomic<uint8_t> Head{0};
    std::atomic<uint8_t> Tail{0};
 
    GenericMsg messages[kMaxNumMsg];

    GenericMsg pop(){
        if(Head == Tail){
            //Nothing stored... Return
            // Todo: throw error / log / Return sentinel GenericMsg Pk;
        }
        // Todo: Guard against request if empty;
        uint8_t toPop = Head; //Does this need to be atomic too? No this is one clk 
        
        Head = (Head + 1) % kMaxNumMsg;
        return messages[toPop];
    }

    void push(const uint8_t* data, uint8_t len){
        // Todo: Deal with buffer overflow... in this current case it blocks pop as Head and Tail will be overlapping..
        memcpy(&messages[Tail],data,len);
        Tail = (Tail + 1) % kMaxNumMsg;
        return;
    }
};
