#pragma once

#include <string>
#include <utility>
#include <vector>

std::string http_get(
    const std::string& url,
    const std::vector<std::string>& headers = {}
);

std::string http_post_form(
    const std::string& url,
    const std::vector<std::pair<std::string, std::string>>& form_fields,
    const std::vector<std::string>& headers = {}
);