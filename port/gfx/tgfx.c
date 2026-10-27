/* TITLE.EXE's EGA graphics in C: the EGA routines of SRC-TITLE/GRAPH.ASM (GR_*_EGA and
 * what they call), InterleaveCharset / InterleaveShapes, LoadCompressed and INFLATE.ASM's
 * LZW decoder _Inflate (see tgfx.h for the entries and the glue).
 *
 * The planes are ega_vram[0..3] (ega_hw.h); each access the routines make goes through
 * the same write path as on the EGA: put() for "mov es:[di],al" in write mode 0 through
 * the map mask, rmw() for "or es:[bx],0FFh" through the bit mask. The register state the
 * routines expect on entry is the one GR_INIT_EGA's mode set leaves and that no title
 * routine changes: write mode 0, function "replace", no set/reset, bit mask FFh (only
 * Gra_Dot changes the bit mask, and puts FFh back).
 *
 * Addresses in the comments are TITLE_0.EXE's: code offsets in _TEXT, data offsets
 * (D_xxxx) in DGROUP. "Not modelled" marks what the original does with arguments the
 * title never passes (indexes past the end of a table read the data that follows it).
 */
#include "tgfx.h"
#include "ega.h"
#include "ega_hw.h"

#include <string.h>

/* ---- GRAPH.ASM's data ---- */

/* 023Ch Lines: the offset of each line. The CGA's interleaved table until GR_INIT_EGA
 * rewrites it to y * 40. Indexes from 200 on are not modelled (the original reads D_03CC,
 * then DriverTable). */
static uint16_t lines[200] = {
	0x0000, 0x2000, 0x0050, 0x2050, 0x00A0, 0x20A0, 0x00F0, 0x20F0,
	0x0140, 0x2140, 0x0190, 0x2190, 0x01E0, 0x21E0, 0x0230, 0x2230,
	0x0280, 0x2280, 0x02D0, 0x22D0, 0x0320, 0x2320, 0x0370, 0x2370,
	0x03C0, 0x23C0, 0x0410, 0x2410, 0x0460, 0x2460, 0x04B0, 0x24B0,
	0x0500, 0x2500, 0x0550, 0x2550, 0x05A0, 0x25A0, 0x05F0, 0x25F0,
	0x0640, 0x2640, 0x0690, 0x2690, 0x06E0, 0x26E0, 0x0730, 0x2730,
	0x0780, 0x2780, 0x07D0, 0x27D0, 0x0820, 0x2820, 0x0870, 0x2870,
	0x08C0, 0x28C0, 0x0910, 0x2910, 0x0960, 0x2960, 0x09B0, 0x29B0,
	0x0A00, 0x2A00, 0x0A50, 0x2A50, 0x0AA0, 0x2AA0, 0x0AF0, 0x2AF0,
	0x0B40, 0x2B40, 0x0B90, 0x2B90, 0x0BE0, 0x2BE0, 0x0C30, 0x2C30,
	0x0C80, 0x2C80, 0x0CD0, 0x2CD0, 0x0D20, 0x2D20, 0x0D70, 0x2D70,
	0x0DC0, 0x2DC0, 0x0E10, 0x2E10, 0x0E60, 0x2E60, 0x0EB0, 0x2EB0,
	0x0F00, 0x2F00, 0x0F50, 0x2F50, 0x0FA0, 0x2FA0, 0x0FF0, 0x2FF0,
	0x1040, 0x3040, 0x1090, 0x3090, 0x10E0, 0x30E0, 0x1130, 0x3130,
	0x1180, 0x3180, 0x11D0, 0x31D0, 0x1220, 0x3220, 0x1270, 0x3270,
	0x12C0, 0x32C0, 0x1310, 0x3310, 0x1360, 0x3360, 0x13B0, 0x33B0,
	0x1400, 0x3400, 0x1450, 0x3450, 0x14A0, 0x34A0, 0x14F0, 0x34F0,
	0x1540, 0x3540, 0x1590, 0x3590, 0x15E0, 0x35E0, 0x1630, 0x3630,
	0x1680, 0x3680, 0x16D0, 0x36D0, 0x1720, 0x3720, 0x1770, 0x3770,
	0x17C0, 0x37C0, 0x1810, 0x3810, 0x1860, 0x3860, 0x18B0, 0x38B0,
	0x1900, 0x3900, 0x1950, 0x3950, 0x19A0, 0x39A0, 0x19F0, 0x39F0,
	0x1A40, 0x3A40, 0x1A90, 0x3A90, 0x1AE0, 0x3AE0, 0x1B30, 0x3B30,
	0x1B80, 0x3B80, 0x1BD0, 0x3BD0, 0x1C20, 0x3C20, 0x1C70, 0x3C70,
	0x1CC0, 0x3CC0, 0x1D10, 0x3D10, 0x1D60, 0x3D60, 0x1DB0, 0x3DB0,
	0x1E00, 0x3E00, 0x1E50, 0x3E50, 0x1EA0, 0x3EA0, 0x1EF0, 0x3EF0,
};
/* the line table read as the original reads it (an index past 199 reads other data) */
static uint16_t line_at(unsigned i)
{
	return i < 200 ? lines[i] : 0;
}

