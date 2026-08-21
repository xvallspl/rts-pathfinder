#pragma once

#include <optional>
#include <vector>

#include "rts/battlefield.hpp"
#include "rts/pathfinder.hpp"

namespace rts {

// Routes several units to several targets, all moving at once, one step per
// tick. Every returned path has the same length and is indexed by tick, so
// paths[unit][tick] is where that unit stands at that tick, and a cell
// repeated on consecutive ticks means it held position.
//
// Targets are claimed nearest-first; with more units than targets, the
// leftovers hold their starting cell. Requires units >= targets >= 1.
// Returns nullopt when no collision-free plan was found -- see the README's
// Design Decisions for the movement rules and where they fall short.
std::optional<std::vector<Path>> findPathsBfs(const Battlefield& field,
                                               const std::vector<Position>& starts,
                                               const std::vector<Position>& targets);

}  // namespace rts
