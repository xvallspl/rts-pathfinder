#include "rts/multi_unit.hpp"

#include <algorithm>
#include <cstddef>
#include <queue>
#include <tuple>
#include <unordered_set>
#include <utility>

namespace rts {

namespace {

constexpr int kUnreachable = -1;
constexpr int kNever = -1;
constexpr int kNoParent = -1;

// How many ticks a unit may take before the search gives up. Generous enough
// that only a genuinely stuck unit reaches it.
int horizonFor(const Battlefield& field) { return 2 * static_cast<int>(field.cellCount()); }

// Walking distance from `origin` to every cell, going around elevated
// terrain rather than through it.
std::vector<int> distancesFrom(const Battlefield& field, Position origin) {
  std::vector<int> distance(field.cellCount(), kUnreachable);
  if (!field.isGround(origin)) {
    return distance;
  }

  std::queue<Position> frontier;
  distance[field.index(origin)] = 0;
  frontier.push(origin);

  while (!frontier.empty()) {
    const Position current = frontier.front();
    frontier.pop();
    for (const Position next : neighbors4(current)) {
      if (field.isGround(next) && distance[field.index(next)] == kUnreachable) {
        distance[field.index(next)] = distance[field.index(current)] + 1;
        frontier.push(next);
      }
    }
  }
  return distance;
}

struct Claim {
  std::size_t unit;
  std::size_t target;
};

// Hands each target to whichever unit can walk to it soonest, closest pair
// first. The order claims are made in is also the order routes are planned.
std::optional<std::vector<Claim>> claimTargets(const Battlefield& field,
                                                const std::vector<Position>& starts,
                                                const std::vector<Position>& targets) {
  struct Candidate {
    int distance;
    std::size_t target;
    std::size_t unit;
  };

  std::vector<Candidate> candidates;
  for (std::size_t target = 0; target < targets.size(); ++target) {
    const std::vector<int> distance = distancesFrom(field, targets[target]);
    for (std::size_t unit = 0; unit < starts.size(); ++unit) {
      if (!field.isGround(starts[unit])) {
        continue;
      }
      const int steps = distance[field.index(starts[unit])];
      if (steps != kUnreachable) {
        candidates.push_back({.distance = steps, .target = target, .unit = unit});
      }
    }
  }

  // Ties are broken by target then unit so the outcome never depends on the
  // order candidates happened to be generated in.
  std::ranges::sort(candidates, [](const Candidate& a, const Candidate& b) {
    return std::tie(a.distance, a.target, a.unit) < std::tie(b.distance, b.target, b.unit);
  });

  std::vector<bool> unitClaimed(starts.size(), false);
  std::vector<bool> targetClaimed(targets.size(), false);
  std::vector<Claim> claims;
  for (const Candidate& candidate : candidates) {
    if (unitClaimed[candidate.unit] || targetClaimed[candidate.target]) {
      continue;
    }
    unitClaimed[candidate.unit] = true;
    targetClaimed[candidate.target] = true;
    claims.push_back({.unit = candidate.unit, .target = candidate.target});
  }

  if (claims.size() != targets.size()) {
    return std::nullopt;  // some target no unit can reach
  }
  return claims;
}

// Units with nothing to claim never move, so they are simply terrain as far
// as route planning is concerned.
Battlefield withIdleUnitsAsTerrain(const Battlefield& field, const std::vector<Position>& starts,
                                    const std::vector<Claim>& claims) {
  std::vector<Terrain> cells;
  cells.reserve(field.cellCount());
  for (int row = 0; row < field.rows(); ++row) {
    for (int col = 0; col < field.cols(); ++col) {
      cells.push_back(field.terrainAt({.row = row, .col = col}));
    }
  }

  std::vector<bool> moves(starts.size(), false);
  for (const Claim& claim : claims) {
    moves[claim.unit] = true;
  }
  for (std::size_t unit = 0; unit < starts.size(); ++unit) {
    if (!moves[unit]) {
      cells[field.index(starts[unit])] = Terrain::Elevated;
    }
  }

  return {field.rows(), field.cols(), std::move(cells)};
}

// Where the units routed so far will be, and when.
class Reservations {
 public:
  Reservations(const Battlefield& field, int horizon)
      : mField(field),
        mHorizon(horizon),
        mHeldFrom(field.cellCount(), kNever),
        mLastUsed(field.cellCount(), kNever) {}

  bool cellFree(Position pos, int tick) const {
    if (!mField.isGround(pos)) {
      return false;
    }
    const std::size_t cell = mField.index(pos);
    if (mHeldFrom[cell] != kNever && tick >= mHeldFrom[cell]) {
      return false;  // a unit has settled here for good
    }
    return !mVertex.contains(key(cell, tick));
  }

  // A unit may only stop on its target once nobody else still needs to pass
  // through that cell.
  bool canSettleAt(Position pos, int tick) const {
    return tick > mLastUsed[mField.index(pos)];
  }

