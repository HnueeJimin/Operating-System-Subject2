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