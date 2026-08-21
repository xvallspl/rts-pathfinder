#include "rts/tilemap_json.hpp"

#include <cstddef>
#include <optional>
#include <utility>

#include "nlohmann/json.hpp"

namespace rts {

namespace {

struct Dimensions {
  int rows;
  int cols;
};

Dimensions resolveDimensions(const nlohmann::json& doc) {
  if (doc.contains("tileswide") && doc.contains("tileshigh")) {
    return {.rows = doc.at("tileshigh").get<int>(), .cols = doc.at("tileswide").get<int>()};
  }

  if (doc.contains("canvas") && doc.contains("tilesets") && doc.at("tilesets").is_array() &&
      !doc.at("tilesets").empty()) {
    const auto& canvas = doc.at("canvas");
    const auto& tileset = doc.at("tilesets").at(0);
    if (canvas.contains("width") && canvas.contains("height") && tileset.contains("tilewidth") &&
        tileset.contains("tileheight")) {
      const double tileWidth = tileset.at("tilewidth").get<double>();
      const double tileHeight = tileset.at("tileheight").get<double>();
      const double canvasWidth = canvas.at("width").get<double>();
      const double canvasHeight = canvas.at("height").get<double>();
      if (tileWidth > 0.0 && tileHeight > 0.0) {
        const double colsExact = canvasWidth / tileWidth;
        const double rowsExact = canvasHeight / tileHeight;
        const auto cols = static_cast<int>(colsExact);
        const auto rows = static_cast<int>(rowsExact);
        if (colsExact == static_cast<double>(cols) && rowsExact == static_cast<double>(rows)) {
          return {.rows = rows, .cols = cols};
        }
      }
    }
  }

  throw MapError(
      "cannot determine map dimensions: no tileswide/tileshigh, and canvas/tileset "
      "dimensions are missing or do not divide evenly");
}

enum class TileRole {
  Ground,
  Start,
  Target,
  Elevated,
};

// The task document defines exactly these four codes. Anything else --
// including a non-integer value -- is outside the documented format; the
// caller turns that into a MapError naming the offending cell.
std::optional<TileRole> classifyTileValue(double value) {
  if (value == -1.0) return TileRole::Ground;
  if (value == 0.0) return TileRole::Start;
  if (value == 8.0) return TileRole::Target;
  if (value == 3.0) return TileRole::Elevated;
  return std::nullopt;
}

std::string describePosition(Position pos) {
  return "(" + std::to_string(pos.row) + "," + std::to_string(pos.col) + ")";
}

}  // namespace

TilemapDocument::TilemapDocument(Battlefield battlefield, std::vector<Position> starts,
                                  std::vector<Position> targets)
    : mBattlefield(std::move(battlefield)),
      mStarts(std::move(starts)),
      mTargets(std::move(targets)) {}

TilemapDocument TilemapDocument::parse(std::string_view jsonText) {
  try {
    nlohmann::json doc = nlohmann::json::parse(jsonText);

    if (!doc.contains("layers") || !doc.at("layers").is_array() || doc.at("layers").empty()) {
      throw MapError("tilemap JSON has no layers[0]");
    }
    const auto& layer0 = doc.at("layers").at(0);
    if (!layer0.contains("data") || !layer0.at("data").is_array()) {
      throw MapError("layers[0] has no data array");
    }
    const auto& data = layer0.at("data");

    const Dimensions dims = resolveDimensions(doc);
    if (dims.rows <= 0 || dims.cols <= 0) {
      // Guards the size_t cast below: a negative dimension wraps around when
      // cast to unsigned, which can make an invalid data.size() look correct
      // (e.g. -2 x -2 wraps to 4, matching a genuine 4-entry array).
      throw MapError("map dimensions must be positive (got " + std::to_string(dims.rows) +
                      " x " + std::to_string(dims.cols) + ")");
    }
    const auto expectedCells =
        static_cast<std::size_t>(dims.rows) * static_cast<std::size_t>(dims.cols);
    if (data.size() != expectedCells) {
      throw MapError("layers[0].data has " + std::to_string(data.size()) +
                      " entries, expected " + std::to_string(dims.rows) + " x " +
                      std::to_string(dims.cols) + " = " + std::to_string(expectedCells));
    }

    std::vector<Terrain> cells(data.size());
    std::vector<Position> starts;
    std::vector<Position> targets;

    for (std::size_t i = 0; i < data.size(); ++i) {
      if (!data[i].is_number()) {
        throw MapError("layers[0].data[" + std::to_string(i) + "] is not a number");
      }
      const double value = data[i].get<double>();
      const Position pos{.row = static_cast<int>(i / static_cast<std::size_t>(dims.cols)),
                          .col = static_cast<int>(i % static_cast<std::size_t>(dims.cols))};

      const std::optional<TileRole> role = classifyTileValue(value);
      if (!role.has_value()) {
        throw MapError("layers[0].data[" + std::to_string(i) + "] at " + describePosition(pos) +
                        " has value " + std::to_string(value) +
                        ", which is not one of the documented codes -1, 0, 3, 8");
      }

      switch (*role) {
        case TileRole::Ground:
          cells[i] = Terrain::Ground;
          break;
        case TileRole::Start:
          cells[i] = Terrain::Ground;
          starts.push_back(pos);
          break;
        case TileRole::Target:
          cells[i] = Terrain::Ground;
          targets.push_back(pos);
          break;
        case TileRole::Elevated:
          cells[i] = Terrain::Elevated;
          break;
      }
    }

    if (starts.empty()) {
      throw MapError("tilemap has no starting position (no cell with document code 0)");
    }
    if (targets.empty()) {
      throw MapError("tilemap has no target position (no cell with document code 8)");
    }

    Battlefield battlefield(dims.rows, dims.cols, std::move(cells));
    return TilemapDocument(std::move(battlefield), std::move(starts), std::move(targets));

  } catch (const MapError&) {
    throw;  // already ours
  } catch (const nlohmann::json::exception& e) {
    throw MapError(std::string("invalid tilemap JSON: ") + e.what());
  }
}

}  // namespace rts
