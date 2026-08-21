#include "mapmodel.hpp"

#include <algorithm>
#include <utility>

namespace {

bool contains(const std::vector<rts::Position>& positions, rts::Position pos) {
  return std::ranges::find(positions, pos) != positions.end();
}

}  // namespace

MapModel::MapModel(QObject* parent) : QAbstractTableModel(parent) {}

int MapModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid() || mBattlefield == nullptr) {
    return 0;
  }
  return mBattlefield->rows();
}

int MapModel::columnCount(const QModelIndex& parent) const {
  if (parent.isValid() || mBattlefield == nullptr) {
    return 0;
  }
  return mBattlefield->cols();
}

QVariant MapModel::data(const QModelIndex& index, int role) const {
  if (mBattlefield == nullptr || !index.isValid()) {
    return {};
  }

  const rts::Position pos{.row = index.row(), .col = index.column()};

  switch (role) {
    case ElevatedRole:
      return mBattlefield->terrainAt(pos) == rts::Terrain::Elevated;
    case IsStartRole:
      return contains(mStarts, pos);
    case IsTargetRole:
      return contains(mTargets, pos);
    case IsPathRole:
      return contains(mPath, pos);
    case IsUnitRole:
      return contains(mUnits, pos);
    default:
      return {};
  }
}

QHash<int, QByteArray> MapModel::roleNames() const {
  return {
      {ElevatedRole, "elevated"},
      {IsStartRole, "isStart"},
      {IsTargetRole, "isTarget"},
      {IsPathRole, "isPath"},
      {IsUnitRole, "isUnit"},
  };
}

void MapModel::setDocument(const rts::Battlefield& field, std::vector<rts::Position> starts,
                           std::vector<rts::Position> targets) {
  beginResetModel();
  mBattlefield = &field;
  mStarts = std::move(starts);
  mTargets = std::move(targets);
  mPath.clear();
  mUnits.clear();
  endResetModel();
}

void MapModel::clear() {
  beginResetModel();
  mBattlefield = nullptr;
  mStarts.clear();
  mTargets.clear();
  mPath.clear();
  mUnits.clear();
  endResetModel();
}

void MapModel::setPath(std::vector<rts::Position> path) {
  mPath = std::move(path);
  if (mBattlefield == nullptr) {
    return;
  }
  emit dataChanged(index(0, 0), index(mBattlefield->rows() - 1, mBattlefield->cols() - 1),
                   {IsPathRole});
}

void MapModel::clearPath() {
  setPath({});
}

void MapModel::setUnitPositions(std::vector<rts::Position> positions) {
  mUnits = std::move(positions);
  if (mBattlefield == nullptr) {
    return;
  }
  emit dataChanged(index(0, 0), index(mBattlefield->rows() - 1, mBattlefield->cols() - 1),
                   {IsUnitRole});
}
