#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "message.h"

message_t* message_new(void* buffer, int count, MPI_Datatype type)
{
	message_t* message = (message_t*) malloc(sizeof(message_t));
	message->buffer = buffer;
	message->count = count;
	message->type = type;
	return message;
}

void message_del(message_t* message)
{
	if (message->buffer)
		free(message->buffer);
	free(message);
}

/** Good size for a full-width terminal. */
const int MESSAGE_BUFFER_STRING_MAX = 96;

void message_buffer_to_string(message_t* message, char* string)
{
	if (message->type != MPI_INT)
	{
		char error[] = "[message buffer has MPI_Datatype != MPI_INT]";
		// sizeof(error) == 45
		memcpy(string, error, sizeof(error));
		return;
	}

	const int count = message->count;
	const int* buffer = message->buffer;

	char tmpbuf[MESSAGE_BUFFER_STRING_MAX];
	int index = 0;
	index += sprintf(string, "[");
	for (int i = 0; i < count; i++)
	{
		int length = sprintf(tmpbuf, "%d%s", buffer[i], (i < count-1 ? "," : ""));
		if (index + length < MESSAGE_BUFFER_STRING_MAX)
		{
			memcpy(string + index, tmpbuf, length);
			index += length;
		}
	}
	if (index < MESSAGE_BUFFER_STRING_MAX - 1)
	{
		index += sprintf(string + index, "]");
		string[index] = '\0';
	}
	else
	{
		// Content overflowed MESSAGE_BUFFER_STRING_MAX chars limitation.
		string[MESSAGE_BUFFER_STRING_MAX - 5] = '.';
		string[MESSAGE_BUFFER_STRING_MAX - 4] = '.';
		string[MESSAGE_BUFFER_STRING_MAX - 3] = '.';
		string[MESSAGE_BUFFER_STRING_MAX - 2] = ']';
		string[MESSAGE_BUFFER_STRING_MAX - 1] = '\0';
	}
}
