#pragma once

#include <QAbstractTableModel>

#include <vector>

#include "rts/battlefield.hpp"

// Read-only grid model over a Battlefield, with start/target/path cells
// exposed as extra roles for the QML delegate to color. No editing support --
// this project's UI only displays and solves, it doesn't author maps.
class MapModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  enum Roles {
    ElevatedRole = Qt::UserRole + 1,
    IsStartRole,
    IsTargetRole,
    IsPathRole,
    IsUnitRole,
  };

  explicit MapModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  // `field` must outlive this model until the next setDocument()/clear()
  // call -- only a pointer is stored, not a copy.
  void setDocument(const rts::Battlefield& field, std::vector<rts::Position> starts,
                    std::vector<rts::Position> targets);
  void clear();

  void setPath(std::vector<rts::Position> path);
  void clearPath();

  // Where the units are standing at the tick currently being replayed.
  void setUnitPositions(std::vector<rts::Position> positions);

 private:
  const rts::Battlefield* mBattlefield = nullptr;
  std::vector<rts::Position> mStarts;
  std::vector<rts::Position> mTargets;
  std::vector<rts::Position> mPath;
  std::vector<rts::Position> mUnits;
};
