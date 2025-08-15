#pragma once

#include <string>
#include <unordered_set>

#include "transport_catalogue.h"
#include "map_renderer.h"

namespace transport_catalogue {

    class RequestHandler {
    public:
        RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

        // Возвращает информацию о маршруте (запрос Bus)
        [[nodiscard]] std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через
        [[nodiscard]] const std::unordered_set<const Bus*>* GetBusesByStop(const std::string_view& stop_name) const;

        // Рендерит карту и возвращает SVG документ
        [[nodiscard]] svg::Document RenderMap() const;

    private:
        const TransportCatalogue& db_;
        const renderer::MapRenderer& renderer_;
    };

}