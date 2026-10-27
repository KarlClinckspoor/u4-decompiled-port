/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <stdlib.h>

/*the world map (WORLD.MAP) is made of 8x8 chunks of 32x32 tiles; this is the
  chunk currently in memory (MapChunks), -1 for none*/
int16 /*D_08CE*/LoadedChunkX = -1;
int16 /*D_08D0*/LoadedChunkY = -1;

/*
 * Load the chunk (chunkX, chunkY) of the world map into MapChunks, after
 * freeing the previous one: only one chunk is in memory at a time.
 * Exits the program on error.
 */
/*C_2399*/LoadMapChunk(chunkY, chunkX)
uint16 chunkY;
uint16 chunkX;
{
	int16 actual;

	if(LoadedChunkX != -1) {
		dfree((void far *)MapChunks[LoadedChunkY][LoadedChunkX]);
		MapChunks[LoadedChunkY][LoadedChunkX] = 0;
	}
	if(MapChunks[chunkY][chunkX] == 0) {
		if(dlseek(File_MAP, (chunkY * 8 + chunkX) * sizeof(tMap32x32)) == -1)
			exit(1);
		MapChunks[chunkY][chunkX] = (tMap32x32 far *)dalloc(sizeof(tMap32x32));
		if(MapChunks[chunkY][chunkX] == 0)
			exit(1);
		actual = dread(File_MAP, MapChunks[chunkY][chunkX], sizeof(tMap32x32));
		if(actual != sizeof(tMap32x32))
			exit(3);
	}
	LoadedChunkX = chunkX;
	LoadedChunkY = chunkY;
}

/*
 * The outdoor map in memory (Area._map.x32x32) is a 32x32 window of the
 * world made of 4 blocks of 16x16 tiles; WinBlock is the position of its
 * north-west block, in blocks (0-15), and WinPos the party's position in
 * the window. These 4 functions copy each block from the world map.
 */
/*C_249C*/LoadWindowNW()
{
	register int16 tx, ty;

	LoadMapChunk(WinBlock.y / 2, WinBlock.x / 2);
	for(ty = 0; ty < 16; ty ++)
		for(tx = 0; tx < 16; tx ++)
			Area._map.x32x32[ty][tx] = 
				(*(MapChunks[WinBlock.y/2][WinBlock.x/2]))[(WinBlock.y&1)*0x10+ty][(WinBlock.x&1)*0x10+tx];
}

/*the block east of, and the block south of WinBlock*/
#define TMP0 (((WinBlock.x+1)&15))
#define TMP1 (((WinBlock.y+1)&15))

/*C_2517*/LoadWindowNE()
{
	register int16 tx, ty;

	LoadMapChunk(WinBlock.y / 2, TMP0 / 2);
	for(ty = 0; ty < 16; ty ++)
		for(tx = 0; tx < 16; tx ++)
			Area._map.x32x32[ty][tx+16] = 
				(*(MapChunks[WinBlock.y/2][TMP0/2]))[(WinBlock.y&1)*0x10+ty][(TMP0&1)*0x10+tx];
}

/*C_259F*/LoadWindowSW()
{
	register int16 tx, ty;

	LoadMapChunk(TMP1 / 2, WinBlock.x / 2);
	for(ty = 0; ty < 16; ty ++)
		for(tx = 0; tx < 16; tx ++)
			Area._map.x32x32[ty+16][tx] = 
				(*(MapChunks[TMP1/2][WinBlock.x/2]))[(TMP1&1)*0x10+ty][(WinBlock.x&1)*0x10+tx];
}

/*C_2624*/LoadWindowSE()
{
	register int16 tx, ty;

	LoadMapChunk(TMP1 / 2, TMP0 / 2);
	for(ty = 0; ty < 16; ty ++)
		for(tx = 0; tx < 16; tx ++)
			Area._map.x32x32[ty+16][tx+16] = 
				(*(MapChunks[TMP1/2][TMP0/2]))[(TMP1&1)*0x10+ty][(TMP0&1)*0x10+tx];
}

/*
 * Load the whole window around the party's position on the world map
 * (around the position outside, Party.out_x/out_y, in a dungeon or shrine),
 * so that the party is in the middle area of the window.
 */
