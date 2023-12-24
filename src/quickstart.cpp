#include <iostream>
#include "../include/simdjson.h"
#include "../include/Server.h"

using namespace simdjson;
using namespace std;
int main(void)
{
    ondemand::parser parser;
    auto cars_json = R"( { "test":[ { "val1":1, "val2":2 }, { "val1":1, "val2":2 } ] }   )"_padded;
    auto doc = parser.iterate(cars_json);
    auto test_array = doc.find_field("test").get_array();
    size_t count = test_array.count_elements(); // requires simdjson 1.0 or better
    std::cout << "Number of elements: " << count << std::endl;
    for (ondemand::object elem : test_array)
    {
        std::cout << simdjson::to_json_string(elem);
    }


    Server server = Server();
    server.Start();

    server.Stop();
    
}