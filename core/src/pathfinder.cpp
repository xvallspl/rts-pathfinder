#include "rts/pathfinder.hpp"

#include <algorithm>
#include <queue>

namespace rts {

namespace {

constexpr int kNoParent = -1;

Path reconstructPath(const Battlefield& field, const std::vector<int>& parent, Position target) {
  Path path{target};
  for (int i = parent[field.index(target)]; i != kNoParent; i = parent[i]) {
    path.push_back({.row = i / field.cols(), .col = i % field.cols()});
  }
  std::ranges::reverse(path);
  return path;
}

}  // namespace

std::optional<Path> findPathBfs(const Battlefield& field, Position start, Position target) {
  if (!field.isGround(start) || !field.isGround(target)) {
    return std::nullopt;
  }

  if (start == target) {
    return Path{start};
  }

  std::vector<bool> visited(field.cellCount(), false);
  std::vector<int> parent(field.cellCount(), kNoParent);

  std::queue<Position> frontier;
  visited[field.index(start)] = true;
  frontier.push(start);

  while (!frontier.empty()) {
    const Position current = frontier.front();
    frontier.pop();

    for (const Position next : neighbors4(current)) {
      if (!field.isGround(next)) {
        continue;
      }
      const std::size_t nextIndex = field.index(next);
      if (visited[nextIndex]) {
        continue;
      }
      visited[nextIndex] = true;
      parent[nextIndex] = static_cast<int>(field.index(current));
      if (next == target) {
        return reconstructPath(field, parent, target);
      }
      frontier.push(next);
    }
  }

  return std::nullopt;
}

}  // namespace rts