/*C_26B6*/LoadWorldWindow()
{
	WinBlock.x = ((Party._loc < LOC_DUNGEONS)?Party._x:Party.out_x) >> 4;
	WinPos.x = ((Party._loc < LOC_DUNGEONS)?Party._x:Party.out_x) & 15;
	if(WinPos.x < 8) {
		WinPos.x += 0x10;
		WinBlock.x = (WinBlock.x - 1) & 15;
	}
	WinBlock.y = ((Party._loc < LOC_DUNGEONS)?Party._y:Party.out_y) >> 4;
	WinPos.y = ((Party._loc < LOC_DUNGEONS)?Party._y:Party.out_y) & 15;
	if(WinPos.y < 8) {
		WinPos.y += 0x10;
		WinBlock.y = (WinBlock.y - 1) & 15;
	}
	LoadWindowNW();
	LoadWindowNE();
	LoadWindowSW();
	LoadWindowSE();
}

/*
 * Back to the world map, from a town/castle or a dungeon: restores the
 * outdoor monsters (OUTMONST.SAV) and the party's position outside.
 * Coming out of Hythloth (0xef, 0xf0), the balloon is put nearby.
 */
/*C_2747*/LeaveTown()
{
	u4_puts("Leaving...\n");
	CurMode = MOD_VISION;
	if(Load("OUTMONST.SAV", sizeof(struct tNPC), &(Area._npc)) == -1)
		exit(3);
	if(File_TLK) {
		dclose(File_TLK);
		File_TLK = 0;
	}
	Party._x = Party.out_x;
	Party._y = Party.out_y;
	LoadWorldWindow();
	CurMode = MOD_OUTDOORS;
	Party._loc = 0;
	Party._fly_light = 0;
	LineOfSight = 1;
	if(Party._x == 0xef && Party._y == 0xf0) {
		Area._npc._tile[31] = Area._npc._gtile[31] = TIL_BALLOON;
		Area._npc._x[31] = 0xe9;
		Area._npc._y[31] = 0xf2;
	}
	u_kbflush();
}

/*
 * Replace the 8 outdoor monsters by daemons, all at x 0xe7 on the row
 * south of the party, unless the horn is in use (spell_effect 1).
 */
/*C_27D9*/SpawnDaemons()
{
	if(spell_effect != 1) {
		register int16 i;

		for(i = 7; i >= 0; i--) {
			Area._npc._tile[i] = Area._npc._gtile[i] = TIL_DAEMON;
			Area._npc._x[i] = 0xe7;
			Area._npc._y[i] = Party._y + 1;
		}
	}
}

/*the 8 pirate ships of SpawnPirates: positions and facing*/
unsigned char /*D_08EC*/PirateX[] = {  0xE0,  0xE0,  0xE2,  0xE3,  0xE4,  0xE5,  0xE5,  0xE4};
unsigned char /*D_08F4*/PirateY[] = {  0xDC,  0xE4,  0xDC,  0xE4,  0xE3,  0xE1,  0xDF,  0xDE};
unsigned char /*D_08FC*/PirateTile[] = {TIL_PIRATE_E,TIL_PIRATE_E,TIL_PIRATE_E,TIL_PIRATE_E,TIL_PIRATE_S,TIL_PIRATE_S,TIL_PIRATE_N,TIL_PIRATE_N};

/*
 * Replace the 8 outdoor monsters by pirate ships, east of the party (the
 * ambush near the Isle of the Abyss).
 */
/*C_280A*/SpawnPirates()
{
	register int16 i;

	for(i = 7; i >= 0; i --) {
		Area._npc._x[i] = PirateX[i];
		Area._npc._y[i] = PirateY[i];
		Area._npc._tile[i] = TIL_PIRATE_W;
		Area._npc._gtile[i] = PirateTile[i];
	}
}

/*
 * Move the party 1 tile north on the world map. When it gets near the north
 * edge of the window, the window moves 16 tiles north: the north half
 * becomes the south half and the new north blocks are loaded.
 */
/*C_2839*/ScrollNorth()
{
	register int16 tx, ty;

	Party._y --;
	WinPos.y = (WinPos.y - 1) & 0xff;
	if(WinPos.y < 5) {
		WinPos.y += 16;
		WinBlock.y = (WinBlock.y - 1) & 15;
		for(ty = 0; ty < 16; ty ++)
			for(tx = 0; tx < 32; tx ++)
				Area._map.x32x32[ty+16][tx] = Area._map.x32x32[ty][tx];
		LoadWindowNW();
		LoadWindowNE();
	}
}

/*
 * Move the party 1 tile south on the world map (see ScrollNorth).
 */
/*C_2891*/ScrollSouth()
{
	register int16 tx, ty;

	Party._y ++;
	WinPos.y = (WinPos.y + 1) & 0xff;
	if(WinPos.y >= 27) {
		WinPos.y -= 16;
		WinBlock.y = (WinBlock.y + 1) & 15;
		for(ty = 0; ty < 16; ty ++)
			for(tx = 0; tx < 32; tx ++)
				Area._map.x32x32[ty][tx] = Area._map.x32x32[ty+16][tx];
		LoadWindowSW();
		LoadWindowSE();
	}
}

