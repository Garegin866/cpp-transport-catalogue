#include "transport_catalogue.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <cmath>
#include <algorithm>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::string_view name, double lat, double lon) {
        stops_.emplace_back(Stop{std::string(name), lat, lon});
        stops_index_[stops_.back().name] = &stops_.back();
    }

    [[nodiscard]] const Stop* TransportCatalogue::FindStop(std::string_view name) const {
        auto it = stops_index_.find(name);
        return it != stops_index_.end() ? it->second : nullptr;
    }

    void TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string>& stops, bool is_roundtrip) {
        buses_.emplace_back(Bus{std::string(name), stops, is_roundtrip});
        buses_index_[buses_.back().name] = &buses_.back();
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
                double distance = std::hypot(curr_stop->latitude - prev_stop->latitude,
                                             curr_stop->longitude - prev_stop->longitude);
                info.route_length += distance;
            }
        }

        return info;
    }

    [[nodiscard]] std::vector<const Bus*> TransportCatalogue::GetBusesForStop(std::string_view stop_name) const {
        std::vector<const Bus*> buses;
        for (const auto& bus_pair : buses_index_) {
            const Bus* bus = bus_pair.second;
            if (std::find(bus->stops.begin(), bus->stops.end(), stop_name) != bus->stops.end()) {
                buses.push_back(bus);
            }
        }
        return buses;
    }

} // namespace transport_catalogue