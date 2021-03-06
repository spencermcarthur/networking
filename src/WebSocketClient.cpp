#include "networking/WebSocketClient.hpp"

#include <iostream>
#include <mutex>

WebSocketClient::WebSocketClient()
    : m_sslCtx{ssl::context::tlsv12_client},
      m_resolver{m_ioCtx},
      m_ws{m_ioCtx, m_sslCtx} {
    m_sslCtx.set_default_verify_paths();
}

WebSocketClient::~WebSocketClient() {
    if (m_ws.is_open()) Disconnect();
}

bool WebSocketClient::Connect(const std::string& host, const uint16_t& port, const std::string& path) {
    boost::beast::error_code ec;

    try {
        auto const results = m_resolver.resolve(host, std::to_string(port), ec);  // resolve domain name
        if (ec) {
#ifdef DEBUG
            std::cerr << "Error resolving domain name: " << ec.message() << std::endl;
#endif
            return false;
        }

        auto ep = net::connect(beast::get_lowest_layer(m_ws), results, ec);  // connect to IP address
        if (ec) {
#ifdef DEBUG
            std::cerr << "Error connecting to resolved address: " << ec.message() << std::endl;
#endif
            return false;
        }

        // set SNI hostname
        if (!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(), host.c_str()))
            throw beast::system_error(
                beast::error_code(
                    static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()),
                "Failed to set SNI Hostname");

        std::string hostPort = host + ":" + std::to_string(ep.port());
        m_ws.next_layer().handshake(ssl::stream_base::client, ec);  // SSL handshake
        if (ec) {
#ifdef DEBUG
            std::cerr << "Error completing SSL handshake: " << ec.message() << std::endl;
#endif
            return false;
        }

        // set User-Agent field in WebSocket upgrade request
        m_ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-client-coro");
            }));

        m_ws.handshake(host, path, ec);  // WebSocket handshake
        if (ec) {
#ifdef DEBUG
            std::cerr << "Error completing WebSocket handshake: " << ec.message() << std::endl;
#endif
            return false;
        }

    } catch (std::exception const& e) {
#ifdef DEBUG
        std::cerr << "Error: " << e.what() << std::endl;
#endif
        return false;
    }

    return true;
}

bool WebSocketClient::Disconnect() {
    beast::error_code ec;
    std::scoped_lock _(m_sendMtx, m_recvMtx, m_pingMtx, m_pongMtx);
    m_ws.close(websocket::close_code::normal, ec);
    if (net::ssl::error::stream_truncated != ec && ec) {
#ifdef DEBUG
        std::cerr << "Error: " << ec.message() << std::endl;
#endif
        return false;
    }
    return true;
}

bool WebSocketClient::Send(const std::string& message) {
    std::lock_guard<std::mutex> _(m_sendMtx);
    if (!m_ws.is_open()) return false;
    m_sendErrCode.clear();
    m_ws.write(net::buffer(message), m_sendErrCode);
    if (m_sendErrCode) {
#ifdef DEBUG
        std::cerr << "Error: " << m_sendErrCode.message() << std::endl;
#endif
        return false;
    }
    return true;
}

bool WebSocketClient::Recv(std::string& message) {
    std::lock_guard<std::mutex> _(m_recvMtx);
    if (!m_ws.is_open()) return false;
    m_recvErrCode.clear();
    m_recvBuffer.clear();
    m_ws.read(m_recvBuffer, m_recvErrCode);
    if (m_recvErrCode) {
#ifdef DEBUG
        std::cerr << "Error: " << m_recvErrCode.message() << std::endl;
#endif
        return false;
    }
    message = beast::buffers_to_string(m_recvBuffer.data());
    return true;
}

bool WebSocketClient::Ping(const std::string& message) {
    std::lock_guard<std::mutex> _(m_pingMtx);
    if (!m_ws.is_open()) return false;
    m_pingErrCode.clear();
    m_ws.ping(websocket::ping_data(message), m_pingErrCode);
    if (m_pingErrCode) {
#ifdef DEBUG
        std::cerr << "Error pinging: " << m_pingErrCode.message() << std::endl;
#endif
        return false;
    }
    return true;
}

bool WebSocketClient::Pong(const std::string& message) {
    std::lock_guard<std::mutex> _(m_pongMtx);
    if (!m_ws.is_open()) return false;
    m_pongErrCode.clear();
    m_ws.pong(websocket::ping_data(message), m_pongErrCode);
    if (m_pongErrCode) {
#ifdef DEBUG
        std::cerr << "Error ponging: " << m_pongErrCode.message() << std::endl;
#endif
        return false;
    }
    return true;
}
