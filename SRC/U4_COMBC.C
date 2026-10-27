/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>
#include <stdlib.h>

/*----------------------------------------
Starting and ending a combat
----------------------------------------*/

/*the port: the monster and weapon tables below are one block laid out as in DOS
  (port/tools/gen_block.py): MonsterIndex reads past MonsterMaxHP for people*/
#ifdef PORT_BLOCK
#include "block_u4_combc.h"
#else
/*base for enemy HP/XP/attack, indexed by MonsterIndex()*/
unsigned char /*D_23D2*/MonsterMaxHP[] = {
	/*80~8f*/
	0xFF,/*pirate*/
	0xFF,/*pirate*/
	0x40,/*nixie*/
	0x60,/*squid*/
	0x80,/*serpent*/
	0x60,/*seahorse*/
	0xFF,/*whirlpool*/
	0xFF,/*twister*/
	/*90~AF*/
	0x30,/*rat*/
	0x30,/*bat*/
	0x40,/*spider*/
	0x50,/*ghost*/
	0x30,/*slime*/
	0x60,/*troll*/
	0x30,/*gremlin*/
	0xC0,/*mimic*/
	0xFF,/*reaper*/
	0x30,/*insects*/
	0xF0,/*gazer*/
	0x80,/*phantom*/
	0x50,/*orc*/
	0x30,/*skeleton*/
	0x50,/*rogue*/
	0x30,/*python*/
	0x70,/*ettin*/
	0x40,/*headless*/
	0x80,/*cyclops*/
	0x40,/*wisp*/
	0xB0,/*mage*/
	0xC0,/*lyche*/
	0x60,/*lava lizard*/
	0xF0,/*zorn*/
	0x70,/*daemon*/
	0xD0,/*hydra*/
	0xE0,/*dragon*/
	0xFF,/*balron*/
	/*20~2f[buggy]*/
	0x70,/*mage*/
	0x30,/*bard*/
	0x60,/*fighter*/
	0x40,/*druid*/
	0x60,/*tinker*/
	0x80,/*paladin*/
	0x90,/*ranger*/
	0x30,/*shepherd*/
	/*50~5f[buggy]*/
	0x80,/*guard*/
	0x30,/*merchant*/
	0x30,/*bard*/
	0x30,/*jester*/
	0x20,/*beggar/sleep*/
	0x20,/*child*/
	0x80,/*bull*/
	0xFF /*lord british*/
};

/*the foe that may join a group of this foe (see PlaceFoes)*/
unsigned char /*D_2406*/MonsterPartner[] = {
	/*80~8f*/
	TIL_ROGUE,/*pirate->rogue*/
	TIL_ROGUE,/*pirate->rogue*/
	TIL_SEAHORSE,/*nixie->seahorse*/
	TIL_SERPENT,/*squid->serpent*/
	TIL_SQUID,/*serpent->squid*/
	TIL_NIXIE,/*seahorse->nixie*/
	TIL_WHIRLPOOL,/*whirlpool->whirlpool*/
	TIL_TWISTER,/*twister->twister*/
	/*90~AF*/
	TIL_SKELETON,/*rat->skeleton*/
	TIL_LAVA_LIZARD_1,/*bat->lava lizard*/
	TIL_RAT,/*spider->rat*/
	TIL_LICHE,/*ghost->lyche*/
	TIL_SLIME,/*slime->slime*/
	TIL_ETTIN,/*troll->ettin*/
	TIL_GREMLIN,/*gremlin->gremlin*/
	TIL_MIMIC,/*mimic->mimic*/
	TIL_REAPER,/*reaper->reaper*/
	TIL_RAT,/*insects->rat*/
	TIL_PHANTOM,/*gazer->phantom*/
	TIL_GHOST,/*phantom->ghost*/
	TIL_TROLL,/*orc->troll*/
	TIL_EVIL_MAGE,/*skeleton->mage*/
	TIL_ROGUE,/*rogue->rogue*/
	TIL_RAT,/*python->rat*/
	TIL_DAEMON,/*ettin->daemon*/
	TIL_GAZER,/*headless->gazer*/
	TIL_ZORN,/*cyclops->zorn*/
	TIL_PHANTOM,/*wisp->phantom*/
	TIL_DAEMON,/*mage->daemon*/
	TIL_DAEMON,/*lyche->daemon*/
	TIL_HYDRA,/*lava lizard->hydra*/
	TIL_GAZER,/*zorn->gazer*/
	TIL_BALRON,/*daemon->balron*/
	TIL_DRAGON,/*hydra->dragon*/
	TIL_BALRON,/*dragon->balron*/
	TIL_BALRON /*balron->balron*/
};

