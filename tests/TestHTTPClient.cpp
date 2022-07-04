#include "HTTPClient.hpp"
#include "gtest/gtest.h"

class HTTPClientTest : public ::testing::Test {
protected:
    HTTPClient client;
    const std::string URL = "api.exchange.coinbase.com";
};

TEST_F(HTTPClientTest, TestSimpleRequest) {
    auto resp = client.Get(URL);
    EXPECT_FALSE("" == resp.body());
#ifdef DEBUG
    std::cout << resp << std::endl;
#endif
}

TEST_F(HTTPClientTest, TestRequestWithPath) {
    auto resp = client.Get(URL, "/products/BTC-USD");
    EXPECT_FALSE("" == resp.body());
#ifdef DEBUG
    std::cout << resp << std::endl;
#endif
}

TEST_F(HTTPClientTest, TestRequestWithPathAndParams) {
    auto resp = client.Get(URL, "/products/BTC-USD/book", {{"level", "1"}});
    EXPECT_FALSE("" == resp.body());
#ifdef DEBUG
    std::cout << resp << std::endl;
#endif
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
