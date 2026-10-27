/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <stdlib.h>

char * /*D_1738*/TalkFiles[] ={
	/*Castles*/
	/*D_1682*/"LCB.TLK",
	/*D_168A*/"LYCAEUM.TLK",
	/*D_1696*/"EMPATH.TLK",
	/*D_16A1*/"SERPENT.TLK",
	/*Townes*/
	/*D_16AD*/"MOONGLOW.TLK",
	/*D_16BA*/"BRITAIN.TLK",
	/*D_16C6*/"JHELOM.TLK",
	/*D_16D1*/"YEW.TLK",
	/*D_16D9*/"MINOC.TLK",
	/*D_16E3*/"TRINSIC.TLK",
	/*D_16EF*/"SKARA.TLK",
	/*D_16F9*/"MAGINCIA.TLK",
	/*Villages*/
	/*D_1706*/"PAWS.TLK",
	/*D_170F*/"DEN.TLK",
	/*D_1717*/"VESPER.TLK",
	/*D_1722*/"COVE.TLK"
};

/*
 * Save the outdoor monsters, then load the map and npcs of castle/towne/village
 * loc (LOC_*) and open its dialogue file (indexed by Party._loc, not loc).
 */
int16 /*C_3E30*/LoadTown(loc)
uint16 loc;
{
	if(Save(/*D_172B*/"OUTMONST.SAV", sizeof(struct tNPC), &(Area._npc)) == -1)
		exit(3);
	if(Load(TownFiles[loc - LOC_BUILDINGS], sizeof(struct tArea), &Area) == -1)
		exit(3);
	File_TLK = dopen(TalkFiles[Party._loc - 1], 0);
	CurMode = MOD_BUILDING;
}

/*
 * Load the map of dungeon Party._loc, keep its file open (File_DNG, for the
 * rooms) and remove all the npcs.
 */
int16 /*C_3E94*/LoadDungeon()
{
	register int16 npc;

	if(Load(DungeonFiles[Party._loc - LOC_DUNGEONS], sizeof(tMap8x8x8), &(Area._map)) == -1)
		exit(3);
	File_DNG = dopen(DungeonFiles[Party._loc - LOC_DUNGEONS], 0);
	for(npc = 0x1f; npc >= 0; npc --)
		Area._npc._tile[npc] = 0;
}

/*Load dungeon Party._loc and play it, from level 1 facing east.*/
int16 /*C_3EE4*/StartDungeon()
{
	LoadDungeon();
	CurMode = MOD_DUNGEON;
	Party._dir = DIR_E;
	Party._z = 0;
	DNG_main();
}

/*
 * Enter dungeon Party._loc (on foot only): save the outdoor monsters and play
 * the dungeon from (1, 1).
 */
int16 /*C_3F03*/EnterDungeon()
{
	Gra_CR();
	Gra_CR();
	if(Party._tile != TIL_AVATAR) {
		Party._loc = 0;
		w_OnlyOnFoot();
		return;
	}
	Party._x = Party._y = 1;
	if(Save(/*D_1758*/"OUTMONST.SAV", sizeof(struct tNPC), &(Area._npc)) == -1)
		exit(3);
	StartDungeon();
}

/*
 * Enter towne/village/ruin Party._loc at its west edge (1, 15).
 * In the eight townes, npc #31 is the companion of the towne's class (Moonglow:
 * mage, Britain: bard, ...); remove it if that companion has joined the party.
 */
int16 /*C_3F4A*/EnterTown()
{
	register int16 member;

	u4_puts(Names[NAME_LOCATIONS + Party._loc - 1]);
	Gra_CR();
	Gra_CR();
	Party._y = WinPos.y = 15;
	Party._x = WinPos.x = 1;
	LoadTown(Party._loc);

	if((Party._loc - LOC_TOWNS) < 8) {
		for(member = Party._members; --member >= 1; ) {
			if(Party.chara[member]._class == (Party._loc - LOC_TOWNS)) {
				Area._npc._tile[31] = Area._npc._gtile[31] = Area._npc._var[31] = 0;
			}
		}
	}
}

