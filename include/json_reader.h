#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "map_renderer.h"

namespace transport_catalogue {

    class JsonReader {
    public:
        explicit JsonReader(json::Document input_doc,
                            TransportCatalogue& db);

        // Process base_requests into the catalogue
        void ProcessBaseRequests();

        // Build JSON array with answers for stat_requests
        [[nodiscard]] json::Array ProcessStatRequests(const RequestHandler& handler) const;

        // Read render settings from the input document
        void ProcessRenderSettings(renderer::MapRenderer& renderer);

    private:
        TransportCatalogue& db_;
        json::Document input_doc_;

        void ReadInput();

        struct StopInput {
            std::string name;
            geo::Coordinates coords;
            std::map<std::string, int> road_distances;
        };

        struct BusInput {
            std::string name;
            std::vector<std::string> stops;
            bool is_roundtrip;
        };

        struct StatRequest {
            std::string type;
            std::string name;
            int id = 0;
        };

        void ParseBaseRequests(const json::Array& reqs);
        void ParseStatRequests(const json::Array& reqs);

        void ParseStopRequests(const json::Dict& dict);
        void ParseBusRequests(const json::Dict& dict);

        std::vector<StopInput> stops_;
        std::vector<BusInput> buses_;
        std::vector<StatRequest> stat_requests_;
    };

} // namespace transport_catalogue