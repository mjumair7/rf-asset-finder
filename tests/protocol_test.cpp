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

  rf::AssetTracker tracker(1000);
  assert(tracker.observe(beacon, 100));
  assert(!tracker.observe(beacon, 120));
  assert(tracker.snapshot(900).front().online);
  assert(!tracker.snapshot(1200).front().online);

  std::cout << "protocol and timeout tests passed\n";
}
