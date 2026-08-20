#include "solvercontroller.hpp"

#include <QFile>
#include <QStringList>

SolverController::SolverController(QObject* parent)
    : QObject(parent), mModel(new MapModel(this)) {}

bool SolverController::loadMap(const QUrl& fileUrl) {
  QFile file(fileUrl.toLocalFile());
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setStatusMessage(tr("Could not open %1").arg(fileUrl.toLocalFile()));
    return false;
  }
  const QByteArray contents = file.readAll();

  mSolution.reset();
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
  setStatusMessage(tr("Loaded %1x%2 map: %3 start(s), %4 target(s).")
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
  if (mDocument->starts().size() != 1 || mDocument->targets().size() != 1) {
    setStatusMessage(tr("This build only supports a single start and target (found %1 "
                         "start(s), %2 target(s)); multi-unit is a stretch goal.")
                          .arg(static_cast<int>(mDocument->starts().size()))
                          .arg(static_cast<int>(mDocument->targets().size())));
    return false;
  }

  const std::optional<rts::Path> result = rts::findPathBfs(
      mDocument->battlefield(), mDocument->starts().front(), mDocument->targets().front());

  if (!result.has_value()) {
    mSolution.reset();
    mModel->clearPath();
    setStatusMessage(tr("No path exists between start and target."));
    setPathText(QString());
    emit hasSolutionChanged();
    return false;
  }

  mSolution = *result;
  mModel->setPath(*mSolution);

  QStringList coords;
  coords.reserve(static_cast<int>(mSolution->size()));
  for (const rts::Position& pos : *mSolution) {
    coords << QStringLiteral("(%1,%2)").arg(pos.row).arg(pos.col);
  }
  setPathText(tr("Path found (%1 steps): %2")
                  .arg(static_cast<int>(mSolution->size()) - 1)
                  .arg(coords.join(QStringLiteral(" → "))));
  setStatusMessage(tr("Solved."));
  emit hasSolutionChanged();
  return true;
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
