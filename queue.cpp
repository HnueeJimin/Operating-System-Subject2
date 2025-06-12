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
    Queue* queue = new Queue();
    queue->level = 0;
    queue->head = new Node();
    queue->head->item.key = 0;
    queue->head->item.value = nullptr;
    queue->head->item.value_size = 0;
    queue->head->node_level = SKIPLIST_MAX_LEVEL - 1;
    queue->head->forward = new Node*[SKIPLIST_MAX_LEVEL];
    for (int i = 0; i < SKIPLIST_MAX_LEVEL; ++i) {
        queue->head->forward[i] = nullptr;
    }
    return queue;
}

void release(Queue* queue) {
    if (!queue) return;
    {
        std::lock_guard<std::mutex> lock(queue->lock);
        Node* curr = queue->head->forward[0];
        while (curr) {
            Node* next = curr->forward[0];
            if (curr->item.value) free(curr->item.value);
            delete[] curr->forward;
            delete curr;
            curr = next;
        }
        delete[] queue->head->forward;
        delete queue->head;
    }
    delete queue;
}

// ========== 노드 생성/해제 ==========

Node* nalloc(Item item) {
    Node* node = new Node();
    node->item.key = item.key;
    node->item.value_size = item.value_size;
    if (item.value_size > 0 && item.value) {
        node->item.value = malloc(item.value_size);
        memcpy(node->item.value, item.value, item.value_size);
    } else {
        node->item.value = nullptr;
    }
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

// ========== enqueue / dequeue ==========

Reply enqueue(Queue* queue, Item item) {
    Reply reply = {false, {0, nullptr, 0}};
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);

    Node* update[SKIPLIST_MAX_LEVEL];
    Node* curr = queue->head;
    for (int i = queue->level; i >= 0; --i) {
        while (curr->forward[i] && curr->forward[i]->item.key > item.key) {
            curr = curr->forward[i];
        }
        update[i] = curr;
    }

    int lvl = random_level();
    if (lvl > queue->level) {
        for (int i = queue->level + 1; i <= lvl; ++i) {
            update[i] = queue->head;
        }
        queue->level = lvl;
    }

    Node* new_node = nalloc(item);
    new_node->node_level = lvl;
    new_node->forward = new Node*[lvl + 1];
    for (int i = 0; i <= lvl; ++i) {
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }

    reply.success = true;
    reply.item.key = item.key;
    reply.item.value_size = item.value_size;
    if (item.value_size > 0 && item.value) {
        reply.item.value = malloc(item.value_size);
        memcpy(reply.item.value, item.value, item.value_size);
    }
    return reply;
}

Reply dequeue(Queue* queue) {
    Reply reply = {false, {0, nullptr, 0}};
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);

    Node* first = queue->head->forward[0];
    if (!first) return reply;

    for (int i = 0; i <= queue->level; ++i) {
        if (queue->head->forward[i] == first) {
            queue->head->forward[i] = first->forward[i];
        }
    }
    while (queue->level > 0 && queue->head->forward[queue->level] == nullptr) {
        --queue->level;
    }

    reply.success = true;
    reply.item.key = first->item.key;
    reply.item.value_size = first->item.value_size;
    if (first->item.value_size > 0 && first->item.value) {
        reply.item.value = malloc(first->item.value_size);
        memcpy(reply.item.value, first->item.value, first->item.value_size);
    }

    if (first->item.value) {
        free(first->item.value);
        first->item.value = nullptr;
    }
    nfree(first);
    return reply;
}

// ========== 범위 검색 (O(log n + m)) ==========

Queue* range(Queue* queue, Key start, Key end) {
    if (!queue) return nullptr;
    std::lock_guard<std::mutex> lock(queue->lock);

    Queue* new_q = init();
    Node* last[SKIPLIST_MAX_LEVEL];
    for (int i = 0; i < SKIPLIST_MAX_LEVEL; ++i) {
        last[i] = new_q->head;
    }

    Node* curr = queue->head;
    for (int i = queue->level; i >= 0; --i) {
        while (curr->forward[i] && curr->forward[i]->item.key > end) {
            curr = curr->forward[i];
        }
    }
    curr = curr->forward[0];

    int max_lvl = 0;
    while (curr && curr->item.key >= start) {
        Node* clone = new Node();
        clone->item.key = curr->item.key;
        clone->item.value_size = curr->item.value_size;
        if (curr->item.value_size > 0 && curr->item.value) {
            clone->item.value = malloc(curr->item.value_size);
            memcpy(clone->item.value, curr->item.value, curr->item.value_size);
        } else {
            clone->item.value = nullptr;
        }
        clone->node_level = curr->node_level;
        int lvl = clone->node_level;
        clone->forward = new Node*[lvl + 1];
        for (int i = 0; i <= lvl; ++i) {
            clone->forward[i] = nullptr;
        }
        for (int i = 0; i <= lvl; ++i) {
            last[i]->forward[i] = clone;
            last[i] = clone;
        }
        if (lvl > max_lvl) max_lvl = lvl;
        curr = curr->forward[0];
    }
    new_q->level = max_lvl;
    return new_q;
}
