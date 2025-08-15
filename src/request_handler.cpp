#include "request_handler.h"

#include <cmath>

namespace transport_catalogue {

    RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
            : db_(db), renderer_(renderer) {}

    [[nodiscard]] std::optional<BusInfo> RequestHandler::GetBusInfo(const std::string_view& bus_name) const {
        return db_.GetBusInfo(bus_name);
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

