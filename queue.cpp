#include <iostream>
#include <mutex>
#include <cstring>
#include "queue.h"

static std::mutex skiplist_lock;

// 랜덤 레벨 생성 함수 (스킵 리스트의 레벨을 결정하는 데 사용)
int random_level() {
    int level = 0;
    while (((double)rand() / RAND_MAX) < SKIPLIST_P && level < SKIPLIST_MAX_LEVEL)
        level++;
    return level;
}

// 스킵 리스트 초기화 함수 (이름은 Queue로 유지)
Queue* init(void) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));  // 동적으로로 메모리 할당
    if(!queue) return NULL; // 메모리 할당 실패 시 NULL 반환

    queue->level = 0;  // 초기 레벨은 0

    // 헤더 노드 생성
    Node* head = (Node*)malloc(sizeof(Node));
    if (!head) {
        free(queue);  // 헤더 노드 할당 실패 시 큐 메모리 해제
        return NULL;  // NULL 반환
    }
    head->item.key = 0;  // 더미 노드의 키는 0
    head->item.value = NULL;  // 더미 노드의 value는 NULL
    head->node_level = SKIPLIST_MAX_LEVEL;

    // forward 포인터 배열 할당 및 초기화
    head->forward = (Node**)malloc(sizeof(Node*) * SKIPLIST_MAX_LEVEL);
    if(!head->forward) {
        free(head);  // forward 포인터 배열 할당 실패 시 헤더 노드 메모리 해제
        free(queue);  // 큐 메모리 해제
        return NULL;  // NULL 반환
    }
    for (int i = 0; i < SKIPLIST_MAX_LEVEL; ++i) {
        head->forward[i] = NULL;  // 모든 forward 포인터를 nullptr로 초기화
    }

    queue->head = head;  // 큐의 head를 더미 노드로 설정
    return queue;  // 초기화된 큐 반환
}

// 메모리 해제 함수
void release(Queue* queue) {
    Node* curr = queue->head;  // 큐의 head부터 시작
    while (curr) {
        Node* next = curr->forward[0];  // 다음 노드 저장
        if (curr->item.value) free(curr->item.value);  // 아이템의 value 메모리 해제
        nfree(curr);  // 현재 노드 메모리 해제
        curr = next;  // 다음 노드로 이동
    }
    free(queue);
}

// 새로운 노드 생성 함수
Node* nalloc(Item item) {
    // Node 생성 후 item으로 초기화
    Node* node = new Node();
    node->item = item;  // 아이템 설정
    return node;
}

// 노드 메모리 해제 함수
void nfree(Node* node) {
    if (node) {
        free(node->forward);  // forward 포인터 배열 메모리 해제
        free(node); // 노드 자체 메모리 해제
    }
}

// 노드 복제 함수
Node* nclone(Node* node) {
    // node의 item을 복사하여 새로운 노드 생성
    if (!node) return NULL;
    
    Item new_item;
    new_item.key = node->item.key;  // key 복사
    new_item.value = malloc(node->item.value_size);  // value 크기만큼 메모리 할당
    memcpy(new_item.value, node->item.value, node->item.value_size);  // value 복사
    new_item.value_size = node->item.value_size;  // value 크기 설정

    return nalloc(new_item);  // 새로운 노드 반환
}

