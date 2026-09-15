cmake_minimum_required(VERSION 3.16)
project(NaturalDisasterSurvivalRuntime C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# -----------------------------------------------------------------------
# Engine core - platform-independent. Must never include OS headers;
# everything OS-specific goes through engine/include/engine/platform/platform.h.
# -----------------------------------------------------------------------
set(ENGINE_CORE_SOURCES
    engine/src/core/log.c
    engine/src/core/config.c
    engine/src/core/clock.c
    engine/src/core/memstat.c
    engine/src/core/app.c
)

# -----------------------------------------------------------------------
# Platform backend selection.
#
# Windows is the only implemented backend so far (TODO.md Phase 0/1,
# chosen as the first buildable target for local development). Future
# platforms (iOS, Android, other desktop) add their own
# engine/src/platform/<name>/ sources and entry point, selected here the
# same way, without touching engine_core.
# -----------------------------------------------------------------------
if(WIN32)
    set(ENGINE_PLATFORM_SOURCES
        engine/src/platform/windows/platform_windows.c
    )
    set(ENGINE_PLATFORM_ENTRY_POINT
        platform/windows/main.c
    )
    set(ENGINE_PLATFORM_LIBS
        user32
        gdi32
    )
else()
    message(FATAL_ERROR
        "No platform backend implemented for this target yet - Windows is "
        "currently the only supported build. See TODO.md Phase 0/1.")
endif()

add_library(engine_core STATIC
    ${ENGINE_CORE_SOURCES}
    ${ENGINE_PLATFORM_SOURCES}
)

target_include_directories(engine_core PUBLIC
    engine/include
)

if(MSVC)
    target_compile_options(engine_core PRIVATE /W4)
else()
    target_compile_options(engine_core PRIVATE -Wall -Wextra)
endif()

add_executable(nds_runtime
    ${ENGINE_PLATFORM_ENTRY_POINT}
)

target_link_libraries(nds_runtime PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
