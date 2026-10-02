#include "asset_tracker.hpp"
#include "protocol.hpp"

#include <cassert>
#include <iostream>

int main() {
  const rf::Packet beacon{rf::PacketType::Beacon, 42, 17, {0x0E, 0x74}};
  const auto encoded = rf::encode(beacon);
  const auto decoded = rf::decode(encoded);
  assert(decoded.has_value());
  assert(decoded.value() == beacon);

  auto corrupt = encoded;
  corrupt[6] ^= 0x40;
  assert(!rf::decode(corrupt).has_value());

  auto unknown_type = encoded;
  unknown_type[3] = 0x7F;
  const auto unknown_crc = rf::crc16_ccitt(unknown_type.data(), unknown_type.size() - 2);
  unknown_type[unknown_type.size() - 2] = static_cast<std::uint8_t>(unknown_crc >> 8);
  unknown_type[unknown_type.size() - 1] = static_cast<std::uint8_t>(unknown_crc & 0xFF);
  assert(!rf::decode(unknown_type).has_value());

  auto wrong_length = encoded;
  wrong_length[8] = 3;
  assert(!rf::decode(wrong_length).has_value());

  rf::AssetTracker tracker(1000);
  assert(tracker.observe(beacon, 100));
  assert(!tracker.observe(beacon, 120));
  assert(tracker.snapshot(900).front().online);
  assert(!tracker.snapshot(1200).front().online);

  const rf::Packet acknowledgement{rf::PacketType::Acknowledgement, 42, 18, {}};
  assert(!tracker.observe(acknowledgement, 1300));

  std::cout << "protocol and timeout tests passed\n";
}
