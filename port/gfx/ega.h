/* The EGA: its four bit planes (mode 0Dh, 320x200, 40 bytes a line) and the game's
 * graphics driver EGA.DRV, in C (ega.c).
 *
 * AVATAR.EXE loads EGA.DRV and calls its 26 entries through U4_GRAPH.ASM (port/sys/graph.c
 * in the port). Each entry here does what the driver's code does to the planes, byte for
 * byte: the same reads, the same EGA register settings (map mask, bit mask, logical
 * function, write mode 1 copies through the latches), so the screen is the one DOS showed.
 * The arguments are the ones the driver finds on its stack.
 */
#ifndef PORT_EGA_H
#define PORT_EGA_H

#include <stdint.h>

#include "ega_hw.h"

/* 1 when the screen changed since ega_take_dirty() */
int ega_take_dirty(void);
/* converts the planes to one byte a pixel (0..15) */
void ega_to_chunky(uint8_t *out320x200);
/* the 6-bit VGA colours of EGA palette value v (0..3Fh) */
void ega_rgb6(int v, uint8_t *rgb);

/* ---- EGA.DRV's entries, in the order of its table ---- */
void ega_active_box(int row, int col);                                  /* 0 */
void ega_zoom_tile_row(int tile, int trow, int srow, int scol, int zoom); /* 1 */
void ega_fill_tiles(int h, int w, int tile, int row, int col);          /* 2 */
void ega_shift_down(int sound);                                         /* 3 */
void ega_shift_up(int sound);                                           /* 4 */
void ega_draw_tiles(int h, int w, const uint8_t *tiles);                /* 5 */
void ega_dot(int y, int x, int color);                                  /* 6 */
void ega_cr(void);                                                      /* 8 */
void ega_invert_view(void);                                             /* 9 */
void ega_clear_view(void);                                              /* 10 */
void ega_invert_status_line(int line);                                  /* 11 */
void ega_invert_char(int x, int y);                                     /* 12 */
void ega_clear_status(void);                                            /* 13 */
void ega_dot_or(int y, int x, int color);                               /* 14 */
void ega_dot_xor(int y, int x, int color);                              /* 15 */
/* buf: 0 for the screen, else 4 planes of 8000 bytes (plane 3 first) */
void ega_load_picture(const char *name, int ofs, uint8_t *buf);         /* 16 */
void ega_reveal_codex(const char *name);                                /* 17 */
void ega_draw_quad(int x0, int x1, int y0, int y1, int y0p, int y1p, int col0, int col1); /* 18 */
void ega_virtue_glyph(const uint16_t *const *karmas);                   /* 19 */
void ega_anim_campfire(void);                                           /* 20 */
void ega_init(uint8_t *shapes, uint8_t *charset);                       /* 21 */
void ega_clear_screen(void);                                            /* 22 */
void ega_anim_flow(int tile);                                           /* 23 */
void ega_anim_flags(void);                                              /* 24 */
void ega_putchar(int y, int x, int c);                                  /* 25 */

/* hooks the driver needs from the port */
void ega_speaker_noise(int toggles);    /* the shake's speaker clicks */
void ega_fatal_exit(int code);          /* the driver's exit callback */
int ega_read_file(const char *name, uint8_t **data, int *size); /* 0 ok */
void ega_free_file(uint8_t *data);
void ega_busy_wait(long iterations);    /* a delay loop of the driver */

#endif
