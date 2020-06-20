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

int my_rank;
char my_node[MPI_MAX_PROCESSOR_NAME];
const int MASTER_RANK = 0;
tsp_t* input(int argc, char** argv);
void master(char* my_node, int my_ncores, tsp_t* problem);
void slave(int my_rank, char* my_node, int my_ncores);
// tsp_solution_t* compute(tsp_t* problem, ? my_tsp_search_states);

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	int my_node_strlen;
	MPI_Get_processor_name(my_node, &my_node_strlen);

	int my_ncores = omp_get_num_procs();
	debug("main", "ncores = %d", my_ncores);

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
	debug("master", "nproc = %d", nproc);

	/**
	 * Broadcast the TSP data to each slave process.
	 */
	tsp_message_t* message = tsp_encode(problem); int count = 1;
	MPI_Bcast(&message->count, count, MPI_INT, my_rank, MPI_COMM_WORLD);
	debug("master::bcast", "message->count = %d", message->count);

	MPI_Bcast(message->buffer, message->count, MPI_INT, my_rank, MPI_COMM_WORLD);
	char strbuf[TSP_MAX_STRING_LENGTH];
	tsp_message_buffer_to_string(strbuf, message);
	debug("master::bcast", "message->buffer = %s", strbuf);
	tsp_message_del(message), message = NULL;

	/**
	 * Pre-compute smaller tasks that can be distributed to each node.
	 * Let's expand search tree nodes up to depth = 1. This will result in
	 * (N-1)*(N-2) nodes held in memory inside the search's list data structure.
	 * NOTE: the cluster is homogeneous, i.e. all nodes are equipped with the
	 *       same hardware. That's why we expand tasks of equal size and
	 *       round-robin them to each process.
	 * IDEA: calculate the "optimal pre-compute depth" using the size of the
	 *       problem N and using information about the cluster topology.
	 * THOUGHT: what if the cluster was heterogeneous?
	*/
	int initial_node = 0;	// "initial_city"
	tsp_search_t* search = tsp_search_new(problem, initial_node);

	// Expand the root node, then expand its N-1 children.
	int niterations = 1 + (problem->n - 1);
	for (int i = 0; i < niterations; i++)
	{
		tsp_search_iterate(search, TSP_SEARCH_BREADTH_FIRST);
		debug("master::tsp_search_iterate", "search->list->length = %d", search->list->length);
	}

	/** Round-robin the search node's to the each process. */
	list_t** all_lists = (list_t**) malloc(nproc * sizeof(list_t*));
	for (int rank = 0; rank < nproc; rank++)
		all_lists[rank] = list_new(NULL);
	int rank = nproc - 1;
	while (search->list->length)
	{
		queue_push(all_lists[rank], queue_pop(search->list));
		debug("master::round_robin", "all_lists[rank = %d]->length = %d", rank, all_lists[rank]->length);
		if (--rank == -1)
			rank = nproc - 1;
	}
	for (int rank = 0; rank < nproc; rank++)
		debug("master", "all_lists[rank = %d]->length = %d", rank, all_lists[rank]->length);

	// int size_of_encoded_tsp_search_node = ?;
	// int size = size_of_encoded_tsp_search_state;
	// int* all_tsp_search_states = malloc(?);
	// for (search in expanded->list; rank in range(nproc))
	// 	all_tsp_search_states[offsets[rank] + index*size] = tsp_search_encode(search);
	// MPI_Scatterv(send: all_tsp_search_states, recv: my_tsp_search_states);

	// tsp_solution_t* my_local_optimum = compute(problem, my_tsp_search_states);
	//
	// // TODO: encode a tsp_solution_t*
	// tsp_solution_t* all_local_optima;
	// MPI_Gather(recv: all_local_optima, send: my_local_optimum);

	// global_optimum = merge(all_local_optima)
	// print(global_optimum)

	// TODO: tsp_message_del(message);
	tsp_search_del(search);
	tsp_del(problem);
}

void slave(int my_rank, char* my_node, int my_ncores)
{
	/** Receive the TSP data from the master process. */
	tsp_message_t* message = tsp_message_new(); int count = 1;
	MPI_Bcast(&message->count, count, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	debug("slave::bcast", "message->count = %d", message->count);

	message->buffer = (int*) malloc(message->count * sizeof(int));
	MPI_Bcast(message->buffer, message->count, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	tsp_t* problem = tsp_decode(message);
	char strbuf[TSP_MAX_STRING_LENGTH];
	tsp_message_buffer_to_string(strbuf, message);
	debug("slave::bcast", "message->buffer = %s", strbuf);
	tsp_message_del(message), message = NULL;

	// my_tsp_search_states = MPI_Scatterv()

	// tsp_solution_t* my_local_optimum = compute(problem, my_tsp_search_states);

	// TODO: encode a tsp_solution_t*
	// MPI_Gather(recv: NULL, send: my_local_optimum);

	tsp_del(problem);
}

// tsp_solution_t* compute(tsp_t* problem, ? my_tsp_search_states)
// {
// **OMP**
// 	// TODO: alloc one search to one thread and go depth first.
// 	return tsp_search_iterate(problem, initial, TSP_DEPTH_FIRST, initial->problem->n);
// }
