#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

#include <optional>
#include <vector>

#include "mapmodel.hpp"
#include "rts/multi_unit.hpp"
#include "rts/pathfinder.hpp"
#include "rts/tilemap_json.hpp"

// The C++/QML bridge: owns the loaded map and the last result, both in
// memory. loadMap() parses a file via rts::TilemapDocument; solve() runs the
// path-finding directly and pushes the result into MapModel -- no file I/O
// happens as part of solving, only as part of loading.
//
// With several units the result is a tick-by-tick recording of the whole
// battlefield, which the UI replays a second at a time: currentTick walks
// from 0 to tickCount - 1.
class SolverController : public QObject {
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(MapModel* mapModel READ mapModel CONSTANT)
  Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
  Q_PROPERTY(QString pathText READ pathText NOTIFY pathTextChanged)
  Q_PROPERTY(bool hasMap READ hasMap NOTIFY hasMapChanged)
  Q_PROPERTY(bool hasSolution READ hasSolution NOTIFY hasSolutionChanged)
  Q_PROPERTY(int currentTick READ currentTick NOTIFY currentTickChanged)
  Q_PROPERTY(int tickCount READ tickCount NOTIFY hasSolutionChanged)
  Q_PROPERTY(bool isReplaying READ isReplaying NOTIFY isReplayingChanged)

 public:
  explicit SolverController(QObject* parent = nullptr);

  Q_INVOKABLE bool loadMap(const QUrl& fileUrl);
  Q_INVOKABLE bool solve();
  Q_INVOKABLE void advance();

  MapModel* mapModel() const { return mModel; }
  QString statusMessage() const { return mStatusMessage; }
  QString pathText() const { return mPathText; }
  bool hasMap() const { return mDocument.has_value(); }
  bool hasSolution() const { return mHasSolution; }
  int currentTick() const { return mCurrentTick; }
  int tickCount() const { return mPaths.empty() ? 0 : static_cast<int>(mPaths.front().size()); }
  bool isReplaying() const { return mIsReplaying; }

 signals:
  void statusMessageChanged();
  void pathTextChanged();
  void hasMapChanged();
  void hasSolutionChanged();
  void currentTickChanged();
  void isReplayingChanged();

 private:
  // One unit takes the shortest route; several have to be routed around each
  // other, which is a different problem (see rts::findPathsBfs).
  bool solveSingleUnit();
  bool solveManyUnits();
  bool reportFailure(const QString& reason);

  void showTick(int tick);
  void setStatusMessage(const QString& message);
  void setPathText(const QString& text);
  void setReplaying(bool replaying);

  MapModel* mModel;
  std::optional<rts::TilemapDocument> mDocument;
  std::vector<rts::Path> mPaths;  // one per unit, tick-indexed; empty unless replaying
  bool mHasSolution = false;
  int mCurrentTick = 0;
  bool mIsReplaying = false;
  QString mStatusMessage;
  QString mPathText;
};
