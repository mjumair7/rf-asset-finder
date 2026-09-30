#include <RH_ASK.h>
#include <SPI.h>

RH_ASK radio(2000, 11, 12, 10);  // bitrate, RX, TX, PTT

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

void setup() {
  Serial.begin(115200);
  if (!radio.init()) Serial.println("radio init failed");
}

void loop() {
  uint8_t frame[32];
  uint8_t length = sizeof(frame);
  if (!radio.recv(frame, &length) || length < 11) return;
  const uint16_t suppliedCrc = readU16(frame + length - 2);
  if (frame[0] != 'R' || frame[1] != 'F' || frame[2] != 1 ||
      static_cast<size_t>(length) != static_cast<size_t>(frame[8]) + 11 ||
      crc16(frame, length - 2) != suppliedCrc) {
    Serial.println("rejected corrupt frame");
    return;
  }

  uint8_t ack[11] = {'R', 'F', 1, 0xA0, 0, 0, 0, 0, 0, 0, 0};
  writeU16(ack + 4, readU16(frame + 4));
  writeU16(ack + 6, readU16(frame + 6));
  writeU16(ack + 9, crc16(ack, 9));
  radio.send(ack, sizeof(ack));
  radio.waitPacketSent();
  Serial.println("valid beacon acknowledged");
}
