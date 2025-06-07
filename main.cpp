#include <iostream>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <mutex>
#include "queue.h"

using namespace std;

// === 큐 출력 디버깅 ===
void print_queue(Queue* q, const string& label) {
    cout << "[" << label << "] 큐 내용: ";
    Node* curr = q->head->next;  // 더미 노드 제외
    while (curr) {
        int val = 0;
        if (curr->item.value != nullptr) {
            memcpy(&val, curr->item.value, sizeof(int));
        }
        cout << "(" << curr->item.key << ", " << val << ") ";
        curr = curr->next;
    }
    cout << "\n";
}

// === Debug Test 1: 깊은 복사 ===
void test_deep_copy() {
    cout << "\n=== [테스트] Deep Copy 검증 ===\n";

    Queue* original = init(); // 원본 큐 초기화
    
    int* value1 = new int(42);
    Item item;
    item.key = 1234;
    item.value_size = sizeof(int);
    item.value = malloc(item.value_size);
    memcpy(item.value, value1, item.value_size);
    delete value1; // 메모리 해제

    enqueue(original, item); // 원본 큐에 아이템 추가
    print_queue(original, "원본 큐 (초기)");

    Queue* copied = range(original, 1000, 1300);
    print_queue(copied, "복사된 큐 (초기)");

    // 원본 수정
    Node* node = original->head->next;
    if (node) {
        int new_value = -1;
        memcpy(node->item.value, &new_value, sizeof(int)); // 원본 큐의 첫 번째 아이템 값 변경
        cout << "[원본 수정] key: " << node->item.key << ", value: " << new_value << "\n";
    }

    print_queue(original, "원본 큐 (수정 후)");
    print_queue(copied, "복사된 큐 (수정 후)");

    release(original); // 원본 큐 해제
    release(copied); // 복사된 큐 해제
}

// === Debug Test 2: 원자성 검증 ===
void test_atomic() {
    cout << "\n=== [테스트] 원자성 검증 ===\n";

    Queue* queue = init(); // 큐 초기화
    atomic<int> count(0); // 원자적 카운터
    atomic<int> inserted_count(0); // 삽입된 아이템 수
    mutex cout_lock; // 출력 동기화용 뮤텍스

    auto worker = [&](int id) {
        for (int i = 0; i < 1000; ++i) {
            int* val = new int(id + i);
            Item item;
            item.key = id + i;
            item.value = malloc(sizeof(int));
            memcpy(item.value, val, sizeof(int)); // 깊은 복사

            Reply r = enqueue(queue, item); // 큐에 아이템 삽입
            if (r.success) {
                inserted_count++;
            }
            delete val; // 메모리 해제
        }

        for (int i = 0; i < 1000; ++i) {
            Reply reply = dequeue(queue); // 큐에서 아이템 제거
            if (reply.success) {
                int val;
                memcpy(&val, reply.item.value, sizeof(int)); // 값 복사
                count++;

                free(reply.item.value); // 메모리 해제
            }
        }
    };

    thread t1(worker, 1000);
    thread t2(worker, 2000);
    t1.join();
    t2.join();

    cout << "총 삽입된 아이템 수: " << inserted_count.load() << endl;
    cout << "정상적으로 처리된 아이템 수: " << count.load() << endl;

    release(queue); // 큐 해제
}

int main() {
    test_deep_copy(); // 깊은 복사 테스트
    test_atomic(); // 원자성 테스트
    return 0;
}

// #define REQUESTS_PER_CLIENT 4
// #define CLIENT_COUNT 2

// atomic<int> sum_key(0);
// atomic<int> sum_value(0);

// typedef enum {
//     GET,
//     SET,
//     GETRANGE
// } Operation;

// typedef struct {
//     Operation op;
//     Item item;
// } Request;

// void deep_copy_test();
// void stress_test_atomicity();

// void client_func(Queue* queue, Request* requests, int n_request) {
//     for (int i = 0; i < n_request; i++) {
//         Reply reply;

//         if (requests[i].op == GET) {
//             reply = dequeue(queue);
//         } else if (requests[i].op == SET) {
//             reply = enqueue(queue, requests[i].item);
//         } else if (requests[i].op == GETRANGE) {
//             Queue* sub = range(queue, 50, 150);  // 예시 범위

