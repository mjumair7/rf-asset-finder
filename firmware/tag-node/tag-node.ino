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

bool sendBeacon() {
  uint8_t frame[13] = {'R', 'F', 1, 0x10, 0, 0, 0, 0, 2, 0, 0, 0, 0};
  writeU16(frame + 4, kTagId);
  writeU16(frame + 6, ++sequenceNumber);
  writeU16(frame + 9, 3700);
  writeU16(frame + 11, crc16(frame, 11));
  for (uint8_t attempt = 0; attempt < 3; ++attempt) {
    radio.send(frame, sizeof(frame));
    radio.waitPacketSent();
    if (radio.waitAvailableTimeout(160)) {
      uint8_t response[8];
      uint8_t length = sizeof(response);
      if (radio.recv(response, &length) && length >= 4 && response[0] == 'R' &&
          response[1] == 'F' && response[3] == 0xA0) return true;
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
