/* replaces the entry stubs of TITLE.EXE's GRAPH.ASM (SRC-TITLE/), with LoadCompressed and
 * the interleaving: each Gra_* entry calls the EGA routine of port/gfx/tgfx.c (the port's
 * PC has an EGA). Compiled with the title's names (T_*, include/title/trename.h).
 *
 * Each entry adds the time the routine took on the PC (u4rt.h): the writes through
 * ega_hw.h count themselves; tile and picture copies are estimated here per byte.
 */
#include "u4rt.h"
#include "../gfx/tgfx.h"
#include "../gfx/ega.h"

#include <stdlib.h>

extern int16_t GraphicsType;
extern void *pShapes, *pCharset, *patch_tandy;
extern int16_t txt_X, txt_Y;
extern uint64_t ega_noise_span;    /* sound.c: the cycles a speaker noise is spread over */

#define COST_TILE (4 * (16 * (2 * 12 + 20)))

int Gra_DrawView(int h, int w, uint8_t *tiles, int line, int x, int seg, int col)
{
	(void)x; (void)seg; (void)col;
	tgfx_draw_view(h, w, tiles, line);
	vc_cycles((uint32_t)h * (uint32_t)((w & 0xFF) ? (w & 0xFF) : 256) * COST_TILE);
	return 0;
}

int Gra_Dot(int y, int x, int c) { tgfx_dot(y, x, c); return 0; }
int Gra_ClearMenu(void) { tgfx_clear_menu(); return 0; }

int Gra_Blit(int w, int h, int src_x, int src_y, uint8_t *pic, int dst_y, int rnd, int dst_x)
{
	(void)rnd;
	tgfx_blit(w, h, src_x, src_y, pic, dst_y, dst_x);
	vc_cycles((uint32_t)(w & 0xFFFF) * (uint32_t)((h & 0xFF) ? (h & 0xFF) : 256) * 4u * 12u);
	return 0;
}

int Gra_LoadPicture(char *name, uint8_t *buf)
{
	tgfx_load_picture(name, buf);
	vc_cycles(32000u * 40u);
	return 0;
}

int Gra_ClearText(void) { tgfx_clear_text(); return 0; }

/* GR_INIT_EGA frees the Tandy patch block (DOS 49h) and clears the pointer */
int Gra_Init(void)
{
	free(patch_tandy);
	patch_tandy = 0;
	tgfx_init(pShapes, pCharset);
	vc_cycles(256u * 128u * 8u * 30u);
	return 0;
}

int Gra_AnimFlow(int tile) { tgfx_anim_flow(tile); vc_cycles(3000); return 0; }
int Gra_AnimFlags(void) { tgfx_anim_flags(); vc_cycles(1500); return 0; }
int Gra_PutChar(int c) { tgfx_putchar(txt_Y, txt_X, c & 0xFF); return 0; }
int Gra_ClearScreen(void) { tgfx_clear_screen(); return 0; }

int Gra_DrawViewOver(int h, int w, uint8_t *tiles, int line, uint8_t *buf, int col)
{
	(void)col;
	tgfx_draw_view_over(h, w, tiles, line, buf);
	vc_cycles((uint32_t)h * (uint32_t)((w & 0xFF) ? (w & 0xFF) : 256) * COST_TILE);
	return 0;
}

/* the logo appears: the speaker clicks once a byte drawn */
int Gra_BlitReveal(int w, int h, int src_x, int src_y, uint8_t *pic, int dst_y, int step, int dst_x)
{
	uint32_t bytes = (uint32_t)(w & 0xFFFF) * (uint32_t)((h & 0xFF) ? (h & 0xFF) : 256);
	ega_noise_span = (uint64_t)bytes * 100u;
	tgfx_blit_reveal(w, h, src_x, src_y, pic, dst_y, step, dst_x);
	ega_noise_span = 0;
	vc_cycles(bytes * 100u);
	return 0;
}

int InterleaveCharset(void)
{
	tgfx_interleave_charset(pCharset);
	return 0;
}

/* RedrawTitle reallocates pShapes: the routines take the new one */
int InterleaveShapes(void)
{
	tgfx_set_shapes(pShapes);
	tgfx_interleave_shapes(pShapes);
	vc_cycles(256u * 128u * 8u * 30u);
	return 0;
}

int LoadCompressed(char *name, uint8_t *buf)
{
	tgfx_load_compressed(name, buf);
	vc_cycles(32768u * 40u);
	return 0;
}
