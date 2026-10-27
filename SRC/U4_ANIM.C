/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

unsigned /*D_1630*/FlowCounter = 0;

/*
 * Animate the flowing tiles (water, lava, force fields) once every speed_info
 * calls, and the campfire on every call.
 */
/*C_34EA*/AnimateTiles()
{
	if(FlowCounter -- == 0) {
		FlowCounter = speed_info - 1;
		Gra_AnimFlow(TIL_DEEP_WATER);
		Gra_AnimFlow(TIL_WATER);
		Gra_AnimFlow(TIL_SHALLOWS);
		Gra_AnimFlow(TIL_LAVA);
		Gra_AnimFlow(TIL_POISON_FIELD);
		Gra_AnimFlow(TIL_FIRE_FIELD);
		Gra_AnimFlow(TIL_SLEEP_FIELD);
		Gra_AnimFlow(TIL_ENERGY_FIELD);
	}
	Gra_AnimCampfire();
}

/*
 * Print label followed by the name of direction dir (DIR_*) on the bottom line
 * of the view frame. In a dungeon, also print the level ("L1".."L8") at the top.
 * The text cursor is preserved.
 */
/*C_353D*/ShowDirection(dir, label)
unsigned dir;
char *label;
{
	unsigned save_x, save_y;

	save_x = txt_X;
	save_y = txt_Y;
	txt_Y = 23;
	txt_X = 7;
	u4_puts(label);
	switch(dir) {
		case DIR_N: u4_puts(" North"); break;
		case DIR_S: u4_puts(" South"); break;
		case DIR_E: u4_puts("  East"); break;
		case DIR_W: u4_puts("  West"); break;
	}
	if(CurMode == MOD_DUNGEON) {
		txt_Y = 0;
		txt_X = 11;
		u4_putc('L');
		u4_putc(Party._z + '1');
	}
	txt_X = save_x;
	txt_Y = save_y;
}

unsigned /*D_1654*/WindCounter = 0;

/*
 * Once every speed_info calls, maybe turn the wind: with a 1/64 chance it
 * rotates a quarter turn either way. Then display it.
 */
/*C_35C7*/UpdateWind()
{
	if(WindCounter-- == 0) {
		WindCounter = speed_info - 1;
		if(!U4_RND1(0xfc))
			WindDir = (U4_RND1(2) + WindDir - 1) & 3;
	}
	ShowDirection(WindDir, "Wind");
}

/*chance (out of 256) for a monster to change frame, by speed_info - 1*/
unsigned /*D_1656*/NpcAnimChance[] = {192, 96, 60, 36, 20, 12, 8};

/*
 * Animate the map's npcs (their _gtile): the two-frame creatures (sea monsters,
 * party classes, townsfolk) toggle between their frames, the four-frame monsters
 * cycle through theirs, others just show their base tile (pirate ships keep
 * their heading). Then animate the tiles and the flags.
 */
/*C_3605*/AnimateNpcs()
{
	register int npc, npc_tile;

	for(npc = 0; npc < 32; npc++) {
		npc_tile = Area._npc._tile[npc];
		if(
			(npc_tile >= TIL_NIXIE && npc_tile <= TIL_TWISTER) ||
			(npc_tile >= TIL_MAGE && npc_tile <= TIL_SHEPHERD) ||
			(npc_tile >= TIL_GUARD && npc_tile <= TIL_LORD_BRITISH)
		) {
			if(U4_RND2(1) != 0)
			if(((speed_info < 8)?NpcAnimChance[speed_info - 1]:0xff) > U4_RND2(0xff))
			Area._npc._gtile[npc] = ((Area._npc._gtile[npc] & 1) | npc_tile) ^ 1;
		} else if(npc_tile >= TIL_RAT) {
			if(((speed_info < 8)?NpcAnimChance[speed_info - 1]:0xff) > U4_RND2(0xff))
			Area._npc._gtile[npc] = ((Area._npc._gtile[npc] + 1) & 3) | npc_tile;
		} else if(npc_tile != TIL_PIRATE_W) {
			Area._npc._gtile[npc] = npc_tile;
		}
	}
	AnimateTiles();
	Gra_AnimFlags();
}

/*
 * Draw the 11x11 ViewTiles on screen, with hit_tile (when set) shown over
 * (hit_x, hit_y), and the blinking box around the active character in combat
 * (CursorBlink counts down: shown for 4 frames, hidden for 9).
 */
