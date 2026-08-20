#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

#include <optional>

#include "mapmodel.hpp"
#include "rts/pathfinder.hpp"
#include "rts/tilemap_json.hpp"

// The C++/QML bridge: owns the loaded map and the last solve result, both
// in memory. loadMap() parses a file via rts::TilemapDocument; solve() calls
// rts::findPathBfs() directly and pushes the result into MapModel -- no file
// I/O happens as part of solving, only as part of loading.
class SolverController : public QObject {
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(MapModel* mapModel READ mapModel CONSTANT)
  Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
  Q_PROPERTY(QString pathText READ pathText NOTIFY pathTextChanged)
  Q_PROPERTY(bool hasMap READ hasMap NOTIFY hasMapChanged)
  Q_PROPERTY(bool hasSolution READ hasSolution NOTIFY hasSolutionChanged)

 public:
  explicit SolverController(QObject* parent = nullptr);

  Q_INVOKABLE bool loadMap(const QUrl& fileUrl);
  Q_INVOKABLE bool solve();

  MapModel* mapModel() const { return mModel; }
  QString statusMessage() const { return mStatusMessage; }
  QString pathText() const { return mPathText; }
  bool hasMap() const { return mDocument.has_value(); }
  bool hasSolution() const { return mSolution.has_value(); }

 signals:
  void statusMessageChanged();
  void pathTextChanged();
  void hasMapChanged();
  void hasSolutionChanged();

 private:
  void setStatusMessage(const QString& message);
  void setPathText(const QString& text);

  MapModel* mModel;
  std::optional<rts::TilemapDocument> mDocument;
  std::optional<rts::Path> mSolution;
  QString mStatusMessage;
  QString mPathText;
};
