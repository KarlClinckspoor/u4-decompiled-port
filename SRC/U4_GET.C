/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*
 * Acid trap: flash the character and hurt him by 0-29 HP.
 */
int16 /*C_70CE*/AcidTrap(chara)
int16 chara;
{
	FlashChara(chara);
	hitChara(chara, U4_RND3(30));
}

/*
 * The chest's content: 10 to 96 gold, added to the party's gold (capped at 9999).
 */
int16 /*C_70F1*/ChestGold()
{
	register int16 gold;

	u4_puts(/*D_2240*/"The Chest Holds:\n");
	gold = U4_RND3(80) + U4_RND1(7) + 10;
	u4_putl(gold, 1, ' ');
	u4_puts(/*D_2252*/" Gold\n");
	Party._gold = ((uint16)(Party._gold + gold) <= 9999)?Party._gold + gold:9999;
}

/*
 * Maybe set off the chest's trap on the character who opens it (-1: nobody, the
 * trap is always evaded), then give the gold.
 * Half of the chests are trapped; the character evades the trap if a d100 roll is
 * at most his dexterity + 25.
 */
int16 /*C_7150*/ChestTrap(chara)
int16 chara;
{
	register uint16 trap;

	if(((trap = U4_RND1(3))&1) == 0) {
		/*note: trap's bit 0 is always clear when reaching here, so
		 trap can only be 0 or 2: the Sleep and Bomb traps never occur.*/
		trap &= u_rand_a();
		if(trap == 0)
			u4_puts(/*D_2259*/"Acid");
		else if(trap == 1)
			u4_puts(/*D_225E*/"Sleep");
		else if(trap == 2)
			u4_puts(/*D_2264*/"Poison");
		else
			u4_puts(/*D_226B*/"Bomb");
		u4_puts(/*D_2270*/" Trap!\n");
		if(chara == -1 || U4_RND3(100) <= (uint16)(Party.chara[chara]._dex + 25)) {
			u4_puts(/*D_2278*/"Evaded!\n");
			sound(SND_FAIL);
		} else if(trap == 0) {/*Acid*/
			AcidTrap(chara);
		} else if(trap == 1) {/*Sleep*/
			Party.chara[chara]._stat = 'S';
			if(CurMode >= MOD_COMBAT)
				Fighters._chtile[chara] = TIL_SLEEPING;
			FlashChara(chara);
		} else if(trap == 2) {/*Poison*/
			Party.chara[chara]._stat = 'P';
			dspl_Stats();
			FlashChara(chara);
		} else {/*Bomb*/
			DamageAllParty();
		}
	}
	ChestGold();
}

/*
 * Open the chest under the party (outside of combat), opened by character chara
 * (-1 for the Open spell: the trap is always evaded).
 * In the dungeon the chest is map tile 0x40, emptied to 0. In a town/castle, a chest
 * that is part of the map is stolen: Honesty, Justice and Honor go down. Otherwise
 * the chest is an object (a monster's loot), removed from the NPC list.
 * Note: if no chest object is found the loop ends at -1 and slot -1 is cleared,
 * writing out of the arrays (cannot happen as tile_cur was a chest).
 */
int16 /*C_722F*/OpenChest(chara)
int16 chara;
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
			register int16 npc;

			for(npc = 0x1f; npc >= 0; npc --)
				if(
					Area._npc._tile[npc] == TIL_CHEST &&
					Party._x == Area._npc._x[npc] &&
					Party._y == Area._npc._y[npc]
				) break;
			Area._npc._tile[npc] = Area._npc._gtile[npc] = 0;
		}
	}
	ChestTrap(chara);
}

/*
 * (G)et chest: ask who opens the chest under the party (not possible while
 * flying the balloon).
 */
int16 /*C_72EC*/CMD_Get()
{
	register int16 chara;

	if(Party._fly_light != 0 && Party._tile == TIL_BALLOON) {
		u4_puts(/*D_2281*/"Get chest\n");
		w_DriftOnly();
		return;
	}
	u4_puts(/*D_228C*/"Get Chest!\n");
	if((chara = AskChara(/*D_2298*/"Who opens?\x12\x12\b")) < 0)
		return;
	if(!isCharaConscious(chara)) {
		w_Disabled();
		return;
	}
	OpenChest(chara);
}

/*
 * Open the chest under the active character in combat: it becomes a floor tile
 * (and the dungeon map's chest is removed too when the combat is a dungeon room).
 */
int16 /*C_7337*/COM_OpenChest()
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

/*
 * (G)et chest in combat.
 */
int16 /*C_73B8*/COM_GetChest()
{
	u4_puts(/*D_22A6*/"Get Chest!\n");
	COM_OpenChest();
}

/*----------------------------------------
FIRE
----------------------------------------*/

char /*D_22C8*/MsgBroadsidesOnly[] = "Broadsides Only!\n";

/*
 * (F)ire the ship's cannons, sideways only. The cannon ball flies up to 3 squares
 * (over any terrain) and stops on the first NPC that is not a whirlpool or a
 * balloon. A monster (NPC slots 0-7, the monsters' slots) is destroyed only 1 time out of 4,
 * Lord British never; any other object is destroyed.
 */
int16 /*C_73C9*/CMD_Fire()
{
	int16 npc, row, stepX, stepY, range, col;

	u4_puts(/*D_22B2*/"Fire ");
	if(Party._tile < TIL_SHIP_W || Party._tile > TIL_SHIP_S) {
		w_What();
		return;
	}
	u4_puts(/*D_22B8*/"Cannon!\n");
	AskDir(/*D_22C1*/"Dir: ", &stepX, &stepY);
	if(!(stepX|stepY))
		return;
	if(stepX != 0) {
		if(Party._tile != TIL_SHIP_N && Party._tile != TIL_SHIP_S) {
			u4_puts(MsgBroadsidesOnly);
			sound(SND_BAD_COMMAND);
			return;
		}
	} else if(Party._tile != TIL_SHIP_W && Party._tile != TIL_SHIP_E) {
		u4_puts(MsgBroadsidesOnly);
		sound(SND_BAD_COMMAND);
		return;
	}
	/*-- FIRING --*/
	sound(SND_SHOOT);
	col = Party._x + stepX;
	row = Party._y + stepY;
	hit_x = stepX + 5;
	hit_y = stepY + 5;
	hit_tile = TIL_MISSILE;
	for(range = 1; ; range++) {
		if(range > 3 || (
			((npc = FindNpcAt(0x1f, col, row)) != -1) &&
			(Area._npc._tile[npc] != TIL_WHIRLPOOL && Area._npc._tile[npc] != TIL_BALLOON)
		)) {
			hit_tile = 0;
			if(npc == -1)
				return;
			/*-- HIT SOMETHING --*/
			hit_tile = TIL_HIT_FLASH; DrawView(); hit_tile = 0;
			sound(SND_HIT);
			if(npc < 8 && U4_RND1(3))
				return;
			if(Area._npc._tile[npc] == TIL_LORD_BRITISH)
				return;
			break;
		}
		DrawView();
		hit_x += stepX; col += stepX;
		hit_y += stepY; row += stepY;
		npc = -1;
	}
	Area._npc._tile[npc] = Area._npc._gtile[npc] = 0;
}

/*----------------------------------------
IGNITE
----------------------------------------*/

/*
 * (I)gnite a torch: dungeon only, 100 more turns of light.
 */
int16 /*C_7525*/CMD_Ignite()
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
