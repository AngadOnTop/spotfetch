#include "pkce.hpp"

#include <random>
#include <stdexcept>
#include <string>
#include <array>

std::string generate_code_verifier(std::size_t length) {
    if (length < 43 || length > 128) {
        throw std::invalid_argument(
            "PKCE verifier length must be between 43 and 128"
        );
    }

    const std::string allowed_characters =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "-._~";

    std::random_device random_device;

    std::uniform_int_distribution<std::size_t> distribution(
        0,
        allowed_characters.size() - 1
    );

    std::string verifier;
    verifier.reserve(length);

    for (std::size_t i = 0; i < length; i++) {
        verifier += allowed_characters[distribution(random_device)];
    }

    return verifier;
}

static std::array<unsigned char, 32> sha256(
    const std::string& input
) {
    std::array<unsigned char, 32> hash{};
    unsigned int hash_length = 0;

    EVP_MD_CTX* context = EVP_MD_CTX_new();

    if (context == nullptr) {
        throw std::runtime_error("Failed to create digest context");
    }

    bool success =
        EVP_DigestInit_ex(context, EVP_sha256(), nullptr) == 1 &&
        EVP_DigestUpdate(
            context,
            input.data(),
            input.size()
        ) == 1 &&
        EVP_DigestFinal_ex(
            context,
            hash.data(),
            &hash_length
        ) == 1;

    EVP_MD_CTX_free(context);

    if (!success || hash_length != hash.size()) {
        throw std::runtime_error("Failed to generate SHA-256 hash");
    }

    return hash;
}

static std::string base64_url_encode(
    const unsigned char* data,
    std::size_t length
) {
    // Base64 output needs roughly 4 characters per 3 input bytes,
    // plus one byte for OpenSSL's null terminator.
    std::size_t encoded_capacity = 4 * ((length + 2) / 3) + 1;

    std::string encoded(encoded_capacity, '\0');

    int encoded_length = EVP_EncodeBlock(
        reinterpret_cast<unsigned char*>(encoded.data()),
        data,
        static_cast<int>(length)
    );

    if (encoded_length < 0) {
        throw std::runtime_error("Failed to Base64 encode hash");
    }

    encoded.resize(static_cast<std::size_t>(encoded_length));

    for (char& character : encoded) {
        if (character == '+') {
            character = '-';
        } else if (character == '/') {
            character = '_';
        }
    }

    while (!encoded.empty() && encoded.back() == '=') {
        encoded.pop_back();
    }

    return encoded;
}

std::string generate_code_challenge(
    const std::string& verifier
) {
    std::array<unsigned char, 32> hash = sha256(verifier);

    return base64_url_encode(
        hash.data(),
        hash.size()
    );
}