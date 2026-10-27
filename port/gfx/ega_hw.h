/* The EGA's planes and its write paths (mode 0Dh, 320x200, 40 bytes a line), shared by
 * EGA.DRV in C (ega.c) and TITLE.EXE's own EGA routines (tgfx.c).
 *
 *   - plane writes through the map mask (write mode 0, function "replace", bit mask FFh);
 *   - read-modify-write instructions ("or es:[di],0FFh") with a logical function and a bit
 *     mask: the read loads the four latches, the written byte is combined with them;
 *   - write mode 1 copies ("rep movsb" inside the video memory): each plane gets its latch.
 */
#ifndef PORT_EGA_HW_H
#define PORT_EGA_HW_H

#include <stdint.h>

#define EGA_LINE 40                 /* bytes a line */
#define EGA_PLANE 0x10000           /* the video segment of each plane */

/* plane k holds bit k of each pixel's colour; bit 7 of a byte is the leftmost pixel */
extern uint8_t ega_vram[4][EGA_PLANE];
/* the colours: 16 EGA palette registers (0..3Fh, rgbRGB) */
extern uint8_t ega_palette_regs[16];
/* set when the planes change; ega_take_dirty() reads and clears it */
extern int ega_dirty;
/* the 8088 cycles the writes took on the PC (the port's clock, port/sys/u4rt.h) */
extern uint32_t ega_cost;
#define EGA_COST_PUT 15     /* mov es:[di],al: one bus write through the map mask */
#define EGA_COST_RMW 30     /* or es:[di],imm: a read and a write */
#define EGA_COST_COPY 25    /* movsb in write mode 1 */

/* ---- the EGA's write paths ---- */

enum { F_REPLACE = 0x00, F_AND = 0x08, F_OR = 0x10, F_XOR = 0x18 };

/* a byte written to the planes of mapmask, write mode 0, replace, bit mask FFh */
static inline void put(int ofs, int mapmask, uint8_t v)
{
	int p;
	ega_cost += EGA_COST_PUT;
	for (p = 0; p < 4; p++)
		if (mapmask & (1 << p))
			ega_vram[p][ofs & 0xFFFF] = v;
}

/* a read-modify-write (or/and/xor es:[di],imm): the read loads the latches from ofs, then
 * imm goes through the logical function with the latch and the bit mask */
static inline void rmw(int ofs, int mapmask, int func, uint8_t bitmask, uint8_t imm)
{
	int p;
	ega_cost += EGA_COST_RMW;
	ofs &= 0xFFFF;
	for (p = 0; p < 4; p++) {
		uint8_t latch = ega_vram[p][ofs], d = imm;
		if (!(mapmask & (1 << p)))
			continue;
		switch (func) {
		case F_AND: d &= latch; break;
		case F_OR: d |= latch; break;
		case F_XOR: d ^= latch; break;
		}
		ega_vram[p][ofs] = (uint8_t)((d & bitmask) | (latch & ~bitmask));
	}
}

/* a byte copied inside the video memory in write mode 1 (the latches of src to dst) */
static inline void copy(int dst, int src, int mapmask)
{
	int p;
	ega_cost += EGA_COST_COPY;
	for (p = 0; p < 4; p++)
		if (mapmask & (1 << p))
			ega_vram[p][dst & 0xFFFF] = ega_vram[p][src & 0xFFFF];
}

static inline int plane_of(int mask)
{
	return mask == 8 ? 3 : mask == 4 ? 2 : mask == 2 ? 1 : 0;
}

#endif
