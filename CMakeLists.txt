cmake_minimum_required(VERSION 3.31)

project(TransportCatalogue)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(TransportCatalogueLib
    src/transport_catalogue.cpp
    src/input_reader.cpp
    src/stat_reader.cpp
)

target_include_directories(TransportCatalogueLib PUBLIC ${CMAKE_SOURCE_DIR}/include)

find_package(GTest REQUIRED)

add_executable(transport_catalogue
        main.cpp
)

target_link_libraries(transport_catalogue PRIVATE TransportCatalogueLib)

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(tests)
