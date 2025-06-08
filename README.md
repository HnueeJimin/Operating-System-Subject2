# 운영체제 과제2

## 목적: Thread-Safe-Queue를 만듦
```
-> 다수의 스레드가 동시에 접근하더라도 데이터의 무결성 및 일관성을 유지하도록 설계된 큐(Queue) 자료구조
-> 멀티스레딩 환경에서 큐를 사용할 때 경쟁 조건(Race Condition), 데이터 손상(Data Corruption) 등의 문제 방지
```
enqueue(): 큐에다가 삽입하는 함수
-> 문제: 여러 개의 스레드가 동시에 enqueue()를 호출할 때 포인터가 누구를 가리킬지 장담할 수 없음

**해결하고자 하는 것: enqueue()/dequeue()를 할 때, 문맥 교환이 발생하지 않도록(atomic 하도록)**

## 고려사항(조건)

- 내부 구조가 반드시 Linked List일 필요 X
- C++ 사용 시, cin 및 cout + thread 관련 기능(thread, atomic 등)만 사용
- queue.h 수정 불가 (제출 x)
- qtype.h 수정 가능 (제출 o)
- queue.cpp 큐 구현 (제출)
- main.c 테스트 코드 (제출 x)
- 새로 생성된 Queue*에는 item 요소들이 **깊은 복사**를 통해 저장되어야 함!
- 기존 큐(queue)의 node나 vaule가 변경 및 해제되더라도, 복사된 큐한테는 영향이 없어야 함
- 단일 큐 내에서는 key 값의 유일성을 보장해야하며, 서로 다른 큐에 대해서는 상관없음

## Parameters
- 0 <= key < 10,000,000 <-- 큐 크기 최소 ~ 최대 값 --> 한 마디로 이를 고려해서 설계해야 함
- client thread: 1~32 개
- item 크기: 1byte ~ 1KB <-- 이게 뭔데? 확인 필요

## Workflow
- 25.05.28, 과제 요구사항 정리
- 25.05.30, queue.cpp 보강
- 25.06.02, 컴파일 에러 수정, 기본적으로 make 시 올바르게 컴파일되도록 수정
- 25.06.06, range() 함수 구현
- 25.06.07, 디버깅용 코드 수정 (1. enqueue/dequeue 는 atomic하게 동작하는지? Deep Copy 검증)
- 25.06.08, 자료구조 변경(Linked List -> Skip List), 디버깅 코드 재수정