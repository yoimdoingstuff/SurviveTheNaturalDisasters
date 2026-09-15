# Patch notes - Phase 0/1 bootstrap (Windows first target)

I don't have push access to the GitHub repo from here, so this is a file
drop: unzip it over your local clone (paths match the repo layout) and
commit it yourself.

## What this adds

Working through TODO.md in order, starting from the first genuinely
unblocked, code-shaped items:

- **`engine/include/engine/platform/platform.h`** - the shared platform
  API (Phase 0: "Define shared platform API for graphics, input, files,
  audio, sockets, timing, lifecycle"). Window/input/files/timing are
  fully implemented for Windows; audio and sockets have their interface
  defined but intentionally return `NDS_ERR_NOT_IMPLEMENTED` - those
  backends are Phase 6 and Phase 9 work respectively, not this pass.
- **`engine/src/platform/windows/platform_windows.c`** - the Windows
  backend: Win32 window, keyboard/mouse input, `QueryPerformanceCounter`
  timing, stdio-based file I/O.
- **`engine/src/core/{log,config,clock,memstat,app}.c`** - Phase 1's
  "early bootstrap" items: logging, a small INI-style config system, a
  frame clock, allocation/frame-time tracking, and `nds_app_run()` which
  wires all of it into an actual boot -> main loop -> shutdown sequence.
  These files are platform-independent - they only ever touch the OS
  through `platform.h`, never `<windows.h>` directly, per AGENTS.md.
- **`platform/windows/main.c`** - the Windows dev-build entry point. Plain
  console-subsystem `main()` on purpose right now, so log output stays
  visible next to the window during early development; can be swapped
  for a `WinMain` at release time without touching engine code.
- **`CMakeLists.txt`** - builds `nds_runtime.exe` on Windows. Fails loudly
  with a clear message on any other target, since no other backend
  exists yet.
- **`.github/workflows/windows-build.yml`** - CI: configures + builds with
  MSVC on `windows-latest`, then actually *runs* the exe with
  `--smoke-test 60` (60 frames, then auto-exit) so a green check means
  the runtime booted, not just that it compiled.
- **`TODO.md`** - updated with this pass's items checked off (see below).

## What I verified here

I don't have a Windows toolchain in this sandbox, so I couldn't compile
`platform_windows.c` or `main.c` directly. What I did do:

- Compiled every platform-independent file (`log.c`, `config.c`,
  `clock.c`, `memstat.c`, `app.c`) with `gcc -std=c11 -Wall -Wextra` -
  clean, no warnings.
- Wrote a throwaway Linux mock of `platform.h` and linked/ran the real
  `app.c` against it end-to-end: window "creation", config file loading
  (including comments, whitespace, and overriding `window.width`/
  `window.height`), the main loop, smoke-test auto-exit, and clean
  shutdown all behaved correctly.
- That validates the shared logic and the platform.h contract itself.
  The Win32-specific code (`platform_windows.c`) is written carefully
  against standard, well-worn Win32 patterns, but its first real compile
  will happen in your CI run - watch the first Actions run for anything
  I couldn't catch from here.

One thing to watch specifically: GitHub's `windows-latest` runners do
have an interactive desktop session, so `CreateWindow`/GDI calls are
expected to work in Actions - but if the smoke-test step ever fails
*only* in CI while the same build runs fine on your machine, that's the
first thing to check.

## How to build locally

```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
build\Release\nds_runtime.exe
```

Pass `--smoke-test [frames]` to run a fixed number of frames and exit
automatically (used by CI); otherwise it runs normally until you close
the window or press Escape.

## TODO.md items this closes out

Phase 0:
- [x] Define shared platform API for graphics, input, files, audio, sockets, timing, lifecycle
- [x] Establish Windows development build
- [x] Create basic CI/build scripts

Phase 1 (early bootstrap):
- [x] Application bootstrap
- [x] Platform abstraction layer *(Windows backend only - other platforms still open)*
- [x] File system abstraction
- [x] Timing/clock system
- [x] Main loop
- [~] Logging/debug console *(logging done; interactive debug console UI still open)*
- [x] Configuration system
- [x] Basic memory/performance instrumentation

## Suggested next step

Per the project's own "Development order" in `README.md`, the next
unblocked item is the **early NDS map/content importer** (Phase 1's
remaining bullets: import workspace, map/package discovery tool, initial
Roblox place/model importer, converting imported maps into the
project-owned format). That's a separate, fairly large chunk of work
(file-format parsing) - happy to start on it next if you want to keep
going in TODO order.
