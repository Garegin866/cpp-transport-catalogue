#include "json_reader.h"

#include "domain.h"

#include <string>
#include <unordered_set>
#include <algorithm>
#include <sstream>

// Constants for JSON keys
constexpr const char* BASE_REQUESTS_KEY = "base_requests";
constexpr const char* STAT_REQUESTS_KEY = "stat_requests";
constexpr const char* RENDER_SETTINGS_KEY = "render_settings";

constexpr const char* ID_KEY = "id";
constexpr const char* TYPE_KEY = "type";
constexpr const char* NAME_KEY = "name";
constexpr const char* LATITUDE_KEY = "latitude";
constexpr const char* LONGITUDE_KEY = "longitude";

constexpr const char* STOP_TYPE = "Stop";
constexpr const char* BUS_TYPE = "Bus";
constexpr const char* MAP_TYPE = "Map";

namespace transport_catalogue {

    JsonReader::JsonReader(json::Document input_doc,
                           TransportCatalogue& db)
        : db_(db), input_doc_(std::move(input_doc)) {
        ReadInput();
    }

    static const json::Node* TryGet(const json::Dict& d, std::string_view k) {
        auto it = d.find(std::string(k));
        return it == d.end() ? nullptr : &it->second;
    }

    static svg::Color ParseColorNode(const json::Node& n) {
        if (n.IsString()) return n.AsString();
        const auto& arr = n.AsArray();
        if (arr.size() == 3) {
            return svg::Rgb(
                    static_cast<uint8_t>(arr[0].AsInt()),
                    static_cast<uint8_t>(arr[1].AsInt()),
                    static_cast<uint8_t>(arr[2].AsInt())
            );
        }
        return svg::Rgba(
                static_cast<uint8_t>(arr[0].AsInt()),
                static_cast<uint8_t>(arr[1].AsInt()),
                static_cast<uint8_t>(arr[2].AsInt()),
                arr[3].AsDouble()
        );
    }

    void JsonReader::ReadInput() {
        const auto& root = input_doc_.GetRoot().AsMap();
        if (auto it = root.find(BASE_REQUESTS_KEY); it != root.end() && it->second.IsArray()) {
            ParseBaseRequests(it->second.AsArray());
        }
        if (auto it = root.find(STAT_REQUESTS_KEY); it != root.end() && it->second.IsArray()) {
            ParseStatRequests(it->second.AsArray());
        }
    }

    void JsonReader::ProcessBaseRequests() {
        for (const auto& s : stops_) {
            db_.AddStop(s.name, s.coords);
        }

        for (const auto& s : stops_) {
            const Stop* from = db_.FindStop(s.name);
            for (const auto& [to_name, dist] : s.road_distances) {
                const Stop* to = db_.FindStop(to_name);
                if (from && to) {
                    db_.SetDistance(from, to, dist);
                }
            }
        }

        for (const auto& b : buses_) {
            std::vector<const Stop*> stops_ptrs;
            stops_ptrs.reserve(b.stops.size());
            for (const auto& stop_name : b.stops) {
                stops_ptrs.push_back(db_.FindStop(stop_name));
            }
            db_.AddBus(b.name, stops_ptrs, b.is_roundtrip);
        }
    }

    json::Array JsonReader::ProcessStatRequests(const RequestHandler& handler) const {
        json::Array responses;
        responses.reserve(stat_requests_.size());

        for (const auto& req : stat_requests_) {
            json::Node response_node;

            if (req.type == BUS_TYPE) {
                auto bus_info = handler.GetBusStat(req.name);
                if (bus_info) {
                    response_node = json::Dict{
                        {"request_id", req.id},
                        {"curvature", bus_info->curvature},
                        {"route_length", bus_info->route_length},
                        {"stop_count", static_cast<int>(bus_info->stops_count)},
                        {"unique_stop_count", static_cast<int>(bus_info->unique_stops_count)}
                    };
                } else {
                    response_node = json::Dict{
                        {"request_id", req.id},
                        {"error_message", "not found"}
                    };
                }
            } else if (req.type == STOP_TYPE) {
                json::Array buses_json;
                auto buses = handler.GetBusesByStop(req.name);
                if (!buses) {
                    response_node = json::Dict{
                        {"request_id", req.id},
                        {"error_message", "not found"}
                    };
                } else {
                    std::vector<std::string> names;
                    names.reserve(buses->size());
                    for (const auto* bus : *buses) {
                        names.emplace_back(bus->name);
                    }
                    std::sort(names.begin(), names.end());
                    names.erase(std::unique(names.begin(), names.end()), names.end());

                    buses_json.reserve(names.size());
                    for (const auto& name : names) {
                        buses_json.emplace_back(name);
                    }

                    response_node = json::Dict{
                            {"request_id", req.id},
                            {"buses",      std::move(buses_json)}
                    };
                }
            } else if (req.type == MAP_TYPE) {
                std::ostringstream svg_buf;
                handler.RenderMap().Render(svg_buf);

                response_node = json::Dict{
                        {"request_id", req.id},
                        {"map", svg_buf.str()}
                };
            }

            responses.push_back(std::move(response_node));
        }

        return responses;
    }

