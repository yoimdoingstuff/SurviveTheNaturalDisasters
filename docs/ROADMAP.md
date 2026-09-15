# ROADMAP.md

## Milestone 0 - Cross-platform foundation

Platform abstraction is defined first. A minimal native build must be possible on Windows, Android 4.0+, and iOS 8 before platform-specific features start multiplying.

## Milestone 1 - It boots

A native executable starts, initializes the platform layer, creates an OpenGL ES context, draws a scene, handles touch input, and shuts down cleanly.

## Milestone 2 - It behaves like a tiny game engine

Instance tree, camera, parts, physics, characters, audio, asset loading, and a script runtime work together.

## Milestone 3 - NDS gameplay prototype

One map, one disaster, one player, one complete round.

## Milestone 4 - Base game

Multiple maps/disasters, round flow, survival logic, HUD, audio, and save/config support.

## Milestone 5 - Menu + lobby

Main menu, offline play, local lobby creation, joining, lobby browser, and server settings. This milestone is mandatory before post-release feature work.

## Milestone 6 - Local multiplayer

Host/client synchronization over local networking, with the same gameplay simulation APIs used by offline mode.

## Milestone 7 - Cross-platform validation

Windows and Android 4.0+ builds must reach feature parity with the shared runtime and successfully load the same game content.

## Milestone 8 - iPad 4 optimization

Memory, thermal, rendering, loading, and touch UX optimized around A6X constraints.

## Milestone 9 - iOS 6 investigation/build

Only after iOS 8 is stable. Port platform layer downward where feasible.

## Milestone 10 - Base release lock

Feature freeze, bug triage, reproducible builds, documentation, and a stable base runtime release.

## Milestone 11 - Update track

Add cosmetics, unlockables, gear, extra settings, progression, and additional content without destabilizing the base release.

## Milestone 12 - Mod system

After the base release is stable, add versioned local mod packages, content registration, sandboxed scripts, dependency handling, and multiplayer compatibility checks.
