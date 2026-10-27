/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*DNG_Advance();
DNG_Retreat();
DNG_TurnLeft();
DNG_TurnRight();
DNG_Klimb();
DNG_Descend();
DNG_EndOfTurn();*/

/*
 * The dungeon loop: read and run the commands until the party climbs out of the
 * dungeon (Party._z < 0). Ctrl-S right after Alt-Z shows the karma (cheat).
 */
/*C_84D2*/DNG_main()
{
	register uint16 cmd;
	int16 prevCmd;

	Redraw3D = 1;
	u_kbflush();
	prevCmd = 0;
	do {
		TileEffects();
		u4_putc(0x10);
		RefreshView();
		if(CheckPartyAlive()) {
			u_delay(25, 1);
			cmd = u_kbhit()?u_kbread():KBD_SPACE;
			if(u4_isupper((unsigned char)cmd))
				cmd = (cmd & 0xff00) | u4_lower((unsigned char)cmd);
			switch(cmd) {
				case KBD_SPACE: w_Pass(); break;
				case KBD_UP:
				case KBD_UP_: DNG_Advance(); break;
				case KBD_LEFT:
				case KBD_LEFT_: DNG_TurnLeft(); break;
				case KBD_DOWN:
				case KBD_DOWN_: DNG_Retreat(); break;
				case KBD_RIGHT:
				case KBD_RIGHT_: DNG_TurnRight(); break;
				case KBD_C: CMD_Cast(); break;
				case KBD_D: DNG_Descend(); break;/*(D)escend [dungeon]*/
				case KBD_G: CMD_Get(); break;
				case KBD_H: CMD_HoleUp(); break;
				case KBD_I: CMD_Ignite(); break;
				case KBD_K: DNG_Klimb(); break;/*(K)limb [dungeon]*/
				case KBD_M: CMD_Mix(); break;
				case KBD_N: CMD_NewOrder(); break;
				case KBD_P: CMD_Peer(); break;
				case KBD_Q: CMD_Quit(); break;
				case KBD_R: CMD_Ready(); break;
				case KBD_S: DNG_Search(); break;/*(S)earch [dungeon]*/
				case KBD_U: CMD_Use(); break;
				case KBD_V: CMD_Volume(); break;
				case KBD_W: CMD_Wear(); break;
				case KBD_Z: CMD_Ztats(); break;
				case KBD_A:
				case KBD_B: 
				case KBD_E:
				case KBD_F:
				case KBD_J:
				case KBD_L:
				case KBD_O:
				case KBD_T:
				case KBD_X:
				case KBD_Y: w_NotHere(); sound(SND_BAD_COMMAND); break;
				case KBD_CTRL_S:
					if(prevCmd == KBD_ALT_Z) {
						CheatShowKarma();
						break;
					}
					/*no break: a plain Ctrl-S is a bad command*/
				default:
					u4_puts(/*D_2626*/"Bad command\n");
					sound(SND_BAD_COMMAND);
					TurnDone = 0;
			}
			DNG_LookAround();
		} else {
			u4_puts(/*D_2633*/"Zzzzz\n");
		}
		if(TurnDone != 0 && Party._z >= 0)
			DNG_EndOfTurn();
		else
			TurnDone = 1;
		prevCmd = cmd;
	} while(Party._z >= 0);
	LeaveTown();
	dclose(File_DNG);
	File_DNG = 0;
}

/*
 * In a dungeon, a low nibble n != 0 in the map's tile means monster
 * TIL_RAT + (n-1)*4 is on that square (except for the traps, fountains, fields,
 * rooms and walls, whose low nibble means something else). If the party stands on
 * one, remove it (from the map and from the NPC list) and fight it.
 * Returns 1 if there was a combat.
 */
/*C_8743*/DNG_CheckEncounter()
{
	register int16 npc;

	if(
		(tile_cur & 0x0f) == 0 ||
		(tile_cur & 0xf0) == 0x80 ||
		(tile_cur & 0xf0) == 0x90 ||
		(tile_cur & 0xf0) == 0xA0 ||
		(tile_cur & 0xf0) == 0xD0 ||
		(tile_cur & 0xf0) == 0xF0
	) return 0;
	CombatFoeTile = CombatFoe = TIL_RAT + (tile_cur & 0x0f) * 4 - 4;
	Area._map.x8x8x8[Party._z][Party._y][Party._x] &= 0xf0;
	for(npc = 0x1f; npc >= 0; npc --) {
		if(
			Area._npc._var[npc] == Party._z &&
			Area._npc._x[npc] == Party._x &&
			Area._npc._y[npc] == Party._y
		) {
			Area._npc._var[npc] =
			Area._npc._tile[npc] =
			Area._npc._x[npc] =
			Area._npc._y[npc] = 0;
			break;
		}
	}
	DungeonCombat();

	return 1;
}

/*
 * End of a turn in the dungeon: sleeping characters may wake up (1/8), poisoned
 * ones lose 2 HP, food, magic points, the monsters move and appear, the torch
 * burns, the spell effects end; then the encounter with a monster under the party,
 * or the rooms the party walks into.
 */
