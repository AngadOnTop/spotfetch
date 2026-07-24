#include "token_store.hpp"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

std::filesystem::path get_auth_path() {
    const char* home = std::getenv("HOME");

    if (home == nullptr) {
        throw std::runtime_error(
            "Could not locate the home directory"
        );
    }

    return std::filesystem::path(home)
        / ".config"
        / "spotfetch"
        / "auth.json";
}

} // namespace

std::optional<StoredAuth> load_stored_auth() {
    const std::filesystem::path path =
        get_auth_path();

    if (!std::filesystem::exists(path)) {
        return std::nullopt;
    }

    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error(
            "Failed to open saved authentication"
        );
    }

    try {
        nlohmann::json data;
        file >> data;

        StoredAuth auth{
            data.at("refresh_token")
                .get<std::string>()
        };

        if (auth.refresh_token.empty()) {
            return std::nullopt;
        }

        return auth;
    } catch (const nlohmann::json::exception& error) {
        throw std::runtime_error(
            std::string("Invalid saved authentication: ") +
            error.what()
        );
    }
}

void save_stored_auth(
    const StoredAuth& auth
) {
    if (auth.refresh_token.empty()) {
        throw std::invalid_argument(
            "Cannot save an empty refresh token"
        );
    }

    const std::filesystem::path path =
        get_auth_path();

    std::filesystem::create_directories(
        path.parent_path()
    );

    const nlohmann::json data{
        {"refresh_token", auth.refresh_token}
    };

    std::ofstream file(
        path,
        std::ios::trunc
    );

    if (!file) {
        throw std::runtime_error(
            "Failed to save authentication"
        );
    }

    file << data.dump(2);
    file.close();

    // Only the current user can read or write the file.
    std::filesystem::permissions(
        path,
        std::filesystem::perms::owner_read |
            std::filesystem::perms::owner_write,
        std::filesystem::perm_options::replace
    );
}

void clear_stored_auth() {
    std::filesystem::remove(
        get_auth_path()
    );
}