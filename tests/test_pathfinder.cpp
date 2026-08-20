#include <cstddef>
#include <vector>

#include "doctest/doctest.h"
#include "rts/pathfinder.hpp"

using rts::Battlefield;
using rts::Path;
using rts::Position;
using rts::Terrain;

namespace {

// A path is valid when it starts at `start`, ends at `target`, every
// consecutive pair is 4-adjacent (differs by exactly one step), and every
// position on it is ground. Shared by every success-case test below.
bool isValidPath(const Battlefield& field, const Path& path, Position start, Position target) {
  if (path.empty() || path.front() != start || path.back() != target) {
    return false;
  }
  for (std::size_t i = 0; i < path.size(); ++i) {
    if (!field.isGround(path[i])) {
      return false;
    }
    if (i > 0) {
      const int rowDelta = path[i].row - path[i - 1].row;
      const int colDelta = path[i].col - path[i - 1].col;
      const int step = (rowDelta < 0 ? -rowDelta : rowDelta) + (colDelta < 0 ? -colDelta : colDelta);
      if (step != 1) {
        return false;
      }
    }
  }
  return true;
}

}  // namespace

TEST_CASE("start equal to target returns a single-position path") {
  const Battlefield field(1, 1, {Terrain::Ground});
  const auto path = rts::findPathBfs(field, {0, 0}, {0, 0});

  REQUIRE(path.has_value());
  CHECK(*path == Path{{0, 0}});
}

TEST_CASE("finds the shortest path across an open field") {
  const Battlefield field(4, 4, std::vector<Terrain>(16, Terrain::Ground));
  const Position start{0, 0};
  const Position target{3, 3};

  const auto path = rts::findPathBfs(field, start, target);

  REQUIRE(path.has_value());
  CHECK(isValidPath(field, *path, start, target));
  CHECK(path->size() == 7);  // Manhattan distance 6 -> 7 positions
}

TEST_CASE("finds a path around a wall, recovering from the dead end it creates") {
  // col:  0 1 2 3
  // row0: S . # .
  // row1: . . # .
  // row2: . . . .
  const Battlefield field(3, 4,
                           {Terrain::Ground, Terrain::Ground, Terrain::Elevated, Terrain::Ground,
                            Terrain::Ground, Terrain::Ground, Terrain::Elevated, Terrain::Ground,
                            Terrain::Ground, Terrain::Ground, Terrain::Ground, Terrain::Ground});
  const Position start{0, 0};
  const Position target{0, 3};

  const auto path = rts::findPathBfs(field, start, target);

  REQUIRE(path.has_value());
  CHECK(isValidPath(field, *path, start, target));
  CHECK(path->size() == 8);  // detour through row 2: BFS distance 7 -> 8 positions
}

TEST_CASE("returns no path when the target is walled off") {
  // col:  0 1 2
  // row0: S # T
  // row1: . # .
  const Battlefield field(2, 3,
                           {Terrain::Ground, Terrain::Elevated, Terrain::Ground,
                            Terrain::Ground, Terrain::Elevated, Terrain::Ground});

  CHECK_FALSE(rts::findPathBfs(field, {0, 0}, {0, 2}).has_value());
}

TEST_CASE("returns no path, not a crash, for an out-of-bounds start/target") {
  const Battlefield field(2, 2, std::vector<Terrain>(4, Terrain::Ground));

  CHECK_FALSE(rts::findPathBfs(field, {-1, 0}, {1, 1}).has_value());
  CHECK_FALSE(rts::findPathBfs(field, {0, 0}, {5, 5}).has_value());
  CHECK(rts::findPathBfs(field, {0, 0}, {0, 0}).has_value());  // sanity: valid case still works
}

TEST_CASE("returns no path for an elevated start or target") {
  // col:  0 1
  // row0: # .
  // row1: . #
  const Battlefield field(2, 2,
                           {Terrain::Elevated, Terrain::Ground, Terrain::Ground,
                            Terrain::Elevated});

  CHECK_FALSE(rts::findPathBfs(field, {0, 0}, {1, 0}).has_value());  // start is elevated
  CHECK_FALSE(rts::findPathBfs(field, {0, 1}, {1, 1}).has_value());  // target is elevated
}

TEST_CASE("finds the only path through a single-row corridor") {
  const Battlefield field(1, 5, std::vector<Terrain>(5, Terrain::Ground));
  const Position start{0, 0};
  const Position target{0, 4};

  const auto path = rts::findPathBfs(field, start, target);

  REQUIRE(path.has_value());
  CHECK(isValidPath(field, *path, start, target));
  CHECK(path->size() == 5);
}

TEST_CASE("is deterministic: repeated calls on the same input return the identical path") {
  const Battlefield field(4, 4, std::vector<Terrain>(16, Terrain::Ground));
  const auto first = rts::findPathBfs(field, {0, 0}, {3, 3});
  const auto second = rts::findPathBfs(field, {0, 0}, {3, 3});

  REQUIRE(first.has_value());
  REQUIRE(second.has_value());
  CHECK(*first == *second);
}
