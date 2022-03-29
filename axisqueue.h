#ifndef AXISQUEUE_H_
#define AXISQUEUE_H_

/**
* @file axisqueue.h
* @brief Simple lock-free circular queue for one producer and one consumer.
*
* Inline functions.
* Optimized for speed. 
* No need for semaphore. 
* C Header only.
*/

#include <stdio.h>
#include <stdlib.h>
#include <atomic>

/**
* Configure
*/
#define QUEUETYPE int

#define ALLOCMEM malloc
#define FREEMEM free

typedef struct AxisQueueS* axisqueue;
#pragma pack(push,1)
struct AxisQueueS
{
	unsigned int size;
	unsigned int mask;
	std::atomic<unsigned int> first;
	std::atomic<unsigned int> last;
	QUEUETYPE* data;
} PACKED;
#pragma pack(pop)

/**
* init a queue
* expand the size to be power of two
*/
inline axisqueue initqueue(unsigned int size)
{
	axisqueue q;
	q = (axisqueue)ALLOCMEM(sizeof(struct AxisQueueS));
	if (q == NULL) return NULL;
	unsigned int sizePowerOfTwo = 2;
	for (unsigned int i = 0; i < (sizeof(unsigned int)<<3); i++)
	{
		if (sizePowerOfTwo >= size) break;
		sizePowerOfTwo = sizePowerOfTwo << 1;
	}
	q->size = sizePowerOfTwo;
	q->first = q->last = 0;
	q->mask = q->size - 1;
	q->data = (QUEUETYPE*)ALLOCMEM(sizeof(QUEUETYPE) * q->size);
	if (q->data == NULL) { FREEMEM(q); return NULL; }
	return q;
}

/**
* destroy
*/
inline void destroyqueue(axisqueue q)
{
	FREEMEM(q->data);
	FREEMEM(q);
}

/**
* @param q queue
* @param v value 
* @return false if the queue is full
*/
inline bool enqueue(axisqueue q,QUEUETYPE v) {
	unsigned int nlast = (q->last.load(std::memory_order_acquire) + 1) & q->mask;
	if (nlast == q->first) return false;
	q->data[q->last] = v;
	atomic_thread_fence(std::memory_order_release);
#if defined(__GNUC__)
	asm volatile("" ::: "memory");
#endif
	q->last.store(nlast, std::memory_order_relaxed);
	return true;
}

/**
* @param q queue
* @param v value 
* @return true if not empty
*/
inline bool dequeue(axisqueue q, QUEUETYPE* v) {
	unsigned int nfirst = (q->first.load(std::memory_order_relaxed) + 1) & q->mask;
	if (q->first.load(std::memory_order_relaxed) == q->last.load(std::memory_order_relaxed)) return false;
	atomic_thread_fence(std::memory_order_acquire);
	*(v) = q->data[q->first];
#if defined(__GNUC__)
	asm volatile("" ::: "memory");
#endif
	q->first = nfirst
	return true;
}

/**
* called by reader thread only
*/
inline void clearqueue(axisqueue q)
{
	q->first.store(q->last,std::memory_order_relaxed);
}

#endif 
