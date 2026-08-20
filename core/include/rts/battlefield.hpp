#pragma once

#include <array>
#include <cstddef>
#include <vector>

namespace rts {

struct Position {
  int row = 0;
  int col = 0;
};

constexpr bool operator==(const Position& a, const Position& b) {
  return a.row == b.row && a.col == b.col;
}

enum class Terrain {
  Ground,
  Elevated,
};

// Row-major grid of terrain. Immutable once constructed.
class Battlefield {
 public:
  // Throws std::invalid_argument if rows/cols aren't positive, or if
  // cells.size() != rows * cols.
  Battlefield(int rows, int cols, std::vector<Terrain> cells);

  int rows() const { return mRows; }
  int cols() const { return mCols; }
  std::size_t cellCount() const { return mCells.size(); }

  bool inBounds(Position pos) const {
    return pos.row >= 0 && pos.row < mRows && pos.col >= 0 && pos.col < mCols;
  }

  // Precondition: inBounds(pos). Use isGround() when pos's validity isn't already known.
  Terrain terrainAt(Position pos) const;

  bool isGround(Position pos) const { return inBounds(pos) && terrainAt(pos) == Terrain::Ground; }

  // Flat row-major index. Precondition: inBounds(pos).
  std::size_t index(Position pos) const {
    return (static_cast<std::size_t>(pos.row) * static_cast<std::size_t>(mCols)) +
           static_cast<std::size_t>(pos.col);
  }

 private:
  int mRows;
  int mCols;
  std::vector<Terrain> mCells;
};

// North, East, South, West, in that fixed order. The fixed order makes
// search exploration deterministic: identical inputs always produce
// identical paths, which tests and committed sample outputs rely on.
std::array<Position, 4> neighbors4(Position pos);

}  // namespace rts
