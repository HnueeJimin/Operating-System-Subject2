#ifndef _QTYPE_H  // header guard
#define _QTYPE_H
#include <mutex>

// ==========이 파일은 수정 가능==========
// 스킵 리스트로 구현

# define SKIPLIST_MAX_LEVEL 24 // key의 최대 레벨, 2^24 = 16,777,216
# define SKIPLIST_P 0.5

typedef unsigned int Key;  // 데이터 식별용 키 (주로 정수형 사용), 값이 클수록 높은 우선순위
typedef void* Value;       // 범용 데이터를 담기 위한 포인터 타입

typedef struct {
    Key key;
    Value value;
    int value_size;  // 포인터가 가리키는 메모리 공간의 크기 함께 전달
} Item;

typedef struct {
    bool success;   // true: 성공, false: 실패
    Item item;
    // 향후 필요시 필드 추가 가능
} Reply;

typedef struct node_t {
    Item item;
    struct node_t** forward; // [레벨 수] 만큼의 포인터 배열
    int node_level; // 현재 노드의 레벨
} Node;

typedef struct {
    Node* head; // 더미 노드, 가장 작은 키를 가진 노드
    int level; // 현재 스킵 리스트의 레벨
    std::mutex lock; // 스킵 리스트에 대한 동기화용 뮤텍스, 각 큐마다 하나씩 존재
} Queue; // queue.h 수정 못하므로 이름은 Queue로 유지

// 함수 선언은 별도 소스 또는 헤더에 정의 가능

#endif
