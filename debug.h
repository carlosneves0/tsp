#ifndef __DEBUG_H__
#define __DEBUG_H__

/**
 * Turn debug output on or off.
 */
#define __DEBUG__ 1

/**
* Debug macro to fprintf stuff to stderr.
*/
#define debug(fmt, ...) \
	do { if (__DEBUG__) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

#endif
