#include "websocketclient.hpp"

#include <iostream>

WebSocketClient::WebSocketClient()
    : m_sslCtx{ssl::context::tlsv12_client},
      m_resolver{m_ioCtx},
      m_ws{m_ioCtx, m_sslCtx} {
    m_sslCtx.set_default_verify_paths();
}

bool WebSocketClient::connect(const std::string& host,
                              const uint16_t& port,
                              const std::string& path) {
    try {
        auto const results = m_resolver.resolve(host, std::to_string(port));  // resolve domain name
        auto ep = net::connect(beast::get_lowest_layer(m_ws), results);       // connect to IP address

        // set SNI hostname
        if (!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(), host.c_str()))
            throw beast::system_error(
                beast::error_code(
                    static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()),
                "Failed to set SNI Hostname");

        std::string hostPort = host + ":" + std::to_string(ep.port());
        m_ws.next_layer().handshake(ssl::stream_base::client);  // SSL handshake
        // set User-Agent field in WebSocket upgrade request
        m_ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-client-coro");
            }));

        m_ws.handshake(host, path);

    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool WebSocketClient::disconnect() {
    beast::error_code ec;
    m_ws.close(websocket::close_code::normal, ec);

    if (ec) {
        std::cerr << "Error: " << ec.message() << std::endl;
        return false;
    }

    return true;
}

bool WebSocketClient::send(const std::string& message) {
    std::lock_guard<std::mutex> _(m_sendMtx);
    beast::error_code ec;

    m_ws.write(net::buffer(message), ec);
    if (ec) {
        std::cerr << "Error: " << ec.message() << std::endl;
        return false;
    }

    return true;
}

bool WebSocketClient::recv(std::string& message) {
    std::lock_guard<std::mutex> _(m_recvMtx);
    beast::error_code ec;

    m_recvBuffer.clear();
    m_ws.read(m_recvBuffer, ec);
    if (ec) {
        std::cerr << "Error: " << ec.message() << std::endl;
        return false;
    }

    message = beast::buffers_to_string(m_recvBuffer.data());
    return true;
}
