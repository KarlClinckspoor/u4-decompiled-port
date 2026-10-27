/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <stdlib.h>

/*the monsters that may attack a camp*/
unsigned char /*D_26B8*/AmbushMonsters[] = {TIL_ORC,TIL_SKELETON,TIL_ROGUE,TIL_PYTHON,TIL_INSECTS,TIL_SLIME,TIL_TROLL,TIL_WISP};

/*
 * The camp is attacked by a random monster, at the party's position: the
 * characters in good health are asleep ('S') when the combat starts.
 */
/*C_8A5A*/Ambush()
{
	register int16 chara;

	u4_puts(/*D_26AC*/"Ambushed!\n");
	CombatFoeX = Party._x;
	CombatFoeY = Party._y;
	CombatFoeTile = CombatFoe = AmbushMonsters[U4_RND1(7)];
	for(chara = Party._members; --chara >= 0; ) {
		if(Party.chara[chara]._stat == 'G')
			Party.chara[chara]._stat = 'S';
	}
	dspl_Stats();
	RunCombat();
}

/*
 * (H)ole up & camp, on foot, outdoors or in a dungeon: the camp map is loaded and the
 * characters sleep. One time in 8 the camp is ambushed; otherwise, once every 100
 * moves, the living characters wake up, and recover HP and MP.
 * Note: the first loop clears 32 entries of Fighters._tile, which has 16: it also
 * clears Fighters._gtile (harmless).
 */
/*C_8AB0*/CMD_HoleUp() {
	register int16 chara;

	u4_puts(/*D_26C0*/"Hole up & Camp\n");
	if(CurMode != MOD_DUNGEON && Party._loc != 0) {
		w_NotHere();
		return;
	}
	if(Party._tile != TIL_AVATAR) {
		w_OnlyOnFoot();
		return;
	}
#ifdef WIN32
	if(Load(CurMode == MOD_DUNGEON?/*D_26D0*/"CAMP.DNG":/*D_26D9*/"CAMP.CON", sizeof(struct tCombat), &Combat) == -1)
#else
	if(Load(CurMode == MOD_DUNGEON?/*D_26D0*/"CAMP.DNG":/*D_26D9*/"CAMP.CON", (char *)&Fighters - (char *)&Combat, &Combat) == -1)
#endif
		exit(3);
	for(chara = 31; chara >= 0; chara--)
		Fighters._tile[chara] = 0;
	for(chara = Party._members; --chara >= 0; ) {
		CharaAnimTile[chara] = Fighters._chtile[chara] = isCharaAlive(chara)?TIL_SLEEPING:0;
	}
	dspl_Stats();
	u4_puts(/*D_26E2*/"Resting...\n");
	SavedMode = CurMode;
	CurMode = MOD_COM_CAMP;
	u_delay(10, 0);
	if(U4_RND1(7) == 0) {
		Ambush();
	} else if(Party._moves / 100 == Party._lastCamp) {
		u4_puts(/*D_26EE*/"No effect.\n");
	} else {
		Party._lastCamp = Party._moves / 100;
		for(chara = Party._members; --chara >= 0; ) {
			if(isCharaAlive(chara)) {
				if(Party.chara[chara]._stat == 'S')
					Party.chara[chara]._stat = 'G';
				HP_inc(chara, U4_RND1(119));
				HP_inc(chara, 99);
				Party.chara[chara]._MP = 99;
			}
		}
		MP_recover();
		u4_puts(/*D_26FA*/"Players Healed!\n");
	}
	dspl_Stats();
	CurMode = SavedMode;
	if(CurMode == MOD_DUNGEON)
		Redraw3D = 1;
}
