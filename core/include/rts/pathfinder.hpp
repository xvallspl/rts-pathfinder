#pragma once

#include <optional>
#include <vector>

#include "rts/battlefield.hpp"

namespace rts {

using Path = std::vector<Position>;

// Shortest path from start to target via 4-connected, unit-cost moves,
// found by BFS. std::nullopt covers both "unreachable" and "start/target
// isn't ground" -- it doesn't distinguish why. start == target returns
// {start}.
std::optional<Path> findPathBfs(const Battlefield& field, Position start, Position target);

}  // namespace rts
