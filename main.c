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
#include "tsp.h"
#include "debug.h"

const int MASTER_RANK = 0;
tsp_t* input(int argc, char** argv);
void master(char* my_node, int my_ncores, tsp_t* problem);
void slave(int my_rank, char* my_node, int my_ncores);
// tsp_solution_t* compute(tsp_t* problem, ? my_tsp_search_states);

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	char my_node[MPI_MAX_PROCESSOR_NAME]; int my_node_length;
	MPI_Get_processor_name(my_node, &my_node_length);

	int my_ncores = omp_get_num_procs();
	debug("[%s][%d] ncores = %d\n", my_node, my_rank, my_ncores);

	if (my_rank == MASTER_RANK)
		master(my_node, my_ncores, input(argc, argv));
	else
		slave(my_rank, my_node, my_ncores);

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

void master(char* my_node, int my_ncores, tsp_t* problem)
{
	int my_rank = MASTER_RANK;

	int nproc;
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	debug("[%s][%d] nproc = %d\n", my_node, my_rank, nproc);

	/** Broadcast the cost matrix's order and data to each slave process. */
	tsp_message_t* message = tsp_encode(problem); int count = 1;
	MPI_Bcast(&message->count, count, MPI_INT, my_rank, MPI_COMM_WORLD);
	debug("[%s][%d][bcast0::send] message->count = %d\n", my_node, my_rank, message->count);

	MPI_Bcast(message->buffer, message->count, MPI_INT, my_rank, MPI_COMM_WORLD);
	debug("[%s][%d][bcast1::send] message->buffer = %s\n", my_node, my_rank, tsp_message_buffer_to_string(message));

	// /** Pre-compute smaller tasks that can be distributed to each node. */
	// int initial_node = 0;	// "initial_city"
	// tsp_search_state_t* initial = tsp_search_new(initial_node);
	// const int PRE_COMPUTE_DEPTH = 2;
	// tsp_search_state_t* expanded = tsp_search_expand(problem, initial, TSP_BREADTH_FIRST, PRE_COMPUTE_DEPTH);
	// // NOTE: the cluster is homogeneous, i.e. all nodes are equipped with the
	// //       same hardware. That's why we expand tasks of equal size and
	// //       round-robin them to each process.
	// // IDEA: calculate the "optimal pre-compute depth" using the size of the
	// //       problem n and using information about the cluster topology.
	// // THOUGHT: what if the cluster was heterogeneous?
	//
	// /** TODO: round-robin the list of tsp_search_node_t's to each process with a scatterv. */
	// int size_of_encoded_tsp_search_state = ?;
	// int size = size_of_encoded_tsp_search_state;
	// int* all_tsp_search_states = malloc(?);
	// for (search in expanded->list; rank in range(nproc))
	// 	all_tsp_search_states[offsets[rank] + index*size] = tsp_search_encode(search);
	// MPI_Scatterv(send: all_tsp_search_states, recv: my_tsp_search_states);
	//
	// tsp_solution_t* my_local_optimum = compute(problem, my_tsp_search_states);
	//
	// // TODO: encode a tsp_solution_t*
	// tsp_solution_t* all_local_optima;
	// MPI_Gather(recv: all_local_optima, send: my_local_optimum);

	tsp_message_del(message);
	tsp_del(problem);
}

void slave(int my_rank, char* my_node, int my_ncores)
{
	/** Receive the TSP data from the master process. */
	tsp_message_t message; int count = 1;
	MPI_Bcast(&message.count, count, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	debug("[%s][%d][bcast0::recv] message.count = %d\n", my_node, my_rank, message.count);

	message.buffer = (int*) malloc(message.count * sizeof(int));
	MPI_Bcast(message.buffer, message.count, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	tsp_t* problem = tsp_decode(&message);
	debug("[%s][%d][bcast1::recv] message.buffer = %s\n", my_node, my_rank, tsp_message_buffer_to_string(&message));

	// TODO: OMP part
	// tsp_solution_t* my_local_optimum = compute(problem, my_tsp_search_states);

	// TODO: encode a tsp_solution_t*
	// MPI_Gather(recv: NULL, send: my_local_optimum);
}

// tsp_solution_t* compute(tsp_t* problem, ? my_tsp_search_states)
// {
// 	// TODO: alloc one search to one thread and go depth first.
// 	return tsp_search_expand(problem, initial, TSP_DEPTH_FIRST, initial->problem->n);
// }
