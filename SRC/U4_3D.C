/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*
 * The first-person dungeon view. The view is drawn one square at a time from the
 * party's square (depth 0) to 3 squares ahead (depth 3; the walls also use depth 4).
 * The tables below give, for each depth, the screen coordinates (x 0x08-0xB7,
 * y 0x08-0xB7, centre 0x5F/0x60) of the elements of a square.
 * Dungeon tiles: high nibble = kind (0x00 corridor, 0x10/0x20/0x30 ladder up/down/
 * both, 0x40 chest, 0x50/0x60 hole in the ceiling/floor, 0x70 magic ball, 0x80 trap,
 * 0x90 fountain, 0xA0 field, 0xB0 altar, 0xC0 door, 0xD0 room entrance, 0xE0 secret
 * door, 0xF0 wall); for the walkable ones the low nibble is the monster there.
 */

/*the near edge of the square at depth n (its far edge is n + 1)*/
unsigned char /*D_2D74*/Wall_x0[] = {0x08,0x28,0x48,0x58,0x5F,0x00};
unsigned char /*D_2D7A*/Wall_x1[] = {0xB7,0x97,0x77,0x67,0x60,0x00};
unsigned char /*D_2D80*/Wall_y0[] = {0x08,0x28,0x48,0x58,0x5F,0x00};
unsigned char /*D_2D86*/Wall_y1[] = {0xB7,0x97,0x77,0x67,0x60,0x00};

/*doors in the side walls*/
unsigned char /*D_2D8C*/LDoor_x0[]  = {0x08,0x30,0x4C,0x5A};
unsigned char /*D_2D90*/LDoor_x1[]  = {0x18,0x40,0x54,0x5C};
unsigned char /*D_2D94*/RDoor_x0[]  = {0xB7,0x8F,0x72,0x64};
unsigned char /*D_2D98*/RDoor_x1[]  = {0xA7,0x7F,0x6A,0x62};
unsigned char /*D_2D9C*/Door_y0[]  = {0x34,0x48,0x56,0x5A};
unsigned char /*D_2DA0*/Door_y0p[] = {0x3D,0x51,0x5B,0x5F};
unsigned char /*D_2DA4*/Door_y1[]  = {0xB7,0x8F,0x73,0x65};
unsigned char /*D_2DA8*/Door_y1p[] = {0xA7,0x7F,0x6B,0x62};

/*holes in the ceiling (Hole_y) and floor (FloorHole_y), and ladders*/
unsigned char /*D_2DAC*/Hole_y0[] = {0x08,0x30,0x4C,0x5A};
unsigned char /*D_2DB0*/Hole_y1[] = {0x18,0x40,0x54,0x5D};
unsigned char /*D_2DB4*/FloorHole_y0[] = {0xB7,0x8F,0x73,0x65};
unsigned char /*D_2DB8*/FloorHole_y1[] = {0xA7,0x7F,0x6B,0x62};
unsigned char /*D_2DBC*/HoleL_x0[] = {0x35,0x48,0x55,0x5C};
unsigned char /*D_2DC0*/HoleL_x1[] = {0x3C,0x50,0x5A,0x5D};
unsigned char /*D_2DC4*/HoleR_x0[] = {0x8B,0x78,0x6A,0x62};
unsigned char /*D_2DC8*/HoleR_x1[] = {0x83,0x70,0x65,0x61};
unsigned char /*D_2DCC*/Ladder_x0[] = {0x48,0x54,0x5C,0x5F};
unsigned char /*D_2DD0*/Ladder_x1[] = {0x77,0x6B,0x63,0x60};
unsigned char /*D_2DD4*/LadderUp_y1[] = {0x10,0x38,0x50,0x5D};/*up ladder rung 1 y*/
unsigned char /*D_2DD8*/LadderUp_y2[] = {0x30,0x48,0x56,0x5E};/*up ladder rung 2 y*/
unsigned char /*D_2DDC*/LadderUp_y3[] = {0x50,0x58,0x5C,0x5F};/*up ladder rung 3 y*/
unsigned char /*D_2DE0*/LadderDown_y1[] = {0xAF,0x87,0x6F,0x62};/*down ladder rung 1 y*/
unsigned char /*D_2DE4*/LadderDown_y2[] = {0x8F,0x77,0x69,0x61};/*down ladder rung 2 y*/
unsigned char /*D_2DE8*/LadderDown_y3[] = {0x6F,0x67,0x63,0x60};/*down ladder rung 3 y*/

