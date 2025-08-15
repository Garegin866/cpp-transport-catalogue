#include "request_handler.h"

#include <cmath>

namespace transport_catalogue {

    RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
            : db_(db), renderer_(renderer) {}

    [[nodiscard]] std::optional<BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        const Bus* bus = db_.FindBus(bus_name);
        if (!bus) {
            return std::nullopt;
        }

        const size_t n = bus->stops.size();
        if (n == 0) {
            return  BusInfo {0, 0, 0.0};
        }

        BusInfo info;
        const auto& stops = bus->stops;
        info.stops_count = bus->is_roundtrip ? n : (n * 2 - 1);
        info.unique_stops_count = std::unordered_set<const Stop*>(stops.begin(), stops.end()).size();

        double road_len = 0.0;
        for (size_t i = 1; i < n; ++i) {
            road_len += db_.GetDistance(stops[i - 1], stops[i]);
        }
        if (!bus->is_roundtrip) {
            for (size_t i = n; i-- > 1; ) {
                road_len += db_.GetDistance(stops[i], stops[i - 1]);
            }
        }
        info.route_length = road_len;

        double geo_len = 0.0;
        for (size_t i = 1; i < n; ++i) {
            geo_len += geo::ComputeDistance(stops[i - 1]->coordinates, stops[i]->coordinates);
        }
        if (!bus->is_roundtrip) {
            for (size_t i = n; i-- > 1; ) {
                geo_len += geo::ComputeDistance(stops[i]->coordinates, stops[i - 1]->coordinates);
            }
        }

        info.curvature = (geo_len > 0.0) ? (road_len / geo_len) : 0.0;

        return info;
    }

    [[nodiscard]] const std::unordered_set<const Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        const Stop* stop = db_.FindStop(stop_name);
        if (!stop) {
            return nullptr;
        }

        return &db_.GetBusesForStop(stop);
    }

    svg::Document RequestHandler::RenderMap() const {
        return renderer_.Render(db_);
    }

}

