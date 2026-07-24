#include "auth.hpp"
#include "http_client.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>

namespace {

std::string url_encode(
    CURL* curl,
    const std::string& value
) {
    char* encoded = curl_easy_escape(
        curl,
        value.c_str(),
        static_cast<int>(value.size())
    );

    if (encoded == nullptr) {
        throw std::runtime_error(
            "Failed to URL-encode value"
        );
    }

    std::string result(encoded);
    curl_free(encoded);

    return result;
}

} // namespace

std::string build_authorization_url(
    const std::string& client_id,
    const std::string& redirect_uri,
    const std::string& code_challenge
) {
    CURL* curl = curl_easy_init();

    if (curl == nullptr) {
        throw std::runtime_error(
            "Failed to initialize libcurl"
        );
    }

    try {
        const std::string scopes =
            "user-read-private "
            "user-read-email "
            "user-top-read "
            "user-read-currently-playing";

        const std::string url =
            "https://accounts.spotify.com/authorize"
            "?client_id=" +
            url_encode(curl, client_id) +

            "&response_type=code"

            "&redirect_uri=" +
            url_encode(curl, redirect_uri) +

            "&scope=" +
            url_encode(curl, scopes) +

            "&code_challenge_method=S256"

            "&code_challenge=" +
            url_encode(curl, code_challenge);

        curl_easy_cleanup(curl);

        return url;
    } catch (...) {
        curl_easy_cleanup(curl);
        throw;
    }
}

SpotifyTokenResponse exchange_authorization_code(
    const std::string& client_id,
    const std::string& authorization_code,
    const std::string& redirect_uri,
    const std::string& code_verifier
) {
    const std::string response =
        http_post_form(
            "https://accounts.spotify.com/api/token",
            {
                {
                    "client_id",
                    client_id
                },
                {
                    "grant_type",
                    "authorization_code"
                },
                {
                    "code",
                    authorization_code
                },
                {
                    "redirect_uri",
                    redirect_uri
                },
                {
                    "code_verifier",
                    code_verifier
                }
            }
        );

    try {
        const nlohmann::json data =
            nlohmann::json::parse(response);

        return SpotifyTokenResponse{
            data.at("access_token")
                .get<std::string>(),

            data.value(
                "refresh_token",
                std::string{}
            ),

            data.value(
                "token_type",
                std::string{"Bearer"}
            ),

            data.value(
                "scope",
                std::string{}
            ),

            data.at("expires_in")
                .get<int>()
        };
    } catch (
        const nlohmann::json::exception& error
    ) {
        throw std::runtime_error(
            std::string(
                "Invalid Spotify token response: "
            ) +
            error.what()
        );
    }
}

SpotifyTokenResponse refresh_access_token(
    const std::string& client_id,
    const std::string& refresh_token
) {
    const std::string response =
        http_post_form(
            "https://accounts.spotify.com/api/token",
            {
                {
                    "grant_type",
                    "refresh_token"
                },
                {
                    "refresh_token",
                    refresh_token
                },
                {
                    "client_id",
                    client_id
                }
            }
        );

    try {
        const nlohmann::json data =
            nlohmann::json::parse(response);

        return SpotifyTokenResponse{
            data.at("access_token")
                .get<std::string>(),

            // Spotify may not issue a new refresh token.
            data.value(
                "refresh_token",
                refresh_token
            ),

            data.value(
                "token_type",
                std::string{"Bearer"}
            ),

            data.value(
                "scope",
                std::string{}
            ),

            data.at("expires_in")
                .get<int>()
        };
    } catch (
        const nlohmann::json::exception& error
    ) {
        throw std::runtime_error(
            std::string(
                "Invalid token refresh response: "
            ) +
            error.what()
        );
    }
}