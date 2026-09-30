#pragma once

#include "protocol.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace rf {

struct AssetState {
  std::uint16_t device_id{0};
  std::uint16_t last_sequence{0};
  std::uint64_t last_seen_ms{0};
  std::uint16_t battery_mv{0};
  bool online{false};
};

class AssetTracker {
 public:
  explicit AssetTracker(std::uint64_t timeout_ms);
  bool observe(const Packet& packet, std::uint64_t now_ms);
  std::vector<AssetState> snapshot(std::uint64_t now_ms);

 private:
  std::uint64_t timeout_ms_;
  std::map<std::uint16_t, AssetState> assets_;
};

std::string describe(const AssetState& asset, std::uint64_t now_ms);

}  // namespace rf
