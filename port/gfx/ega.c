/* EGA.DRV in C: the graphics driver AVATAR.EXE loads on an EGA (see ega.h).
 *
 * The driver works on the EGA's planes through its registers. Here the planes are
 * ega_vram[0..3] and each kind of access the driver makes is a small function below:
 *   - plane writes through the map mask (write mode 0, function "replace", bit mask FFh);
 *   - read-modify-write instructions ("or es:[di],0FFh") with a logical function and a bit
 *     mask: the read loads the four latches, the written byte is combined with them;
 *   - write mode 1 copies ("rep movsb" inside the video memory): each plane gets its latch.
 * Addresses in the comments are the driver's own (EGA.DRV, offsets in its segment).
 */
#include "ega.h"
#include "ega_hw.h"

#include <string.h>

uint8_t ega_vram[4][EGA_PLANE];
/* 01E2h: the palette registers given to int 10h AX=1002h */
uint8_t ega_palette_regs[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};
/* 01F3h: the map mask (the planes) for the line colours 0..3 */
static const uint8_t color_planes[4] = {0x00, 0x02, 0x01, 0x07};

static uint8_t *shapes, *charset;  /* 0046h, 004Ah: far pointers given to Init */
static uint8_t seed[4] = {0x12, 0x34, 0x56, 0x78};  /* 0036h */
int ega_dirty = 1;

#define LINE(y) ((y) * EGA_LINE)    /* 0052h: the table of line offsets */

int ega_take_dirty(void)
{
	int d = ega_dirty;
	ega_dirty = 0;
	return d;
}

void ega_to_chunky(uint8_t *out)
{
	int i, b;
	for (i = 0; i < EGA_LINE * 200; i++) {
		uint8_t p0 = ega_vram[0][i], p1 = ega_vram[1][i], p2 = ega_vram[2][i], p3 = ega_vram[3][i];
		for (b = 7; b >= 0; b--)
			*out++ = (uint8_t)((p0 >> b & 1) | (p1 >> b & 1) << 1 | (p2 >> b & 1) << 2 | (p3 >> b & 1) << 3);
	}
}

void ega_rgb6(int v, uint8_t *rgb)
{
	/* rgbRGB: bit 5 r, 4 g, 3 b (one third), bits 2..0 R G B (two thirds) */
	rgb[0] = (uint8_t)((v & 4 ? 42 : 0) + (v & 32 ? 21 : 0));
	rgb[1] = (uint8_t)((v & 2 ? 42 : 0) + (v & 16 ? 21 : 0));
	rgb[2] = (uint8_t)((v & 1 ? 42 : 0) + (v & 8 ? 21 : 0));
}

/* 0EC6h: the driver's own random numbers (the carry going in is always clear) */
static uint8_t rnd(void)
{
	uint8_t al = (uint8_t)(seed[3] + seed[2]);
	seed[2] = al;
	seed[1] ^= al;
	al = (uint8_t)(seed[1] + seed[0]);
	al = (uint8_t)(al >> 1 | al << 7);
	seed[0] = al;
	seed[3] = al;
	return al;
}

static int sign(int v) { return v < 0 ? -1 : v > 0 ? 1 : 0; }   /* 0EEEh */
static int iabs(int v) { return v < 0 ? -v : v; }                 /* 0F09h */

/* ---- the entries ---- */

/* 11C6h: the white 16x16 frame around the active character (row, col: view tiles) */
void ega_active_box(int row, int col)
{
	int di = LINE(row * 16 + 8) + col * 2 + 1, i;
	rmw(di, 15, F_OR, 0xFF, 0xFF); rmw(di + 1, 15, F_OR, 0xFF, 0xFF);
	di += EGA_LINE;
	rmw(di, 15, F_OR, 0xFF, 0xFF); rmw(di + 1, 15, F_OR, 0xFF, 0xFF);
	di += EGA_LINE;
	for (i = 0; i < 12; i++)
		rmw(di + i * EGA_LINE, 15, F_OR, 0xC0, 0xC0);
	for (i = 0; i < 12; i++)
		rmw(di + 1 + i * EGA_LINE, 15, F_OR, 0x03, 0x03);
	di += 12 * EGA_LINE;
	rmw(di, 15, F_OR, 0xFF, 0xFF); rmw(di + 1, 15, F_OR, 0xFF, 0xFF);
	di += EGA_LINE;
	rmw(di, 15, F_OR, 0xFF, 0xFF); rmw(di + 1, 15, F_OR, 0xFF, 0xFF);
	ega_dirty = 1;
}