/*countdowns (reloaded from speed_info) for the trap flicker and the monster frames*/
int16 /*D_2DEC*/PitAnimTick = 0;
int16 /*D_2DEE*/MonsterAnimTick = 0;

/*D_8CE8*/static unsigned char STA_A/*tile 2 squares ahead*/;
/*D_8CE9*/static unsigned char STA_B/*tile 3 squares ahead*/;
/*the state of the last full redraw (Render3DNow), see DNG_Render*/
/*D_8CEA*/static int16 STA_C;/*(Party._fly_light == 0): no light*/
/*D_8CEC*/static int16 STA_D;/*Party._z*/
/*D_8CEE*/static int16 STA_E;/*current rendering depth*/
/*D_8CF0*/static unsigned char STA_F;/*Party._x*/
/*D_8CF2*/static uint16 STA_G;/*Party._dir*/
/*D_8CF4*/static unsigned char STA_H;/*Party._y*/
/*D_8CF5*/static unsigned char STA_I;/*tile_cur*/
/*D_8CF6*/static unsigned char STA_J;/*tile_forward*/
/*D_8CF7*/static unsigned char STA_K;/*tile_forward2*/
/*D_8CF8*/static unsigned char STA_L;/*tile_forward3*/
/*The statics keep their address-like names: Microsoft C 4.00 lays out a module's
 uninitialised statics in the order of its symbol table, which depends on the names.
 Renaming them would move them; the readable names are macros.*/
#define tile_forward2 STA_A
#define tile_forward3 STA_B
#define Saved_dark STA_C
#define Saved_z STA_D
#define Depth STA_E
#define Saved_x STA_F
#define Saved_dir STA_G
#define Saved_y STA_H
#define Saved_tile STA_I
#define Saved_forward STA_J
#define Saved_forward2 STA_K
#define Saved_forward3 STA_L
/*the static of RenderSquare3D*/
#define MonsterFrame D_8CFA

/*
 * Draw a force field across the near edge of the square at the current depth; its
 * kind (poison/energy/fire/sleep) is the low 2 bits of tile.
 */
/*C_A7F2*/DrawField3D(tile)
unsigned char tile;
{
	Gra_FillTiles(
		(Wall_y1[Depth] - Wall_y0[Depth] + 1) / 16,
		(Wall_x1[Depth] - Wall_x0[Depth] + 1) / 16,
		(tile & 3) | TIL_POISON_FIELD,
		Wall_y0[Depth],
		Wall_x0[Depth] >> 2
	);
}

/*
 * Draw a slanted line from (x0, y0) to (x1, y1) (Bresenham), ORing colour 3.
 * Called by Gra_Line (U4_GRAPH.ASM) for the lines that are neither horizontal nor
 * vertical.
 */
/*C_A842*/DrawLine(x0, y0, x1, y1)
int16 x0;
int16 y0;
int16 x1;
int16 y1;
{
	int16 y, stepX, stepY, acc, x;

	x = x0;
	y = y0;
	stepX = u4_sign(x1 - x0);
	stepY = u4_sign(y1 - y0);

	if(u4_abs(x0 - x1) < u4_abs(y0 - y1)) {
		acc = u4_abs(y1 - y0) / 2;
		do {
			Gra_DotOr(y, x, 3);
			y += stepY;
			acc -= u4_abs(x1 - x0);
			if(acc <= 0) {
				x += stepX;
				acc += u4_abs(y1 - y0);
			}
		} while(y != y1);
	} else {
		acc = u4_abs(x1 - x0) / 2;
		do {
			Gra_DotOr(y, x, 3);
			x += stepX;
			acc -= u4_abs(y1 - y0);
			if(acc <= 0) {
				y += stepY;
				acc += u4_abs(x1 - x0);
			}
		} while(x != x1);
	}
	Gra_DotOr(y, x, 3);
}

