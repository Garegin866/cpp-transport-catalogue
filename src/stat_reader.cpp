#include "stat_reader.h"
#include "geo.h"
#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_set>

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue,
                       std::string_view request,
                       std::ostream& output) {
    std::string_view prefix = "Bus ";
    if (request.substr(0, prefix.size()) != prefix) {
        return;
    }

    std::string bus_name = std::string(request.substr(prefix.size()));

    const Bus* bus = transport_catalogue.FindBus(bus_name);
    if (!bus) {
        output << "Bus " << bus_name << ": not found\n";
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
            route_length += ComputeDistance({prev_stop->latitude, prev_stop->longitude},
                                            {curr_stop->latitude, curr_stop->longitude});
        }
    }

    output << "Bus " << bus_name << ": "
            << stops_count << " stops on route, "
            << unique_stops_count << " unique stops, "
            << std::setprecision(6) << route_length << " route length\n";
}