/*maximum size of a group of this foe (see PlaceFoes)*/
unsigned char /*D_242A*/MonsterGroupSize[] = {
	/*80~8F*/
	 1,/*pirate*/
	 1,/*pirate*/
	12,/*nixie*/
	 4,/*squid*/
	 4,/*serpent*/
	 8,/*seahorse*/
	 1,/*whirlpool*/
	 1,/*twister*/
	/*90~AF*/
	12,/*rat*/
	12,/*bat*/
	 6,/*spider*/
	 4,/*ghost*/
	15,/*slime*/
	 6,/*troll*/
	15,/*gremlin*/
	 1,/*mimic*/
	 1,/*reaper*/
	15,/*insects*/
	 4,/*gazer*/
	 8,/*phantom*/
	10,/*orc*/
	12,/*skeleton*/
	10,/*rogue*/
	12,/*python*/
	 6,/*ettin*/
	 8,/*headless*/
	 6,/*cyclops*/
	12,/*wisp*/
	 6,/*mage*/
	 4,/*lyche*/
	 8,/*lava lizard*/
	 4,/*zorn*/
	 6,/*daemon*/
	 4,/*hydra*/
	 4,/*dragon*/
	 1,/*balron*/
	/* */
/*__244e*/0, 0/*padding? (read by the buggy MonsterIndex(0))*/
};

/*weapons damage*/
unsigned char /*D_2450*/WeaponDamage[] = {0x08,0x10,0x18,0x20,0x28,0x30,0x40,0x28,0x38,0x40,0x60,0x60,0x80,0x50,0xA0,0xFF};
/*armors protection*/
unsigned char /*D_2460*/ArmorDefense[] = {0x60,0x80,0x90,0xA0,0xB0,0xC0,0xD0,0xF8};
/*range weapons flag*/
unsigned char /*D_2468*/WeaponIsRanged[] = {0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xFF,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0x00};
#endif

/*
 * Index of a foe tile in the monster tables (MonsterMaxHP...): the sea monsters
 * (2-tile animations, 0x80-0x8F) give 0-7, the others (4-tile animations) 8-35.
 * The humans (tiles below 0x80) were meant to give 36-51 (see below).
 */
int16 /*C_7C25*/MonsterIndex(tile)
unsigned char tile;
{
	if(tile >= TIL_PIRATE_W) {
		tile &= 0x7f;
		if((tile >> 1) < 8)
			return tile >> 1;
		return (tile >> 2) + 4;
	}
	/* not monster */
#ifdef WIN32
	return ((tile&0x1f) >> 1) + 0x24;
#else
	/*this looks like an original bug:
	the values from MonsterMaxHP are not
	correctly indexed.
	Without the patch the index will go
	out-of-bound and access unrelated values
	from MonsterPartner. Hence, except for the mage
	and the fighter, all the non-monster
	livings will get faulty values.
	On the original environment:
	  mage         0x70 should be 0x70
	  bard         0x60 should be 0x30
	  fighter      0x60 should be 0x60
	  druid        0x90 should be 0x40
	  tinker       0x80 should be 0x60
	  paladin      0x30 should be 0x80
	  ranger       0x20 should be 0x90
	  shepherd     0x80 should be 0x30
	  guard        0xC8 should be 0x80
	  merchant     0x8A should be 0x30
	  bard         0x86 should be 0x30
	  jester       0x8C should be 0x30
	  beggar/sleep 0xC4 should be 0x20
	  child        0x90 should be 0x20
	  bull         0xA0 should be 0x80
	  lord british 0xA8 should be 0xFF

	  because of different padding/alignment
	  policies, we may get different values
	  with another compiler.
	 (issue raised by @plaidpants,
	 with cooperation from @Fenyx4)
	*/
	return (tile&0x1f) + 0x24;
#endif
}

