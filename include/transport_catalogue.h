#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <unordered_set>

namespace transport_catalogue {

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
        void AddStop(std::string_view name, double lat, double lon);

        // Finds a stop by its name.
        [[nodiscard]] const Stop *FindStop(std::string_view name) const;

        // Adds a bus to the transport catalogue.
        void AddBus(std::string_view name, const std::vector<std::string> &stops, bool is_roundtrip);

        // Finds a bus by its name.
        [[nodiscard]] const Bus *FindBus(std::string_view name) const;

        // Gets information about a bus.
        [[nodiscard]] BusInfo GetBusInfo(std::string_view name) const;

        // Returns a list of buses that pass through a given stop.
        [[nodiscard]] std::vector<const Bus *> GetBusesForStop(std::string_view stop_name) const;

    private:
        std::unordered_map<std::string_view, const Stop *> stops_index_;
        std::unordered_map<std::string_view, const Bus *> buses_index_;
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
    };

} // namespace transport_catalogue