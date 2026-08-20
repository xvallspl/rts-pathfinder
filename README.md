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

The task document requires "C++17 or later"; this project targets **C++23** (`std::expected`,
`std::ranges`). CI is pinned to toolchains recent enough to support it.

### Compiler warnings: on, not fatal

`-Wall -Wextra` / `/W4` are enabled but not treated as errors, so a newer reviewer
toolchain surfacing an unseen warning can't break the build.


## AI Usage

This project was built with Claude Code as a pair-programming tool, under direct
supervision: every design decision was discussed and argued before being adopted (see
[Design Decisions](#design-decisions)), every code change was reviewed and could be
edited before being committed, and the assumptions behind the sample JSON's tile encoding
were verified empirically (by importing test maps into the live RiskyLab Tilemap editor)
rather than taken on faith from either the AI or the task document alone. Full detail on
what was AI-assisted versus author-driven is added here as the project progresses.

## Feedback on the Assessment

*(added at the end, per the task document's feedback section)*
