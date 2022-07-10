#include "networking/WebSocketClient.hpp"
#include "gtest/gtest.h"

class WebSocketClientTest : public ::testing::Test {
protected:
    WebSocketClient client;
    const std::string URL = "ws-feed-public.sandbox.exchange.coinbase.com";
    const std::string SEND_MESSAGE = R"({"type":"subscribe","channels":[{"name":"heartbeat","product_ids":["BTC-USD"]}]})";
    std::string RECV_MESSAGE;
};

TEST_F(WebSocketClientTest, TestConnect) {
    EXPECT_FALSE(client.IsConnected());
    EXPECT_TRUE(client.Connect(URL));
    EXPECT_TRUE(client.IsConnected());
}

TEST_F(WebSocketClientTest, TestSend) {
    // can't send before connecting
    EXPECT_FALSE(client.Send(SEND_MESSAGE));

    client.Connect(URL);

    EXPECT_TRUE(client.Send(SEND_MESSAGE));
}

TEST_F(WebSocketClientTest, TestRecv) {
    // can't receive before connecting
    EXPECT_FALSE(client.Recv(RECV_MESSAGE));

    client.Connect(URL);
    client.Send(SEND_MESSAGE);

    EXPECT_TRUE(client.Recv(RECV_MESSAGE));
    EXPECT_FALSE("" == RECV_MESSAGE);
}

TEST_F(WebSocketClientTest, TestDisconnect) {
    client.Connect(URL);
    EXPECT_TRUE(client.IsConnected());

    EXPECT_TRUE(client.Disconnect());
    EXPECT_FALSE(client.IsConnected());
}

TEST_F(WebSocketClientTest, FullTest) {
    EXPECT_FALSE(client.Send(SEND_MESSAGE));
    EXPECT_FALSE(client.Recv(RECV_MESSAGE));

    EXPECT_TRUE(client.Connect(URL));
    EXPECT_TRUE(client.Send(SEND_MESSAGE));

    for (int i = 0; i < 2; i++) {
        RECV_MESSAGE.clear();
        EXPECT_TRUE(client.Recv(RECV_MESSAGE));
    }

    EXPECT_TRUE(client.Disconnect());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