/*
 * Draw a hole in the ceiling of the square at the current depth.
 */
/*C_A94A*/DrawCeilingHole()
{
	register int16 d;

	d = Depth;
	Gra_Line(HoleL_x0[d],   Hole_y0[d], HoleL_x1[d],   Hole_y1[d], 3);
	Gra_Line(HoleL_x0[d]+1, Hole_y0[d], HoleL_x1[d]+1, Hole_y1[d], 3);
	Gra_Line(HoleL_x1[d]+1, Hole_y1[d], HoleR_x1[d],   Hole_y1[d], 3);
	Gra_Line(HoleR_x1[d],   Hole_y1[d], HoleR_x0[d],   Hole_y0[d], 3);
	Gra_Line(HoleR_x1[d]-1, Hole_y1[d], HoleR_x0[d]-1, Hole_y0[d], 3);
	Gra_Line(HoleR_x0[d]-1, Hole_y0[d], HoleL_x0[d]+1, Hole_y0[d], 3);
}

/*
 * Draw a hole in the floor of the square at the current depth.
 */
/*C_AA29*/DrawFloorHole()
{
	register int16 d;

	d = Depth;
	Gra_Line(HoleL_x0[d],   FloorHole_y0[d], HoleL_x1[d],   FloorHole_y1[d], 3);
	Gra_Line(HoleL_x0[d]+1, FloorHole_y0[d], HoleL_x1[d]+1, FloorHole_y1[d], 3);
	Gra_Line(HoleL_x1[d]+1, FloorHole_y1[d], HoleR_x1[d],   FloorHole_y1[d], 3);
	Gra_Line(HoleR_x1[d],   FloorHole_y1[d], HoleR_x0[d],   FloorHole_y0[d], 3);
	Gra_Line(HoleR_x1[d]-1, FloorHole_y1[d], HoleR_x0[d]-1, FloorHole_y0[d], 3);
	Gra_Line(HoleR_x0[d]-1, FloorHole_y0[d], HoleL_x0[d]+1, FloorHole_y0[d], 3);
}

/*
 * The object of dungeon tile in the square at the current depth. Returns the
 * tile of the sprite to draw (TIL_SHIP_RAIL for a field, which is drawn here), 0 if
 * nothing (traps sometimes flicker as a hole: a hint), -1 for doors, rooms and walls.
 * For fields, cursor_delay (the cursor delay, see u_delay) is adjusted to the time the
 * redraw takes.
 */
/*C_AB08*/Special3D(tile)
unsigned char tile;
{
	if(PitAnimTick-- == 0)
		PitAnimTick = speed_info - 1;
	switch(tile & 0xf0) {
		default: return ((tile & 0xf0) < 0xc0)?0:-1;
		case 0x40: return TIL_CHEST;
		case 0x70: return TIL_MAGIC_BOLT;
		case 0x80:
			if(PitAnimTick == 1 && U4_RND1(0x3f) == 0 && (tile & 0xf)) {
				if((tile & 0x0f) >= 8)
					DrawFloorHole();
				else
					DrawCeilingHole();
			}
		return 0;
		case 0x90: return TIL_SHALLOWS;
		case 0xa0:
			DrawField3D((tile & 3) | TIL_POISON_FIELD);
			switch(Depth) {
				case 0: cursor_delay = 0; break;
				case 1: cursor_delay = 1; break;
				case 2: cursor_delay = 6; break;
				case 3: cursor_delay = 10; break;
			}
		return TIL_SHIP_RAIL;
		case 0xb0: return TIL_ALTAR;
	}
	return -1;
}

/*
 * Is there a monster (the low nibble) on this dungeon tile? Traps, fountains,
 * fields and doors/rooms/walls have other uses for the low nibble.
 */
