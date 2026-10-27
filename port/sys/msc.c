/* replaces the pieces of Microsoft C 4.00's library that are DOS calls (the rest is the
 * host's C library): TITLE.EXE's open/read/write/close (io.h, on the port's DOS file
 * system) and its far heap. */
#include "dosfs.h"
#include "u4rt.h"
#include "../platform/plat.h"

#include <stdarg.h>
#include <stdlib.h>

int port_open(const char *name, int mode, ...)
{
	int16_t h;
	vc_cycles(3000);
	return dosfs_open(name, mode & 3, &h) == 0 ? h : -1;
}

int port_read(int handle, void *buf, unsigned count)
{
	uint16_t done = 0;
	vc_add_ns((uint64_t)count * 10000u);
	return dosfs_read((int16_t)handle, buf, (uint16_t)count, &done) == 0 ? done : -1;
}

int port_write(int handle, const void *buf, unsigned count)
{
	uint16_t done = 0;
	if (handle == 1 || handle == 2) {
		plat_log("%.*s", (int)count, (const char *)buf);
		return (int)count;
	}
	vc_add_ns((uint64_t)count * 10000u);
	return dosfs_write((int16_t)handle, buf, (uint16_t)count, &done) == 0 ? done : -1;
}

int port_close(int handle)
{
	return dosfs_close((int16_t)handle) == 0 ? 0 : -1;
}

void *port_fmalloc(unsigned n)
{
	return calloc(1, n ? n : 1);
}

void port_ffree(void *p)
{
	free(p);
}

/* strnicmp: compares at most n characters, A-Z as a-z */
int port_strnicmp(const char *a, const char *b, size_t n)
{
	for (; n; n--, a++, b++) {
		int ca = (unsigned char)*a, cb = (unsigned char)*b;
		if (ca >= 'A' && ca <= 'Z')
			ca += 'a' - 'A';
		if (cb >= 'A' && cb <= 'Z')
			cb += 'a' - 'A';
		if (ca != cb)
			return ca - cb;
		if (!ca)
			break;
	}
	return 0;
}

/* LOCAL_AT (portable.h): a read of buf[i] that may fall before the array (i < 0) */
char port_local_at(char *buf, int16_t i)
{
	return i >= 0 ? buf[i] : 0;
}
