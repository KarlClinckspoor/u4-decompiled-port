/* TITLE.EXE's own graphics in C (tgfx.c): the EGA variants of SRC-TITLE/GRAPH.ASM, the
 * title's InterleaveCharset / InterleaveShapes, LoadCompressed and INFLATE.ASM's _Inflate.
 *
 * TITLE.EXE does not use EGA.DRV: GRAPH.ASM has its own routines for CGA, EGA and Tandy,
 * picked by GraphicsType (1 CGA, 2 EGA, 3 Tandy) through DriverTable (DGROUP:03CEh):
 * entry n of the 13 Gra_* stubs (1186h..11C2h) jumps to DriverTable[n * 3 + GraphicsType - 1].
 * Only the EGA column is here. Each function does to the planes (ega_hw.h) what the EGA
 * routine does, byte for byte: the same reads, the same map masks / bit masks, the same
 * quirks; tests/tgfx_diff.py runs TITLE_0.EXE's code in unicorn against it.
 *
 * Addresses are TITLE_0.EXE's (link map TITLE_0.MAP): code offsets in _TEXT, data offsets
 * in DGROUP. All entries are near calls of the small model; "cdecl" ones (the names with
 * a leading underscore in GRAPH.ASM) have their first argument at [bp+4] and the caller
 * pops; "pascal" ones (MSC /Gc) push their arguments left to right and pop them with
 * RET n, so with one argument it is also at [bp+4]. A far pointer argument is two words,
 * offset at the lower address.
 *
 * ---- the entries, how SRC-TITLE calls them, and the glue to write ----
 *
 *  Gra_DrawView    1186h cdecl -> GR_0_EGA 124Dh
 *      Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);            (TITLE_0.C ShowView)
 *      [bp+4] h (word: rows of tiles), [bp+6] w (byte: tiles a row, 0 = 256),
 *      [bp+8] tiles (near DGROUP pointer, one byte a tile), [bp+0Ah] line (first pixel
 *      line), [bp+0Ch] x, [bp+0Eh] segment, [bp+10h] column: the last three are only
 *      used by the CGA / Tandy variants (EGA always draws on the screen at byte column 1).
 *      glue: tgfx_draw_view(h, w, tiles, line)
 *  Gra_Dot         118Bh cdecl -> GR_DOT_EGA 14ABh
 *      Gra_Dot(191 - SignatureDots[dot+1], SignatureDots[dot] + 20, 3);  Gra_Dot(31, step, 2);
 *      [bp+4] y, [bp+6] x, [bp+8] colour (byte, 0..3)
 *      glue: tgfx_dot(y, x, color)
 *  Gra_ClearMenu   1190h pascal, no argument -> GR_2_EGA 155Bh
 *      Gra_ClearMenu();                                           (ShowMenu)
 *      glue: tgfx_clear_menu()
 *  Gra_Blit        1195h cdecl -> GR_3_EGA 16D1h
 *      Gra_Blit(4, 4, 19, 17, pTitle, 17, -1, 19);  Gra_Blit(40, 152, 0, 0, pPicture, 0, -1, 0);
 *      [bp+4] w (bytes = 8 pixels), [bp+6] h (byte: lines, 0 = 256), [bp+8] src_x (bytes),
 *      [bp+0Ah] src_y (line), [bp+0Ch] far picture (4 planes of 8000 bytes, plane 3
 *      first, as Gra_LoadPicture leaves them), [bp+10h] dst_y, [bp+12h] rand (only the
 *      CGA / Tandy variants read it), [bp+14h] dst_x (bytes)
 *      glue: tgfx_blit(w, h, src_x, src_y, pic, dst_y, dst_x)   (pic: the far pointer)
 *  Gra_LoadPicture 119Ah cdecl -> GR_INFLATE_EGA 195Fh
 *      Gra_LoadPicture("TITLE.EGA", pTitle);  Gra_LoadPicture(CardPictures[virtue/2], pic);
 *      [bp+4] name (near), [bp+6] far buffer (32 KB from _fmalloc)
 *      Files (EGA): TITLE, ANIMATE, TREE, PORTAL, OUTSIDE, INSIDE, WAGON, GYPSY, ABACUS,
 *      HONCOM, VALJUS, SACHONOR, SPIRHUM (.EGA, LZW compressed, see tgfx_inflate).
 *      glue: tgfx_load_picture(name, buf)
 *  Gra_ClearText   119Fh pascal, no argument -> GR_5_EGA 1A79h
 *      glue: tgfx_clear_text()
 *  Gra_Init        11A4h pascal, no argument -> GR_INIT_EGA 1B3Dh
 *      Gra_Init();  (main, after loading CHARSET.EGA and LoadCompressed("SHAPES.EGZ"))
 *      It reads pShapes and pCharset (DGROUP 707Ah, 7084h) and frees the Tandy patch:
 *      int 21h AH=49h with ES = patch_tandy's segment (DGROUP 6935h), then sets that word
 *      to 0. tgfx_init does not model DOS memory: the glue sets patch_tandy to 0 itself.
 *      glue: patch_tandy = 0; tgfx_init(pShapes, pCharset)
 *  Gra_AnimFlow    11A9h pascal (tile) -> GR_ANIMFLOW_EGA 1BFAh
 *      Gra_AnimFlow(0x4c);                                        (AnimateViewTiles)
 *      glue: tgfx_anim_flow(tile)
 *  Gra_AnimFlags   11AEh pascal, no argument -> GR_ANIMFLAG_EGA 1CF4h
 *      glue: tgfx_anim_flags()
 *  Gra_PutChar     11B3h pascal (c) -> GR_PUTCHAR_EGA 1F2Eh
 *      Gra_PutChar(c);  (u4_putc, UTIL2.ASM) at the text cursor: it reads the game's
 *      txt_X (DGROUP 7088h, byte column) and txt_Y (708Ah, text line of 8 pixels)
 *      glue: tgfx_putchar(txt_Y, txt_X, c)
 *  Gra_ClearScreen 11B8h pascal, no argument -> GR_CLRSCR_EGA 204Ah
 *      glue: tgfx_clear_screen()
 *  Gra_DrawViewOver 11BDh cdecl -> GR_C_EGA 131Fh
 *      Gra_DrawViewOver(5, 19, ViewBackground, 104, pTitle, 2);   (TitleSequence)
 *      [bp+4] h, [bp+6] w (byte), [bp+8] tiles (near), [bp+0Ah] line, [bp+0Ch] far
 *      buffer (a picture: 4 planes of 8000 bytes), [bp+10h] column (not used by EGA).
 *      Draws the tiles into the picture, not on the screen.
 *      glue: tgfx_draw_view_over(h, w, tiles, line, buf)
 *  Gra_BlitReveal  11C2h cdecl -> GR_B_EGA 173Fh
 *      Gra_BlitReveal(30, 45, 5, 34, pTitle, 34, step, 5);  step 0..56  (the "Ultima IV" logo)
 *      the arguments of Gra_Blit, with [bp+12h] step: a random part of each non-black byte
 *      is drawn, more as step grows, and the speaker clicks (see tgfx_blit_reveal).
 *      glue: tgfx_blit_reveal(w, h, src_x, src_y, pic, dst_y, step, dst_x)
 *  InterleaveCharset 20B4h near (DS = DGROUP), no argument; called by Gra_Init only.
 *      glue (if ever needed): tgfx_interleave_charset(pCharset)
 *  InterleaveShapes  210Ch pascal, no argument: reads pShapes.
 *      InterleaveShapes();   (RedrawTitle, after LoadCompressed("SHAPES.EGZ", pShapes))
 *      glue: tgfx_set_shapes(pShapes); tgfx_interleave_shapes(pShapes)
 *  LoadCompressed  217Eh pascal (char *name, void far *buf): [bp+8] name, [bp+4] buf
 *      LoadCompressed("SHAPES.EGZ", pShapes);   (main, RedrawTitle)
 *      glue: tgfx_load_compressed(name, buf)
 *
 * The game's variables these routines read:
 *   - GraphicsType: the glue calls tgfx_* only for 2 (EGA).
 *   - pShapes, pCharset: given to tgfx_init, which keeps them (as the original keeps
 *     reading the game's variables). RedrawTitle allocates a new pShapes after NewGame
 *     freed the old one: the glue passes it with tgfx_set_shapes (simplest: in its
 *     InterleaveShapes, as above). Shapes: 256 tiles of 128 bytes; charset: 128
 *     characters of 32 bytes (8 KB if characters above 7Fh are drawn).
 *   - txt_X, txt_Y: arguments of tgfx_putchar.
 *   - patch_tandy: see Gra_Init.
 * GRAPH.ASM's own data are static in tgfx.c: Lines (023Ch), D_03CC/D_03CD (the reveal's
 * random state), D_3A86 (the flags' random seed), the tables of CONST.
 *
 * Hooks (declared in ega.h, shared with ega.c):
 *   - ega_read_file / ega_free_file: _Inflate's open/lseek/malloc/read/close/free of the
 *     whole compressed file (Gra_LoadPicture, LoadCompressed).
 *   - ega_fatal_exit(0x36): exit(0x36) when _Inflate fails (the file is missing; on DOS
 *     also when the near heap cannot hold it).
 *   - ega_speaker_noise(n): Gra_BlitReveal toggled the speaker bit (port 61h bit 1,
 *     "in al,61h; xor al,2; out 61h,al") n times during the call, then restored the
 *     port as it found it; called once at the end of each call, n may be 0. There is
 *     at most one toggle per non-black byte of the picture (just before its 4 plane
 *     writes), so the n clicks are spread over the call's w * h bytes (30 x 45 for
 *     the logo, one call per step; ega_cost gives the call's length).
 * No delay loop and no other sound in the EGA paths.
 */
