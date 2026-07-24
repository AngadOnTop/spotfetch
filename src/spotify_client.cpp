#include "spotify_client.hpp"
#include "http_client.hpp"

#include <utility>
#include <vector>

SpotifyClient::SpotifyClient(std::string token)
    : access_token_(std::move(token)) {
}

std::string SpotifyClient::get_profile() {
    std::vector<std::string> headers{
        "Authorization: Bearer " + access_token_
    };

    return http_get(
        "https://api.spotify.com/v1/me",
        headers
    );
}