/*C_ABE5*/IsMonster3D(tile)
unsigned char tile;
{
	register int16 kind;

	kind = tile & 0xf0;
	if(
		kind == 0x80 ||
		kind == 0x90 ||
		kind == 0xa0 ||
		kind >= 0xd0
	) return 0;
	if(!(tile & 0x0f))
		return 0;
	return 1;
}

/*
 * Draw tile as a sprite in the square at the current depth, zoomed (x3, x2, x1 or
 * shrunk), then wait a while, shorter when near (the zoomed drawings take longer).
 */
/*C_AC23*/DrawSprite3D(tile)
unsigned char tile;
{
	register int16 line;
	int32 pause;

	cursor_delay = 0;
	switch(Depth) {
		case 0:
			for(line = 0; line < 48; line++)
				Gra_ZoomTileRow(tile, line / 3, line + 0x70, 0x14, 2);
			pause = 1000 * speed_info;
			while(--pause);
		break;
		case 1:
			for(line = 0; line < 32; line++)
				Gra_ZoomTileRow(tile, line >> 1, line + 0x68, 0x14, 2);
			pause = 1200 * speed_info;
			while(--pause);
		break;
		case 2:
			Gra_FillTiles(1, 1, tile, 0x60, 0x16);
			pause = 2400 * speed_info;
			while(--pause);
		break;
		case 3:
			for(line = 0; line < 8; line ++)
				Gra_ZoomTileRow(tile, line << 1, line + 0x60, 0x16, 1);
			pause = 3000 * speed_info;
			while(--pause);
		break;
	}
}

/*
 * Draw the monster or object of dungeon tile, at the given depth. Returns 0 when
 * the squares behind are hidden (by a sprite, a room entrance, a wall, or a door
 * other than the one the party stands in), else 1.
 */
/*C_AD6A*/RenderSquare3D(tile, depth)
unsigned char tile;
int16 depth;
{
	static int16 D_8CFA;
	int16 spriteTile;

	Depth = depth;
	if(MonsterAnimTick-- == 0) {
		MonsterAnimTick = speed_info - 1;
		MonsterFrame = U4_RND1(3);
	}
	if(IsMonster3D(tile)) {
		DrawSprite3D(TIL_RAT + ((tile & 0xf) << 2) - 4 + MonsterFrame);
		return 0;
	}
	if((spriteTile = Special3D(tile)) > 0) {
		if(spriteTile != TIL_SHIP_RAIL)
			DrawSprite3D(spriteTile);
		return 0;
	}
	if(spriteTile != 0) {
		if(Depth != 0 || tile >= 0xd0)
			return 0;
	}
	return 1;
}

/*
 * Draw the sprites (monsters, objects, fields) of the 4 squares ahead, from the
 * nearest, until one hides the rest; without light, only clears the view.
 */
/*C_ADEF*/RenderSprites3D()
{
	if(Party._fly_light == 0) {
		Gra_ClearView();
		return;
	}
	if(!RenderSquare3D(tile_cur, 0))
		return;
	if(!RenderSquare3D(tile_forward, 1))
		return;
	if(!RenderSquare3D(tile_forward2, 2))
		return;
	RenderSquare3D(tile_forward3, 3);
}

/*
 * Read the dungeon tiles behind the party, under it, and 1, 2 and 3 squares ahead.
 */
/*C_AE41*/DNG_LookAround()
{
	unsigned char x, y;

	/*back*/
	x = DNG_X_m(Party._x, Party._dir);
	y = DNG_Y_m(Party._y, Party._dir);
	tile_back = Area._map.x8x8x8[Party._z][y][x];
	/*current tile*/
	tile_cur = Area._map.x8x8x8[Party._z][Party._y][Party._x];
	/*forward*/
	x = DNG_X_p(Party._x, Party._dir);
	y = DNG_Y_p(Party._y, Party._dir);
	tile_forward = Area._map.x8x8x8[Party._z][y][x];
	/*2 and 3 squares ahead*/
	x = DNG_X_p(x, Party._dir);
	y = DNG_Y_p(y, Party._dir);
	tile_forward2 = Area._map.x8x8x8[Party._z][y][x];

	x = DNG_X_p(x, Party._dir);
	y = DNG_Y_p(y, Party._dir);
	tile_forward3 = Area._map.x8x8x8[Party._z][y][x];
}

