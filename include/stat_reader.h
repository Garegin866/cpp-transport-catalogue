#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"


namespace transport_catalogue::stat {

    void ParseAndPrintStat(const TransportCatalogue &tansport_catalogue, std::string_view request,
                           std::ostream &output);

    namespace details {

        void PrintBusStat(const TransportCatalogue &transport_catalogue, std::string_view request, std::ostream &output);
        void PrintStopStat(const TransportCatalogue &transport_catalogue, std::string_view request, std::ostream &output);

    } // namespace details

} // namespace transport_catalogue::stat

