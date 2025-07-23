#include "transport_catalogue.h"
#include "stat_reader.h"
#include "input_reader.h"
#include "helpers.h"

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

void Process(std::istream& input, std::ostream& output) {
    using namespace transport_catalogue;
    using namespace transport_catalogue::input;
    using namespace transport_catalogue::stat;

    TransportCatalogue catalogue;

    int base_request_count;
    input >> base_request_count >> std::ws;

    InputReader reader;
    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        reader.ParseLine(line);
    }
    reader.ApplyCommands(catalogue);

    int stat_request_count;
    input >> stat_request_count >> std::ws;

    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}

void ProcessInputAndPrintStat(const std::string& input_file, const std::string& output_file) {
    using namespace std;

    ifstream input(input_file);
    ASSERT_TRUE(input.is_open()) << "Input file not found";

    ostringstream output;

    Process(input, output);

    ifstream expected(output_file);
    ASSERT_TRUE(expected.is_open()) << "Expected output file not found";

    stringstream expected_ss;
    expected_ss << expected.rdbuf();

    EXPECT_EQ(transport_catalogue::details::normalize_line_endings(output.str()), transport_catalogue::details::normalize_line_endings(expected_ss.str()))
                        << "Output does not match expected output";
}

TEST(TransportCatalogueIntegration, ParseAndPrintStat_BusAndStop) {
    using namespace transport_catalogue;
    using namespace transport_catalogue::stat;

    TransportCatalogue catalogue;

    catalogue.AddStop("A", {0, 0});
    catalogue.AddStop("B", {3, 4});

    const Stop* stop_a = catalogue.FindStop("A");
    const Stop* stop_b = catalogue.FindStop("B");

    ASSERT_NE(stop_a, nullptr);
    ASSERT_NE(stop_b, nullptr);

    catalogue.AddBus("Bus1", {stop_a, stop_b, stop_a}, true);

    std::ostringstream output;

    ParseAndPrintStat(catalogue, "Bus Bus1", output);
    std::string bus_output = output.str();

    output.str("");
    output.clear();

    ParseAndPrintStat(catalogue, "Stop A", output);
    std::string stop_output = output.str();
    EXPECT_NE(stop_output.find("Stop A:"), std::string::npos);
    EXPECT_NE(stop_output.find("buses Bus1"), std::string::npos);

    output.str("");
    output.clear();

    ParseAndPrintStat(catalogue, "Stop UnknownStop", output);
    EXPECT_EQ(output.str(), "Stop UnknownStop: not found\n");
}

TEST(TransportCatalogueIntegrationTest, ProcessInputAndPrintStat_Case1) {
    ProcessInputAndPrintStat("data/tsA_case1_input.txt", "data/tsA_case1_output.txt");
}

TEST(TransportCatalogueIntegrationTest, ProcessInputAndPrintStat_Case2) {
    ProcessInputAndPrintStat("data/tsA_case2_input.txt", "data/tsA_case2_output.txt");
}

TEST(TransportCatalogueIntegrationTest, ProcessInputAndPrintStat_Case3) {
    ProcessInputAndPrintStat("data/tsB_case1_input.txt", "data/tsB_case1_output.txt");
}

TEST(TransportCatalogueIntegrationTest, ProcessInputAndPrintStat_Case4) {
    ProcessInputAndPrintStat("data/tsB_case2_input.txt", "data/tsB_case2_output.txt");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
