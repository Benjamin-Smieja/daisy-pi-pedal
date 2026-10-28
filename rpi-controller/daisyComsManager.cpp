#include "daisyComsManager.h"
#include <cstdint>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

daisyComsManager::daisyComsManager(int adapter_nr, int device_address)
    : m_adapterNr(adapter_nr), m_deviceAddress(device_address), m_i2cFile(-1), m_isConnected(false) {}

daisyComsManager::~daisyComsManager() {
    if (m_i2cFile >= 0) {
        close(m_i2cFile);
    }
}

bool daisyComsManager::initialize() {
    char filename[20];
    snprintf(filename, sizeof(filename), "/dev/i2c-%d", m_adapterNr);

    // Open ONCE during startup
    m_i2cFile = open(filename, O_RDWR);
    if (m_i2cFile < 0) {
        std::cerr << "Failed to open I2C bus: " << filename << "\n";
        return false;
    }

    // Set slave address ONCE
    if (ioctl(m_i2cFile, I2C_SLAVE, m_deviceAddress) < 0) {
        std::cerr << "Failed to acquire bus access to address 0x" << std::hex << m_deviceAddress << "\n";
        close(m_i2cFile);
        m_i2cFile = -1;
        return false;
    }

    m_isConnected = true;
    return true;
}

bool daisyComsManager::sendValueToDaisy(uint8_t effect_id, uint8_t param_id, uint8_t value) {
    if (!m_isConnected || m_i2cFile < 0) {
        return false;
    }

    PedalCommand cmd;
    cmd.effect_id = effect_id;
    cmd.param_id = param_id;
    cmd.normalized_val = value;

    // Fast, lightweight raw write using the persistent file descriptor
    ssize_t bytesWritten = write(m_i2cFile, &cmd, sizeof(PedalCommand));
    if (bytesWritten != sizeof(PedalCommand)) {
        std::cerr << "I2C write failed!\n";
        return false;
    }

    return true;
}