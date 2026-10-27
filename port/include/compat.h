/* The portability layer, force-included (-include) into every game source compiled for the
 * host. The game's C stays as Microsoft C 4.00 saw it; this file gives it a meaning on a
 * 64-bit host:
 *   - the memory-model and calling-convention keywords go away (every pointer is a flat
 *     host pointer, and the port's calls need no pascal/cdecl distinction);
 *   - WIN32 is not defined: the sources' "#ifdef WIN32" parts belong to an old Visual
 *     Studio build (forVS/) that changed the game; the port keeps DOS's behaviour;
 *   - exit() leaves the game through the port (port_exit), main is renamed game_main;
 *   - every struct is packed (MSC /Zp), and globals may sit at any address (below).
 */
#ifndef PORT_COMPAT_H
#define PORT_COMPAT_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#undef WIN32
#undef _WIN32_GAME

#define far
#define near
#define huge
#define pascal
#define cdecl
#define interrupt
#define _cdecl

/* ---- the C library of Microsoft C 4.00 that the game uses ---- */
#define exit port_exit
void port_exit(int code);
#define strnicmp port_strnicmp
int port_strnicmp(const char *a, const char *b, size_t n);
/* TITLE.EXE's far heap */
#define _fmalloc(n) port_fmalloc(n)
#define _ffree(p) port_ffree(p)
void *port_fmalloc(unsigned n);
void port_ffree(void *p);

/* ---- the game's entry point ---- */
#define main game_main

/* MSC /Zp packs structs: byte alignment for every struct the game declares. */
#pragma pack(1)

/* The game's variables live at DOS's offsets inside blocks (port/sys/bss.c, the blocks
 * made by port/tools/gen_block.py), so an array may start at any address. The x86-64 ABI
 * lets a compiler assume 16-byte alignment for any global array of 16 bytes or more, and
 * clang does for extern declarations too. Declaring every extern with alignment 1 takes
 * that assumption away. (After the system headers above.) */
#define extern extern __attribute__((aligned(1)))

#endif
