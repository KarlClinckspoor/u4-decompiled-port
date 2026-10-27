/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

unsigned char /*D_2D74*/Wall_x0[] = {0x08,0x28,0x48,0x58,0x5F,0x00};
unsigned char /*D_2D7A*/Wall_x1[] = {0xB7,0x97,0x77,0x67,0x60,0x00};
unsigned char /*D_2D80*/Wall_y0[] = {0x08,0x28,0x48,0x58,0x5F,0x00};
unsigned char /*D_2D86*/Wall_y1[] = {0xB7,0x97,0x77,0x67,0x60,0x00};

unsigned char /*D_2D8C*/LDoor_x0[]  = {0x08,0x30,0x4C,0x5A};
unsigned char /*D_2D90*/LDoor_x1[]  = {0x18,0x40,0x54,0x5C};
unsigned char /*D_2D94*/RDoor_x0[]  = {0xB7,0x8F,0x72,0x64};
unsigned char /*D_2D98*/RDoor_x1[]  = {0xA7,0x7F,0x6A,0x62};
unsigned char /*D_2D9C*/Door_y0[]  = {0x34,0x48,0x56,0x5A};
unsigned char /*D_2DA0*/Door_y0p[] = {0x3D,0x51,0x5B,0x5F};
unsigned char /*D_2DA4*/Door_y1[]  = {0xB7,0x8F,0x73,0x65};
unsigned char /*D_2DA8*/Door_y1p[] = {0xA7,0x7F,0x6B,0x62};

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

int /*D_2DEC*/PitAnimTick = 0;
int /*D_2DEE*/MonsterAnimTick = 0;

/*TODO*/
/*D_8CE8*/static unsigned char STA_A/*tile_forward_1*/;
/*D_8CE9*/static unsigned char STA_B/*tile_forward_2*/;
/*D_8CEA*/static int STA_C;/*<backup (Party._fly_light == 0)>*/
/*D_8CEC*/static int STA_D;/*<backup Party._z>*/
/*D_8CEE*/static int STA_E;/*current rendering depth*/
/*D_8CF0*/static unsigned char STA_F;/*<backup Party._x>*/
/*D_8CF2*/static unsigned STA_G;/*<backup Party._dir>*/
/*D_8CF4*/static unsigned char STA_H;/*<backup Party._y>*/
/*D_8CF5*/static unsigned char STA_I;/*<backup tile_cur>*/
/*D_8CF6*/static unsigned char STA_J;/*<backup tile_forward>*/
/*D_8CF7*/static unsigned char STA_K;/*<backup tile_forward_1>*/
/*D_8CF8*/static unsigned char STA_L;/*<backup tile_forward_2>*/
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
#define MonsterFrame D_8CFA

/*render force fields?*/
/*C_A7F2*/DrawField3D(bp04)
unsigned char bp04;
{
	Gra_FillTiles(
		(Wall_y1[Depth] - Wall_y0[Depth] + 1) / 16,
		(Wall_x1[Depth] - Wall_x0[Depth] + 1) / 16,
		(bp04 & 3) | 0x44,
		Wall_y0[Depth],
		Wall_x0[Depth] >> 2
	);
}

/*drawLine*/
/*C_A842*/DrawLine(bp0a, bp08, bp06, bp04)
int bp0a;
int bp08;
int bp06;
int bp04;
{
	int loc_A, loc_B, loc_C, loc_D, loc_E;

	loc_E = bp0a;
	loc_A = bp08;
	loc_B = u4_sign(bp06 - bp0a);
	loc_C = u4_sign(bp04 - bp08);

	if(u4_abs(bp0a - bp06) < u4_abs(bp08 - bp04)) {
		loc_D = u4_abs(bp04 - bp08) / 2;
		do {
			Gra_DotOr(loc_A, loc_E, 3);
			loc_A += loc_C;
			loc_D -= u4_abs(bp06 - bp0a);
			if(loc_D <= 0) {
				loc_E += loc_B;
				loc_D += u4_abs(bp04 - bp08);
			}
		} while(loc_A != bp04);
	} else {
		loc_D = u4_abs(bp06 - bp0a) / 2;
		do {
			Gra_DotOr(loc_A, loc_E, 3);
			loc_E += loc_B;
			loc_D -= u4_abs(bp04 - bp08);
			if(loc_D <= 0) {
				loc_A += loc_C;
				loc_D += u4_abs(bp06 - bp0a);
			}
		} while(loc_E != bp06);
	}
	Gra_DotOr(loc_A, loc_E, 3);
}

