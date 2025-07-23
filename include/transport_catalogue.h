#pragma once

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

namespace transport_catalogue {

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> stops;
        bool is_roundtrip;
    };

    struct BusInfo {
        size_t stops_count = 0;
        size_t unique_stops_count = 0;
        double route_length = 0.0;
    };

    struct PtrPairHasher {
        size_t operator()(const std::pair<const void*, const void*>& p) const {
            size_t h1 = std::hash<const void*>{}(p.first);
            size_t h2 = std::hash<const void*>{}(p.second);
            return h1 + 37 * h2;
        }
    };

    class TransportCatalogue {
    public:
        // Adds a stop to the transport catalogue.
        void AddStop(std::string_view name, const geo::Coordinates &coordinates);

        // Finds a stop by its name.
        [[nodiscard]] const Stop *FindStop(std::string_view name) const;

        // Adds a bus to the transport catalogue.
        void AddBus(std::string_view name, const std::vector<const Stop*>& stops, bool is_roundtrip);

        // Finds a bus by its name.
        [[nodiscard]] const Bus *FindBus(std::string_view name) const;

        // Gets information about a bus.
        [[nodiscard]] BusInfo GetBusInfo(std::string_view name) const;

        // Returns a list of buses that pass through a given stop.
        [[nodiscard]] const std::unordered_set<const Bus*>& GetBusesForStop(const Stop *stop) const;

        // Sets the distance between two stops.
        void SetDistance(const Stop *from, const Stop *to, double distance);

        // Gets the distance between two stops.
        [[nodiscard]] double GetDistance(const Stop *from, const Stop *to) const;

    private:
        std::unordered_map<std::string_view, const Stop *> stops_index_;
        std::unordered_map<std::string_view, const Bus *> buses_index_;
        std::unordered_map<const Stop *, std::unordered_set<const Bus*>> stop_to_buses_;
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::pair<const Stop*, const Stop*>, double, PtrPairHasher> distances_;
    };

} // namespace transport_catalogue