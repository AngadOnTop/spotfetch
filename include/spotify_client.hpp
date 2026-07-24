#pragma once

#include <string>
class SpotifyClient {
public:
    explicit SpotifyClient(std::string token);

    std::string get_profile();

private:
    std::string access_token_;
};