#include <stdlib.h>
#include "tsp.h"

tsp_t* tsp_new(FILE* stream)
{
	tsp_t* problem = (tsp_t*) malloc(sizeof(tsp_t));

	fscanf(stream, "%d\n", &problem->n);
	const int N = problem->n;

	problem->cost = (int**) malloc(N * sizeof(int*));
	for (int i = 0; i < N; i++)
	{
		problem->cost[i] = (int*) malloc(N * sizeof(int));
		for (int j = 0; j < N; j++)
			fscanf(stream, "%d", &problem->cost[i][j]);
	}

	return problem;
}

void tsp_del(tsp_t* problem)
{
	for (int i = 0; i < problem->n; i++)
		free(problem->cost[i]);
	free(problem->cost);
	free(problem);
}

// void tsp_fprint(FILE* stream, tsp_t* problem)
// {
// 	const int N = problem->n;
// 	for (int i = 0; i < N; i++)
// 	{
// 		for (int j = 0; j < N; j++)
// 			fprintf(stream, "%d%s", problem->cost[i][j], (j < N-1 ? " " : ""));
// 		fprintf(stream, "\n");
// 	}
// 	fflush(stream);
// }

tsp_message_t* tsp_encode(tsp_t* problem)
{
	// 1 int for n
	// n*n int's for cost
	tsp_message_t* message = (tsp_message_t*) malloc(sizeof(tsp_message_t));

	const int N = problem->n;

	message->count = 1 + N*N;

	message->buffer = (int*) malloc(message->count * sizeof(int));

	message->buffer[0] = N;

	int offset = 1;
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			message->buffer[offset + (i*N + j)] = problem->cost[i][j];

	return message;
}

tsp_t* tsp_decode(tsp_message_t* message)
{
	tsp_t* problem = (tsp_t*) malloc(sizeof(tsp_t));

	problem->n = message->buffer[0];
	const int N = problem->n;

	int offset = 1;
	problem->cost = (int**) malloc(N * sizeof(int*));
	for (int i = 0; i < N; i++)
	{
		problem->cost[i] = (int*) malloc(N * sizeof(int));
		for (int j = 0; j < N; j++)
			problem->cost[i][j] = message->buffer[offset + (i*N + j)];
	}

	return problem;
}

void tsp_message_del(tsp_message_t* message)
{
	free(message->buffer);
	free(message);
}

char* tsp_message_buffer_to_string(tsp_message_t* message)
{
	const int count = message->count;
	const int* buffer = message->buffer;
	static const int MAX_INT_LENGTH = 8;
	char* string = malloc(2 + count*MAX_INT_LENGTH + 1); // "[1,2,3,4]\0"

	char* s = string;
	s += sprintf(s, "[");
	for (int i = 0; i < count; i++)
		s += sprintf(s, "%d%s", buffer[i], (i < count-1 ? "," : ""));
	s += sprintf(s, "]");
	s[0] = '\0';

	// Try to free(string)...
	// I know there will be memory leaks... but I just wanna be able to do:
	// printf("foo %s bar", to_string(message));
	// Is there a way to do this without memory leaks?
	return string;
}
