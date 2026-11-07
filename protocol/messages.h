#pragma once
// #include <cstdint>
// #include <stdint.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
constexpr uint8_t kNameLen = 16;

enum class MsgType : uint8_t {
  // Note 0 is reserved for no data!
  DescibeRequest = 1,
  EffectDesc,
  ParamDesc,
  EnumLabel,
  DescribeDone,
  SetParam,
  SetEffectOn
};

enum class EffectParamType : uint8_t { Bool, Float, Int, Enum };

struct __attribute__((packed)) DescibeRequestMsg {
  uint8_t msg_type = static_cast<uint8_t>(MsgType::DescibeRequest);
};

struct __attribute__((packed)) EffectDescMsg {
  uint8_t msg_type = static_cast<uint8_t>(MsgType::EffectDesc);
  uint8_t effect_id;
  uint8_t num_params;
  char name[kNameLen];
};

struct __attribute__((packed)) ParamDescMsg {
  uint8_t msg_type = static_cast<uint8_t>(MsgType::ParamDesc);
  uint8_t effect_id;
  uint8_t param_id;
  uint8_t param_type;
  float min, max, def;
  char name[kNameLen];
};

struct __attribute__((packed)) EnumLabelMsg {
  uint8_t msg_type = static_cast<uint8_t>(MsgType::EnumLabel);
  uint8_t enum_idx;
  char enumName[kNameLen];
};

struct __attribute__((packed)) DescribeDoneMsg {
  uint8_t msg_type = static_cast<uint8_t>(MsgType::DescribeDone);
};

struct __attribute__((packed)) SetParamMsg {
  uint8_t msg_type = static_cast<uint8_t>(MsgType::SetParam);
  uint8_t effect_id;
  uint8_t param_id;
  float val;
};

struct __attribute__((packed)) SetEffectOnMsg {
  uint8_t msg_type = static_cast<uint8_t>(MsgType::SetEffectOn);
  uint8_t effect_id;
  bool setOn;
};


struct __attribute__((packed)) HeaderMsg{
    uint8_t numBytes;
};

constexpr size_t kMaxMsgSize = sizeof(ParamDescMsg);
const uint8_t MsgSizes[7] = {sizeof(DescibeRequestMsg), sizeof(EffectDescMsg),
                             sizeof(ParamDescMsg),   sizeof(EnumLabelMsg),
                             sizeof(DescribeDoneMsg),   sizeof(SetParamMsg),
                             sizeof(SetEffectOnMsg)};

struct __attribute__((packed)) GenericMsg {
  uint8_t msg_type;
  uint8_t payload[kMaxMsgSize];
};

constexpr uint8_t kMaxNumMsg = 8;

struct MessageRingBuffer {
  std::atomic<uint8_t> Head{0};
  std::atomic<uint8_t> Tail{0};

  GenericMsg messages[kMaxNumMsg];

  GenericMsg pop() {
    if (!hasMsg()) {
      // Nothing stored... Return
      //  Todo: throw error / log / Return sentinel GenericMsg Pk;
    }
    // Todo: Guard against request if empty;
    uint8_t toPop = Head; // Does this need to be atomic too? No this is one clk

    Head = (Head + 1) % kMaxNumMsg;
    return messages[toPop];
  }

  void popNoCpy(){
    if(!hasMsg()){

    }
    Head = (Head + 1) % kMaxNumMsg;
  }

  void push(const void *data, uint8_t len) {
    // Todo: Deal with buffer overflow... in this current case it blocks pop as
    // Head and Tail will be overlapping..
    memcpy(&messages[Tail], data, len);
    Tail = (Tail + 1) % kMaxNumMsg;
    return;
  }

  bool hasMsg(){
    if(Head != Tail){
        return true;
    }
    return false;
  }

  uint8_t getHeadSize(){
    return MsgSizes[messages[Head].msg_type - 1];
  }

  uint8_t* getPointerToHead(){
    return reinterpret_cast<uint8_t*>(&messages[Head]);
  }

};
