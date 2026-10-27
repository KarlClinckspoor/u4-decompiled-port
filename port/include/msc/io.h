/* stand-in for Microsoft C's io.h: the low-level file calls TITLE.EXE's FILE.C makes,
 * on the port's DOS file system (port/sys/msc.c) */
#ifndef PORT_MSC_IO_H
#define PORT_MSC_IO_H
#define open port_open
#define read port_read
#define write port_write
#define close port_close
int port_open(const char *name, int mode, ...);
int port_read(int handle, void *buf, unsigned count);
int port_write(int handle, const void *buf, unsigned count);
int port_close(int handle);
#endif