/*
 * Move the party 1 tile west on the world map (see ScrollNorth).
 */
/*C_28E9*/ScrollWest()
{
	register int16 tx, ty;

	Party._x --;
	WinPos.x = (WinPos.x - 1) & 0xff;
	if(WinPos.x < 5) {
		WinPos.x += 16;
		WinBlock.x = (WinBlock.x - 1) & 15;
		for(ty = 0; ty < 32; ty ++)
			for(tx = 0; tx < 16; tx ++)
				Area._map.x32x32[ty][tx+16] = Area._map.x32x32[ty][tx];
		LoadWindowNW();
		LoadWindowSW();
	}
}

/*
 * Move the party 1 tile east on the world map (see ScrollNorth).
 */
/*C_2941*/ScrollEast()
{
	register int16 tx, ty;

	Party._x ++;
	WinPos.x = (WinPos.x + 1) & 0xff;
	if(WinPos.x >= 27) {
		WinPos.x -= 16;
		WinBlock.x = (WinBlock.x + 1) & 15;
		for(ty = 0; ty < 32; ty ++)
			for(tx = 0; tx < 16; tx ++)
				Area._map.x32x32[ty][tx] = Area._map.x32x32[ty][tx+16];
		LoadWindowNE();
		LoadWindowSE();
	}
}

/*the tiles the party can walk on (IsWalkable)*/
unsigned char /*D_0904*/WalkableTiles[] = {
	TIL_SWAMP,TIL_GRASS,TIL_SCRUB,TIL_FOREST,TIL_HILLS,TIL_DUNGEON,TIL_TOWN,TIL_CASTLE,TIL_VILLAGE,
	TIL_SHIP_W,TIL_SHIP_N,TIL_SHIP_E,TIL_SHIP_S,TIL_HORSE_W,TIL_HORSE_E,TIL_TILED_FLOOR,TIL_BRIDGE,TIL_BALLOON,
	TIL_BRIDGE_N,TIL_BRIDGE_S,TIL_LADDER_UP,TIL_LADDER_DOWN,TIL_RUINS,TIL_SHRINE,TIL_CHEST,TIL_BRICK_FLOOR,TIL_WOOD_FLOOR,
	TIL_MOONGATE,TIL_POISON_FIELD,TIL_FIRE_FIELD,TIL_SLEEP_FIELD,TIL_SECRET_DOOR,TIL_ALTAR,TIL_LAVA,TIL_TWISTER,TIL_TWISTER_1,
	0/*End of list*/
};

/*
 * Returns 1 if the party can walk on the tile (see WalkableTiles), else 0.
 */
/*C_2999*/IsWalkable(tile)
unsigned char tile;
{
	register int16 i;

	for(i = 0; WalkableTiles[i]; i ++)
		if(tile == WalkableTiles[i])
			return 1;
	return 0;
}

/*C_29C3*/w_Blocked()
{
	u4_puts("Blocked!\n");
	sound(SND_BEEP);
	u_kbflush();
}

/*C_29DE*/w_DriftOnly()
{
	u4_puts("Drift Only!\n");
	u_kbflush();
}

/*
 * Returns 1 if walking onto the tile fails this turn (slow progress):
 * 1 time in 8 in a swamp, 1 in 4 in scrub or forest, 1 in 2 in the hills
 * or in a fire field; else 0.
 */
/*C_29EF*/IsSlowTerrain(tile)
unsigned char tile;
{
	switch(tile) {
		case TIL_SWAMP: if(U4_RND1(7) != 0) return 0; break;
		case TIL_SCRUB: case TIL_FOREST: if(U4_RND1(3) != 0) return 0; break;
		case TIL_HILLS: case TIL_FIRE_FIELD: if(U4_RND1(1) == 0) return 0; break;
		default: return 0;
	}
	return 1;
}

/*
 * Returns 1 if a ship can sail on the tile: deep water, water, whirlpool
 * or twister.
 */
/*C_2A38*/IsSailable(tile)
unsigned char tile;
{
	return (tile < TIL_SHALLOWS) || ((tile >= TIL_WHIRLPOOL) && (tile < TIL_RAT));
}

/*
 * Returns 1 if the ship can sail in the direction dir this turn: only 1 move
 * in 4 into the wind (WindDir being where the wind comes from), 3 in 4 with
 * the wind behind, always across the wind.
 */
