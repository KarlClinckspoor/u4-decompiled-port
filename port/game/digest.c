/* replaces nothing: a digest of the game's state for the replay checkpoints.
 * Compiled like the game's own sources (compat.h, u4.h): it reads the game's globals. It
 * covers the party, the area (its map and people), the combat, the mode and the random
 * generator, which is what a replay that went another way changes first. */
#include "u4.h"

extern uint8_t seed_128[16];

static uint64_t h;

static void mix(const void *p, size_t n)
{
	const uint8_t *b = (const uint8_t *)p;
	size_t i;
	for (i = 0; i < n; i++) {
		h ^= b[i];
		h *= 0x100000001b3ull;	/* FNV-1a 64 */
	}
}

uint64_t port_game_digest(void)
{
	h = 0xcbf29ce484222325ull;
	mix(&Party, sizeof Party);
	mix(&Area, sizeof Area);
	mix(&Combat, sizeof Combat);
	mix(&Fighters, sizeof Fighters);
	mix(&CurMode, sizeof CurMode);
	mix(&txt_X, sizeof txt_X);
	mix(&txt_Y, sizeof txt_Y);
	mix(seed_128, sizeof seed_128);
	return h;
}