    void JsonReader::ProcessRenderSettings(renderer::MapRenderer& renderer) {
        const auto& root = input_doc_.GetRoot().AsMap();
        auto it_rs = root.find(RENDER_SETTINGS_KEY);
        if (it_rs == root.end()) return; // nothing to render

        const auto& rs = it_rs->second.AsMap();
        renderer::RenderSettings s{};   // have sensible defaults in this struct

        if (auto p = TryGet(rs, "width")) {
            s.width = p->AsDouble();
        }
        if (auto p = TryGet(rs, "height")) {
            s.height = p->AsDouble();
        }
        if (auto p = TryGet(rs, "padding")) {
            s.padding = p->AsDouble();
        }
        if (auto p = TryGet(rs, "line_width")) {
            s.line_width = p->AsDouble();
        }
        if (auto p = TryGet(rs, "stop_radius")) {
            s.stop_radius = p->AsDouble();
        }
        if (auto p = TryGet(rs, "bus_label_font_size")) {
            s.bus_label_font_size = p->AsInt();
        }
        if (auto p = TryGet(rs, "bus_label_offset")) {
            const auto& a = p->AsArray();
            if (a.size() >= 2) {
                s.bus_label_offset = { a[0].AsDouble(), a[1].AsDouble() };
            }
        }
        if (auto p = TryGet(rs, "stop_label_font_size")) {
            s.stop_label_font_size = p->AsInt();
        }
        if (auto p = TryGet(rs, "stop_label_offset")) {
            const auto& a = p->AsArray();
            if (a.size() >= 2) {
                s.stop_label_offset = { a[0].AsDouble(), a[1].AsDouble() };
            }
        }
        if (auto p = TryGet(rs, "underlayer_color")) {
            s.underlayer_color = ParseColorNode(*p);
        }
        if (auto p = TryGet(rs, "underlayer_width")) {
            s.underlayer_width = p->AsDouble();
        }
        if (auto p = TryGet(rs, "color_palette")) {
            for (const auto& c : p->AsArray()) {
                s.color_palette.push_back(ParseColorNode(c));
            }
        }

        renderer.SetSettings(std::move(s));
    }

    void JsonReader::ParseBaseRequests(const json::Array& reqs) {
        for (const auto& node : reqs) {
            if (!node.IsMap()) continue;
            const auto& m = node.AsMap();

            const auto* type_n = TryGet(m, TYPE_KEY);
            if (!type_n || !type_n->IsString()) continue;
            const std::string type = type_n->AsString();

            if (type == STOP_TYPE) {
                ParseStopRequests(m);
            } else if (type == BUS_TYPE) {
                ParseBusRequests(m);
            }
        }
    }

    void JsonReader::ParseStatRequests(const json::Array& reqs) {
        for (const auto& node : reqs) {
            if (!node.IsMap()) continue;
            const auto& m = node.AsMap();

            StatRequest stat_request;
            if (const auto* type_n = TryGet(m, TYPE_KEY); type_n && type_n->IsString()) {
                stat_request.type = type_n->AsString();
            }
            if (const auto* name_n = TryGet(m, NAME_KEY); name_n && name_n->IsString()) {
                stat_request.name = name_n->AsString();
            }
            if (const auto* id_n = TryGet(m, ID_KEY); id_n && id_n->IsInt()) {
                stat_request.id = id_n->AsInt();
            }

            stat_requests_.push_back(std::move(stat_request));
        }
    }

    void JsonReader::ParseStopRequests(const json::Dict& dict) {
        const auto* name_n = TryGet(dict, NAME_KEY);
        const auto* lat_n  = TryGet(dict, LATITUDE_KEY);
        const auto* lng_n  = TryGet(dict, LONGITUDE_KEY);
        if (!name_n || !lat_n || !lng_n || !name_n->IsString() || !lat_n->IsDouble() || !lng_n->IsDouble()) {
            return;
        }

        StopInput s;
        s.name   = name_n->AsString();
        s.coords = { lat_n->AsDouble(), lng_n->AsDouble() };

        if (auto rd_it = dict.find("road_distances"); rd_it != dict.end() && rd_it->second.IsMap()) {
            for (const auto& [to_name, dist_node] : rd_it->second.AsMap()) {
                if (dist_node.IsInt()) {
                    s.road_distances[to_name] = dist_node.AsInt();
                } else if (dist_node.IsDouble()) {
                    s.road_distances[to_name] = static_cast<int>(dist_node.AsDouble());
                }
            }
        }

        stops_.push_back(std::move(s));
    }

    void JsonReader::ParseBusRequests(const json::Dict& dict) {
        const auto* name_n = TryGet(dict, NAME_KEY);
        if (!name_n || !name_n->IsString()) return;

        BusInput b;
        b.name = name_n->AsString();

        if (const auto* rt = TryGet(dict, "is_roundtrip"); rt && rt->IsBool()) {
            b.is_roundtrip = rt->AsBool();
        } else {
            b.is_roundtrip = false;
        }

        if (const auto* stops_n = TryGet(dict, "stops"); stops_n && stops_n->IsArray()) {
            const auto& arr = stops_n->AsArray();
            b.stops.reserve(arr.size());
            for (const auto& s : arr) {
                if (s.IsString()) b.stops.push_back(s.AsString());
            }
        }

        buses_.push_back(std::move(b));
    }

} // namespace transport_catalogue