/*C_87E2*/DNG_EndOfTurn()
{
	register int16 chara;
	register struct tChara *pChara;

	CheckPartyAlive();
	for(pChara = &(Party.chara[0]), chara = 0; chara < Party._members; pChara ++, chara++) {
		if(pChara->_stat == 'S' && U4_RND1(7) == 0) {
			pChara->_stat = 'G';
		} else if(pChara->_stat == 'P') {
			hitChara(chara, 2);
			Gra_InvertStatusLine(chara);
			sound(SND_HIT);
			Gra_InvertStatusLine(chara);
		}
	}
	if(food_dec(Party._members)) {
		u4_puts(/*D_263A*/"\nStarving!!!\n");
		for(chara = 0; chara < Party._members; chara++) {
			if(isCharaAlive(chara))
				hitChara(chara, 2);
		}
		for(chara = Party._members; --chara >= 0; )
			Gra_InvertStatusLine(chara);
		sound(SND_HIT);
		for(chara = Party._members; --chara >= 0; )
			Gra_InvertStatusLine(chara);
	}
	MP_recover();
	DNG_MoveMonsters();
	DNG_SpawnMonsters();
	if(Party._fly_light)
		Party._fly_light --;
	Party._moves ++;
	if(spell_turns) {
		if(--spell_turns == 0)
			spell_effect = 7;
	}
	if(Party._fly_light == 0)
		u4_puts(/*D_2648*/"It's Dark!\n");
	dspl_Stats();
	if(DNG_CheckEncounter() == 0) {
		while((tile_cur & 0xf0) == 0xd0) {
			CheckPartyAlive();/*check death*/
			EnterDungeonRoom();/*enter dungeon room*/
			DNG_Render();/*render 3d*/
		}
	}
}

/*
 * Can the party walk onto dungeon tile tile? Doors, secret doors and rooms
 * (0xC0, 0xE0, 0xD0) only when moving forward (backward == 0); otherwise any tile
 * below 0xC0 (not the walls) except the energy fields (0xA1).
 */
/*C_88E8*/DNG_CanEnter(tile, backward)
unsigned char tile;
uint16 backward;
{
	if(backward == 0) {
		if(
			(tile & 0xf0) == 0xc0 ||
			(tile & 0xf0) == 0xe0 ||
			(tile & 0xf0) == 0xd0
		) return 1;
	}
	if(tile != 0xa1 && tile < 0xc0)
		return 1;
	return 0;
}

/*
 * Up arrow: move one square forward.
 */
/*C_891E*/DNG_Advance()
{
	u4_puts(/*D_2654*/"Advance\n");
	if(!DNG_CanEnter(tile_forward, 0)) {
		w_Blocked();
		return;
	}
	Party._x = DNG_X_p(Party._x, Party._dir);
	Party._y = DNG_Y_p(Party._y, Party._dir);
}

/*
 * Down arrow: move one square backward (facing the same way).
 */
/*C_895F*/DNG_Retreat()
{
	u4_puts(/*D_265D*/"Retreat\n");
	if(!DNG_CanEnter(tile_back, 1)) {
		w_Blocked();
		return;
	}
	Party._x = DNG_X_m(Party._x, Party._dir);
	Party._y = DNG_Y_m(Party._y, Party._dir);
}

/*
 * Left arrow: turn left (does not count as a turn).
 */
/*C_899F*/DNG_TurnLeft()
{
	u4_puts(/*D_2666*/"Turn Left\n");
	Party._dir = (Party._dir - 1) & 3;
	TurnDone = 0;
}

/*
 * Right arrow: turn right (does not count as a turn).
 */
/*C_89BD*/DNG_TurnRight()
{
	u4_puts(/*D_2671*/"Turn Right\n");
	Party._dir = (Party._dir + 1) & 3;
	TurnDone = 0;
}

/*
 * (K)limb a ladder up (tile 0x10 or 0x30); climbing above level 1 leaves the
 * dungeon (Party._z < 0 ends DNG_main).
 */
/*C_89DB*/DNG_Klimb()
{
	u4_puts(/*D_267D*/"Klimb ");
	if(
		(tile_cur & 0xf0) == 0x10 ||
		(tile_cur & 0xf0) == 0x30
	) {
		u4_puts(/*D_2684*/"up!\n");
		if(--Party._z >= 0) {
			u4_puts(/*D_2689*/"To level ");
			u4_putc(Party._z + '1');
			Gra_CR();
		}
	} else {
		w_What();
	}
}

/*
 * (D)escend a ladder down (tile 0x20 or 0x30).
 */
/*C_8A1F*/DNG_Descend()
{
	u4_puts(/*D_2693*/"Descend ");
	if(
		(tile_cur & 0xf0) == 0x20 ||
		(tile_cur & 0xf0) == 0x30
	) {
		Party._z ++;
		u4_puts(/*D_269C*/"down to level ");
		u4_putc(Party._z + '1');
		Gra_CR();
	} else {
		w_What();
	}
}
