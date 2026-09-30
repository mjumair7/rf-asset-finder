#include "asset_tracker.hpp"

#include <sstream>

namespace rf {

AssetTracker::AssetTracker(std::uint64_t timeout_ms) : timeout_ms_(timeout_ms) {}

bool AssetTracker::observe(const Packet& packet, std::uint64_t now_ms) {
  if (packet.type != PacketType::Beacon) return false;
  auto& asset = assets_[packet.device_id];
  if (asset.last_seen_ms != 0 && packet.sequence <= asset.last_sequence) return false;

  asset.device_id = packet.device_id;
  asset.last_sequence = packet.sequence;
  asset.last_seen_ms = now_ms;
  asset.online = true;
  if (packet.payload.size() >= 2) {
    asset.battery_mv = static_cast<std::uint16_t>((packet.payload[0] << 8) | packet.payload[1]);
  }
  return true;
}

std::vector<AssetState> AssetTracker::snapshot(std::uint64_t now_ms) {
  std::vector<AssetState> result;
  for (auto& [id, asset] : assets_) {
    (void)id;
    asset.online = now_ms >= asset.last_seen_ms && now_ms - asset.last_seen_ms <= timeout_ms_;
    result.push_back(asset);
  }
  return result;
}

std::string describe(const AssetState& asset, std::uint64_t now_ms) {
  std::ostringstream out;
  const auto age = now_ms >= asset.last_seen_ms ? now_ms - asset.last_seen_ms : 0;
  out << "TAG-" << asset.device_id << "  " << (asset.online ? "ONLINE " : "MISSING")
      << "  last_seen=" << age << "ms  battery=" << asset.battery_mv << "mV";
  return out.str();
}

}  // namespace rf