/*
 * Enter the Great Stygian Abyss: only with the bell, book and candle, all
 * three already used at the entrance; otherwise "Can't".
 */
int16 /*C_3FB9*/EnterAbyss()
{
	if((
		(Party.mItems >> ST_BOOK) &
		(Party.mItems >> ST_BELL) &
		(Party.mItems >> ST_CANDLE) &
		(Party.mItems >> ST_USE_BOOK) &
		(Party.mItems >> ST_USE_BELL) &
		(Party.mItems >> ST_USE_CANDLE)
	) & 1) {
		u4_puts(Names[NAME_LOCATIONS + Party._loc - 1]);
		EnterDungeon();
	} else {
		Party._loc = 0;
		w_Cant_t();
	}
}

/*
 * (E)nter: find the location at the party's position (LocationX/Y) and enter
 * it according to the tile there. Lava enters the Abyss only at (0xe9, 0xe9)
 * (falls through to the fire field case).
 */
int16 /*C_4018*/CMD_Enter()
{
	register int16 place;

	u4_puts(/*D_1765*/"Enter ");
	if(Party._loc != 0 || Party._tile == TIL_BALLOON) {
		w_What();
		return;
	}
	place = 31;
	while(place >= 0) {
		/*MSC compares the bytes (cmp [si+LocationX],al): the casts say so*/
		if(Party._x == (unsigned char)LocationX[place] && Party._y == (unsigned char)LocationY[place])
			break;
		place--;
	}
	if(place == -1) {
		w_What();
		return;
	}
	Party._loc = place + 1;
	Party.out_x = Party._x;
	Party.out_y = Party._y;
	switch(Area._map.x32x32[WinPos.y][WinPos.x]) {
		case TIL_DUNGEON:
			u4_puts(/*D_176C*/"dungeon!\n\n");
			u4_puts(Names[NAME_LOCATIONS + Party._loc - 1]);
			EnterDungeon();
		break;
		case TIL_TOWN:
			u4_puts(/*D_1777*/"towne!\n\n");
			EnterTown();
		break;
		case TIL_CASTLE: case TIL_LCB_ENTRANCE:
			u4_puts(/*D_1780*/"castle!\n\n");
			u4_puts(Names[NAME_LOCATIONS + Party._loc - 1]);
			Gra_CR();
			Gra_CR();
			Party._y = WinPos.y = 0x1e;
			Party._x = WinPos.x = 0x0f;
			Party._z = 0;
			LoadTown(Party._loc);
		break;
		case TIL_VILLAGE:
			u4_puts(/*D_178A*/"village!\n\n");
			EnterTown();
		break;
		case TIL_RUINS:
			u4_puts(/*D_1795*/"ruin!\n\n");
			EnterTown();
		break;
		case TIL_LAVA:
			if(Party._x != 0xe9 || Party._y != 0xe9) {
				w_What();
				return;
			}
		case TIL_FIRE_FIELD:
			EnterAbyss();
		break;
		case TIL_SHRINE:
			u4_puts(/*D_179D*/"the Shrine of\n");
			u4_puts(Names[NAME_VIRTUES + Party._loc - LOC_SHRINES]);
			Gra_CR();
			EnterShrine();
		break;
		default:
			w_What();
			return;
	}
}

/*
 * Leave the vehicle as an npc on the map (in a free slot among #8-#31, else
 * overwriting one of #16-#31 at random) and continue on foot.
 */
int16 /*C_4164*/LeaveVehicle()
{
	register int16 npc;

	for(npc = 31; npc >= 8 ;npc --) {
		if(Area._npc._tile[npc] == 0)
			break;
	}
	if(npc == 7)
		npc = U4_RND1(15) | 0x10;
	Area._npc._gtile[npc] = Area._npc._tile[npc] = Party._tile;
	Area._npc._x[npc] = Party._x;
	Area._npc._y[npc] = Party._y;
	Area._npc._var[npc] = Area._npc._tlkidx[npc] = 0;
	Party._tile = TIL_AVATAR;
	Gra_CR();
}

/*
 * X-it the ship (remembering where it was left, see CMD_Board), the horse
 * (stops galloping) or the balloon (only once landed).
 */
