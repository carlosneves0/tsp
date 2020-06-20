#ifndef __TSP_H__
#define __TSP_H__
#include <stdio.h>
#include "list.h"
#include "message.h"

/**
 * tsp_t
 * The data that describe an instance of the TSP.
 */
struct tsp
{
	int n;
	int** cost;
};
typedef struct tsp tsp_t;
tsp_t* tsp_new(FILE* stream);
void tsp_del(tsp_t* problem);
message_t* tsp_encode(tsp_t* problem);
tsp_t* tsp_decode(message_t* message);

/**
 * tsp_solution_t
 * A tsp_solution_t is a circuit in the TSP's graph.
 */
struct tsp_solution
{
	int cost;
	int* circuit;
};
typedef struct tsp_solution tsp_solution_t;

/**
 * tsp_search_node_t
 * The data that describe a node in the TSP's search tree.
 */
struct tsp_search_node
{
	int depth;
	int* visited;
	int* unvisited;
};
typedef struct tsp_search_node tsp_search_node_t;
tsp_search_node_t* tsp_search_node_new(tsp_search_node_t* parent, int new_visited_node, const int N);
void tsp_search_node_del(tsp_search_node_t* node);
message_t* tsp_search_node_encode(tsp_t* problem, tsp_search_node_t* node);
tsp_search_node_t* tsp_search_node_decode(message_t* message);

/**
 * tsp_search_t
 * This struct describes the state of a **no early cut-offs** search on the
 * TSP's graph. The execution of the search itself generates a tree structure.
 */
struct tsp_search
{
	tsp_t* problem;
	tsp_solution_t* optimum;
	list_t* list;
};
typedef struct tsp_search tsp_search_t;
/**
 * tsp_search_strategy_t
 * Expand new children using BFS or DFS?
 */
enum tsp_search_strategy { TSP_SEARCH_BREADTH_FIRST, TSP_SEARCH_DEPTH_FIRST };
typedef enum tsp_search_strategy tsp_search_strategy_t;
tsp_search_t* tsp_search_new(tsp_t* problem, int initial_node);
void tsp_search_del(tsp_search_t* search);
void tsp_search_iterate(tsp_search_t* search, tsp_search_strategy_t strategy);






// OLD: void tsp_expand(tsp_t* problem, int depth = -1);
// OLD: int tsp_cost(tsp_t* problem, int u, int v);

// OLD: tsp_search_node_t* tsp_search_node_new(tsp_search_node_t* node, int x);
// OLD: tsp_search_node_t* tsp_search_node_cpy(tsp_search_node_t* node);
// OLD: void tsp_search_node_del(tsp_search_node_t* node);
// OLD: void tsp_search_node_print(tsp_search_node_t* node);
// OLD: int* tsp_search_node_children(tsp_search_node_t* node);

#endif
