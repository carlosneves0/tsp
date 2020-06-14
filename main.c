#include <stdio.h>
#include "tsp.h"

int main(int argc, char** argv)
{
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

	/** TODO: solve this instance of the TSP */
	path_t* solution = tsp_solve(problem);

	/** TODO: stdout */
	printf("%u\n", solution->cost);
	path_print(solution);

	/** free */
	path_del(solution);
	tsp_del(problem);

	return 0;
}