int16 /*C_41C0*/CMD_X_it()
{
	u4_puts(/*D_17AC*/"X-it ");
	if(Party._tile < TIL_HORSE_W) {
		ship_x = Party._x;
		ship_y = Party._y;
		LeaveVehicle();
	} else if(Party._tile < TIL_TILED_FLOOR) {
		Gallop = 0;
		LeaveVehicle();
	} else if(Party._tile == TIL_BALLOON && Party._fly_light == 0) {
		LeaveVehicle();
	} else {
		w_What();
	}
}

/*
 * Remove the vehicle npc of kind vehicle (tile with the direction bits
 * cleared) at the party's position, and take its tile as the party's tile.
 * Note the misleading indentation: the last statement is not part of the if.
 */
int16 /*C_4206*/BoardVehicle(vehicle)
uint16 vehicle;
{
	register int16 npc;

	for(npc = 31; npc >= 8; npc --) {
		if(
			Party._x == Area._npc._x[npc] &&
			Party._y == Area._npc._y[npc] &&
			vehicle == (Area._npc._tile[npc] & ~3)
		) break;
	}
	if(npc != 7)
		Area._npc._gtile[npc] = Area._npc._tile[npc] = 0;
		Party._tile = tile_cur;
}

/*
 * Board a horse, the balloon or a ship. A ship other than the one the party
 * last left (ship_x, ship_y) comes with a fully repaired hull (50).
 */
int16 /*C_4253*/CMD_Board()
{
	if(Party._tile != TIL_AVATAR) {
		u4_puts(/*D_17B2*/"Board: ");
		w_Cant_t();
		return;
	}
	if(tile_cur == TIL_HORSE_W || tile_cur == TIL_HORSE_E) {
		BoardVehicle(TIL_HORSE_W);
		u4_puts(/*D_17BA*/"Mount Horse!\n");
		return;
	}
	u4_puts(/*D_17C8*/"Board ");
	if(tile_cur == TIL_BALLOON) {
		BoardVehicle(TIL_BALLOON);
		u4_puts(/*D_17CF*/"Balloon!\n");
		Party._fly_light = 0;
		return;
	}
	if(tile_cur < TIL_SHIP_W || tile_cur > TIL_SHIP_S) {
		w_What();
		return;
	}
	BoardVehicle(TIL_SHIP_W);
	u4_puts(/*D_17D9*/"Frigate!\n");
	if(ship_x != Party._x || ship_y != Party._y)
		Party._ship = 50;
}

/*Yell: on horseback, toggle galloping.*/
int16 /*C_42E8*/CMD_Yell()
{
	u4_puts(/*D_17E3*/"Yell ");
	if(Party._tile == TIL_HORSE_W || Party._tile == TIL_HORSE_E) {
		if(Gallop ^= 1)
			u4_puts(/*D_17E9*/"Giddyup!\n");
		else
			u4_puts(/*D_17F3*/"Whoa!\n");
	} else {
		w_What();
	}
}

uint16 /*D_17FA*/DoorX = 0;/*current opened door's x*/
uint16 /*D_17FC*/DoorY = 0;/*current opened door's y*/
uint16 /*D_17FE*/DoorTimer = 0;/*current opened door's delay*/

/*Count down the opened door's timer and close the door when it expires.*/
int16 /*C_431D*/AutoCloseDoor()
{
	if(CurMode == MOD_BUILDING && DoorTimer) {
		if(--DoorTimer == 0)
			Area._map.x32x32[DoorY][DoorX] = TIL_DOOR;
	} else {
		DoorTimer = 0;
	}
}

/*
 * Open a door for 5 turns (closes any door still open first). Only doors to
 * the east or the west can be opened: any north/south direction is "Not here".
 */