/* 0277h: row trow of a tile drawn at line srow, column scol (pixels / 4), zoom 1 or 2
 * (each pixel doubled): the dungeon's monsters, shrinking with the distance */
void ega_zoom_tile_row(int tile, int trow, int srow, int scol, int zoom)
{
	const uint8_t *row = shapes + (tile & 0x1FF) * 128 + trow * 2;
	int mask;
	for (mask = 8; mask; mask >>= 1, row += 0x20) {
		int di = LINE(srow) + (scol >> 1), i, p = plane_of(mask);
		for (i = 0; i < 2; i++) {
			uint8_t al = row[i];
			if (zoom != 1) {
				uint16_t bx = 0;
				int b;
				for (b = 7; b >= 0; b--)
					bx = (uint16_t)(bx << 2 | (al >> b & 1) * 3);
				ega_vram[p][di++ & 0xFFFF] = (uint8_t)(bx >> 8);
				ega_vram[p][di++ & 0xFFFF] = (uint8_t)bx;
			} else {
				ega_vram[p][di++ & 0xFFFF] = al;
			}
		}
	}
	ega_dirty = 1;
}

/* a tile's 16 lines of plane p (2 bytes a line) to the screen at di */
static void tile_plane(int di, const uint8_t *src, int p)
{
	int k;
	for (k = 0; k < 16; k++) {
		ega_vram[p][(di + k * EGA_LINE) & 0xFFFF] = src[k * 2];
		ega_vram[p][(di + k * EGA_LINE + 1) & 0xFFFF] = src[k * 2 + 1];
	}
}

/* 0350h: h x w copies of one tile from line row, column col (pixels / 4) */
void ega_fill_tiles(int h, int w, int tile, int row, int col)
{
	const uint8_t *t = shapes + (tile & 0x1FF) * 128;
	int line = row;
	for (; h > 0; h--, line += 16) {
		int di = LINE(line) + (col >> 1), n = w & 0xFF;
		/* test fix (ega_diff.py): the width is a byte counted down after each tile
		 * ("dec dl; jnz"), so 0 (mod 256) draws 256 tiles; this loop drew none */
		do {
			int mask;
			const uint8_t *s = t;
			for (mask = 8; mask; mask >>= 1, s += 32)
				tile_plane(di, s, plane_of(mask));
			di += 2;
		} while (--n & 0xFF);
	}
	ega_dirty = 1;
}

/* 041Dh, 048Fh: the view shaken by two lines down or up, with the speaker crackling */
static void shift(int down, int sound)
{
	int bx, toggles = 0;
	for (bx = down ? 0x15A : 0; down ? bx != 0 : bx != 0x15A; bx += down ? -2 : 2) {
		int r = bx / 2, di, si, i;
		di = LINE(r + (down ? 10 : 8)) + 1;
		si = LINE(r + (down ? 8 : 10)) + 1;
		for (i = 0; i < 0x16; i++)
			copy(di + i, si + i, 15);
		if (sound && (rnd() & 1))
			toggles++;
	}
	ega_speaker_noise(sound ? toggles : -1);
	ega_dirty = 1;
}
void ega_shift_down(int sound) { shift(1, sound & 0xFF); }
void ega_shift_up(int sound) { shift(0, sound & 0xFF); }

/* 0505h: the 11x11 view at (8, 8) from a table of tile numbers (h lines of 11; the width
 * argument is not used) */
void ega_draw_tiles(int h, int w, const uint8_t *tiles)
{
	int line = 8;
	(void)w;
	for (; h > 0; h--, line += 16) {
		int di = LINE(line) + 1, n;
		for (n = 0; n < 11; n++, di += 2) {
			const uint8_t *s = shapes + (*tiles++) * 128;
			int mask;
			for (mask = 8; mask; mask >>= 1, s += 32)
				tile_plane(di, s, plane_of(mask));
		}
	}
	ega_dirty = 1;
}

