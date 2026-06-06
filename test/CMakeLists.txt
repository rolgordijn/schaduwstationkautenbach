cmake_minimum_required(VERSION 3.14)
project(schaduwstation_tests CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Catch2 ────────────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# ── Include paths ─────────────────────────────────────────────────────────────
# mock/ must come before .. so Arduino.h, HardwareSerial.h, MCP23017.h
# resolve to stubs rather than the real Arduino SDK headers.
include_directories(BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}/mock
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# ── Production sources (hardware-independent only) ────────────────────────────
set(PROD_SOURCES
    ../IO.cpp
    ../VirtualIO.cpp
    ../Track.cpp
    ../Wissel.cpp
    ../InrijPoort.cpp
    ../Kopspoor.cpp
    ../Autopilot.cpp
    ../Knipper.cpp
)

# ── Test executable ───────────────────────────────────────────────────────────
add_executable(tests
    ${PROD_SOURCES}
    mock/Arduino.cpp
    test_track.cpp
    test_wissel.cpp
    test_inrijpoort.cpp
    test_autopilot.cpp
    test_kopspoor.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
