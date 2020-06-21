#include <stdlib.h>
#include <string.h>
#include <limits.h>
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

	// if parent_notvisited_count == 0, we're still good, we can malloc(0).
	// https://stackoverflow.com/questions/2022335/whats-the-point-of-malloc0
	int parent_notvisited_count = N - (parent->depth + 1);
	search_node->notvisited = (int*) malloc((parent_notvisited_count - 1) * sizeof(int));
	for (int j = 0, k = 0; j < parent_notvisited_count; j++)
		if (parent->notvisited[j] != new_visited_node)
			search_node->notvisited[k++] = parent->notvisited[j];

	// %DEBUG%
	// int search_node_visited_count = search_node->depth + 1, search_node_notvisited_count = N - search_node_visited_count;
	// printf("search_node->depth = %d\n", search_node->depth);
	// printf("parent_visited_count = %d\n", parent_visited_count);
	// printf("search_node->visited = ");
	// for (int j = 0; j < search_node_visited_count; j++)
	// 	printf("%d ", search_node->visited[j]);
	// printf("\n"); fflush(stdout);
	// printf("parent_notvisited_count = %d\n", parent_notvisited_count);
	// printf("search_node->notvisited = ");
	// for (int j = 0; j < search_node_notvisited_count; j++)
	// 	printf("%d ", search_node->notvisited[j]);
	// printf("\n"); fflush(stdout);
	// %DEBUG%

	return search_node;
}

void tsp_search_node_del(tsp_search_node_t* node)
{
	free(node->visited);
	free(node->notvisited);
	free(node);
}

message_t* tsp_search_node_encode(tsp_t* problem, tsp_search_node_t* node)
{
	const int N = problem->n;

	// 1 int for node->depth
	// N ints for node->visited + node->notvisited sets
	int count = 1 + N;
	int* buffer = (int*) malloc(count * sizeof(int));

	buffer[0] = node->depth;

	int offset = 1;
	int visited_count = node->depth + 1;
	for (int i = 0; i < visited_count; i++)
		buffer[offset + i] = node->visited[i];

	offset = 1 + visited_count;
	int notvisited_count = N - visited_count;
	for (int i = 0; i < notvisited_count; i++)
		buffer[offset + i] = node->notvisited[i];

	message_t* msg = (message_t*) malloc(sizeof(message_t));
	msg->buffer = (void*) buffer;
	msg->count = count;
	msg->type = MPI_INT;
	return msg;
}

tsp_search_node_t* tsp_search_node_decode(tsp_t* problem, message_t* msg)
{
	int* buffer = (int*) msg->buffer;

	int depth = buffer[0];

	int offset = 1;
	int visited_count = depth + 1;
	int* visited = (int*) malloc(visited_count * sizeof(int));
	for (int i = 0; i < visited_count; i++)
		visited[i] = buffer[offset + i];

	// if notvisited_count == 0, we're still good, we can malloc(0).
	// https://stackoverflow.com/questions/2022335/whats-the-point-of-malloc0
	offset = 1 + visited_count;
	int notvisited_count = problem->n - visited_count;
	int* notvisited = (int*) malloc(notvisited_count * sizeof(int));
	for (int i = 0; i < notvisited_count; i++)
		notvisited[i] = buffer[offset + i];

	tsp_search_node_t* search_node = (tsp_search_node_t*) malloc(sizeof(tsp_search_node_t));
	search_node->depth = depth;
	search_node->visited = visited;
	search_node->notvisited = notvisited;
	return search_node;
}

/*******************************************************************************
 * tsp_solution_t
 * A tsp_solution_t is a circuit in the TSP's graph.
 ******************************************************************************/
tsp_solution_t* tsp_solution_new(tsp_t* problem, tsp_search_node_t* search_node)
{
	const int N = search_node->depth + 1;

	int* circuit = (int*) malloc(N * sizeof(int));
	memcpy(circuit, search_node->visited, N * sizeof(int));

	int cost = 0;
	int** c = problem->cost;
	for (int i = 1; i < N; i++)
	{
		int u = circuit[i-1], v = circuit[i];
		cost += c[u][v];
	}
	cost += c[circuit[N-1]][circuit[0]];

	tsp_solution_t* solution = (tsp_solution_t*) malloc(sizeof(tsp_solution_t));
	solution->problem = problem;
	solution->cost = cost;
	solution->circuit = circuit;
	return solution;
}

void tsp_solution_del(tsp_solution_t* solution)
{
	free(solution->circuit);
	free(solution);
}

const int TSP_SOLUTION_STRING_MAX = 128;

void tsp_solution_to_string(tsp_solution_t* solution, char* string)
{
	const int TSP_SOLUTION_CIRCUIT_STRING_MAX = 53;
	const int N = solution->problem->n;

	char circuit[TSP_SOLUTION_CIRCUIT_STRING_MAX];
	char tmpbuf[32];
	int offset = 0, length;
	offset += sprintf(circuit, "[");
	for (int i = 0; i < N + 1; i++)
	{
		if (i < N)
			length = sprintf(tmpbuf, "%d ", solution->circuit[i]);
		else
			length = sprintf(tmpbuf, "%d", solution->circuit[0]);

		if (offset + length < TSP_SOLUTION_CIRCUIT_STRING_MAX)
		{
			memcpy(circuit + offset, tmpbuf, length);
			offset += length;
		}
	}
	if (offset < TSP_SOLUTION_CIRCUIT_STRING_MAX - 1)
	{
		offset += sprintf(circuit + offset, "]");
		circuit[offset] = '\0';
	}
	else
	{
		// Content overflowed TSP_SOLUTION_CIRCUIT_STRING_MAX chars limitation.
		circuit[TSP_SOLUTION_CIRCUIT_STRING_MAX - 5] = '.';
		circuit[TSP_SOLUTION_CIRCUIT_STRING_MAX - 4] = '.';
		circuit[TSP_SOLUTION_CIRCUIT_STRING_MAX - 3] = '.';
		circuit[TSP_SOLUTION_CIRCUIT_STRING_MAX - 2] = ']';
		circuit[TSP_SOLUTION_CIRCUIT_STRING_MAX - 1] = '\0';
	}

	// strlen("{ cost = %d, circuit = %s }") == 27
	// 27 - strlen("%d%s") = 23
	// strmaxlen(circuit) == 53
	// 23 + 53 = 76
	// 128 - 76 = 56, cost's %d has 56 chars; just to be extra safe.
	offset = sprintf(string, "{ cost = %d, circuit = %s }", solution->cost, circuit);

	// I just found out about snprintf ...
	// TODO: rewrite the many `sprintf` to be `snprintf`.
}

