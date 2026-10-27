/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*C_919A*/SleepParty()
{
	register int si;

	for(si = Party._members; --si >= 0; ) {
		if(isCharaConscious(si) && U4_RND1(3) == 0) {
			Party.chara[si]._stat = 'S';
			FlashChara(si);
			dspl_Stats();
		}
	}
}

/*C_91D1*/PoisonParty()
{
	register int si;

	for(si = Party._members; --si >= 0; ) {
		if(Party.chara[si]._stat == 'G' && U4_RND1(7) == 0) {
			Party.chara[si]._stat = 'P';
			FlashChara(si);
			dspl_Stats();
		}
	}
}

/*randomEvent*/
/*C_9209*/TileEffects()
{
	unsigned bp_02;

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
				bp_02 = U4_RND3(32) + 16;
				hitChara(activeChara, bp_02);
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
