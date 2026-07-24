#include "http_client.hpp"
#include "mock_data.hpp"
#include "renderer.hpp"
#include "spotify_client.hpp"

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

int main() {
    std::string response = http_get("https://postman-echo.com/get?project=spotfetch", {});
    
    try {
        nlohmann::json data = nlohmann::json::parse(response);
    
        std::string project = data.at("args").at("project");
        std::cout << project << '\n';
    }
    catch (const nlohmann::json::exception& error) {
        std::cerr << "JSON error: " << error.what() << '\n';
    }

    return 0;
}