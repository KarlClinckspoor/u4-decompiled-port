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

#endif
