/* replaces nothing: checks at compile time that the game's structures have their DOS sizes
 * on the host (they are read and written whole: the saved game, the maps, the combat
 * maps), as Microsoft C 4.00 laid them out with /Zp. */
#include "u4.h"

_Static_assert(sizeof(struct tChara) == 0x27, "tChara");
_Static_assert(sizeof(struct tParty) == 0x1F6, "tParty (PARTY.SAV)");
_Static_assert(sizeof(struct tNPC) == 0x100, "tNPC (MONSTERS.SAV, OUTMONST.SAV)");
_Static_assert(sizeof(struct tArea) == 0x500, "tArea (*.ULT)");
_Static_assert(sizeof(struct tCombat) == 0xC0, "tCombat (*.CON)");
_Static_assert(sizeof(struct tFighters) == 0x68, "tFighters");
_Static_assert(sizeof(tMap8x8x8) == 0x200, "tMap8x8x8 (*.DNG)");
