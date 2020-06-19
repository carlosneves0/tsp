#ifndef __DEBUG_H__
#define __DEBUG_H__

/**
 * Turn debug output on or off.
 */
#define __DEBUG__ 1

/**
 * Debug output goes to stderr.
 */
void debug(const char* format, ...);

#endif
