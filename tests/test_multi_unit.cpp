#include <cstddef>
#include <vector>

#include "doctest/doctest.h"
#include "rts/multi_unit.hpp"
#include "rts/pathfinder.hpp"

using rts::Battlefield;
using rts::Path;
using rts::Position;
using rts::Terrain;

namespace {

Battlefield openField(int rows, int cols) {
  return {rows, cols, std::vector<Terrain>(static_cast<std::size_t>(rows * cols), Terrain::Ground)};
}

int stepsBetween(Position a, Position b) {
  return std::abs(a.row - b.row) + std::abs(a.col - b.col);
}

// The rules a plan has to obey. Note what is deliberately absent: two units
// exchanging cells is allowed, because the task document constrains only how
// many units may stand on a position at once.
bool isValidPlan(const Battlefield& field, const std::vector<Path>& paths,
                 const std::vector<Position>& starts) {
  if (paths.size() != starts.size() || paths.empty()) {
    return false;
  }
  const std::size_t ticks = paths.front().size();

  for (std::size_t unit = 0; unit < paths.size(); ++unit) {
    if (paths[unit].size() != ticks || paths[unit].front() != starts[unit]) {
      return false;
    }
    for (std::size_t tick = 0; tick < ticks; ++tick) {
      if (!field.isGround(paths[unit][tick])) {
        return false;
      }
      // One step per tick, or none at all -- never a jump or a diagonal.
      if (tick > 0 && stepsBetween(paths[unit][tick - 1], paths[unit][tick]) > 1) {
        return false;
      }
    }
  }

  for (std::size_t tick = 0; tick < ticks; ++tick) {
    for (std::size_t a = 0; a < paths.size(); ++a) {
      for (std::size_t b = a + 1; b < paths.size(); ++b) {
        if (paths[a][tick] == paths[b][tick]) {
          return false;
        }
      }
    }
  }
  return true;
}

bool neverMoves(const Path& path) {
  for (const Position& pos : path) {
    if (!(pos == path.front())) {
      return false;
    }
  }
  return true;
}

bool holdsPositionAtSomePoint(const Path& path) {
  for (std::size_t tick = 1; tick < path.size(); ++tick) {
    if (path[tick] == path[tick - 1]) {
      return true;
    }
  }
  return false;
}

}  // namespace

TEST_CASE("one unit and one target is the ordinary single-unit case") {
  const Battlefield field = openField(4, 4);
  const std::vector<Position> starts{{.row = 0, .col = 0}};
  const std::vector<Position> targets{{.row = 3, .col = 3}};

  const auto paths = rts::findPathsBfs(field, starts, targets);

  REQUIRE(paths.has_value());
  CHECK(isValidPlan(field, *paths, starts));
  CHECK((*paths)[0].back() == Position{3, 3});
  CHECK((*paths)[0].size() == rts::findPathBfs(field, {0, 0}, {3, 3})->size());
}

TEST_CASE("units whose routes never meet both reach their targets") {
  const Battlefield field = openField(4, 4);
  const std::vector<Position> starts{{.row = 0, .col = 0}, {.row = 3, .col = 3}};
  const std::vector<Position> targets{{.row = 0, .col = 3}, {.row = 3, .col = 0}};

  const auto paths = rts::findPathsBfs(field, starts, targets);

  REQUIRE(paths.has_value());
  CHECK(isValidPlan(field, *paths, starts));
  CHECK((*paths)[0].back() == Position{0, 3});
  CHECK((*paths)[1].back() == Position{3, 0});
}

TEST_CASE("a unit settling in a corridor can strand the one behind it") {
  // Both targets lie up the corridor. The leading unit is nearest to the
  // nearer one, claims it, and stops there for good -- which walls the
  // corridor off before the unit behind can pass. Planning the two in the
  // other order would have worked, which is exactly the incompleteness that
  // comes with routing units one at a time; the planner reports it rather
  // than searching for a better order.
  const Battlefield field = openField(1, 4);
  const std::vector<Position> starts{{.row = 0, .col = 0}, {.row = 0, .col = 1}};
  const std::vector<Position> targets{{.row = 0, .col = 2}, {.row = 0, .col = 3}};

  CHECK_FALSE(rts::findPathsBfs(field, starts, targets).has_value());
}

