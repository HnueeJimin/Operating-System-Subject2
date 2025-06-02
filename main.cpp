#include <iostream>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <ctime>
#include "queue.h"

using namespace std;

#define REQUESTS_PER_CLIENT 10000
#define CLIENT_COUNT 4

atomic<int> sum_key(0);
atomic<int> sum_value(0);

typedef enum {
    GET,
    SET
} Operation;

typedef struct {
    Operation op;
    Item item;
} Request;

void client_func(Queue* queue, Request* requests, int n_request) {
    for (int i = 0; i < n_request; i++) {
        Reply reply;

        if (requests[i].op == GET) {
            reply = dequeue(queue);
        } else {
            reply = enqueue(queue, requests[i].item);
        }

        if (reply.success) {
            sum_key += reply.item.key;
            sum_value += reinterpret_cast<intptr_t>(reply.item.value);
        }
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    Queue* queue = init();

    // 정적 배열로 요청 저장
    Request all_requests[CLIENT_COUNT][REQUESTS_PER_CLIENT];
    thread clients[CLIENT_COUNT];

    for (int c = 0; c < CLIENT_COUNT; ++c) {
        for (int i = 0; i < REQUESTS_PER_CLIENT / 2; ++i) {
            all_requests[c][i].op = SET;
            all_requests[c][i].item.key = rand() % 10000000;
            all_requests[c][i].item.value = reinterpret_cast<void*>(rand() % 1000000);
        }
        for (int i = REQUESTS_PER_CLIENT / 2; i < REQUESTS_PER_CLIENT; ++i) {
            all_requests[c][i].op = GET;
        }
    }

    // 클라이언트 스레드 시작
    for (int c = 0; c < CLIENT_COUNT; ++c) {
        clients[c] = thread(client_func, queue, all_requests[c], REQUESTS_PER_CLIENT);
    }

    // join
    for (int c = 0; c < CLIENT_COUNT; ++c) {
        clients[c].join();
    }

    cout << "sum of returned keys   = " << sum_key.load() << endl;
    cout << "sum of returned values = " << sum_value.load() << endl;

    release(queue);
    return 0;
}
