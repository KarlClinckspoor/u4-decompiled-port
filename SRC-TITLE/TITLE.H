#ifndef __TITLE_H__
#define __TITLE_H__

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

#include "u4_scan.h"

#define u4_lower(c) ((c) + 'a' - 'A')
#define u4_isupper(c) ((c) >= 'A' && (c) <= 'Z')
#define u4_islower(c) ((c) >= 'a' && (c) <= 'z')
#define u4_toupper(c) if(u4_islower(c)) (c) -= 'a' - 'A'
#define u4_toupper2(c) if(u4_islower(c)) (c) += 'A' - 'a'
#define u4_tolower(c) if(u4_isupper(c)) (c) += 'a' - 'A'

extern uint16 /*D_0036*/Monster1Pos;
extern uint16 /*D_0038*/Monster2Pos;

struct tPSP {
	unsigned char _00[0x80];
	unsigned char _80[0x80];
};

extern struct tPSP far *far_psp;/*D_041C*/

extern uint16 /*D_31BC*/cursor_delay;
extern uint16 /*D_31C0*/cursor_enabled;

extern unsigned char /*D_3380*/Monster1Seq[];
extern unsigned char /*D_33F8*/Monster2Seq[];
extern unsigned char /*D_3438*/MonsterFrameY[];
extern unsigned char /*D_344A*/Monster1FrameX[];
extern unsigned char /*D_345C*/Monster2FrameX[];
extern unsigned char /*D_346E*/SignatureDots[];
extern unsigned char /*D_3683*/ViewBackground[19*5];
extern unsigned char /*D_36E2*/ViewScript[];

/*====---- ----====*/

extern uint16 /*D_692E*/speed_info;
extern uint16 /*D_6931*/equip_flags;

struct tNPC {
	unsigned char _gtile[32];/*__6D76*/
	unsigned char _x[32];/*__6D96*/
	unsigned char _y[32];/*__6DB6*/
	unsigned char _tile[32];/*__6DD6*/
	unsigned char _old_x[32];/*__6DF6*/
	unsigned char _old_y[32];/*__6E16*/
	unsigned char _var[32];/*__6E36*/
	unsigned char _tlkidx[32];/*__6E56*/
};

typedef unsigned char tMap32x32[32][32];
typedef unsigned char tMap8x8x8[8][8][8];

/* *.ULT/*.DNG */
struct tArea {
	/*DNGMAP.SAV*/
	union {
		tMap8x8x8 x8x8x8;
		tMap32x32 x32x32;
	} /*__6976.__000*/_map;
	/*MONSTERS.SAV/OUTMONST.SAV/*.ULT(last 0x100 bytes)*/
	struct tNPC /*__6D76/__6976.__400*/_npc;
};

extern struct tArea /*D_6976*/Area;

/*====---- c_common ----====*/
extern uint16 /*D_6E80*/MonstersShown;


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
	/*+24*/char _sex;
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
	/*+156*/U16 _torches;/*torches*/
	/*+158*/U16 _gems;/*Gems*/
	/*+15a*/U16 _keys;/*Keys*/
	/*+15c*/U16 _sextants;/*Sextants*/
	/*+15e*/U16 _armors[8];/*Armors*/
	/*+16e*/U16 _weapons[16];/*Weapons*/
	/*+18e*/U16 _reagents[8];/*Reagents*/
	/*+19e*/U16 _mixtures[26];/*Mixtures*/
	/*+1d2*/U16 mItems;/*Items mask*/
	/*+1d4,+1d5*/unsigned char _x, _y;
	/*+1d6*/unsigned char mStones;/*Stones mask*/
	/*+1d7*/unsigned char mRunes;/*Runes mask*/
	/*+1d8*/U16 _members;/*characters #*/
	/*+1da*/U16 _tile;
	/*+1dc*/U16 _fly_light;
	/*+1de/+1e0*/U16 _trammel,_felucca;/*moons phase*/
	/*+1e2*/U16 _ship;/*hull integrity*/
	/*+1e4*/U16 _metLB;/*did met with LB*/
	/*+1e6*/U16 _lastCamp;/*last hole up&camp*/
	/*+1e8*/U16 _lastFound;/*last found*/
	/*+1ea*/U16 _lastMeditate;/*last meditation/Hawkwind*/
	/*+1ec*/U16 _lastTalk;/*last karma-conversation*/
	/*+1ee/+1ef*/unsigned char out_x, out_y;
	/*+1f0*/U16 _dir;/*[dungeon]*/
	/*+1f2*/S16 _z;/*[dungeon]*/
	/*+1f4*/U16 _loc;
};

#ifdef WIN32
	#pragma pack()
#endif

extern struct tParty /*D_6E82*/Party;
extern uint16 /*D_7078*/GraphicsType;
extern void far /*D_707A*/*pShapes;
extern void far /*D_707E*/*pAnim;
extern uint16 /*D_7082*/TwoDrives;
extern void far /*D_7084*/*pCharset;
extern uint16 /*D_7088*/txt_X;
extern uint16 /*D_708A*/txt_Y;
extern void far /*D_708C*/*pTitle;
/*====---- ----====*/

/*====---- ----====*/

extern int16 /*C_0010*/Console(char *);
extern int16 /*C_0051*/Load(char *, uint16, void far *);
extern int16 /*C_0110*/Save(char *, uint16, void /*far */*);

extern int16 /*C_027A*/u4_putc();

extern int16 cdecl Gra_DrawView(
#if 0
#endif
);
extern int16 cdecl Gra_Dot();
extern int16 Gra_ClearMenu();
extern int16 cdecl Gra_Blit(
#if 0
	int16 w/*in 8 pixels blocs*/, int16 h,
	int16 src_x/*in 8 pixels blocs*/, int16 src_y,
	void far *pic_data,
	int16 dst_y,
	int16 rand_pts,/*only works for cga?*/
	int16 dst_x
#endif
);
extern int16 cdecl Gra_LoadPicture(
#if 0
#endif
);
extern int16 Gra_ClearText();
extern int16 Gra_Init();
extern int16 Gra_AnimFlow();
extern int16 Gra_AnimFlags();
extern int16 Gra_ClearScreen();
extern int16 Gra_PutChar();
extern int16 cdecl Gra_BlitReveal(
#if 0
	int16 w/*in 8 pixels blocs*/, int16 h,
	int16 src_x/*in 8 pixels blocs*/, int16 src_y,
	void far *pic_data,
	int16 dst_y,
	int16 rand_pts,
	int16 dst_x
#endif
);
extern int16 cdecl Gra_DrawViewOver(
#if 0
#endif
);
extern int16 InterleaveShapes();
extern int16 /*C_217E*/LoadCompressed(char *, void far *);

/*low.asm*/
extern int16 /*C_244E*/low_init();
extern int16 /*C_2551*/low_clean();
extern int16 /*C_2583*/low_gra();

extern int16 /*C_3030*/NewGame();

extern int16 cdecl /*C_3149*/u_delay(uint16, uint16);
extern int16 /*C_31ED*/u_rand_a();
extern unsigned char /*C_321E*/u_rand_b();
extern int16 /*C_3290*/u_getdrive();
extern int16 /*C_3299*/u_setdrive();
extern int16 /*C_32AB*/u_fexist();
extern int16 /*C_331E*/u_setint24();

extern int16 /*C_3F77*/Unused3F77(uint32, uint32*);

#endif
