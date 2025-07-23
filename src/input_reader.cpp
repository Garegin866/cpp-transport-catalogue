#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>

namespace transport_catalogue::input {

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
    transport_catalogue::geo::Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return {nan, nan};
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));

        return {lat, lng};
    }

/**
 * Удаляет пробелы в начале и конце строки
 */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }

    CommandDescription ParseCommandDescription(std::string_view line) {
        auto colon_pos = line.find(':');
        if (colon_pos == line.npos) {
            return {};
        }

        auto space_pos = line.find(' ');
        if (space_pos >= colon_pos) {
            return {};
        }

        auto not_space = line.find_first_not_of(' ', space_pos);
        if (not_space >= colon_pos) {
            return {};
        }

        return {std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1))};
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    struct DeferredDistance {
        std::string_view from;
        std::string_view to;
        double distance;
    };

    std::vector<DeferredDistance> ParseDistances(std::string_view from_stop, std::string_view description) {
        std::vector<DeferredDistance> result;

        while (!description.empty()) {
            auto m_pos = description.find("m to ");
            if (m_pos == std::string_view::npos) {
                break;
            }

            size_t dist_start = description.find_last_of(", ", m_pos);
            if (dist_start == std::string_view::npos || description[dist_start] == ',') {
                dist_start = 0;
            } else {
                dist_start += 1;
            }

            const double distance = std::stod(std::string(Trim(description.substr(dist_start, m_pos - dist_start))));

            std::string_view after_m_to = description.substr(m_pos + 5);
            size_t next_comma = after_m_to.find(',');

            std::string_view to_stop = Trim(after_m_to.substr(0, next_comma));
            result.push_back({from_stop, to_stop, distance});

            if (next_comma == std::string_view::npos) break;
            description = after_m_to.substr(next_comma + 1);
        }

        return result;
    }

    void InputReader::ApplyCommands(TransportCatalogue &catalogue) const {
        std::vector<DeferredDistance> deferred_distances;

        for (const auto &cmd: commands_) {
            if (cmd.command == "Stop") {
                auto coordinates = ParseCoordinates(cmd.description);
                catalogue.AddStop(cmd.id, coordinates);

                std::string_view desc_view = cmd.description;
                auto second_comma = cmd.description.find(',', cmd.description.find(',') + 1);
                std::string_view distances_part = desc_view.substr(second_comma + 1);

                if (second_comma != std::string::npos) {
                    auto distances = ParseDistances(cmd.id, distances_part);
                    deferred_distances.insert(deferred_distances.end(), distances.begin(), distances.end());
                }
            }
        }

        for (const auto &cmd: commands_) {
            if (cmd.command == "Bus") {
                bool is_roundtrip = cmd.description.find('>') != std::string::npos;
                auto stops = ParseRoute(cmd.description);
                std::vector<const Stop *> stop_ptrs;
                stop_ptrs.reserve(stops.size());
                for (const auto &stop_name: stops) {
                    const Stop *stop = catalogue.FindStop(stop_name);
                    if (stop) {
                        stop_ptrs.push_back(stop);
                    } else {
                        std::cerr << "Warning: Stop '" << stop_name << "' not found for bus '" << cmd.id << "'." << std::endl;
                    }
                }
                catalogue.AddBus(cmd.id, stop_ptrs, is_roundtrip);
            }
        }

        for (const auto &dist: deferred_distances) {
            const auto *from_stop = catalogue.FindStop(dist.from);
            const auto *to_stop = catalogue.FindStop(dist.to);
            if (from_stop && to_stop) {
                catalogue.SetDistance(from_stop, to_stop, dist.distance);
            } else {
                std::cerr << "Warning: Distance from '" << dist.from << "' to '" << dist.to
                          << "' could not be set due to missing stops." << std::endl;
            }
        }
    }

} // namespace transport_catalogue::input