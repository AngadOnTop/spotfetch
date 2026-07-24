#include "callback_server.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>
#include <string>

namespace {

std::string extract_query_value(
    const std::string& request_target,
    const std::string& key
) {
    const std::string search = key + "=";

    std::size_t start = request_target.find(search);

    if (start == std::string::npos) {
        return "";
    }

    start += search.size();

    std::size_t end = request_target.find('&', start);

    if (end == std::string::npos) {
        end = request_target.size();
    }

    return request_target.substr(start, end - start);
}

void send_browser_response(
    int client_socket,
    const std::string& status,
    const std::string& message
) {
    const std::string body =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<title>Spotfetch</title>"
        "</head>"
        "<body>"
        "<h1>" + message + "</h1>"
        "<p>You can close this browser tab.</p>"
        "</body>"
        "</html>";

    const std::string response =
        "HTTP/1.1 " + status + "\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        body;

    send(
        client_socket,
        response.c_str(),
        response.size(),
        0
    );
}

} // namespace

std::string wait_for_authorization_code(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        throw std::runtime_error(
            "Failed to create callback socket"
        );
    }

    int reuse_address = 1;

    setsockopt(
        server_socket,
        SOL_SOCKET,
        SO_REUSEADDR,
        &reuse_address,
        sizeof(reuse_address)
    );

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);

    if (
        bind(
            server_socket,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)
        ) == -1
    ) {
        close(server_socket);

        throw std::runtime_error(
            "Failed to bind callback socket"
        );
    }

    if (listen(server_socket, 1) == -1) {
        close(server_socket);

        throw std::runtime_error(
            "Failed to listen on callback socket"
        );
    }

    int client_socket = accept(
        server_socket,
        nullptr,
        nullptr
    );

    if (client_socket == -1) {
        close(server_socket);

        throw std::runtime_error(
            "Failed to accept callback connection"
        );
    }

    char buffer[4096]{};

    ssize_t bytes_received = recv(
        client_socket,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (bytes_received <= 0) {
        close(client_socket);
        close(server_socket);

        throw std::runtime_error(
            "Failed to read callback request"
        );
    }

    std::string request(
        buffer,
        static_cast<std::size_t>(bytes_received)
    );

    std::size_t first_space = request.find(' ');
    std::size_t second_space = request.find(
        ' ',
        first_space + 1
    );

    if (
        first_space == std::string::npos ||
        second_space == std::string::npos
    ) {
        send_browser_response(
            client_socket,
            "400 Bad Request",
            "Invalid callback request"
        );

        close(client_socket);
        close(server_socket);

        throw std::runtime_error(
            "Invalid HTTP callback request"
        );
    }

    std::string request_target = request.substr(
        first_space + 1,
        second_space - first_space - 1
    );

    std::string error = extract_query_value(
        request_target,
        "error"
    );

    if (!error.empty()) {
        send_browser_response(
            client_socket,
            "400 Bad Request",
            "Spotify authorization failed"
        );

        close(client_socket);
        close(server_socket);

        throw std::runtime_error(
            "Spotify authorization error: " + error
        );
    }

    std::string authorization_code =
        extract_query_value(
            request_target,
            "code"
        );

    if (authorization_code.empty()) {
        send_browser_response(
            client_socket,
            "400 Bad Request",
            "Authorization code missing"
        );

        close(client_socket);
        close(server_socket);

        throw std::runtime_error(
            "Spotify callback did not contain a code"
        );
    }

    send_browser_response(
        client_socket,
        "200 OK",
        "Spotfetch login successful"
    );

    close(client_socket);
    close(server_socket);

    return authorization_code;
}