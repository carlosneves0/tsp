#ifndef __TSP_H__
#define __TSP_H__
#include <stdio.h>
#include "queue.h"

typedef unsigned int uint;

/** tsp_t
 * The data that describe an instance of the TSP.
 */
struct tsp
{
	uint order; // The order of the square matrix "cost"
	uint** cost; // "Cost" square matrix
};
typedef struct tsp tsp_t;

/** path_t
 * The data that describe a path in the TSP's graph.
 */
struct path
{
	uint length, cost;
	uint* nodes;
};
typedef struct path path_t;

tsp_t* tsp_new(FILE* file);
void tsp_del(tsp_t* instance);
path_t* tsp_solve(tsp_t* instance);

path_t* path_new(path_t* p, uint x);
path_t* path_cpy(path_t* p);
void path_del(path_t* p);
void path_print(path_t* p);
uint* path_children(path_t* p);

#endif
