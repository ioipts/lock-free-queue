#include "axisqueue.h"
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <atomic>

#define TESTNUM 10000000

#if defined(_MSC_VER)
#pragma warning( disable : 4996 )	// disable deprecated warning for Windows
#define _CRT_SECURE_NO_WARNINGS		// disable deprecated warning for Windows
#endif

axisqueue q;
int exitflag = 0;
std::atomic<unsigned int> countwriter;
std::atomic<unsigned int> countreader;

void writerthread()
{
	int i = 0;
	while (i < TESTNUM) {
		char* data = (char*)malloc(16);
		strcpy(data, "data");
		if (!enqueue(q, (QUEUETYPE)data)) {
			free(data);
		}
		else countwriter++;
		i++;
	}
	exitflag++;
}

void readerthread()
{
	char* v;
	while (exitflag == 0) {
		if (dequeue(q, (QUEUETYPE*)&v)) {
			if (strcmp(v, "data") != 0) printf("error\n");
			else countreader++;
			free(v);
		}
	}
	while (dequeue(q, (QUEUETYPE*)&v)) {
		free(v);
		countreader++;
	}
}

void multiplewriterthread()
{
	int i = 0;
	while (i < TESTNUM) {
		char* data = (char*)malloc(16);
		strcpy(data, "data");
		if (!multipleenqueue(q, (QUEUETYPE)data)) {
			free(data);
		}
		else countwriter++;
		i++;
	}
	exitflag++;
}

void singlereaderthread()
{
	char* v;
	while (exitflag < 3) {
		if (singledequeue(q, (QUEUETYPE*)&v)) {
			if (strcmp(v, "data") != 0) printf("error\n");
			else countreader++;
			free(v);
		}
	}
	while (singledequeue(q, (QUEUETYPE*)&v)) {
		free(v);
		countreader++;
	}
}

void mastermultiplereaderthread()
{
	char* v;
	while (exitflag < 3) {
		if (mastermultipledequeue(q, (QUEUETYPE*)&v)) {
			if (strcmp(v, "data") != 0) printf("error\n");
			else countreader++;
			free(v);
		}
		std::this_thread::sleep_for(std::chrono::nanoseconds(1000000));
	}
	while (mastermultipledequeue(q, (QUEUETYPE*)&v)) {
		free(v);
		countreader++;
	}
}

void multiplereaderthread()
{
	while (exitflag < 3) {
		char* v;
		if (multipledequeue(q, (QUEUETYPE*)&v)) {
			if (strcmp(v, "data") != 0) printf("error\n");
			else countreader++;
			free(v);
		}
	}
}

void testqueue()
{
	q = initqueue(3);
	std::thread r1(readerthread);
	std::thread w1(writerthread);
	w1.join();
	r1.join();
	destroyqueue(q);
}

void testmultiplesinglequeue()
{
	q = initmultiplequeue(10, 3, 1);
	std::thread r1(singlereaderthread);
	std::thread w1(multiplewriterthread);
	std::thread w2(multiplewriterthread);
	std::thread w3(multiplewriterthread);
	w1.join();
	w2.join();
	w3.join();
	r1.join();
	destroyqueue(q);
}

void testmultiplemultiplequeue()
{
	q = initmultiplequeue(10, 3, 3);
	std::thread r1(mastermultiplereaderthread);
	std::thread r2(multiplereaderthread);
	std::thread r3(multiplereaderthread);
	std::thread w1(multiplewriterthread);
	std::thread w2(multiplewriterthread);
	std::thread w3(multiplewriterthread);
	w1.join();
	w2.join();
	w3.join();
	r3.join();
	r2.join();
	r1.join();
	destroyqueue(q);
}

int main(int argc, char** argv)
{
	countreader=0;
	countwriter=0;
	//testqueue();
	//testmultiplesinglequeue();
	testmultiplemultiplequeue();
	if (countwriter != countreader) printf("not completed\n");
	else  printf("completed\n");
	return 0;
}