/* 088Eh: a pixel of colour 0..3 (the line colours) */
void ega_dot(int y, int x, int color)
{
	int di = LINE(y) + ((unsigned)x >> 3);
	uint8_t bit = (uint8_t)(0x80 >> (x & 7));
	rmw(di, 15, F_AND, bit, 0);
	rmw(di, color_planes[color & 0xFF], F_OR, bit, 0xFF);
	ega_dirty = 1;
}

/* 05D3h: the text window (columns 24-39, text lines 12-23) scrolled up by a line */
void ega_cr(void)
{
	int r, i;
	for (r = 96; r < 96 + 0x58; r++)
		for (i = 0; i < 16; i++)
			copy(LINE(r) + 0x18 + i, LINE(r + 8) + 0x18 + i, 15);
	for (r = 184; r < 192; r++)
		for (i = 0; i < 16; i++)
			put(LINE(r) + 0x18 + i, 15, 0);
	ega_dirty = 1;
}

/* 0646h: the view (176x176 at 8, 8) inverted: colour c becomes c ^ 7 */
void ega_invert_view(void)
{
	int r, i;
	for (r = 8; r < 8 + 0xB0; r++)
		for (i = 1; i <= 0x16; i++)
			rmw(LINE(r) + i, 7, F_XOR, 0xFF, 0xFF);
	ega_dirty = 1;
}

/* 0694h: the view cleared */
void ega_clear_view(void)
{
	int r, i;
	for (r = 8; r < 184; r++)
		for (i = 1; i <= 22; i++)
			put(LINE(r) + i, 15, 0);
	ega_dirty = 1;
}

/* 06C9h: line `line` of the status area (a party member) inverted */
void ega_invert_status_line(int line)
{
	int r, i, y = ((line + 1) & 0xFFFF) * 8;
	for (r = 0; r < 8; r++)
		for (i = 0; i < 15; i++)
			rmw(LINE(y + r) + 0x18 + i, 7, F_XOR, 0xFF, 0xFF);
	ega_dirty = 1;
}

/* 0724h: one character cell inverted (not called by the game) */
void ega_invert_char(int x, int y)
{
	int r, bx = LINE(y * 8) + x;
	for (r = 0; r < 8; r++)
		rmw(bx + r * EGA_LINE, 7, F_XOR, 0xFF, 0xFF);
	ega_dirty = 1;
}

/* 0792h: the status area (lines 8-71, columns 24-38) cleared */
void ega_clear_status(void)
{
	int r, i;
	for (r = 8; r < 72; r++)
		for (i = 0; i < 15; i++)
			put(LINE(r) + 0x18 + i, 15, 0);
	ega_dirty = 1;
}

/* 07C4h: a pixel ORed with a line colour */
void ega_dot_or(int y, int x, int color)
{
	rmw(LINE(y) + ((unsigned)x >> 3), color_planes[color & 0xFF], F_REPLACE, (uint8_t)(0x80 >> (x & 7)), 0xFF);
	ega_dirty = 1;
}

/* 081Dh: a pixel XORed with a line colour */
void ega_dot_xor(int y, int x, int color)
{
	rmw(LINE(y) + ((unsigned)x >> 3), color_planes[color & 0xFF], F_XOR, (uint8_t)(0x80 >> (x & 7)), 0xFF);
	ega_dirty = 1;
}

/* 091Dh: a picture (*.EGA: run-length coded, 02h count value; two pixels a byte) drawn
 * ORed onto the screen, or decoded into buf (4 planes of 8000 bytes, plane 3 first) */
