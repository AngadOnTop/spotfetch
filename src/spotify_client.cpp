#include "spotify_client.hpp"
#include "http_client.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

SpotifyClient::SpotifyClient(std::string token)
    : access_token_(std::move(token)) {
}

UserProfile SpotifyClient::get_profile() {
    const std::vector<std::string> headers{
        "Authorization: Bearer " + access_token_
    };

    const std::string response = http_get(
        "https://api.spotify.com/v1/me",
        headers
    );

    try {
        const nlohmann::json data =
            nlohmann::json::parse(response);

        UserProfile profile{};

        profile.id =
            data.at("id").get<std::string>();

        if (
            data.contains("display_name") &&
            data["display_name"].is_string()
        ) {
            profile.display_name =
                data["display_name"].get<std::string>();
        } else {
            profile.display_name = profile.id;
        }

        if (
            data.contains("followers") &&
            data["followers"].is_object() &&
            data["followers"].contains("total") &&
            data["followers"]["total"].is_number_integer()
        ) {
            profile.followers =
                data["followers"]["total"].get<int>();
        }

        return profile;
    } catch (const nlohmann::json::exception& error) {
        throw std::runtime_error(
            std::string(
                "Invalid Spotify profile response: "
            ) +
            error.what()
        );
    }
}

std::vector<Track> SpotifyClient::get_top_tracks(
    std::size_t limit
) {
    if (limit < 1 || limit > 50) {
        throw std::invalid_argument(
            "Top-tracks limit must be between 1 and 50"
        );
    }

    const std::vector<std::string> headers{
        "Authorization: Bearer " + access_token_
    };

    const std::string url =
        "https://api.spotify.com/v1/me/top/tracks"
        "?time_range=medium_term"
        "&limit=" +
        std::to_string(limit);

    const std::string response =
        http_get(url, headers);

    try {
        const nlohmann::json data =
            nlohmann::json::parse(response);

        std::vector<Track> tracks;
        tracks.reserve(limit);

        for (const auto& item : data.at("items")) {
            std::string artist = "Unknown artist";

            if (
                item.contains("artists") &&
                item["artists"].is_array() &&
                !item["artists"].empty()
            ) {
                artist =
                    item["artists"]
                        .at(0)
                        .at("name")
                        .get<std::string>();
            }

            tracks.push_back(
                Track{
                    item.at("name").get<std::string>(),
                    artist,
                    0,
                    0,
                    item.at("duration_ms").get<int>() / 1000,
                    false
                }
            );
        }

        return tracks;
    } catch (const nlohmann::json::exception& error) {
        throw std::runtime_error(
            std::string(
                "Invalid Spotify top-tracks response: "
            ) +
            error.what()
        );
    }
}

Track SpotifyClient::get_current_track() {
    const std::vector<std::string> headers{
        "Authorization: Bearer " + access_token_
    };

    const std::string response = http_get(
        "https://api.spotify.com/v1/me/player/currently-playing",
        headers
    );

    // Spotify can return 204 No Content when nothing is playing.
    if (response.empty()) {
        return Track{
            "Nothing playing",
            "Spotify",
            0,
            0,
            0,
            false
        };
    }

    try {
        const nlohmann::json data =
            nlohmann::json::parse(response);

        if (
            !data.contains("item") ||
            data["item"].is_null()
        ) {
            return Track{
                "Nothing playing",
                "Spotify",
                0,
                0,
                0,
                false
            };
        }

        const nlohmann::json& item = data.at("item");

        std::string creator = "Unknown artist";

        // Normal music track
        if (
            item.contains("artists") &&
            item["artists"].is_array() &&
            !item["artists"].empty()
        ) {
            creator =
                item["artists"]
                    .at(0)
                    .at("name")
                    .get<std::string>();
        }
        // Podcast episode
        else if (
            item.contains("show") &&
            item["show"].is_object() &&
            item["show"].contains("name")
        ) {
            creator =
                item["show"]
                    .at("name")
                    .get<std::string>();
        }

        int progress_seconds = 0;

        if (
            data.contains("progress_ms") &&
            data["progress_ms"].is_number_integer()
        ) {
            progress_seconds =
                data["progress_ms"].get<int>() / 1000;
        }

        int duration_seconds = 0;

        if (
            item.contains("duration_ms") &&
            item["duration_ms"].is_number_integer()
        ) {
            duration_seconds =
                item["duration_ms"].get<int>() / 1000;
        }

        const bool is_playing =
            data.value("is_playing", false);

        return Track{
            item.at("name").get<std::string>(),
            creator,
            0,
            progress_seconds,
            duration_seconds,
            is_playing
        };
    } catch (const nlohmann::json::exception& error) {
        throw std::runtime_error(
            std::string(
                "Invalid currently-playing response: "
            ) +
            error.what()
        );
    }
}

void SpotifyClient::set_access_token(std::string token) {
    access_token_ = std::move(token);
}