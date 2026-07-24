cmake_minimum_required(VERSION 3.20)

project(spotfetch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(spotfetch
    src/main.cpp
    src/renderer.cpp
    src/mock_data.cpp
    src/http_client.cpp
    src/spotify_client.cpp
    src/pkce.cpp
    src/auth.cpp
    src/callback_server.cpp
    src/token_store.cpp
)

target_include_directories(
    spotfetch
    PRIVATE
    include
)

target_link_libraries(
    spotfetch
    PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    OpenSSL::Crypto
)