TEST_CASE("a unit holds position to let another cross in front of it") {
  // The two routes cross at the centre cell. Whoever is routed second finds
  // it taken on the tick it wanted it, and waiting one tick beats walking
  // the long way round.
  const Battlefield field = openField(3, 3);
  const std::vector<Position> starts{{.row = 1, .col = 0}, {.row = 0, .col = 1}};
  const std::vector<Position> targets{{.row = 1, .col = 2}, {.row = 2, .col = 1}};

  const auto paths = rts::findPathsBfs(field, starts, targets);

  REQUIRE(paths.has_value());
  CHECK(isValidPlan(field, *paths, starts));
  CHECK((*paths)[0].back() == Position{1, 2});
  CHECK((*paths)[1].back() == Position{2, 1});
  CHECK(holdsPositionAtSomePoint((*paths)[1]));
}

TEST_CASE("units already standing on the targets need not move at all") {
  // Nearest-first claiming gives each unit the target under its own feet, so
  // nobody has to trade places with anybody.
  const Battlefield field = openField(1, 2);
  const std::vector<Position> starts{{.row = 0, .col = 0}, {.row = 0, .col = 1}};
  const std::vector<Position> targets{{.row = 0, .col = 1}, {.row = 0, .col = 0}};

  const auto paths = rts::findPathsBfs(field, starts, targets);

  REQUIRE(paths.has_value());
  CHECK(isValidPlan(field, *paths, starts));
  CHECK(neverMoves((*paths)[0]));
  CHECK(neverMoves((*paths)[1]));
}

TEST_CASE("with more units than targets, the ones left over stay put") {
  const Battlefield field = openField(1, 5);
  const std::vector<Position> starts{{.row = 0, .col = 0}, {.row = 0, .col = 3}};
  const std::vector<Position> targets{{.row = 0, .col = 4}};  // the unit at (0,3) is far closer

  const auto paths = rts::findPathsBfs(field, starts, targets);

  REQUIRE(paths.has_value());
  CHECK(isValidPlan(field, *paths, starts));
  CHECK((*paths)[1].back() == Position{0, 4});
  CHECK(neverMoves((*paths)[0]));
}

TEST_CASE("the target goes to whoever can walk there soonest, not who looks closest") {
  // Column 2 is a wall with its only gap on the bottom row. The unit at
  // (0,1) is two cells from the target as the crow flies but ten on foot;
  // the one at (4,4) is five away by either measure and should claim it.
  std::vector<Terrain> cells(25, Terrain::Ground);
  for (int row = 0; row < 4; ++row) {
    cells[static_cast<std::size_t>((row * 5) + 2)] = Terrain::Elevated;
  }
  const Battlefield field(5, 5, std::move(cells));
  const std::vector<Position> starts{{.row = 0, .col = 1}, {.row = 4, .col = 4}};
  const std::vector<Position> targets{{.row = 0, .col = 3}};

  const auto paths = rts::findPathsBfs(field, starts, targets);

  REQUIRE(paths.has_value());
  CHECK(isValidPlan(field, *paths, starts));
  CHECK((*paths)[1].back() == Position{0, 3});
  CHECK(neverMoves((*paths)[0]));
}

TEST_CASE("fewer units than targets is rejected") {
  const Battlefield field = openField(4, 4);
  const std::vector<Position> starts{{.row = 0, .col = 0}};
  const std::vector<Position> targets{{.row = 0, .col = 3}, {.row = 3, .col = 0}};

  CHECK_FALSE(rts::findPathsBfs(field, starts, targets).has_value());
}

TEST_CASE("no plan when a target is walled off from every unit") {
  // Column 1 is solid, so nothing can reach column 2.
  const Battlefield field(
      3, 3,
      {Terrain::Ground, Terrain::Elevated, Terrain::Ground, Terrain::Ground, Terrain::Elevated,
       Terrain::Ground, Terrain::Ground, Terrain::Elevated, Terrain::Ground});
  const std::vector<Position> starts{{.row = 0, .col = 0}, {.row = 1, .col = 0}};
  const std::vector<Position> targets{{.row = 0, .col = 2}};

  CHECK_FALSE(rts::findPathsBfs(field, starts, targets).has_value());
}

TEST_CASE("no targets at all is rejected") {
  const Battlefield field = openField(3, 3);
  CHECK_FALSE(rts::findPathsBfs(field, {{0, 0}}, {}).has_value());
}
