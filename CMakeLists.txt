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
    engine/src/game/player.c
    engine/src/game/round.c
    engine/src/scene/instance.c
    engine/src/scene/instance_child_alias.c
    engine/src/scene/part.c
    engine/src/content/map_loader.c
    engine/src/render/draw.c
    engine/src/render/math.c
    engine/src/render/frustum.c
    engine/src/render/mesh.c
    engine/src/render/mesh_cache.c
    engine/src/render/texture.c
    engine/src/render/texture_cache.c
    engine/src/render/renderer.c
    engine/src/render/gles2.c
)

if(WIN32)
    list(APPEND ENGINE_CORE_SOURCES engine/src/render/gles2_backend.c)
    set(ENGINE_PLATFORM_SOURCES
        engine/src/platform/windows/platform_windows.c
    )
    set(ENGINE_PLATFORM_LIBS
        user32
        gdi32
        opengl32
    )
    set(ENGINE_PLATFORM_ENTRY_POINT
        platform/windows/main.c
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
    engine/src/render
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

add_executable(nds_map_loader_test engine/tests/map_loader_test.c)
target_link_libraries(nds_map_loader_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_map_loader_test COMMAND nds_map_loader_test)
set_tests_properties(nds_map_loader_test PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_executable(nds_draw_list_test engine/tests/draw_list_test.c)
target_link_libraries(nds_draw_list_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_draw_list_test COMMAND nds_draw_list_test)
set_tests_properties(nds_draw_list_test PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_executable(nds_render_math_test engine/tests/render_math_test.c)
target_link_libraries(nds_render_math_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_render_math_test COMMAND nds_render_math_test)

add_executable(nds_frustum_test engine/tests/frustum_test.c)
target_link_libraries(nds_frustum_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_frustum_test COMMAND nds_frustum_test)

add_executable(nds_mesh_test engine/tests/mesh_test.c)
target_link_libraries(nds_mesh_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_mesh_test COMMAND nds_mesh_test)
set_tests_properties(nds_mesh_test PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_executable(nds_mesh_cache_test engine/tests/mesh_cache_test.c)
target_link_libraries(nds_mesh_cache_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_mesh_cache_test COMMAND nds_mesh_cache_test)
set_tests_properties(nds_mesh_cache_test PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_executable(nds_texture_test engine/tests/texture_test.c)
target_link_libraries(nds_texture_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_texture_test COMMAND nds_texture_test)
set_tests_properties(nds_texture_test PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_executable(nds_texture_cache_test engine/tests/texture_cache_test.c)
target_link_libraries(nds_texture_cache_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_texture_cache_test COMMAND nds_texture_cache_test)
set_tests_properties(nds_texture_cache_test PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_executable(nds_round_test engine/tests/round_test.c)
target_link_libraries(nds_round_test PRIVATE engine_core ${ENGINE_PLATFORM_LIBS})
add_test(NAME nds_round_test COMMAND nds_round_test)
