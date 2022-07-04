#pragma once

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

class HTTPClient : public std::enable_shared_from_this<HTTPClient> {
public:
    HTTPClient();

    http::response<http::string_body> Get(
        const std::string& host, const std::string& path = "/",
        const std::map<std::string, std::string>& params = {},
        const std::map<std::string, std::string>& headers = {},
        const std::chrono::seconds& timeout = std::chrono::seconds(5));

protected:
    net::io_context m_ioCtx;
    ssl::context m_sslCtx;
    tcp::resolver m_resolver;
};
