#include <RH_ASK.h>
#include <SPI.h>

RH_ASK radio(2000, 11, 12, 10);  // bitrate, RX, TX, PTT
constexpr uint16_t kTagId = 1;
uint16_t sequenceNumber = 0;

uint16_t crc16(const uint8_t* data, size_t length) {
  uint16_t crc = 0xFFFF;
  while (length--) {
    crc ^= static_cast<uint16_t>(*data++) << 8;
    for (uint8_t bit = 0; bit < 8; ++bit) crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
  }
  return crc;
}

void writeU16(uint8_t* target, uint16_t value) {
  target[0] = static_cast<uint8_t>(value >> 8);
  target[1] = static_cast<uint8_t>(value & 0xFF);
}

uint16_t readU16(const uint8_t* source) {
  return static_cast<uint16_t>(source[0] << 8 | source[1]);
}

bool validAck(const uint8_t* response, uint8_t length, uint16_t expectedSequence) {
  if (length != 11 || response[0] != 'R' || response[1] != 'F' ||
      response[2] != 1 || response[3] != 0xA0 || response[8] != 0) {
    return false;
  }
  if (readU16(response + 4) != kTagId || readU16(response + 6) != expectedSequence) {
    return false;
  }
  return readU16(response + 9) == crc16(response, 9);
}

bool sendBeacon() {
  uint8_t frame[13] = {'R', 'F', 1, 0x10, 0, 0, 0, 0, 2, 0, 0, 0, 0};
  writeU16(frame + 4, kTagId);
  const uint16_t currentSequence = ++sequenceNumber;
  writeU16(frame + 6, currentSequence);
  writeU16(frame + 9, 3700);
  writeU16(frame + 11, crc16(frame, 11));
  for (uint8_t attempt = 0; attempt < 3; ++attempt) {
    radio.send(frame, sizeof(frame));
    radio.waitPacketSent();
    if (radio.waitAvailableTimeout(160)) {
      uint8_t response[8];
      uint8_t length = sizeof(response);
      if (radio.recv(response, &length) && validAck(response, length, currentSequence)) return true;
    }
    delay(40 + attempt * 35);
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  if (!radio.init()) Serial.println("radio init failed");
}

void loop() {
  Serial.println(sendBeacon() ? "beacon acknowledged" : "beacon timed out");
  delay(1500);
}