char * /*D_2512*/CombatMapFiles[] = {
	/*D_2478*//* 0*/"GRASS.CON",
	/*D_2482*//* 1*/"BRIDGE.CON",
	/*D_248D*//* 2*/"BRICK.CON",
	/*D_2497*//* 3*/"DUNGEON.CON",
	/*D_24A3*//* 4*/"HILL.CON",
	/*D_24AC*//* 5*/"FOREST.CON",
	/*D_24B7*//* 6*/"BRUSH.CON",
	/*D_24C1*//* 7*/"MARSH.CON",
	/*D_24CB*//* 8*/"SHIPSEA.CON",
	/*D_24D7*//* 9*/"SHIPSHOR.CON",
	/*D_24E4*//*10*/"SHORE.CON",
	/*D_24EE*//*11*/"SHIPSHIP.CON",
	/*D_24FB*//*12*/"SHORSHIP.CON",
	/*D_2508*//*13*/"CAMP.CON"
};

/*
 * Load the combat map (.CON) of an outdoor/town combat: against a pirate ship, the
 * ship-to-ship or shore-to-ship map (and the foes are rogues); else, when the party
 * is on a ship, the ship-at-sea or ship-at-shore map; when the foe is on water, the
 * shore map; else the map of the terrain under the party.
 */
int16 /*C_7C65*/LoadCombatMap()
{
	register int16 map;

	if(Party._tile <= TIL_SHIP_S || (tile_cur & ~3) == TIL_SHIP_W) {
		if(CombatFoeTile == TIL_PIRATE_W) {
			CombatFoe = TIL_ROGUE;
			map = 11;
		} else if(CombatGround < TIL_SWAMP) {
			map = 8;
		} else {
			map = 9;
		}
	} else {
		if(CombatFoeTile == TIL_PIRATE_W) {
			CombatFoe = TIL_ROGUE;
			map = 12;
		} else if(CombatGround < TIL_SWAMP) {
			map = 10;
		} else {
			switch(tile_cur) {
				case TIL_SWAMP: map = 7; break;
				case TIL_SCRUB: map = 6; break;
				case TIL_FOREST: map = 5; break;
				case TIL_HILLS: map = 4; break;
				case TIL_DUNGEON: map = 3; break;
				case TIL_BRICK_FLOOR: map = 2; break;
				case TIL_BRIDGE:
				case TIL_BRIDGE_N:
				case TIL_BRIDGE_S:
				case TIL_WOOD_FLOOR: map = 1; break;
				default: map = 0;
			}
		}
	}
#ifdef WIN32
	if(Load(CombatMapFiles[map], sizeof(struct tCombat), &Combat) == -1)
#else
	if(Load(CombatMapFiles[map], (char *)&Fighters - (char *)&Combat, &Combat) == -1)
#endif
		exit(3);
}

/*combat map for each kind of dungeon tile (tile_cur >> 4): corridor, ladders,
 chest, door (5) and secret door (6)*/
int16 /*D_256E*/DngRoomMapIndex[] = {0,1,2,3,4,0,0,0,0,0,0,0,5,0,6,0};

char * /*D_258E*/DngCombatMapFiles[] = {
	/*D_252E*/"DNG0.CON",
	/*D_2537*/"DNG1.CON",
	/*D_2540*/"DNG2.CON",
	/*D_2549*/"DNG3.CON",
	/*D_2552*/"DNG4.CON",
	/*D_255B*/"DNG5.CON",
	/*D_2564*/"DNG6.CON"
};

/*
 * Combat with a monster met in a dungeon corridor: the map depends on the tile the
 * party stands on.
 */
