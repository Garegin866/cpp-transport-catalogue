// tests_basic.cpp
#include <gtest/gtest.h>

#include "transport_catalogue.h"
#include "geo.h"

using namespace transport_catalogue;

TEST(TransportCatalogue, AddAndFindStop) {
    TransportCatalogue tc;

    geo::Coordinates a{55.0, 37.0};
    geo::Coordinates b{59.9, 30.3};

    tc.AddStop("A", a);
    tc.AddStop("B", b);

    const Stop* sa = tc.FindStop("A");
    const Stop* sb = tc.FindStop("B");
    const Stop* sn = tc.FindStop("NoSuchStop");

    ASSERT_NE(sa, nullptr);
    ASSERT_NE(sb, nullptr);
    EXPECT_EQ(sa->name, "A");
    EXPECT_EQ(sb->name, "B");
    EXPECT_EQ(sa->coordinates.lat, a.lat);
    EXPECT_EQ(sa->coordinates.lng, a.lng);
    EXPECT_EQ(sn, nullptr);
}

TEST(TransportCatalogue, SetAndGetDistance) {
    TransportCatalogue tc;

    tc.AddStop("A", {55.0, 37.0});
    tc.AddStop("B", {59.9, 30.3});

    const Stop* sa = tc.FindStop("A");
    const Stop* sb = tc.FindStop("B");

    tc.SetDistance(sa, sb, 1234.0);

    // Stored direction
    EXPECT_DOUBLE_EQ(tc.GetDistance(sa, sb), 1234.0);

    // If your GetDistance implements reverse fallback, check it here.
    // If not, replace with EXPECT_DOUBLE_EQ(tc.GetDistance(sb, sa), 0.0);
    // (depends on your implementation contract)
    EXPECT_DOUBLE_EQ(tc.GetDistance(sb, sa), 1234.0);
}

TEST(TransportCatalogue, BusesByStopIndexing) {
    TransportCatalogue tc;

    tc.AddStop("A", {0.0, 0.0});
    tc.AddStop("B", {1.0, 1.0});
    tc.AddStop("C", {2.0, 2.0});

    const Stop* sa = tc.FindStop("A");
    const Stop* sb = tc.FindStop("B");
    const Stop* sc = tc.FindStop("C");

    // Add two buses passing A
    tc.AddBus("10", std::vector<const Stop*>{sa, sb}, false);
    tc.AddBus("20", std::vector<const Stop*>{sa, sc}, true);

    // A has both buses
    const auto& buses_at_a = tc.GetBusesForStop(sa);
    ASSERT_EQ(buses_at_a.size(), 2u);

    // B has only "10"
    const auto& buses_at_b = tc.GetBusesForStop(sb);
    ASSERT_EQ(buses_at_b.size(), 1u);
    EXPECT_EQ((*buses_at_b.begin())->name, "10");

    // Nonexistent stop yields empty set (by contract your impl returns a reference to a static empty set)
    const Stop bogus{"X", {0,0}};
    const auto& empty_ref = tc.GetBusesForStop(&bogus);
    EXPECT_TRUE(empty_ref.empty());
}

TEST(TransportCatalogue, FindBusAndStopReturnNullForMissing) {
    TransportCatalogue tc;
    EXPECT_EQ(tc.FindStop("nope"), nullptr);
    EXPECT_EQ(tc.FindBus("nope"), nullptr);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}