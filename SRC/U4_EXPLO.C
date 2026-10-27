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

/*load casle/towne/village files*/
/*C_3E30*/LoadTown(bp04)
unsigned bp04;
{
	if(Save(/*D_172B*/"OUTMONST.SAV", sizeof(struct tNPC), &(Area._npc)) == -1)
		exit(3);
	if(Load(TownFiles[bp04 - 0x01], sizeof(struct tArea), &Area) == -1)
		exit(3);
	File_TLK = dopen(TalkFiles[Party._loc - 1], 0);
	CurMode = MOD_BUILDING;
}

/*load dungeon files*/
/*C_3E94*/LoadDungeon()
{
	register int si;

	if(Load(DungeonFiles[Party._loc - 0x11], sizeof(tMap8x8x8), &(Area._map)) == -1)
		exit(3);
	File_DNG = dopen(DungeonFiles[Party._loc - 0x11], 0);
	for(si = 0x1f; si >= 0; si --)
		Area._npc._tile[si] = 0;
}

/*C_3EE4*/StartDungeon()
{
	LoadDungeon();
	CurMode = MOD_DUNGEON;
	Party._dir = DIR_E;
	Party._z = 0;
	DNG_main();
}

/*enter dungeon*/
/*C_3F03*/EnterDungeon()
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

/*enter towne*/
/*C_3F4A*/EnterTown()
{
	register int si;

	u4_puts(Names[127 + Party._loc - 1]);
	Gra_CR();
	Gra_CR();
	Party._y = WinPos.y = 15;
	Party._x = WinPos.x = 1;
	LoadTown(Party._loc);

	if((Party._loc - 0x05) < 8) {
		for(si = Party._members; --si >= 1; ) {
			if(Party.chara[si]._class == (Party._loc - 0x05)) {
				Area._npc._tile[31] = Area._npc._gtile[31] = Area._npc._var[31] = 0;
			}
		}
	}
}

/*enter the abyss?*/
/*C_3FB9*/EnterAbyss()
{
	if((
		(Party.mItems >> ST_BOOK) &
		(Party.mItems >> ST_BELL) &
		(Party.mItems >> ST_CANDLE) &
		(Party.mItems >> ST_USE_BOOK) &
		(Party.mItems >> ST_USE_BELL) &
		(Party.mItems >> ST_USE_CANDLE)
	) & 1) {
		u4_puts(Names[127 + Party._loc - 1]);
		EnterDungeon();
	} else {
		Party._loc = 0;
		w_Cant_t();
	}
}

/*C_4018*/CMD_Enter()
{
	register int si;

	u4_puts(/*D_1765*/"Enter ");
	if(Party._loc != 0 || Party._tile == TIL_BALLOON) {
		w_What();
		return;
	}
	si = 31;
	while(si >= 0) {
#ifdef WIN32
		if(Party._x == (unsigned char)LocationX[si] && Party._y == (unsigned char)LocationY[si])
#else
		if(Party._x == LocationX[si] && Party._y == LocationY[si])
#endif
			break;
		si--;
	}
	if(si == -1) {
		w_What();
		return;
	}
	Party._loc = si + 1;
	Party.out_x = Party._x;
	Party.out_y = Party._y;
	switch(Area._map.x32x32[WinPos.y][WinPos.x]) {
		case TIL_DUNGEON:
			u4_puts(/*D_176C*/"dungeon!\n\n");
			u4_puts(Names[127 + Party._loc - 1]);
			EnterDungeon();
		break;
		case TIL_TOWN:
			u4_puts(/*D_1777*/"towne!\n\n");
			EnterTown();
		break;
		case TIL_CASTLE: case TIL_LCB_ENTRANCE:
			u4_puts(/*D_1780*/"castle!\n\n");
			u4_puts(Names[127 + Party._loc - 1]);
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
			u4_puts(Names[151 + Party._loc - 0x19]);
			Gra_CR();
			EnterShrine();
		break;
		default:
			w_What();
			return;
	}
}