/*up pit*/
/*C_A94A*/DrawCeilingHole()
{
	register int si;

	si = Depth;
	Gra_Line(HoleL_x0[si],   Hole_y0[si], HoleL_x1[si],   Hole_y1[si], 3);
	Gra_Line(HoleL_x0[si]+1, Hole_y0[si], HoleL_x1[si]+1, Hole_y1[si], 3);
	Gra_Line(HoleL_x1[si]+1, Hole_y1[si], HoleR_x1[si],   Hole_y1[si], 3);
	Gra_Line(HoleR_x1[si],   Hole_y1[si], HoleR_x0[si],   Hole_y0[si], 3);
	Gra_Line(HoleR_x1[si]-1, Hole_y1[si], HoleR_x0[si]-1, Hole_y0[si], 3);
	Gra_Line(HoleR_x0[si]-1, Hole_y0[si], HoleL_x0[si]+1, Hole_y0[si], 3);
}

/*low pit*/
/*C_AA29*/DrawFloorHole()
{
	register int si;

	si = Depth;
	Gra_Line(HoleL_x0[si],   FloorHole_y0[si], HoleL_x1[si],   FloorHole_y1[si], 3);
	Gra_Line(HoleL_x0[si]+1, FloorHole_y0[si], HoleL_x1[si]+1, FloorHole_y1[si], 3);
	Gra_Line(HoleL_x1[si]+1, FloorHole_y1[si], HoleR_x1[si],   FloorHole_y1[si], 3);
	Gra_Line(HoleR_x1[si],   FloorHole_y1[si], HoleR_x0[si],   FloorHole_y0[si], 3);
	Gra_Line(HoleR_x1[si]-1, FloorHole_y1[si], HoleR_x0[si]-1, FloorHole_y0[si], 3);
	Gra_Line(HoleR_x0[si]-1, FloorHole_y0[si], HoleL_x0[si]+1, FloorHole_y0[si], 3);
}

/*C_AB08*/Special3D(bp04)
unsigned char bp04;
{
	if(PitAnimTick-- == 0)
		PitAnimTick = speed_info - 1;
	switch(bp04 & 0xf0) {
		default: return ((bp04 & 0xf0) < 0xc0)?0:-1;
		case 0x40: return TIL_CHEST;
		case 0x70: return TIL_MAGIC_BOLT;
		case 0x80:
			if(PitAnimTick == 1 && U4_RND1(0x3f) == 0 && (bp04 & 0xf)) {
				if((bp04 & 0x0f) >= 8)
					DrawFloorHole();
				else
					DrawCeilingHole();
			}
		return 0;
		case 0x90: return TIL_SHALLOWS;
		case 0xa0:
			DrawField3D((bp04 & 3) | TIL_POISON_FIELD);
			switch(Depth) {
				case 0: cursor_rate = 0; break;
				case 1: cursor_rate = 1; break;
				case 2: cursor_rate = 6; break;
				case 3: cursor_rate = 10; break;
			}
		return TIL_SHIP_RAIL;
		case 0xb0: return TIL_ALTAR;
	}
	return -1;
}

/*isSpriteRenderable?*/
/*C_ABE5*/IsMonster3D(bp04)
unsigned char bp04;
{
	register int si;

	si = bp04 & 0xf0;
	if(
		si == 0x80 ||
		si == 0x90 ||
		si == 0xa0 ||
		si >= 0xd0
	) return 0;
	if(!(bp04 & 0x0f))
		return 0;
	return 1;
}

