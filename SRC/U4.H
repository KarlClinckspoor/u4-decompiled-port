/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#ifndef __U4_H__
#define __U4_H__

#include "portable.h"

#ifdef WIN32
	#define far
	#pragma pack(1)
	typedef uint16 U16;
	typedef int16 S16;
#else
	typedef uint16 U16;
	typedef int16 S16;
#endif

#include <setjmp.h>

#include "u4_scan.h"
#include "u4_shape.h"
#include "u4_loc.h"

/*sound effects: sound(SND_xxx) (U4_SOUND.ASM)*/
#define SND_STEP        0 /*a step*/
#define SND_BEEP        1 /*blocked, wrong key*/
#define SND_BAD_COMMAND 2
#define SND_SHOOT       3 /*a missile or a cannon ball leaves*/
#define SND_ATTACK      4 /*the active character attacks*/
#define SND_FOE_ATTACK  5 /*a foe attacks in contact*/
#define SND_HIT         6 /*something is hit, damage, explosion*/
#define SND_HURT        7 /*a character is hurt*/
#define SND_FAIL        8 /*failure, evasion, flight, theft*/
#define SND_MAGIC       9 /*sound(SND_MAGIC, pitch): magic zap*/
#define SND_CAST       10 /*sound(SND_CAST, length): spell casting*/
#define SND_WHIRLPOOL  11
#define SND_TWISTER    12

/*first index of each group in Names[] (U4_CSTES.C)*/
#define NAME_CREATURES     1 /*+ (tile - TIL_PIRATE_W) / 2 for 0x80-0x8F*/
#define NAME_MONSTERS      9 /*+ (tile - TIL_RAT) / 4 for 0x90-0xFF*/
#define NAME_PHANTOM      20
#define NAME_WEAPONS      37
#define NAME_ARMOURS      53
#define NAME_WEAPON_CODES 61
#define NAME_CLASSES      77 /*then the townsfolk: guard, merchant, ...*/
#define NAME_REAGENTS     93
#define NAME_SPELLS      101
#define NAME_LOCATIONS   127 /*+ Party._loc - 1*/
#define NAME_VIRTUES     151
#define NAME_VILLAGER    159

/*character classes (tChara._class)*/
#define CLASS_MAGE     0
#define CLASS_BARD     1
#define CLASS_FIGHTER  2
#define CLASS_DRUID    3
#define CLASS_TINKER   4
#define CLASS_PALADIN  5
#define CLASS_RANGER   6
#define CLASS_SHEPHERD 7

/*weapons (tChara._weapon, tParty._weapons[])*/
#define WPN_HANDS        0
#define WPN_STAFF        1
#define WPN_DAGGER       2
#define WPN_SLING        3
#define WPN_MACE         4
#define WPN_AXE          5
#define WPN_SWORD        6
#define WPN_BOW          7
#define WPN_CROSSBOW     8
#define WPN_OIL          9
#define WPN_HALBERD     10
#define WPN_MAGIC_AXE   11
#define WPN_MAGIC_SWORD 12
#define WPN_MAGIC_BOW   13
#define WPN_WAND        14
#define WPN_MYSTIC      15

/*armours (tChara._armor, tParty._armors[])*/
#define ARM_SKIN        0
#define ARM_CLOTH       1
#define ARM_LEATHER     2
#define ARM_CHAIN       3
#define ARM_PLATE       4
#define ARM_MAGIC_CHAIN 5
#define ARM_MAGIC_PLATE 6
#define ARM_MYSTIC      7

#define DIR_W 0
#define DIR_N 1
#define DIR_E 2
#define DIR_S 3

/*constants for "struct tParty.mItems" masking*/
#define ST_USE_BELL   12
#define ST_USE_BOOK   11
#define ST_USE_CANDLE 10
#define ST_WHEEL       9
#define ST_HORN        8
#define ST_KEY_T       7
#define ST_KEY_L       6
#define ST_KEY_C       5
#define ST_BELL        4
#define ST_BOOK        3
#define ST_CANDLE      2
#define ST_CAST_SKULL  1
#define ST_SKULL       0

