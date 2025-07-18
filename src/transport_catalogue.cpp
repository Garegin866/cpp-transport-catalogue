#include "transport_catalogue.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <cmath>
#include <algorithm>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::string_view name, const geo::Coordinates& coordinates) {
        stops_.emplace_back(Stop{std::string(name), coordinates});
        stops_index_[stops_.back().name] = &stops_.back();
    }

    [[nodiscard]] const Stop* TransportCatalogue::FindStop(std::string_view name) const {
        auto it = stops_index_.find(name);
        return it != stops_index_.end() ? it->second : nullptr;
    }

    void TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string_view>& stops, bool is_roundtrip) {
        buses_.emplace_back(Bus{std::string(name), stops, is_roundtrip});
        buses_index_[buses_.back().name] = &buses_.back();

        const Bus* bus_ptr = &buses_.back();
        for (std::string_view stop_name : stops) {
            stop_to_buses_[stop_name].insert(bus_ptr);
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
        info.unique_stops_count = std::unordered_set<std::string>(bus->stops.begin(), bus->stops.end()).size();

        for (size_t i = 1; i < bus->stops.size(); ++i) {
            const Stop* prev_stop = FindStop(bus->stops[i - 1]);
            const Stop* curr_stop = FindStop(bus->stops[i]);
            if (prev_stop && curr_stop) {
                double distance = std::hypot(curr_stop->coordinates.lat - prev_stop->coordinates.lat,
                                             curr_stop->coordinates.lng - prev_stop->coordinates.lng);
                info.route_length += distance;
            }
        }

        return info;
    }

    [[nodiscard]] const std::unordered_set<const Bus*>& TransportCatalogue::GetBusesForStop(std::string_view stop_name) const {
        static const std::unordered_set<const Bus*> empty_result;

        auto it = stop_to_buses_.find(stop_name);
        if (it != stop_to_buses_.end()) {
            return it->second;
        }
        return empty_result;
    }

} // namespace transport_catalogue