// 큐에 item을 추가 (enqueue)
Reply enqueue(Queue* queue, Item item) {
    std::lock_guard<std::mutex> lock(skiplist_lock);  // skiplist_lock을 사용하여 동기화

    Reply reply = { false, {0, nullptr} };  // 초기화된 응답 구조체

    Node* update[SKIPLIST_MAX_LEVEL];  // 업데이트할 노드 배열
    Node* curr = queue->head;  // 큐의 head부터 시작

    for (int i = queue->level; i>=0; i--) {
        while (curr->forward[i] && curr->forward[i]->item.key > item.key) {
            curr = curr->forward[i];  // 현재 노드의 forward 포인터를 따라 이동
        }
        update[i] = curr;  // 업데이트할 노드 저장
    }

    curr = curr->forward[0];  // 현재 노드의 forward[0]로 이동
    if (curr && curr->item.key == item.key) {
        // 이미 같은 키가 존재하는 경우
        if (curr->item.value) free(curr->item.value); // 기존 value 메모리 해제
        curr->item.value = item.value;  // value를 덮어쓰기
        curr->item.value_size = item.value_size;  // value 크기 업데이트
        reply.success = true;  // 성공 응답
        reply.item = curr->item;  // 응답 아이템 설정
        return reply;  // 실패 응답 반환
    }

    int level = random_level();  // 새로운 노드의 레벨 생성
    if (level > queue->level) {
        // 새로운 레벨이 현재 큐의 레벨보다 높으면 큐 레벨 업데이트
        for (int i = queue->level + 1; i <= level; i++) {
            update[i] = queue->head;  // 더미 노드로 초기화
        }
        queue->level = level;  // 큐 레벨 업데이트
    }

    Node* new_node = nalloc(item);  // 새로운 노드 생성
    new_node->node_level = level;  // 노드 레벨 설정
    new_node->forward = (Node**)malloc(sizeof(Node*) * (level + 1));  // forward 포인터 배열 할당
    for (int i = 0; i <= level; i++) {
        new_node->forward[i] = NULL;  // forward 포인터 초기화
    }

    for (int i = 0; i <= level; i++) {
        // 새로운 노드의 forward 포인터 설정
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;  // 업데이트된 노드의 forward 포인터 설정
    }

    reply.success = true;  // 성공적으로 추가됨
    reply.item = item;  // 응답 아이템 설정
    return reply;  // 성공 응답 반환
}

// 큐에서 item을 제거하여 반환 (dequeue)
Reply dequeue(Queue* queue) {
    std::lock_guard<std::mutex> lock(skiplist_lock);  // skiplist_lock을 사용하여 동기화

    Reply reply = { false, {0, nullptr} };  // 초기화된 응답 구조체
    Node* first = queue->head->forward[0];  // 첫 번째 노드 (가장 작은 키를 가진 노드)
    if (!first) return reply;  // 비어있으면 실패 응답 반환

    for (int i = 0; i <= queue->level; i++) { // 현재 레벨까지 반복
        if (queue->head->forward[i] == first) { // 현재 레벨에서 첫 번째 노드가 head의 forward[i]와 같으면
            queue->head->forward[i] = first->forward[i];  // head의 forward[i]를 다음 노드로 업데이트
        }
    }

    while (queue->level > 0 && queue->head->forward[queue->level] == nullptr) {
        // 현재 레벨이 0이 아니고, head의 forward[현재 레벨]이 nullptr이면 레벨 감소
        queue->level--;  // 큐 레벨 감소
    }

    reply.success = true;  // 성공적으로 제거됨
    reply.item = first->item;  // 응답 아이템 설정

    nfree(first);  // 이전 head 노드 메모리 해제
    return reply;
}

// start와 end 키 범위의 아이템을 포함한 새 큐 반환 (GETRANGE)
Queue* range(Queue* queue, Key start, Key end) {
    std::lock_guard<std::mutex> lock(skiplist_lock);  // skiplist_lock을 사용하여 동기화

    Queue* new_queue = init();  // 새 큐 초기화
    Node* curr = queue->head->forward[0];  // 현재 노드 (첫 번째 노드부터 시작)

    while (curr) {
        if (curr->item.key >= start && curr->item.key <= end) { // 현재 노드의 키가 범위에 포함되면
            // 깊은 복사
            void *value_copy = malloc(curr->item.value_size);  // value 크기만큼 메모리 할당
            memcpy(value_copy, curr->item.value, curr->item.value_size);  // value 복사

            Item new_item;
            new_item.key = curr->item.key;  // 키 복사
            new_item.value = value_copy;  // 복사된 value 포인터 설정
            new_item.value_size = curr->item.value_size;  // value 크기 복사
            
            enqueue(new_queue, new_item);  // 새 큐에 아이템 추가
        }
        curr = curr->forward[0];  // 다음 노드로 이동
    }

    return new_queue;  // 새 큐 반환
}
