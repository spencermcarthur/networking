#include "HTTPClient.hpp"

HTTPClient::HTTPClient()
    : m_ioCtx{},
      m_sslCtx{ssl::context::tlsv12_client},
      m_resolver{m_ioCtx} {
    m_sslCtx.set_default_verify_paths();
    m_sslCtx.set_verify_mode(ssl::verify_peer);
}

http::response<http::string_body> HTTPClient::Get(const std::string& host, const std::string& path,
                                                  const std::map<std::string, std::string>& params,
                                                  const std::map<std::string, std::string>& headers,
                                                  const std::chrono::seconds& timeout) {
    std::string host_ = host;
    std::string path_ = path;
    const int version = 11;
    const uint16_t port = 443;

    if (host[host.size() - 1] == '/') host_ = host.substr(0, host.size() - 1);  // strip trailing forward slash
    // strip leading https://
    size_t i = host_.find_first_of("://");
    if (i != std::string::npos) {
        if (host_.substr(0, i) == "http") throw std::runtime_error("Non-SSL connection not supported");
        host_ = host_.substr(i + 3);
    }

    if (!params.empty()) {
        path_ += "?";
        for (const auto& it : params) {
            path_ += it.first + "=" + it.second;
        }
    }

    beast::ssl_stream<beast::tcp_stream> stream{m_ioCtx, m_sslCtx};

    if (!SSL_set_tlsext_host_name(stream.native_handle(), host_.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }

    auto const results = m_resolver.resolve(host_, std::to_string(port));  // resolve domain name
    beast::get_lowest_layer(stream).connect(results);                      // connect to IP address
    stream.handshake(ssl::stream_base::client);                            // SSL handshake

    // build request
    http::request<http::string_body> req{http::verb::get, path_, version};
    req.set(http::field::host, host_);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    // set user-defined headers
    if (!headers.empty())
        for (const auto& it : headers) req.set(it.first, it.second);
    http::write(stream, req);  // send request

    // receive response
    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    beast::get_lowest_layer(stream).expires_after(timeout);  // set timeout
    http::read(stream, buffer, res);

    beast::error_code ec;
    stream.shutdown(ec);
    if (net::error::eof != ec && ssl::error::stream_truncated != ec && ec)
        throw beast::system_error{ec};

    // handle redirect
    if (res.result_int() == 301) {
        std::string newHost;
        if (res.find("Location") != res.end())
            newHost = res.at("Location").to_string();
        else if (res.find("location") != res.end())
            newHost = res.at("location").to_string();

        res = Get(newHost, path_, params, headers, timeout);
    }

    return res;
}