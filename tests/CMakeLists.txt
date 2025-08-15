# -----------------------
#  ✅ Unit Tests
# -----------------------
add_executable(
        TransportCatalogueTests
        transport_catalogue_tests.cpp
)

target_link_libraries(
        TransportCatalogueTests
        PRIVATE
        TransportCatalogueLib
        GTest::gtest
)

target_include_directories(
        TransportCatalogueTests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(TransportCatalogueTests)