#include "daisyComsManager.h"
#include "messages.h"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

daisyComsManager::daisyComsManager(int adapter_nr, int device_address)
    : adapterNr_(adapter_nr), deviceAddress_(device_address), i2cFile_(-1),
      isConnected_(false) {}

daisyComsManager::~daisyComsManager() {
  if (i2cFile_ >= 0) {
    close(i2cFile_);
  }
}

bool daisyComsManager::initialize() {
  char filename[20];
  snprintf(filename, sizeof(filename), "/dev/i2c-%d", adapterNr_);

  // Open ONCE during startup
  i2cFile_ = open(filename, O_RDWR);
  if (i2cFile_ < 0) {
    std::cerr << "Failed to open I2C bus: " << filename << "\n";
    return false;
  }

  // Set Target address ONCE
  if (ioctl(i2cFile_, I2C_SLAVE, deviceAddress_) < 0) {
    std::cerr << "Failed to acquire bus access to address 0x" << std::hex
              << deviceAddress_ << "\n";
    close(i2cFile_);
    i2cFile_ = -1;
    return false;
  }

  isConnected_ = true;

  std::cerr << "Aquired bus accesss to address 0x"<< std::hex
              << deviceAddress_ << "\n";
  return true;
}

bool daisyComsManager::sendSetParamToDaisy(uint8_t effect_id, uint8_t param_id,
                                           float value) {
  // Todo: Add guards for effect id and param id, and value?
  SetParamMsg setParamMsg{static_cast<uint8_t>(MsgType::SetParam), effect_id,
                          param_id, value};

  return sendDataToDaisy(&setParamMsg, sizeof(SetParamMsg));
}

bool daisyComsManager::sendDataToDaisy(void *bf, uint8_t num_bytes) {
  if (!isConnected_ || i2cFile_ < 0) {
    return false;
  }

  uint8_t numBytesToSend = num_bytes;

  // Send header first
  ssize_t bytesWritten = write(i2cFile_, &numBytesToSend, 1);
  if (bytesWritten != 1) {
    std::cerr << "I2C header write failed!\n";
    return false;
  }

  std::cerr << "I2C header write complete!\n";

  // DEBUG: dump every byte that is about to go out on the bus
  std::cerr << "Header (" << std::dec << +numBytesToSend << " bytes): ";
  std::cerr << std::hex << std::setfill('0');
  std::cerr << std::setw(2) << +numBytesToSend << "\n";

  std::cerr << "Payload (" << std::dec << +num_bytes << " bytes): ";
  const uint8_t *payloadBytes = static_cast<const uint8_t *>(bf);
  for (uint8_t i = 0; i < num_bytes; ++i) {
    std::cerr << std::setw(2) << +payloadBytes[i] << " ";
  }
  std::cerr << std::dec << "\n";

  // Send payload
  bytesWritten = write(i2cFile_, bf, num_bytes);
  if (bytesWritten != num_bytes) {
    std::cerr << "I2C payload write failed!\n";
    return false;
  }
  
   std::cerr << "I2C payload write complete!\n\n";
  return true;
}

void daisyComsManager::getPedalDescription() {

  if (requestEffectDescription()) {
    // poll for effect & Param description
    GenericMsg msg;

    // Process all messages until DescribeDone Message
    do {

      // Poll for messages
      do {
        // Todo: Convert to non-blocking timeout?
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        msg = readValueFromDaisy();
      } while (msg.msg_type == 0);

      // Process Ui element...
      if (msg.msg_type == static_cast<uint8_t>(MsgType::EffectDesc)) {
        // Process Effect
         std::cerr << "Process Effect Recevied!\n";

      } else if (msg.msg_type == static_cast<uint8_t>(MsgType::ParamDesc)) {
        // Process Param
        std::cerr << "Process Param Recevied!\n";
      }

      // Todo: guard against data corruption & missed DescriptionDone...
    } while (msg.msg_type != static_cast<uint8_t>(MsgType::DescribeDone));
  }
}

bool daisyComsManager::requestEffectDescription() {

  DescibeRequestMsg describeReqMsg{
      static_cast<uint8_t>(MsgType::DescibeRequest)};

  return sendDataToDaisy(&describeReqMsg, sizeof(DescibeRequestMsg));
}

GenericMsg daisyComsManager::readValueFromDaisy() {
  GenericMsg msg;
  msg.msg_type = 0; // Explicitly set sentinel value here

  if (!isConnected_ || i2cFile_ < 0) {
    // Return empty package?
    return msg;
  }

  // Read Header First... If 0 return empty package
  uint8_t payload_size = 0;

  ssize_t headerbytesRead = read(i2cFile_, &payload_size, 1);
  if (headerbytesRead != 1) {
    std::cerr << "I2C Header read failed!\n";
    // Todo Return empty package?
    return msg;
  }

  // Get Payload of size specified in 1 byte header
  if (payload_size > 0) {

    std::cerr << std::dec << "payload_size: " << +payload_size << " Bytes\n";

    ssize_t payloadbytesRead = read(i2cFile_, &msg.payload, payload_size);
    if (payloadbytesRead != payload_size) {
      std::cerr << "I2C Payload Read failed!\n";
      // Todo Return empty package?
      msg.msg_type = 0;
      return msg;
    } else {
      msg.msg_type = msg.payload[0];
      return msg;
    }
  }
  return msg;
}
