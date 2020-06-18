#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "tsp.h"

const int MASTER_RANK = 0;
tsp_t* input(int argc, char** argv);

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	char my_node[MPI_MAX_PROCESSOR_NAME]; int my_node_length;
	MPI_Get_processor_name(my_node, &my_node_length);

	if (my_rank == MASTER_RANK)
	{
		tsp_t* problem = input(argc, argv);

		int nproc;
		MPI_Comm_size(MPI_COMM_WORLD, &nproc);

		printf("[%s][%d] nproc = %d\n", my_node, my_rank, nproc); fflush(stdout);

		tsp_del(problem);
	}
	else
	{
		printf("[%s][%d] Hi there!\n", my_node, my_rank);
		fflush(stdout);
	}

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

	// /** Solve this instance of the TSP */
	// path_t* solution = tsp_solve(problem);
	//
	// /** stdout */
	// printf("%u\n", solution->cost);
	// path_print(solution);
	//
	// /** free */
	// path_del(solution);

	MPI_Finalize();
	return 0;
}

tsp_t* input(int argc, char** argv)
{
	/** Handle argc and argv */
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s input-file.txt\n", argv[0]);
		MPI_Finalize();
		exit(1);
	}

	/** Read the TSP from "input-file.txt" */
	FILE* input_file = fopen(argv[1], "r");
	if (!input_file)
	{
		fprintf(stderr, "Failed to open file \"%s\" in reading mode\n", argv[1]);
		MPI_Finalize();
		exit(1);
	}
	tsp_t* problem = tsp_new(input_file);
	fclose(input_file);

	return problem;
}
