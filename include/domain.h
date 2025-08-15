#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> stops;
        bool is_roundtrip = false;
    };

    struct BusInfo {
        size_t stops_count = 0;
        size_t unique_stops_count = 0;
        double route_length = 0.0;
        double curvature = 0.0;
    };

}  // namespace transport_catalogue