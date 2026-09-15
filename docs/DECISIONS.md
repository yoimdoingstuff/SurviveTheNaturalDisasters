# DECISIONS.md

## D001 - Offline-first

Core gameplay must never require internet access.

## D002 - Menu/lobby are base features

A menu screen and local lobby creation are required before the post-launch update phase.

## D003 - Local networking is authoritative-state based

One device hosts the simulation; peers render their own views locally.

## D004 - Legacy graphics baseline

OpenGL ES 2 is the baseline renderer target for the iPad 4 path.

## D005 - Small compatibility API

Implement only the game-facing API actually needed instead of recreating the entire modern Roblox platform.

## D006 - Update systems are modular

Cosmetics, unlockables, gear, and expanded settings must not become prerequisites for base gameplay.

## D007 - Mostly cross-platform architecture

Windows, Android 4.0+, and iOS are first-class targets. Shared C/C++ engine/game code is mandatory; platform APIs are hidden behind small adapters.

## D008 - Legacy graphics baseline

OpenGL ES 2 is the common mobile graphics baseline. Windows uses an OpenGL 2.1-class baseline initially so the common renderer remains compatible with the legacy target set.

## D009 - Android minimum

Android 4.0 / API 14 is the minimum Android target. Newer Android APIs may be used only through compatibility wrappers or optional paths.
