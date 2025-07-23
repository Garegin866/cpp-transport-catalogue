#include "transport_catalogue.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <cmath>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::string_view name, const geo::Coordinates& coordinates) {
        stops_.emplace_back(Stop{std::string(name), coordinates});
        stops_index_[stops_.back().name] = &stops_.back();
    }

    [[nodiscard]] const Stop* TransportCatalogue::FindStop(std::string_view name) const {
        auto it = stops_index_.find(name);
        return it != stops_index_.end() ? it->second : nullptr;
    }

    void TransportCatalogue::AddBus(std::string_view name, const std::vector<const Stop*>& stops, bool is_roundtrip) {
        buses_.emplace_back(Bus{std::string(name), stops, is_roundtrip});
        buses_index_[buses_.back().name] = &buses_.back();

        const Bus* bus_ptr = &buses_.back();
        for (const Stop* stop : stops) {
            if (stop) {
                stop_to_buses_[stop].insert(bus_ptr);
            }
        }
    }

    [[nodiscard]] const Bus* TransportCatalogue::FindBus(std::string_view name) const {
        auto it = buses_index_.find(name);
        return it != buses_index_.end() ? it->second : nullptr;
    }

    [[nodiscard]] BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const {
        const Bus* bus = FindBus(name);
        if (!bus) {
            return {};
        }

        BusInfo info;
        info.stops_count = bus->stops.size();
        info.unique_stops_count = std::unordered_set<const Stop*>(bus->stops.begin(), bus->stops.end()).size();

        for (size_t i = 1; i < bus->stops.size(); ++i) {
            const Stop* prev_stop = bus->stops[i - 1];
            const Stop* curr_stop = bus->stops[i];
            if (prev_stop && curr_stop) {
                double distance = std::hypot(curr_stop->coordinates.lat - prev_stop->coordinates.lat,
                                             curr_stop->coordinates.lng - prev_stop->coordinates.lng);
                info.route_length += distance;
            }
        }

        return info;
    }

    [[nodiscard]] const std::unordered_set<const Bus*>& TransportCatalogue::GetBusesForStop(const Stop* stop) const {
        auto it = stop_to_buses_.find(stop);
        if (it != stop_to_buses_.end()) {
            return it->second;
        }
        static const std::unordered_set<const Bus*> empty_set;
        return empty_set;
    }

    void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, double distance) {
        if (from && to) {
            distances_[{from, to}] = distance;
        }
    }

    double TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
        if (from && to) {
            auto it = distances_.find({from, to});
            if (it != distances_.end()) {
                return it->second;
            }

            // If the distance is not found, check the reverse direction
            it = distances_.find({to, from});
            if (it != distances_.end()) {
                return it->second;
            }
        }
        return 0.0; // Return 0 if distance is not set
    }

} // namespace transport_catalogue