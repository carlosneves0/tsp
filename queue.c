#include <stdlib.h>
#include "queue.h"

queue_t* queue_new(void* data)
{
	queue_t* queue = malloc(sizeof(queue_t));
	queue->length = 1u;
	queue_node_t* first = malloc(sizeof(queue_node_t));
	first->data = data;
	first->next = NULL;
	queue->head = first;
	queue->tail = first;
	return queue;
}

void queue_del(queue_t* queue)
{
	queue_node_t* x = queue->head;
	while (x)
	{
		queue_node_t* del = x;
		x = x->next;
		if (del->data)
			free(del->data);
		free(del);
	}
	free(queue);
}

void* queue_pop(queue_t* queue)
{
	if (queue->length == 0u)
		return NULL;

	void* data = queue->head->data;
	if (queue->length == 1u)
	{
		free(queue->head);
		queue->head = queue->tail = NULL;
		queue->length = 0u;
	}
	else
	{
		queue_node_t* old_head = queue->head;
		queue->head = queue->head->next;
		free(old_head);
		queue->length--;
	}

	return data;
}

void queue_push(queue_t* queue, void* data)
{
	queue_node_t* new_node = malloc(sizeof(queue_node_t));
	new_node->data = data;
	new_node->next = NULL;

	if (queue->length == 0u)
	{
		queue->head = new_node;
		queue->tail = new_node;
		queue->length = 1u;
	}
	else
	{
		queue->tail->next = new_node;
		queue->tail = new_node;
		queue->length++;
	}
}