/*C_4164*/LeaveVehicle()
{
	register int si;

	for(si = 31; si >= 8 ;si --) {
		if(Area._npc._tile[si] == 0)
			break;
	}
	if(si == 7)
		si = U4_RND1(15) | 0x10;
	Area._npc._gtile[si] = Area._npc._tile[si] = Party._tile;
	Area._npc._x[si] = Party._x;
	Area._npc._y[si] = Party._y;
	Area._npc._var[si] = Area._npc._tlkidx[si] = 0;
	Party._tile = TIL_AVATAR;
	Gra_CR();
}

/*C_41C0*/CMD_X_it()
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

/*C_4206*/BoardVehicle(bp04)
unsigned bp04;
{
	register int si;

	for(si = 31; si >= 8; si --) {
		if(
			Party._x == Area._npc._x[si] &&
			Party._y == Area._npc._y[si] &&
			bp04 == (Area._npc._tile[si] & ~3)
		) break;
	}
	if(si != 7)
		Area._npc._gtile[si] = Area._npc._tile[si] = 0;
		Party._tile = tile_cur;
}

/*C_4253*/CMD_Board()
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

/*C_42E8*/CMD_Yell()
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

unsigned /*D_17FA*/DoorX = 0;/*current opened door's x*/
unsigned /*D_17FC*/DoorY = 0;/*current opened door's y*/
unsigned /*D_17FE*/DoorTimer = 0;/*current opened door's delay*/

/*automatic door close*/
/*C_431D*/AutoCloseDoor()
{
	if(CurMode == MOD_BUILDING && DoorTimer) {
		if(--DoorTimer == 0)
			Area._map.x32x32[DoorY][DoorX] = TIL_DOOR;
	} else {
		DoorTimer = 0;
	}
}

/*C_4353*/CMD_Open()
{
	unsigned bp_02, bp_04;

	while(DoorTimer)
		AutoCloseDoor();
	if(CurMode != MOD_BUILDING) {
		u4_puts(/*D_1800*/"Open; ");
		w_NotHere();
		return;
	}
	AskDir(/*D_1807*/"Open: ", &bp_02, &bp_04);
	if(!(bp_02 | bp_04))
		return;
	if((bp_02 + Party._x) > 0x1f || bp_04) {
		w_NotHere();
	} else {
		register unsigned char *si;

		si = &(Area._map.x32x32[Party._y][bp_02 + Party._x]);
		if(*si == TIL_LOCKED_DOOR) {
			w_Cant_t();
		} else if(*si == TIL_DOOR) {
			DoorX = (bp_02 + Party._x);
			DoorY = Party._y;
			DoorTimer = 5;
			*si = TIL_BRICK_FLOOR;
			u4_puts(/*D_180E*/"\nOpened!\n");
		} else {
			w_NotHere();
		}
	}
}

/*C_43F8*/CMD_Jimmy()
{
	int bp_04, bp_02;

	u4_puts(/*D_1818*/"Jimmy lock!\n");
	AskDir(/*D_1825*/"Dir: ", &bp_02, &bp_04);
	if(!(bp_02 | bp_04))
		return;
	if(CurMode != MOD_BUILDING || (bp_02 + Party._x) > 0x1f || bp_04) {
		w_NotHere();
	} else {
		register unsigned char *si;

		si = &(Area._map.x32x32[Party._y][bp_02 + Party._x]);
		if(*si != TIL_LOCKED_DOOR) {
			w_NotHere();
			return;
		}
		if(Party._keys == 0) {
			u4_puts(/*D_182B*/"No keys left!\n");
			return;
		}
		Party._keys --;
		*si = TIL_DOOR;
	}
}

/*C_4477*/CMD_Klimb()
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
	if(Party._loc == 1 && tile_cur == TIL_LADDER_UP) {
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

/*C_44EE*/CMD_Descend()
{
	if(Party._tile == TIL_BALLOON) {
		u4_puts(/*D_1867*/"Land Balloon\n");
		if(tile_cur != TIL_GRASS) {
			sound(1);
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
	if(Party._loc != 0x01) {
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
		Party._loc = 0x17;
		StartDungeon();
		return;
	}
	u4_puts(/*D_18AA*/"to first floor!\n");
	if(Load(/*D_18BB*/"LCB_1.ULT", sizeof(struct tArea), &Area) == -1)
		exit(3);
}
