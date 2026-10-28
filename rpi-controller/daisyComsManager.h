#pragma once

#include <cstdint>
#include <string>

class daisyComsManager {
public:
    daisyComsManager(int adapter_nr = 1, int device_address = 0x40);
    ~daisyComsManager();

    // Prevent copying to avoid duplicate file descriptor issues
    daisyComsManager(const daisyComsManager&) =delete;
    daisyComsManager& operator=(const daisyComsManager&) =delete;

    bool initialize();
    bool sendValueToDaisy(uint8_t effect_id, uint8_t param_id, uint8_t value);

private:
    int m_adapterNr;
    int m_deviceAddress;
    int m_i2cFile;
    bool m_isConnected;

    // Packed struct matching your future protocol
    struct __attribute__((packed)) PedalCommand {
        uint8_t normalized_val;
        uint8_t effect_id;
        uint8_t param_id;
    };
};