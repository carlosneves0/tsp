#include <stdlib.h>
#include "tsp.h"

/*******************************************************************************
 * tsp_t
 * The data that describe an instance of the TSP.
 ******************************************************************************/

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

/*******************************************************************************
 * tsp_search_t
 * This struct describes the state a
 ******************************************************************************/

tsp_search_t* tsp_search_new(tsp_t* problem, int initial_node)
{
	tsp_search_t* search = (tsp_search_t*) malloc(sizeof(tsp_search_t));

	search->problem = problem;

	search->optimum = NULL;

	tsp_search_node_t* root = (tsp_search_node_t*) malloc(sizeof(tsp_search_node_t));
	root->depth = 0;
	root->visited = (int*) malloc(sizeof(int));
	root->visited[0] = initial_node;
	const int N = problem->n;
	root->unvisited = (int*) malloc((N-1) * sizeof(int));
	for (int node = 0, index = 0; node < N; node++)
		if (node != initial_node)
			root->unvisited[index++] = node;
	search->list = list_new((void*) root);

	return search;
}

void tsp_search_del(tsp_search_t* search)
{
	list_del(search->list);
	free(search);
}

char* tsp_search_to_string(tsp_search_t* search)
{
	// TODO: max_string_length = 128 characters
	// TODO: sprintf(string, "{optimum = %s,", (search->optimum ? tsp_solution_to_string(search->optimum) : "NULL"));
	// TODO: sprintf(string, "list = %s}", list_to_string(search->list));
	// return string;
	static const int STRING_MAX_LENGTH = 128;
	char* string = (char*) malloc(STRING_MAX_LENGTH + 1);
	
	return NULL;
}

void tsp_search_expand(tsp_search_t* search, tsp_search_strategy_t strategy, int depth)
{
	if depth == -1
		while (!list.empty())

	while (depth--)
	{
		x = list
	}
}

/*******************************************************************************
 * tsp_message_t
 * A generic data type that describes a message as a sequential block of int's.
 ******************************************************************************/

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
