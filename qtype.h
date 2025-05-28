#ifndef _QTYPE_H  // header guard
#define _QTYPE_H

// ========== 큐 자료구조 관련 정의 ==========

typedef unsigned int Key;  // 데이터 식별용 키 (주로 정수형 사용)
typedef void* Value;       // 범용 데이터를 담기 위한 포인터 타입

typedef struct {
    Key key;
    Value value;
} Item;

typedef struct {
    bool success;   // true: 성공, false: 실패
    Item item;
    // 향후 필요시 필드 추가 가능
} Reply;

typedef struct node_t {
    Item item;
    struct node_t* next;
    // 향후 필요시 필드 추가 가능
} Node;

typedef struct {
    Node* head;
    Node* tail;
    // 향후 필요시 필드 추가 가능 (예: 크기 정보 등)
} Queue;

// 함수 선언은 별도 소스 또는 헤더에 정의 가능

#endif
