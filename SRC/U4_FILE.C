/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "u4_dos.h"

/*
 * Write a message to the standard output (DOS handle 1), for the file errors.
 * Exits the program (code 3) if it cannot be written.
 */
int16 /*C_0812*/Console(msg)
char *msg;
{
	int16 written;

	if((written = dwrite(1, msg, strlen(msg))) == -1 || strlen(msg) != written) {
		exit(3);
	}
}

/*
 * Read size bytes of the file fname into buf.
 * Returns 0, or -1 after printing what went wrong.
 * Note: the file is not closed when the read fails.
 */
int16 /*C_0855*/Load(fname, size, buf)
char *fname;
uint16 size;
void far *buf;
{
	int16 fd, actual;

	if((fd = dopen(fname, 0)) == -1) {
		Console("\r\nOpening ");
		Console(fname);
		return -1;
	}
	if((actual = dread(fd, buf, size)) == -1 || size != (uint16)(actual)) {
		Console("\r\nReading ");
		if(actual == -1) {
			Console(fname);
		} else {
			Console(fname);
			Console(": File wrong size\r\n");
		}
		return -1;
	}
	if(dclose(fd) == -1) {
		Console("\r\nClosing ");
		Console(fname);
		return -1;
	}
	return 0;
}

/*
 * Write size bytes from buf to the (existing) file fname.
 * Returns 0, or -1 after printing what went wrong; a failed write
 * exits the program (code 1).
 */
int16 /*C_08D1*/Save(fname, size, buf)
char *fname;
uint16 size;
void far *buf;
{
	int16 fd, actual;

	if((fd = dopen(fname, 1)) == -1) {
		Console("\r\nOpening ");
		Console(fname);
		return -1;
	}
	if((actual = dwrite(fd, buf, size)) == -1 || size != (uint16)(actual)) {
		Console("\r\nWriting ");
		if(actual == -1) {
			Console(fname);
		} else {
			Console(fname);
			Console(": File wrong size\r\n");
		}
		exit(1);
	}
	if(dclose(fd) == -1) {
		Console("\r\nClosing ");
		Console(fname);
		return -1;
	}
	return 0;
}