/*C_36C7*/DrawView()
{
	register unsigned under;

	under = ViewTiles[hit_y*11 + hit_x];
	if(hit_tile)
		ViewTiles[hit_y*11 + hit_x] = hit_tile;
	Gra_DrawTiles(11, 11, ViewTiles, 4  * 80 + 2);
	if(activeCharaX != -1) {
		register int old_blink;

		old_blink = CursorBlink;
		CursorBlink -= u4_sign(CursorBlink);
		if(CursorBlink == 0) {
			if(old_blink < 0)
				CursorBlink = 4;
			else
				CursorBlink = -9;
		}
		if(CursorBlink > 0)
			Gra_ActiveBox(activeCharaY, activeCharaX);
	}
	ViewTiles[hit_y*11 + hit_x] = under;
}

/*
 * Cast one line-of-sight ray from (x, y) in direction (dx, dy): copy each still
 * black square of the view from Combat._map until an opaque tile is reached.
 * Each step of a diagonal ray also lights the two squares between it and the
 * previous step; each step of a straight ray starts two diagonal rays from the
 * squares on its sides. Recursive.
 */
/*C_376F*/CastRay(dx, dy, x, y)
int dx;
int dy;
unsigned char x;
unsigned char y;
{
	register unsigned idx;
	unsigned map_tile;

	idx = &(ViewTiles[y*11 + x]) - ViewTiles;
	if(x > 10 || y > 10)
		return;
	if(ViewTiles[idx] != TIL_BLACK)
		return;
	map_tile = Combat._map[idx];
	ViewTiles[idx] = map_tile;
	/*opaque tiles*/
	if(
		map_tile == TIL_FOREST ||
		map_tile == TIL_MOUNTAINS ||
		map_tile == TIL_BLACK ||
		map_tile == TIL_SECRET_DOOR ||
		map_tile == TIL_WALL
	) return;

	x += dx;
	y += dy;
	CastRay(dx, dy, x, y);
	if(dx & dy) {
		CastRay(dx, dy, x,        y - dy);
		CastRay(dx, dy, x - dx, y);
	} else {
		CastRay((!dx) * dy + dx, dy - (!dy) * dx, dy + x, y - dx);
		CastRay(dx - (!dx) * dy, (!dy) * dx + dy, x - dy, dx + y);
	}
}

/*
 * Line of sight: fill ViewTiles with the squares of Combat._map visible from
 * the center (5, 5), the others being TIL_BLACK.
 */
/*C_38BB*/ComputeVisibility()
{
	register int cell;

	/*-- set all visible squares to TIL_BLACK --*/
	for(cell = 10 * 11 + 10; cell >= 0; cell --)
		ViewTiles[cell] = TIL_BLACK;
	/*-- --*/
	ViewTiles[5*11 + 5] = Combat_MAP(5, 5);/*display character's icon*/
/*                 -1   0  +1
    0   1   2   3   4   5   6   7   8   9   a
  +---+---+---+---+---+---+---+---+---+---+---+
0 | . |   |   |   |   | . |   |   |   |   | . |
  +---+---+---+---+---+---+---+---+---+---+---+
1 |   | . |   |   |   | . |   |   |   | . |   |
  +---+---+---+---+---+---+---+---+---+---+---+
2 |   |   | . |   |   | . |   |   | . |   |   |
  +---+---+---+---+---+---+---+---+---+---+---+
3 |   |   |   | . |   | . |   | . |   |   |   |
  +---+---+---+---+---+---+---+---+---+---+---+
4 |   |   |   |   |4 4|5 4|6 4|   |   |   |   | -1
  +---+---+---+---+---+---+---+---+---+---+---+
5 | . | . | . | . |4 5|   |6 5| . | . | . | . |  0
  +---+---+---+---+---+---+---+---+---+---+---+
6 |   |   |   |   |4 5|5 6|6 6|   |   |   |   | +1
  +---+---+---+---+---+---+---+---+---+---+---+
7 |   |   |   | . |   | . |   | . |   |   |   |
  +---+---+---+---+---+---+---+---+---+---+---+
8 |   |   | . |   |   | . |   |   | . |   |   |
  +---+---+---+---+---+---+---+---+---+---+---+
9 |   | . |   |   |   | . |   |   |   | . |   |
  +---+---+---+---+---+---+---+---+---+---+---+
a | . |   |   |   |   | . |   |   |   |   | . |
  +---+---+---+---+---+---+---+---+---+---+---+*/
	CastRay( 0, -1, 5, 4);/*UP*/
	CastRay( 0,  1, 5, 6);/*DOWN*/
	CastRay(-1,  0, 4, 5);/*LEFT*/
	CastRay( 1,  0, 6, 5);/*RIGHT*/
	CastRay( 1,  1, 6, 6);/*DOWN RIGHT*/
	CastRay( 1, -1, 6, 4);/*UP RIGHT*/
	CastRay(-1,  1, 4, 6);/*DOWN LEFT*/
	CastRay(-1, -1, 4, 4);/*UP LEFT*/
}

