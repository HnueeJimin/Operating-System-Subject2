#include <iostream>
#include <thread>
#include <atomic>
#include "queue.h"

using namespace std;

// 다중 클라이언트 테스트
// 설명: 아래 요청(Operation, Request)을 처리할 때
// 큐의 Item은 void*이므로 간단하게 받을 수 있음

#define REQUEST_PER_CLINET	10000

atomic<int> sum_key = 0;
atomic<int> sum_value = 0;
//atomic<double> response_time_tot = 0.0;

typedef enum {
	GET,
	SET,
	GETRANGE
} Operation;

typedef struct {
	Operation op;
	Item item;
} Request;

void client_func(Queue* queue, Request requests[], int n_request) {
	Reply reply = { false, 0 };

	// start_time = .....

	for (int i = 0; i < n_request; i++) {
		if (requests[i].op == GET) {
			reply = dequeue(queue);
		}
		else { // SET
			reply = enqueue(queue, requests[i].item);
		}

		if (reply.success) {
			// 응답이 성공했을 경우 key, value 누적 계산 (아무 의미 없는 연산)
			sum_key += reply.item.key;
			sum_value += (int)reply.item.value; // void*에서 다시 int로 변환

			// 응답된 key, value를 출력
			// ...생략...
		}
		else {
			// noop
		}
	}

	// 추후 필요 시 처리 시간 계산 코드
	//
	// elapsed_time = finish_time - start_time;
	// finish_time = ....;
	// average_response_time = elapsed_time / REQUEST_PER_CLIENT;
	// printf(...average_response_time of client1 = .....);
	// response_time_tot += finish_time - start_time;
}

int main(void) {
	srand((unsigned int)time(NULL));

	// 요청 배열 생성 (GETRANGE는 구현 생략
	Request requests[REQUEST_PER_CLINET];
	for (int i = 0; i < REQUEST_PER_CLINET / 2; i++) {
		requests[i].op = SET;
		requests[i].item.key = i;
		requests[i].item.value = (void*)(rand() % 1000000);
	}
	for (int i = REQUEST_PER_CLINET / 2; i < REQUEST_PER_CLINET; i++) {
		requests[i].op = GET;
	}

	Queue* queue = init();
	//if (queue == NULL) return 0;

	// 단일 스레드 클라이언트 실행, 추후 다중 클라이언트로 확장 가능
	thread client = thread(client_func, queue, requests, REQUEST_PER_CLINET);
	client.join();

	release(queue);

	// 응답된 항목들의 합산 출력
	cout << "sum of returned keys = " << sum_key << endl;
	cout << "sum of returned values = " << sum_value << endl;

	// 추후 필요 시 전체 평균 응답 시간 계산 코드 추가
	// total_average_response_time = total_response_time / n_client;
	// printf("total average response time = ....");
	return 0;
}