/* 03CCh, 03CDh: Gra_BlitReveal's random state (initialised data) */
static uint8_t reveal_a = 0x35, reveal_b = 0x9B;
/* 3A86h: Gra_AnimFlags' random seed. It is in _BSS, which the C startup clears, and
 * nothing else writes it: from zeros the generator stays at zero, so on DOS rnd() always
 * returns 0 and every flag swaps its rows at every call (kept). */
static uint8_t seed[4];
/* 3A96h InterleaveBuf: one tile or character being interleaved */
static uint8_t ibuf[0x80];

/* 32D0h: Gra_BlitReveal's masks, 64 words; the EGA code uses the low byte of word
 * (step + random 0..7) & 0FFh, so step + 7 must stay below 64 (the title passes 0..56;
 * larger indexes read D_3350 and on, not modelled: mask FFh). */
static const uint16_t reveal_masks[64] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0001, 0xC004, 0x0010, 0x0080, 0x0104, 0x0400, 0x0401,
	0x0100, 0xC3C0, 0x1000, 0xC410, 0x1004, 0x0411, 0x0110, 0x03C4,
	0x1080, 0xC481, 0x0110, 0xC104, 0x1010, 0xD101, 0x1084, 0x13C1,
	0x1010, 0x1304, 0xC484, 0x3CF0, 0x0F3C, 0x0F3F, 0x3C84, 0x3C3F,
	0x10F1, 0x13FC, 0xCFC1, 0x3CF1, 0xCFCF, 0x3CFF, 0xD13F, 0x3FF1,
	0xD3F1, 0xF3FC, 0x3C8F, 0xCF8F, 0x3F3F, 0xFFCF, 0x13FC, 0xCFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};
/* 3369h: the palette registers (and the border) given to int 10h AX=1002h */
static const uint8_t palette[17] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x00};
/* 337Ah: the map mask of the dot colours 0..3 (black, green, red, light cyan); colours
 * 4 and 5 read GR_SEGMENT (00h, A0h after Init), higher ones Monster1Seq: not modelled */
static const uint8_t dot_planes[6] = {0x00, 0x02, 0x04, 0x0B, 0x00, 0xA0};

/* pShapes, pCharset (DGROUP 707Ah, 7084h): the game's far pointers */
static uint8_t *shapes, *charset;

/* ---- helpers ---- */

/* 208Ah: the flags' random numbers. "adc" adds the carry: on the first call it comes from
 * the dispatcher (its last "shl bx,1" shifts out a 0), on the next ones from the "test"
 * that ends this routine, so it is always 0. */
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

/* "rcl byte ptr [bx],1" */
static void rcl(uint8_t *b, int *cf)
{
	int out = *b >> 7;
	*b = (uint8_t)(*b << 1 | *cf);
	*cf = out;
}

/* ---- the entries ---- */

void tgfx_set_shapes(uint8_t *s) { shapes = s; }
void tgfx_set_charset(uint8_t *c) { charset = c; }

/* 20B4h InterleaveCharset: each of the 128 characters (8 lines of 4 bytes, two pixels a
 * byte) becomes 4 planes of 8 bytes, plane 3 first, in place */
