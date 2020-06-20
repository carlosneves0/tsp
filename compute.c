#include "compute.h"

tsp_solution_t* compute(tsp_search_t* search)
{
	/**
	 * Expand search tree nodes depth-first so as to try and save some RAM.
	 * https://www.quora.com/Why-is-DFS-usually-more-space-efficient-than-BFS
	 */
	// **OMP**
	// 	// TODO: alloc one search to one thread and go depth first.
	while (search->list->length)
		tsp_search_iterate(search, TSP_SEARCH_DEPTH_FIRST);

	return search->optimum;
}
