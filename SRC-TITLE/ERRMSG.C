/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

/*Not compiled: the build assembles ERRMSG.ASM, which holds the same two strings.
 They are shown by the INT 24h (critical error) handler of UTIL2.ASM: the disk error on
 the bottom text line, waiting for a key before retrying; the other for a fatal error
 that is not a disk error (the handler then tells DOS to abort the program).*/
char huge /*D_3906*/MSG_DSKERR[] = "Disk error. Press any key to try again.";
char huge /*D_392E*/MSG_SYSERR[] = "DOS: Fatal System Error!";
