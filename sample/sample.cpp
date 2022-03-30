#include "axisqueue.h"
#include <thread>
#include <string.h>

#define TESTNUM 10000000

axisqueue q;
int exitflag=0;

void writerthread()
{
	int i=0;
	while (i<TESTNUM) {
		char* data=(char*)malloc(16);
		strcpy(data,"data");
		if (!enqueue(q, (QUEUETYPE)data)) {
			free(data);
		}
		i++;
	}
	exitflag++;
}

void readerthread()
{
	while (exitflag==0) {
		char* v;
		if (singledequeue(q, (QUEUETYPE*)&v)) {
			if (strcmp(v,"data")!=0) printf("error\n");
			free(v);
		}
	}
}

void multiplewriterthread()
{
	int i=0;
	while (i<TESTNUM) {
		char* data=(char*)malloc(16);
		strcpy(data,"data");
		if (!multipleenqueue(q, (QUEUETYPE)data)) {
			free(data);
		}
		i++;
	}
	exitflag++;
}

void singlereaderthread()
{
	while (exitflag<3) {
		char* v;
		if (singledequeue(q, (QUEUETYPE*)&v)) {
			if (strcmp(v,"data")!=0) printf("error\n");
			free(v);
		}
	}
}

void mastermultiplereaderthread()
{
	while (exitflag<3) {
		char* v;
		if (mastermultipledequeue(q, (QUEUETYPE*)&v)) {
			if (strcmp(v,"data")!=0) printf("error\n");
			free(v);
		}
	}
}

void multiplereaderthread()
{
	while (exitflag<3) {
		char* v;
		if (multipledequeue(q, (QUEUETYPE*)&v)) {
			if (strcmp(v,"data")!=0) printf("error\n");
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
	q = initmultiplequeue(10,3,1);
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
	q = initmultiplequeue(10,3,3);
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
	//testqueue();
	testmultiplesinglequeue();
	//testmultiplemultiplequeue();
	printf("completed\n");
	return 0;
}