int16 /*C_4353*/CMD_Open()
{
	uint16 dx, dy;

	while(DoorTimer)
		AutoCloseDoor();
	if(CurMode != MOD_BUILDING) {
		u4_puts(/*D_1800*/"Open; ");
		w_NotHere();
		return;
	}
	AskDir(/*D_1807*/"Open: ", &dx, &dy);
	if(!(dx | dy))
		return;
	if((dx + Party._x) > 0x1f || dy) {
		w_NotHere();
	} else {
		register unsigned char *p_door;

		p_door = &(Area._map.x32x32[Party._y][dx + Party._x]);
		if(*p_door == TIL_LOCKED_DOOR) {
			w_Cant_t();
		} else if(*p_door == TIL_DOOR) {
			DoorX = (dx + Party._x);
			DoorY = Party._y;
			DoorTimer = 5;
			*p_door = TIL_BRICK_FLOOR;
			u4_puts(/*D_180E*/"\nOpened!\n");
		} else {
			w_NotHere();
		}
	}
}

/*
 * Jimmy a locked door with a key (east or west only, as with Open).
 * The door stays a normal closed door.
 */
int16 /*C_43F8*/CMD_Jimmy()
{
	int16 dy, dx;

	u4_puts(/*D_1818*/"Jimmy lock!\n");
	AskDir(/*D_1825*/"Dir: ", &dx, &dy);
	if(!(dx | dy))
		return;
	if(CurMode != MOD_BUILDING || (dx + Party._x) > 0x1f || dy) {
		w_NotHere();
	} else {
		register unsigned char *p_door;

		p_door = &(Area._map.x32x32[Party._y][dx + Party._x]);
		if(*p_door != TIL_LOCKED_DOOR) {
			w_NotHere();
			return;
		}
		if(Party._keys == 0) {
			u4_puts(/*D_182B*/"No keys left!\n");
			return;
		}
		Party._keys --;
		*p_door = TIL_DOOR;
	}
}

/*
 * Klimb: outdoors, make the balloon fly (no line of sight while flying);
 * in Lord British's castle, take the ladder up to the second floor.
 */
int16 /*C_4477*/CMD_Klimb()
{
	u4_puts(/*D_183A*/"Klimb ");
	if(Party._loc == 0) {
		if(Party._tile != TIL_BALLOON) {
			w_What();
			return;
		}
		u4_puts(/*D_1841*/"altitude\n");
		Party._fly_light = 1;
		LineOfSight = 0;
		return;
	}
	if(Party._loc == LOC_LCB && tile_cur == TIL_LADDER_UP) {
		if(Party._tile != TIL_AVATAR) {
			Gra_CR();
			w_OnlyOnFoot();
			return;
		}
		u4_puts(/*D_184B*/"to second floor!\n");
		if(Load(/*D_185D*/"LCB_2.ULT", sizeof(struct tArea), &Area) == -1)
			exit(3);
	} else {
		w_What();
	}
}

/*
 * Descend: land the balloon (on grass only); in Lord British's castle, take
 * the ladder down to the first floor, or from the first floor (the ladder at
 * y == 2) into the dungeon Hythloth.
 */
int16 /*C_44EE*/CMD_Descend()
{
	if(Party._tile == TIL_BALLOON) {
		u4_puts(/*D_1867*/"Land Balloon\n");
		if(tile_cur != TIL_GRASS) {
			sound(SND_BEEP);
			w_NotHere();
			return;
		}
		if(Party._fly_light == 0) {
			u4_puts(/*D_1875*/"Already Landed!\n");
			return;
		}
		Party._fly_light = 0;
		LineOfSight = 1;
		return;
	}
	if(Party._loc != LOC_LCB) {
		u4_puts(/*D_1886*/"Descend ");
		w_What();
		return;
	}
	/*in LB's castle*/
	if(Party._tile != TIL_AVATAR) {
		w_OnlyOnFoot();
		return;
	}
	u4_puts(/*D_188F*/"Descend ");
	if(tile_cur != TIL_LADDER_DOWN) {
		w_What();
		return;
	}
	if(Party._y == 2) {
		u4_puts(/*D_1898*/"into the depths!\n");
		Party.out_x = 0xef;
		Party.out_y = 0xf0;
		Party._y = Party._x = 5;
		Party._loc = LOC_HYTHLOTH;
		StartDungeon();
		return;
	}
	u4_puts(/*D_18AA*/"to first floor!\n");
	if(Load(/*D_18BB*/"LCB_1.ULT", sizeof(struct tArea), &Area) == -1)
		exit(3);
}
