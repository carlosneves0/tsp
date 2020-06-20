#include <stdlib.h>
#include <mpi.h>
#include "tsp.h"
#include "message.h"
#include "list.h"
#include "debug.h"

tsp_t* input(int argc, char** argv);
message_t* encode_all_lists_recvcounts(tsp_t* problem, int nproc, list_t** all_lists);

void master(int argc, char** argv, int my_rank, char* my_node, int my_ncores)
{
	tsp_t* problem = input(argc, argv);

	int nproc;
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	debug("master", "nproc = %d", nproc);

	/**
	 * Broadcast the TSP data to each slave process.
	 */
	message_t* sendmsg = tsp_encode(problem);
	int sendcount = 1; MPI_Datatype sendtype = MPI_INT;
	MPI_Bcast(
		(void*) &sendmsg->count, sendcount, sendtype,
		my_rank, MPI_COMM_WORLD);
	debug("master::bcast", "sendmsg->count = %d", sendmsg->count);

	MPI_Bcast(
		sendmsg->buffer, sendmsg->count, sendmsg->type,
		my_rank, MPI_COMM_WORLD);
	char strbuf[MESSAGE_BUFFER_STRING_MAX];
	message_buffer_to_string(sendmsg, strbuf);
	debug("master::bcast", "sendmsg->buffer = %s", strbuf);
	message_del(sendmsg), sendmsg = NULL;

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
	 * THOUGHT: what if the cluster's topology changed as a function of time?
	*/
	/**
	 * Expand the root node, then expand its N-1 children. Expand breadth first,
	 * so that the resulting search nodes in the list are of equal size.
	 */
	int initial_node = 0;	// "initial_city"
	tsp_search_t* search = tsp_search_new(problem, initial_node);
	int niterations = 1 + (problem->n - 1);
	for (int i = 0; i < niterations; i++)
	{
		tsp_search_iterate(search, TSP_SEARCH_BREADTH_FIRST);
		/* VERBOSE: / debug("master::pre_compute", "search->list->length = %d", search->list->length); /**/
	}

	/** Round-robin the search nodes to the each process. */
	list_t** all_lists = (list_t**) malloc(nproc * sizeof(list_t*));
	for (int rank = 0; rank < nproc; rank++)
		all_lists[rank] = list_new(NULL);
	int rank = nproc - 1;
	while (search->list->length)
	{
		list_enqueue(all_lists[rank], list_dequeue(search->list));
		/* VERBOSE: / debug("master::round_robin", "all_lists[rank = %d]->length = %d", rank, all_lists[rank]->length); /**/
		if (--rank == -1)
			rank = nproc - 1;
	}
	for (int rank = 0; rank < nproc; rank++)
		debug("master", "all_lists[rank = %d]->length = %d", rank, all_lists[rank]->length);

	/**
	 * Scatter each process' list recvcount to each process. This "recvcount"
	 * will be used by each process in an MPI_Scatterv to receive its
	 * individual list from the master.
	 */
	sendmsg = encode_all_lists_recvcounts(problem, nproc, all_lists);
	int my_list_recvcount, recvcount = 1; MPI_Datatype recvtype = MPI_INT;
	MPI_Scatter(
		sendmsg->buffer, sendmsg->count, sendmsg->type,
		&my_list_recvcount, recvcount, recvtype,
		my_rank, MPI_COMM_WORLD);
	message_del(sendmsg), sendmsg = NULL;
	debug("master::scatter", "my_list_recvcount = %d", my_list_recvcount);

	/** Scatterv each process' list to each process. */
	// messagesv_t* sendsv = tsp_encode_all_lists(all_lists);
	// tsp_search_node_t* my_encoded_list; recvcount = my_list_recvcount;
	// MPI_Scatterv(
	// 	sendsv->buffer, sendsv->counts, sendsv->offsets, sendsv->type,
	// 	(void*) my_list, recvcount, MPI_INT, my_rank,
	// 	MPI_COMM_WORLD);

	// int size_of_encoded_tsp_search_node = ?;
	// int size = size_of_encoded_tsp_search_state;
	// int* all_tsp_search_states = malloc(?);
	// for (search in expanded->list; rank in range(nproc))
	// 	all_tsp_search_states[offsets[rank] + index*size] = tsp_search_encode(search);

	// tsp_solution_t* my_local_optimum = compute(problem, my_tsp_search_states);
	//
	// // TODO: encode a tsp_solution_t*
	// tsp_solution_t* all_local_optima;
	// MPI_Gather(recv: all_local_optima, send: my_local_optimum);

	// global_optimum = merge(all_local_optima)
	// print(global_optimum)

	tsp_search_del(search);
	tsp_del(problem);
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

message_t* encode_all_lists_recvcounts(tsp_t* problem, int nproc, list_t** all_lists)
{
	message_t* message = (message_t*) malloc(sizeof(message_t));

	// 1 tsp_search_node_t encoded as int* is:
	// 1 int for depth + N ints for visited + unvisited sets.
	int sizeof_encoded_tsp_search_node = (1 + problem->n);
	int* buffer = (int*) malloc(nproc * sizeof(int));
	for (int rank = 0; rank < nproc; rank++)
		buffer[rank] = all_lists[rank]->length * sizeof_encoded_tsp_search_node;
	message->buffer = (void*) buffer;

	// Send only 1 int to each process.
	message->count = 1;

	message->type = MPI_INT;

	return message;
}
