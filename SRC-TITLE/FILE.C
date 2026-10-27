/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>

/*
 * Write a string to standard output (DOS handle 1); exit with code 3 if it could not be
 * written completely.
 */
/*C_0010*/Console(msg)
char *msg;
{
	int written;

	if((written = write(1, msg, strlen(msg))) == -1 || strlen(msg) != written) {
		exit(3);
	}
}

/*
 * Load the file fname (exactly size bytes) into the far buffer dest, going through a
 * temporary near buffer taken from malloc. Returns 0, or -1 when the file cannot be
 * opened, read (or has the wrong size) or closed (a message is printed); exits with
 * code 0x13 when the temporary buffer cannot be allocated.
 */
/*C_0051*/Load(fname, size, dest)
char *fname;
unsigned size;
void far *dest;
{
	void *nearBuf;
	int handle;
	void far *farPtr;
	int readCount;

	nearBuf = malloc(size);
	farPtr = nearBuf;
	if(nearBuf == 0) {
		Console(/*D_003A*/"\r\nError allocating space\r\n");
		exit(0x13);
	}

	if((handle = open(fname, O_BINARY|O_RDONLY)) == -1) {
		Console("\r\nOpening ");
		Console(fname);
		return -1;
	}
	if((readCount = read(handle, nearBuf, size)) == -1 || size != readCount) {
		Console("\r\nReading ");
		if(readCount == -1) {
			Console(fname);
		} else {
			Console(fname);
			Console(": File wrong size\r\n");
		}
		return -1;
	}
	if(close(handle) == -1) {
		Console("\r\nClosing ");
		Console(fname);
		return -1;
	}
	movedata(FP_SEG(farPtr), FP_OFF(farPtr), FP_SEG(dest), FP_OFF(dest), size);
	free(nearBuf);

	return 0;
}

/*
 * Write size bytes from buf to the existing file fname (it is opened for writing, not
 * created). Returns 0, or -1 when the file cannot be opened or closed; a write error or
 * short write exits the program with code 1.
 * Unlike AVATAR.EXE's version, the source buffer is near, not far.
 */
/*C_0110*/Save(fname, size, buf)
char *fname;
unsigned size;
void /*far */*buf;
{
	int handle, writeCount;

	if((handle = open(fname, O_BINARY|O_WRONLY)) == -1) {
		Console("\r\nOpening ");
		Console(fname);
		return -1;
	}
	if((writeCount = write(handle, buf, size)) == -1 || size != writeCount) {
		Console("\r\nWriting ");
		if(writeCount == -1) {
			Console(fname);
		} else {
			Console(fname);
			Console(": File wrong size\r\n");
		}
		exit(1);
	}
	if(close(handle) == -1) {
		Console("\r\nClosing ");
		Console(fname);
		return -1;
	}
	return 0;
}
