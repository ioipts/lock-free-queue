#include "axisqueue.h"
#include <thread>

#define TESTNUM 10000

axisqueue q;

void writerthread()
{
	QUEUETYPE i = 1;
	while (i <= TESTNUM) {
		if (enqueue(q, i)) i++;
	}
}

void readerthread()
{
	QUEUETYPE v=0;
	unsigned int sum = 0;
	unsigned int check = ((TESTNUM+1)*TESTNUM)>>1;
	while (sum!=check) {
		if (dequeue(q, &v)) {
			sum += v;
		}
	}
}

int main(int argc, char** argv)
{
	q = initqueue(3);
	std::thread t1(readerthread);
	std::thread t2(writerthread);
	t1.join();
	t2.join();
	destroyqueue(q);
	printf("completed\n");
	return 0;
}