#define DNG_X_p(x, dir) (((x) + DirDX[(dir)]) & 7)
#define DNG_X_m(x, dir) (((x) - DirDX[(dir)]) & 7)
#define DNG_Y_p(y, dir) (((y) + DirDY[(dir)]) & 7)
#define DNG_Y_m(y, dir) (((y) - DirDY[(dir)]) & 7)

#define u4_lower(c) ((c) + 'a' - 'A')
#define u4_isupper(c) ((c) >= 'A' && (c) <= 'Z')
#define u4_islower(c) ((c) >= 'a' && (c) <= 'z')
#define u4_toupper(c) if(u4_islower(c)) (c) -= 'a' - 'A'
#define u4_toupper2(c) if(u4_islower(c)) (c) += 'A' - 'a'
#define u4_tolower(c) if(u4_isupper(c)) (c) += 'a' - 'A'

#define u4_max(a,b) ((a) >= (b)?(a):(b))
#define u4_min(a,b) ((a) >  (b)?(b):(a))

typedef unsigned char tMap32x32[32][32];
typedef unsigned char tMap8x8x8[8][8][8];

struct tNPC /*size:0x100*/ {
	/*_00*/unsigned char _gtile[32];
	/*_20*/unsigned char _x[32],_y[32];
	/*_60*/unsigned char _tile[32];
	/*_80*/unsigned char _old_x[32], _old_y[32];
	/*_c0*/unsigned char _var[32];/*_agressivity (or _z in dungeon)*/
	/*_e0*/unsigned char _tlkidx[32];
};

/* *.ULT/*.DNG */
struct tArea {
	/*DNGMAP.SAV*/
	union {
		tMap8x8x8 x8x8x8;
		tMap32x32 x32x32;
	} /*__8742.__000*/_map;
	/*MONSTERS.SAV/OUTMONST.SAV/*.ULT(last 0x100 bytes)*/
	struct tNPC /*__8B42/__8742.__400*/_npc;
};

struct tChara /*size:0x27*/ {
	/*+00*/U16 _HP[2];
	/*+04*/U16 _XP;
	/*+06*/U16 _str;
	/*+08*/U16 _dex;
	/*+0a*/U16 _int;
	/*+0c*/U16 _MP;
	/*+0e*/char _unused0e[2];
	/*+10*/U16 _weapon;
	/*+12*/U16 _armor;
	/*+14*/char _name[16];
	/*+24*/char _sex;/*sex char*/
	/*+25*/char _class;
	/*+26*/char _stat;
};

struct tParty /*size:0x1f6*/ {
	/*+000*/uint32 _tick;/*a counter*/
	/* */
	/*+004*/uint32 _moves;
	/*+008*/struct tChara chara[8];
	/*+140*/int32 _food;
	/*+144*/U16 _gold;
	/*karmas*/
	/*+146*/U16 _hones;
	/*+148*/U16 _compa;
	/*+14a*/U16 _valor;
	/*+14c*/U16 _justi;
	/*+14e*/U16 _sacri;
	/*+150*/U16 _honor;
	/*+152*/U16 _spiri;
	/*+154*/U16 _humil;
	/* */
	/*+156*/U16 _torches;
	/*+158*/U16 _gems;
	/*+15a*/U16 _keys;
	/*+15c*/U16 _sextants;
	/*+15e*/U16 _armors[8];
	/*+16e*/U16 _weapons[16];
	/*+18e*/U16 _reagents[8];
	/*+19e*/U16 _mixtures[26];
	/*+1d2*/U16 mItems;
	/*+1d4,+1d5*/unsigned char _x, _y;
	/*+1d6*/unsigned char mStones;
	/*+1d7*/unsigned char mRunes;
	/*+1d8*/U16 _members;/*characters #*/
	/*+1da*/U16 _tile;
	/*+1dc*/U16 _fly_light;/*isFlying or light[dungeon]*/
	/*+1de/+1e0*/U16 _trammel,_felucca;/*moons phase*/
	/*+1e2*/U16 _ship;/*hull integrity*/
	/*+1e4*/U16 _metLB;/*did met with LB*/
	/*+1e6*/U16 _lastCamp;/*last hole up&camp*/
	/*+1e8*/U16 _lastFound;/*last found*/
	/*+1ea*/U16 _lastMeditate;/*last meditation/Hawkwind*/
	/*+1ec*/U16 _lastTalk;/*last karma-conversation*/
	/*+1ee,+1ef*/unsigned char out_x, out_y;
	/*+1f0*/U16 _dir;/*[dungeon]*/
	/*+1f2*/S16 _z;/*[dungeon]*/
	/*+1f4*/U16 _loc;
};