/*
 * Map coordinates (*getX, *getY) of the square at the current depth ahead of the
 * party, in column side (-1 left, 0 straight ahead, +1 right), according to the
 * party's position and facing (the level wraps around).
 */
/*C_AF36*/DNG_SquareAt(getX, getY, side)
register int16 *getX;
register int16 *getY;
int16 side;
{
	*getX = Party._x;
	*getY = Party._y;
	switch(Party._dir) {
		case DIR_N:
			*getY -= Depth; *getY &= 7;
			*getX += side;   *getX &= 7;
		break;
		case DIR_S:
			*getY += Depth; *getY &= 7;
			*getX -= side;   *getX &= 7;
		break;
		case DIR_E:
			*getX += Depth; *getX &= 7;
			*getY += side;   *getY &= 7;
		break;
		case DIR_W:
			*getX -= Depth; *getX &= 7;
			*getY -= side;   *getY &= 7;
		break;
	}
}

#define VERTICAL(x, y0, y1, c) Gra_Line((unsigned char)(x), (y0), (unsigned char)(x), (y1), (c));
#define HORIZONTAL(x0, y, x1, c) Gra_Line((unsigned char)(x0), (y), (unsigned char)(x1), (y), (c));

/*
 * Draw a ladder going up (through a hole in the ceiling) at the current depth; seen
 * edge-on when facing north or south.
 */
/*C_AFB5*/DrawLadderUp()
{
	register int16 d;

	d = Depth;
	DrawCeilingHole();
	if(Party._dir == DIR_N || Party._dir == DIR_S) {
		VERTICAL(0x5f,   Hole_y0[d], 0x5f, 3);
		VERTICAL(0x5f+1, Hole_y0[d], 0x5f, 3);
	} else {
		/*left stile*/
		VERTICAL(Ladder_x0[d],   Hole_y0[d], 0x5e, 3);
		VERTICAL(Ladder_x0[d]+1, Hole_y0[d], 0x5e, 3);
		/*right stile*/
		VERTICAL(Ladder_x1[d],   Hole_y0[d], 0x5e, 3);
		VERTICAL(Ladder_x1[d]-1, Hole_y0[d], 0x5e, 3);
		/*rungs*/
		HORIZONTAL(Ladder_x0[d], LadderUp_y1[d], Ladder_x1[d], 3);
		HORIZONTAL(Ladder_x0[d], LadderUp_y2[d], Ladder_x1[d], 3);
		HORIZONTAL(Ladder_x0[d], LadderUp_y3[d], Ladder_x1[d], 3);
	}
}

/*
 * Draw a ladder going down (through a hole in the floor) at the current depth; seen
 * edge-on when facing north or south.
 */
/*C_B0E9*/DrawLadderDown()
{
	register int16 d;

	d = Depth;
	DrawFloorHole();
	if(Party._dir == DIR_N || Party._dir == DIR_S) {
		VERTICAL(0x5f,   FloorHole_y0[d], 0x5f, 3);
		VERTICAL(0x5f+1, FloorHole_y0[d], 0x5f, 3);
	} else {
		/*left stile*/
		VERTICAL(Ladder_x0[d],   0x5f, FloorHole_y0[d], 3);
		VERTICAL(Ladder_x0[d]+1, 0x5f, FloorHole_y0[d], 3);
		/*right stile*/
		VERTICAL(Ladder_x1[d],   0x5f, FloorHole_y0[d], 3);
		VERTICAL(Ladder_x1[d]-1, 0x5f, FloorHole_y0[d], 3);
		/*rungs*/
		HORIZONTAL(Ladder_x0[d], LadderDown_y1[d], Ladder_x1[d], 3);
		HORIZONTAL(Ladder_x0[d], LadderDown_y2[d], Ladder_x1[d], 3);
		HORIZONTAL(Ladder_x0[d], LadderDown_y3[d], Ladder_x1[d], 3);
	}
}

