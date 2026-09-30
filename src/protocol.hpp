#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace rf {

constexpr std::uint8_t kProtocolVersion = 1;
constexpr std::size_t kMaxPayload = 16;

enum class PacketType : std::uint8_t {
  Beacon = 0x10,
  Acknowledgement = 0xA0,
};

struct Packet {
  PacketType type{PacketType::Beacon};
  std::uint16_t device_id{0};
  std::uint16_t sequence{0};
  std::vector<std::uint8_t> payload;

  bool operator==(const Packet& other) const;
};

std::uint16_t crc16_ccitt(const std::uint8_t* data, std::size_t length);
std::vector<std::uint8_t> encode(const Packet& packet);
std::optional<Packet> decode(const std::vector<std::uint8_t>& frame);

}  // namespace rf
