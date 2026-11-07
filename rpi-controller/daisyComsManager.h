#pragma once

#include <cstdint>
#include <string>
#include "messages.h"

class daisyComsManager {
public:
    daisyComsManager(int adapter_nr = 1, int device_address = 0x40);
    ~daisyComsManager();

    // Prevent copying to avoid duplicate file descriptor issues
    daisyComsManager(const daisyComsManager&) =delete;
    daisyComsManager& operator=(const daisyComsManager&) =delete;

    bool initialize();
    bool sendSetParamToDaisy(uint8_t effect_id, uint8_t param_id, float value);
    bool sendDataToDaisy(void* bf, uint8_t num_bytes);
    GenericMsg readValueFromDaisy();
    void getPedalDescription();
    bool requestEffectDescription();

private:
    int adapterNr_;
    int deviceAddress_;
    int i2cFile_;
    bool isConnected_;
};