#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "nlohmann/json.hpp"
#include "rts/battlefield.hpp"

namespace rts {

// The single error type thrown across tilemap parsing: malformed JSON, a
// tile value outside the documented codes, undeterminable dimensions, or a
// document with no start or no target.
class MapError : public std::runtime_error {
 public:
  explicit MapError(const std::string& message) : std::runtime_error(message) {}
};

// Parses a RiskyLab Tilemap JSON document (https://riskylab.com/tilemap/)
// into a Battlefield plus unit start/target positions.
//
// Tile values must be exactly one of the codes the task document defines:
// -1 ground, 0 start, 8 target, 3 elevated. Any other value -- including a
// non-integer one -- is outside the documented format and throws MapError
// naming the offending cell; see the README's Design Decisions.
class TilemapDocument {
 public:
  // Throws MapError on malformed JSON, missing/mismatched layers[0].data,
  // undeterminable map dimensions, an out-of-domain tile value, or a
  // document with no start or no target.
  static TilemapDocument parse(std::string_view jsonText);

  const Battlefield& battlefield() const { return mBattlefield; }
  const std::vector<Position>& starts() const { return mStarts; }
  const std::vector<Position>& targets() const { return mTargets; }

  // Re-serializes the parsed document. Parsing does not mutate the raw
  // document, so this is byte-equivalent (modulo JSON formatting) to the input.
  std::string toJson() const;

 private:
  TilemapDocument(nlohmann::json raw, Battlefield battlefield, std::vector<Position> starts,
                   std::vector<Position> targets);

  nlohmann::json mRaw;
  Battlefield mBattlefield;
  std::vector<Position> mStarts;
  std::vector<Position> mTargets;
};

}  // namespace rts