/*Put the npcs that are within 5 squares of the party into the 11x11 Combat._map.*/
/*C_3960*/PlaceNpcsInView()
{
	register int npc;
	unsigned char off_x, off_y;

	for(npc = 31; npc >= 0; npc --) {

		off_x = Area._npc._x[npc] - Party._x + 5;
		off_y = Area._npc._y[npc] - Party._y + 5;
		if(Area._npc._gtile[npc] && off_x < 11 && off_y < 11)
			Combat_MAP(off_y, off_x) = Area._npc._gtile[npc];
	}
}

/*
 * Complete the 11x11 Combat._map built from the map (npcs, open moongate),
 * note the tiles around the party (tile_cur, tile_north, ...), put the party
 * in the center, apply the line of sight if needed, and draw.
 */
/*C_39BA*/FinishView()
{
	register int cell;

	PlaceNpcsInView();
	if(MoongateTile) {
		unsigned char gate_x, gate_y;

		gate_x = OpenGateX - Party._x + 5;
		gate_y = OpenGateY - Party._y + 5;
		if(gate_x < 11 && gate_y < 11)
			Combat_MAP(gate_y, gate_x) = MoongateTile;
	}
	tile_cur   = Combat_MAP(5, 5);
	tile_north = Combat_MAP(4, 5);
	tile_south = Combat_MAP(6, 5);
	tile_east  = Combat_MAP(5, 6);
	tile_west  = Combat_MAP(5, 4);
	Combat_MAP(5, 5) = Party._tile;
	if(LineOfSight)
		ComputeVisibility();
	else {
		for(cell = 10 * 11 + 10; cell >= 0; cell --)
			ViewTiles[cell] = Combat._map[cell];
	}
	DrawView();
}

/*
 * The moons: MoonPhaseCounter's top 2 bits count 4 sub-steps; after each full
 * turn Trammel advances by 2 and Felucca by 6 (their top 3 bits are the phase).
 */
unsigned char /*D_1664*/MoonSubStep = 0;
unsigned char /*D_1665*/TrammelCounter = 0;/*trammel counter*/
unsigned char /*D_1666*/FeluccaCounter = 0;/*felucca counter*/

/*
 * Set OpenGateX/OpenGateY to the moongate of Trammel's current phase and
 * return the sub-step (0-3) used as the moongate's animation frame.
 */
/*C_3A4F*/UpdateOpenGate()
{
	OpenGateX = MoongateX[(TrammelCounter & 0xe0) >> 5];
	OpenGateY = MoongateY[(TrammelCounter & 0xe0) >> 5];
	return ((MoonSubStep & 0xc0) >> 6);
}

unsigned /*D_1668*/MoonTick = 0;

/*
 * Advance the moons every speed_info / 2 + 1 calls (the moongate grows during the
 * first 4 sub-steps of a Trammel phase and shrinks during the last 4), then
 * display Trammel's and Felucca's phases at the top of the view.
 */
/*C_3A80*/UpdateMoons()
{
	unsigned char save_x, save_y;

	if(MoonTick-- == 0) {
		MoonTick = speed_info / 2;
		MoonSubStep += 0x40;
		if(MoonSubStep == 0) {
			TrammelCounter += 2;
			FeluccaCounter += 6;
		}
		Party._trammel = TrammelCounter >> 5;
		Party._felucca = FeluccaCounter >> 5;
		if((TrammelCounter & 0x1f) == 0) {
			MoongateTile = UpdateOpenGate() + TIL_MOONGATE_1;
		} else if((TrammelCounter & 0x1f) == 0x1e){
			MoongateTile = (UpdateOpenGate() ^ 3) + TIL_MOONGATE_1;
		}
	}
	save_x = txt_X;
	save_y = txt_Y;
	txt_Y = 0;
	txt_X = 11;
	Gra_PutChar((((TrammelCounter >> 5) - 1) & 7) + 0x14);
	txt_X ++;
	Gra_PutChar((((FeluccaCounter >> 5) - 1) & 7) + 0x14);
	txt_X = save_x;
	txt_Y = save_y;
}

/*Draw the outdoors view: the 11x11 squares around WinPos, then the npcs, ...*/
/*C_3B35*/DrawOutdoors()
{
	register unsigned cell;
	int view_y, view_x;

	AnimateNpcs();/*animate sprites*/
	UpdateMoons();/*display tramel and feluca*/
	cell = 10 * 11 + 10;
	for(view_y = 10; view_y >= 0; view_y --) {
		for(view_x = 10; view_x >= 0; view_x --) {
			Combat._map[cell] = Area._map.x32x32[view_y+WinPos.y-5][view_x+WinPos.x-5];
			cell --;
		}
	}
	FinishView();/*display 11x11 icons zone*/
}

/*
 * Draw the view inside a towne/castle: the squares around the party, outside
 * of the 32x32 map shown as grass; then the npcs, ... and the wind.
 */
