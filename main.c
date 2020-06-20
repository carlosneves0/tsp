/**
 * GROUP 3
 * Carlos
 * Felipe
 * Ivan
 * Jonathan
 * Kevin
 *
 * This code solves the Travelling Salesman Problem (TSP) using MPI and OpenMP.
 * It was designed to run on a homogeneous cluster.
 *
 * Forgive us for the many "nodes" in the code:
 *   1. The cluster has many nodes.
 *   2. The cities in the TSP are nodes in a graph.
 *   3. Finally the search tree that enumerates all the solutions in the TSP is
 *   also composed of nodes.
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include "master.h"
#include "slave.h"
#include "debug.h"

/** These two need to be globals so debug() can work. */
int my_rank;
char my_node[MPI_MAX_PROCESSOR_NAME];
const int MASTER_RANK = 0;

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	int my_node_strlen;
	MPI_Get_processor_name(my_node, &my_node_strlen);

	int my_ncores = omp_get_num_procs();
	debug("main", "ncores = %d", my_ncores);

	if (my_rank == MASTER_RANK)
		master(argc, argv, MASTER_RANK, my_node, my_ncores);
	else
		slave(my_rank, my_node, my_ncores);

	MPI_Finalize();

	return 0;
}
