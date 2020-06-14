#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "tsp.h"
#include "queue.h"

uint _global_order;
uint** _global_cost;

tsp_t* tsp_new(char* filename)
{
	FILE* file = fopen(filename, "r");
	if (!file)
		return NULL;
	tsp_t* instance = (tsp_t*) malloc(sizeof(tsp_t));
	fscanf(file, "%u\n", &instance->order);
	instance->cost = (uint**) malloc(instance->order * sizeof(uint*));
	for (uint i = 0u; i < instance->order; i++)
	{
		instance->cost[i] = (uint*) malloc(instance->order * sizeof(uint));
		for (uint j = 0u; j < instance->order; j++)
			fscanf(file, "%u", &instance->cost[i][j]);
	}
	return instance;
}

void tsp_del(tsp_t* instance)
{
	for (uint i = 0u; i < instance->order; i++)
		free(instance->cost[i]);
	free(instance->cost);
	free(instance);
}

path_t* tsp_solve(tsp_t* instance)
{
	_global_order = instance->order;
	_global_cost = instance->cost;
	const uint INITIAL_NODE = 0u;
	const uint SOLUTION_MIN_LENGTH = instance->order + 1u;

	path_t* optimal = NULL;
	queue_t* queue = queue_new(path_new(NULL, INITIAL_NODE));
	while (queue->length)
	{
		path_t* p = queue_pop(queue);
		printf("FUCK OFF\n");
		path_print(p);
		printf("FUCK OFF\n");

		if (p->length == SOLUTION_MIN_LENGTH && p->cost < optimal->cost)
		{
			if (optimal)
				path_del(optimal);
			optimal = path_copy(p);
		}

		uint* children = path_children(p);
		for (uint* k = children; *k != UINT_MAX; k++)
			queue_push(queue, path_new(p, *k));

		path_del(p);
	}
	queue_del(queue);
	_global_cost = NULL;
	_global_order = UINT_MAX;
	return optimal;
	// ---
	// cost = instance->cost;
	//
	// uint v[instance->order];
	// for (uint i = 0u; i < instance->order; i++)
	// 	v[i] = i + 1u;
	// v[instance->order - 1u] = -1u;
	//
	// // setprint(v);
	// // uint w[instance->order - 1u];
	// // setsub(w, v, 2u);
	// // setprint(w);
	// //
	// // return 0u;
	// return tsp_solve_recursive(0u, v);
}

path_t* path_new(path_t* p, uint x)
{
	path_t* q = malloc(sizeof(path_t));
	if (p)
	{
		q->length = p->length + 1u;

		uint p_back = p->nodes[p->length - 1];
		q->cost = p->cost + _global_cost[p_back][x];

		q->nodes = malloc(q->length * sizeof(uint));
		for (uint i = 0u; i < p->length; i++)
		q->nodes[i] = p->nodes[i];
		q->nodes[q->length - 1] = x;
	}
	else
	{
		q->length = q->cost = 1u;
		q->nodes = malloc(sizeof(uint));
		q->nodes[0] = x;
	}
	return q;
}

void path_del(path_t* p)
{
	free(p->nodes);
	free(p);
}

void path_print(path_t* p)
{
	for (uint i = 0; i < p->length; i++)
		printf("%u%s", p->nodes[i], (i < p->length - 1 ? ", " : ""));
	printf("\n");
}

path_t* path_copy(path_t* p)
{
	path_t* q = malloc(sizeof(path_t));
	q->length = p->length;
	q->cost = p->cost;
	q->nodes = malloc(q->length * sizeof(uint));
	for (uint i = 0u; i < q->length; i++)
		q->nodes[i] = p->nodes[i];
	return q;
}

uint* path_children(path_t* p)
{
	if (p->length > _global_order)
		return NULL;
	else if (p->length == _global_order)
	{
		uint* children = malloc(2 * sizeof(uint));
		children[0] = 0u;
		children[1] = UINT_MAX;
		return children;
	}
	else
	{
		uint _children[_global_order], index = 0u;
		for (uint i = 0u; i < _global_order; i++)
		{
			int visited = 0;
			for (uint j = 0u; j < p->length; j++)
				if (p->nodes[j] == i)
					visited = 1;
			if (!visited)
				_children[index++] = i;
		}
		uint* children = malloc((index + 1) * sizeof(uint));
		for (uint i = 0u; i < index; i++)
			children[i] = _children[i];
		children[index] = UINT_MAX;
		return children;
	}
}

// uint** cost;
// uint tsp_solve_recursive(uint i, uint* v);
// uint setlen(uint* s);
// void setsub(uint* s, uint* t, uint i);
// void setprint(uint* s);
//
// uint tsp_solve_recursive(uint i, uint* v)
// {
// 	printf("f(i = %d, V = ", i);
// 	setprint(v);
// 	printf(")\n");
//
// 	if (setlen(v) == 1u)
// 	{
// 		uint j = v[0];
// 		return cost[i][j] + cost[j][0];
// 	}
// 	uint min_cost = UINT_MAX, mink = UINT_MAX;
// 	for (uint* k = v; *k != -1u; k++)
// 	{
// 		uint w[setlen(v)];
// 		setsub(w, v, *k); // W = V - {k}
// 		uint kcost = cost[i][*k] + tsp_solve_recursive(*k, w);
// 		if (kcost < min_cost)
// 		{
// 			min_cost = kcost;
// 			mink = *k;
// 		}
// 	}
// 	printf("mink = %u\n", mink);
// 	return min_cost;
// }
//
// uint setlen(uint* s)
// {
// 	uint* t = s;
// 	while (*t++ != -1u)
// 		;
// 	return t - s - 1u;
// }
//
// void setsub(uint* s, uint* t, uint i)
// {
// 	while (*t != -1u)
// 	{
// 		if (*t != i)
// 			*s++ = *t;
// 		t++;
// 	}
// 	*s = -1u;
// }
//
// void setprint(uint* s)
// {
// 	printf("{");
// 	int first = 1;
// 	while (*s != -1u)
// 	{
// 		if (first)
// 		{
// 			printf("%u", *s);
// 			first = 0;
// 		}
// 		else
// 			printf(", %u", *s);
// 		s++;
// 	}
// 	printf("}");
// }
//
// // extern problem_t problem;
// //
// // tsp_solution_t f(node_t i, set_t v)
// // {
// //   if (set_size(v) > 1)
// //   {
// //     return min{c[i][k] + f(k, v - {k}), for k in v};
// //   }
// //   else
// //   {
// //     j = v[0]
// //     return {
// //       cost = c[i][j] + c[j][0]
// //       path = {i, j, 0}
// //     }
// //   }
// // }
// //
// // void foo(void)
// // {
// //   solution s;
// //   queue_t q;
// //   while(!queue_empty(q))
// //   {
// //     x = queue_pop(q);
// //
// //   }
// // }