#define TST_MSK(mask, bit) ((mask) & (1 << (bit)))
#define SET_MSK(mask, bit) (mask) |= (1 << (bit))
#define RST_MSK(mask, bit) (mask) &= ~(1 << (bit))

/*CON file structure*/
struct tCombat/*size:0xc0*/ {
	char _npcX[16],_npcY[16];/*_000/_010 D_9470/D_9480*/
	unsigned char _charaX[8],_charaY[8];/*_20/_28 D_9490/D_9498*/
	char _030[16];/*padding?*/
	unsigned char _map[11*11];/*_040 D_94B0*/
	char _0b9[7];/*padding?*/
};

#define Combat_MAP(a,b) (Combat._map[(a)*11+(b)])

struct tFighters {
	/*000*/char _x[16],_y[16];
	/*020*/char _HP[16];
	/*030*/char _tile[16],_gtile[16];
	/*050*/char _sleeping[16];
	/*060*/char _chtile[8];
};

/***************************************************************/
/* note: pascal functions restore the stack by themsevles      */
/* for C functions(cdecl), it is the caller's responsability   */
/***************************************************************/

/*u4_low*/
extern int16 /*C_0012*/low_init();
extern int16 /*C_0115*/low_clean();
extern int16 /*C_0147*/low_gra();
/*u4_use*/
extern int16 /*C_07AE*/CMD_Use();

#include "u4_file.h"

/*u4_util*/
extern int16 /*C_095E*/ShakeScreen();
extern int16 /*C_097D*/XP_inc(int16, int16);
extern int16 /*C_09B1*/HP_inc(int16, int16);
extern int16 /*C_09D9*/FlashChara(int16);
extern int16 /*C_09A3*/w_Pass();
extern int16 /*C_09F8*/karma_inc(U16 *, uint16);
extern int16 /*C_0A17*/karma_dec(U16 *, uint16);
extern int16 /*C_0A58*/FindNpcAt(int16, unsigned char, unsigned char);
extern int16 /*C_0A8B*/FindLocalNpcAt(unsigned char, unsigned char);
extern int16 /*C_0AB1*/SafeModulo(int16, int16);
extern int16 /*C_0ACF*/CharaTile(int16);
extern int16 /*C_0AFE*/IsNonEvil(unsigned char);
extern int16 /*C_0B38*/u4_puts(char *);
extern int16 /*C_0C03*/u4_putl(int32, int16, char);
extern int16 /*C_0C9F*/u4_putc(int16);
extern int16 /*C_0CF7*/dspl_Stats();
extern int16 /*C_0E0E*/dspl_Gold();
extern int16 /*C_0E4E*/isCharaAlive(uint16);
extern int16 /*C_0E82*/isCharaConscious(int16);
extern int16 /*C_10FD*/CheckPartyAlive();
extern int16 /*C_1135*/hitChara(uint16, int16);
extern int16 /*C_1188*/w_What();
extern int16 /*C_1199*/w_Cant_t();
extern int16 /*C_11AA*/w_NotHere();
extern int16 /*C_11BB*/w_Disabled();
extern int16 /*C_11CC*/w_OnlyOnFoot();
extern int16 /*C_11DD*/w_SlowProgress();
extern int16 /*C_11EB*/w_NoneLeft();
extern int16 /*C_11F9*/AskLetter(char *, char, char);
extern int16 /*C_1287*/AskChara(char *);
extern int16 /*C_12D6*/AskDir(char *, int16 *, int16 *);
extern int16 /*C_138B*/food_dec(int16);
extern int16 /*C_13B6*/MP_recover();
extern int16 /*C_1445*/u4_gets(char *, int16);
extern char * /*C_1513*/CreatureName(int16);
extern int16 /*C_1584*/DamageAllParty();
extern int16 /*C_162F*/AskY_N();
extern int32 /*C_169C*/AskInt(int16);
/*u4_util2*/
extern void cdecl /*C_16CD*/u_delay(int16, int16);

