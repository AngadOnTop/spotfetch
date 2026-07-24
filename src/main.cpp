#include "auth.hpp"
#include "callback_server.hpp"
#include "mock_data.hpp"
#include "pkce.hpp"
#include "renderer.hpp"
#include "spotify_client.hpp"
#include "token_store.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

SpotifyTokenResponse perform_browser_login(
    const std::string& client_id,
    const std::string& redirect_uri
) {
    const std::string verifier =
        generate_code_verifier();

    const std::string challenge =
        generate_code_challenge(verifier);

    const std::string authorization_url =
        build_authorization_url(
            client_id,
            redirect_uri,
            challenge
        );

    const std::string open_command =
        "open \"" + authorization_url + "\"";

    std::cout << "Opening Spotify login...\n";

    if (std::system(open_command.c_str()) != 0) {
        throw std::runtime_error(
            "Failed to open Spotify login page"
        );
    }

    const std::string authorization_code =
        wait_for_authorization_code();

    std::cout << "Authorization code received\n";

    SpotifyTokenResponse tokens =
        exchange_authorization_code(
            client_id,
            authorization_code,
            redirect_uri,
            verifier
        );

    if (tokens.refresh_token.empty()) {
        throw std::runtime_error(
            "Spotify did not return a refresh token"
        );
    }

    save_stored_auth(
        StoredAuth{
            tokens.refresh_token
        }
    );

    std::cout << "Spotify login saved\n";

    return tokens;
}

std::chrono::steady_clock::time_point calculate_refresh_time(
    int expires_in
) {
    const int refresh_after =
        std::max(1, expires_in - 60);

    return std::chrono::steady_clock::now() +
        std::chrono::seconds(refresh_after);
}

} // namespace

int main() {
    try {
        const char* client_id_environment =
            std::getenv("SPOTIFY_CLIENT_ID");

        if (client_id_environment == nullptr) {
            std::cerr
                << "SPOTIFY_CLIENT_ID is not set\n";

            return 1;
        }

        const std::string client_id =
            client_id_environment;

        const std::string redirect_uri =
            "http://127.0.0.1:8888/callback";

        SpotifyTokenResponse tokens;

        const std::optional<StoredAuth> stored_auth =
            load_stored_auth();

        if (stored_auth.has_value()) {
            try {
                std::cout
                    << "Restoring Spotify login...\n";

                tokens = refresh_access_token(
                    client_id,
                    stored_auth->refresh_token
                );

                save_stored_auth(
                    StoredAuth{
                        tokens.refresh_token
                    }
                );

                std::cout
                    << "Spotify login restored\n";
            } catch (const std::exception& error) {
                std::cerr
                    << "Saved login could not be restored: "
                    << error.what()
                    << '\n';

                clear_stored_auth();

                tokens = perform_browser_login(
                    client_id,
                    redirect_uri
                );
            }
        } else {
            tokens = perform_browser_login(
                client_id,
                redirect_uri
            );
        }

        SpotifyClient spotify(
            tokens.access_token
        );

        auto token_refresh_time =
            calculate_refresh_time(
                tokens.expires_in
            );

        const UserProfile profile =
            spotify.get_profile();

        SpotifyStats stats =
            create_mock_stats();

        stats.username =
            profile.display_name;

        stats.top_tracks =
            spotify.get_top_tracks(5);

        constexpr int api_refresh_interval = 5;

        int seconds_since_api_refresh =
            api_refresh_interval;

        while (true) {
            // Refresh the access token shortly before it expires.
            if (
                std::chrono::steady_clock::now() >=
                token_refresh_time
            ) {
                tokens = refresh_access_token(
                    client_id,
                    tokens.refresh_token
                );

                spotify.set_access_token(
                    tokens.access_token
                );

                save_stored_auth(
                    StoredAuth{
                        tokens.refresh_token
                    }
                );

                token_refresh_time =
                    calculate_refresh_time(
                        tokens.expires_in
                    );
            }

            if (
                seconds_since_api_refresh >=
                api_refresh_interval
            ) {
                stats.current_track =
                    spotify.get_current_track();

                seconds_since_api_refresh = 0;
            } else if (
                stats.current_track.is_playing &&
                stats.current_track.progress_seconds <
                    stats.current_track.duration_seconds
            ) {
                stats.current_track.progress_seconds++;
            }

            std::system("clear");

            render_dashboard(stats);

            std::cout
                << "\nSpotify ID: "
                << profile.id
                << '\n';

            std::cout
                << "Followers: "
                << profile.followers
                << '\n';

            std::cout
                << "\nPress Ctrl+C to exit.\n";

            std::cout.flush();

            std::this_thread::sleep_for(
                std::chrono::seconds(1)
            );

            seconds_since_api_refresh++;
        }
    } catch (const std::exception& error) {
        std::cerr
            << "Error: "
            << error.what()
            << '\n';

        return 1;
    }
}