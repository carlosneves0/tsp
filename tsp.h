#ifndef __TSP_H__
#define __TSP_H__
#include <stdio.h>
// #include "list.h"

/**
 * tsp_t
 * The data that describe an problem of the TSP.
 */
struct tsp
{
	int n;
	int** cost;
};
typedef struct tsp tsp_t;

/**
 * tsp_message_t
 * The data that describe an problem of the TSP encoded
 * as a sequential block of int's.
 */
struct tsp_message
{
	int count;
	int* buffer;
};
typedef struct tsp_message tsp_message_t;

// /**
//  * tsp_search_tree_node_t
//  * The data that describe a node in the TSP's search tree.
//  */
// struct tsp_search_tree_node
// {
// 	int depth;
// 	vector_t* visited;
// 	vector_t* unvisited;
// };
// typedef struct tsp_search_tree_node tsp_search_tree_node_t;
//
// /**
//  * tsp_solution_t
//  * A tsp_solution_t is a search node that has been expanded completely.
//  */
// typedef struct tsp_search_tree_node_t tsp_solution_t;
//
// struct tsp_search_state
// {
// 	tsp_t* problem;
// 	tsp_solution_t* optimum;
// 	list_t* memory;
// };
// typedef struct tsp_search_state tsp_search_state_t;

tsp_t* tsp_new(FILE* stream);
void tsp_del(tsp_t* problem);
tsp_message_t* tsp_encode(tsp_t* problem);
tsp_t* tsp_decode(tsp_message_t* message);
// void tsp_expand(tsp_t* problem, int depth = -1);
// int tsp_cost(tsp_t* problem, int u, int v);

void tsp_message_del(tsp_message_t* message);
char* tsp_message_buffer_to_string(tsp_message_t* message);

// tsp_search_tree_node_t* tsp_search_tree_node_new(tsp_search_tree_node_t* node, int x);
// tsp_search_tree_node_t* tsp_search_tree_node_cpy(tsp_search_tree_node_t* node);
// void tsp_search_tree_node_del(tsp_search_tree_node_t* node);
// void tsp_search_tree_node_print(tsp_search_tree_node_t* node);
// int* tsp_search_tree_node_children(tsp_search_tree_node_t* node);

#endif
