/* Test shim for ega_diff.py: ega.c built into a shared library with recording hooks.
 * ega.c is included (not linked) so the test can reach its static state (the random seed,
 * the shapes and charset pointers). Built into a scratch directory by ega_diff.py. */
#include "../gfx/ega.c"

#include "shim_hooks.c"

/* the driver's own state */
uint8_t *shim_seed(void) { return seed; }
uint8_t *shim_shapes(void) { return shapes; }
uint8_t *shim_charset(void) { return charset; }
