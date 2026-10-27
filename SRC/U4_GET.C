/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*C_70CE*/AcidTrap(bp04)
int bp04;
{
	FlashChara(bp04);
	hitChara(bp04, U4_RND3(30));
}

/*C_70F1*/ChestGold()
{
	register int si;

	u4_puts(/*D_2240*/"The Chest Holds:\n");
	si = U4_RND3(80) + U4_RND1(7) + 10;
	u4_putl(si, 1, ' ');
	u4_puts(/*D_2252*/" Gold\n");
	Party._gold = (Party._gold + si <= 9999)?Party._gold + si:9999;
}

/*trapped chest ?*/
/*C_7150*/ChestTrap(bp04)
int bp04;
{
	register unsigned si;

	if(((si = U4_RND1(3))&1) == 0) {
		/*note:si's bit 0 is always reset when reaching here,
		 therefore, Sleep or Poison trap never occur.*/
		si &= u_rand_a();
		if(si == 0)
			u4_puts(/*D_2259*/"Acid");
		else if(si == 1)
			u4_puts(/*D_225E*/"Sleep");
		else if(si == 2)
			u4_puts(/*D_2264*/"Poison");
		else
			u4_puts(/*D_226B*/"Bomb");
		u4_puts(/*D_2270*/" Trap!\n");
		if(bp04 == -1 || U4_RND3(100) <= Party.chara[bp04]._dex + 25) {
			u4_puts(/*D_2278*/"Evaded!\n");
			sound(8);
		} else if(si == 0) {/*Acid*/
			AcidTrap(bp04);
		} else if(si == 1) {/*Sleep*/
			Party.chara[bp04]._stat = 'S';
			if(CurMode >= MOD_COMBAT)
				Fighters._chtile[bp04] = TIL_SLEEPING;
			FlashChara(bp04);
		} else if(si == 2) {/*Poison*/
			Party.chara[bp04]._stat = 'P';
			dspl_Stats();
			FlashChara(bp04);
		} else {/*Bomb*/
			DamageAllParty();
		}
	}
	ChestGold();
}

/*C_722F*/OpenChest(bp04)
int bp04;
{
	if(CurMode == MOD_DUNGEON) {
		if(tile_cur != 0x40) {
			w_NotHere();
			return;
		}
		Area._map.x8x8x8[Party._z][Party._y][Party._x] = 0;
	} else {
		if(tile_cur != TIL_CHEST) {
			w_NotHere();
			return;
		}
		if(Party._loc != 0 && Area._map.x32x32[Party._y][Party._x] == TIL_CHEST) {
			Area._map.x32x32[Party._y][Party._x] = TIL_BRICK_FLOOR;
			karma_dec(&(Party._hones), 1);
			karma_dec(&(Party._justi), 1);
			karma_dec(&(Party._honor), 1);
		} else {
			register int si;

			for(si = 0x1f; si >= 0; si --)
				if(
					Area._npc._tile[si] == TIL_CHEST &&
					Party._x == Area._npc._x[si] &&
					Party._y == Area._npc._y[si]
				) break;
			Area._npc._tile[si] = Area._npc._gtile[si] = 0;
		}
	}
	ChestTrap(bp04);
}

/*C_72EC*/CMD_Get()
{
	register int si;

	if(Party._fly_light != 0 && Party._tile == TIL_BALLOON) {
		u4_puts(/*D_2281*/"Get chest\n");
		w_DriftOnly();
		return;
	}
	u4_puts(/*D_228C*/"Get Chest!\n");
	if((si = AskChara(/*D_2298*/"Who opens?\x12\x12\b")) < 0)
		return;
	if(!isCharaConscious(si)) {
		w_Disabled();
		return;
	}
	OpenChest(si);
}

/*C_7337*/COM_OpenChest()
{
	if(Combat_MAP(Combat._charaY[activeChara], Combat._charaX[activeChara]) != TIL_CHEST) {
		w_NotHere();
		return;
	}
	Combat_MAP(Combat._charaY[activeChara], Combat._charaX[activeChara]) = TIL_TILED_FLOOR;
	if(Area._map.x8x8x8[Party._z][Party._y][Party._x] == 0x40)
		Area._map.x8x8x8[Party._z][Party._y][Party._x] = 0;
	ChestTrap(activeChara);
}

/*(G)et chest [combat]*/
/*C_73B8*/COM_GetChest()
{
	u4_puts(/*D_22A6*/"Get Chest!\n");
	COM_OpenChest();
}

/*----------------------------------------
FIRE
----------------------------------------*/

char /*D_22C8*/MsgBroadsidesOnly[] = "Broadsides Only!\n";

/*C_73C9*/CMD_Fire()
{
	int loc_A, loc_B, loc_C, loc_D, loc_E, loc_F;

	u4_puts(/*D_22B2*/"Fire ");
	if(Party._tile < TIL_SHIP_W || Party._tile > TIL_SHIP_S) {
		w_What();
		return;
	}
	u4_puts(/*D_22B8*/"Cannon!\n");
	AskDir(/*D_22C1*/"Dir: ", &loc_C, &loc_D);
	if(!(loc_C|loc_D))
		return;
	if(loc_C != 0) {
		if(Party._tile != TIL_SHIP_N && Party._tile != TIL_SHIP_S) {
			u4_puts(MsgBroadsidesOnly);
			sound(2);
			return;
		}
	} else if(Party._tile != TIL_SHIP_W && Party._tile != TIL_SHIP_E) {
		u4_puts(MsgBroadsidesOnly);
		sound(2);
		return;
	}
	/*-- FIRING --*/
	sound(3);
	loc_F = Party._x + loc_C;
	loc_B = Party._y + loc_D;
	hit_x = loc_C + 5;
	hit_y = loc_D + 5;
	hit_tile = TIL_MISSILE;
	for(loc_E = 1; ; loc_E++) {
		if(loc_E > 3 || (
			((loc_A = FindNpcAt(0x1f, loc_F, loc_B)) != -1) &&
			(Area._npc._tile[loc_A] != TIL_WHIRLPOOL && Area._npc._tile[loc_A] != TIL_BALLOON)
		)) {
			hit_tile = 0;
			if(loc_A == -1)
				return;
			/*-- HIT SOMETHING --*/
			hit_tile = TIL_HIT_FLASH; DrawView(); hit_tile = 0;
			sound(6);
			if(loc_A < 8 && U4_RND1(3))
				return;
			if(Area._npc._tile[loc_A] == TIL_LORD_BRITISH)
				return;
			break;
		}
		DrawView();
		hit_x += loc_C; loc_F += loc_C;
		hit_y += loc_D; loc_B += loc_D;
		loc_A = -1;
	}
	Area._npc._tile[loc_A] = Area._npc._gtile[loc_A] = 0;
}

/*----------------------------------------
IGNITE
----------------------------------------*/

/*C_7525*/CMD_Ignite()
{
	u4_puts(/*D_22DA*/"Ignite Torch!\n");
	if(CurMode != MOD_DUNGEON) {
		w_NotHere();
		return;
	}
	if(Party._torches == 0) {
		w_NoneLeft();
		return;
	}
	Party._torches --;
	Party._fly_light += 100;
	if(CurMode == MOD_DUNGEON)
		DNG_LookAround();
}
