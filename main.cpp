#include <iostream>
#include <thread>
#include <chrono>
#include "queue.h"

using namespace std;
using namespace std::chrono;

#define REQUEST_PER_CLIENT 10000
#define RANGE_START_KEY 100
#define RANGE_END_KEY 110

typedef enum {
    GET,
    SET,
    GETRANGE
} Operation;

typedef struct {
    Operation op;
    Item item;
} Request;

// enqueue/dequeue 테스트용 쓰레드
void client_func(Queue* queue, Request requests[], int n_request) {
    auto start_time = high_resolution_clock::now();

    for (int i = 0; i < n_request; i++) {
        Reply reply;
        if (requests[i].op == GET) {
            reply = dequeue(queue);
            if (reply.success) {
                int val = *((int*)reply.item.value);
                cout << "[GET] key: " << reply.item.key << ", value: " << val << endl;
            }
        } else if (requests[i].op == SET) {
            reply = enqueue(queue, requests[i].item);
            if (reply.success) {
                int val = *((int*)reply.item.value);
                cout << "[SET] key: " << reply.item.key << ", value: " << val << endl;
            }
        }
    }

    auto end_time = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end_time - start_time).count();
    cout << "[Client Thread] Time: " << elapsed << " ms" << endl;
}

// 깊은 복사 검증용 쓰레드
void range_checker(Queue* queue, Key start, Key end) {
    auto start_time = high_resolution_clock::now();

    Queue* copied = range(queue, start, end);

    auto end_time = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end_time - start_time).count();
    cout << "[Range Thread] Time: " << elapsed << " ms" << endl;

    cout << "[Range Thread] Deep copy result:" << endl;
    Node* curr = copied->head->forward[0];
    while (curr) {
        int val = *((int*)curr->item.value);
        cout << "  [COPY] key: " << curr->item.key << ", value: " << val << endl;
        curr = curr->forward[0];
    }

    release(copied);
}

int main(void) {
    srand((unsigned int)time(NULL));

    Request requests[REQUEST_PER_CLIENT];
    for (int i = 0; i < REQUEST_PER_CLIENT / 2; i++) {
        requests[i].op = SET;
        requests[i].item.key = i;

        int* val = new int(rand() % 1000000);
        requests[i].item.value = val;
        requests[i].item.value_size = sizeof(int);
    }

    for (int i = REQUEST_PER_CLIENT / 2; i < REQUEST_PER_CLIENT; i++) {
        requests[i].op = GET;
    }

    Queue* queue = init();

    thread t1(client_func, queue, requests, REQUEST_PER_CLIENT);
    thread t2(range_checker, queue, RANGE_START_KEY, RANGE_END_KEY);

    t1.join();
    t2.join();

    release(queue);

    return 0;
}
