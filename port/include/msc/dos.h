/* stand-in for Microsoft C's dos.h: far pointers are host pointers */
#ifndef PORT_MSC_DOS_H
#define PORT_MSC_DOS_H
#include <string.h>
/* movedata(srcseg, srcoff, dstseg, dstoff, n) with FP_SEG/FP_OFF of host pointers: the
 * "segment" carries the pointer, the offset is 0 */
#define FP_SEG(p) ((void *)(p))
#define FP_OFF(p) 0
#define movedata(ss, so, ds, dso, n) memmove((ds), (ss), (n))
#endif