#ifndef PORT_TGFX_H
#define PORT_TGFX_H

#include <stdint.h>

/* Gra_Init (GR_INIT_EGA 1B3Dh): mode 0Dh (planes cleared), the palette, the line table,
 * then InterleaveCharset and InterleaveShapes on the two buffers, which it keeps. */
void tgfx_init(uint8_t *shapes, uint8_t *charset);
/* the game's pShapes / pCharset changed (RedrawTitle reallocates pShapes) */
void tgfx_set_shapes(uint8_t *shapes);
void tgfx_set_charset(uint8_t *charset);

/* Gra_DrawView (GR_0_EGA 124Dh): h rows of w tiles from tiles[] (tile numbers, one
 * byte each, read in sequence over all the rows) from pixel line `line`, byte column 1 */
void tgfx_draw_view(int h, int w, const uint8_t *tiles, int line);
/* Gra_DrawViewOver (GR_C_EGA 131Fh): the same tiles drawn into the picture buf (4 planes
 * of 8000 bytes, plane 3 first) instead of the screen */
void tgfx_draw_view_over(int h, int w, const uint8_t *tiles, int line, uint8_t *buf);
/* Gra_Dot (GR_DOT_EGA 14ABh): pixel (x, y) ORed with colour 0..3 */
void tgfx_dot(int y, int x, int color);
/* Gra_ClearMenu (GR_2_EGA 155Bh): lines 104-183, byte columns 1-38 cleared */
void tgfx_clear_menu(void);
/* Gra_ClearText (GR_5_EGA 1A79h): lines 152-199 cleared */
void tgfx_clear_text(void);
/* Gra_ClearScreen (GR_CLRSCR_EGA 204Ah): the first 8000 bytes of each plane cleared */
void tgfx_clear_screen(void);
/* Gra_Blit (GR_3_EGA 16D1h): w bytes x h lines of the picture pic (far pointer) from
 * (src_x, src_y) to the screen at (dst_x, dst_y); x in bytes, y in lines */