int16 /*C_7D50*/DungeonCombat()
{
#ifdef WIN32
	if(Load(DngCombatMapFiles[DngRoomMapIndex[tile_cur>>4]], sizeof(struct tCombat), &Combat) == -1)
#else
	if(Load(DngCombatMapFiles[DngRoomMapIndex[tile_cur>>4]], (char *)&Fighters - (char *)&Combat, &Combat) == -1)
#endif
		exit(3);
	RunCombat();
	Redraw3D = 1;	/*force 3d render*/
}

/*
 * Combat at night in an inn (see InnAmbush).
 */
int16 /*C_7D92*/InnCombat()
{
#ifdef WIN32
	if(Load(/*D_259C*/"INN.CON", sizeof(struct tCombat), &Combat) == -1)
#else
	if(Load(/*D_259C*/"INN.CON", (char *)&Fighters - (char *)&Combat, &Combat) == -1)
#endif
		exit(3);
	RunCombat();
}

/*
 * Start an outdoor/town combat against CombatFoeTile: clear the fighters, load
 * the map, fight.
 */
int16 /*C_7DBC*/CombatBegin()
{
	register int16 slot;

	u4_puts(/*D_25A4*/"\n\n**** Combat ****\n\n");
	CombatFoe = CombatFoeTile;
	for(slot = 15; slot >= 0; slot --) {
		Fighters._tile[slot] = 
		Fighters._x[slot] = 
		Fighters._y[slot] = 
		Fighters._sleeping[slot] = 0;
	}
	for(slot = 7; slot >= 0; slot --)
		Fighters._chtile[slot] = 0;
	LoadCombatMap();
	RunCombat();
}

/*
 * Start a combat against the NPC in slot npc: remember its tile, position and the
 * ground under it (outdoors the map is the 32x32 window at WinBlock), remove it
 * from the map, fight.
 */
int16 /*C_7DFE*/StartCombat(npc)
register uint16 npc;
{
	CombatFoeTile = Area._npc._tile[npc];
	CombatFoeX = Area._npc._x[npc];
	CombatFoeY = Area._npc._y[npc];
	if(Party._loc == 0x00) {
		CombatGround = Area._map.x32x32[(CombatFoeY - (WinBlock.y<<4))][(CombatFoeX - (WinBlock.x<<4))];
	} else {
		CombatGround = Area._map.x32x32[CombatFoeY][CombatFoeX];
	}
	Area._npc._var[npc] =
	Area._npc._gtile[npc] =
	Area._npc._tile[npc] = 0;
	CombatBegin();
}

/*
 * Choose the foes and give them their HP; place the characters.
 * There are count + 1 foes: in a town, 2 per party member against guards, else 1.
 * Elsewhere, for a monster, usually 2 to 8 (count = U4_RND1(7)); one time in 8 (and
 * always for a human) count comes from MonsterGroupSize, but for a monster it is
 * read with MonsterIndex(0) (count was overwritten by the random number: probable
 * original bug), which gives 0: a single foe (and for a human MonsterIndex is out of
 * the table's bounds). The count is then re-rolled until there are at most 2 foes
 * per party member.
 * Some of the foes (not the first one) may be replaced by the foe's "partner"
 * (1/8), or by the partner's partner (1/32).
 * A foe gets HP = (maxHP / 2) | random(maxHP).
 */
int16 /*C_7E7E*/PlaceFoes()
{
	register int16 count, fighter;
	int16 i, hp;
	uint16 type;

	for(fighter = 15; fighter >= 0; fighter --)
		Fighters._tile[fighter] = 0;
	if(SavedMode == MOD_BUILDING && CurMode != MOD_COM_CAMP) {
		if(CombatFoeTile == TIL_GUARD)
			count = Party._members * 2 - 1;
		else
			count = 0;
	} else if((count = CombatFoe, CombatFoe < TIL_PIRATE_W) || (count = U4_RND1(7)) == 0) {
		count = (U4_RND4(MonsterGroupSize[MonsterIndex(count)]) + MonsterGroupSize[MonsterIndex(count)]) / 2;
	}
	while(fighter = count, (uint16)((count >> 1)) >= Party._members)
		count = U4_RND4(Party._members * 2);
	for(i = fighter; i >= 0; i--) {
		do {
			fighter = U4_RND1(15);
		} while(Fighters._tile[fighter]);
		type = CombatFoe;
		/* add some "partners" */
		if(CombatFoe > TIL_PIRATE_W && i) {
			if(U4_RND1(31) == 0) {
				type = MonsterPartner[MonsterIndex(MonsterPartner[MonsterIndex(CombatFoe)])];
			} else if(U4_RND1(7) == 0) {
				type = MonsterPartner[MonsterIndex(CombatFoe)];
			}
		}
		/* */
		Fighters._tile[fighter] = Fighters._gtile[fighter] = type;
		hp = MonsterMaxHP[MonsterIndex(type)];
		Fighters._HP[fighter] = (hp >> 1) | U4_RND4(hp);
	}
	for(fighter = Party._members - 1; fighter >= 0; fighter --) {
		if(!isCharaAlive(fighter))
			Fighters._chtile[fighter] = 0;
		else
			CharaAnimTile[fighter] = Fighters._chtile[fighter] = CharaTile(fighter);
	}
}

