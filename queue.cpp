#include <iostream>
#include <cstring>
#include <cstdlib>
#include <mutex>
#include "queue.h"

// ========== 내부 유틸리티 함수 ==========

// 0 ~ SKIPLIST_MAX_LEVEL-1 범위의 랜덤 레벨 생성
static int random_level() {
    int lvl = 0;
    while (lvl < SKIPLIST_MAX_LEVEL - 1 && ((double)rand() / RAND_MAX) < SKIPLIST_P) {
        ++lvl;
    }
    return lvl;
}

// ========== 큐 초기화/해제 ==========

Queue* init(void) {
    // Queue 및 head 노드 생성
    Queue* queue = new Queue();
    queue->level = 0;
    // head 노드는 최대 레벨 크기로 forward 배열을 가짐
    queue->head = new Node();
    queue->head->item.key = 0;
    queue->head->item.value = nullptr;
    queue->head->item.value_size = 0;
    queue->head->node_level = SKIPLIST_MAX_LEVEL - 1;
    queue->head->forward = new Node * [SKIPLIST_MAX_LEVEL];
    for (int i = 0; i < SKIPLIST_MAX_LEVEL; ++i) {
        queue->head->forward[i] = nullptr;
    }
    return queue;
}

void release(Queue* queue) {
    if (!queue) return;
    // 모든 노드 순회하면서 해제
    {
        std::lock_guard<std::mutex> lock(queue->lock);
        Node* curr = queue->head->forward[0];
        while (curr) {
            Node* next = curr->forward[0];
            // 값 메모리 해제
            if (curr->item.value) free(curr->item.value);
            // forward 배열 및 노드 자체 해제
            delete[] curr->forward;
            delete curr;
            curr = next;
        }
        // head 해제
        delete[] queue->head->forward;
        delete queue->head;
        // 큐 구조체 해제
    }
    delete queue;
}

// ========== 노드 생성/해제/복제 ==========

Node* nalloc(Item item) {
    Node* node = new Node();
    node->item.key = item.key;
    node->item.value_size = item.value_size;
    if (item.value_size > 0 && item.value) {
        node->item.value = malloc(item.value_size);
        memcpy(node->item.value, item.value, item.value_size);
    }
    else {
        node->item.value = nullptr;
    }
    // forward 배열과 level 설정은 enqueue에서 수행
    node->forward = nullptr;
    node->node_level = 0;
    return node;
}

void nfree(Node* node) {
    if (!node) return;
    if (node->item.value) free(node->item.value);
    if (node->forward) delete[] node->forward;
    delete node;
}

Node* nclone(Node* node) {
    if (!node) return nullptr;
    Item tmp;
    tmp.key = node->item.key;
    tmp.value_size = node->item.value_size;
    if (tmp.value_size > 0 && node->item.value) {
        tmp.value = malloc(tmp.value_size);
        memcpy(tmp.value, node->item.value, tmp.value_size);
    }
    else {
        tmp.value = nullptr;
    }
    Node* clone = nalloc(tmp);
    // nalloc이 deep-copy 했으므로 임시 메모리 해제
    if (tmp.value) free(tmp.value);
    return clone;
}

// ========== enqueue / dequeue ==========

Reply enqueue(Queue* queue, Item item) {
    Reply reply = { false, {0, nullptr, 0} };
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);

    // 1) 위치 탐색을 위한 update 배열 구성
    Node* update[SKIPLIST_MAX_LEVEL];
    Node* curr = queue->head;
    for (int i = queue->level; i >= 0; --i) {
        while (curr->forward[i] && curr->forward[i]->item.key > item.key) {
            curr = curr->forward[i];
        }
        update[i] = curr;
    }

    // 2) 새 노드 생성 및 레벨 결정
    int lvl = random_level();
    if (lvl > queue->level) {
        for (int i = queue->level + 1; i <= lvl; ++i) {
            update[i] = queue->head;
        }
        queue->level = lvl;
    }

    Node* new_node = nalloc(item);
    new_node->node_level = lvl;
    new_node->forward = new Node * [lvl + 1];
    for (int i = 0; i <= lvl; ++i) {
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }

    // 3) Reply 구성 (값도 깊은 복사)
    reply.success = true;
    reply.item.key = item.key;
    reply.item.value_size = item.value_size;
    if (item.value_size > 0 && item.value) {
        reply.item.value = malloc(item.value_size);
        memcpy(reply.item.value, item.value, item.value_size);
    }
    else {
        reply.item.value = nullptr;
    }
    return reply;
}

Reply dequeue(Queue* queue) {
    Reply reply = { false, {0, nullptr, 0} };
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);

    Node* first = queue->head->forward[0];
    if (!first) return reply;

    // head 포인터 재연결
    for (int i = 0; i <= queue->level; ++i) {
        if (queue->head->forward[i] == first) {
            queue->head->forward[i] = first->forward[i];
        }
    }
    // 레벨 축소
    while (queue->level > 0 && queue->head->forward[queue->level] == nullptr) {
        --queue->level;
    }

    // Reply 구성 (값 깊은 복사)
    reply.success = true;
    reply.item.key = first->item.key;
    reply.item.value_size = first->item.value_size;
    if (first->item.value_size > 0 && first->item.value) {
        reply.item.value = malloc(first->item.value_size);
        memcpy(reply.item.value, first->item.value, first->item.value_size);
    }
    else {
        reply.item.value = nullptr;
    }

    // 노드 해제
    // 먼저 원본 value 포인터를 nullptr로 하여 nfree가 재해제하지 않도록 함
    if (first->item.value) {
        free(first->item.value);
        first->item.value = nullptr;
    }
    nfree(first);

    return reply;
}

// ========== 범위 검색 ==========

Queue* range(Queue* queue, Key start, Key end) {
    if (!queue) return nullptr;
    std::lock_guard<std::mutex> lock(queue->lock);

    Queue* new_q = init();
    Node* curr = queue->head->forward[0];
    while (curr) {
        if (curr->item.key >= start && curr->item.key <= end) {
            // enqueue 내부에서 deep-copy 됨
            enqueue(new_q, curr->item);
        }
        curr = curr->forward[0];
    }
    return new_q;
}