void tgfx_interleave_charset(uint8_t *c)
{
	uint8_t *si = c;
	int ch;
	for (ch = 0; ch < 0x80; ch++) {
		uint8_t *di = si, *bx = ibuf;
		int dh, dl;
		for (dh = 8; dh; dh--, bx++)
			for (dl = 4; dl; dl--) {
				uint8_t al = *si++;
				int k, cf;
				for (k = 0; k < 8; k++) {      /* bits 7..0: planes 3,2,1,0 of two pixels */
					cf = al >> 7;
					al = (uint8_t)(al << 1);
					rcl(&bx[(k & 3) * 8], &cf);
				}
			}
		memcpy(di, ibuf, 0x20);
		si = di + 0x20;
	}
}

/* 210Ch InterleaveShapes: each of the 256 tiles (16 lines of 8 bytes) becomes 4 planes of
 * 32 bytes (16 lines of 2), plane 3 first, in place */
void tgfx_interleave_shapes(uint8_t *s)
{
	uint8_t *si = s;
	int tile;
	for (tile = 0; tile < 0x100; tile++) {
		uint8_t *di = si, *bx = ibuf;
		int dh, dl;
		for (dh = 0x10; dh; dh--, bx += 2)
			for (dl = 8; dl; dl--) {
				uint8_t al = *si++;
				int k, cf;
				for (k = 0; k < 8; k++) {      /* a 16-bit shift: [bx+1] into [bx] */
					cf = al >> 7;
					al = (uint8_t)(al << 1);
					rcl(&bx[(k & 3) * 0x20 + 1], &cf);
					rcl(&bx[(k & 3) * 0x20], &cf);
				}
			}
		memcpy(di, ibuf, 0x80);
		si = di + 0x80;
	}
}

/* 1B3Dh GR_INIT_EGA. Before the mode set it frees the Tandy patch (int 21h AH=49h with
 * ES = patch_tandy's segment) and clears that segment word: left to the glue. Int 10h
 * AX=000Dh clears the planes; GR_SEGMENT (337Eh) becomes A000h (it was B800h: the
 * routines below always draw on the EGA planes, the title draws nothing before Init). */
void tgfx_init(uint8_t *s, uint8_t *c)
{
	int y;
	shapes = s;
	charset = c;
	memset(ega_vram, 0, sizeof ega_vram);
	memcpy(ega_palette_regs, palette, 16);      /* int 10h AX=1002h ES:DX = 3369h */
	for (y = 0; y < 200; y++)
		lines[y] = (uint16_t)(y * EGA_LINE);
	tgfx_interleave_charset(c);
	tgfx_interleave_shapes(s);
	ega_dirty = 1;
}

/* 124Dh GR_0_EGA: the view. h is a word counted down after each row (0 would be 65536),
 * w a byte counted down after each tile (0 is 256); the tile numbers are read as words
 * and masked to a byte. Each tile: 4 planes (map mask 8, 4, 2, 1) of 16 lines of 2 bytes. */
void tgfx_draw_view(int h, int w, const uint8_t *tiles, int line)
{
	unsigned bx = (unsigned)(line & 0xFFFF);
	uint16_t rows = (uint16_t)h;
	do {
		uint8_t n = (uint8_t)w;                 /* [bp-2] */
		int di = line_at(bx) + 1;
		do {
			const uint8_t *si = shapes + (uint16_t)(*tiles++ << 7);
			int ch, k;
			for (ch = 8; ch; ch >>= 1)
				for (k = 0; k < 16; k++, si += 2) {
					put(di + k * EGA_LINE, ch, si[0]);
					put(di + k * EGA_LINE + 1, ch, si[1]);
				}
			di += 2;
		} while (--n);
		bx += 16;
	} while (--rows);
	ega_dirty = 1;
}

/* 131Fh GR_C_EGA: the view drawn into a picture buffer: plane k of a tile goes 1F40h * k
 * further, plane 3 first (offsets within buf's segment; buf holds 32000 bytes) */
