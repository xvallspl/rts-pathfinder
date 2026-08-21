#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "nlohmann/json.hpp"
#include "rts/tilemap_json.hpp"

using rts::MapError;
using rts::Position;
using rts::TilemapDocument;

namespace {

// tilewidth/tileheight of 1 makes the canvas-derived dimensions equal to
// (rows, cols) directly, so tests can build tiny maps without doing tile
// arithmetic in their own head.
std::string makeMapJson(int rows, int cols, std::vector<double> data) {
  nlohmann::json doc;
  doc["layers"] = nlohmann::json::array({{{"name", "world"}, {"data", std::move(data)}}});
  doc["tilesets"] = nlohmann::json::array({{{"tilewidth", 1}, {"tileheight", 1}}});
  doc["canvas"] = {{"width", cols}, {"height", rows}};
  return doc.dump();
}

std::string readFile(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    throw std::runtime_error("cannot open " + path);
  }
  std::ostringstream contents;
  contents << in.rdbuf();
  return contents.str();
}

// Parses `jsonText`, expecting a MapError whose message contains `needle`.
void checkRejected(const std::string& jsonText, const std::string& needle) {
  bool threw = false;
  try {
    TilemapDocument::parse(jsonText);
  } catch (const MapError& e) {
    threw = true;
    CHECK(std::string(e.what()).find(needle) != std::string::npos);
  }
  CHECK(threw);
}

}  // namespace

TEST_CASE("parses starts, target, and elevated terrain from a small map") {
  // row0: start  ground elevated
  // row1: ground target ground
  const auto doc = TilemapDocument::parse(makeMapJson(2, 3, {0, -1, 3, -1, 8, -1}));

  CHECK(doc.starts() == std::vector<Position>{{0, 0}});
  CHECK(doc.targets() == std::vector<Position>{{1, 1}});
  CHECK(doc.battlefield().isGround({0, 0}));
  CHECK_FALSE(doc.battlefield().isGround({0, 2}));
  CHECK(doc.battlefield().isGround({1, 1}));
}

TEST_CASE("a value outside the documented codes is rejected, naming the cell") {
  checkRejected(makeMapJson(1, 3, {0, 5, 8}), "(0,1)");
}

TEST_CASE("a fractional value is rejected -- codes are integers, period") {
  checkRejected(makeMapJson(1, 2, {0, 8.5}), "(0,1)");
}

TEST_CASE("throws when the map has no starting position") {
  CHECK_THROWS_AS(TilemapDocument::parse(makeMapJson(1, 2, {-1, 8})), MapError);
}

TEST_CASE("throws when the map has no target position") {
  CHECK_THROWS_AS(TilemapDocument::parse(makeMapJson(1, 2, {0, -1})), MapError);
}

TEST_CASE("throws on malformed JSON") {
  CHECK_THROWS_AS(TilemapDocument::parse("{ this is not valid json"), MapError);
}

TEST_CASE("throws when layers[0].data does not match the derived dimensions") {
  // canvas/tileset says 2x2 == 4 cells; only 3 are provided.
  CHECK_THROWS_AS(TilemapDocument::parse(makeMapJson(2, 2, {0, -1, 8})), MapError);
}

TEST_CASE("resolves dimensions from explicit tileswide/tileshigh when present") {
  nlohmann::json doc;
  doc["tileswide"] = 3;
  doc["tileshigh"] = 1;
  doc["layers"] = nlohmann::json::array({{{"name", "world"}, {"data", {0, -1, 8}}}});

  const auto parsed = TilemapDocument::parse(doc.dump());
  CHECK(parsed.battlefield().rows() == 1);
  CHECK(parsed.battlefield().cols() == 3);
}

TEST_CASE("throws on negative tileswide/tileshigh instead of wrapping to a bogus size") {
  // -2 x -2 wraps to size_t 4 when cast unsigned, which would otherwise match
  // a genuine 4-entry data array and slip past the size check undetected.
  nlohmann::json doc;
  doc["tileswide"] = -2;
  doc["tileshigh"] = -2;
  doc["layers"] = nlohmann::json::array({{{"name", "world"}, {"data", {0, -1, -1, 8}}}});

  CHECK_THROWS_AS(TilemapDocument::parse(doc.dump()), MapError);
}

TEST_CASE("throws when map dimensions cannot be determined at all") {
  nlohmann::json doc;
  doc["layers"] = nlohmann::json::array({{{"name", "world"}, {"data", {0, -1, 8}}}});
  // No tileswide/tileshigh, no canvas/tilesets.

  CHECK_THROWS_AS(TilemapDocument::parse(doc.dump()), MapError);
}

TEST_CASE("the interviewer's sample map does not conform to the documented codes and is rejected") {
  // Contains 0.5 and 8.1, which are not in {-1, 0, 3, 8}; see the README's
  // Design Decisions. (8,30)=0.5 is hit before (25,0)=8.1 in row-major order.
  checkRejected(readFile(std::string(RTS_SAMPLES_DIR) + "/interviewer_sample.json"), "(8,30)");
}

TEST_CASE("parses our own spec-conformant sample map") {
  const auto doc = TilemapDocument::parse(readFile(std::string(RTS_SAMPLES_DIR) + "/sample_map.json"));

  CHECK(doc.battlefield().rows() == 32);
  CHECK(doc.battlefield().cols() == 32);
  CHECK(doc.starts() == std::vector<Position>{{0, 0}});
  CHECK(doc.targets() == std::vector<Position>{{31, 31}});
}
