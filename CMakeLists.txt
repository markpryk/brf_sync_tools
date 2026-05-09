# CMakeLists.txt � brf_sync standalone (Qt-free)
# Copyright (C) 2026 markpryk
# Licensed under the GNU General Public License v2.0
# Build: cmake -B build_standalone -G "Visual Studio 17 2022" && cmake --build build_standalone --config Release
cmake_minimum_required(VERSION 3.16)
project(brf_sync VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === Paths ===
set(VCGLIB "${CMAKE_SOURCE_DIR}/dependencies/vcglib")

# === Defines ===
add_definitions(-DBRF_NO_GUI)       # Existing guard in vcgmesh.cpp (skips QOpenGLWidget)
add_definitions(-DBRF_NO_QT)        # Our new guard for brfData.h Qt includes
add_definitions(-DNOMINMAX)
add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

# === Include paths ===
include_directories("${VCGLIB}")
include_directories("${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/dependencies")  # for nlohmann/json.hpp

# === Sources ===
# Core BRF logic (Qt-free already)
set(CORE_SOURCES
    brfMesh.cpp
    brfData.cpp
    brfShader.cpp
    brfTexture.cpp
    brfMaterial.cpp
    brfSkeleton.cpp
    brfAnimation.cpp
    brfBody.cpp
    brfHitBox_stub.cpp
    vcgmesh_stub.cpp         # Stubs for setGotNormals/etc (replaces full vcgmesh.cpp)
    ioSMD.cpp
    ioMD3.cpp               # Qt-free, pure FILE* I/O
    saveLoad.cpp
)
# NOT including: ioMB.cpp (Maya Binary — uses QString/QFile/QDebug extensively)
# NOT including: iniData.cpp (module.ini parser — uses QtCore/QTextBrowser)
# NOT including: ioOBJ.cpp (uses QFile/QString — replaced by ioOBJ_sync.cpp)

# Sync-specific sources (Qt-free replacements)
set(SYNC_SOURCES
    main_sync_standalone.cpp
    brfJson_sync.cpp
    ioOBJ_sync.cpp
)

# === Stubs for excluded Qt-dependent code ===
# ioMB.cpp provides IoMB class — we need a stub so linker doesn't complain
# if any core file references it (they don't, but safety)
# iniData.cpp provides IniData class — same situation

# === Target ===
add_executable(brf_sync ${CORE_SOURCES} ${SYNC_SOURCES})

# brfData.h now has #ifndef BRF_NO_QT guard — no forced include needed

# No external library dependencies needed for standalone build

# === Output ===
set_target_properties(brf_sync PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/Debug"
)

# === Post-build: show binary size ===
add_custom_command(TARGET brf_sync POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Built: $<TARGET_FILE:brf_sync>"
)

