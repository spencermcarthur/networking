#pragma once

#include <memory>
#include <mutex>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/beast.hpp"
#include "boost/beast/ssl.hpp"
#include "boost/beast/websocket.hpp"
#include "boost/beast/websocket/ssl.hpp"

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
namespace net = boost::asio;             // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;        // from <boost/asio/ssl.hpp>
namespace http = beast::http;            // from <boost/beast/http.hpp>
using tcp = boost::asio::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

class WebSocketClient
    : public std::enable_shared_from_this<WebSocketClient> {
public:
    WebSocketClient();

    bool connect(const std::string& host, const uint16_t& port = 443, const std::string& path = "/");
    bool disconnect();

    bool send(const std::string& message);
    bool recv(std::string& message);

private:
    net::io_context m_ioCtx;
    ssl::context m_sslCtx;
    tcp::resolver m_resolver;
    websocket::stream<beast::ssl_stream<tcp::socket>> m_ws;

    std::mutex m_sendMtx;
    std::mutex m_recvMtx;
    beast::flat_buffer m_recvBuffer;
};
