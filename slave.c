#include <stdlib.h>
#include <mpi.h>
#include "slave.h"
#include "message.h"
#include "tsp.h"
#include "compute.h"
#include "debug.h"

/** Declared and initialized in main.c. */
extern const int MASTER_RANK;

void slave(int my_rank, char* my_node, int my_ncores)
{
	/** Receive the TSP data from the master process. */
	message_t* recvmsg = message_new(MPI_INT);
	int recvcount = 1;
	MPI_Bcast(
		&recvmsg->count, recvcount, MPI_INT,
		MASTER_RANK, MPI_COMM_WORLD);
	debug("slave::bcast", "recvmsg->count = %d", recvmsg->count);

	recvmsg->buffer = malloc(recvmsg->count * sizeof(int));
	MPI_Bcast(
		recvmsg->buffer, recvmsg->count, recvmsg->type,
		MASTER_RANK, MPI_COMM_WORLD);
	tsp_t* problem = tsp_decode(recvmsg);

	char strbuf[MESSAGE_BUFFER_STRING_MAX];
	message_buffer_to_string(recvmsg, strbuf);
	debug("slave::bcast", "recvmsg->buffer = %s", strbuf);
	message_del(recvmsg), recvmsg = NULL;

	/** Receive the recvcount for my_list_encoded from the master process. */
	recvmsg = message_new(MPI_INT);
	recvcount = 1; MPI_Datatype recvtype = MPI_INT;
	MPI_Scatter(
		NULL, 0, MPI_INT,
		&recvmsg->count, recvcount, recvtype,
		MASTER_RANK, MPI_COMM_WORLD);
	debug("slave::scatter", "recvmsg->count = %d", recvmsg->count);

	/** Receive my_list_encoded from the master process. */
	recvmsg->buffer = malloc(recvmsg->count * sizeof(int));
	MPI_Scatterv(
		NULL, NULL, NULL, MPI_INT,
		recvmsg->buffer, recvmsg->count, recvmsg->type,
		MASTER_RANK, MPI_COMM_WORLD);
	tsp_search_t* my_local_search = tsp_search_decode(problem, recvmsg);

	message_buffer_to_string(recvmsg, strbuf);
	debug("slave::scatterv", "recvmsg->buffer = %s", strbuf);
	message_del(recvmsg), recvmsg = NULL;

	/**
	 * COMPUTE! Finally! hahahahaha
	 * Here I have my_local_search populated with some tsp_search_nodes.
	 * Let's expand these search tree nodes to find my_local_optimum.
	 */
	tsp_solution_t* my_local_optimum = compute(my_local_search);

	char my_local_optimum_string[TSP_SOLUTION_STRING_MAX];
	tsp_solution_to_string(my_local_optimum, my_local_optimum_string);
	debug("slave", "my_local_optimum = %s", my_local_optimum_string);

	/**
	 * Gather all local_optima in the master process.
	 */
	message_t* sendmsg = tsp_solution_encode(problem, my_local_optimum);
	MPI_Gather(
		sendmsg->buffer, sendmsg->count, sendmsg->type,
		NULL, 0, MPI_INT,
		MASTER_RANK, MPI_COMM_WORLD);

	message_buffer_to_string(sendmsg, strbuf);
	debug("slave::gather", "sendmsg->buffer = %s", strbuf);
	message_del(sendmsg), sendmsg = NULL;

	tsp_search_del(my_local_search);
	tsp_del(problem);
}