void ega_load_picture(const char *name, int ofs, uint8_t *buf)
{
	uint8_t *data, *si, *end;
	int size;
	if (ega_read_file(name, &data, &size) != 0) {
		ega_fatal_exit(3);
		return;
	}
	si = data;
	end = data + size;
	if (buf) {
		uint8_t ah = 0x80;
		int di = ofs, bx = 0x1F40;
		memset(buf + ofs, 0, 0x7D00);
		for (;;) {
			uint8_t al, ch = 0;
			int cl = 1;
			if (si >= end)
				goto done;
			al = *si++;
			if (al == 2 && si + 2 <= end) {
				cl = *si++;
				al = *si++;
				ch = al;
			}
			for (;;) {
				int half;
				for (half = 0; half < 2; half++) {
					if (al & 0x80) buf[di] |= ah;
					if (al & 0x40) buf[di + 0x1F40] |= ah;
					if (al & 0x20) buf[di + 0x3E80] |= ah;
					if (al & 0x10) buf[di + 0x5DC0] |= ah;
					al = (uint8_t)(al << 4);
					if (half == 0)
						ah >>= 1;
				}
				if (ah & 1) {           /* the shift out of the last pixel of the byte */
					ah = 0x80;
					di++;
					if (--bx == 0)
						goto done;
				} else {
					ah >>= 1;
				}
				al = ch;
				if (--cl & 0xFF)
					continue;
				break;
			}
		}
	} else {
		int dx = 0, bx = 0, di = LINE(0) + ofs;
		uint8_t mask = 0x80;
		for (;;) {
			uint8_t al;
			unsigned cx = 1;     /* "loop": a count of 0 is 65536 here */
			if (si >= end)
				goto done;
			al = *si++;
			if (al == 2 && si + 2 <= end) {
				cx = *si++;
				al = *si++;
			}
			do {
				rmw(di + bx, al >> 4, F_REPLACE, mask, 0xFF);
				mask = (uint8_t)(mask >> 1 | mask << 7);
				if (mask == 0x80)
					bx++;
				rmw(di + bx, al & 0x0F, F_REPLACE, mask, 0xFF);
				mask = (uint8_t)(mask >> 1 | mask << 7);
				if (mask == 0x80)
					bx++;
				if (bx >= 0x28) {
					if (++dx == 200)
						goto done;
					di = LINE(dx);
					bx = 0;
				}
			} while (--cx & 0xFFFF);
		}
	}
done:
	ega_free_file(data);
	ega_dirty = 1;
}

/* 0AE3h: the Codex's picture (RUNE_5) revealed: the view opens from its middle, like a
 * curtain, with the picture appearing in the gap */
void ega_reveal_codex(const char *name)
{
	static uint8_t buf[0x7D00];
	int cl, r, bx;
	ega_load_picture(name, 0, buf);
	for (cl = 1; cl <= 11; cl++) {
		if (cl < 11) {
			for (r = 8; r < 0xB8; r++) {
				int si = LINE(r);
				for (bx = 1; bx < 12 - cl; bx++)
					copy(si + bx, si + bx + 1, 15);
				for (bx = 0x16; bx >= 12 + cl; bx--)
					copy(si + bx, si + bx - 1, 15);
			}
		}
		{
			int mask, plane_ofs = 0;
			for (mask = 8; mask; mask >>= 1, plane_ofs += 0x1F40) {
				int p = plane_of(mask);
				for (r = 8; r < 0xB8; r++) {
					int from = 12 - cl, n = 2 * cl, i;
					for (i = 0; i < n; i++)
						ega_vram[p][LINE(r) + from + i] = buf[LINE(r) + plane_ofs + from + i];
				}
			}
		}
		ega_dirty = 1;
		ega_busy_wait(0x10000L);
	}
}

/* 0C14h: the dungeon's walls: the columns x0..x1-1, each filled from the top edge (y0 to
 * y0p) to the bottom edge (y1 to y1p); colour col0, or black when col1 is set */
void ega_draw_quad(int x0, int x1, int y0, int y1, int y0p, int y1p, int col0, int col1)
{
	int x = x0, ya = y0, yb = y1;
	int dx = (int16_t)(x1 - x0);
	int sx = sign(dx), sa = sign((int16_t)(y0p - y0)), sb = sign((int16_t)(y1p - y1));
	int errA = (uint16_t)iabs(dx) >> 1, errB = errA;
	for (;;) {
		uint8_t bit = (uint8_t)(0x80 >> (x & 7));
		int si = (x & 0xFFFF) >> 3, y, n = (int16_t)(yb - ya) + 1;
		for (y = ya; n > 0; n--, y++) {
			if (col1 & 0xFF)
				rmw(LINE(y) + si, 15, F_REPLACE, bit, 0);
			else
				rmw(LINE(y) + si, color_planes[col0 & 0xFF], F_REPLACE, bit, 0xFF);
		}
		x += sx;
		errA = (int16_t)(errA - iabs((int16_t)(y0p - y0)));
		errB = (int16_t)(errB - iabs((int16_t)(y1p - y1)));
		/* the driver tests the bottom edge's error for both edges */
		if (errB < 0) {
			ya += sa;
			errA = (int16_t)(errA + iabs(dx));
		}
		if (errB < 0) {
			yb += sb;
			errB = (int16_t)(errB + iabs(dx));
		}
		if ((int16_t)x == (int16_t)x1)
			break;
	}
	ega_dirty = 1;
}

