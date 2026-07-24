#pragma once

#include <optional>
#include <string>

struct StoredAuth {
    std::string refresh_token;
};

std::optional<StoredAuth> load_stored_auth();

void save_stored_auth(
    const StoredAuth& auth
);

void clear_stored_auth();