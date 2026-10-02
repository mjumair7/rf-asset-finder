#include "protocol.hpp"

#include <stdexcept>

namespace rf {
namespace {
constexpr std::uint8_t kMagic0 = 'R';
constexpr std::uint8_t kMagic1 = 'F';
constexpr std::size_t kHeaderBytes = 9;

void append_u16(std::vector<std::uint8_t>& out, std::uint16_t value) {
  out.push_back(static_cast<std::uint8_t>(value >> 8));
  out.push_back(static_cast<std::uint8_t>(value & 0xFF));
}

std::uint16_t read_u16(const std::vector<std::uint8_t>& bytes, std::size_t index) {
  return static_cast<std::uint16_t>((bytes[index] << 8) | bytes[index + 1]);
}
}  // namespace

bool Packet::operator==(const Packet& other) const {
  return type == other.type && device_id == other.device_id &&
         sequence == other.sequence && payload == other.payload;
}

std::uint16_t crc16_ccitt(const std::uint8_t* data, std::size_t length) {
  std::uint16_t crc = 0xFFFF;
  for (std::size_t i = 0; i < length; ++i) {
    crc ^= static_cast<std::uint16_t>(data[i]) << 8;
    for (int bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x8000) ? static_cast<std::uint16_t>((crc << 1) ^ 0x1021)
                           : static_cast<std::uint16_t>(crc << 1);
    }
  }
  return crc;
}

std::vector<std::uint8_t> encode(const Packet& packet) {
  if (packet.payload.size() > kMaxPayload) {
    throw std::invalid_argument("RF payload exceeds 16 bytes");
  }

  std::vector<std::uint8_t> frame{kMagic0, kMagic1, kProtocolVersion,
                                  static_cast<std::uint8_t>(packet.type)};
  append_u16(frame, packet.device_id);
  append_u16(frame, packet.sequence);
  frame.push_back(static_cast<std::uint8_t>(packet.payload.size()));
  frame.insert(frame.end(), packet.payload.begin(), packet.payload.end());
  append_u16(frame, crc16_ccitt(frame.data(), frame.size()));
  return frame;
}

std::optional<Packet> decode(const std::vector<std::uint8_t>& frame) {
  if (frame.size() < kHeaderBytes + 2 || frame[0] != kMagic0 ||
      frame[1] != kMagic1 || frame[2] != kProtocolVersion) {
    return std::nullopt;
  }

  if (frame[3] != static_cast<std::uint8_t>(PacketType::Beacon) &&
      frame[3] != static_cast<std::uint8_t>(PacketType::Acknowledgement)) {
    return std::nullopt;
  }

  const auto payload_length = static_cast<std::size_t>(frame[8]);
  if (payload_length > kMaxPayload || frame.size() != kHeaderBytes + payload_length + 2) {
    return std::nullopt;
  }

  const auto expected = read_u16(frame, frame.size() - 2);
  const auto actual = crc16_ccitt(frame.data(), frame.size() - 2);
  if (expected != actual) return std::nullopt;

  Packet packet;
  packet.type = static_cast<PacketType>(frame[3]);
  packet.device_id = read_u16(frame, 4);
  packet.sequence = read_u16(frame, 6);
  packet.payload.assign(frame.begin() + kHeaderBytes, frame.end() - 2);
  return packet;
}

}  // namespace rf