extern unsigned char u_rand_a();
#ifdef M_I86
#define U4_RND1(a) (u_rand_a()&(a))
#else
/*the port: the operand first, as MSC computes U4_RND1(U4_RND2(1)) (port/sys/util2.c)*/
extern int16 u4_rnd1(int16);
#define U4_RND1(a) u4_rnd1(a)
#endif
#define U4_RND3(a) (u_rand_a()%(a))
#define U4_RND4(a) (SafeModulo(u_rand_a(), (a)))

/*used by U4_ANIM.C only?*/
extern unsigned char u_rand_b();
#define U4_RND2(a) (u_rand_b()&(a))

extern int16 cdecl /*C_17C5*/u_kbhit();
extern int16 cdecl /*C_17F4*/u_kbread();
extern void cdecl /*C_1804*/u_kbflush();

extern unsigned char /*C_1814*/u_getdrive();
extern int16 /*C_181D*/u_setdrive(uint16);
extern int16 /*C_182F*/u_fexist(char *);
extern int16 /*C_184F*/u_diskcheck();
extern int16 /*C_18A2*/u_setint24();

extern int16 cdecl sound();

#include "u4_dos.h"

#include "u4_graph.h"

/*u4_map*/
extern int16 /*C_26B6*/LoadWorldWindow();
extern int16 /*C_2747*/LeaveTown();
extern int16 /*C_2839*/ScrollNorth();
extern int16 /*C_2891*/ScrollSouth();
extern int16 /*C_28E9*/ScrollWest();
extern int16 /*C_2941*/ScrollEast();
extern int16 /*C_29C3*/w_Blocked();
extern int16 /*C_29DE*/w_DriftOnly();
extern int16 /*C_29EF*/IsSlowTerrain(unsigned char);
extern int16 /*C_2999*/IsWalkable(unsigned char);
extern int16 /*C_2A5A*/WindAllows(uint16);
extern int16 /*C_2B8C*/CMDDIR_Up();
extern int16 /*C_2C8A*/CMDDIR_Down();
extern int16 /*C_2DA9*/CMDDIR_Left();
extern int16 /*C_2EC5*/CMDDIR_Right();
/*u4_end*/
extern int16 /*C_31F4*/EndGame();
/**/
extern int16 /*C_36C7*/DrawView();
extern int16 /*C_3A80*/UpdateMoons();
extern int16 /*C_3C54*/DrawCombat();
extern int16 /*C_3DC8*/RefreshView();
extern int16 /*C_3E94*/LoadDungeon();
extern int16 /*C_4018*/CMD_Enter();
extern int16 /*C_41C0*/CMD_X_it();
extern int16 /*C_4253*/CMD_Board();
extern int16 /*C_42E8*/CMD_Yell();
extern int16 /*C_431D*/AutoCloseDoor();
extern int16 /*C_4353*/CMD_Open();
extern int16 /*C_43F8*/CMD_Jimmy();
extern int16 /*C_4477*/CMD_Klimb();
extern int16 /*C_44EE*/CMD_Descend();
extern int16 /*C_4649*/ZtatsClearTitle();
extern int16 /*C_4832*/ZtatsWeapons();
extern int16 /*C_48F8*/ZtatsArmour();
extern int16 /*C_4987*/ZtatsEquipment();
extern int16 /*C_4BC7*/ZtatsReagents();
extern int16 /*C_4C42*/ZtatsMixtures();
extern int16 /*C_4E45*/CMD_Ztats();
extern int16 /*C_4FBE*/u4_abs(int16);
extern int16 /*C_4FD7*/u4_sign(int16);
extern int16 /*C_5834*/MoveNpcs();
extern int16 /*C_5851*/SpawnMonsters();
extern int16 /*C_59D5*/COM_GetFighterId(unsigned char, unsigned char);
extern int16 /*C_5A6B*/COM_main();
extern int16 /*C_5DAB*/COM_DoDamage(int16, int16, unsigned char);
extern int16 /*C_5EA2*/COM_MissileStep(unsigned char, unsigned char);
extern int16 /*C_628F*/CMD_Attack();
extern int16 /*C_636D*/IsUndead(int16);
extern int16 /*C_6E4A*/CMD_Cast();
extern int16 /*C_6F29*/CMD_Quit();
extern int16 /*C_6FF9*/SwapCharas(struct tChara *, struct tChara *);
extern int16 /*C_722F*/OpenChest(int16);
extern int16 /*C_7337*/COM_OpenChest();
extern int16 /*C_7631*/CMD_Ready();
extern int16 /*C_7525*/CMD_Ignite();
extern int16 /*C_7034*/CMD_NewOrder();
extern int16 /*C_70AD*/CMD_Volume();
extern int16 /*C_72EC*/CMD_Get();
extern int16 /*C_73B8*/COM_GetChest();
extern int16 /*C_73C9*/CMD_Fire();
extern int16 /*C_755E*/CMD_Locate();
extern int16 /*C_7732**/CMD_Wear();
extern int16 /*C_7918*/VortexContacts();
extern int16 /*C_7B89*/COM_North();
extern int16 /*C_7BAF*/COM_South();
extern int16 /*C_7BD5*/COM_East();
extern int16 /*C_7BFD*/COM_West();
extern int16 /*C_7C25*/MonsterIndex(unsigned char);
extern int16 /*C_7D50*/DungeonCombat();
extern int16 /*C_7D92*/InnCombat();
extern int16 /*C_7DBC*/CombatBegin();
extern int16 /*C_7DFE*/StartCombat(uint16);
extern int16 /*C_7FD7*/RunCombat();
extern int16 /*C_7FFD*/EnterDungeonRoom();
extern int16 /*C_837A*/CombatEnd();
extern int16 /*C_84D2*/DNG_main();
extern int16 /*C_8AB0*/CMD_HoleUp();
extern int16 /*C_8C08*/CMD_Mix();
extern int16 /*C_913A*/CMD_Search();
extern int16 /*C_9209*/TileEffects();
extern int16 /*C_9414*/DNG_MoveMonsters();
extern int16 /*C_95AA*/DNG_SpawnMonsters();
extern int16 /*C_9F7B*/COM_FoesTurn();
extern int16 /*C_A6F3*/CMD_Talk();
extern int16 /*C_ADEF*/RenderSprites3D();
extern int16 /*C_AE41*/DNG_LookAround();
extern int16 /*C_B677*/DNG_Render();
extern int16 /*C_B9B2*/DNG_Search();
extern int16 /*C_B9EF*/ShowAreaMap(uint16);
extern int16 /*C_C403*/ShowMap();
extern int16 /*C_C41D*/CMD_Peer();
/*u4_init*/
extern int16 /*C_C51C*/GameInit();
/* */
extern int16 /*C_E59B*/TalkToLB();
extern int16 /*C_E72C*/EnterShrine();

