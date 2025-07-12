#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <unordered_set>

struct Stop {
    std::string name;
    double latitude = 0.0;
    double longitude = 0.0;
};

struct Bus {
    std::string name;
    std::vector<std::string> stops;
    bool is_roundtrip;
};

struct BusInfo {
    size_t stops_count = 0;
    size_t unique_stops_count = 0;
    double route_length = 0.0;
};

class TransportCatalogue {
public:
    // Adds a stop to the transport catalogue.
    void AddStop(const std::string& name, double lat, double lon);

    // Finds a stop by its name.
    [[nodiscard]] const Stop* FindStop(const std::string& name) const;

    // Adds a bus to the transport catalogue.
    void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);

    // Finds a bus by its name.
    [[nodiscard]] const Bus* FindBus(const std::string& name) const;

    // Gets information about a bus.
    [[nodiscard]] BusInfo GetBusInfo(const std::string& name) const;

private:
    std::unordered_map<std::string_view, const Stop*> stops_index_;
    std::unordered_map<std::string_view, const Bus*> buses_index_;
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

};