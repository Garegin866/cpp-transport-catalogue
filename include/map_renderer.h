#pragma once

#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <algorithm>

#include "svg.h"
#include "geo.h"
#include "domain.h"
#include "transport_catalogue.h"

namespace transport_catalogue::renderer {

    struct RenderSettings {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;

        double line_width = 0.0;
        double stop_radius = 0.0;

        int bus_label_font_size = 0;
        svg::Point bus_label_offset{0,0};

        int stop_label_font_size = 0;
        svg::Point stop_label_offset{0,0};

        svg::Color underlayer_color = svg::NoneColor;
        double underlayer_width = 0.0;

        std::vector<svg::Color> color_palette;
    };

    namespace detail {

        class SphereProjector {
        public:
            template <typename It>
            SphereProjector(It begin, It end, double width, double height, double padding)
                    : padding_(padding) {
                if (begin == end) return;

                const auto [minmax_lat, minmax_lng] = std::pair{
                        std::minmax_element(begin, end, [](const auto& a, const auto& b) { return a.lat < b.lat; }),
                        std::minmax_element(begin, end, [](const auto& a, const auto& b) { return a.lng < b.lng; })
                };

                min_lat_ = (*minmax_lat.first).lat;
                max_lat_ = (*minmax_lat.second).lat;
                min_lng_ = (*minmax_lng.first).lng;
                max_lng_ = (*minmax_lng.second).lng;

                const double width_zoom  = (max_lng_ - min_lng_) == 0 ? 0.0 : (width  - 2*padding_) / (max_lng_ - min_lng_);
                const double height_zoom = (max_lat_ - min_lat_) == 0 ? 0.0 : (height - 2*padding_) / (max_lat_ - min_lat_);

                if (width_zoom == 0.0 && height_zoom == 0.0) {
                    zoom_coeff_ = 0.0;
                } else if (width_zoom == 0.0) {
                    zoom_coeff_ = height_zoom;
                } else if (height_zoom == 0.0) {
                    zoom_coeff_ = width_zoom;
                } else {
                    zoom_coeff_ = std::min(width_zoom, height_zoom);
                }
            }

            svg::Point operator()(geo::Coordinates coords) const {
                const double x = (max_lng_ - min_lng_) == 0 ? (padding_) :
                                 (coords.lng - min_lng_) * zoom_coeff_ + padding_;
                const double y = (max_lat_ - min_lat_) == 0 ? (padding_) :
                                 (max_lat_ - coords.lat) * zoom_coeff_ + padding_;
                return {x, y};
            }

        private:
            double padding_ = 0.0;
            double min_lat_ = 0.0, max_lat_ = 0.0;
            double min_lng_ = 0.0, max_lng_ = 0.0;
            double zoom_coeff_ = 0.0;
        };

    } // namespace detail

    class MapRenderer {
    public:
        MapRenderer() = default;

        void SetSettings(RenderSettings s) { settings_ = std::move(s); }

        [[nodiscard]] svg::Document Render(const transport_catalogue::TransportCatalogue& db) const;

    private:
        RenderSettings settings_;

        [[nodiscard]] const svg::Color& ColorForIndex(size_t i) const;
        [[nodiscard]] svg::Text MakeBusTextUnderlayer(svg::Point p, std::string_view name) const;
        [[nodiscard]] svg::Text MakeBusText(svg::Point p, std::string_view name, const svg::Color& color) const;
        [[nodiscard]] svg::Text MakeStopTextUnderlayer(svg::Point p, std::string_view name) const;
        [[nodiscard]] svg::Text MakeStopText(svg::Point p, std::string_view name) const;

    private:
        void RenderBusLines(svg::Document& doc,
                            const std::vector<const Bus*>& buses,
                            const detail::SphereProjector& proj,
                            std::vector<size_t>& bus_color_index) const;

        void RenderBusLabels(svg::Document& doc,
                             const std::vector<const Bus*>& buses,
                             const detail::SphereProjector& proj,
                             const std::vector<size_t>& bus_color_index) const;

        void RenderStopCircles(svg::Document& doc,
                               const std::vector<const Stop*>& stops,
                               const detail::SphereProjector& proj) const;

        void RenderStopLabels(svg::Document& doc,
                              const std::vector<const Stop*>& stops,
                              const detail::SphereProjector& proj) const;

    };

} // namespace transport_catalogue::renderer
