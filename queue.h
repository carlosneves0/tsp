#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef unsigned int uint;

struct queue_node
{
	void* data;
	struct queue_node* next;
};
typedef struct queue_node queue_node_t;

struct queue
{
	uint length;
	queue_node_t* head;
	queue_node_t* tail;
};
typedef struct queue queue_t;

queue_t* queue_new(void* data);
void queue_del(queue_t* queue);
void* queue_pop(queue_t* queue);
void queue_push(queue_t* queue, void* data);

#endif
