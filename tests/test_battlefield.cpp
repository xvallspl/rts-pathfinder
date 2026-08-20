#include <stdexcept>
#include <vector>

#include "doctest/doctest.h"
#include "rts/battlefield.hpp"

using rts::Battlefield;
using rts::Position;
using rts::Terrain;

TEST_CASE("neighbors4 returns North, East, South, West in that fixed order") {
  const auto n = rts::neighbors4({5, 5});
  CHECK(n[0] == Position{4, 5});
  CHECK(n[1] == Position{5, 6});
  CHECK(n[2] == Position{6, 5});
  CHECK(n[3] == Position{5, 4});
}

TEST_CASE("Battlefield rejects non-positive dimensions") {
  CHECK_THROWS_AS(Battlefield(0, 5, std::vector<Terrain>(0)), std::invalid_argument);
  CHECK_THROWS_AS(Battlefield(5, 0, std::vector<Terrain>(0)), std::invalid_argument);
}

TEST_CASE("Battlefield rejects a cell count that doesn't match rows * cols") {
  CHECK_THROWS_AS(Battlefield(2, 2, std::vector<Terrain>(3, Terrain::Ground)),
                   std::invalid_argument);
}

TEST_CASE("inBounds is true only strictly inside the grid") {
  const Battlefield field(2, 3, std::vector<Terrain>(6, Terrain::Ground));
  CHECK(field.inBounds({0, 0}));
  CHECK(field.inBounds({1, 2}));
  CHECK_FALSE(field.inBounds({-1, 0}));
  CHECK_FALSE(field.inBounds({0, -1}));
  CHECK_FALSE(field.inBounds({2, 0}));
  CHECK_FALSE(field.inBounds({0, 3}));
}

TEST_CASE("isGround combines the bounds check and the terrain check") {
  // . #
  // . .
  const Battlefield field(2, 2,
                           {Terrain::Ground, Terrain::Elevated, Terrain::Ground, Terrain::Ground});
  CHECK(field.isGround({0, 0}));
  CHECK_FALSE(field.isGround({0, 1}));  // elevated
  CHECK_FALSE(field.isGround({5, 5}));  // out of bounds
}

TEST_CASE("index is row-major") {
  const Battlefield field(3, 4, std::vector<Terrain>(12, Terrain::Ground));
  CHECK(field.index({0, 0}) == 0);
  CHECK(field.index({1, 0}) == 4);
  CHECK(field.index({2, 3}) == 11);
}
