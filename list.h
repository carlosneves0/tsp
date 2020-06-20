#ifndef __LIST_H__
#define __LIST_H__

struct list_node
{
	void* data;
	struct list_node* next;
};
typedef struct list_node list_node_t;

struct list
{
	int length;
	list_node_t* head;
	list_node_t* tail;
};
typedef struct list list_t;

list_t* list_new(void* data);
void list_del(list_t* list);

void list_enqueue(list_t* list, void* data);
void* list_dequeue(list_t* list);

void list_stack(list_t* list, void* data);
void* list_unstack(list_t* list);

#endif
