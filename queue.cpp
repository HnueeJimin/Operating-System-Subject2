#include <iostream>
#include <mutex>
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
        delete curr;              // 현재 노드 메모리 해제
        curr = next;              // 다음 노드로 이동
    }
    delete queue;  // 큐 자체 메모리 해제

    return;
}

// 새로운 노드 생성 함수
Node* nalloc(Item item) {
    // Node 생성 후 item으로 초기화
    Node* node = new Node();
    node->item = item;  // 아이템 설정
    node->next = nullptr;  // 다음 노드는 nullptr로 초기화
    return NULL;
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
    Reply reply = { false, {0, nullptr} };
    
    Node* node = nalloc(item);  // 새로운 노드 생성

    std::lock_guard<std::mutex> lock(tail_lock);  // tail_lock을 사용하여 동기화
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
    // 큐를 순회하면서 조건에 맞는 아이템들을 새로운 큐에 복사
    return NULL;
}