/*C_2A5A*/WindAllows(dir)
uint16 dir;
{
	if(WindDir == dir)
		return !(Party._moves & 3);
	if(WindDir == ((dir+2)&3))
		return (Party._moves & 3);
	return 1;
}

/*
 * If the tile is a moongate (the party has just walked onto it), the party
 * goes through: to the Shrine of Spirituality when both moons are in phase
 * 4, else to the moongate given by the phase of Felucca.
 * Returns 1 if the party went through, else 0.
 */
/*C_2A91*/CheckMoongate(tile)
unsigned char tile;
{
	if(tile != TIL_MOONGATE)
		return 0;
	if(Party._trammel == 4 && Party._felucca == 4) {
		RefreshView();
		Gra_InvertView();
		Party._loc = LOC_SPIRITUALITY;
		sound(SND_MAGIC, 0xa0);
		EnterShrine();
		return 1;
	}
	RefreshView();
	Party._x = MoongateX[Party._felucca];
	Party._y = MoongateY[Party._felucca];
	Gra_InvertView(); sound(SND_MAGIC, 0xa0); Gra_InvertView();
	LoadWorldWindow();
	RefreshView();
	Gra_InvertView(); sound(SND_MAGIC, 0xa0); Gra_InvertView();
	return 1;
}

/*
 * Walk one tile north (on foot or on horseback). Returns 1 if the party
 * moved (or tried, slowly), so that a galloping horse can move again;
 * 0 if blocked, through a moongate or out of the town.
 * In a town, walking off the map edge leaves the town.
 */
/*C_2B19*/MoveNorth()
{
	/*the entrance of Lord British's castle can only be walked into
	  from the south, and left to the south*/
	if(tile_cur == TIL_LCB_ENTRANCE) {
		w_Blocked();
		return 0;
	}
	if(tile_north != TIL_LCB_ENTRANCE && !IsWalkable(tile_north)) {
		w_Blocked();
		return 0;
	}
	if(IsSlowTerrain(tile_north)) {
		w_SlowProgress();
		return 1;
	}
	sound(SND_STEP);
	if(CurMode == MOD_OUTDOORS) {
		ScrollNorth();
		if(CheckMoongate(tile_north))
			return 0;
		return 1;
	}
	Party._y --;
	WinPos.y = Party._y;
	if(WinPos.y <= 0x1f) {
		return 1;
	}
	LeaveTown();
	return 0;
}

/*
 * The North arrow key: a ship turns north, or sails north if it already
 * faces north (against the wind or not, see WindAllows); the balloon can
 * only drift; else the party walks north (twice when galloping).
 */
/*C_2B8C*/CMDDIR_Up()
{
	if(Party._tile == TIL_SHIP_W || Party._tile == TIL_SHIP_E || Party._tile == TIL_SHIP_S) {
		Party._tile = TIL_SHIP_N;
		u4_puts("Turn North!\n");
	} else if(Party._tile == TIL_SHIP_N) {
		u4_puts("Sail North!\n");
		if(!IsSailable(tile_north)) {
			w_Blocked();
		} else if(!WindAllows(DIR_N)) {
			w_SlowProgress();
		} else {
			ScrollNorth();
		}
	} else if(Party._tile == TIL_BALLOON) {
		w_DriftOnly();
	} else {
		sound(SND_STEP);
		u4_puts("North\n");
		if(MoveNorth() && Gallop) {
			RefreshView();
			sound(SND_STEP);
			MoveNorth();
		}
	}
}

/*
 * Walk one tile south (see MoveNorth).
 */
/*C_2C25*/MoveSouth()
{
	if(!IsWalkable(tile_south)) {
		w_Blocked();
		return 0;
	}
	if(IsSlowTerrain(tile_south)) {
		w_SlowProgress();
		return 1;
	}
	sound(SND_STEP);
	if(CurMode == MOD_OUTDOORS) {
		ScrollSouth();
		if(CheckMoongate(tile_south))
			return 0;
		return 1;
	}
	Party._y ++;
	WinPos.y = Party._y;
	if(WinPos.y <= 0x1f) {
		return 1;
	}
	LeaveTown();
	return 0;
}

/*
 * The South arrow key (see CMDDIR_Up).
 */
