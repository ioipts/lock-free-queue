#ifndef AXISQUEUE_H_
#define AXISQUEUE_H_

/**
* @file axisqueue.h
* @brief Simple lock-free and wait-free circular queue  
*
* - single producer and single consumer.
* - multiple producer and single consumer.
* - multiple producer and multiple consumer.
*
*
* Inline functions.
* Optimized for speed. 
* No need for semaphore. 
* Require C/C++11
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
	unsigned int num_producer;
	unsigned int num_consumer;
	std::atomic<unsigned int> entries;
	std::atomic<unsigned int> first_res;
	std::atomic<unsigned int> last_res;
	std::atomic<unsigned int> first;
	std::atomic<unsigned int> last;
	QUEUETYPE* data;
} PACKED;
#pragma pack(pop)

/**
* init a single-producer-single-consumer queue
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
	q->first_res = q->first = q->last = 0;
	q->mask = q->size - 1;
	q->data = (QUEUETYPE*)ALLOCMEM(sizeof(QUEUETYPE) * q->size);
	if (q->data == NULL) { FREEMEM(q); return NULL; }
	return q;
}

/**
* destroy the queue
*/
inline void destroyqueue(axisqueue q)
{
	FREEMEM(q->data);
	FREEMEM(q);
}

/**
* Single producer, single consumer
* @param q queue
* @param v value 
* @return false if the queue is full
*/
inline bool enqueue(axisqueue q,QUEUETYPE v) {
	unsigned int nlast = (q->last.load(std::memory_order_acquire) + 1) & q->mask;
	if (nlast == q->first.load(std::memory_order_acquire)) return false;
	q->data[q->last] = v;
//#if defined(__GNUC__)
//	asm volatile("sfence" ::: "memory");
//#endif
	atomic_thread_fence(std::memory_order_release);
	q->last.store(nlast, std::memory_order_release);
	return true;
}

/**
* Single producer, single consumer
* @param q queue
* @param v value 
* @return true if not empty
*/
inline bool dequeue(axisqueue q, QUEUETYPE* v) {
	unsigned int nfirst = (q->first.load(std::memory_order_acquire) + 1) & q->mask;
	if (q->first == q->last.load(std::memory_order_acquire)) return false;
	*(v) = q->data[q->first];
//#if defined(__GNUC__)
//	asm volatile("sfence" ::: "memory");
//#endif
	atomic_thread_fence(std::memory_order_release);
	q->first.store(nfirst, std::memory_order_release);
	return true;
}

/**
* init a multiple producer queue
* expand the size to be power of two
*/
inline axisqueue initmultiplequeue(unsigned int size,unsigned int num_producer,unsigned int num_consumer)
{
	axisqueue q;
	q = (axisqueue)ALLOCMEM(sizeof(struct AxisQueueS));
	if (q == NULL) return NULL;
	unsigned int sizePowerOfTwo = 2;
	for (unsigned int i = 0; i < (sizeof(unsigned int) << 3); i++)
	{
		if (sizePowerOfTwo >= size) break;
		sizePowerOfTwo = sizePowerOfTwo << 1;
	}
	q->size = sizePowerOfTwo;
	q->num_producer = num_producer;
	q->num_consumer = num_consumer;
	q->entries = 0;
	q->first_res = q->last_res = q->first = q->last = 0;
	q->mask = q->size - 1;
	q->data = (QUEUETYPE*)ALLOCMEM(sizeof(QUEUETYPE) * q->size);
	if (q->data == NULL) { FREEMEM(q); return NULL; }
	return q;
}

/**
* multiple producer
* @param q queue
* @param v value
* @return false if the queue is full
*/
inline bool multipleenqueue(axisqueue q, QUEUETYPE v) {
	if (q->entries.load(std::memory_order_acquire)+q->num_producer >= q->size) return false;
	//reserve and wait
	unsigned int lastr = q->last_res.fetch_add(1);
	q->last_res.fetch_and(q->mask);
	lastr &= q->mask;
	while (lastr != q->last.load(std::memory_order_acquire)) ;
	q->data[q->last] = v;
	q->last.store((q->last+1) & q->mask, std::memory_order_release);
//#if defined(__GNUC__)
//	asm volatile("sfence" ::: "memory");
//#endif
	atomic_thread_fence(std::memory_order_release);
	q->entries++;
	return true;
}

/**
* multiple producer, single consumer
* @param q queue
* @param v value
* @return true if not empty
*/
inline bool singledequeue(axisqueue q, QUEUETYPE* v) {
	unsigned int nfirst = (q->first.load(std::memory_order_acquire) + 1) & q->mask;
	if (q->entries==0) return false;
	*(v) = q->data[q->first];
	q->first.store(nfirst, std::memory_order_release);
//#if defined(__GNUC__)
//	asm volatile("sfence" ::: "memory");
//#endif
	atomic_thread_fence(std::memory_order_release);
	q->entries--;
	return true;
}

/**
* multiple producer, multiple consumer (other thread)
* @param q queue
* @param v value
* @return true if not empty
*/
inline bool multipledequeue(axisqueue q, QUEUETYPE* v) {
	if (q->entries<=q->num_consumer) return false;
	//reserve and wait
	unsigned int firstr = q->first_res.fetch_add(1);
	q->first_res.fetch_and(q->mask);
	firstr &= q->mask;
	while (firstr != q->first.load(std::memory_order_acquire)) ;
	*(v) = q->data[q->first];
	q->first.store((q->first+1) & q->mask, std::memory_order_release);
//#if defined(__GNUC__)
//	asm volatile("sfence" ::: "memory");
//#endif
	atomic_thread_fence(std::memory_order_release);
	q->entries--;
	return true;
}

/**
* multiple producer, multiple consumer (primary thread)
* @param q queue
* @param v value
* @return true if not empty
*/
inline bool mastermultipledequeue(axisqueue q, QUEUETYPE* v) {
	if (q->entries==0) return false;
	//reserve and wait
	unsigned int firstr = q->first_res.fetch_add(1);
	q->first_res.fetch_and(q->mask);
	firstr &= q->mask;
	while (firstr != q->first.load(std::memory_order_acquire)) ;
	*(v) = q->data[q->first];
	q->first.store((q->first+1) & q->mask, std::memory_order_release);
//#if defined(__GNUC__)
//	asm volatile("sfence" ::: "memory");
//#endif
	atomic_thread_fence(std::memory_order_release);
	q->entries--;
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


