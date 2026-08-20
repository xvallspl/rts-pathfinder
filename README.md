# RTS Battle Unit Path-Finding

Software Candidate Assessment take-home project: path-finding for battle units on a
grid-based battlefield, with a C++17 core and a Qt Quick (QML) user interface.

Status: **work in progress.** This README grows alongside the code — each section below
is filled in as the corresponding piece is built, not written up front. See
[Design Decisions](#design-decisions) for a running log of choices and the reasoning
behind them, kept honest by writing each entry at the moment the decision is made.

## Table of Contents

- [Project Structure](#project-structure)
- [Build Instructions](#build-instructions)
- [Design Decisions](#design-decisions)
- [Third-Party Libraries](#third-party-libraries)
- [Sample Runs](#sample-runs)
- [AI Usage](#ai-usage)
- [Feedback on the Assessment](#feedback-on-the-assessment)

## Project Structure

*(filled in as each part lands)*

## Build Instructions

*(filled in once there is something to build)*

## Design Decisions

### C++23, not C++17

The task document requires "C++17 or later"; this project targets **C++23**. CI is pinned to toolchains recent enough to support it.

### Compiler warnings: on, not fatal

`-Wall -Wextra` / `/W4` are enabled but not treated as errors, so a newer reviewer
toolchain surfacing an unseen warning can't break the build.

### Tile values: exactly the four documented codes, nothing else

The task document defines exactly four values: `-1` ground, `0` start, `8` target, `3`
elevated. A value outside that set — including a non-integer one — is not a variant of a
documented code, it's outside the format; the parser throws `MapError` naming the
offending cell rather than guessing at undocumented meaning.

The interviewer's own sample map doesn't conform to this: it has no plain `8`, and
carries `0.5`/`8.1` instead of documented codes. Rather than build tolerance into the
parser to make that one file happen to parse, `samples/interviewer_sample.json` is kept
as a negative test — proof the parser rejects it cleanly, with a clear, specific error —
and a separate, spec-conformant `samples/sample_map.json` (generated, guaranteed
solvable) is used everywhere a positive example is needed.

### Map dimensions: derived, not assumed

The sample JSON has no `tileswide`/`tileshigh` fields (unlike typical RiskyLab exports).
Dimensions are resolved as: explicit `tileswide`/`tileshigh` if present, else
`canvas.width / tilesets[0].tilewidth` (and the equivalent for height), else a `MapError`.

## Third-Party Libraries

- **nlohmann/json 3.11.3** (MIT) — JSON parsing for the RiskyLab tilemap format.
  Vendored as a single header in `third_party/nlohmann/`. A hand-rolled parser was
  considered, since our schema needs are narrow; rejected because JSON's grammar
  (escaping, number formats) is easy to get subtly wrong, and export must preserve
  unknown fields (tileset name, image path, ...) byte-faithfully for the file to stay
  RiskyLab-compatible.
- **doctest 2.4.11** (MIT) — unit testing. Vendored as a single header in
  `third_party/doctest/`. Chosen over Catch2/GoogleTest for compile speed and
  zero-dependency, single-header integration.

## AI Usage

This project was built with Claude Code as a pair-programmer, under direct supervision:
every design decision was discussed and argued before being adopted, every code change
was reviewed and editable before being committed. That review caught and reversed a real
overreach — an initial tile-value rule was built by importing test maps into the live
RiskyLab editor and inferring undocumented icon-set behavior from what rendered, so it
would happen to parse the interviewer's sample. It was rejected in favor of the simpler,
literal reading in [Design Decisions](#design-decisions): implement exactly what the task
document specifies, and treat non-conforming input as an error rather than something to
be cleverly decoded. Full detail on what was AI-assisted versus author-driven is added
here as the project progresses.

## Feedback on the Assessment

*(added at the end, per the task document's feedback section)*
