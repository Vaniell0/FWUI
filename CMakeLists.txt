cmake_minimum_required(VERSION 3.22)
project(FWUI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(FWUI_BUILD_TESTS "Build unit tests" OFF)

# --- Dependencies (find_package first, FetchContent fallback for Windows) ---
include(FetchContent)

# fmt
find_package(fmt QUIET)
if(NOT fmt_FOUND)
  FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        11.1.4
  )
  FetchContent_MakeAvailable(fmt)
endif()

# nlohmann_json
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
  FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
  )
  set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(nlohmann_json)
endif()

# Inja (header-only, Populate only — its CMakeLists.txt conflicts with our nlohmann_json)
cmake_policy(SET CMP0169 OLD)
FetchContent_Declare(inja
    GIT_REPOSITORY https://github.com/pantor/inja.git
    GIT_TAG        v3.4.0
)
FetchContent_Populate(inja)
set(INJA_INCLUDE_DIR ${inja_SOURCE_DIR}/include)

# --- Core static library ---
add_library(fwui STATIC
    src/core.cpp
    src/elements.cpp
    src/decorators.cpp
    src/renderer.cpp
    src/registry.cpp
    src/template_engine.cpp
    src/template_page_loader.cpp
    src/file_watcher.cpp
    src/hot_reload.cpp
)

target_include_directories(fwui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(fwui PRIVATE ${INJA_INCLUDE_DIR})

target_link_libraries(fwui PUBLIC
    fmt::fmt
    nlohmann_json::nlohmann_json
)

# --- Demo executable ---
add_executable(fwui-demo src/main.cpp)
target_link_libraries(fwui-demo PRIVATE fwui)

# --- Benchmark ---
add_executable(fwui-bench tests/bench_cpp.cpp)
target_link_libraries(fwui-bench PRIVATE fwui)

# --- Memory benchmark ---
add_executable(fwui-bench-mem tests/bench_memory.cpp)
target_link_libraries(fwui-bench-mem PRIVATE fwui)

# --- Static site generator CLI ---
add_executable(fwui-ssg src/ssg.cpp src/pages/ssg_pages.cpp)
target_link_libraries(fwui-ssg PRIVATE fwui)

# --- Embed generator (pages -> constexpr C++ source) ---
add_executable(fwui-embed src/embed_gen.cpp src/pages/ssg_pages.cpp)
target_link_libraries(fwui-embed PRIVATE fwui)

# --- Install ---
install(TARGETS fwui
    ARCHIVE DESTINATION lib
)
install(TARGETS fwui-demo fwui-ssg fwui-embed
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/fwui DESTINATION include)

# --- Tests ---
if(FWUI_BUILD_TESTS)
  enable_testing()

  FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
  )
  FetchContent_MakeAvailable(Catch2)

  add_executable(fwui-tests
    tests/test_node.cpp
    tests/test_elements.cpp
    tests/test_decorators.cpp
    tests/test_renderer.cpp
    tests/test_registry.cpp
    tests/test_template_engine.cpp
  )
  target_link_libraries(fwui-tests PRIVATE fwui Catch2::Catch2WithMain)

  include(CTest)
  include(Catch)
  catch_discover_tests(fwui-tests)
endif()
