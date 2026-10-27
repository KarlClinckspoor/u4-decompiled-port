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

/*C_84D2*/DNG_main()
{
	register unsigned si;
	int bp_04;

	Redraw3D = 1;
	u_kbflush();
	bp_04 = 0;
	do {
		TileEffects();
		u4_putc(0x10);
		RefreshView();
		if(CheckPartyAlive()) {
			u_delay(25, 1);
			si = u_kbhit()?u_kbread():KBD_SPACE;
			if(u4_isupper((unsigned char)si))
				si = (si & 0xff00) | u4_lower((unsigned char)si);
			switch(si) {
				case KBD_SPACE: w_Pass(); break;
				case KBD_UP:
				case 0x487e: DNG_Advance(); break;
				case KBD_LEFT:
				case 0x4b7c: DNG_TurnLeft(); break;
				case KBD_DOWN:
				case 0x5060: DNG_Retreat(); break;
				case KBD_RIGHT:
				case 0xf400: DNG_TurnRight(); break;
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
				case KBD_Y: w_NotHere(); sound(2); break;
				case KBD_CTRL_S:
					if(bp_04 == KBD_ALT_Z) {
						CheatShowKarma();
						break;
					}
				default:
					u4_puts(/*D_2626*/"Bad command\n");
					sound(2);
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
		bp_04 = si;
	} while(Party._z >= 0);
	LeaveTown();
	dclose(File_DNG);
	File_DNG = 0;
}

/*C_8743*/DNG_CheckEncounter()
{
	register int si;

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
	for(si = 0x1f; si >= 0; si --) {
		if(
			Area._npc._var[si] == Party._z &&
			Area._npc._x[si] == Party._x &&
			Area._npc._y[si] == Party._y
		) {
			Area._npc._var[si] =
			Area._npc._tile[si] =
			Area._npc._x[si] =
			Area._npc._y[si] = 0;
			break;
		}
	}
	DungeonCombat();

	return 1;
}

/*C_87E2*/DNG_EndOfTurn()
{
	register int si;
	register struct tChara *di;

	CheckPartyAlive();
	for(di = &(Party.chara[0]), si = 0; si < Party._members; di ++, si++) {
		if(di->_stat == 'S' && U4_RND1(7) == 0) {
			di->_stat = 'G';
		} else if(di->_stat == 'P') {
			hitChara(si, 2);
			Gra_InvertStatusLine(si);
			sound(6);
			Gra_InvertStatusLine(si);
		}
	}
	if(food_dec(Party._members)) {
		u4_puts(/*D_263A*/"\nStarving!!!\n");
		for(si = 0; si < Party._members; si++) {
			if(isCharaAlive(si))
				hitChara(si, 2);
		}
		for(si = Party._members; --si >= 0; )
			Gra_InvertStatusLine(si);
		sound(6);
		for(si = Party._members; --si >= 0; )
			Gra_InvertStatusLine(si);
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

/*C_88E8*/DNG_CanEnter(bp06, bp04)
unsigned char bp06;
unsigned bp04;
{
	if(bp04 == 0) {
		if(
			(bp06 & 0xf0) == 0xc0 ||
			(bp06 & 0xf0) == 0xe0 ||
			(bp06 & 0xf0) == 0xd0
		) return 1;
	}
	if(bp06 != 0xa1 && bp06 < 0xc0)
		return 1;
	return 0;
}

/*Advance*/
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

/*Retreat*/
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

/*Turn Left*/
/*C_899F*/DNG_TurnLeft()
{
	u4_puts(/*D_2666*/"Turn Left\n");
	Party._dir = (Party._dir - 1) & 3;
	TurnDone = 0;
}

/*Turn Right*/
/*C_89BD*/DNG_TurnRight()
{
	u4_puts(/*D_2671*/"Turn Right\n");
	Party._dir = (Party._dir + 1) & 3;
	TurnDone = 0;
}

/*(K)limb [dungeon]*/
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

/*(D)escend [dungeon]*/
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
