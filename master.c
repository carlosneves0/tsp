#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "tsp.h"
#include "message.h"
#include "list.h"
#include "compute.h"
#include "debug.h"

tsp_t* input(int argc, char** argv);
messages_t* encode_all_lists_recvcounts(tsp_t* problem, int nproc, list_t** all_lists);
messagesv_t* encode_all_lists(tsp_t* problem, int nproc, list_t** all_lists);
messages_t* alloc_recvmsgs(tsp_t* problem, int nproc);
tsp_solution_t** decode_all_local_optima(tsp_t* problem, int nproc, messages_t* msgs);
void output(tsp_solution_t* solution);

void master(int argc, char** argv, int my_rank, char* my_node, int my_ncores)
{
	tsp_t* problem = input(argc, argv);

	if (problem->n < 3)
	{
		fprintf(stderr, "Invalid problem size N = %d < 3\n", problem->n);
		// `MPI_Finalize();` wasn't enough here.
		MPI_Abort(MPI_COMM_WORLD, 1);
		exit(1);
	}

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
	tsp_search_t* global_search = tsp_search_new(problem, initial_node);
	int niterations = 1 + (problem->n - 1);
	for (int i = 0; i < niterations; i++)
	{
		tsp_search_iterate(global_search, TSP_SEARCH_BREADTH_FIRST);
		// VERBOSE: debug("master::pre_compute", "global_search->list->length = %d", global_search->list->length);
	}

	/** Round-robin the global_search->list's nodes to the each process. */
	list_t** all_lists = (list_t**) malloc(nproc * sizeof(list_t*));
	for (int rank = 0; rank < nproc; rank++)
		all_lists[rank] = list_new(NULL);
	int rank = nproc - 1;
	while (global_search->list->length)
	{
		list_enqueue(all_lists[rank], list_dequeue(global_search->list));
		// VERBOSE: debug("master::round_robin", "all_lists[rank = %d]->length = %d", rank, all_lists[rank]->length);
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
	messages_t* sendmsgs = encode_all_lists_recvcounts(problem, nproc, all_lists);
	message_t* recvmsg = message_new(MPI_INT);
	int recvcount = 1; MPI_Datatype recvtype = MPI_INT;
	MPI_Scatter(
		sendmsgs->buffer, sendmsgs->scatter_count, sendmsgs->type,
		&recvmsg->count, recvcount, recvtype,
		my_rank, MPI_COMM_WORLD);

	message_buffer_to_string((message_t*) sendmsgs, strbuf);
	debug("master::scatter", "sendmsgs->buffer = %s", strbuf);
	messages_del(sendmsgs), sendmsgs = NULL;
	debug("master::scatter", "recvmsg->count = %d", recvmsg->count);

	/** Scatterv each process' list to each process. */
	messagesv_t* sendmsgsv = encode_all_lists(problem, nproc, all_lists);
	for (int rank = 0; rank < nproc; rank++)
		list_del(all_lists[rank]);
	free(all_lists), all_lists = NULL;
	recvmsg->buffer = malloc(recvmsg->count * sizeof(int));
	MPI_Scatterv(
		sendmsgsv->buffer, sendmsgsv->counts, sendmsgsv->offsets, sendmsgsv->type,
		recvmsg->buffer, recvmsg->count, recvmsg->type,
		my_rank, MPI_COMM_WORLD);
	tsp_search_t* my_local_search = tsp_search_decode(problem, recvmsg);

	message_buffer_to_string((message_t*) sendmsgsv, strbuf);
	debug("master::scatterv", "sendmsgsv->buffer = %s", strbuf);
	messagesv_del(sendmsgsv), sendmsgsv = NULL;
	message_buffer_to_string(recvmsg, strbuf);
	debug("master::scatterv", "recvmsg->buffer = %s", strbuf);
	message_del(recvmsg), recvmsg = NULL;

	/**
	 * COMPUTE! Finally! hahahahaha
	 * Here I have my_local_search populated with some tsp_search_nodes.
	 * Let's expand these search tree nodes to find my_local_optimum.
	 */
	tsp_solution_t* my_local_optimum = compute(my_local_search, my_ncores);

	char my_local_optimum_string[TSP_SOLUTION_STRING_MAX];
	tsp_solution_to_string(my_local_optimum, my_local_optimum_string);
	debug("master", "my_local_optimum = %s", my_local_optimum_string);

	/**
	 * Gather all local_optima in the master process.
	 */
	sendmsg = tsp_solution_encode(problem, my_local_optimum);
	messages_t* recvmsgs = alloc_recvmsgs(problem, nproc);
	MPI_Gather(
		sendmsg->buffer, sendmsg->count, sendmsg->type,
		recvmsgs->buffer, recvmsgs->gather_count, recvmsgs->type,
		my_rank, MPI_COMM_WORLD);
	tsp_solution_t** all_local_optima = decode_all_local_optima(problem, nproc, recvmsgs);

	message_buffer_to_string(sendmsg, strbuf);
	debug("master::gather", "sendmsg->buffer = %s", strbuf);
	message_del(sendmsg), sendmsg = NULL;

	message_buffer_to_string((message_t*) recvmsgs, strbuf);
	debug("master::gather", "recvmsgs->buffer = %s", strbuf);
	messages_del(recvmsgs), recvmsgs = NULL;

	/**************
	 *    \||/    *
	 *    \||/    *
	 *  .<><><>.  *
	 * .<><><><>. *
	 * '<><><><>' *
	 *  '<><><>'  *
	 **************/
	tsp_solution_t* global_optimum = all_local_optima[0];
	for (int rank = 1; rank < nproc; rank++)
		if (all_local_optima[rank]->cost < global_optimum->cost)
			global_optimum = all_local_optima[rank];
	output(global_optimum);

	for (int rank = 0; rank < nproc; rank++)
		tsp_solution_del(all_local_optima[rank]);
	free(all_local_optima);
	tsp_search_del(my_local_search);
	tsp_search_del(global_search);
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

messages_t* encode_all_lists_recvcounts(tsp_t* problem, int nproc, list_t** all_lists)
{
	messages_t* messages = (messages_t*) malloc(sizeof(messages_t));

	// 1 tsp_search_node_t encoded as int* is:
	// 1 int for depth + N ints for visited + unvisited sets.
	int encoded_tsp_search_node_count = (1 + problem->n);
	int* buffer = (int*) malloc(nproc * sizeof(int));
	for (int rank = 0; rank < nproc; rank++)
		buffer[rank] = all_lists[rank]->length * encoded_tsp_search_node_count;
	messages->buffer = (void*) buffer;

	messages->count = nproc;

	messages->type = MPI_INT;

	// Send only 1 int to each process.
	messages->scatter_count = 1;

	return messages;
}

messagesv_t* encode_all_lists(tsp_t* problem, int nproc, list_t** all_lists)
{
	int encoded_tsp_search_node_count = (1 + problem->n);
	const int NODE_COUNT = encoded_tsp_search_node_count;

	int total_count = 0;
	int* counts = (int*) malloc(nproc * sizeof(int));
	int* offsets = (int*) malloc(nproc * sizeof(int));
	for (int rank = 0; rank < nproc; rank++)
	{
		list_t* my_list = all_lists[rank];

		counts[rank] = my_list->length * NODE_COUNT;
		total_count += counts[rank];

		if (rank == 0)
			offsets[rank] = 0;
		else
			offsets[rank] = offsets[rank - 1] + counts[rank - 1];
	}

	int* buffer = (int*) malloc(total_count * sizeof(int));

	for (int rank = 0; rank < nproc; rank++)
	{
		int index = 0;
		list_t* my_list = all_lists[rank];
		while (my_list->length)
		{
			tsp_search_node_t* search_node = list_dequeue(my_list);
			message_t* msg = tsp_search_node_encode(problem, search_node);
			memcpy(
				buffer + offsets[rank] + index*NODE_COUNT,
				msg->buffer,
				msg->count * sizeof(int));
			message_del(msg);
			tsp_search_node_del(search_node);
			index++;
		}
	}

	messagesv_t* msgv = (messagesv_t*) malloc(sizeof(messagesv_t));
	msgv->buffer = (void*) buffer;
	msgv->count = total_count;
	msgv->type = MPI_INT;
	msgv->counts = counts;
	msgv->offsets = offsets;
	return msgv;
}

messages_t* alloc_recvmsgs(tsp_t* problem, int nproc)
{
	const int N = problem->n;
	const int TSP_SOLUTION_COUNT = 1 + N; // 1 int for cost + N ints for circuit

	int count = nproc * TSP_SOLUTION_COUNT;
	int* buffer = (int*) malloc(count * sizeof(int));
	MPI_Datatype type = MPI_INT;
	int gather_count = TSP_SOLUTION_COUNT;

	messages_t* recvmsgs = (messages_t*) malloc(sizeof(messages_t));
	recvmsgs->buffer = (void*) buffer;
	recvmsgs->count = count;
	recvmsgs->type = type;
	recvmsgs->gather_count = gather_count;
	return recvmsgs;
}

tsp_solution_t** decode_all_local_optima(tsp_t* problem, int nproc, messages_t* msgs)
{
	const int N = problem->n;
	const int TSP_SOLUTION_COUNT = 1 + N; // 1 int for cost + N ints for circuit

	tsp_solution_t** all_local_optima = (tsp_solution_t**) malloc(nproc * sizeof(tsp_solution_t*));
	for (int rank = 0; rank < nproc; rank++)
	{
		all_local_optima[rank] = (tsp_solution_t*) malloc(sizeof(tsp_solution_t));

		all_local_optima[rank]->problem = problem;

		int* buffer = ((int*) msgs->buffer) + rank*TSP_SOLUTION_COUNT;

		all_local_optima[rank]->cost = buffer[0];

		int offset = 1;
		all_local_optima[rank]->circuit = (int*) malloc(N * sizeof(int));
		memcpy(all_local_optima[rank]->circuit, buffer + offset, N * sizeof(int));
	}

	return all_local_optima;
}

void output(tsp_solution_t* solution)
{
	const int N = solution->problem->n;
	printf("%d\n", solution->cost);
	for (int i = 0; i < N; i++)
		printf("%d ", solution->circuit[i]);
	printf("%d\n", solution->circuit[0]);
}