/*C_2C8A*/CMDDIR_Down()
{
	if(Party._tile == TIL_BALLOON) {
		w_DriftOnly();
	} else if(Party._tile == TIL_SHIP_W || Party._tile == TIL_SHIP_E || Party._tile == TIL_SHIP_N) {
		Party._tile = TIL_SHIP_S;
		u4_puts("Turn South!\n");
	} else if(Party._tile == TIL_SHIP_S) {
		u4_puts("Sail South!\n");
		if(!IsSailable(tile_south)) {
			w_Blocked();
		} else if(!WindAllows(DIR_S)) {
			w_SlowProgress();
		} else {
			ScrollSouth();
		}
	} else {
		sound(SND_STEP);
		u4_puts("South\n");
		if(MoveSouth() && Gallop) {
			RefreshView();
			sound(SND_STEP);
			MoveSouth();
		}
		/*walking south in this area of the Isle of the Abyss
		  brings the daemons*/
		if(
			Party._x >= 0xe5 && Party._x < 0xea &&
			Party._y >= 0xd4 && Party._y < 0xd9
		) SpawnDaemons();
	}
}

/*
 * Walk one tile west (see MoveNorth).
 */
/*C_2D44*/MoveWest()
{
	if(!IsWalkable(tile_west)) {
		w_Blocked();
		return 0;
	}
	if(IsSlowTerrain(tile_west)) {
		w_SlowProgress();
		return 1;
	}
	sound(SND_STEP);
	if(CurMode == MOD_OUTDOORS) {
		ScrollWest();
		if(CheckMoongate(tile_west))
			return 0;
		return 1;
	}
	Party._x --;
	WinPos.x = Party._x;
	if(WinPos.x <= 0x1f) {
		return 1;
	}
	LeaveTown();
	return 0;
}

/*
 * The West arrow key (see CMDDIR_Up). A horse turns west.
 */
/*C_2DA9*/CMDDIR_Left()
{
	if(Party._tile == TIL_BALLOON) {
		w_DriftOnly();
	} else if(Party._tile == TIL_SHIP_S || Party._tile == TIL_SHIP_E || Party._tile == TIL_SHIP_N) {
		Party._tile = TIL_SHIP_W;
		u4_puts("Turn West!\n");
	} else if(Party._tile == TIL_SHIP_W) {
		u4_puts("Sail West!\n");
		if(!IsSailable(tile_west)) {
			w_Blocked();
		} else if(!WindAllows(DIR_W)) {
			w_SlowProgress();
		} else {
			ScrollWest();
		}
	} else {
		if(Party._tile == TIL_HORSE_E)
			Party._tile = TIL_HORSE_W;
		sound(SND_STEP);
		u4_puts("West\n");
		if(MoveWest() && Gallop) {
			RefreshView();
			sound(SND_STEP);
			MoveWest();
		}
	}
}

/*
 * Walk one tile east (see MoveNorth). Arriving at (0xdd, 0xe0) on the
 * world map brings the pirates (SpawnPirates).
 */
/*C_2E4F*/MoveEast()
{
	if(!IsWalkable(tile_east)) {
		w_Blocked();
		return 0;
	}
	if(IsSlowTerrain(tile_east)) {
		w_SlowProgress();
		return 1;
	}
	sound(SND_STEP);
	if(CurMode == MOD_OUTDOORS) {
		ScrollEast();
		if(Party._y == 0xe0 && Party._x == 0xdd)
			SpawnPirates();
		if(CheckMoongate(tile_east))
			return 0;
		return 1;
	}
	Party._x ++;
	WinPos.x = Party._x;
	if(WinPos.x <= 0x1f) {
		return 1;
	}
	LeaveTown();
	return 0;
}


/*
 * The East arrow key (see CMDDIR_Up). A horse turns east; sailing to
 * (0xdd, 0xe0) brings the pirates.
 */
/*C_2EC5*/CMDDIR_Right()
{
	if(Party._tile == TIL_BALLOON) {
		w_DriftOnly();
	} else if(Party._tile == TIL_SHIP_S || Party._tile == TIL_SHIP_W || Party._tile == TIL_SHIP_N) {
		Party._tile = TIL_SHIP_E;
		u4_puts("Turn East!\n");
	} else if(Party._tile == TIL_SHIP_E) {
		u4_puts("Sail East!\n");
		if(!IsSailable(tile_east)) {
			w_Blocked();
		} else if(!WindAllows(DIR_E)) {
			w_SlowProgress();
		} else {
			ScrollEast();
			if(Party._y == 0xe0 && Party._x == 0xdd)
				SpawnPirates();
		}
	} else {
		if(Party._tile == TIL_HORSE_W)
			Party._tile = TIL_HORSE_E;
		sound(SND_STEP);
		u4_puts("East\n");
		if(MoveEast() && Gallop) {
			RefreshView();
			sound(SND_STEP);
			MoveEast();
		}
	}
}
