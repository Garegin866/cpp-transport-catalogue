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


# ----------------------------
#  ✅ Integration Tests
# ----------------------------
add_executable(
        TransportCatalogueIntegrationTests
        transport_catalogue_integration_tests.cpp
        helpers.h
)

target_link_libraries(
        TransportCatalogueIntegrationTests
        PRIVATE
        TransportCatalogueLib
        GTest::gtest
)

target_include_directories(
        TransportCatalogueIntegrationTests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Copy tests/data to build dir next to test binary
add_custom_command(
        TARGET TransportCatalogueIntegrationTests
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/tests/data
                ${CMAKE_CURRENT_BINARY_DIR}/data
)

# Discover integration tests
gtest_discover_tests(TransportCatalogueIntegrationTests)