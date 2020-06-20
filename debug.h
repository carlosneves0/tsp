#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <mpi.h>

/**
 * Turn debug output on or off.
 */
#define __DEBUG__ 1

extern int my_rank;
extern char my_node[MPI_MAX_PROCESSOR_NAME];

/**
* Debug macro to fprintf stuff to stderr.
*/
#define debug(tag, fmt, ...) \
	do { if (__DEBUG__) fprintf(stderr, "[%s][%d][" tag "] " fmt "\n", my_node, my_rank, __VA_ARGS__); } while (0)

#endif
