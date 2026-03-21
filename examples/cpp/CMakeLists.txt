cmake_minimum_required(VERSION 3.22)
project(fwui_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Option 1: FetchContent (recommended for new projects) ────
#
# include(FetchContent)
# FetchContent_Declare(fwui
#   GIT_REPOSITORY https://github.com/Vaniell0/fwui.git
#   GIT_TAG        v1.0.0
# )
# set(FWUI_GEM_BUILD ON CACHE BOOL "" FORCE)  # skip server deps
# FetchContent_MakeAvailable(fwui)

# ── Option 2: Local path (for development) ───────────────────
set(FWUI_GEM_BUILD ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/fwui)

# ── Build example ────────────────────────────────────────────
add_executable(example main.cpp)
target_link_libraries(example PRIVATE fwui)
