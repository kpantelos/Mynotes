# My Notes

Native Windows desktop notes application built with C++ and Qt Widgets.

## Current source baseline

This repository contains the source files imported from the supplied ZIP archive. The current baseline includes note creation, editing, deletion, favorites, text search, and a local SQLite database.

The ZIP did **not** contain the described persistent folders/subfolders, rich-text editing, attachment management, or their database migrations. Those features are not claimed as implemented in this baseline.

## Build environment

- Visual Studio 2026 (MSVC v145 project toolset)
- Qt 6.12.1 for MSVC 2022 64-bit (`6.12.1_msvc2022_64`)
- Qt modules: Core, Gui, Widgets, Sql
- Platform: x64

Open `MyNotes.slnx` in Visual Studio. Ensure the Qt Visual Studio Tools extension is installed and the Qt version named above is configured in Qt VS Tools.

## Data location

The application creates/uses `MyNotes.db` under `QStandardPaths::AppDataLocation`. Do not delete or overwrite this directory when updating the application. The database is deliberately not tracked in Git.

## Repository hygiene

Build output, Visual Studio user settings/caches, local databases, and user attachment data must not be committed.
