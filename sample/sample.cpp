#include "axisqueue.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <ctime> 

#define TESTNUM 1000000

#if defined(_MSC_VER)
// disable deprecated warning for Windows
#pragma warning( disable : 4996 )	
#define _CRT_SECURE_NO_WARNINGS		
#endif

axisqueue q;
int exitflag;
std::atomic<unsigned int> countwriter;
std::atomic<unsigned int> countreader;
std::mutex m;

void writerlockthread()
{
	int i = 0;
	while (i < TESTNUM) {
		char* data = (char*)malloc(16);
		strcpy(data, "data");
		m.lock();
		bool ret = enqueue(q, (QUEUETYPE)data);
		m.unlock();
		if (!ret) {
			free(data);
		}
		else countwriter++;
		i++;
	}
	exitflag++;

}

void readerlockthread()
{
	char* v;
	while (exitflag == 0) {
		m.lock();
		bool ret = dequeue(q, (QUEUETYPE*)&v);
		m.unlock();
		if (ret) {
			if (strcmp(v, "data") != 0) printf("error\n");
			else countreader++;
			free(v);
		}
	}
	m.lock();
	while (dequeue(q, (QUEUETYPE*)&v)) {
		free(v);
		countreader++;
	}
	m.unlock();
}

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

void singlewriterthread()
{
	int i = 0;
	while (i < TESTNUM) {
		char* data = (char*)malloc(16);
		strcpy(data, "data");
		if (!singleenqueue(q, (QUEUETYPE)data)) {
			free(data);
		}
		else countwriter++;
		i++;
	}
	exitflag = 3;
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
		std::this_thread::sleep_for(std::chrono::nanoseconds(10000000));
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
		std::this_thread::sleep_for(std::chrono::nanoseconds(1000000));
	}
}

void testsinglesinglequeue()
{
	exitflag = 0;
	q = initqueue(3);
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	std::thread r1(readerthread);
	std::thread w1(writerthread);
	w1.join();
	r1.join();
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	destroyqueue(q);
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "lock free elapsed time: " << elapsed_seconds.count() << "s\n";
}

void testmultiplesinglequeue()
{
	exitflag = 0;
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

void testsinglemultiplequeue()
{
	exitflag = 0;
	q = initmultiplequeue(10, 1, 3);
	std::thread r1(mastermultiplereaderthread);
	std::thread r2(multiplereaderthread);
	std::thread r3(multiplereaderthread);
	std::thread w1(singlewriterthread);
	w1.join();
	r1.join();
	r2.join();
	r3.join();
	destroyqueue(q);
}

void testmultiplemultiplequeue()
{
	exitflag = 0;
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

void testlockqueue()
{
	exitflag = 0;
	q = initqueue(3);
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	std::thread r1(readerlockthread);
	std::thread w1(writerlockthread);
	w1.join();
	r1.join();
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	destroyqueue(q);
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "mutex lock elapsed time: " << elapsed_seconds.count() << "s\n";
}

int main(int argc, char** argv)
{
	countreader = 0;
	countwriter = 0;
	testlockqueue();
	testsinglesinglequeue();
	testsinglemultiplequeue();
	testmultiplesinglequeue();
	testmultiplemultiplequeue();
	if (countwriter != countreader) printf("not completed\n");
	else  printf("completed\n");
	return 0;
}


