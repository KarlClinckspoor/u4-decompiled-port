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

/*u_rand_b (UTIL2.ASM) starts with "adc al,...": it adds the carry flag the caller's
  code left. The port is given that carry: RND2_CARRY(the condition whose comparison set
  it) right before U4_RND2, where it can be set; 0 elsewhere*/
#ifdef M_I86
#define RND2_CARRY(cf)
#else
extern int u_rand_b_carry;
#define RND2_CARRY(cf) (u_rand_b_carry = (cf)),
#endif

#endif
