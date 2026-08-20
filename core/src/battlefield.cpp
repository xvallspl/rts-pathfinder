#include "rts/battlefield.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace rts {

Battlefield::Battlefield(int rows, int cols, std::vector<Terrain> cells)
    : mRows(rows), mCols(cols), mCells(std::move(cells)) {
  if (rows <= 0 || cols <= 0) {
    throw std::invalid_argument("Battlefield dimensions must be positive");
  }
  if (mCells.size() != static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols)) {
    throw std::invalid_argument("Battlefield cell count does not match rows * cols");
  }
}

Terrain Battlefield::terrainAt(Position pos) const {
  assert(inBounds(pos) && "Battlefield::terrainAt: position out of bounds");
  return mCells[index(pos)];
}

std::array<Position, 4> neighbors4(Position pos) {
  return {{
      {.row = pos.row - 1, .col = pos.col},
      {.row = pos.row, .col = pos.col + 1},
      {.row = pos.row + 1, .col = pos.col},
      {.row = pos.row, .col = pos.col - 1},
  }};
}

}  // namespace rts
