#include <stdlib.h>
#include <mpi.h>
#include "slave.h"
#include "message.h"
#include "tsp.h"
#include "debug.h"

/** Declared and initialized in main.c. */
extern const int MASTER_RANK;

void slave(int my_rank, char* my_node, int my_ncores)
{
	/** Receive the TSP data from the master process. */
	message_t* recvmsg = message_new(NULL, 0, MPI_INT);
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

	int my_list_recvcount; recvcount = 1; MPI_Datatype recvtype = MPI_INT;
	MPI_Scatter(
		NULL, 0, MPI_INT,
		&my_list_recvcount, recvcount, recvtype,
		MASTER_RANK, MPI_COMM_WORLD);
	debug("slave::scatter", "my_list_recvcount = %d", my_list_recvcount);

	// my_tsp_search_states = MPI_Scatterv()

	// tsp_solution_t* my_local_optimum = compute(problem, my_tsp_search_states);

	// TODO: encode a tsp_solution_t*
	// MPI_Gather(recv: NULL, send: my_local_optimum);

	tsp_del(problem);
}