/*render sprites?*/
/*C_AC23*/DrawSprite3D(bp04)
unsigned char bp04;
{
	register int loc_B;
	long loc_A;

	cursor_rate = 0;
	switch(Depth) {
		case 0:
			for(loc_B = 0; loc_B < 48; loc_B++)
				Gra_ZoomTileRow(bp04, loc_B / 3, loc_B + 0x70, 0x14, 2);
			loc_A = 1000 * speed_info;
			while(--loc_A);
		break;
		case 1:
			for(loc_B = 0; loc_B < 32; loc_B++)
				Gra_ZoomTileRow(bp04, loc_B >> 1, loc_B + 0x68, 0x14, 2);
			loc_A = 1200 * speed_info;
			while(--loc_A);
		break;
		case 2:
			Gra_FillTiles(1, 1, bp04, 0x60, 0x16);
			loc_A = 2400 * speed_info;
			while(--loc_A);
		break;
		case 3:
			for(loc_B = 0; loc_B < 8; loc_B ++)
				Gra_ZoomTileRow(bp04, loc_B << 1, loc_B + 0x60, 0x16, 1);
			loc_A = 3000 * speed_info;
			while(--loc_A);
		break;
	}
}

/*C_AD6A*/RenderSquare3D(bp06, bp04)
unsigned char bp06;
int bp04;
{
	static int D_8CFA;
	int bp_02;

	Depth = bp04;
	if(MonsterAnimTick-- == 0) {
		MonsterAnimTick = speed_info - 1;
		MonsterFrame = U4_RND1(3);
	}
	if(IsMonster3D(bp06)) {
		DrawSprite3D(TIL_RAT + ((bp06 & 0xf) << 2) - 4 + MonsterFrame);
		return 0;
	}
	if((bp_02 = Special3D(bp06)) > 0) {
		if(bp_02 != TIL_SHIP_RAIL)
			DrawSprite3D(bp_02);
		return 0;
	}
	if(bp_02 != 0) {
		if(Depth != 0 || bp06 >= 0xd0)
			return 0;
	}
	return 1;
}

/*animate 3D sprites?*/
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

/*compute nearest tiles*/
/*C_AE41*/DNG_LookAround()
{
	unsigned char bp_04, bp_02;

	/*back*/
	bp_04 = DNG_X_m(Party._x, Party._dir);
	bp_02 = DNG_Y_m(Party._y, Party._dir);
	tile_back = Area._map.x8x8x8[Party._z][bp_02][bp_04];
	/*current tile*/
	tile_cur = Area._map.x8x8x8[Party._z][Party._y][Party._x];
	/*forward*/
	bp_04 = DNG_X_p(Party._x, Party._dir);
	bp_02 = DNG_Y_p(Party._y, Party._dir);
	tile_forward = Area._map.x8x8x8[Party._z][bp_02][bp_04];
	/*another 2 steps forward*/
	bp_04 = DNG_X_p(bp_04, Party._dir);
	bp_02 = DNG_Y_p(bp_02, Party._dir);
	tile_forward2 = Area._map.x8x8x8[Party._z][bp_02][bp_04];

	bp_04 = DNG_X_p(bp_04, Party._dir);
	bp_02 = DNG_Y_p(bp_02, Party._dir);
	tile_forward3 = Area._map.x8x8x8[Party._z][bp_02][bp_04];
}

/*compute the actual x,y of the expected square on the map
 according to current {x, y, orientation}
 Depth: current depth of rendering
 bp04: current column of rendering:
 .-1 left
 . 0 straight
 .+1 right*/
/*C_AF36*/DNG_SquareAt(getX, getY, bp04)
register int *getX;
register int *getY;
int bp04;
{
	*getX = Party._x;
	*getY = Party._y;
	switch(Party._dir) {
		case DIR_N:
			*getY -= Depth; *getY &= 7;
			*getX += bp04;   *getX &= 7;
		break;
		case DIR_S:
			*getY += Depth; *getY &= 7;
			*getX -= bp04;   *getX &= 7;
		break;
		case DIR_E:
			*getX += Depth; *getX &= 7;
			*getY += bp04;   *getY &= 7;
		break;
		case DIR_W:
			*getX -= Depth; *getX &= 7;
			*getY -= bp04;   *getY &= 7;
		break;
	}
}

#define VERTICAL(x, y0, y1, c) Gra_Line((unsigned char)(x), (y0), (unsigned char)(x), (y1), (c));
#define HORIZONTAL(x0, y, x1, c) Gra_Line((unsigned char)(x0), (y), (unsigned char)(x1), (y), (c));

