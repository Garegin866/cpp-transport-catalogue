#include "transport_catalogue.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>

TEST(TransportCatalogueTest, AddAndFindStop) {
    using namespace transport_catalogue;

    TransportCatalogue catalogue;

    catalogue.AddStop("A", {10.0, 20.0});
    const Stop* stop = catalogue.FindStop("A");

    ASSERT_NE(stop, nullptr);
    EXPECT_DOUBLE_EQ(stop->coordinates.lat, 10.0);
    EXPECT_DOUBLE_EQ(stop->coordinates.lng, 20.0);

    EXPECT_EQ(catalogue.FindStop("Unknown"), nullptr);
}

TEST(TransportCatalogueTest, AddBusAndGetInfo) {
    using namespace transport_catalogue;

    TransportCatalogue catalogue;

    catalogue.AddStop("A", {0, 0});
    catalogue.AddStop("B", {3, 4});

    const Stop* stop_a = catalogue.FindStop("A");
    const Stop* stop_b = catalogue.FindStop("B");
    ASSERT_NE(stop_a, nullptr);
    ASSERT_NE(stop_b, nullptr);

    catalogue.AddBus("Bus1", {stop_a, stop_b, stop_a}, true);

    BusInfo info = catalogue.GetBusInfo("Bus1");

    EXPECT_EQ(info.stops_count, 3);
    EXPECT_EQ(info.unique_stops_count, 2);
    EXPECT_NEAR(info.route_length, 10.0, 1e-6);
}

TEST(TransportCatalogueTest, GetBusesForStop) {
    using namespace transport_catalogue;

    TransportCatalogue catalogue;

    catalogue.AddStop("Stop1", {0, 0});
    catalogue.AddStop("Stop2", {1, 1});

    const Stop* stop1 = catalogue.FindStop("Stop1");
    const Stop* stop2 = catalogue.FindStop("Stop2");
    ASSERT_NE(stop1, nullptr);
    ASSERT_NE(stop2, nullptr);

    catalogue.AddBus("BusA", {stop1, stop2}, false);
    catalogue.AddBus("BusB", {stop1}, false);

    const auto& buses = catalogue.GetBusesForStop(stop1);
    std::vector<std::string> bus_names;
    bus_names.reserve(buses.size());
    for (const Bus* bus : buses) {
        bus_names.push_back(bus->name);
    }
    std::sort(bus_names.begin(), bus_names.end());

    EXPECT_EQ(bus_names.size(), 2u);
    EXPECT_EQ(bus_names[0], "BusA");
    EXPECT_EQ(bus_names[1], "BusB");

    const auto& empty_buses = catalogue.GetBusesForStop(catalogue.FindStop("Unknown"));
    EXPECT_TRUE(empty_buses.empty());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
