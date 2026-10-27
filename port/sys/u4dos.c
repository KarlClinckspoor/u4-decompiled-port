/* replaces U4_DOS.ASM: AVATAR.EXE's memory and files through DOS (int 21h), on the port's
 * DOS file system (dosfs.h) and the host's heap.
 *
 *   dalloc(n)           48h: n bytes (rounded up to paragraphs); exits with code 2Dh if
 *                       DOS has not enough memory
 *   dfree(p)            49h
 *   dopen(name, mode)   3Dh: a handle, or -1
 *   dlseek(h, pos)      42h from the start: the new position, or -1
 *   dread/dwrite(h, buf, n)  3Fh/40h: the count done, or -1
 *   dclose(h)           3Eh: 0, or -1
 */
#include "dosfs.h"
#include "u4rt.h"

#include <stdlib.h>

void port_exit(int code);

void *dalloc(int16_t size)
{
	uint32_t paras = ((uint16_t)size + 15u) >> 4;
	void *p = calloc(1, paras ? paras * 16u : 16u);
	if (!p)
		port_exit(0x2D);
	return p;
}

int dfree(void *p)
{
	free(p);
	return 0;
}

int dopen(char *name, int16_t mode)
{
	int16_t h;
	vc_cycles(3000);
	return dosfs_open(name, mode & 0xFF, &h) == 0 ? h : -1;
}

int32_t dlseek(int16_t h, uint32_t pos)
{
	uint32_t at;
	return dosfs_seek(h, 0, (int32_t)pos, &at) == 0 ? (int32_t)at : -1;
}

/* a disk reads ~100 KB/s on the PC of the period: the time the data took */
static void disk_time(uint16_t n)
{
	vc_cycles(2000);
	vc_add_ns((uint64_t)n * 10000u);
}

int dread(int16_t h, void *buf, int16_t count)
{
	uint16_t done = 0;
	disk_time((uint16_t)count);
	return dosfs_read(h, buf, (uint16_t)count, &done) == 0 ? (int16_t)done : -1;
}

int dwrite(int16_t h, void *buf, int16_t count)
{
	uint16_t done = 0;
	if (h == 1 || h == 2) {	/* the console (Console's error messages) */
		extern void plat_log(const char *fmt, ...);
		plat_log("%.*s", (int)(uint16_t)count, (const char *)buf);
		return (uint16_t)count;
	}
	disk_time((uint16_t)count);
	return dosfs_write(h, buf, (uint16_t)count, &done) == 0 ? (int16_t)done : -1;
}

int dclose(int16_t h)
{
	return dosfs_close(h) == 0 ? 0 : -1;
}
