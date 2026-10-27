/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */
/*
	Types and macros shared by the DOS build and the native port (port/).

	Under Microsoft C 4.00 (M_I86) every one is the original type or the
	original tokens, so the DOS build compiles to the same bytes
	(tools/dosbuild.py checks AVATAR.EXE and TITLE.EXE). On a modern host int
	is 32 bits and long 64 (32 on Windows), so the game's integers are written
	with explicit widths: int16 for MSC's int, uint16 for unsigned, int32 for
	long, uint32 for unsigned long.
*/
#ifndef __PORTABLE_H__
#define __PORTABLE_H__

#ifdef M_I86
typedef int int16;
typedef unsigned uint16;
typedef long int32;
typedef unsigned long uint32;
#else
#include <stdint.h>
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
#endif

/*the body of a busy-wait loop ("while(--count);"): nothing in DOS, where the loop itself
  is the wait; the port adds the time one pass of MSC's loop takes on the PC (u4rt.h)*/
#ifdef M_I86
#define BUSY_PASS
#else
extern void port_busy_pass(void);
#define BUSY_PASS port_busy_pass()
#endif

/*a read that can fall before the start of a local array (u4_gets on an empty answer
  reads buf[-1]): DOS reads whatever its stack holds there, the port reads a 0*/
#ifdef M_I86
#define LOCAL_AT(buf, i) buf[i]
#else
extern char port_local_at(char *buf, int16 i);
#define LOCAL_AT(buf, i) port_local_at(buf, i)
#endif

#endif
