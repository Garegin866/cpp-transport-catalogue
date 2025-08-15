#include <iostream>
#include <string>
#include <sstream>

#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace std;
using namespace transport_catalogue;

int main() {
    using namespace json;

    Document doc = Load(cin);

    TransportCatalogue catalogue;
    JsonReader reader(doc, catalogue);
    reader.ProcessBaseRequests();

    renderer::MapRenderer renderer;
    reader.ReadRenderSettings(renderer);

    RequestHandler handler(catalogue, renderer);

    Array responses = reader.ProcessStatRequests(handler);
    Document response_doc{Node{responses}};
    Print(response_doc, cout);
}
