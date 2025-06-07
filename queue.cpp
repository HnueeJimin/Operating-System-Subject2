#include <iostream>
#include <mutex>
#include <cstring>
#include "queue.h"

static std::mutex head_lock;
static std::mutex tail_lock;

// 큐 초기화 함수
Queue* init(void) {
    // 새로운 큐 생성 및 초기화
    Node* dummy = new Node;  // 더미 노드 생성
    dummy->next = nullptr;   // 더미 노드의 next를 nullptr로 설정

    Queue* queue = new Queue; // 새로운 큐 생성
    queue->head = dummy;      // 큐의 head를 더미 노드로 설정, 초기 설정
    queue->tail = dummy;      // 큐의 tail도 더미 노드로 설정, 초기 설정

    return queue;
}

// 큐 메모리 해제 함수
void release(Queue* queue) {
    Node* curr = queue->head;  // 큐의 head부터 시작
    while (curr != nullptr) {
        Node* next = curr->next;  // 다음 노드 저장

        if (curr->item.value) {
            free(curr->item.value);  // 아이템의 value 메모리 해제
            curr->item.value = nullptr;  // 포인터 초기화
        }

        delete curr;  // 현재 노드 메모리 해제
        curr = next;  // 다음 노드로 이동
    }
    delete queue;  // 큐 자체 메모리 해제
}

// 새로운 노드 생성 함수
Node* nalloc(Item item) {
    // Node 생성 후 item으로 초기화
    Node* node = new Node();
    node->item = item;  // 아이템 설정
    node->next = nullptr;  // 다음 노드는 nullptr로 초기화
    return node;
}

// 노드 메모리 해제 함수
void nfree(Node* node) {
    delete node;  // 노드 메모리 해제
    return;
}

// 노드 복제 함수
Node* nclone(Node* node) {
    // node의 item을 복사하여 새로운 노드 생성
    if (!node) return nullptr;
    return nalloc(node->item);  
}

// 큐에 item을 추가 (enqueue)
Reply enqueue(Queue* queue, Item item) {
    Reply reply = { false, {0, nullptr, 0} }; // 초기화된 응답 구조체
    
    Node* node = nalloc(item);  // 새로운 노드 생성

    std::lock_guard<std::mutex> lock(tail_lock);  // tail_lock을 사용하여 동기화, 멀티쓰레드에서 안전하게 처리
    queue->tail->next = node;  // 현재 tail의 next를 새 노드로 설정
    queue->tail = node;  // 큐의 tail을 새 노드로 업데이트

    reply.success = true;  // 성공적으로 추가됨
    reply.item = item;  // 반환할 아이템 설정   
    return reply;
}

// 큐에서 item을 제거하여 반환 (dequeue)
Reply dequeue(Queue* queue) {
    Reply reply = { false, {0, nullptr} };
    
    std::lock_guard<std::mutex> lock(head_lock);  // head_lock을 사용하여 동기화
    Node* first = queue->head;
    Node* next = first->next;  // 첫 번째 노드의 다음 노드

    if (next == nullptr) {
        // 큐가 비어있음
        return reply;  // 실패 응답 반환
    }

    reply.success = true;  // 성공적으로 제거됨
    reply.item = next->item;  // 반환할 아이템 설정

    queue->head = next;  // 큐의 head를 다음 노드로 업데이트
    nfree(first);  // 이전 head 노드 메모리 해제
    return reply;
}

// start와 end 키 범위의 아이템을 포함한 새 큐 반환 (GETRANGE)
Queue* range(Queue* queue, Key start, Key end) {
    Queue* new_queue = init();  // 새 큐 생성

    Node* curr = queue->head->next;  // 첫 번째 노드부터 시작 (더미 노드 제외)

    while (curr != nullptr) {
        Key k = curr->item.key; // key 값도 복사, 어차피 서로 다른 큐

        if (start <= k && k <= end) { // 현재 노드의 key가 범위 내에 있는 경우
            // 깊은 복사
            void* value_copy = malloc(curr->item.value_size); // value 크기만큼 메모리 할당
            memcpy(value_copy, curr->item.value, curr->item.value_size); // value 복사

            Item copied;
            copied.key = curr->item.key; // key 복사
            copied.value = value_copy; // 복사한 value 설정
            copied.value_size = curr->item.value_size; // value 크기 설정

            enqueue(new_queue, copied);  // 새 큐에 아이템 추가
        }
        curr = curr->next;  // 다음 노드로 이동
    }

    return new_queue;  // 새 큐 반환
}