  void add(const Path& route) {
    for (std::size_t tick = 0; tick < route.size(); ++tick) {
      const std::size_t cell = mField.index(route[tick]);
      mVertex.insert(key(cell, static_cast<int>(tick)));
      mLastUsed[cell] = std::max(mLastUsed[cell], static_cast<int>(tick));
    }
    mHeldFrom[mField.index(route.back())] = static_cast<int>(route.size()) - 1;
  }

 private:
  std::size_t key(std::size_t cell, int tick) const {
    return (cell * static_cast<std::size_t>(mHorizon + 1)) + static_cast<std::size_t>(tick);
  }

  const Battlefield& mField;
  int mHorizon;
  std::vector<int> mHeldFrom;  // per cell: tick a unit settled on it for good
  std::vector<int> mLastUsed;  // per cell: last tick any routed unit is on it
  std::unordered_set<std::size_t> mVertex;
};

// Searching over (cell, tick) rather than just (cell): whether a cell can be
// entered depends on when you get there. Every move costs one tick -- waiting
// included -- so breadth-first search still finds the earliest arrival.
std::optional<Path> routeOneUnit(const Battlefield& field, const Reservations& reserved,
                                  Position start, Position target, int horizon) {
  if (!field.isGround(start) || !field.isGround(target) || !reserved.cellFree(start, 0)) {
    return std::nullopt;
  }

  const auto stateOf = [&](Position pos, int tick) {
    return (field.index(pos) * static_cast<std::size_t>(horizon + 1)) +
           static_cast<std::size_t>(tick);
  };

  std::vector<bool> visited(field.cellCount() * static_cast<std::size_t>(horizon + 1), false);
  std::vector<int> parent(visited.size(), kNoParent);

  std::queue<std::pair<Position, int>> frontier;
  visited[stateOf(start, 0)] = true;
  frontier.emplace(start, 0);

  while (!frontier.empty()) {
    const auto [current, tick] = frontier.front();
    frontier.pop();

    if (current == target && reserved.canSettleAt(current, tick)) {
      Path route{current};
      for (int state = parent[stateOf(current, tick)]; state != kNoParent;
           state = parent[static_cast<std::size_t>(state)]) {
        const auto cell = static_cast<int>(static_cast<std::size_t>(state) /
                                            static_cast<std::size_t>(horizon + 1));
        route.push_back({.row = cell / field.cols(), .col = cell % field.cols()});
      }
      std::ranges::reverse(route);
      return route;
    }

    const int nextTick = tick + 1;
    if (nextTick > horizon) {
      continue;
    }

    // Moving is tried before holding position, so a unit only waits when it
    // gains something by it. Both cost one tick.
    std::array<Position, 5> options{};
    const std::array<Position, 4> neighbors = neighbors4(current);
    std::ranges::copy(neighbors, options.begin());
    options[4] = current;

    for (const Position next : options) {
      const std::size_t state = stateOf(next, nextTick);
      if (visited[state] || !reserved.cellFree(next, nextTick)) {
        continue;
      }
      visited[state] = true;
      parent[state] = static_cast<int>(stateOf(current, tick));
      frontier.emplace(next, nextTick);
    }
  }

  return std::nullopt;
}

}  // namespace

std::optional<std::vector<Path>> findPathsBfs(const Battlefield& field,
                                               const std::vector<Position>& starts,
                                               const std::vector<Position>& targets) {
  if (targets.empty() || starts.size() < targets.size()) {
    return std::nullopt;
  }

  const std::optional<std::vector<Claim>> claims = claimTargets(field, starts, targets);
  if (!claims.has_value()) {
    return std::nullopt;
  }

  const Battlefield battlefield = withIdleUnitsAsTerrain(field, starts, *claims);
  const int horizon = horizonFor(battlefield);

  // Units that never move still need a path, so start everyone off standing
  // on their own cell and overwrite the ones that have somewhere to be.
  std::vector<Path> paths(starts.size());
  for (std::size_t unit = 0; unit < starts.size(); ++unit) {
    paths[unit] = Path{starts[unit]};
  }

  Reservations reserved(battlefield, horizon);
  for (const Claim& claim : *claims) {
    const std::optional<Path> route =
        routeOneUnit(battlefield, reserved, starts[claim.unit], targets[claim.target], horizon);
    if (!route.has_value()) {
      return std::nullopt;
    }
    reserved.add(*route);
    paths[claim.unit] = *route;
  }

  // The run ends when the last target is claimed; everyone else holds
  // position until then, so that a tick means the same thing in every path.
  std::size_t ticks = 0;
  for (const Path& path : paths) {
    ticks = std::max(ticks, path.size());
  }
  for (Path& path : paths) {
    path.resize(ticks, path.back());
  }

  return paths;
}

}  // namespace rts
