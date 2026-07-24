#include "http_client.hpp"
#include <curl/curl.h>
#include <iostream>

static size_t write_callback(
    char* data,
    size_t size,
    size_t count,
    void* user_data
) {
    std::size_t total_bytes = size * count;

    auto* response = static_cast<std::string*>(user_data);
    response->append(data, total_bytes);

    return total_bytes;
}

std::string http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    long status_code = 0;

    if (!curl) {
        return "";
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode result = curl_easy_perform(curl);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "spotfetch/0.1");

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &status_code
    );

    std::cout << "HTTP status: " << status_code << '\n';

    curl_easy_cleanup(curl);

    if (result != CURLE_OK) {
        return "";
    }

    return response;
}