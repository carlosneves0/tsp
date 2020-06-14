#include <stdio.h>
#include <stdlib.h>
#include "tsp.h"

problem_t problem;

// void input(tsp_t* root);
// void spawn(tsp_t* root);
// void scatter(tsp_t* root);
// void compute(tsp_t* root);
// void gather(tsp_t* root);
// void output(tsp_solution_t* solution);

int main(int argc, char** argv)
{
	// if (my_rank == MPI_MASTER)
	// {
	// 	tsp_t root = input(argc, argv);
	//
	// 	tsp_t* smaller_problems = split(root);
	//
	// 	tsp_t my_problem = scatter(smaller_problems);
	//
	// 	tsp_solution_t solutions[];
	//
	// 	solutions[0] = tsp_solve(root);
	//
	// 	gather(solutions);
	//
	// 	root_solution = merge(solutions);
	//
	// 	output(root_solution);
	// }
	// else
	// {
	// 	tsp_t my_problem = scatter(NULL);
	//
	// 	tsp_solution_t solution = tsp_solve(my_problem);
	//
	// 	gather(solution);
	// }

	/** argv */
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s input-file.txt\n", argv[0]);
		return 1;
	}

	/** read TSP from input-file.txt && malloc */
	FILE* input_file = fopen(argv[1], "r");
	if (!input_file)
	{
		fprintf(stderr, "Failed to open file \"%s\" in reading mode\n", argv[1]);
		return 1;
	}
	fscanf(input_file, "%u\n", &root.order);
	root.cost = malloc(root.order * sizeof(uint*));
	for (uint i = 0u; i < root.order; i++)
	{
		root.cost[i] = malloc(root.order * sizeof(uint));
		for (uint j = 0u; j < root.order; j++)
			fscanf(input_file, "%u", &root.cost[i][j]);
	}

	/** solve this instance of the TSP */
	// f(i, C) = minjєC {ci,j + f(j, C-{j})}
	f(&)

	/** TODO: stdout */
	// tsp_print(&root);

	/** free */
	for (uint i = 0u; i < root.order; i++)
		free(root.cost[i]);
	free(root.cost);

	return 0;
}

void tsp_print(tsp_t* problem)
{
	printf(">>> order = %u\n", problem->order);
	printf(">>> cost = ");
	for (uint i = 0u; i < problem->order; i++)
	{
		if (i != 0u)
			printf(">>>        ");
		for (uint j = 0u; j < problem->order; j++)
			printf("%u%s", problem->cost[i][j], (j < problem->order - 1 ? " " : ""));
		printf("\n");
	}
}