void tgfx_draw_view_over(int h, int w, const uint8_t *tiles, int line, uint8_t *buf)
{
	unsigned bx = (unsigned)(line & 0xFFFF);
	uint16_t rows = (uint16_t)h;
	do {
		uint8_t n = (uint8_t)w;
		uint16_t di = (uint16_t)(line_at(bx) + 1);
		do {
			const uint8_t *si = shapes + (uint16_t)(*tiles++ << 7);
			int ch, k;
			for (ch = 8; ch; ch >>= 1) {
				for (k = 0; k < 16; k++, si += 2) {
					buf[(uint16_t)(di + k * EGA_LINE)] = si[0];
					buf[(uint16_t)(di + k * EGA_LINE + 1)] = si[1];
				}
				di += 0x1F40;
			}
			di -= 0x7D00;
			di += 2;
		} while (--n);
		bx += 16;
	} while (--rows);
}

/* 14ABh GR_DOT_EGA: bit mask = the pixel, map mask = the colour's planes, then
 * "or es:[bx],0FFh" in function replace: the pixel's bits are set in those planes and
 * kept in the others (a dot ORs its colour onto what is there; colour 0 writes nothing) */
void tgfx_dot(int y, int x, int color)
{
	unsigned c = (unsigned)color & 0xFF;
	int ofs = line_at((unsigned)y & 0xFFFF) + (((unsigned)x & 0xFFFF) >> 3);
	rmw(ofs, c < 6 ? dot_planes[c] : 0, F_REPLACE, (uint8_t)(0x80 >> (x & 7)), 0xFF);
	ega_dirty = 1;
}

/* 155Bh GR_2_EGA: the menu area: lines 104-183, 19 words from byte column 1 */
void tgfx_clear_menu(void)
{
	int y, i;
	for (y = 0x68; y < 0xB8; y++)
		for (i = 0; i < 0x26; i++)
			put(lines[y] + 1 + i, 0x0F, 0);
	ega_dirty = 1;
}

/* 1A79h GR_5_EGA: the text area: lines 152-199, whole lines */
void tgfx_clear_text(void)
{
	int y, i;
	for (y = 0x98; y < 0xC8; y++)
		for (i = 0; i < 0x28; i++)
			put(lines[y] + i, 0x0F, 0);
	ega_dirty = 1;
}

/* 204Ah GR_CLRSCR_EGA: 0FA0h words from offset 0 in the four planes */
void tgfx_clear_screen(void)
{
	int i;
	for (i = 0; i < 8000; i++)
		put(i, 0x0F, 0);
	ega_dirty = 1;
}

/* 16D1h GR_3_EGA: plane by plane (map mask 8, 4, 2, 1; the picture's plane 3 first), h
 * lines (a byte: 0 is 256) of "rep movsb" of w bytes */
void tgfx_blit(int w, int h, int src_x, int src_y, const uint8_t *pic, int dst_y, int dst_x)
{
	uint16_t plane_ofs = 0;                         /* [bp-2] */
	int al;
	for (al = 8; al; al >>= 1, plane_ofs += 0x1F40) {
		unsigned dx = (unsigned)src_y & 0xFFFF, bx = (unsigned)dst_y & 0xFFFF;
		uint8_t ah = (uint8_t)h;
		do {
			uint16_t di = (uint16_t)(dst_x + line_at(bx++));
			uint16_t si = (uint16_t)(src_x + line_at(dx++) + plane_ofs);
			uint16_t cx = (uint16_t)w;
			for (; cx; cx--)
				put(di++, al, pic[si++]);
		} while (--ah);
	}
	ega_dirty = 1;
}

/* 173Fh GR_B_EGA: the logo appearing. From the last line up, each byte whose four planes
 * are not all 0 gets a random mask: the random state steps ("add dl,1Dh / adc dl,[03CDh]"),
 * its low 3 bits plus step pick the mask in reveal_masks, and when the state plus step
 * carries out of a byte the speaker bit is toggled; the four planes are then written
 * ANDed with the mask (so a revealed pixel can go dark again). Black bytes are skipped.
 * Port 61h is read at the start and written back at the end. h is a word (0 would be
 * 65536 lines), w a word (0 is 65536 bytes a line). The offsets wrap at the end of the
 * picture's segment: here 64 KB after pic, the same when pic is at offset 0 of its
 * segment, as _fmalloc's blocks are. */
