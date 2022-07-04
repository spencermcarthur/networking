#pragma once

#include <memory>
#include <mutex>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/beast.hpp"
#include "boost/beast/ssl.hpp"
#include "boost/beast/websocket.hpp"
#include "boost/beast/websocket/ssl.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

class WebSocketClient
    : public std::enable_shared_from_this<WebSocketClient> {
public:
    WebSocketClient();
    virtual ~WebSocketClient();

    bool connect(const std::string& host, const uint16_t& port = 443, const std::string& path = "/");
    bool disconnect();

    bool send(const std::string& message);
    bool recv(std::string& message);

    bool ping(const std::string& message);
    bool pong(const std::string& message);

private:
    net::io_context m_ioCtx;
    ssl::context m_sslCtx;
    tcp::resolver m_resolver;
    websocket::stream<beast::ssl_stream<tcp::socket>> m_ws;

    std::mutex m_ioMtx;

    boost::beast::error_code m_sendErrCode;
    boost::beast::error_code m_recvErrCode;
    boost::beast::error_code m_pingErrCode;
    boost::beast::error_code m_pongErrCode;

    beast::flat_buffer m_recvBuffer;
};
