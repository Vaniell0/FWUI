cmake_minimum_required(VERSION 3.22.1)
project(FWUI VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

# Поиск OpenCV
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    include_directories(${OpenCV_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "OpenCV not found. Please install OpenCV")
endif()

# Директории
include_directories(src include elements)

# Исполняемый файл
add_executable(my-app
    src/main.cpp
    src/utils.cpp
    
    src/pages/about.cpp
    src/pages/contacts.cpp
    src/pages/footer.cpp
    src/pages/header.cpp
    src/pages/navigation.cpp
    src/pages/projects.cpp

    elements/text.cpp
    elements/html.cpp
    elements/decorators.cpp
    elements/containers.cpp
    elements/factories.cpp
    elements/image.cpp
)

# Связывание библиотек
target_link_libraries(my-app PRIVATE 
    fmt::fmt
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)

# Определение макроса для использования OpenCV
target_compile_definitions(my-app PRIVATE HAVE_OPENCV)

set(EXECUTABLE_OUTPUT_PATH "./bin")
