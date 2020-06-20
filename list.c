#include <stdlib.h>
#include "list.h"

list_t* list_new(void* data)
{
	list_t* list = (list_t*) malloc(sizeof(list_t));
	if (data)
	{
		list_node_t* initial_node = (list_node_t*) malloc(sizeof(list_node_t));
		initial_node->data = data;
		initial_node->next = NULL;
		list->head = initial_node;
		list->tail = initial_node;
		list->length = 1;
	}
	else
	{
		list->head = NULL;
		list->tail = NULL;
		list->length = 0;
	}
	return list;
}

void list_del(list_t* list)
{
	list_node_t* x = list->head;
	while (x)
	{
		list_node_t* del = x;
		x = x->next;
		if (del->data)
			free(del->data);
		free(del);
	}
	free(list);
}

void list_enqueue(list_t* list, void* data)
{
	list_node_t* new_node = (list_node_t*) malloc(sizeof(list_node_t));
	new_node->data = data;
	new_node->next = NULL;

	if (list->length == 0)
	{
		list->head = new_node;
		list->tail = new_node;
		list->length = 1;
	}
	else
	{
		list->tail->next = new_node;
		list->tail = new_node;
		list->length++;
	}
}

void* list_dequeue(list_t* list)
{
	if (list->length == 0)
		return NULL;

	void* data = list->head->data;

	if (list->length == 1)
	{
		free(list->head);
		list->head = list->tail = NULL;
		list->length = 0;
	}
	else
	{
		list_node_t* old_head = list->head;
		list->head = list->head->next;
		free(old_head);
		list->length--;
	}

	return data;
}

void list_stack(list_t* list, void* data)
{}

void* list_unstack(list_t* list)
{
	return NULL;
}
