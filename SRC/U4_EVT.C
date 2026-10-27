/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*
 * Sleep field: each conscious character falls asleep one time in 4.
 */
/*C_919A*/SleepParty()
{
	register int chara;

	for(chara = Party._members; --chara >= 0; ) {
		if(isCharaConscious(chara) && U4_RND1(3) == 0) {
			Party.chara[chara]._stat = 'S';
			FlashChara(chara);
			dspl_Stats();
		}
	}
}

/*
 * Swamp or poison field: each character in good health is poisoned one time in 8.
 */
/*C_91D1*/PoisonParty()
{
	register int chara;

	for(chara = Party._members; --chara >= 0; ) {
		if(Party.chara[chara]._stat == 'G' && U4_RND1(7) == 0) {
			Party.chara[chara]._stat = 'P';
			FlashChara(chara);
			dspl_Stats();
		}
	}
}

/*
 * The effects of the tile the party (in combat: the active character) stands on,
 * at the start of each turn:
 * - outdoors, on a bridge: bridge trolls attack one time in 8;
 * - nothing while flying the balloon;
 * - in combat, on the active character only: swamp/poison field poison, sleep field
 *   sleep, fire field/lava 16-47 damage;
 * - in a dungeon: the fields (0xA0 poison, 0xA2 fire, 0xA3 sleep; the energy field
 *   0xA1 cannot be entered) and the traps (0x80 winds blow the torch out, 0x81-0x87
 *   falling rocks, 0x88-0x8F pit);
 * - elsewhere: swamp/poison field, fire field/lava, sleep field, on the whole party.
 */
/*C_9209*/TileEffects()
{
	unsigned damage;

	if(CurMode == MOD_OUTDOORS && tile_cur == TIL_BRIDGE && U4_RND1(7) == 0) {
		u4_puts(/*D_2998*/"\nBridge Trolls!\n");
		CombatFoeTile = TIL_TROLL;
		CombatFoeX = Party._x;
		CombatFoeY = Party._y;
		CombatGround = TIL_BRIDGE;
		CombatBegin();
		return;
	}
	if(CurMode <= MOD_BUILDING && Party._fly_light != 0)
		return;
	if(CurMode >= MOD_COMBAT) {
		if(Fighters._chtile[activeChara] == 0)
			return;
		switch(Combat_MAP(Combat._charaY[activeChara], Combat._charaX[activeChara])) {
			case TIL_SWAMP: case TIL_POISON_FIELD:
				if(Party.chara[activeChara]._stat == 'G') {
					Party.chara[activeChara]._stat = 'P';
					FlashChara(activeChara);
					dspl_Stats();
				}
			break;
			case TIL_SLEEP_FIELD:
				if(isCharaConscious(activeChara)) {
					Party.chara[activeChara]._stat = 'S';
					Fighters._chtile[activeChara] = TIL_SLEEPING;
					FlashChara(activeChara);
					dspl_Stats();
				}
			break;
			case TIL_FIRE_FIELD: case TIL_LAVA:
				FlashChara(activeChara);
				damage = U4_RND3(32) + 16;
				hitChara(activeChara, damage);
			break;
		}
		return;
	}
	if(CurMode == MOD_DUNGEON) {
		if((tile_cur & 0xf0) == 0xa0) {
			switch(tile_cur & 3) {
				case 0: PoisonParty(); break;
				case 2: DamageAllParty(); break;
				case 3: SleepParty(); break;
			}
		} else if((tile_cur & 0xf0) == 0x80) {
			if(tile_cur == 0x80) {
				u4_puts(/*D_29A9*/"\nWinds!\n");
				Party._fly_light = 0;
			} else {
				u4_puts((tile_cur < 0x88)?/*D_29B2*/"\nFalling Rocks!\n":/*D_29C3*/"\nPit!\n");
				DamageAllParty();
			}
		}
		return;
	}
	switch(tile_cur) {
		case TIL_SWAMP: case TIL_POISON_FIELD: PoisonParty(); break;
		case TIL_FIRE_FIELD: case TIL_LAVA: DamageAllParty(); break;
		case TIL_SLEEP_FIELD: SleepParty(); break;
	}
}
