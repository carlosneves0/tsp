#include <stdlib.h>
#include <omp.h>
#include "compute.h"

tsp_solution_t* compute(tsp_search_t* global_search, int ncores)
{
	/**
	 * Expand the global_search list if the are too few nodes.
	 * This will only happen for small values of N.
	 * I don't think this will become the bottleneck.
	 */
	while (global_search->list->length > 0 && global_search->list->length < ncores)
		tsp_search_iterate(global_search, TSP_SEARCH_BREADTH_FIRST);

	tsp_search_t** all_local_searches = (tsp_search_t**) malloc(ncores * sizeof(tsp_search_t*));
	for (int core = 0; core < ncores; core++)
	{
		all_local_searches[core] = (tsp_search_t*) malloc(sizeof(tsp_search_t));
		all_local_searches[core]->problem = global_search->problem;
		all_local_searches[core]->optimum = tsp_solution_cpy(global_search->optimum);
		all_local_searches[core]->list = list_new(NULL);
	}

	int core = 0;
	while (global_search->list->length)
	{
		list_enqueue(all_local_searches[core]->list, list_dequeue(global_search->list));
		core = (core + 1) % ncores;
	}

	/**
	 * Expand search tree nodes depth-first so as to try and save some RAM.
	 * https://www.quora.com/Why-is-DFS-usually-more-space-efficient-than-BFS
	 */
	#pragma omp parallel num_threads(ncores)
	{
		int my_core = omp_get_thread_num();
		tsp_search_t* my_local_search = all_local_searches[my_core];
		while (my_local_search->list->length)
			tsp_search_iterate(my_local_search, TSP_SEARCH_DEPTH_FIRST);
	}

	tsp_solution_t* global_optimum = all_local_searches[0]->optimum;
	for (int core = 1; core < ncores; core++)
		if (!global_optimum)
			global_optimum = all_local_searches[core]->optimum;
		else if (all_local_searches[core]->optimum)
			if (all_local_searches[core]->optimum->cost < global_optimum->cost)
				global_optimum = all_local_searches[core]->optimum;

	if (global_search->optimum)
		tsp_solution_del(global_search->optimum);
	global_search->optimum = tsp_solution_cpy(global_optimum);

	char my_local_optimum_string[TSP_SOLUTION_STRING_MAX];
	tsp_solution_to_string(global_search->optimum, my_local_optimum_string);

	for (int core = 0; core < ncores; core++)
	{
		if (all_local_searches[core]->optimum)
			tsp_solution_del(all_local_searches[core]->optimum);
		list_del(all_local_searches[core]->list);
		free(all_local_searches[core]);
	}
	free(all_local_searches);

	return global_search->optimum;
}