void tgfx_blit(int w, int h, int src_x, int src_y, const uint8_t *pic, int dst_y, int dst_x);
/* Gra_BlitReveal (GR_B_EGA 173Fh): Gra_Blit drawing only the non-black bytes, each ANDed
 * with a random mask that has more bits as step grows (0..56 in the title) */
void tgfx_blit_reveal(int w, int h, int src_x, int src_y, const uint8_t *pic, int dst_y,
	int step, int dst_x);
/* Gra_PutChar (GR_PUTCHAR_EGA 1F2Eh): character c at byte column x (txt_X), text line y
 * (txt_Y) */
void tgfx_putchar(int y, int x, int c);
/* Gra_AnimFlow (GR_ANIMFLOW_EGA 1BFAh): a tile's rows rotated down by one, each plane */
void tgfx_anim_flow(int tile);
/* Gra_AnimFlags (GR_ANIMFLAG_EGA 1CF4h): the flags of five tiles wave */
void tgfx_anim_flags(void);

/* Gra_LoadPicture (GR_INFLATE_EGA 195Fh): an LZW-compressed picture (two pixels a byte)
 * into buf as 4 planes of 8000 bytes, plane 3 first (buf: 32000 bytes at least) */
void tgfx_load_picture(const char *name, uint8_t *buf);
/* LoadCompressed (217Eh): the decompressed bytes of the file into buf (SHAPES.EGZ:
 * 32768 bytes) */
void tgfx_load_compressed(const char *name, uint8_t *buf);
/* InterleaveCharset (20B4h): 128 characters from 4 bits a pixel to 4 planes of 8 bytes */
void tgfx_interleave_charset(uint8_t *charset);
/* InterleaveShapes (210Ch): 256 tiles from 4 bits a pixel to 4 planes of 32 bytes */
void tgfx_interleave_shapes(uint8_t *shapes);
/* _Inflate (INFLATE.ASM 23C3h): the file decompressed, each byte given to cb; 0, or -1
 * when the file cannot be read */
int tgfx_inflate(const char *name, void (*cb)(uint8_t));

#endif
