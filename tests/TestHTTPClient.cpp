#include <thread>

#include "networking/HTTPClient.hpp"
#include "gtest/gtest.h"

class HTTPClientTest : public ::testing::Test {
protected:
    HTTPClient client;
    const std::string URL = "api.exchange.coinbase.com";
    const int nIter = 5;
    std::chrono::milliseconds delay{100};
};

TEST_F(HTTPClientTest, TestConnectDisconnect) {
    EXPECT_FALSE(client.IsConnected());

    EXPECT_TRUE(client.Connect(URL));
    EXPECT_TRUE(client.Disconnect());

    EXPECT_FALSE(client.IsConnected());
}

TEST_F(HTTPClientTest, TestReconnect) {
    EXPECT_FALSE(client.IsConnected());

    for (int i = 0; i < nIter; i++) {
        EXPECT_TRUE(client.Connect(URL));
        EXPECT_TRUE(client.Disconnect());

        std::this_thread::sleep_for(delay);
    }

    EXPECT_FALSE(client.IsConnected());
}

TEST_F(HTTPClientTest, TestReconnectNoDisconnect) {
    EXPECT_FALSE(client.IsConnected());

    for (int i = 0; i < nIter; i++) {
        EXPECT_TRUE(client.Connect(URL));

        std::this_thread::sleep_for(delay);
    }

    EXPECT_TRUE(client.IsConnected());
}

TEST_F(HTTPClientTest, TestDisconnectNotConnected) {
    EXPECT_FALSE(client.IsConnected());

    for (int i = 0; i < nIter; i++)
        EXPECT_TRUE(client.Disconnect());

    EXPECT_FALSE(client.IsConnected());
}

TEST_F(HTTPClientTest, TestSimpleRequest) {
    EXPECT_FALSE(client.IsConnected());

    EXPECT_TRUE(client.Connect(URL));

    auto resp = client.Get("/products/BTC-USD");
    EXPECT_FALSE("" == resp.body());
    EXPECT_TRUE(400 > resp.result_int());
#ifdef DEBUG
    std::cout << resp << std::endl;
#endif

    EXPECT_TRUE(client.Disconnect());

    EXPECT_FALSE(client.IsConnected());
}

TEST_F(HTTPClientTest, TestRequestWithPathAndParams) {
    EXPECT_FALSE(client.IsConnected());

    EXPECT_TRUE(client.Connect(URL));

    std::time_t end = std::round((double)std::time(NULL) / 86400.) * 84600.;
    std::time_t start = end - 14 * 86400;
    std::vector<std::pair<std::string, std::string>> params{};
    params.push_back({"granularity", "86400"});
    params.push_back({"start", std::to_string(start)});
    params.push_back({"end", std::to_string(end)});
    auto resp = client.Get("/products/BTC-USD/candles", params);
    EXPECT_FALSE("" == resp.body());
    EXPECT_TRUE(400 > resp.result_int());
#ifdef DEBUG
    std::cout << resp << std::endl;
#endif

    EXPECT_TRUE(client.Disconnect());

    EXPECT_FALSE(client.IsConnected());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
