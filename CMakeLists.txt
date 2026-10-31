# GHMetalRecomp - static recompilation of Guitar Hero: Metallica (Xbox 360), built on the ReXGlue SDK, drawn by the SDK's Xenos
# emulation or by NativeXRenderer (native_x_renderer/, a submodule).
#
# The SDK is the thirdparty/rexglue-sdk git submodule: ElGuslml/rexglue-sdk, branch recomp, the official
# nightly of 2026-10-02 (v0.10.0 and 24 later commits) with the changes these projects and NativeXRenderer
# need, one commit each (RECOMP.md there).
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(ghmetallica LANGUAGES C CXX)

# macOS: DiligentCore (NativeXRenderer) compiles Objective-C++ sources for its Apple platform layer, and a
# subproject cannot enable a language for its parent.
if(APPLE)
    enable_language(OBJC OBJCXX)
endif()

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

# The shell: the executable without the recompiled game, built without default.xex (no codegen). It starts, reads its
# settings, checks for the game files and says where they go, so that packaging (the macOS app and disk image,
# installer/macos/) and startup can be tried on a machine without the game. It cannot play: the real build needs
# assets/default.xex.
option(GHMETALLICA_SHELL "Build the executable without the recompiled game (no default.xex needed; cannot play)" OFF)

# Both graphics backends: D3D12 and Vulkan.
if(WIN32)
    set(REXGLUE_USE_D3D12 ON CACHE BOOL "Enable the D3D12 graphics backend")
endif()
set(REXGLUE_USE_VULKAN ON CACHE BOOL "Enable the Vulkan graphics backend")

include(generated/rexglue.cmake)
# The SDK's helpers find its files through REXGLUE_ROOT (macOS: MoltenVK_icd.json, staged next to the executable),
# which the SDK sets only in its own directory scope when it is built from source.
if(NOT DEFINED REXGLUE_ROOT)
    set(REXGLUE_ROOT "${REXSDK_DIR}")
endif()

# The first build of a fresh tree runs the codegen, which writes generated/default/sources.cmake;
# CMake has to configure again to pick up the generated sources (otherwise the link fails with
# PPCImageConfig). A marker file holds the sources.cmake this configure saw and is a configure
# dependency; it is refreshed with copy_if_different after the codegen, so it only changes (and
# triggers a new configure) when the list of generated sources changes. Ninja brings configure
# dependencies up to date before building anything else, so the first build runs the codegen,
# configures again and goes on within the same command. (The codegen stamp itself cannot be the
# dependency: the codegen touches it on every run.)
set(GHMETALLICA_CODEGEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated/default")
if(NOT GHMETALLICA_SHELL)
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
endif()

# Sources
set(GHMETALLICA_SOURCES
    src/framerate.cpp
    src/keyboard.cpp
    src/main.cpp
    src/native_x_renderer_ghmetallica.cpp
    src/options.cpp
    src/crowd.cpp
    src/fmod_mix.cpp
    src/players.cpp
    src/screen_fx.cpp
    src/quit_prompt.cpp
    src/startup.cpp
    src/thread_waits.cpp
    src/timer.cpp
    src/widescreen.cpp
)
if(GHMETALLICA_SHELL)
    # The game's hooks call the recompiled originals (__imp__sub_<address>), which only the codegen defines: in the
    # shell each is a stub that is never called (the shell stops before any game code runs).
    set(_shell_stub_names "")
    foreach(_source IN LISTS GHMETALLICA_SOURCES)
        file(READ "${_source}" _text)
        string(REGEX MATCHALL "__imp__sub_[0-9A-Fa-f]+" _imports "${_text}")
        string(REGEX MATCHALL "NATIVE_X_RENDERER_HOOK\\([0-9A-Fa-f]+" _renderer_hooks "${_text}")
        list(TRANSFORM _renderer_hooks REPLACE "NATIVE_X_RENDERER_HOOK\\(" "__imp__sub_")
        list(APPEND _shell_stub_names ${_imports} ${_renderer_hooks})
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_source}")
    endforeach()
    list(REMOVE_DUPLICATES _shell_stub_names)
    set(_shell_stubs "// Generated by CMakeLists.txt (GHMETALLICA_SHELL): the recompiled functions the hooks call.\n")
    string(APPEND _shell_stubs "#include <cstdlib>\n#include <rex/ppc/context.h>\n#include <rex/ppc/func.h>\n\n")
    foreach(_name IN LISTS _shell_stub_names)
        string(APPEND _shell_stubs "REX_EXTERN(${_name}) { std::abort(); }\n")
    endforeach()
    file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shell_stubs.cpp" CONTENT "${_shell_stubs}" @ONLY)
    list(APPEND GHMETALLICA_SOURCES src/shell_init.cpp src/shell_screen.cpp "${CMAKE_CURRENT_BINARY_DIR}/shell_stubs.cpp")
endif()

# macOS: the Vulkan driver the game draws with (MoltenVK, or KosmicKrisp when bundled and chosen).
if(APPLE)
    list(APPEND GHMETALLICA_SOURCES src/macos_vulkan_driver.cpp)
endif()

if(WIN32)
    add_executable(ghmetallica WIN32 ${GHMETALLICA_SOURCES})
else()
    add_executable(ghmetallica ${GHMETALLICA_SOURCES})
endif()