#undef VERTICAL
#undef HORIZONTAL

/*Gra_DrawQuad(
	x0, x1,      the two vertical edges
	y0, y1,      top and bottom at x0
	y0p, y1p,    top and bottom at x1
	col0, col1   colours
);
The walls facing north/south and those facing east/west get different colours.*/

/*the wall (or door) across the near edge of the square at the current depth*/
/*C_B21D*/DrawFrontWall()
{
	Gra_DrawQuad(
		Wall_x0[Depth], Wall_x1[Depth] + 1,
		Wall_y0[Depth], Wall_y1[Depth],
		Wall_y0[Depth], Wall_y1[Depth],
		(Party._dir & 1) + 1, 0
	);
}

/*the far wall of the open square left of the current depth, seen through the opening*/
/*C_B25E*/DrawLeftFarWall()
{
	Gra_DrawQuad(
		Wall_x0[Depth],   Wall_x0[Depth+1],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		(Party._dir & 1) + 1, 0
	);
}

/*the far wall of the open square right of the current depth*/
/*C_B29C*/DrawRightFarWall()
{
	Gra_DrawQuad(
		Wall_x1[Depth],   Wall_x1[Depth+1],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		(Party._dir & 1) + 1, 0
	);
}

/*the side wall on the left of the square at the current depth*/
/*C_B2DA*/DrawLeftSideWall()
{
	Gra_DrawQuad(
		Wall_x0[Depth],   Wall_x0[Depth+1],
		Wall_y0[Depth],   Wall_y1[Depth],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		!(Party._dir & 1) + 1, 0
	);
}

/*the side wall on the right of the square at the current depth*/
/*C_B31B*/DrawRightSideWall()
{
	Gra_DrawQuad(
		Wall_x1[Depth],   Wall_x1[Depth+1],
		Wall_y0[Depth],   Wall_y1[Depth],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		!(Party._dir & 1) + 1, 0
	);
}

/*the opening of a door drawn by DrawFrontWall*/
/*C_B35C*/DrawFrontDoor()
{
	Gra_DrawQuad(
		Wall_x0[Depth+1], Wall_x1[Depth+1],
		Wall_y0[Depth+1], Wall_y1[Depth],
		Wall_y0[Depth+1], Wall_y1[Depth],
		!Depth?0:3, !Depth?3:0
	);
}

/*a door in the left side wall*/
/*C_B3AE*/DrawLeftDoor()
{
	Gra_DrawQuad(
		LDoor_x0[Depth], LDoor_x1[Depth],
		Door_y0[Depth],  Door_y1[Depth],
		Door_y0p[Depth], Door_y1p[Depth],
		3, 0
	);
}

/*a door in the right side wall*/
/*C_B3E3*/DrawRightDoor()
{
	Gra_DrawQuad(
		RDoor_x0[Depth], RDoor_x1[Depth],
		Door_y0[Depth],  Door_y1[Depth],
		Door_y0p[Depth], Door_y1p[Depth],
		3, 0
	);
}

/*
 * Draw what is on the right at the current depth: the wall of the next square seen
 * beyond an opening, or a side wall (with a door for doors and rooms).
 */
/*C_B418*/DrawRightWalls()
{
	int16 y, x;

	DNG_SquareAt(&x, &y, 1);
	switch(Area._map.x8x8x8[Party._z][y][x] >> 4) {
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb: DrawRightFarWall(); break;
		case 0xc:
		case 0xd: DrawRightSideWall(); DrawRightDoor(); break;
		case 0xe:
		case 0xf: DrawRightSideWall(); break;
	}
}

/*
 * Same as DrawRightWalls for the left side, then the right side.
 */
