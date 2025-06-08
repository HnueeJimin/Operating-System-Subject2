#include <iostream>
#include <thread>
#include <chrono>
#include "queue.h"

using namespace std;
using namespace std::chrono;

int enqueue_count_t1 = 0;
int enqueue_count_t2 = 0;
int dequeue_count_t1 = 0;
int dequeue_count_t2 = 0;
int dequeue_count_t3 = 0;

#define ENQUEUE_PER_THREAD 500

// ================= enqueue =====================
void client_func_enqueue(Queue* queue, int start_key, int count, int& enqueue_count) {
    for (int i = 0; i < count; i++) {
        Item item;
        item.key = start_key + i;
        
        int* val = (int*)malloc(sizeof(int));
        *val = rand() % 1000000;

        item.value = val;
        item.value_size = sizeof(int);

        Reply reply = enqueue(queue, item);
        if (reply.success) {
            enqueue_count++;
            cout << "enqueue : success=true  [key=" << item.key << ", value=" << *val << "]" << endl;
        } else {
            cout << "enqueue : success=false" << endl;
        }
    }
}

// ================= dequeue =====================
void client_func_dequeue(Queue* queue, int count, int& dequeue_count, const string& name) {
    for (int i = 0; i < count; i++) {
        Reply reply = dequeue(queue);
        if (reply.success) {
            dequeue_count++;
            cout << name << " dequeue : success=true  [key=" << reply.item.key
                 << ", value=" << *((int*)reply.item.value) << "]" << endl;
        } else {
            cout << name << " dequeue : success=false" << endl;
        }
    }
}

// ================= queue 상태 출력 =====================
void print_queue_state(Queue* queue, const string& name, int enqueue_count, int dequeue_count) {
    cout << "\n== " << name << " Final queue state ==" << endl;
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

    bool safe = (enqueue_count == dequeue_count + node_count);
    cout << "Safety check: enqueue=" << enqueue_count
         << "; dequeue=" << dequeue_count
         << "; remaining=" << node_count
         << "; safe=" << (safe ? "true" : "false") << endl;
}

int main(void) {
    srand((unsigned int)time(NULL));

    // =================== Step 1: enqueue =====================
    Queue* queue_t1 = init();
    Queue* queue_t2 = init();

    thread t1(client_func_enqueue, queue_t1, 0, ENQUEUE_PER_THREAD, ref(enqueue_count_t1));
    thread t2(client_func_enqueue, queue_t2, 10000, ENQUEUE_PER_THREAD, ref(enqueue_count_t2));

    t1.join();
    t2.join();

    // =================== Step 2: deepcopy queue_t2 -> queue_t3 =====================
    Queue* queue_t3 = range(queue_t2, 10000, 10000 + ENQUEUE_PER_THREAD - 1);

    // t2의 첫 노드 값 수정
    Node* node = queue_t2->head->forward[0];
    if (node && node->item.value) {
        *((int*)node->item.value) = -9999;  // 강제로 값 변경
    }

    // =================== Step 3: dequeue =====================
    thread d1(client_func_dequeue, queue_t1, ENQUEUE_PER_THREAD, ref(dequeue_count_t1), "t1");
    thread d2(client_func_dequeue, queue_t2, ENQUEUE_PER_THREAD, ref(dequeue_count_t2), "t2");
    thread d3(client_func_dequeue, queue_t3, ENQUEUE_PER_THREAD, ref(dequeue_count_t3), "t3");

    d1.join(); d2.join(); d3.join();

    // =================== Step 4: deep copy 확인 =====================
    cout << "\n== Deep Copy 검증 결과 ==" << endl;
    Node* n3 = queue_t3->head->forward[0];
    if (n3) {
        int val3 = *((int*)n3->item.value);
        cout << "t3 첫 노드 value: " << val3 << " (should not be -9999)" << endl;
        cout << "deep_copy_check: " << (val3 == -9999 ? "false" : "true") << endl;
    } 
    // else {
    //     cout << "t3 is empty" << endl;
    // }

    // =================== Step 5: 상태 출력 및 해제 =====================
    print_queue_state(queue_t1, "t1", enqueue_count_t1, dequeue_count_t1);
    print_queue_state(queue_t2, "t2", enqueue_count_t2, dequeue_count_t2);
    print_queue_state(queue_t3, "t3", enqueue_count_t2, dequeue_count_t3); // t3는 t2의 복사본이므로 enqueue_count 같음

    release(queue_t1);
    release(queue_t2);
    release(queue_t3);

    return 0;
}
