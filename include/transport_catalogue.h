#pragma once

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>

#include "domain.h"

namespace transport_catalogue {

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

        // Returns bus information if it exists, otherwise returns std::nullopt.
        [[nodiscard]] std::optional<BusInfo> GetBusInfo(const std::string_view& bus_name) const;

        // Returns a list of buses that pass through a given stop.
        [[nodiscard]] const std::unordered_set<const Bus*>& GetBusesForStop(const Stop *stop) const;

        // Sets the distance between two stops.
        void SetDistance(const Stop *from, const Stop *to, double distance);

        // Gets the distance between two stops.
        [[nodiscard]] double GetDistance(const Stop *from, const Stop *to) const;

        // Public accessors for buses and stops
        [[nodiscard]] const std::deque<Bus>& GetAllBuses() const { return buses_; }
        [[nodiscard]] const std::deque<Stop>& GetAllStops() const { return stops_; }

    private:
        std::unordered_map<std::string_view, const Stop *> stops_index_;
        std::unordered_map<std::string_view, const Bus *> buses_index_;
        std::unordered_map<const Stop *, std::unordered_set<const Bus*>> stop_to_buses_;
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::pair<const Stop*, const Stop*>, double, PtrPairHasher> distances_;
    };

} // namespace transport_catalogue