/* replaces U4_GRAPH.ASM: AVATAR.EXE's side of the graphics driver.
 *
 * In DOS, LoadGraphicsDriver reads CGA.DRV, EGA.DRV or TANDY.DRV into memory and each
 * Gra_* entry far-calls the driver through the table at its start, some after pushing an
 * argument of their own (SoundFlag, the codex picture's name, pKarmas, txt_X/txt_Y). The
 * port has EGA.DRV in C (port/gfx/ega.c): the entries call it directly. Also here:
 * DrawGameFrame, Gra_Line (straight lines itself, slanted ones through the game's
 * DrawLine), and the interleaving of SHAPES.EGA and CHARSET.EGA into the driver's plane
 * order.
 *
 * Each entry adds the 8088 time the driver took (u4rt.h): the EGA writes made through
 * ega_hw.h count themselves (ega_cost); the routines that copy tiles straight to the
 * planes are estimated here per byte.
 */
#include "u4rt.h"
#include "dosfs.h"
#include "../gfx/ega.h"

#include <stdlib.h>
#include <string.h>

void port_exit(int code);
int16_t DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);   /* the game's (U4_3D.C) */

extern int16_t GraphicsType;
extern void *pShapes, *pCharset;
extern int16_t txt_X, txt_Y;
extern char SoundFlag;
extern uint16_t *pKarmas[];

/* the 8088 cycles of a tile drawn by the driver: 4 planes of 16 lines of 2 bytes */
#define COST_TILE (4 * (16 * (2 * 12 + 20)))

/* C_20C1: reads the driver; exits with code 49h if it cannot. The port's driver is built
 * in, but the file must be there as in DOS. */
int LoadGraphicsDriver(char *name)
{
	if (!dosfs_exists(name))
		port_exit(0x49);
	vc_cycles(4000);
	vc_add_ns(5000u * 10000u);	/* ~5 KB from the disk */
	return 0;
}

int Gra_ClearScreen(void);
int Gra_LoadPicture(char *name, int ofs, int buf);

/* C_213B: the screen's frame: START.PIC (CGA) or START.EGA */
int DrawGameFrame(void)
{
	Gra_ClearScreen();
	Gra_LoadPicture(GraphicsType == 1 ? "start.pic" : "start.ega", 0, 0);
	return 0;
}

/* C_215A: a line from (x0, y0) to (x1, y1) in colour c: straight ones dot by dot (Gra_DotOr),
 * slanted ones through DrawLine (which uses colour 3) */
int Gra_Line(int x0, int y0, int x1, int y1, int c)
{
	int step, n;
	if (x0 != x1 && y0 != y1) {
		DrawLine((int16_t)x0, (int16_t)y0, (int16_t)x1, (int16_t)y1);
		return 0;
	}
	if (x0 == x1) {
		int d = (int16_t)(y1 - y0);
		step = d > 0 ? 1 : d < 0 ? -1 : 0;
		n = (d < 0 ? -d : d) + 1;
		do {
			ega_dot_or(y0, x0, c);
			y0 += step;
		} while (--n & 0xFFFF);
	} else {
		int d = (int16_t)(x1 - x0);
		step = d > 0 ? 1 : d < 0 ? -1 : 0;
		n = (d < 0 ? -d : d) + 1;
		do {
			ega_dot_or(y0, x0, c);
			x0 += step;
		} while (--n & 0xFFFF);
	}
	return 0;
}

/* ---- the driver's entries (the table's order) ---- */
int Gra_ActiveBox(int row, int col) { ega_active_box(row, col); return 0; }

int Gra_ZoomTileRow(int tile, int trow, int srow, int scol, int zoom)
{
	ega_zoom_tile_row(tile, trow, srow, scol, zoom);
	vc_cycles(4 * 16 * zoom * 30);
	return 0;
}

int Gra_FillTiles(int h, int w, int tile, int row, int col)
{
	ega_fill_tiles(h, w, tile, row, col);
	vc_cycles((uint32_t)((h & 0xFF) * (w & 0xFF)) * COST_TILE);
	return 0;
}

int Gra_ShiftDown(void) { ega_shift_down((uint8_t)SoundFlag); return 0; }
int Gra_ShiftUp(void) { ega_shift_up((uint8_t)SoundFlag); return 0; }

int Gra_DrawTiles(int h, int w, uint8_t *tiles, int dst)
{
	(void)dst;
	ega_draw_tiles(h, w, tiles);
	vc_cycles((uint32_t)(h & 0xFF) * 11u * COST_TILE);
	return 0;
}

int Gra_Dot(int y, int x, int c) { ega_dot(y, x, c); return 0; }

/* C_2230: a new line in the text window (bottom right) */
int Gra_CR(void)
{
	txt_Y = 23;
	txt_X = 24;
	ega_cr();
	return 0;
}

int Gra_InvertView(void) { ega_invert_view(); return 0; }
int Gra_ClearView(void) { ega_clear_view(); return 0; }
int Gra_InvertStatusLine(int line) { ega_invert_status_line(line); return 0; }
int Gra_InvertChar(int x, int y) { ega_invert_char(x, y); return 0; }
int Gra_ClearStatus(void) { ega_clear_status(); return 0; }
int Gra_DotOr(int y, int x, int c) { ega_dot_or(y, x, c); return 0; }
int Gra_DotXor(int y, int x, int c) { ega_dot_xor(y, x, c); return 0; }