//             // sum에 누적해서 결과 확인
//             Node* node = sub->head->next;
//             while (node) {
//                 sum_key += node->item.key;
//                 int val;
//                 memcpy(&val, node->item.value, sizeof(int));
//                 sum_value += val;
//                 node = node->next;
//             }

//             release(sub);
//             continue; // GETRANGE는 Reply 없음
//         }

//         // GET/SET만 해당
//         if (reply.success && (requests[i].op != GETRANGE)) {
//             sum_key += reply.item.key;
//             int val;
//             memcpy(&val, reply.item.value, sizeof(int));
//             sum_value += val;
//         }
//     }
// }


// int main() {
//     srand(static_cast<unsigned>(time(nullptr)));
//     Queue* queue = init();

//     Request all_requests[CLIENT_COUNT][REQUESTS_PER_CLIENT];
//     thread clients[CLIENT_COUNT];

//     // 테스트: 중복 key 의도적 삽입 + 깊은 복사된 value 설정
//     for (int c = 0; c < CLIENT_COUNT; ++c) {
//     for (int i = 0; i < REQUESTS_PER_CLIENT / 3; ++i) {
//         all_requests[c][i].op = SET;
//         all_requests[c][i].item.key = 100 + rand() % 100;
//         int* val = new int(rand() % 1000);
//         all_requests[c][i].item.value = malloc(sizeof(int));
//         memcpy(all_requests[c][i].item.value, val, sizeof(int));
//         all_requests[c][i].item.value_size = sizeof(int);
//         delete val;
//     }
//     for (int i = REQUESTS_PER_CLIENT / 3; i < 2 * REQUESTS_PER_CLIENT / 3; ++i) {
//         all_requests[c][i].op = GET;
//     }
//     for (int i = 2 * REQUESTS_PER_CLIENT / 3; i < REQUESTS_PER_CLIENT; ++i) {
//         all_requests[c][i].op = GETRANGE;
//     }
// }

//     for (int c = 0; c < CLIENT_COUNT; ++c) {
//         clients[c] = thread(client_func, queue, all_requests[c], REQUESTS_PER_CLIENT);
//     }

//     for (int c = 0; c < CLIENT_COUNT; ++c) {
//         clients[c].join();
//     }

//     cout << "sum of returned keys   = " << sum_key.load() << endl;
//     cout << "sum of returned values = " << sum_value.load() << endl;

//     cout << "=== Deep Copy Test ===" << endl;
//     deep_copy_test();

//     cout << "=== Atomic Test ===" << endl;
//     stress_test_atomicity();

//     release(queue);
//     return 0;
// }

// void deep_copy_test() {
//     Queue* a = init();

//     int* v = new int(999);
//     Item item = { 123, malloc(sizeof(int)), sizeof(int) };
//     memcpy(item.value, v, sizeof(int));
//     enqueue(a, item);

//     delete v;  // 메모리 해제

//     Queue* b = range(a, 100, 130);  // 복사 큐

//     //원본 큐 비우기
//     dequeue(a);
//     release(a);

//     // 복사 큐에서 값 확인
//     Node* node = b->head->next;  // 더미 노드 제외
//     if (node) {
//         int val;
//         memcpy(&val, node->item.value, sizeof(int));
//         cout << "Copied value: " << val << endl;  // 999 출력
//     } else {
//         cout << "No items in the copied queue(deep copy failed)." << endl;
//     }

//     release(b);  // 복사 큐 해제
// }

// void stress_test_atomicity() {
//     Queue* q = init();
//     const int total = 10000;
//     std::atomic<int> success_count(0);

//     auto worker = [&](int tid) {
//         for (int i = 0; i < total; i++) {
//             Item item;
//             item.key = i + tid * total;
//             item.value = malloc(sizeof(int));
//             *(int*)(item.value) = item.key;
//             item.value_size = sizeof(int);
//             enqueue(q, item);
//         }

//         for (int i = 0; i < total; i++) {
//             Reply r = dequeue(q);
//             if (r.success) {
//                 success_count++;
//             }
//         }
//     };

//     std::thread t1(worker, 0);
//     std::thread t2(worker, 1);
//     t1.join();
//     t2.join();

//     std::cout << "Successful operations: " << success_count.load() << std::endl;
//     release(q);
// }