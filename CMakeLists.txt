cmake_minimum_required(VERSION 3.16)
project(NaturalDisasterSurvivalRuntime C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

enable_testing()

set(ENGINE_CORE_SOURCES
    engine/src/core/log.c
    engine/src/core/config.c
    engine/src/core/clock.c
    engine/src/core/memstat.c
    engine/src/core/app.c
    engine/src/scene/instance.c
    engine/src/scene/part.c
    engine/src/content/map_loader.c
    engine/src/render/draw.c
    engine/src/render/renderer.c
)

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
    target_compile_options(engine_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(nds_runtime
    ${ENGINE_PLATFORM_ENTRY_POINT}
)

target_link_libraries(nds_runtime PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})

add_executable(nds_map_loader_test
    engine/tests/map_loader_test.c
)
target_link_libraries(nds_map_loader_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_map_loader_test COMMAND nds_map_loader_test)

add_executable(nds_draw_list_test
    engine/tests/draw_list_test.c
)
target_link_libraries(nds_draw_list_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_draw_list_test COMMAND nds_draw_list_test)