/*====---- initialized ----====*/

struct tPSP {
	unsigned char _00[0x80];
	unsigned char _80[0x80];
};
extern struct tPSP far *far_psp;/*D_0078*/

extern uint16 /*D_0672*/cursor_delay;
extern char cdecl /*D_06A6*/SoundFlag;
extern uint16 /*D_07F8*/TurnDone;
extern U16 /*D_07FA*/*pKarmas[];
extern int16 /*D_080A*/activeCharaX;
extern char /*D_080C*/DirDX[], /*D_0810*/DirDY[];
extern char /*D_0814*/MoongateX[], /*D_081C*/MoongateY[];/*moongates positions*/
extern char * /*D_0824*/TownFiles[];
extern char /*D_0844*/LocationX[];
extern char /*D_0864*/LocationY[];
extern char * /*D_0884*/StoneColors[];
extern char * /*D_0894*/DungeonFiles[];
/*strings*/
extern char * /*D_1E98*/Names[];

extern unsigned char /*D_23D2*/MonsterMaxHP[];
extern unsigned char /*D_2450*/WeaponDamage[];
extern unsigned char /*D_2460*/ArmorDefense[];
extern unsigned char /*D_2468*/WeaponIsRanged[];

/*====---- uninitialized ----====*/
extern uint16 /*D_8728*/speed_info;
extern uint16 /*D_872B*/equip_flags;
extern void far *patch_tandy;/*D_872D*/
extern struct tArea /*D_8742*/Area;

