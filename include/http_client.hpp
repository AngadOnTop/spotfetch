#pragma once

#include <string>
#include <vector>

std::string http_get(
    const std::string& url,
    const std::vector<std::string>& headers

);