#include <stdio.h>
#include "tsp.h"

int main(int argc, char** argv)
{
	// --- PSEUDOCODE ---
	// if (my_rank == MPI_MASTER)
	// {
	// 	tsp_t* problem = input(argc, argv);
	//
	// 	nproc = ask_mpi();
	//
	// 	spawn(nproc);
	//
	// 	ncores = gather_ncores();
	//
	// 	bcast(cost_matrix);
	//
	// 	tsp_t* subproblems = split(problem, nproc, ncores);
	//
	// 	tsp_t* my_subproblems = scatterv(subproblems);
	//
	// 	path_t* my_subsolutions;
	//
	//  #pragma omp shit
	// 	my_subsolutions[i] = tsp_solve(my_subproblems[i]);
	//
	// 	path_t* all_subsolutions = gatherv(my_subsolutions);
	//
	// 	path_t* solution = merge(all_subsolutions);
	//
	// 	output(solution);
	// }
	// else
	// {
	// 	gather_ncores();
	//
	// 	bcast(cost_matrix);
	//
	// 	tsp_t* my_subproblems = scatterv(subproblems);
	//
	// 	path_t* my_subsolutions;
	//
	//  #pragma omp shit
	// 	my_subsolutions[i] = tsp_solve(my_subproblems[i]);
	//
	// 	gatherv(my_subsolutions);
	// }
	// --- PSEUDOCODE ---

	/** Handle argc and argv */
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s input-file.txt\n", argv[0]);
		return 1;
	}

	/** Read the TSP from "input-file.txt" */
	tsp_t* problem = tsp_new(argv[1]);
	if (!problem)
	{
		fprintf(stderr, "Failed to open file \"%s\" in reading mode\n", argv[1]);
		return 1;
	}

	/** Solve this instance of the TSP */
	path_t* solution = tsp_solve(problem);

	/** stdout */
	printf("%u\n", solution->cost);
	path_print(solution);

	/** free */
	path_del(solution);
	tsp_del(problem);

	return 0;
}
