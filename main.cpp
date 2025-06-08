#include <iostream>
#include <thread>
#include <chrono>
#include "queue.h"

using namespace std;
using namespace std::chrono;

#define REQUEST_PER_CLIENT 1000
#define RANGE_START_KEY 100
#define RANGE_END_KEY 110

int enqueue_count = 0;
int dequeue_count = 0;

typedef enum {
    GET,
    SET
} Operation;

typedef struct {
    Operation op;
    Item item;
} Request;

void client_func(Queue* queue, Request requests[], int n_request) {
    auto start_time = high_resolution_clock::now();

    for (int i = 0; i < n_request; i++) {
        Reply reply;

        if (requests[i].op == GET) {
            reply = dequeue(queue);
            if (reply.success) {
                dequeue_count++;
                cout << "dequeue : success=true  [key=" << reply.item.key
                     << ", value=" << *((int*)reply.item.value) << "]" << endl;
            } else {
                cout << "dequeue : success=false" << endl;
            }
        } else if (requests[i].op == SET) {
            reply = enqueue(queue, requests[i].item);
            if (reply.success) {
                enqueue_count++;
                cout << "enqueue : success=true  [key=" << reply.item.key
                     << ", value=" << *((int*)reply.item.value) << "]" << endl;
            } else {
                cout << "enqueue : success=false" << endl;
            }
        }
    }

    auto end_time = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end_time - start_time).count();
    cout << "[Client Thread] Time: " << elapsed << " ms" << endl;
}

void print_queue_state(Queue* queue) {
    cout << "\n== Final queue state ==" << endl;
    Node* curr = queue->head->forward[0];
    int node_count = 0;

    if (!curr) {
        cout << "current queue : <empty>" << endl;
    } else {
        cout << "current queue :" << endl;
        while (curr) {
            int val = *((int*)curr->item.value);
            cout << "  key=" << curr->item.key << ", value=" << val << endl;
            node_count++;
            curr = curr->forward[0];
        }
    }

    bool safe = (enqueue_count == (dequeue_count + node_count));
    cout << "Safety check: enqueue_count=" << enqueue_count
         << "; dequeue_count=" << dequeue_count
         << "; node_count=" << node_count
         << "; safe=" << (safe ? "true" : "false") << endl;
}

void test_range(Queue* queue, Key start, Key end) {
    cout << "\n== Performing range(" << start << ", " << end << ") ==" << endl;
    Queue* copied = range(queue, start, end);

    Node* curr = copied->head->forward[0];
    if (!curr) {
        cout << "range result : <empty>" << endl;
    } else {
        cout << "range result :" << endl;
        while (curr) {
            int val = *((int*)curr->item.value);
            cout << "  [COPY] key=" << curr->item.key << ", value=" << val << endl;
            curr = curr->forward[0];
        }
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
    t1.join();

    test_range(queue, RANGE_START_KEY, RANGE_END_KEY);

    print_queue_state(queue);
    release(queue);
    return 0;
}
