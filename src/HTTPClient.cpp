#include "HTTPClient.hpp"

class HTTPError : public std::exception {
public:
    HTTPError(const std::string& message)
        : m_message{message} {}

    const char* what() const noexcept {
        return m_message.c_str();
    }

private:
    std::string m_message;
};

HTTPClient::HTTPClient()
    : m_ioCtx{},
      m_sslCtx{ssl::context::tlsv12_client},
      m_resolver{m_ioCtx},
      m_stream{m_ioCtx, m_sslCtx},
      m_connected{false} {
    m_sslCtx.set_default_verify_paths();
    m_sslCtx.set_verify_mode(ssl::verify_peer);
}

HTTPClient::~HTTPClient() {
    if (m_connected)
        Disconnect();
}

bool HTTPClient::Connect(const std::string& host, const uint16_t& port) {
    if (m_connected)
        Disconnect();

    std::lock_guard<std::mutex> lockIo(m_ioMtx);

    // replace stream object
    m_stream = beast::ssl_stream<beast::tcp_stream>{m_ioCtx, m_sslCtx};

    // set host/port
    m_host = host;
    m_port = port;

    // strip trailing forward slash
    if (m_host[m_host.size() - 1] == '/')
        m_host = m_host.substr(0, m_host.size() - 1);

    // strip scheme
    size_t i = m_host.find_first_of("://");
    if (i != std::string::npos) {
        if (m_host.substr(0, i) == "http")
            throw std::runtime_error("Non-SSL connection not supported");
        m_host = m_host.substr(i + 3);
    }

    // set SNI hostname
    if (!SSL_set_tlsext_host_name(m_stream.native_handle(), m_host.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }

    beast::error_code ec;

    // resolve domain name
    auto const results = m_resolver.resolve(m_host, std::to_string(port), ec);
    if (ec) {
#ifdef DEBUG
        std::cerr << "Error: " << ec.message() << std::endl;
#endif
        return false;
    }

    // connect to IP address
    beast::get_lowest_layer(m_stream).connect(results, ec);
    if (ec) {
#ifdef DEBUG
        std::cerr << "Error: " << ec.message() << std::endl;
#endif
        return false;
    }

    // perform SSL handshake
    m_stream.handshake(ssl::stream_base::client, ec);
    if (ec) {
#ifdef DEBUG
        std::cerr << "Error: " << ec.message() << std::endl;
#endif
        return false;
    }

    m_connected = true;
    return true;
}

bool HTTPClient::Disconnect() {
    if (!m_connected) return true;

    std::lock_guard<std::mutex> lockIo(m_ioMtx);

    beast::error_code ec;
    m_stream.shutdown(ec);
    if (ssl::error::stream_truncated != ec && ec) {
#ifdef DEBUG
        std::cerr << "Error: " << ec.message() << std::endl;
#endif
        return false;
    }

    m_connected = false;
    return true;
}

bool HTTPClient::IsConnected() const { return m_connected; }

http::response<http::string_body> HTTPClient::Get(const std::string& path,
                                                  const std::vector<std::pair<std::string, std::string>>& params,
                                                  const std::vector<std::pair<std::string, std::string>>& headers,
                                                  const std::chrono::seconds& timeout) {
    std::lock_guard<std::mutex> lockIo(m_ioMtx);

    m_response.clear();
    if (!m_connected) {
#ifdef DEBUG
        std::cerr << "Not connected" << std::endl;
#endif
        return m_response;
    }

    // set params
    std::string path_ = path;
    if (!params.empty()) {
        path_ += "?";
        for (const auto& it : params) {
            path_ += it.first + "=" + it.second;
            if (it.first != params.rbegin()->first)
                path_ += "&";
        }
    }

    // build request
    m_request.clear();
    m_request.method(http::verb::get);
    m_request.target(path_);
    m_request.version(11);
    m_request.keep_alive(true);
    m_request.set(http::field::host, m_host);
    m_request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // set user-defined headers
    if (!headers.empty())
        for (const auto& it : headers)
            m_request.set(it.first, it.second);

    // send request
    http::write(m_stream, m_request);

    // receive response
    m_recvBuffer.clear();
    beast::get_lowest_layer(m_stream).expires_after(timeout);  // set timeout
    http::read(m_stream, m_recvBuffer, m_response);

    return m_response;
}