message_t* tsp_solution_encode(tsp_t* problem, tsp_solution_t* solution)
{
	const int N = problem->n;
	const int TSP_SOLUTION_COUNT = 1 + N; // 1 int for cost + N ints for circuit

	int count = TSP_SOLUTION_COUNT;
	int* buffer = (int*) malloc(count * sizeof(int));

	buffer[0] = solution->cost;

	int offset = 1;
	memcpy(buffer + offset, solution->circuit, N * sizeof(int));

	message_t* msg = message_new(MPI_INT);
	msg->buffer = (void*) buffer;
	msg->count = count;
	return msg;
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
	root->notvisited = (int*) malloc((N-1) * sizeof(int));
	for (int node = 0, index = 0; node < N; node++)
		if (node != initial_node)
			root->notvisited[index++] = node;
	search->list = list_new((void*) root);

	return search;
}

void tsp_search_del(tsp_search_t* search)
{
	if (search->optimum)
		tsp_solution_del(search->optimum);
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

	// Check if x is a search tree leaf, i.e. a solution.
	// And save x's solution if it's better than our current optimum.
	if (x->depth == N - 1)
	{
		tsp_solution_t* s = tsp_solution_new(search->problem, x);
		if (!search->optimum)
			search->optimum = s;
		else
		{
			if (s->cost < search->optimum->cost)
			{
				tsp_solution_del(search->optimum);
				search->optimum = s;
			}
			else
				tsp_solution_del(s);
		}
	}

	// %DEBUG%
	// int x_visited_count = x->depth + 1;
	// printf("x->visited = ");
	// for (int j = 0; j < x_visited_count; j++)
	// 	printf("%d ", x->visited[j]);
	// printf("\n"); fflush(stdout);
	// int x_notvisited_count = N - x_visited_count;
	// printf("x->notvisited = ");
	// for (int j = 0; j < x_notvisited_count; j++)
	// 	printf("%d ", x->notvisited[j]);
	// printf("\n"); fflush(stdout);
	// %DEBUG%

	// Expand x's children and add them to the search list.
	int x_notvisited_count = N - (x->depth + 1);
	// %DEBUG%
	// printf("x_notvisited_count = %d\n", x_notvisited_count);
	// %DEBUG%
	for (int i = 0; i < x_notvisited_count; i++)
	{
		// %DEBUG%
		// printf("\ni = %d\tx->notvisited[%d] = %d\tN = %d\n", i, i, x->notvisited[i], N);
		// %DEBUG%

		tsp_search_node_t* y = tsp_search_node_new(x, x->notvisited[i], N);

		// %DEBUG%
		// int y_visited_count = y->depth + 1;
		// printf("y_visited_count = %d\n", y_visited_count);
		// printf("y->visited = "); fflush(stdout);
		// for (int j = 0; j < y_visited_count; j++)
		// 	printf("%d ", y->visited[j]); fflush(stdout);
		// printf("\n"); fflush(stdout);
		// int y_notvisited_count = N - y_visited_count;
		// printf("y_notvisited_count = %d\n", y_notvisited_count);
		// printf("y->notvisited = "); fflush(stdout);
		// for (int j = 0; j < y_notvisited_count; j++)
		// 	printf("%d ", y->notvisited[j]); fflush(stdout);
		// printf("\n"); fflush(stdout);
		// %DEBUG%

		if (strategy == TSP_SEARCH_DEPTH_FIRST)
			list_stack(search->list, (void*) y);
		else
			list_enqueue(search->list, (void*) y);
	}

	tsp_search_node_del(x);
}

tsp_search_t* tsp_search_decode(tsp_t* problem, message_t* recvmsg)
{
	const int N = problem->n;
	const int TSP_SEARCH_NODE_COUNT = 1 + N; // 1 for depth + N for vist+notvist

	list_t* search_list = list_new(NULL);
	message_t tmp;
	for (int i = 0; i < recvmsg->count; i += TSP_SEARCH_NODE_COUNT)
	{
		tmp.buffer = (void*) (((int*) recvmsg->buffer) + i);
		tmp.count = TSP_SEARCH_NODE_COUNT;
		tsp_search_node_t* search_node = tsp_search_node_decode(problem, &tmp);
		list_enqueue(search_list, (void*) search_node);
	}

	tsp_search_t* search = (tsp_search_t*) malloc(sizeof(tsp_search_t));
	search->problem = problem;
	search->optimum = NULL;
	search->list = search_list;

	if (recvmsg->count == 0)
	{
		search->optimum = (tsp_solution_t*) malloc(sizeof(tsp_solution_t));
		search->optimum->problem = problem;
		search->optimum->cost = INT_MAX;
		search->optimum->circuit = (int*) malloc(N * sizeof(int)); // this solution should never be printed.
		memset(search->optimum->circuit, 0, N * sizeof(int));
	}

	return search;
}