/*C_3B83*/DrawTown()
{
	register int dx, dy;

	AnimateNpcs();
	for(dy = -5; dy <= 5; dy ++) {
		for(dx = -5; dx <= 5; dx ++) {
			if(
				((int)Party._x + dx < 0) || ((int)Party._x + dx > 0x1f) ||
				((int)Party._y + dy < 0) || ((int)Party._y + dy > 0x1f)
			) {
				Combat_MAP(dy+5, dx+5) = TIL_GRASS;
			} else {
				Combat_MAP(dy+5, dx+5) = Area._map.x32x32[Party._y + dy][Party._x + dx];
			}
		}
	}
	FinishView();/*display 11x11 icons zone*/
	UpdateWind();/*update/display wind*/
}

unsigned /*D_166A*/BalloonTick = 0;

/*Once every speed_info + 1 calls, the flying balloon drifts with the wind (one move).*/
/*C_3C08*/DriftBalloon()
{
	if(BalloonTick -- == 0) {
		BalloonTick = speed_info;
		Party._moves ++;
		switch(WindDir) {
			case DIR_N: ScrollSouth(); break;
			case DIR_S: ScrollNorth(); break;
			case DIR_E: ScrollWest(); break;
			case DIR_W: ScrollEast(); break;
		}
	}
}

/*random mask for the combat animations, by speed_info - 1*/
unsigned /*D_166C*/CombatAnimMask[] = {1,3,3,7,7,7,7,15};

/*
 * Draw the combat (or shrine) view: Combat._map, with the animated fighters and
 * party members (unless in a shrine).
 * A mimic still showing as a chest stays a chest.
 */
/*C_3C54*/DrawCombat()
{
	register int n;
	unsigned frame_mask;
	unsigned char *p_gtile;
	unsigned char *p_cell;
	unsigned char *p_tile;

	frame_mask = (speed_info < 8)?CombatAnimMask[speed_info - 1]:15;
	AnimateTiles();
	for(n = 10 * 11 + 10; n >= 0; n--)
		ViewTiles[n] = Combat._map[n];
	if(CurMode != MOD_SHRINE) {
		for(n = 15; n >= 0; n--) {
			p_tile = &(Fighters._tile[n]);
			if(*p_tile == 0)
				continue;
			p_cell = &(ViewTiles[Combat._npcY[n]*11 + Combat._npcX[n]]);
			p_gtile = &(Fighters._gtile[n]);
			if(Fighters._sleeping[n])
				*p_cell = *p_gtile;
			else if(*p_tile >= TIL_RAT) {
				if(*p_tile == TIL_MIMIC && *p_gtile == TIL_CHEST || U4_RND1(frame_mask) == 0) {
					*p_cell = *p_gtile;
				} else if(U4_RND1(frame_mask) == 0) {
					*p_gtile = ((*p_gtile + 1) & 3) | *p_tile;
					*p_cell = *p_gtile;
				} else {
					*p_cell = *p_gtile;
				}
			} else if(*p_tile == TIL_SLEEPING) {
				*p_cell = *p_gtile = TIL_SLEEPING;
			} else {
/*C_3D35*/
/*note: I had a hard time figuring out how to get this statement right*/
				*p_cell = *p_gtile ^= U4_RND1(U4_RND2(1));
			}
		}
		for(n = Party._members - 1; n >= 0; n --) {
			unsigned char *p_view;
			unsigned char *p_chtile;
			unsigned char chtile;

			p_chtile = &(Fighters._chtile[n]);
			chtile = *p_chtile;
			if(chtile == 0)
				continue;
			p_view = &(ViewTiles[Combat._charaY[n]*11 + Combat._charaX[n]]);
			if(chtile == TIL_SLEEPING) {
				*p_view = CharaAnimTile[n] = chtile;
				continue;
			}
			*p_view = (CharaAnimTile[n] ^= !U4_RND1(frame_mask));
		}
	}
	DrawView();
}

/*Redraw the game view for the current mode (CurMode).*/
/*C_3DC8*/RefreshView()
{
	if(CurMode == MOD_VISION) {
		cursor_delay = 233;
		return;
	}
	cursor_delay = 0;
	if(CurMode == MOD_OUTDOORS) {
		if(Party._tile == TIL_BALLOON && Party._fly_light)
			DriftBalloon();
		UpdateWind();/*update/display wind*/
		DrawOutdoors();
	} else if(CurMode == MOD_BUILDING) {
		DrawTown();
	} else if(CurMode == MOD_DUNGEON) {
		ShowDirection(Party._dir, "DIR:");
		AnimateTiles();
		DNG_Render();
		RenderSprites3D();
	} else {/*Combat*/
		DrawCombat();
	}
}
