#include "http_client.hpp"

#include <curl/curl.h>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

struct CurlHandleDeleter {
    void operator()(CURL* curl) const {
        curl_easy_cleanup(curl);
    }
};

struct CurlHeaderListDeleter {
    void operator()(curl_slist* headers) const {
        curl_slist_free_all(headers);
    }
};

using CurlHandle =
    std::unique_ptr<CURL, CurlHandleDeleter>;

using CurlHeaderList =
    std::unique_ptr<curl_slist, CurlHeaderListDeleter>;

std::size_t write_callback(
    char* data,
    std::size_t size,
    std::size_t count,
    void* user_data
) {
    const std::size_t total_bytes = size * count;

    auto* response =
        static_cast<std::string*>(user_data);

    response->append(data, total_bytes);

    return total_bytes;
}

CurlHeaderList build_header_list(
    const std::vector<std::string>& headers
) {
    curl_slist* list = nullptr;

    for (const std::string& header : headers) {
        curl_slist* updated_list =
            curl_slist_append(
                list,
                header.c_str()
            );

        if (updated_list == nullptr) {
            curl_slist_free_all(list);

            throw std::runtime_error(
                "Failed to create HTTP header list"
            );
        }

        list = updated_list;
    }

    return CurlHeaderList(list);
}

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
            "Failed to URL-encode form field"
        );
    }

    std::string result(encoded);
    curl_free(encoded);

    return result;
}

std::string perform_request(
    CURL* curl,
    curl_slist* header_list
) {
    std::string response;

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        write_callback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response
    );

    curl_easy_setopt(
        curl,
        CURLOPT_FOLLOWLOCATION,
        1L
    );

    curl_easy_setopt(
        curl,
        CURLOPT_USERAGENT,
        "spotfetch/0.1"
    );

    if (header_list != nullptr) {
        curl_easy_setopt(
            curl,
            CURLOPT_HTTPHEADER,
            header_list
        );
    }

    const CURLcode result =
        curl_easy_perform(curl);

    if (result != CURLE_OK) {
        throw std::runtime_error(
            std::string("HTTP request failed: ") +
            curl_easy_strerror(result)
        );
    }

    long status_code = 0;

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &status_code
    );

    if (status_code < 200 || status_code >= 300) {
        throw std::runtime_error(
            "HTTP request returned status " +
            std::to_string(status_code) +
            ": " +
            response
        );
    }

    return response;
}

} // namespace

std::string http_get(
    const std::string& url,
    const std::vector<std::string>& headers
) {
    CurlHandle curl(curl_easy_init());

    if (!curl) {
        throw std::runtime_error(
            "Failed to initialize libcurl"
        );
    }

    CurlHeaderList header_list =
        build_header_list(headers);

    curl_easy_setopt(
        curl.get(),
        CURLOPT_URL,
        url.c_str()
    );

    return perform_request(
        curl.get(),
        header_list.get()
    );
}

std::string http_post_form(
    const std::string& url,
    const std::vector<
        std::pair<std::string, std::string>
    >& form_fields,
    const std::vector<std::string>& headers
) {
    CurlHandle curl(curl_easy_init());

    if (!curl) {
        throw std::runtime_error(
            "Failed to initialize libcurl"
        );
    }

    std::string body;

    for (
        std::size_t i = 0;
        i < form_fields.size();
        ++i
    ) {
        if (i > 0) {
            body += '&';
        }

        body += url_encode(
            curl.get(),
            form_fields[i].first
        );

        body += '=';

        body += url_encode(
            curl.get(),
            form_fields[i].second
        );
    }

    std::vector<std::string> all_headers =
        headers;

    all_headers.push_back(
        "Content-Type: "
        "application/x-www-form-urlencoded"
    );

    CurlHeaderList header_list =
        build_header_list(all_headers);

    curl_easy_setopt(
        curl.get(),
        CURLOPT_URL,
        url.c_str()
    );

    curl_easy_setopt(
        curl.get(),
        CURLOPT_POST,
        1L
    );

    curl_easy_setopt(
        curl.get(),
        CURLOPT_POSTFIELDS,
        body.c_str()
    );

    curl_easy_setopt(
        curl.get(),
        CURLOPT_POSTFIELDSIZE,
        static_cast<long>(body.size())
    );

    return perform_request(
        curl.get(),
        header_list.get()
    );
}