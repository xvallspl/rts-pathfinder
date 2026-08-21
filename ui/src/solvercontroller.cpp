#include "solvercontroller.hpp"

#include <QFile>
#include <QStringList>
#include <iterator>
#include <utility>

namespace {

QString describe(const rts::Path& path) {
  QStringList coords;
  coords.reserve(static_cast<int>(path.size()));
  for (const rts::Position& pos : path) {
    coords << QStringLiteral("(%1,%2)").arg(pos.row).arg(pos.col);
  }
  return coords.join(QStringLiteral(" → "));
}

}  // namespace

SolverController::SolverController(QObject* parent) : QObject(parent), mModel(new MapModel(this)) {}

bool SolverController::loadMap(const QUrl& fileUrl) {
  QFile file(fileUrl.toLocalFile());
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setStatusMessage(tr("Could not open %1").arg(fileUrl.toLocalFile()));
    return false;
  }
  const QByteArray contents = file.readAll();

  mHasSolution = false;
  mPaths.clear();
  setReplaying(false);
  setPathText(QString());

  try {
    mDocument = rts::TilemapDocument::parse(contents.toStdString());
  } catch (const rts::MapError& e) {
    mDocument.reset();
    mModel->clear();
    setStatusMessage(tr("Failed to load map: %1").arg(QString::fromUtf8(e.what())));
    emit hasMapChanged();
    emit hasSolutionChanged();
    return false;
  }

  mModel->setDocument(mDocument->battlefield(), mDocument->starts(), mDocument->targets());
  setStatusMessage(tr("Loaded %1x%2 map: %3 unit(s), %4 target(s).")
                       .arg(mDocument->battlefield().rows())
                       .arg(mDocument->battlefield().cols())
                       .arg(static_cast<int>(mDocument->starts().size()))
                       .arg(static_cast<int>(mDocument->targets().size())));
  emit hasMapChanged();
  emit hasSolutionChanged();
  return true;
}

bool SolverController::solve() {
  if (!mDocument.has_value()) {
    setStatusMessage(tr("Load a map first."));
    return false;
  }
  return mDocument->starts().size() == 1 ? solveSingleUnit() : solveManyUnits();
}

bool SolverController::solveSingleUnit() {
  if (mDocument->targets().size() != 1) {
    return reportFailure(tr("One unit, but %1 targets — expected exactly one.")
                             .arg(static_cast<int>(mDocument->targets().size())));
  }

  const std::optional<rts::Path> path = rts::findPathBfs(
      mDocument->battlefield(), mDocument->starts().front(), mDocument->targets().front());
  if (!path.has_value()) {
    return reportFailure(tr("No path exists between start and target."));
  }

  mPaths = {*path};
  mHasSolution = true;
  setPathText(
      tr("Path found (%1 steps): %2").arg(static_cast<int>(path->size()) - 1).arg(describe(*path)));
  setStatusMessage(tr("Solved."));

  emit hasSolutionChanged();
  showTick(0);
  setReplaying(true);
  return true;
}

bool SolverController::solveManyUnits() {
  std::optional<std::vector<rts::Path>> paths =
      rts::findPathsBfs(mDocument->battlefield(), mDocument->starts(), mDocument->targets());
  if (!paths.has_value()) {
    return reportFailure(tr("No plan found that moves %1 units to %2 target(s) without "
                            "collisions.")
                             .arg(static_cast<int>(mDocument->starts().size()))
                             .arg(static_cast<int>(mDocument->targets().size())));
  }

  mPaths = std::move(*paths);
  mHasSolution = true;

  QStringList lines;
  for (std::size_t unit = 0; unit < mPaths.size(); ++unit) {
    lines << tr("Unit %1: %2").arg(unit + 1).arg(describe(mPaths[unit]));
  }
  setPathText(tr("%1 units over %2 ticks (a repeated cell is a unit holding position):\n%3")
                  .arg(static_cast<int>(mPaths.size()))
                  .arg(tickCount() - 1)
                  .arg(lines.join(QStringLiteral("\n"))));
  setStatusMessage(tr("Solved."));

  emit hasSolutionChanged();
  showTick(0);
  setReplaying(true);
  return true;
}

void SolverController::advance() {
  if (mCurrentTick + 1 >= tickCount()) {
    setReplaying(false);
    return;
  }
  showTick(mCurrentTick + 1);
}

void SolverController::showTick(int tick) {
  mCurrentTick = tick;
  const auto reached = static_cast<std::size_t>(tick);

  std::vector<rts::Position> units;
  std::vector<rts::Position> trail;
  units.reserve(mPaths.size());
  for (const rts::Path& path : mPaths) {
    units.push_back(path[reached]);
    trail.insert(trail.end(), path.begin(),
                 std::next(path.begin(), static_cast<std::ptrdiff_t>(reached) + 1));
  }
  mModel->setPath(std::move(trail));
  mModel->setUnitPositions(std::move(units));

  emit currentTickChanged();
}

bool SolverController::reportFailure(const QString& reason) {
  mHasSolution = false;
  mPaths.clear();
  setReplaying(false);
  mModel->clearPath();
  mModel->setUnitPositions({});
  setPathText(QString());
  setStatusMessage(reason);
  emit hasSolutionChanged();
  return false;
}

void SolverController::setStatusMessage(const QString& message) {
  if (mStatusMessage == message) {
    return;
  }
  mStatusMessage = message;
  emit statusMessageChanged();
}

void SolverController::setPathText(const QString& text) {
  if (mPathText == text) {
    return;
  }
  mPathText = text;
  emit pathTextChanged();
}

void SolverController::setReplaying(bool replaying) {
  if (mIsReplaying == replaying) {
    return;
  }
  mIsReplaying = replaying;
  emit isReplayingChanged();
}