/*====---- c_common ----====*/
extern unsigned char /*D_9140*/CombatFoeY;
extern unsigned char /*D_9141*/MoongateTile;
extern int16 /*D_9142*/ShopId;
extern struct tParty /*D_9144*/Party;
extern tMap32x32 far * /*D_933A*/MapChunks[][8];
extern int16 /*D_943A*/GraphicsType;/* graphic adaptater type */
extern unsigned char /*D_943C*/tile_forward;
extern int16 /*D_943E*/AltarRoom;
extern uint16 /*D_9440*/LineOfSight;
extern unsigned char /*D_9442,D_9443*/ship_x, ship_y;
extern unsigned char /*D_9444*/tile_cur;
extern char /*D_9445*/OpenGateX;
extern uint16 /*D_9446*/Redraw3D;
extern char /*D_9448*/OpenGateY;
extern char /*D_944A*/CharaAnimTile[];
extern unsigned char /*D_9452*/CombatFoe;
extern void far /*D_9454*/*pShapes;
extern jmp_buf /*D_9458*/GameLoopJmp;

#define MOD_VISION   0
#define MOD_OUTDOORS 1
#define MOD_BUILDING 2
#define MOD_DUNGEON  3
#define MOD_COMBAT   4
#define MOD_COM_CAMP 5
#define MOD_COM_ROOM 6
#define MOD_SHRINE   7
extern uint16 /*D_946A*/CurMode;

struct tPos {
	unsigned char x, y;
};

extern unsigned char /*D_946C*/CombatGround;
extern uint16 /*D_946E*/spell_turns;
extern struct tCombat /*D_9470*/Combat;
extern struct tFighters /*D_9530*/Fighters;
extern int16 /*D_9598*/File_DNG;
extern int16 /*D_959A*/activeChara;
extern struct tPos /*D_959C*/WinPos;
extern unsigned char /*D_959E*/tile_east;
extern void far /*D_95A0*/*pCharset;
extern char /*D_95A4*/spell_effect;
extern struct tPos /*D_95A5*/WinBlock;
extern int16 /*D_95A8*/txt_X;
extern char /*D_95AA*/tile_north;
extern uint16 /*D_95AC*/activeCharaY;
extern uint16 /*D_95AE*/hit_tile;
extern unsigned char /*D_95B0,D_95B1*/hit_x, hit_y;
extern unsigned char /*D_95B2*/RoomTriggerData[];
extern unsigned char /*D_95C2*/tile_west;
extern unsigned char /*D_95C3*/tile_south;
extern int16 /*D_95C4*/txt_Y;
extern uint16 /*D_95C6*/Gallop;
extern int16 /*D_95C8*/CursorBlink;
extern unsigned char /*D_95CA*/tile_back;
extern int16 /*D_95CC*/SavedMode;
extern unsigned char /*D_95CE*/TalkData[];/*current .tlk data*/
extern char /*D_96EE*/RoomExitX;/*room exit x[dungeon]*/
extern int16 /*D_96F0*/File_MAP;
extern uint16 /*D_96F2*/WindDir;
extern char /*D_96F4*/RoomExitY;/*room exit y[dungeon]*/
extern int16 /*D_96F6*/File_TLK;
extern unsigned char /*D_96F8*/CombatFoeTile;
extern unsigned char /*D_96F9*/ViewTiles[11*11];
extern unsigned char /*D_9772*/CombatFoeX;

#ifdef WIN32
	#pragma pack()
#endif

#endif
