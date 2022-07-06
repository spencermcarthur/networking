#include <thread>

#include "HTTPClient.hpp"
#include "TokenBucket.h"
#include "benchmark/benchmark.h"

static const int REST_MSG_LMT = 10;
static const int REST_MSG_LMT_BURST = 15;

void BM_Get(benchmark::State& state) {
    HTTPClient client;
    client.Connect("api.exchange.coinbase.com");

    TokenBucket tb(REST_MSG_LMT, REST_MSG_LMT_BURST);

    http::response<http::string_body> resp;

    for (auto _ : state) {
        while (!tb.consume(1)) std::this_thread::sleep_for(std::chrono::milliseconds(1000 / REST_MSG_LMT / 2));
        resp = client.Get("/products/BTC-USD/stats");
    }
}
BENCHMARK(BM_Get);

BENCHMARK_MAIN();
