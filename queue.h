#ifndef _QUEUE_H // header guard
#define _QUEUE_H 

// ========== 큐 관련 함수 선언 ==========
// 주의: 일반적으로 queue_init()처럼 함수 이름에 prefix를 붙이지만
// 본 구현에서는 단순함을 위해 짧은 이름을 사용함

#include "qtype.h"

// 큐 초기화 및 해제
Queue* init(void);
void release(Queue* queue);

// ========== concurrent operations ==========

// 노드 생성 및 초기화, 해제, 복제
Node* nalloc(Item item);
void nfree(Node* node);
Node* nclone(Node* node);

// (key, item) 쌍을 큐에 삽입
// 성공 시: success = true, 실패 시: success = false
Reply enqueue(Queue* queue, Item item);

// 큐에서 맨 앞의 항목(가장 오래된 항목)을 제거하여 반환
// 큐가 비어있으면: success = false
// 항목이 있으면: success = true, item = 맨 앞 항목
Reply dequeue(Queue* queue);

// start <= key <= end 조건을 만족하는 항목들을 찾아
// 새로운 큐에 복사하여 반환
Queue* range(Queue* queue, Key start, Key end);

#endif