/* (name, offset, buffer): the game passes 0, 0: the screen */
int Gra_LoadPicture(char *name, int ofs, int buf)
{
	(void)buf;
	ega_load_picture(name, ofs, 0);
	vc_cycles(32000u * 40u);
	return 0;
}

/* C_2280: the codex's picture appears (RUNE_5.PIC on a CGA) */
int Gra_RevealCodex(void)
{
	ega_reveal_codex(GraphicsType == 1 ? "rune_5.pic" : "rune_5.ega");
	return 0;
}

int Gra_DrawQuad(int x0, int x1, int y0, int y1, int y0p, int y1p, int c0, int c1)
{
	ega_draw_quad(x0, x1, y0, y1, y0p, y1p, c0, c1);
	return 0;
}

int Gra_VirtueGlyph(void) { ega_virtue_glyph((const uint16_t *const *)pKarmas); return 0; }
int Gra_AnimCampfire(void) { ega_anim_campfire(); vc_cycles(1500); return 0; }

/* C_22AC: (&pShapes, &pCharset, the exit function) */
int Gra_Init(void **shapes, void **charset, void *exit_fn)
{
	(void)exit_fn;
	ega_init((uint8_t *)*shapes, (uint8_t *)*charset);
	vc_cycles(5000);
	return 0;
}

int Gra_ClearScreen(void) { ega_clear_screen(); return 0; }
int Gra_AnimFlow(int tile) { ega_anim_flow(tile); vc_cycles(3000); return 0; }
int Gra_AnimFlags(void) { ega_anim_flags(); vc_cycles(1500); return 0; }

/* C_22C0: a character at (txt_X, txt_Y) */
int Gra_PutChar(int c)
{
	ega_putchar(txt_Y, txt_X, c & 0xFF);
	return 0;
}

/* ---- C_22CF, C_2341: the interleaving ---- */
static uint8_t interleave_buf[0x80];   /* D_8C4A */

/* each tile of SHAPES.EGA (128 bytes: 16 lines of 16 pixels, two a byte) becomes its four
 * planes, 32 bytes each (16 lines of 2 bytes), plane 3 first */
int InterleaveShapes(void)
{
	uint8_t *si = pShapes;
	int t, line, k, b;
	for (t = 0; t < 0x100; t++, si += 0x80) {
		uint8_t *src = si;
		memset(interleave_buf, 0, sizeof interleave_buf);
		for (line = 0; line < 16; line++) {
			uint8_t *bx = interleave_buf + line * 2;
			for (k = 0; k < 8; k++) {
				uint8_t al = *src++;
				for (b = 0; b < 2; b++) {
					/* "shl al,1 / rcl [bx+1],1 / rcl [bx],1" for planes 3, 2, 1, 0 */
					int p;
					for (p = 0; p < 4; p++) {
						uint8_t *w = bx + p * 0x20;
						unsigned v = (unsigned)w[0] << 8 | w[1];
						v = v << 1 | (al >> 7);
						al = (uint8_t)(al << 1);
						w[0] = (uint8_t)(v >> 8);
						w[1] = (uint8_t)v;
					}
				}
			}
		}
		memcpy(si, interleave_buf, 0x80);
	}
	vc_cycles(256u * 128u * 8u * 30u);
	return 0;
}

/* the first 128 characters of CHARSET.EGA (32 bytes: 8 lines of 8 pixels) become four
 * planes of 8 bytes */
int InterleaveCharset(void)
{
	uint8_t *si = pCharset;
	int c, line, k, b;
	for (c = 0; c < 0x80; c++, si += 0x20) {
		uint8_t *src = si;
		memset(interleave_buf, 0, 0x20);
		for (line = 0; line < 8; line++) {
			uint8_t *bx = interleave_buf + line;
			for (k = 0; k < 4; k++) {
				uint8_t al = *src++;
				for (b = 0; b < 2; b++) {
					int p;
					for (p = 0; p < 4; p++) {
						bx[p * 8] = (uint8_t)(bx[p * 8] << 1 | (al >> 7));
						al = (uint8_t)(al << 1);
					}
				}
			}
		}
		memcpy(si, interleave_buf, 0x20);
	}
	vc_cycles(128u * 32u * 8u * 25u);
	return 0;
}

/* ---- the driver's hooks (ega.h) ---- */
void ega_fatal_exit(int code)
{
	port_exit(code);
}

int ega_read_file(const char *name, uint8_t **data, int *size)
{
	int16_t h;
	uint32_t len;
	uint16_t got;
	uint8_t *buf;
	if (dosfs_open(name, 0, &h) != 0)
		return -1;
	dosfs_seek(h, 2, 0, &len);
	dosfs_seek(h, 0, 0, 0);
	buf = malloc(len ? len : 1);
	if (!buf || len > 0xFFFF || dosfs_read(h, buf, (uint16_t)len, &got) != 0 || got != len) {
		free(buf);
		dosfs_close(h);
		return -1;
	}
	dosfs_close(h);
	vc_add_ns((uint64_t)len * 10000u);
	*data = buf;
	*size = (int)len;
	return 0;
}

void ega_free_file(uint8_t *data)
{
	free(data);
}

void ega_busy_wait(long iterations)
{
	vc_cycles((uint32_t)iterations * 17u);	/* "loop $" */
}
