#pragma once

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "boost/asio/connect.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/ssl/error.hpp"
#include "boost/asio/ssl/stream.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"
#include "boost/beast/ssl.hpp"
#include "boost/beast/version.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

class HTTPClient : public std::enable_shared_from_this<HTTPClient> {
public:
    HTTPClient();
    ~HTTPClient();

    bool Connect(const std::string& host, const uint16_t& port = 443);
    bool Disconnect();
    bool IsConnected() const;

    http::response<http::string_body> Get(
        const std::string& path = "/",
        const std::vector<std::pair<std::string, std::string>>& params = {},
        const std::vector<std::pair<std::string, std::string>>& headers = {},
        const std::chrono::seconds& timeout = std::chrono::seconds(5));

protected:
    net::io_context m_ioCtx;
    ssl::context m_sslCtx;
    tcp::resolver m_resolver;
    beast::ssl_stream<beast::tcp_stream> m_stream;

    std::mutex m_ioMtx;
    std::atomic_bool m_connected;
    std::string m_host;
    uint16_t m_port;

    http::response<http::string_body> m_response;
    http::request<http::string_body> m_request;
    beast::flat_buffer m_recvBuffer;
};
