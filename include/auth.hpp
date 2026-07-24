#pragma once

#include <string>

struct SpotifyTokenResponse {
    std::string access_token;
    std::string refresh_token;
    std::string token_type;
    std::string scope;
    int expires_in;
};

std::string build_authorization_url(
    const std::string& client_id,
    const std::string& redirect_uri,
    const std::string& code_challenge
);

SpotifyTokenResponse exchange_authorization_code(
    const std::string& client_id,
    const std::string& authorization_code,
    const std::string& redirect_uri,
    const std::string& code_verifier
);

SpotifyTokenResponse refresh_access_token(
    const std::string& client_id,
    const std::string& refresh_token
);