if(GHMETALLICA_SHELL)
    # rexglue_setup_target without the generated code and the codegen it depends on.
    target_include_directories(ghmetallica PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(ghmetallica PRIVATE rex::runtime)
    rexglue_configure_target(ghmetallica GPU_PLUGINS xenos)
    target_compile_definitions(ghmetallica PRIVATE GHMETALLICA_SHELL)
else()
    rexglue_setup_target(ghmetallica GPU_PLUGINS xenos)
    add_dependencies(ghmetallica ghmetallica_sources_marker)
endif()

# NativeXRenderer: the native renderer (native_x_renderer/README.md). The game side (hooks, profile) is in
# src/native_x_renderer_ghmetallica.cpp; the game draws on the Xenos path while native_x_renderer is off.
# On macOS the SDK builds MoltenVK, whose SPIRV-Cross (the spirv-cross-core target DiligentCore reuses instead of
# its own) lives in a renamed namespace. DiligentCore reads SPIRV_CROSS_NAMESPACE_OVERRIDE to match it.
if(TARGET spirv-cross-core AND NOT DEFINED SPIRV_CROSS_NAMESPACE_OVERRIDE)
    get_target_property(_spirv_cross_defs spirv-cross-core INTERFACE_COMPILE_DEFINITIONS)
    if(_spirv_cross_defs MATCHES "SPIRV_CROSS_NAMESPACE_OVERRIDE=([A-Za-z0-9_]+)")
        set(SPIRV_CROSS_NAMESPACE_OVERRIDE "${CMAKE_MATCH_1}")
    endif()
endif()
add_subdirectory(native_x_renderer)
target_link_libraries(ghmetallica PRIVATE native_x_renderer)
# NVIDIA's DLSS runtime next to the executable.
native_x_renderer_copy_runtime(ghmetallica)

# The target and the app keep the short name "ghmetallica" (ghmetallica.toml, logs\ghmetallica_NNN.log); only the
# executable gets a descriptive name.
set_target_properties(ghmetallica PROPERTIES OUTPUT_NAME "GHMetallicaRecomp")
if(APPLE)
    # glslang (and its SPIR-V code) is linked statically into both this executable (NativeXRenderer's SDK sources) and
    # the GPU plugin, librexgpu-xenos.dylib. dyld coalesces weak symbols (templates, inline functions) across images to
    # the main executable's, so the plugin's shader compiler ran this copy's templates against this copy's state, which
    # its thread never initialized: a fault on the GPU thread at the first shader. Not exported, they stay apart (as
    # each DLL keeps its own on Windows).
    target_link_options(ghmetallica PRIVATE
        "LINKER:-unexported_symbol,*glslang*" "LINKER:-unexported_symbol,*3spv*" "LINKER:-unexported_symbol,*spvutils*"
        "LINKER:-unexported_symbol,*spv_*")
    target_link_libraries(ghmetallica PRIVATE "-framework CoreFoundation")  # src/startup.cpp's alerts
endif()

# With the SDK built from source, rexruntime does not pass the Dear ImGui include directory on to the
# host target (the installed SDK ships imgui.h in include/); rex_app.cpp and ghmetallica_app.h need it. The
# symbols still come from rexruntime.dll.
target_include_directories(ghmetallica PRIVATE "${REXSDK_DIR}/thirdparty/imgui")

# Game files: assets/ next to GHMetallicaRecomp.exe (release layout) or, if absent, the repository's assets/.
target_compile_definitions(ghmetallica PRIVATE
    GHMETALLICA_DEFAULT_GAME_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets")

# GH Metallica Launcher: the options, the first run from the player's ISO, the achievements and the updates, in a separate
# executable so that GHMetallicaRecomp.exe starts right away (for example from Steam Big Picture); "Play" starts the game, and
# the launcher comes back when it closes. It writes ghmetallica_options.toml next to GHMetallicaRecomp.exe. The launcher's
# code is the RecompLauncher submodule (recomp_launcher/, the same in every game); the game's own names, files and
# options are launcher/game.h (with src/project.h, which the game shares) and launcher/game.cpp, its colors
# launcher/palette.h, its art launcher/art/, its resources res/launcher.rc. RECOMP_LAUNCHER_DIR points elsewhere to try
# a working copy of it.
set(RECOMP_LAUNCHER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/recomp_launcher" CACHE PATH "The RecompLauncher tree")
if(NOT EXISTS "${RECOMP_LAUNCHER_DIR}/RecompLauncher.cmake")
    message(FATAL_ERROR "The RecompLauncher submodule (${RECOMP_LAUNCHER_DIR}) is missing: git submodule update --init")
endif()
# Windows and macOS (RecompLauncher.cmake: Direct3D 11 there, Metal here).
if(WIN32 OR APPLE)
    include("${RECOMP_LAUNCHER_DIR}/RecompLauncher.cmake")
    recomp_launcher_add(ghmetallica_launcher
        OUTPUT_NAME "GH Metallica Launcher"
        GAME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/launcher"
        RC "${CMAKE_CURRENT_SOURCE_DIR}/res/launcher.rc"
        SDK_DIR "${REXSDK_DIR}"
        INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/res"
        DEFAULT_GAME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    # The game reads which of the launchers' profiles it runs with (src/startup.cpp, ConfigureProfile).
    recomp_launcher_add_profiles(ghmetallica SDK_DIR "${REXSDK_DIR}")
endif()

# Windows resources: file properties (res/version.h) and the icon, res/icon.ico, when it exists. The
# executable's first icon is also the one SDL and the launcher use for their windows.
if(WIN32)
    target_sources(ghmetallica PRIVATE res/game.rc)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/icon.ico")
        target_compile_definitions(ghmetallica PRIVATE RECOMP_HAS_ICON)
        target_compile_definitions(ghmetallica_launcher PRIVATE RECOMP_HAS_ICON)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/res/icon.ico")
    endif()
endif()