void tgfx_blit_reveal(int w, int h, int src_x, int src_y, const uint8_t *pic, int dst_y,
	int step, int dst_x)
{
	uint16_t rows = (uint16_t)h;                   /* [bp+6] */
	uint8_t st = (uint8_t)step;
	int toggles = 0;
	do {
		uint16_t di = (uint16_t)(line_at((uint16_t)(dst_y + rows - 1)) + dst_x);
		uint16_t si = (uint16_t)(line_at((uint16_t)(src_y + rows - 1)) + src_x);
		uint16_t cx = (uint16_t)w;
		do {
			if (pic[si] | pic[(uint16_t)(si + 0x1F40)] | pic[(uint16_t)(si + 0x3E80)] |
				pic[(uint16_t)(si + 0x5DC0)]) {
				unsigned sum = reveal_a + 0x1Du;
				uint8_t dl = (uint8_t)sum, dh = dl, m;
				unsigned idx;
				dl = (uint8_t)(dl + reveal_b + (sum >> 8));
				reveal_a = dl;
				reveal_b = dh;
				dh = dl & 7;
				if (dl + st > 0xFF)
					toggles++;
				idx = (uint8_t)(st + dh);
				m = idx < 64 ? (uint8_t)reveal_masks[idx] : 0xFF;
				put(di, 8, pic[si] & m);
				put(di, 4, pic[(uint16_t)(si + 0x1F40)] & m);
				put(di, 2, pic[(uint16_t)(si + 0x3E80)] & m);
				put(di, 1, pic[(uint16_t)(si + 0x5DC0)] & m);
			}
			si++;
			di++;
		} while (--cx);
	} while (--rows);
	ega_speaker_noise(toggles);
	ega_dirty = 1;
}

/* 1F2Eh GR_PUTCHAR_EGA: 8 bytes in each plane (map mask 8, 4, 2, 1) */
void tgfx_putchar(int y, int x, int c)
{
	const uint8_t *si = charset + ((c & 0xFF) << 5);
	int bx = line_at((uint16_t)(y << 4) >> 1) + x, cl, k;
	for (cl = 8; cl; cl >>= 1)
		for (k = 0; k < 8; k++)
			put(bx + k * EGA_LINE, cl, *si++);
	ega_dirty = 1;
}

/* 1BFAh GR_ANIMFLOW_EGA: the last line of each plane saved, the tile's 128 bytes moved
 * up by a word ("std; rep movsw"), the saved lines put back as each plane's first line */
void tgfx_anim_flow(int tile)
{
	uint8_t *bx = shapes + (uint16_t)(tile << 7);
	uint8_t a[2], b[2], c[2], d[2];
	memcpy(a, bx + 0x1E, 2);
	memcpy(b, bx + 0x3E, 2);
	memcpy(c, bx + 0x5E, 2);
	memcpy(d, bx + 0x7E, 2);
	memmove(bx + 2, bx, 0x7E);
	memcpy(bx + 0x60, d, 2);
	memcpy(bx + 0x40, c, 2);
	memcpy(bx + 0x20, b, 2);
	memcpy(bx, a, 2);
}

/* 1CF4h GR_ANIMFLAG_EGA: five flags (the town, the castle, the ships, Lord British's
 * castle); for each, when rnd() is below 80h, the words at +0 and +2 of each plane are
 * swapped (the flag's top line) */
void tgfx_anim_flags(void)
{
	static const uint16_t at[5] = {0x506, 0x582, 0x804, 0x904, 0x702};
	int i, k;
	for (i = 0; i < 5; i++) {
		if (rnd() & 0x80)
			continue;
		for (k = 0; k < 4; k++) {
			uint8_t *p = shapes + at[i] + k * 0x20, t[2];
			memcpy(t, p, 2);
			memcpy(p, p + 2, 2);
			memcpy(p + 2, t, 2);
		}
	}
}

/* ---- INFLATE.ASM: the LZW decoder (12-bit codes, 100h and up for strings) ---- */

static uint16_t in_left;                /* 3B20h: bytes left in the file */
static const uint8_t *in_ptr;           /* 3B22h */
static uint8_t in_half;                 /* 3B24h: 1 when half a byte is waiting */
static uint8_t in_nibble;               /* 3B25h */
static uint16_t prefix[0xF00];          /* 3B26h: code 100h + i: its prefix | 2000h */
static uint8_t suffix[0xF00];           /* 5926h: and its last byte */
/* 6826h: the bytes of a string, last first. The original has 100h bytes there; a string
 * longer than that would overwrite the variables that follow (speed_info, ...). The
 * title's files never come near it (tgfx_diff.py checks); this one is larger. */
