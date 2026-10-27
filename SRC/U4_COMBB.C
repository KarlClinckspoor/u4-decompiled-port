/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*----------------------------------------
Whirlpools and twisters
----------------------------------------*/

/*
 * The vortex (NPC slot vortex) destroys every other NPC on its square, playing
 * sound effect snd for each one.
 */
/*C_7821*/SwallowNpcsAt(vortex, snd)
register int16 vortex;
int16 snd;
{
	register int16 npc;

	for(npc = 31; npc >= 0; npc --) {
		if(
			npc != vortex &&
			Area._npc._tile[npc] &&
			Area._npc._x[npc] == Area._npc._x[vortex] &&
			Area._npc._y[npc] == Area._npc._y[vortex]
		) {
			Area._npc._tile[npc] = Area._npc._gtile[npc] = 0;
			RefreshView();
			sound(snd);
		}
	}
}

/*
 * Whirlpool (NPC slot vortex): if it is on the party, the party is damaged and
 * thrown (as a ship heading west) to the fixed position (0x7f,0x4e), in Lock Lake.
 * Then it swallows the NPCs on its square.
 */
/*C_786F*/WhirlpoolContact(vortex)
register int16 vortex;
{
	if(Party._x == Area._npc._x[vortex] && Party._y == Area._npc._y[vortex]) {
		RefreshView();
		hit_y = hit_x = 5;
		hit_tile = TIL_WHIRLPOOL;
		DrawView();
		Party._x = 0x7f;
		Party._y = 0x4e;
		sound(SND_WHIRLPOOL);
		DamageAllParty();
		Party._tile = TIL_SHIP_W;
		hit_tile = 0;
		LoadWorldWindow();
	}
	SwallowNpcsAt(vortex, SND_WHIRLPOOL);
}

/*
 * Twister (NPC slot vortex): if it is on the party, the party is damaged 4 times
 * (resetting hit_tile looks like a leftover from WhirlpoolContact).
 * Then it swallows the NPCs on its square.
 */
/*C_78D1*/TwisterContact(vortex)
register int16 vortex;
{
	if(Party._x == Area._npc._x[vortex] && Party._y == Area._npc._y[vortex]) {
		RefreshView();
		sound(SND_TWISTER);
		DamageAllParty();
		DamageAllParty();
		DamageAllParty();
		DamageAllParty();
		hit_tile = 0;
	}
	SwallowNpcsAt(vortex, SND_TWISTER);
}

/*
 * Outdoors, the whirlpools and twisters among the monsters (NPC slots 0-3) hit the
 * party and the NPCs on their square. (Despite the name, nothing moves here:
 * the vortices move with the other NPCs, in MoveNpcs.)
 */
/*C_7918*/VortexContacts()
{
	register int16 npc;

	if(CurMode != MOD_OUTDOORS)
		return;

	for(npc = 3; npc >= 0; npc--) {
		if(Area._npc._tile[npc] == TIL_WHIRLPOOL)
			WhirlpoolContact(npc);
		else if(Area._npc._tile[npc] == TIL_TWISTER)
			TwisterContact(npc);
	}
}

/*----------------------------------------
Moving in combat
----------------------------------------*/

/*C_794D*/w_SameExit()
{
	u4_puts(/*D_239E*/"All must use same exit!\n");
	sound(SND_BEEP);
}

/*
 * The active character leaves the combat map (position 0x80 = gone).
 * Fleeing a normal combat against evil foes while unhurt (HP at max) costs
 * Valor and Sacrifice.
 */
/*C_7962*/COM_CharaExits()
{
	Combat._charaX[activeChara] = Combat._charaY[activeChara] = 0x80;
	Fighters._chtile[activeChara] = 0;
	sound(SND_FAIL);
	if(CurMode == MOD_COMBAT && !IsNonEvil(CombatFoeTile) && Party.chara[activeChara]._HP[0] == Party.chara[activeChara]._HP[1]) {
		karma_dec(&(Party._valor), 2);
		karma_dec(&(Party._sacri), 2);
	}
}

/*
 * The active character tries to step off the map at (x, y). In a dungeon room
 * the whole party must leave by the same side: the first one to exit sets
 * RoomExitX (left/right side, x = 0xff or 11) or RoomExitY (top/bottom side).
 */
/*C_79C9*/COM_TryExit(x, y)
unsigned char x;
unsigned char y;
{
	if(CurMode == MOD_COM_ROOM) {
		/*-- dungeon room --*/
		/*someone already exited W/E*/
		if(RoomExitX != 0) {
			if(x != (unsigned char)RoomExitX) {
				w_SameExit();
				return;
			}
			COM_CharaExits();
			return;
		}
		/*someone already exited N/S*/
		if(RoomExitY != 0) {
			if(y != (unsigned char)RoomExitY) {
				w_SameExit();
				return;
			}
			COM_CharaExits();
			return;
		}
		/*-- --*/
		if(x > 10)
			RoomExitX = x;
		else
			RoomExitY = y;
	}
	COM_CharaExits();
}

/*
 * A character stepped on (x, y) in a dungeon room: fire the room's triggers there.
 * RoomTriggerData holds 4 triggers of 4 bytes: the tile to place (0: no trigger),
 * the trigger position (x in the high nibble, y in the low one), then two target
 * positions coded the same way, where the tile is placed (unless the target is 0,0).
 */
/*C_7A15*/RoomTriggers(x, y)
unsigned char x;
unsigned char y;
{
	register int16 n;
	int16 row, col;

	for(n = 3; n >= 0; n --) {
		if(RoomTriggerData[(n << 2)]) {
			if(
				(x << 12) == (*(U16 *)(RoomTriggerData+(n << 2)) & 0xf000) &&
				(y <<  8) == (*(U16 *)(RoomTriggerData+(n << 2)) & 0x0f00)
			) {
				row = *(U16 *)(RoomTriggerData+(n << 2)+2) & 0xf;
				if(
					row |
					(col = (*(U16 *)(RoomTriggerData+(n << 2)+2) >> 4) & 0xf)
				) Combat_MAP(row, col) = RoomTriggerData[(n << 2)];
				row = (*(U16 *)(RoomTriggerData+(n << 2)+2) >> 8) & 0xf;
				if(
					row |
					(col = (*(U16 *)(RoomTriggerData+(n << 2)+2) >> 12) & 0xf)
				) Combat_MAP(row, col) = RoomTriggerData[(n << 2)] & 0xff;
			}
		}
	}
}

/*
 * Move character chara to (x, y) in combat: leaving the map is an exit attempt;
 * the destination must be walkable and slow terrain may waste the turn.
 */
/*C_7AE3*/COM_MoveChara(chara, x, y)
register int16 chara;
unsigned char x;
unsigned char y;
{
	register unsigned char *tile;

	sound(SND_STEP);
	/*-- out of bounds=>exiting*/
	if((uint16)x > 10 || (uint16)y > 10) {
		COM_TryExit(x, y);
		return;
	}
	tile = &(ViewTiles[y*11 + x]);
	if(!IsWalkable(*tile)) {
		w_Blocked();
		return;
	}
	if(IsSlowTerrain(*tile)) {
		w_SlowProgress();
		return;
	}
	Combat._charaX[chara] = x;
	Combat._charaY[chara] = y;
	sound(SND_STEP);
	if(CurMode == MOD_COM_ROOM)
		RoomTriggers(x, y);
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