/*
 * Run a combat on the loaded map, then come back to the previous mode (a camp
 * combat stays in MOD_COM_CAMP and does not change SavedMode).
 */
int16 /*C_7FD7*/RunCombat()
{
	if(CurMode != MOD_COM_CAMP) {
		SavedMode = CurMode;
		CurMode = MOD_COMBAT;
	}
	PlaceFoes();
	COM_main();
	CurMode = SavedMode;
}

/*
 * The party enters a dungeon room (tile_cur's low nibble is the room number; in the
 * Abyss each pair of levels has its own 16 rooms): read the room after the 8x8x8 map
 * in the .DNG file, place the characters on the side they enter from, the foes,
 * the map and the triggers, and fight. Room 15 of the dungeons other than the Abyss
 * is an altar room (Truth, Love or Courage, depending on the party's x).
 */
int16 /*C_7FFD*/EnterDungeonRoom()
{
	struct /*dungeon room info*/ {
		/*the 4 triggers (see RoomTriggers)*/
		unsigned char _000[16];
		/*foes*/
		unsigned char _010[16];/*tile*/
		unsigned char _020[16],_030[16];/*x, y*/
		/*characters' positions (x[8], y[8]) for each side of entry*/
		unsigned char _040[8],_048[8];/*from the north (going south)*/
		unsigned char _050[8],_058[8];/*from the east (going west)*/
		unsigned char _060[8],_068[8];/*from the south (going north)*/
		unsigned char _070[8],_078[8];/*from the west (going east)*/
		/*room map (11x11)*/
		unsigned char _080[0x80];
	} room;
	register int16 i, j;

	dlseek(File_DNG, sizeof(tMap8x8x8) + (Party._loc == LOC_ABYSS?
		((Party._z >> 1) * 0x1000) + ((tile_cur & 0xf) * 0x100):
		((tile_cur & 0xf) * 0x100)
	));
	dread(File_DNG, &room, 0x100);
	SavedMode = MOD_DUNGEON;
	CurMode = MOD_COM_ROOM;
	memcpy(RoomTriggerData, room._000, 16);
	AltarRoom = -1;
	if((tile_cur & 0xf) == 0xf && Party._loc < LOC_ABYSS) {
		u4_puts(/*D_25B9*/"\nThe Altar Room of ");
		if(Party._x == 3) {
			u4_puts(/*D_25CD*/"Love\n");
			AltarRoom = 1;
		} else if(Party._x <= 2) {
			u4_puts(/*D_25D3*/"Truth\n");
			AltarRoom = 0;
		} else {
			u4_puts(/*D_25DA*/"Courage\n");
			AltarRoom = 2;
		}
	}
	switch(((Party._dir - 1) ^ 2) & 3) {
		case 2:
			for(i = 7; i >= 0; i --) {
				Combat._charaX[i] = room._060[i];
				Combat._charaY[i] = room._068[i];
			}
		break;
		case 0:
			for(i = 7; i >= 0; i --) {
				Combat._charaX[i] = room._040[i];
				Combat._charaY[i] = room._048[i];
			}
		break;
		case 3:
			for(i = 7; i >= 0; i --) {
				Combat._charaX[i] = room._070[i];
				Combat._charaY[i] = room._078[i];
			}
		break;
		case 1:
			for(i = 7; i >= 0; i --) {
				Combat._charaX[i] = room._050[i];
				Combat._charaY[i] = room._058[i];
			}
		break;
	}
	for(i = Party._members; --i >= 0; ) {
		if(isCharaAlive(i)) {
			CharaAnimTile[i] = Fighters._chtile[i] = CharaTile(i);
		} else {
			Fighters._chtile[i] = 0;
		}
	}
	for(i = 15; i >= 0; i--) {
/*C_817D*/
		/*original bug: the test is always true, it was probably meant to be
		 "if(room._010[i])"; the empty slots get tile 0 (no foe) anyway*/
		if(room._010) {
			Combat._npcX[i] = room._020[i];
			Combat._npcY[i] = room._030[i];
			Fighters._tile[i] = Fighters._gtile[i] = room._010[i];
			j = MonsterMaxHP[MonsterIndex(room._010[i])];
			Fighters._HP[i] = (j >> 1) | U4_RND4(j);
			if(Fighters._tile[i] == (char)TIL_MIMIC)
				Fighters._gtile[i] = TIL_CHEST;
		}
	}
	for(i = 10; i >= 0; i--)
		for(j = 10; j >= 0; j --)
			Combat._map[i*11+j] = room._080[i*11+j];
	COM_main();
	CurMode = SavedMode;
}

