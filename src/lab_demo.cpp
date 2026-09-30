#include "asset_tracker.hpp"

#include <iostream>

int main() {
  rf::AssetTracker tracker(3000);
  tracker.observe({rf::PacketType::Beacon, 1, 10, {0x0E, 0x74}}, 1000);
  tracker.observe({rf::PacketType::Beacon, 2, 41, {0x0E, 0x42}}, 1400);
  tracker.observe({rf::PacketType::Beacon, 3, 7, {0x0E, 0x88}}, 2200);
  tracker.observe({rf::PacketType::Beacon, 1, 11, {0x0E, 0x70}}, 4300);

  constexpr std::uint64_t now_ms = 5000;
  std::cout << "RF ASSET FINDER / " << now_ms << "ms\n";
  for (const auto& asset : tracker.snapshot(now_ms)) {
    std::cout << rf::describe(asset, now_ms) << '\n';
  }
}
