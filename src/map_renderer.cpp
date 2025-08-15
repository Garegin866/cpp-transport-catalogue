#include "map_renderer.h"

#include <set>

using namespace std;

constexpr const char* FONT_FAMILY = "Verdana";
constexpr const char* FONT_WEIGHT = "bold";
constexpr const char* FONT_COLOR = "black";

namespace transport_catalogue::renderer {

    static bool BusLessByName(const Bus* a, const Bus* b) {
        return a->name < b->name;
    }

    static vector<const Bus*> GetBusesSorted(const TransportCatalogue& db) {
        vector<const Bus*> buses;
        for (const auto& bus : db.GetAllBuses()) {
            if (!bus.stops.empty()) {
                buses.push_back(&bus);
            }
        }
        sort(buses.begin(), buses.end(), BusLessByName);
        return buses;
    }

    static vector<const Stop*> CollectPlottedStopsSorted(const TransportCatalogue& db) {
        unordered_set<const Stop*> used;
        for (const auto& bus : db.GetAllBuses()) {
            for (auto* s : bus.stops) if (s) used.insert(s);
        }
        vector<const Stop*> out(used.begin(), used.end());
        sort(out.begin(), out.end(), [](const Stop* a, const Stop* b){ return a->name < b->name; });
        return out;
    }

    svg::Text MapRenderer::MakeBusTextUnderlayer(svg::Point p, std::string_view name) const {
        svg::Text t;
        t.SetPosition(p)
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size))
                .SetFontFamily(FONT_FAMILY)
                .SetFontWeight(FONT_WEIGHT)
                .SetData(string(name))
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        return t;
    }

    svg::Text MapRenderer::MakeBusText(svg::Point p, std::string_view name, const svg::Color& color) const {
        svg::Text t;
        t.SetPosition(p)
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size))
                .SetFontFamily(FONT_FAMILY)
                .SetFontWeight(FONT_WEIGHT)
                .SetData(string(name))
                .SetFillColor(color);
        return t;
    }

    svg::Text MapRenderer::MakeStopTextUnderlayer(svg::Point p, std::string_view name) const {
        svg::Text t;
        t.SetPosition(p)
                .SetOffset(settings_.stop_label_offset)
                .SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size))
                .SetFontFamily(FONT_FAMILY)
                .SetData(string(name))
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        return t;
    }

    svg::Text MapRenderer::MakeStopText(svg::Point p, std::string_view name) const {
        svg::Text t;
        t.SetPosition(p)
                .SetOffset(settings_.stop_label_offset)
                .SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size))
                .SetFontFamily(FONT_FAMILY)
                .SetData(string(name))
                .SetFillColor(string(FONT_COLOR));
        return t;
    }

    const svg::Color& MapRenderer::ColorForIndex(size_t i) const {
        return settings_.color_palette[i % settings_.color_palette.size()];
    }

    void MapRenderer::RenderBusLines(svg::Document& doc,
                                     const std::vector<const Bus*>& buses,
                                     const detail::SphereProjector& proj,
                                     std::vector<size_t>& bus_color_index) const {
        size_t color_idx = 0;
        for (size_t i = 0; i < buses.size(); ++i) {
            const Bus* bus = buses[i];
            if (bus->stops.empty()) continue;

            bus_color_index[i] = color_idx;
            const auto& color = ColorForIndex(color_idx++);
            svg::Polyline pl;
            pl.SetFillColor(svg::NoneColor)
                    .SetStrokeColor(color)
                    .SetStrokeWidth(settings_.line_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            for (const Stop* s : bus->stops) pl.AddPoint(proj(s->coordinates));
            if (!bus->is_roundtrip && bus->stops.size() > 1) {
                for (size_t k = bus->stops.size() - 2; k < bus->stops.size(); --k) {
                    pl.AddPoint(proj(bus->stops[k]->coordinates));
                    if (k == 0) break;
                }
            }
            doc.Add(std::move(pl));
        }
    }

    void MapRenderer::RenderBusLabels(svg::Document& doc,
                                      const std::vector<const Bus*>& buses,
                                      const detail::SphereProjector& proj,
                                      const std::vector<size_t>& bus_color_index) const {
        for (size_t i = 0; i < buses.size(); ++i) {
            const Bus* bus = buses[i];
            if (bus->stops.empty()) continue;

            const auto& color = ColorForIndex(bus_color_index[i]);
            const Stop* first = bus->stops.front();
            doc.Add(MakeBusTextUnderlayer(proj(first->coordinates), bus->name));
            doc.Add(MakeBusText(proj(first->coordinates), bus->name, color));

            if (!bus->is_roundtrip) {
                const Stop* last = bus->stops.back();
                if (last != first) {
                    doc.Add(MakeBusTextUnderlayer(proj(last->coordinates), bus->name));
                    doc.Add(MakeBusText(proj(last->coordinates), bus->name, color));
                }
            }
        }
    }

    void MapRenderer::RenderStopCircles(svg::Document& doc,
                                        const std::vector<const Stop*>& stops,
                                        const detail::SphereProjector& proj) const {
        for (const Stop* s : stops) {
            doc.Add(svg::Circle()
                            .SetCenter(proj(s->coordinates))
                            .SetRadius(settings_.stop_radius)
                            .SetFillColor("white"));
        }
    }

    void MapRenderer::RenderStopLabels(svg::Document& doc,
                                       const std::vector<const Stop*>& stops,
                                       const detail::SphereProjector& proj) const {
        for (const Stop* s : stops) {
            doc.Add(MakeStopTextUnderlayer(proj(s->coordinates), s->name));
            doc.Add(MakeStopText(proj(s->coordinates), s->name));
        }
    }

    svg::Document MapRenderer::Render(const TransportCatalogue& db) const {
        svg::Document doc;

        auto buses = GetBusesSorted(db);
        auto stops = CollectPlottedStopsSorted(db);

        std::vector<geo::Coordinates> coords;
        coords.reserve(stops.size());
        for (auto* s : stops) coords.push_back(s->coordinates);
        detail::SphereProjector proj(coords.begin(), coords.end(),
                             settings_.width, settings_.height, settings_.padding);

        std::vector<size_t> bus_color_index(buses.size());
        RenderBusLines(doc, buses, proj, bus_color_index);
        RenderBusLabels(doc, buses, proj, bus_color_index);
        RenderStopCircles(doc, stops, proj);
        RenderStopLabels(doc, stops, proj);

        return doc;
    }

} // namespace transport_catalogue::renderer
