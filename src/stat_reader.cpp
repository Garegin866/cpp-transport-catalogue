#include "stat_reader.h"
#include "geo.h"
#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <algorithm>

namespace transport_catalogue::stat {

    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue,
                           std::string_view request,
                           std::ostream& output) {
        if (request.empty()) {
            return;
        }

        if (request.substr(0, 3) == "Bus") {
            details::PrintBusStat(transport_catalogue, request, output);
        } else if (request.substr(0, 4) == "Stop") {
            details::PrintStopStat(transport_catalogue, request, output);
        }
    }

    namespace details {

        void PrintBusStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
            constexpr std::string_view prefix = "Bus ";
            if (request.substr(0, prefix.size()) != prefix) {
                return;
            }

            std::string bus_name = std::string(request.substr(prefix.size()));

            const transport_catalogue::Bus* bus = transport_catalogue.FindBus(bus_name);
            if (!bus) {
                output << prefix << bus_name << ": not found\n";
                return;
            }

            int stops_count = static_cast<int>(bus->stops.size());

            std::unordered_set<std::string_view> unique_stops;
            for (const auto& stop : bus->stops) {
                unique_stops.insert(stop);
            }
            int unique_stops_count = static_cast<int>(unique_stops.size());

            double route_length = 0.0;
            for (size_t i = 1; i < bus->stops.size(); ++i) {
                const auto& prev_stop = transport_catalogue.FindStop(bus->stops[i - 1]);
                const auto& curr_stop = transport_catalogue.FindStop(bus->stops[i]);
                if (prev_stop && curr_stop) {
                    route_length += transport_catalogue::geo::ComputeDistance({prev_stop->coordinates.lat, prev_stop->coordinates.lng},
                                                                              {curr_stop->coordinates.lat, curr_stop->coordinates.lng});
                }
            }

            output << prefix << bus_name << ": "
                   << stops_count << " stops on route, "
                   << unique_stops_count << " unique stops, "
                   << std::setprecision(6) << route_length << " route length\n";
        }

        void PrintStopStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
            constexpr std::string_view prefix = "Stop ";
            if (request.substr(0, prefix.size()) != prefix) {
                return;
            }

            std::string stop_name = std::string(request.substr(prefix.size()));

            const transport_catalogue::Stop* stop = transport_catalogue.FindStop(stop_name);
            if (!stop) {
                output << prefix << stop_name << ": not found\n";
                return;
            }

            const auto& buses = transport_catalogue.GetBusesForStop(stop_name);
            if (buses.empty()) {
                output << prefix << stop_name << ": no buses\n";
            } else {
                std::vector<std::string_view> bus_names;
                bus_names.reserve(buses.size());
                for (const auto* bus : buses) {
                    bus_names.push_back(bus->name);
                }
                std::sort(bus_names.begin(), bus_names.end());
                output << prefix << stop_name << ": buses";
                for (const auto& name : bus_names) {
                    output << " " << name;
                }
                output << "\n";
            }
        }

    } // namespace details

} // namespace transport_catalogue::stat