/*C_B47A*/DrawLeftWalls()
{
	int16 y, x;

	DNG_SquareAt(&x, &y, -1);
	switch(Area._map.x8x8x8[Party._z][y][x] >> 4) {
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb: DrawLeftFarWall(); break;
		case 0xc:
		case 0xd: DrawLeftSideWall(); DrawLeftDoor(); break;
		case 0xe:
		case 0xf: DrawLeftSideWall(); break;
	}
	DrawRightWalls();
}

/*
 * Draw the walls, doors, ladders and holes of the view, from the party's square to
 * 3 squares ahead, stopping at the first door (other than the one the party stands
 * in), room entrance, field or wall; then the wall 4 squares ahead if there is one.
 */
/*C_B4DF*/Render3D()
{
	int16 x, y;

	x = Party._x;
	y = Party._y;
	for(Depth = 0; Depth <= 3; Depth ++) {
		DNG_SquareAt(&x, &y, 0);
		switch(Area._map.x8x8x8[Party._z][y][x] & 0xf0) {
			case 0x30: DrawLadderDown();/*no break: up and down*/
			case 0x10: DrawLadderUp();
			case 0x00:
			case 0x40:
			case 0x70:
			case 0x80:
			case 0x90:
			case 0xb0: DrawLeftWalls(); break;
			case 0x20: DrawLadderDown(); DrawLeftWalls(); break;
			case 0x50: DrawCeilingHole(); DrawLeftWalls(); break;
			case 0x60: DrawFloorHole(); DrawLeftWalls(); break;
			case 0xc0: DrawFrontWall(); DrawFrontDoor(); if(Depth) return; break;
			case 0xd0: DrawFrontWall(); DrawFrontDoor(); return;
			case 0xa0: DrawField3D(Area._map.x8x8x8[Party._z][y][x]); return;
			case 0xe0:
			case 0xf0: DrawFrontWall(); return;
		}
	}
	Depth = 4;
	DNG_SquareAt(&x, &y, 0);
	if(
		(Area._map.x8x8x8[Party._z][y][x] & 0xf0) == 0xf0 ||
		(Area._map.x8x8x8[Party._z][y][x] & 0xf0) == 0xe0
	) DrawFrontWall();
}

/*
 * Remember the state the view is drawn for, then clear the view and draw it (only
 * with light).
 */
/*C_B61C*/Render3DNow()
{
	Saved_z = Party._z;
	Saved_x = Party._x;
	Saved_y = Party._y;
	Saved_dir = Party._dir;
	Saved_dark = (Party._fly_light == 0);
	Saved_tile = tile_cur;
	Saved_forward = tile_forward;
	Saved_forward2 = tile_forward2;
	Saved_forward3 = tile_forward3;
	Redraw3D = 0;
	Gra_ClearView();
	if(Party._fly_light)
		Render3D();
}

/*
 * Redraw the dungeon view if something visible changed since the last redraw: the
 * party's position, facing or light, or a tile in view (only the tiles up to the
 * first one that blocks the view count).
 */
/*C_B677*/DNG_Render()
{
	unsigned char kind;

	cursor_delay = (Party._fly_light != 0)?13:1;
	DNG_LookAround();
	if(
		Redraw3D == 0 &&
		Saved_z == Party._z &&
		Saved_x == Party._x &&
		Saved_y == Party._y &&
		Saved_dir == Party._dir &&
		Saved_tile == tile_cur &&
		Saved_dark == (Party._fly_light == 0)
	) {
		kind = tile_cur & 0xf0;
		if(kind == 0xa0 || kind >= 0xd0)
			return;
		if(tile_forward != Saved_forward)
			goto redraw;
		if((kind = tile_forward & 0xf0) == 0xa0 || kind >= 0xc0)
			return;
		if(tile_forward2 != Saved_forward2)
			goto redraw;
		if((kind = tile_forward2 & 0xf0) == 0xa0 || kind >= 0xc0)
			return;
		/*original bug: compares Saved_forward3 with itself (always true) instead
		  of with tile_forward3, so a change 3 squares ahead never forces a redraw*/
		if(Saved_forward3 == Saved_forward3)
			return;
	}
redraw:
	Render3DNow();
}