static uint8_t stack_[0x1000];
static unsigned out_count;              /* bytes given to the callback (for the tests) */

/* 2290h: the next 12-bit code, or FFFFh at the end of the file */
static uint16_t read12(void)
{
	const uint8_t *bx = in_ptr;
	uint16_t ax;
	if (in_left-- == 0) {                   /* "sub [3B20h],1 / jb" */
		ax = 0xFFFF;
	} else {
		ax = *bx++;
		if (!in_half) {
			if (in_left-- == 0) {
				ax = 0xFFFF;
			} else {
				uint8_t dl = *bx++;
				in_nibble = dl;
				ax = (uint16_t)(ax << 4 | dl >> 4);
			}
		} else {
			ax = (uint16_t)((in_nibble & 0x0F) << 8 | ax);
		}
	}
	in_half ^= 1;
	in_ptr = bx;
	return ax;
}

/* 21E5h: the slot (100h..FFFh) for the string old + fc: a free one, or the one that
 * already holds it */
static uint16_t slot_for(uint16_t old, uint16_t fc)
{
	uint16_t ax = (uint16_t)(((fc << 4) ^ old) & 0x0FFF), cx;
	if (ax >> 8) {
		cx = prefix[ax - 0x100];
		if (!cx)
			return ax;
		if ((cx & ~0x2000) == old && suffix[ax - 0x100] == (uint8_t)fc)
			return ax;
	}
	{
		/* 221Ch: bits 4..15 of the square of 800h | (fc * 2 + old), shifted twice to the
		 * left through the carry (which MUL sets: the high word is never 0) */
		uint16_t a = (uint16_t)((uint16_t)(((fc & 0xFF) << 1) + old) | 0x800);
		uint32_t p = (uint32_t)a * a;
		uint16_t lo = (uint16_t)p, hi = (uint16_t)(p >> 16);
		int cf = hi != 0, t;
		t = lo >> 15; lo = (uint16_t)(lo << 1 | cf); cf = t;
		t = hi >> 15; hi = (uint16_t)(hi << 1 | cf); cf = t;
		t = lo >> 15; lo = (uint16_t)(lo << 1 | cf); cf = t;
		hi = (uint16_t)(hi << 1 | cf);
		ax = (uint16_t)((hi & 0x0F) << 8 | lo >> 8);
	}
	if (ax >> 8) {
		cx = prefix[ax - 0x100];
		if (!cx)
			return ax;
		if ((cx & ~0x2000) == old && suffix[ax - 0x100] == (uint8_t)fc)
			return ax;
	}
	for (;;) {                              /* 2261h: steps of 1FDh */
		do
			ax = (uint16_t)((ax + 0x1FD) & 0x0FFF);
		while (!(ax >> 8));
		cx = prefix[ax - 0x100];
		if (!cx)
			return ax;
		if ((cx & ~0x2000) == old && suffix[ax - 0x100] == (uint8_t)fc)
			return ax;
	}
}

static void emit(void (*cb)(uint8_t), uint8_t b)
{
	out_count++;
	cb(b);
}

/* 22D5h: the codes decoded, each byte to cb. old [bp-6] is the previous code, fc [bp-8]
 * the first byte of the last string; after 0CCDh new strings the table starts over.
 * An empty file still gives one byte (FFh: the end mark taken as the first code). */
