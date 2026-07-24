#pragma once

#include <cstddef>
#include <string>

#include <openssl/evp.h>

#include <array>
#include <string>

std::string generate_code_verifier(std::size_t length = 64);

std::string generate_code_challenge(
    const std::string& verifier
);