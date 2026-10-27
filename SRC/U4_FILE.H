/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */
#ifndef __U4_FILE_H__
#define __U4_FILE_H__

#include "portable.h"

#ifdef WIN32
#define far
#endif

extern int16 /*C_0812*/Console(char *);
extern int16 /*C_0855*/Load(char *, uint16, void far *);
extern int16 /*C_08D1*/Save(char *, uint16, void far *);

#endif
