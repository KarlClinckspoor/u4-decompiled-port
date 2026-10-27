/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*----------------------------------------
Whirlpool and Twister management?
----------------------------------------*/

/*remove monster?*/
/*C_7821*/SwallowNpcsAt(si/*bp06*/, bp04)
register int si;
int bp04;
{
	register int di;

	for(di = 31; di >= 0; di --) {
		if(
			di != si &&
			Area._npc._tile[di] &&
			Area._npc._x[di] == Area._npc._x[si] &&
			Area._npc._y[di] == Area._npc._y[si]
		) {
			Area._npc._tile[di] = Area._npc._gtile[di] = 0;
			RefreshView();
			sound(bp04);
		}
	}
}

/*whirlpool contact?*/
/*C_786F*/WhirlpoolContact(si/*bp04*/)
register int si;
{
	if(Party._x == Area._npc._x[si] && Party._y == Area._npc._y[si]) {
		RefreshView();
		hit_y = hit_x = 5;
		hit_tile = TIL_WHIRLPOOL;
		DrawView();
		Party._x = 0x7f;
		Party._y = 0x4e;
		sound(11);
		DamageAllParty();
		Party._tile = TIL_SHIP_W;
		hit_tile = 0;
		LoadWorldWindow();
	}
	SwallowNpcsAt(si, 11);
}

/*twister contact?*/
/*C_78D1*/TwisterContact(si/*bp04*/)
register int si;
{
	if(Party._x == Area._npc._x[si] && Party._y == Area._npc._y[si]) {
		RefreshView();
		sound(12);
		DamageAllParty();
		DamageAllParty();
		DamageAllParty();
		DamageAllParty();
		hit_tile = 0;
	}
	SwallowNpcsAt(si, 12);
}

/*C_7918*/MoveVortices()
{
	register int si;

	if(CurMode != MOD_OUTDOORS)
		return;

	for(si = 3; si >= 0; si--) {
		if(Area._npc._tile[si] == TIL_WHIRLPOOL)
			WhirlpoolContact(si);
		else if(Area._npc._tile[si] == TIL_TWISTER)
			TwisterContact(si);
	}
}

/*----------------------------------------

----------------------------------------*/

/*C_794D*/w_SameExit()
{
	u4_puts(/*D_239E*/"All must use same exit!\n");
	sound(1);
}

/*exit combat zone*/
/*C_7962*/COM_CharaExits()
{
	Combat._charaX[activeChara] = Combat._charaY[activeChara] = 0x80;
	Fighters._chtile[activeChara] = 0;
	sound(8);
	if(CurMode == MOD_COMBAT && !IsNonEvil(CombatFoeTile) && Party.chara[activeChara]._HP[0] == Party.chara[activeChara]._HP[1]) {
		karma_dec(&(Party._valor), 2);
		karma_dec(&(Party._sacri), 2);
	}
}

/*exiting fight*/
/*C_79C9*/COM_TryExit(bp06, bp04)
unsigned char bp06;
unsigned char bp04;
{
	if(CurMode == MOD_COM_ROOM) {
		/*-- dungeon room --*/
		/*someone already exited W/E*/
		if(RoomExitX != 0) {
			if(bp06 != (unsigned char)RoomExitX) {
				w_SameExit();
				return;
			}
			COM_CharaExits();
			return;
		}
		/*someone already exited N/S*/
		if(RoomExitY != 0) {
			if(bp04 != (unsigned char)RoomExitY) {
				w_SameExit();
				return;
			}
			COM_CharaExits();
			return;
		}
		/*-- --*/
		if(bp06 > 10)
			RoomExitX = bp06;
		else
			RoomExitY = bp04;
	}
	COM_CharaExits();
}

/*room special tiles*/
/*C_7A15*/RoomTriggers(bp06, bp04)
unsigned char bp06;
unsigned char bp04;
{
	register int /*si*/loc_A;
	int loc_B, loc_C/*bp_04, bp_06*/;

	for(loc_A = 3; loc_A >= 0; loc_A --) {
		if(RoomTriggerData[(loc_A << 2)]) {
			if(
				(bp06 << 12) == (*(U16 *)(RoomTriggerData+(loc_A << 2)) & 0xf000) &&
				(bp04 <<  8) == (*(U16 *)(RoomTriggerData+(loc_A << 2)) & 0x0f00)
			) {
				loc_B = *(U16 *)(RoomTriggerData+(loc_A << 2)+2) & 0xf;
				if(
					loc_B |
					(loc_C = (*(U16 *)(RoomTriggerData+(loc_A << 2)+2) >> 4) & 0xf)
				) Combat_MAP(loc_B, loc_C) = RoomTriggerData[(loc_A << 2)];
				loc_B = (*(U16 *)(RoomTriggerData+(loc_A << 2)+2) >> 8) & 0xf;
				if(
					loc_B |
					(loc_C = (*(U16 *)(RoomTriggerData+(loc_A << 2)+2) >> 12) & 0xf)
				) Combat_MAP(loc_B, loc_C) = RoomTriggerData[(loc_A << 2)] & 0xff;
			}
		}
	}
}

/*move [fight]*/
/*C_7AE3*/COM_MoveChara(si/*bp08*/, bp06, bp04)
register int si;
unsigned char bp06;
unsigned char bp04;
{
	register unsigned char *di;

	sound(0);
	/*-- out of bounds=>exiting*/
	if((unsigned)bp06 > 10 || (unsigned)bp04 > 10) {
		COM_TryExit(bp06, bp04);
		return;
	}
	di = &(ViewTiles[bp04*11 + bp06]);
	if(!IsWalkable(*di)) {
		w_Blocked();
		return;
	}
	if(IsSlowTerrain(*di)) {
		w_SlowProgress();
		return;
	}
	Combat._charaX[si] = bp06;
	Combat._charaY[si] = bp04;
	sound(0);
	if(CurMode == MOD_COM_ROOM)
		RoomTriggers(bp06, bp04);
}

/*North [combat]*/
/*C_7B89*/COM_North()
{
	u4_puts(/*D_23B7*/"North\n");
	COM_MoveChara(activeChara, Combat._charaX[activeChara], Combat._charaY[activeChara] - 1);
}

/*South [combat]*/
/*C_7BAF*/COM_South()
{
	u4_puts(/*D_23BE*/"South\n");
	COM_MoveChara(activeChara, Combat._charaX[activeChara], Combat._charaY[activeChara] + 1);
}

/*East [combat]*/
/*C_7BD5*/COM_East()
{
	u4_puts(/*D_23C5*/"East\n");
	COM_MoveChara(activeChara, Combat._charaX[activeChara] + 1, Combat._charaY[activeChara]);
}

/*West [combat]*/
/*C_7BFD*/COM_West()
{
	u4_puts(/*D_23CB*/"West\n");
	COM_MoveChara(activeChara, Combat._charaX[activeChara] - 1, Combat._charaY[activeChara]);
}