/*up ladder*/
/*C_AFB5*/DrawLadderUp()
{
	register int si;

	si = Depth;
	DrawCeilingHole();
	if(Party._dir == DIR_N || Party._dir == DIR_S) {
		VERTICAL(0x5f,   Hole_y0[si], 0x5f, 3);
		VERTICAL(0x5f+1, Hole_y0[si], 0x5f, 3);
	} else {
		/*left stile*/
		VERTICAL(Ladder_x0[si],   Hole_y0[si], 0x5e, 3);
		VERTICAL(Ladder_x0[si]+1, Hole_y0[si], 0x5e, 3);
		/*right stile*/
		VERTICAL(Ladder_x1[si],   Hole_y0[si], 0x5e, 3);
		VERTICAL(Ladder_x1[si]-1, Hole_y0[si], 0x5e, 3);
		/*rungs*/
		HORIZONTAL(Ladder_x0[si], LadderUp_y1[si], Ladder_x1[si], 3);
		HORIZONTAL(Ladder_x0[si], LadderUp_y2[si], Ladder_x1[si], 3);
		HORIZONTAL(Ladder_x0[si], LadderUp_y3[si], Ladder_x1[si], 3);
	}
}

/*down ladder*/
/*C_B0E9*/DrawLadderDown()
{
	register int si;

	si = Depth;
	DrawFloorHole();
	if(Party._dir == DIR_N || Party._dir == DIR_S) {
		VERTICAL(0x5f,   FloorHole_y0[si], 0x5f, 3);
		VERTICAL(0x5f+1, FloorHole_y0[si], 0x5f, 3);
	} else {
		/*left stile*/
		VERTICAL(Ladder_x0[si],   0x5f, FloorHole_y0[si], 3);
		VERTICAL(Ladder_x0[si]+1, 0x5f, FloorHole_y0[si], 3);
		/*right stile*/
		VERTICAL(Ladder_x1[si],   0x5f, FloorHole_y0[si], 3);
		VERTICAL(Ladder_x1[si]-1, 0x5f, FloorHole_y0[si], 3);
		/*rungs*/
		HORIZONTAL(Ladder_x0[si], LadderDown_y1[si], Ladder_x1[si], 3);
		HORIZONTAL(Ladder_x0[si], LadderDown_y2[si], Ladder_x1[si], 3);
		HORIZONTAL(Ladder_x0[si], LadderDown_y3[si], Ladder_x1[si], 3);
	}
}

#undef VERTICAL
#undef HORIZONTAL

/*Gra_DrawQuad(
	x0, x1,
	y0, y1,
	y0p, y1p,
	col0, col1
);*/

/*front wall?*/
/*C_B21D*/DrawFrontWall()
{
	Gra_DrawQuad(
		Wall_x0[Depth], Wall_x1[Depth] + 1,
		Wall_y0[Depth], Wall_y1[Depth],
		Wall_y0[Depth], Wall_y1[Depth],
		(Party._dir & 1) + 1, 0
	);
}

/*left front wall?*/
/*C_B25E*/DrawLeftFrontWall()
{
	Gra_DrawQuad(
		Wall_x0[Depth],   Wall_x0[Depth+1],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		(Party._dir & 1) + 1, 0
	);
}

/*right front wall?*/
/*C_B29C*/DrawRightFrontWall()
{
	Gra_DrawQuad(
		Wall_x1[Depth],   Wall_x1[Depth+1],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		(Party._dir & 1) + 1, 0
	);
}

/*left lateral wall?*/
/*C_B2DA*/DrawLeftSideWall()
{
	Gra_DrawQuad(
		Wall_x0[Depth],   Wall_x0[Depth+1],
		Wall_y0[Depth],   Wall_y1[Depth],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		!(Party._dir & 1) + 1, 0
	);
}

/*right lateral wall?*/
/*C_B31B*/DrawRightSideWall()
{
	Gra_DrawQuad(
		Wall_x1[Depth],   Wall_x1[Depth+1],
		Wall_y0[Depth],   Wall_y1[Depth],
		Wall_y0[Depth+1], Wall_y1[Depth+1],
		!(Party._dir & 1) + 1, 0
	);
}

/*front door?*/
/*C_B35C*/DrawFrontDoor()
{
	Gra_DrawQuad(
		Wall_x0[Depth+1], Wall_x1[Depth+1],
		Wall_y0[Depth+1], Wall_y1[Depth],
		Wall_y0[Depth+1], Wall_y1[Depth],
		!Depth?0:3, !Depth?3:0
	);
}

