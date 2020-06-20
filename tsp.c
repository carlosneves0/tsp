#include <stdlib.h>
#include <string.h>
#include <mpi.h>
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

message_t* tsp_encode(tsp_t* problem)
{
	// 1 int for tsp_t::n
	// N*N ints for tsp_t::cost
	message_t* message = (message_t*) malloc(sizeof(message_t));

	const int N = problem->n;

	message->count = 1 + N*N;

	int* buffer = (int*) malloc(message->count * sizeof(int));
	buffer[0] = N;
	int offset = 1;
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			buffer[offset + (i*N + j)] = problem->cost[i][j];
	message->buffer = (void*) buffer;

	message->type = MPI_INT;

	return message;
}

tsp_t* tsp_decode(message_t* message)
{
	tsp_t* problem = (tsp_t*) malloc(sizeof(tsp_t));

	int* buffer = (int*) message->buffer;
	const int N = problem->n = buffer[0];

	int offset = 1;
	problem->cost = (int**) malloc(N * sizeof(int*));
	for (int i = 0; i < N; i++)
	{
		problem->cost[i] = (int*) malloc(N * sizeof(int));
		for (int j = 0; j < N; j++)
			problem->cost[i][j] = buffer[offset + (i*N + j)];
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

void tsp_search_iterate(tsp_search_t* search, tsp_search_strategy_t strategy)
{
	// %DEBUG%
	// printf("strategy = %d\n", strategy);
	// printf("TSP_SEARCH_DEPTH_FIRST = %d\n", TSP_SEARCH_DEPTH_FIRST);
	// printf("TSP_SEARCH_BREADTH_FIRST = %d\n", TSP_SEARCH_BREADTH_FIRST);
	// %DEBUG%

	const int N = search->problem->n;

	tsp_search_node_t* x;
	if (strategy == TSP_SEARCH_DEPTH_FIRST)
		x = (tsp_search_node_t*) list_unstack(search->list);
	else
		x = (tsp_search_node_t*) list_dequeue(search->list);

	// TODO: Check if x is a search tree leaf, i.e. a solution.
	// "search->optimum = x";

	// %DEBUG%
	// int x_visited_count = x->depth + 1;
	// printf("x->visited = ");
	// for (int j = 0; j < x_visited_count; j++)
	// 	printf("%d ", x->visited[j]);
	// printf("\n"); fflush(stdout);
	// int x_unvisited_count = N - x_visited_count;
	// printf("x->unvisited = ");
	// for (int j = 0; j < x_unvisited_count; j++)
	// 	printf("%d ", x->unvisited[j]);
	// printf("\n"); fflush(stdout);
	// %DEBUG%

	// Expand x's children and add them to the search list.
	int x_unvisited_count = N - (x->depth + 1);
	// %DEBUG%
	// printf("x_unvisited_count = %d\n", x_unvisited_count);
	// %DEBUG%
	for (int i = 0; i < x_unvisited_count; i++)
	{
		// %DEBUG%
		// printf("\ni = %d\tx->unvisited[%d] = %d\tN = %d\n", i, i, x->unvisited[i], N);
		// %DEBUG%

		tsp_search_node_t* y = tsp_search_node_new(x, x->unvisited[i], N);

		// %DEBUG%
		// int y_visited_count = y->depth + 1;
		// printf("y_visited_count = %d\n", y_visited_count);
		// printf("y->visited = "); fflush(stdout);
		// for (int j = 0; j < y_visited_count; j++)
		// 	printf("%d ", y->visited[j]); fflush(stdout);
		// printf("\n"); fflush(stdout);
		// int y_unvisited_count = N - y_visited_count;
		// printf("y_unvisited_count = %d\n", y_unvisited_count);
		// printf("y->unvisited = "); fflush(stdout);
		// for (int j = 0; j < y_unvisited_count; j++)
		// 	printf("%d ", y->unvisited[j]); fflush(stdout);
		// printf("\n"); fflush(stdout);
		// %DEBUG%

		if (strategy == TSP_SEARCH_DEPTH_FIRST)
			list_stack(search->list, (void*) y);
		else
			list_enqueue(search->list, (void*) y);
	}

	tsp_search_node_del(x);
}

/*******************************************************************************
 * tsp_search_node_t
 * The data that describe a node in the TSP's search tree.
 ******************************************************************************/
tsp_search_node_t* tsp_search_node_new(tsp_search_node_t* parent, int new_visited_node, const int N)
{
	tsp_search_node_t* search_node = (tsp_search_node_t*) malloc(sizeof(tsp_search_node_t));

	search_node->depth = parent->depth + 1;

	int parent_visited_count = parent->depth + 1;
	search_node->visited = (int*) malloc((parent_visited_count + 1) * sizeof(int));
	memcpy(search_node->visited, parent->visited, parent_visited_count * sizeof(int));
	search_node->visited[parent_visited_count] = new_visited_node;

	int parent_unvisited_count = N - (parent->depth + 1);
	search_node->unvisited = (int*) malloc((parent_unvisited_count - 1) * sizeof(int));
	for (int j = 0, k = 0; j < parent_unvisited_count; j++)
		if (parent->unvisited[j] != new_visited_node)
			search_node->unvisited[k++] = parent->unvisited[j];

	// %DEBUG%
	// int search_node_visited_count = search_node->depth + 1, search_node_unvisited_count = N - search_node_visited_count;
	// printf("search_node->depth = %d\n", search_node->depth);
	// printf("parent_visited_count = %d\n", parent_visited_count);
	// printf("search_node->visited = ");
	// for (int j = 0; j < search_node_visited_count; j++)
	// 	printf("%d ", search_node->visited[j]);
	// printf("\n"); fflush(stdout);
	// printf("parent_unvisited_count = %d\n", parent_unvisited_count);
	// printf("search_node->unvisited = ");
	// for (int j = 0; j < search_node_unvisited_count; j++)
	// 	printf("%d ", search_node->unvisited[j]);
	// printf("\n"); fflush(stdout);
	// %DEBUG%

	return search_node;
}

void tsp_search_node_del(tsp_search_node_t* node)
{
	free(node->visited);
	free(node->unvisited);
	free(node);
}

message_t* tsp_search_node_encode(tsp_t* problem, tsp_search_node_t* node)
{
	const int N = problem->n;

	// 1 int for node->depth
	// N ints for node->visited + node->unvisited sets
	int count = 1 + N;
	int* buffer = (int*) malloc(count * sizeof(int));

	buffer[0] = node->depth;

	int offset = 1;
	int visited_count = node->depth + 1;
	for (int i = 0; i < visited_count; i++)
		buffer[offset + i] = node->visited[i];

	offset = 1 + visited_count;
	int unvisited_count = N - visited_count;
	for (int i = 0; i < unvisited_count; i++)
		buffer[offset + i] = node->unvisited[i];

	message_t* msg = (message_t*) malloc(sizeof(message_t));
	msg->buffer = (void*) buffer;
	msg->count = count;
	msg->type = MPI_INT;
	return msg;
}

tsp_search_node_t* tsp_search_node_decode(message_t* message)
{}