/*
 * After a combat: redraw the dungeon view if needed, flush the keyboard.
 */
int16 /*C_8214*/CombatCleanup()
{
	if(SavedMode == MOD_DUNGEON)
		DNG_LookAround();
	u_kbflush();
}

/*
 * Find a free NPC slot for an object left after a combat, from slot 31 down,
 * random (8-31) if none is free. Outdoors slots 0-7 are meant to be kept for
 * the monsters, but the loop condition is parsed as
 * "(slot >= (SavedMode == MOD_OUTDOORS)) ? 8 : 0" (original bug): outdoors the
 * search goes down to slot 1 (and returns slot 0 when none is free).
 */
int16 /*C_8228*/FreeNpcSlot()
{
	register int16 slot;

	for(slot = 31; slot >= (SavedMode == MOD_OUTDOORS)?8:0; slot--) {
		if(!Area._npc._tile[slot])
			break;
	}
	if(slot == -1 || (slot == 7 && SavedMode == MOD_OUTDOORS))
		slot = U4_RND3(24) + 8;

	return slot;
}

/*
 * After a victory: a pirate ship becomes a free ship; in a dungeon most foes leave
 * a chest (if the square is empty); elsewhere the walking foes leave a chest
 * object where they stood (on walkable ground), except the bats, slimes, gazers,
 * insects and wisps (and the sea monsters).
 * Note: SavedMode is never MOD_COM_CAMP (see CombatEnd), so the first test of the
 * third case is always true.
 */
int16 /*C_8283*/CombatSpoils()
{
	register int16 tile;/*then the NPC slot*/

	tile = CombatFoeTile;
	if(tile == TIL_PIRATE_W) {
		/*pirate ship becomes player ship*/
		tile = FreeNpcSlot();
		Area._npc._x[tile] = CombatFoeX;
		Area._npc._y[tile] = CombatFoeY;
		Area._npc._tile[tile] = Area._npc._gtile[tile] = TIL_SHIP_W;
	} else
	if(SavedMode == MOD_DUNGEON) {
		/*monster leaves chest?*/
		if(tile != TIL_BAT && tile != TIL_INSECTS && tile != TIL_SLIME) {
			if(Area._map.x8x8x8[Party._z][Party._y][Party._x] == 0)
				Area._map.x8x8x8[Party._z][Party._y][Party._x] = 0x40;/*chest*/
		}
	} else
	if(SavedMode != MOD_COM_CAMP || tile_cur < TIL_SHIP_W || tile_cur > TIL_SHIP_S) {
		/*monster leaves chest?*/
		if(
			tile >= TIL_RAT ||
			(tile >= TIL_MAGE && tile <= TIL_SHEPHERD) ||
			(tile >= TIL_GUARD && tile <= TIL_LORD_BRITISH)
		) {
			if(tile != TIL_BAT && tile != TIL_SLIME && tile != TIL_GAZER && tile != TIL_INSECTS && tile != TIL_WISP) {
				if(IsWalkable(CombatGround)) {
					tile = FreeNpcSlot();
					Area._npc._x[tile] = CombatFoeX;
					Area._npc._y[tile] = CombatFoeY;
					Area._npc._tile[tile] = Area._npc._gtile[tile] = TIL_CHEST;
				}
			}
		}
	}
	CombatCleanup();
}