/*left door?*/
/*C_B3AE*/DrawLeftDoor()
{
	Gra_DrawQuad(
		LDoor_x0[Depth], LDoor_x1[Depth],
		Door_y0[Depth],  Door_y1[Depth],
		Door_y0p[Depth], Door_y1p[Depth],
		3, 0
	);
}

/*right door?*/
/*C_B3E3*/DrawRightDoor()
{
	Gra_DrawQuad(
		RDoor_x0[Depth], RDoor_x1[Depth],
		Door_y0[Depth],  Door_y1[Depth],
		Door_y0p[Depth], Door_y1p[Depth],
		3, 0
	);
}

/*right walls*/
/*C_B418*/DrawRightWalls()
{
	int bp_02, bp_04;

	DNG_SquareAt(&bp_04, &bp_02, 1);
	switch(Area._map.x8x8x8[Party._z][bp_02][bp_04] >> 4) {
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
		case 0xb: DrawRightFrontWall(); break;
		case 0xc:
		case 0xd: DrawRightSideWall(); DrawRightDoor(); break;
		case 0xe:
		case 0xf: DrawRightSideWall(); break;
	}
}

/*left & right walls*/
/*C_B47A*/DrawSideWalls()
{
	int bp_02, bp_04;

	DNG_SquareAt(&bp_04, &bp_02, -1);
	switch(Area._map.x8x8x8[Party._z][bp_02][bp_04] >> 4) {
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
		case 0xb: DrawLeftFrontWall(); break;
		case 0xc:
		case 0xd: DrawLeftSideWall(); DrawLeftDoor(); break;
		case 0xe:
		case 0xf: DrawLeftSideWall(); break;
	}
	DrawRightWalls();
}

/*render 3d*/
/*C_B4DF*/Render3D()
{
	int bp_04, bp_02;

	bp_04 = Party._x;
	bp_02 = Party._y;
	for(Depth = 0; Depth <= 3; Depth ++) {
		DNG_SquareAt(&bp_04, &bp_02, 0);
		switch(Area._map.x8x8x8[Party._z][bp_02][bp_04] & 0xf0) {
			case 0x30: DrawLadderDown();
			case 0x10: DrawLadderUp();
			case 0x00:
			case 0x40:
			case 0x70:
			case 0x80:
			case 0x90:
			case 0xb0: DrawSideWalls(); break;
			case 0x20: DrawLadderDown(); DrawSideWalls(); break;
			case 0x50: DrawCeilingHole(); DrawSideWalls(); break;
			case 0x60: DrawFloorHole(); DrawSideWalls(); break;
			case 0xc0: DrawFrontWall(); DrawFrontDoor(); if(Depth) return; break;
			case 0xd0: DrawFrontWall(); DrawFrontDoor(); return;
			case 0xa0: DrawField3D(Area._map.x8x8x8[Party._z][bp_02][bp_04]); return;
			case 0xe0:
			case 0xf0: DrawFrontWall(); return;
		}
	}
	Depth = 4;
	DNG_SquareAt(&bp_04, &bp_02, 0);
	if(
		(Area._map.x8x8x8[Party._z][bp_02][bp_04] & 0xf0) == 0xf0 ||
		(Area._map.x8x8x8[Party._z][bp_02][bp_04] & 0xf0) == 0xe0
	) DrawFrontWall();
}

/*backup dungeon infos & render 3D*/
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

/*render 3d if necessary*/
/*C_B677*/DNG_Render()
{
	unsigned char bp_02;

	cursor_rate = (Party._fly_light != 0)?13:1;
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
		bp_02 = tile_cur & 0xf0;
		if(bp_02 == 0xa0 || bp_02 >= 0xd0)
			return;
		if(tile_forward != Saved_forward)
			goto C_B729;
		if((bp_02 = tile_forward & 0xf0) == 0xa0 || bp_02 >= 0xc0)
			return;
		if(tile_forward2 != Saved_forward2)
			goto C_B729;
		if((bp_02 = tile_forward2 & 0xf0) == 0xa0 || bp_02 >= 0xc0)
			return;
		if(Saved_forward3 == Saved_forward3/*bug?*/)
			return;
	}
C_B729:
	Render3DNow();
}
