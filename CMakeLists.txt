# GHMetalRecomp - static recompilation of Guitar Hero: Metallica (Xbox 360), built on the ReXGlue SDK, drawn by the SDK's Xenos
# emulation (and, once integrated, by NativeXRenderer: native_x_renderer/, a submodule).
#
# The SDK is the thirdparty/rexglue-sdk git submodule: ElGuslml/rexglue-sdk, branch recomp, the official
# nightly of 2026-10-02 (v0.10.0 and 24 later commits) with the changes these projects and NativeXRenderer
# need, one commit each (RECOMP.md there).
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(ghmetallica LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Same CPU baseline as the official SDK builds (its win-amd64 preset): x86-64-v2, i.e. SSE4.2 and
# POPCNT on top of the SSSE3/SSE4.1 the SDK code requires. Not -march=native, so the binaries run on
# any x86-64-v2 CPU.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    add_compile_options(-march=x86-64-v2)
endif()

# Build the SDK from the submodule. RECOMP.md marks the recomp branch: a checkout of the official SDK lacks
# its changes and would not build this project.
set(REXSDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rexglue-sdk" CACHE PATH
    "Path to the rexglue-sdk source tree" FORCE)
if(NOT EXISTS "${REXSDK_DIR}/RECOMP.md")
    message(FATAL_ERROR
        "The ReXGlue SDK submodule (${REXSDK_DIR}) is missing or not on the recomp branch of "
        "ElGuslml/rexglue-sdk. Run: git submodule sync && git submodule update --init --recursive --force")
endif()

# Both graphics backends: D3D12 and Vulkan.
if(WIN32)
    set(REXGLUE_USE_D3D12 ON CACHE BOOL "Enable the D3D12 graphics backend")
endif()
set(REXGLUE_USE_VULKAN ON CACHE BOOL "Enable the Vulkan graphics backend")

include(generated/rexglue.cmake)

# The first build of a fresh tree runs the codegen, which writes generated/default/sources.cmake;
# CMake has to configure again to pick up the generated sources (otherwise the link fails with
# PPCImageConfig). A marker file holds the sources.cmake this configure saw and is a configure
# dependency; it is refreshed with copy_if_different after the codegen, so it only changes (and
# triggers a new configure) when the list of generated sources changes. Ninja brings configure
# dependencies up to date before building anything else, so the first build runs the codegen,
# configures again and goes on within the same command. (The codegen stamp itself cannot be the
# dependency: the codegen touches it on every run.)
set(GHMETALLICA_CODEGEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated/default")
set(GHMETALLICA_SOURCES_MARKER "${GHMETALLICA_CODEGEN_DIR}/sources.cmake.configured")
if(EXISTS "${GHMETALLICA_CODEGEN_DIR}/sources.cmake")
    file(COPY_FILE "${GHMETALLICA_CODEGEN_DIR}/sources.cmake" "${GHMETALLICA_SOURCES_MARKER}" ONLY_IF_DIFFERENT)
elseif(NOT EXISTS "${GHMETALLICA_SOURCES_MARKER}")
    file(WRITE "${GHMETALLICA_SOURCES_MARKER}" "")
endif()
add_custom_command(
    OUTPUT "${GHMETALLICA_SOURCES_MARKER}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${GHMETALLICA_CODEGEN_DIR}/sources.cmake" "${GHMETALLICA_SOURCES_MARKER}"
    DEPENDS "${GHMETALLICA_CODEGEN_DIR}/codegen.build.stamp"
    COMMENT "Checking the list of generated sources"
    VERBATIM)
add_custom_target(ghmetallica_sources_marker DEPENDS "${GHMETALLICA_SOURCES_MARKER}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${GHMETALLICA_SOURCES_MARKER}")

# Sources
set(GHMETALLICA_SOURCES
    src/main.cpp
    src/options.cpp
    src/startup.cpp
    src/thread_waits.cpp
    src/timer.cpp
)

if(WIN32)
    add_executable(ghmetallica WIN32 ${GHMETALLICA_SOURCES})
else()
    add_executable(ghmetallica ${GHMETALLICA_SOURCES})
endif()

rexglue_setup_target(ghmetallica GPU_PLUGINS xenos)
add_dependencies(ghmetallica ghmetallica_sources_marker)

# NativeXRenderer: the native renderer (native_x_renderer/README.md), built and linked from the start. Nothing
# references it until the game side exists (src/native_x_renderer_ghmetallica.cpp: hooks and profile; the library's
# docs/integrating-a-game.md), so the game draws on the Xenos path until then.
add_subdirectory(native_x_renderer)
target_link_libraries(ghmetallica PRIVATE native_x_renderer)
# NVIDIA's DLSS runtime next to the executable.
native_x_renderer_copy_runtime(ghmetallica)

# The target and the app keep the short name "ghmetallica" (ghmetallica.toml, logs\ghmetallica_NNN.log); only the
# executable gets a descriptive name.
set_target_properties(ghmetallica PROPERTIES OUTPUT_NAME "GHMetallicaRecomp")

# With the SDK built from source, rexruntime does not pass the Dear ImGui include directory on to the
# host target (the installed SDK ships imgui.h in include/); rex_app.cpp and ghmetallica_app.h need it. The
# symbols still come from rexruntime.dll.
target_include_directories(ghmetallica PRIVATE "${REXSDK_DIR}/thirdparty/imgui")

# Game files: assets/ next to GHMetallicaRecomp.exe (release layout) or, if absent, the repository's assets/.
target_compile_definitions(ghmetallica PRIVATE
    GHMETALLICA_DEFAULT_GAME_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets")

# GH Metallica Launcher: the options, the first run from the player's ISO, the achievements and the updates, in a separate
# executable so that GHMetallicaRecomp.exe starts right away (for example from Steam Big Picture); "Play" starts the game, and
# the launcher comes back when it closes. It writes ghmetallica_options.toml next to GHMetallicaRecomp.exe. The launcher is
# POTCAWERecomp's, shared through recomp-tools' template: the game's own names, files and options are
# launcher/game.h (with src/project.h, which the game shares) and launcher/game.cpp, its art launcher/art/. SDL3
# and Dear ImGui from the SDK tree, linked statically (one executable, without the SDK's DLLs), drawn with
# Direct3D 11 (the SDK builds SDL3 without SDL_render); nanosvg draws its icons and art.
if(WIN32)
    add_executable(ghmetallica_launcher WIN32
        launcher/main.cpp
        launcher/app.cpp
        launcher/game.cpp
        launcher/settings.cpp
        launcher/texts.cpp
        launcher/page_play.cpp
        launcher/page_settings.cpp
        launcher/page_game.cpp
        launcher/page_keyboard.cpp
        launcher/page_gamepad.cpp
        launcher/page_updates.cpp
        launcher/page_achievements.cpp
        launcher/page_install.cpp
        launcher/iso.cpp
        launcher/install.cpp
        launcher/achievements.cpp
        launcher/net.cpp
        launcher/updater.cpp
        launcher/zip.cpp
        launcher/ui/gfx.cpp
        launcher/ui/ui.cpp
        "${REXSDK_DIR}/thirdparty/imgui/backends/imgui_impl_sdl3.cpp"
        "${REXSDK_DIR}/thirdparty/imgui/backends/imgui_impl_dx11.cpp")
    set_target_properties(ghmetallica_launcher PROPERTIES OUTPUT_NAME "GH Metallica Launcher")
    target_compile_definitions(ghmetallica_launcher PRIVATE UNICODE _UNICODE NOMINMAX IMGUI_DEFINE_MATH_OPERATORS)
    target_include_directories(ghmetallica_launcher PRIVATE launcher src res launcher/thirdparty/nanosvg
        "${REXSDK_DIR}/thirdparty/imgui/backends" "${REXSDK_DIR}/thirdparty/tomlplusplus"
        "${REXSDK_DIR}/thirdparty/inja/third_party/include" "${REXSDK_DIR}/thirdparty/stb")
    target_link_libraries(ghmetallica_launcher PRIVATE imgui SDL3::SDL3 d3d11 dxgi d3dcompiler shcore windowscodecs ole32
        winhttp bcrypt)
    # A development build's game files, as the game falls back to them: the launcher then skips the first run.
    target_compile_definitions(ghmetallica_launcher PRIVATE
        LAUNCHER_DEFAULT_GAME_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets")
    # The embedded fonts and art (res/launcher.rc): a change rebuilds the resources.
    file(GLOB GHMETALLICA_LAUNCHER_RESOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/launcher/fonts/*.ttf" "${CMAKE_CURRENT_SOURCE_DIR}/launcher/art/*.svg")
    set_source_files_properties(res/launcher.rc PROPERTIES OBJECT_DEPENDS "${GHMETALLICA_LAUNCHER_RESOURCES}")
endif()

# Windows resources: file properties (res/version.h) and the icon, res/icon.ico, when it exists. The
# executable's first icon is also the one SDL and the launcher use for their windows.
if(WIN32)
    target_sources(ghmetallica PRIVATE res/game.rc)
    target_sources(ghmetallica_launcher PRIVATE res/launcher.rc)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/icon.ico")
        target_compile_definitions(ghmetallica PRIVATE RECOMP_HAS_ICON)
        target_compile_definitions(ghmetallica_launcher PRIVATE RECOMP_HAS_ICON)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/res/icon.ico")
    endif()
endif()
