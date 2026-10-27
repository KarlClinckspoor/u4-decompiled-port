/* Test shim for tgfx_diff.py: tgfx.c built into a shared library (with ega.c, for the
 * planes) and recording hooks. tgfx.c is included (not linked) so the test can reach its
 * static state: the line table, the random states, the decoder's output count. */
#include "../gfx/tgfx.c"

#include "shim_hooks.c"

uint16_t *shim_lines(void) { return lines; }
uint8_t *shim_seed(void) { return seed; }
uint8_t *shim_reveal_a(void) { return &reveal_a; }
uint8_t *shim_reveal_b(void) { return &reveal_b; }
unsigned shim_out_count(void) { return out_count; }
