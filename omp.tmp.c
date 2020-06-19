#include "tsp.h"

int main(void)
{
	// https://www.quora.com/Why-is-DFS-usually-more-space-efficient-than-BFS
	int initial_node = 0;	// "initial_city"
	tsp_search_state_t* initial = tsp_search_new(initial_node);

	int pre_compute_depth = 2;
	tsp_search_state_t* expanded = tsp_search_expand(problem, initial, TSP_BREADTH_FIRST, pre_compute_depth);
	return 0;
}
