#include "gtest/gtest.h"
#include "WebSocketClient.hpp"

class WebSocketClientTest : public ::testing::Test {
protected:
    const std::string URL = "ws-feed-public.sandbox.exchange.coinbase.com";
    const std::string SEND_MESSAGE = R"({"type":"subscribe","channels":[{"name":"heartbeat","product_ids":["BTC-USD"]}]})";
};

TEST_F(WebSocketClientTest, TestConnect) {
    WebSocketClient client;

    EXPECT_TRUE(client.connect(URL));
}

TEST_F(WebSocketClientTest, TestSend) {
    WebSocketClient client;

    client.connect(URL);
    EXPECT_TRUE(client.send(SEND_MESSAGE));
}

TEST_F(WebSocketClientTest, TestRecv) {
    WebSocketClient client;

    client.connect(URL);
    client.send(SEND_MESSAGE);

    std::string msg{};
    EXPECT_TRUE(client.recv(msg));
    EXPECT_NE("", msg);
}

TEST_F(WebSocketClientTest, TestDisconnect) {
    WebSocketClient client;

    client.connect(URL);
    EXPECT_TRUE(client.disconnect());
}

TEST_F(WebSocketClientTest, FullUpTest) {
    WebSocketClient client;

    EXPECT_TRUE(client.connect(URL));
    EXPECT_TRUE(client.send(SEND_MESSAGE));

    std::string msg;
    for (int i = 0; i < 2; i++) {
        EXPECT_TRUE(client.recv(msg));
    }

    EXPECT_TRUE(client.disconnect());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
