#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <mpi.h>

/**
 * Turn debug output on or off.
 */
#ifdef DEBUG
#define __DEBUG__ 1
#else
#define __DEBUG__ 0
#endif

/** Both declared and initialized in main.c. */
extern int my_rank;
extern char my_node[MPI_MAX_PROCESSOR_NAME];

/**
* Debug macro to fprintf stuff to stderr.
*/
#define debug(tag, fmt, ...) \
	do { if (__DEBUG__) fprintf(stderr, "[%s][%d][" tag "] " fmt "\n", my_node, my_rank, __VA_ARGS__); fflush(stderr); } while (0)

#endif
