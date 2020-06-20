#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include <mpi.h>

/**
 * message_t
 * A generic data type that describes an MPI message.
 */
struct message
{
	void* buffer;
	int count;
	MPI_Datatype type;
};
typedef struct message message_t;
message_t* message_new(MPI_Datatype type);
void message_del(message_t* message);
extern const int MESSAGE_BUFFER_STRING_MAX;
void message_buffer_to_string(message_t* message, char string[MESSAGE_BUFFER_STRING_MAX]);

/**
 * messages_t
 * A generic data type that describes an MPI message suitable to be used
 * in MPI_Scatter and MPI_Gather calls.
 */
struct messages
{
	void* buffer;
	int count;
	MPI_Datatype type;
	/* ^^^ backwards compatible with message_t */
	int scatter_count;
};
typedef struct messages messages_t;
void messages_del(messages_t* messages);

/**
 * messagesv_t
 * A generic data type that describes an MPI message suitable to be used
 * in MPI_Scatterv and MPI_Scatterv calls.
 */
struct messagesv
{
	void* buffer;
	int count;
	MPI_Datatype type;
	/* ^^^ backwards compatible with message_t */
	int* counts;
	int* offsets;
};
typedef struct messagesv messagesv_t;
void messagesv_del(messagesv_t* messagesv);

#endif