static void lzw(void (*cb)(uint8_t))
{
	uint16_t count = 0, code, old, fc;
	in_half = 0;
	memset(prefix, 0, sizeof prefix);
	old = fc = read12();
	emit(cb, (uint8_t)fc);
	for (;;) {
		code = read12();
		if (code == 0xFFFF)
			return;
		if (code >> 8) {
			uint8_t *sp = stack_;
			uint16_t ax;
			if (!prefix[code - 0x100]) {   /* not defined yet: old's string + its first byte */
				*sp++ = (uint8_t)fc;
				ax = old;
			} else {
				ax = code;
			}
			while (ax >> 8) {
				/* only a corrupt file gets here (a chain longer than the 0CCDh strings
				 * of a table, or a loop): the original runs over DGROUP or hangs */
				if (sp == stack_ + sizeof stack_)
					return;
				*sp++ = suffix[ax - 0x100];
				ax = prefix[ax - 0x100] & ~0x2000;
			}
			fc = ax;
			emit(cb, (uint8_t)ax);
			do
				emit(cb, *--sp);
			while (sp != stack_);
		} else {
			emit(cb, (uint8_t)code);
			fc = (uint16_t)((fc & 0xFF00) | (code & 0xFF));    /* "mov [bp-8],al" */
		}
		{
			uint16_t slot = slot_for(old, fc);
			suffix[slot - 0x100] = (uint8_t)fc;
			prefix[slot - 0x100] = (uint16_t)(old | 0x2000);
		}
		if (++count > 0xCCC) {
			count = 0;
			memset(prefix, 0, sizeof prefix);
			code = read12();
			fc = code;
			if (code == 0xFFFF)
				return;
			emit(cb, (uint8_t)code);
		}
		old = code;
	}
}

/* 23C3h _Inflate: the whole file read (open, lseek to the end for the size, malloc on the
 * near heap, read, close), decoded, freed. -1 when the file cannot be opened (the original
 * then frees its uninitialised buffer pointer [bp-4]) or the near heap cannot hold it (not
 * modelled). Only the low word of the size is used. */
int tgfx_inflate(const char *name, void (*cb)(uint8_t))
{
	uint8_t *data;
	int size;
	if (ega_read_file(name, &data, &size) != 0)
		return -1;
	in_left = (uint16_t)size;
	in_ptr = data;
	out_count = 0;
	lzw(cb);
	ega_free_file(data);
	return 0;
}

/* 3B16h: the far pointer the callbacks write to (its offset only steps: it wraps within
 * the segment, here within 64 KB of buf), 3B1Ch: the pixel mask of CB_ega */
static uint8_t *out_buf;
static uint16_t out_ofs;
static uint8_t out_mask;

/* 21ADh CB_dummy: the byte stored, the pointer advanced */
static void cb_bytes(uint8_t al)
{
	out_buf[out_ofs++] = al;
}

/* 217Eh LoadCompressed */
void tgfx_load_compressed(const char *name, uint8_t *buf)
{
	out_buf = buf;
	out_ofs = 0;
	if (tgfx_inflate(name, cb_bytes) == -1)
		ega_fatal_exit(0x36);
}

/* 199Eh CB_ega: a byte is two pixels; bit 3..0 of each colour ORed into planes 3..0
 * (1F40h apart, plane 3 first) under the pixel's mask; the mask is rotated twice and the
 * pointer advances when it wraps (every 4 bytes). Nothing stops it at 32000 bytes: the
 * pictures decode to exactly 32000 (tgfx_diff.py checks); writes past 32 KB are dropped
 * here (the original would write into the next far block). */
static void cb_picture(uint8_t al)
{
	uint8_t ah = out_mask;
	int half, cf;
	for (half = 0; half < 2; half++) {
		static const uint16_t plane[4] = {0, 0x1F40, 0x3E80, 0x5DC0};
		int k;
		for (k = 0; k < 4; k++, al = (uint8_t)(al << 1))
			if (al & 0x80) {
				uint16_t o = (uint16_t)(out_ofs + plane[k]);
				if (o < 0x8000)
					out_buf[o] |= ah;
			}
		ah >>= 1;
	}
	out_mask = (uint8_t)(out_mask >> 1 | out_mask << 7);
	cf = out_mask & 1;
	out_mask = (uint8_t)(out_mask >> 1 | out_mask << 7);
	out_ofs = (uint16_t)(out_ofs + cf);         /* "adc word ptr [3B16h],0" */
}

/* 195Fh GR_INFLATE_EGA: the buffer cleared (3E80h words), the picture decoded into it */
void tgfx_load_picture(const char *name, uint8_t *buf)
{
	out_mask = 0x80;
	out_buf = buf;
	out_ofs = 0;
	memset(buf, 0, 0x7D00);
	if (tgfx_inflate(name, cb_picture) == -1)
		ega_fatal_exit(0x36);
}
