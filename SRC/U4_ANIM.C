/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

unsigned /*D_1630*/FlowCounter = 0;

/*animate force fields, water, ...*/
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

/*display direction*/
/*C_353D*/ShowDirection(bp06, bp04)
unsigned bp06;
char *bp04;
{
	unsigned bp_02, bp_04;

	bp_02 = txt_X;
	bp_04 = txt_Y;
	txt_Y = 23;
	txt_X = 7;
	u4_puts(bp04);
	switch(bp06) {
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
	txt_X = bp_02;
	txt_Y = bp_04;
}

unsigned /*D_1654*/WindCounter = 0;

/*update/display wind*/
/*C_35C7*/UpdateWind()
{
	if(WindCounter-- == 0) {
		WindCounter = speed_info - 1;
		if(!U4_RND1(0xfc))
			WindDir = (U4_RND1(2) + WindDir - 1) & 3;
	}
	ShowDirection(WindDir, "Wind");
}

unsigned /*D_1656*/NpcAnimChance[] = {192, 96, 60, 36, 20, 12, 8};

/*animate sprites*/
/*C_3605*/AnimateNpcs()
{
	register int si, di;

	for(si = 0; si < 32; si++) {
		di = Area._npc._tile[si];
		if(
			(di >= TIL_NIXIE && di <= TIL_TWISTER) ||
			(di >= TIL_MAGE && di <= TIL_SHEPHERD) ||
			(di >= TIL_GUARD && di <= TIL_LORD_BRITISH)
		) {
			if(U4_RND2(1) != 0)
			if(((speed_info < 8)?NpcAnimChance[speed_info - 1]:0xff) > U4_RND2(0xff))
			Area._npc._gtile[si] = ((Area._npc._gtile[si] & 1) | di) ^ 1;
		} else if(di >= TIL_RAT) {
			if(((speed_info < 8)?NpcAnimChance[speed_info - 1]:0xff) > U4_RND2(0xff))
			Area._npc._gtile[si] = ((Area._npc._gtile[si] + 1) & 3) | di;
		} else if(di != TIL_PIRATE_W) {
			Area._npc._gtile[si] = di;
		}
	}
	AnimateTiles();
	Gra_AnimFlags();
}

/*C_36C7*/DrawView()
{
	register unsigned si;

	si = ViewTiles[hit_y*11 + hit_x];
	if(hit_tile)
		ViewTiles[hit_y*11 + hit_x] = hit_tile;
	Gra_DrawTiles(11, 11, ViewTiles, 4  * 80 + 2);
	if(activeCharaX != -1) {
		register int di;

		di = CursorBlink;
		CursorBlink -= u4_sign(CursorBlink);
		if(CursorBlink == 0) {
			if(di < 0)
				CursorBlink = 4;
			else
				CursorBlink = -9;
		}
		if(CursorBlink > 0)
			Gra_ActiveBox(activeCharaY, activeCharaX);
	}
	ViewTiles[hit_y*11 + hit_x] = si;
}

/*cast one ray*/
/*C_376F*/CastRay(bp0a, bp08, bp06, bp04)
int bp0a;/*diff_x*/
int bp08;/*diff_y*/
unsigned char bp06;/*pos_x*/
unsigned char bp04;/*pos_y*/
{
	register unsigned si;/*tmp_x*/
	unsigned bp_04;/*tmp_y*/

	si = &(ViewTiles[bp04*11 + bp06]) - ViewTiles;
	if(bp06 > 10 || bp04 > 10)
		return;
	if(ViewTiles[si] != TIL_BLACK)
		return;
	bp_04 = Combat._map[si];
	ViewTiles[si] = bp_04;
	/*opaque tiles*/
	if(
		bp_04 == TIL_FOREST ||
		bp_04 == TIL_MOUNTAINS ||
		bp_04 == TIL_BLACK ||
		bp_04 == TIL_SECRET_DOOR ||
		bp_04 == TIL_WALL
	) return;

	bp06 += bp0a;
	bp04 += bp08;
	CastRay(bp0a, bp08, bp06, bp04);
	if(bp0a & bp08) {
		CastRay(bp0a, bp08, bp06,        bp04 - bp08);
		CastRay(bp0a, bp08, bp06 - bp0a, bp04);
	} else {
		CastRay((!bp0a) * bp08 + bp0a, bp08 - (!bp08) * bp0a, bp08 + bp06, bp04 - bp0a);
		CastRay(bp0a - (!bp0a) * bp08, (!bp08) * bp0a + bp08, bp06 - bp08, bp0a + bp04);
	}
}

/*process visible area (raycast)*/
/*C_38BB*/ComputeVisibility()
{
	register int si;

	/*-- set all visible squares to TIL_BLACK --*/
	for(si = 10 * 11 + 10; si >= 0; si --)
		ViewTiles[si] = TIL_BLACK;
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

/*C_3960*/PlaceNpcsInView()
{
	register int loc_B;
	unsigned char loc_A, loc_C;

	for(loc_B = 31; loc_B >= 0; loc_B --) {

		loc_A = Area._npc._x[loc_B] - Party._x + 5;
		loc_C = Area._npc._y[loc_B] - Party._y + 5;
		if(Area._npc._gtile[loc_B] && loc_A < 11 && loc_C < 11)
			Combat_MAP(loc_C, loc_A) = Area._npc._gtile[loc_B];
	}
}

/*C_39BA*/FinishView()
{
	register int loc_A;

	PlaceNpcsInView();
	if(MoongateTile) {
		unsigned char loc_B, loc_C;

		loc_B = OpenGateX - Party._x + 5;
		loc_C = OpenGateY - Party._y + 5;
		if(loc_B < 11 && loc_C < 11)
			Combat_MAP(loc_C, loc_B) = MoongateTile;
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
		for(loc_A = 10 * 11 + 10; loc_A >= 0; loc_A --)
			ViewTiles[loc_A] = Combat._map[loc_A];
	}
	DrawView();
}

unsigned char /*D_1664*/MoonPhaseCounter = 0;
unsigned char /*D_1665*/TrammelCounter = 0;/*trammel counter*/
unsigned char /*D_1666*/FeluccaCounter = 0;/*felucca counter*/

/*C_3A4F*/GetMoongatePhase()
{
	OpenGateX = MoongateX[(TrammelCounter & 0xe0) >> 5];
	OpenGateY = MoongateY[(TrammelCounter & 0xe0) >> 5];
	return ((MoonPhaseCounter & 0xc0) >> 6);
}

unsigned /*D_1668*/MoonTick = 0;

/*display tramel and feluca ?*/
/*C_3A80*/UpdateMoons()
{
	unsigned char bp_02, bp_04;

	if(MoonTick-- == 0) {
		MoonTick = speed_info / 2;
		MoonPhaseCounter += 0x40;
		if(MoonPhaseCounter == 0) {
			TrammelCounter += 2;
			FeluccaCounter += 6;
		}
		Party._trammel = TrammelCounter >> 5;
		Party._felucca = FeluccaCounter >> 5;
		if((TrammelCounter & 0x1f) == 0) {
			MoongateTile = GetMoongatePhase() + 0x40;
		} else if((TrammelCounter & 0x1f) == 0x1e){
			MoongateTile = (GetMoongatePhase() ^ 3) + 0x40;
		}
	}
	bp_02 = txt_X;
	bp_04 = txt_Y;
	txt_Y = 0;
	txt_X = 11;
	Gra_PutChar((((TrammelCounter >> 5) - 1) & 7) + 0x14);
	txt_X ++;
	Gra_PutChar((((FeluccaCounter >> 5) - 1) & 7) + 0x14);
	txt_X = bp_02;
	txt_Y = bp_04;
}

/*C_3B35*/DrawOutdoors()
{
	register unsigned loc_B;
	int loc_C, loc_A;

	AnimateNpcs();/*animate sprites*/
	UpdateMoons();/*display tramel and feluca*/
	loc_B = 10 * 11 + 10;
	for(loc_C = 10; loc_C >= 0; loc_C --) {
		for(loc_A = 10; loc_A >= 0; loc_A --) {
			Combat._map[loc_B] = Area._map.x32x32[loc_C+WinPos.y-5][loc_A+WinPos.x-5];
			loc_B --;
		}
	}
	FinishView();/*display 11x11 icons zone*/
}

/*display inside towne/castle ...*/
/*C_3B83*/DrawTown()
{
	register int si, di;

	AnimateNpcs();
	for(di = -5; di <= 5; di ++) {
		for(si = -5; si <= 5; si ++) {
			if(
				((int)Party._x + si < 0) || ((int)Party._x + si > 0x1f) ||
				((int)Party._y + di < 0) || ((int)Party._y + di > 0x1f)
			) {
				Combat_MAP(di+5, si+5) = TIL_GRASS;
			} else {
				Combat_MAP(di+5, si+5) = Area._map.x32x32[Party._y + di][Party._x + si];
			}
		}
	}
	FinishView();/*display 11x11 icons zone*/
	UpdateWind();/*update/display wind*/
}

unsigned /*D_166A*/BalloonTick = 0;

/*move balloon*/
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

unsigned /*D_166C*/CombatAnimMask[] = {1,3,3,7,7,7,7,15};

/*C_3C54*/DrawCombat()
{
	register int loc_A;
	unsigned loc_B;
	unsigned char *loc_C;
	unsigned char *loc_D;
	unsigned char *loc_E;

	loc_B = (speed_info < 8)?CombatAnimMask[speed_info - 1]:15;
	AnimateTiles();
	for(loc_A = 10 * 11 + 10; loc_A >= 0; loc_A--)
		ViewTiles[loc_A] = Combat._map[loc_A];
	if(CurMode != MOD_SHRINE) {
		for(loc_A = 15; loc_A >= 0; loc_A--) {
			loc_E = &(Fighters._tile[loc_A]);
			if(*loc_E == 0)
				continue;
			loc_D = &(ViewTiles[Combat._npcY[loc_A]*11 + Combat._npcX[loc_A]]);
			loc_C = &(Fighters._gtile[loc_A]);
			if(Fighters._sleeping[loc_A])
				*loc_D = *loc_C;
			else if(*loc_E >= TIL_RAT) {
				if(*loc_E == TIL_MIMIC && *loc_C == TIL_CHEST || U4_RND1(loc_B) == 0) {
					*loc_D = *loc_C;
				} else if(U4_RND1(loc_B) == 0) {
					*loc_C = ((*loc_C + 1) & 3) | *loc_E;
					*loc_D = *loc_C;
				} else {
					*loc_D = *loc_C;
				}
			} else if(*loc_E == TIL_SLEEPING) {
				*loc_D = *loc_C = TIL_SLEEPING;
			} else {
/*C_3D35*/
/*note: I had a hard time figuring out how to get this statement right*/
				*loc_D = *loc_C ^= U4_RND1(U4_RND2(1));
			}
		}
		for(loc_A = Party._members - 1; loc_A >= 0; loc_A --) {
			unsigned char *bp_0c;
			unsigned char *bp_0e;
			unsigned char bp_10;

			bp_0e = &(Fighters._chtile[loc_A]);
			bp_10 = *bp_0e;
			if(bp_10 == 0)
				continue;
			bp_0c = &(ViewTiles[Combat._charaY[loc_A]*11 + Combat._charaX[loc_A]]);
			if(bp_10 == TIL_SLEEPING) {
				*bp_0c = CharaAnimTile[loc_A] = bp_10;
				continue;
			}
			*bp_0c = (CharaAnimTile[loc_A] ^= !U4_RND1(loc_B));
		}
	}
	DrawView();
}

/*display game zone*/
/*C_3DC8*/RefreshView()
{
	if(CurMode == MOD_VISION) {
		cursor_rate = 233;
		return;
	}
	cursor_rate = 0;
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
