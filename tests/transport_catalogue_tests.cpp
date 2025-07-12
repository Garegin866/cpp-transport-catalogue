#include <gtest/gtest.h>

#include "transport_catalogue.h"

TEST(TransportCatalogueTest, AddAndFindStopWithCoordinates) {
    TransportCatalogue catalogue;

    catalogue.AddStop("A", 55.0, 37.0);
    catalogue.AddStop("B", 56.0, 38.0);

    const Stop* stopA = catalogue.FindStop("A");
    ASSERT_NE(stopA, nullptr);
    EXPECT_DOUBLE_EQ(stopA->latitude, 55.0);
    EXPECT_DOUBLE_EQ(stopA->longitude, 37.0);

    const Stop* stopB = catalogue.FindStop("B");
    ASSERT_NE(stopB, nullptr);
    EXPECT_DOUBLE_EQ(stopB->latitude, 56.0);
    EXPECT_DOUBLE_EQ(stopB->longitude, 38.0);

    EXPECT_EQ(catalogue.FindStop("X"), nullptr);
}

TEST(TransportCatalogueTest, AddBusAndBusInfo) {
    TransportCatalogue catalogue;

    catalogue.AddStop("A", 0.0, 0.0);
    catalogue.AddStop("B", 3.0, 4.0); // расстояние A-B = 5

    catalogue.AddBus("Bus1", {"A", "B", "A"}, true);
    BusInfo info1 = catalogue.GetBusInfo("Bus1");

    EXPECT_EQ(info1.stops_count, 3);
    EXPECT_EQ(info1.unique_stops_count, 2);
    EXPECT_NEAR(info1.route_length, 10.0, 1e-6);

    catalogue.AddBus("Bus2", {"A", "B"}, false);
    BusInfo info2 = catalogue.GetBusInfo("Bus2");

    EXPECT_EQ(info2.stops_count, 2);
    EXPECT_EQ(info2.unique_stops_count, 2);
    EXPECT_NEAR(info2.route_length, 5.0, 1e-6);

    BusInfo infoX = catalogue.GetBusInfo("NoBus");
    EXPECT_EQ(infoX.stops_count, 0);
    EXPECT_EQ(infoX.route_length, 0.0);
}

TEST(TransportCatalogueTest, UniqueStopsCount) {
    TransportCatalogue catalogue;

    catalogue.AddStop("A", 0.0, 0.0);
    catalogue.AddStop("B", 1.0, 1.0);
    catalogue.AddStop("C", 2.0, 2.0);

    catalogue.AddBus("LoopBus", {"A", "B", "A", "C"}, true);
    BusInfo info = catalogue.GetBusInfo("LoopBus");

    EXPECT_EQ(info.stops_count, 4);
    EXPECT_EQ(info.unique_stops_count, 3);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