/*altar rooms/dungeons connection
	deceit   - truth
	despise  - love
	destard  - courage
	wrong    - truth love
	covetous - love courage
	shame    - truth courage
	hythloth - truth love courage
 the dungeon reached by leaving an altar room going north, east, south, west*/
unsigned char /*D_261A*/AltarRoomExits[][4] = {
	{LOC_DECEIT,LOC_SHAME,LOC_HYTHLOTH,LOC_WRONG},/*truth*/
	{LOC_DESPISE,LOC_WRONG,LOC_HYTHLOTH,LOC_COVETOUS},/*love*/
	{LOC_DESTARD,LOC_COVETOUS,LOC_HYTHLOTH,LOC_SHAME} /*courage*/
};

/*
 * End of a combat. If no character is left on the map (all dead or fled), outside
 * of a dungeon room: fleeing from non-evil foes raises Compassion and Justice,
 * otherwise "Battle is lost!" lowers Valor.
 * In a dungeon room: the party leaves the room by the side it exited (RoomExitX or
 * RoomExitY: 0xff/-1 or 11), moving one square in the dungeon; from an altar room
 * that leads into another dungeon (AltarRoomExits).
 * Victory: maybe Valor for evil foes, and the spoils (always: SavedMode is never
 * MOD_COM_CAMP; probably meant CurMode, so the camp/inn ambushes also leave
 * spoils, at the stale CombatFoeX/Y for the inn).
 */
int16 /*C_837A*/CombatEnd()
{
	register int16 chara;

	for(chara = 7; ; chara --)
		if(chara < 0 || Fighters._chtile[chara] != 0)
			break;

	if(chara == -1 || CurMode == MOD_COM_ROOM) {
		if(CurMode != MOD_COM_ROOM) {
			if(IsNonEvil(CombatFoeTile)) {
				karma_inc(&(Party._compa), 2);
				karma_inc(&(Party._justi), 2);
			} else {
/*C_83CE:*/
				u4_puts(/*D_25E3*/"Battle is lost!\n");
				karma_dec(&(Party._valor), 2);
			}
			CombatCleanup();
			return;
		}
/*C_83E6:*/
		u4_puts(/*D_25F4*/"Leave Room!\n");
		if(RoomExitX == 0) {
			if(RoomExitY < 0)
				Party._dir = DIR_N;
			else
				Party._dir = DIR_S;
		} else {
			if(RoomExitX < 0)
				Party._dir = DIR_W;
			else
				Party._dir = DIR_E;
		}
		if(AltarRoom != -1) {
			Party._loc = AltarRoomExits[AltarRoom][(Party._dir - 1) & 3];
			Party.out_x = LocationX[Party._loc - 1];
			Party.out_y = LocationY[Party._loc - 1];
			u4_puts(/*D_2601*/"into Dungeon\n");
			u4_puts(Names[NAME_LOCATIONS + Party._loc - 1]);
			dclose(File_DNG);
			File_DNG = 0;
			LoadDungeon();
		}
		Gra_CR();
		u_kbflush();
		Party._x = DNG_X_p(Party._x, Party._dir);
		Party._y = DNG_Y_p(Party._y, Party._dir);
		return;
	}
/*C_849E:*/
	u4_puts(/*D_260F*/"\nVictory!\n");
	if(!IsNonEvil(CombatFoeTile))
		karma_inc(&(Party._valor), U4_RND1(1));
	if(SavedMode != MOD_COM_CAMP)
		CombatSpoils();
}