/* 0D39h: character 7 rebuilt from character 0 (the ankh), a row a virtue: a row shows
 * when the virtue's karma is 0 (partial Avatarhood) */
void ega_virtue_glyph(const uint16_t *const *karmas)
{
	int si;
	for (si = 7; si >= 0; si--) {
		int k;
		for (k = 0; k < 4; k++)
			charset[0xE0 + si + k * 8] = *karmas[si] == 0 ? charset[si + k * 8] : 0;
	}
}

/* 0D9Eh: the campfire's flames flicker (tile 4Bh) */
void ega_anim_campfire(void)
{
	static const uint8_t m[8][2] = {{0x00, 0x0C}, {0x80, 0x0D}, {0xC0, 0x1F}, {0xC0, 0x1F},
		{0x80, 0x0F}, {0xC0, 0x1F}, {0x40, 0x12}, {0xC0, 0x18}};
	int i;
	for (i = 0; i < 8; i++) {
		uint8_t ah = rnd(), al = rnd();
		shapes[0x25AE + i * 2] ^= (uint8_t)(al & m[i][1]);
		shapes[0x25AF + i * 2] ^= (uint8_t)(ah & m[i][0]);
	}
}

/* 0E2Eh: mode 0Dh, the palette, and the tables */
void ega_init(uint8_t *s, uint8_t *c)
{
	/* test fix (ega_diff.py): the palette was only the static initial value; Init loads
	 * it (int 10h AX=1002h from 01E2h), so it is reloaded here */
	static const uint8_t palette[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};
	shapes = s;
	charset = c;
	memset(ega_vram, 0, sizeof ega_vram);
	memcpy(ega_palette_regs, palette, sizeof palette);
	ega_dirty = 1;
}

/* 0EA5h */
void ega_clear_screen(void)
{
	int p;
	for (p = 0; p < 4; p++)
		memset(ega_vram[p], 0, 8000);
	ega_dirty = 1;
}

/* 0FACh: a tile's rows rotated down by one, each plane (water, lava, force fields) */
void ega_anim_flow(int tile)
{
	uint8_t *t = shapes + (tile & 0x1FF) * 128;
	int p;
	for (p = 0; p < 4; p++) {
		uint8_t *pl = t + p * 32, a = pl[30], b = pl[31];
		memmove(pl + 2, pl, 30);
		pl[0] = a;
		pl[1] = b;
	}
}

/* 0FFFh: the flags of the town, the castle, the ships and Lord British's castle wave:
 * two rows of each swapped, each with an even chance */
void ega_anim_flags(void)
{
	static const uint16_t at[5] = {0x506, 0x582, 0x804, 0x904, 0x702};
	int i, k;
	for (i = 0; i < 5; i++) {
		if (rnd() & 0x80)
			continue;
		for (k = 0; k < 4; k++) {
			uint8_t *a = shapes + at[i] + k * 0x20, t0 = a[0], t1 = a[1];
			a[0] = a[2]; a[1] = a[3];
			a[2] = t0; a[3] = t1;
		}
	}
}

/* 114Ch: character c at text column x, text line y */
void ega_putchar(int y, int x, int c)
{
	const uint8_t *si = charset + (c & 0xFF) * 32;
	int bx = LINE((y & 0xFFFF) * 8) + x, mask, k;
	for (mask = 8; mask; mask >>= 1)
		for (k = 0; k < 8; k++)
			ega_vram[plane_of(mask)][(bx + k * EGA_LINE) & 0xFFFF] = *si++;
	ega_dirty = 1;
}
