/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>
#include <stdlib.h>

/*C_C445*/Unused1() {}
/*C_C44A*/Unused2() {}
/*C_C44F*/Unused3() {}

static int /*D_9138*/TwoDrives;

/*confirmFileInDrive?*/
/*C_C454*/AskForDisk(bp06, bp04)
char *bp06;
char *bp04;
{
	int bp_02;

	if(!u_fexist(bp04)) {
		CurMode = MOD_VISION;
		txt_Y = 10;
		txt_X = (unsigned int)(40 - strlen(bp06)) >> 1;
		u4_puts(bp06);
		txt_Y ++;
		txt_X = 18;
		u4_puts(/*D_3024*/"and");
		txt_Y ++;
		txt_X = 11;
		u4_puts(/*D_3028*/"press drive letter");
		while(!u_kbhit());
		do {
			bp_02 = (unsigned char)u_kbread();
			u4_toupper(bp_02);
			if(bp_02 != 'B' || TwoDrives != 0) {
				if(bp_02 >= 'A' && bp_02 <= 'P') {
					bp_02 &= 0xf;
					u_setdrive(bp_02);
				}
			}
			if(u_getdrive() == bp_02 && u_fexist(bp04))
				return;
			sound(1);
			while(!u_kbhit());
		} while(1);
	}
}

#define NPARAM (far_psp->_80[0])
#define PARAM0 (far_psp->_80[2])
#define PARAM1 (far_psp->_80[4])

/*C_C51C*/GameInit()
{
	int bp_02, bp_04;

	u4_toupper(PARAM1);
	u4_toupper(PARAM0);
	/*set drive number*/
	if(NPARAM < 4) {
		TwoDrives = ((equip_flags & 0xc0) >> 6)?1:0;
		PARAM1 = TwoDrives + '0';
	} else if (PARAM1 == '0') {
		TwoDrives = 0;
	} else if (PARAM1 == '1') {
		TwoDrives = 1;
	} else {
		TwoDrives = ((equip_flags & 0xc0) >> 6)?1:0;
	}
	/*set graphic type*/
	if(NPARAM < 2) {
		GraphicsType = low_gra();
	} else if(PARAM0 == 'C') {
		GraphicsType = 1;
	} else if(PARAM0 == 'E') {
		GraphicsType = 2;
	} else if(PARAM0 == 'T') {
		GraphicsType = 3;
	} else {
		GraphicsType = low_gra();
	}
	/* */
	pShapes = dalloc((GraphicsType == 1)?0x4000:0x8000);
	pCharset = dalloc((GraphicsType == 1)?0x0800:0x1000);
	if(pShapes == 0 || pCharset == 0)
		exit(3);
	if(Load((GraphicsType == 1)?/*D_303B*/"CHARSET.CGA":/*D_3047*/"CHARSET.EGA", (GraphicsType == 1)?0x0800:0x1000, pCharset) == -1)
		exit(3);
	if(GraphicsType == 1) {
		if(Load(/*D_3053*/"SHAPES.CGA", 0x4000, pShapes) == -1)
			exit(3);
	}
	bp_02 = u_getdrive();
	switch(GraphicsType) {
		case 1:
			dfree(patch_tandy);
			patch_tandy = 0;
			LoadGraphicsDriver(/*D_305E*/"CGA.DRV");
		break;
		case 2:
			dfree(patch_tandy);
			patch_tandy = 0;
			LoadGraphicsDriver(/*D_3066*/"EGA.DRV");
		break;
		case 3:
			LoadGraphicsDriver(/*D_306E*/"TANDY.DRV");
		break;
		case -1:
			Console(/*D_3078*/"I can't find a color graphics card.\r\n");
			exit(2);
		break;
	}
	bp_04 = u_diskcheck();/*piracy check function ?*/
	Gra_Init(&pShapes, &pCharset, exit);
	if(GraphicsType == 2)
		InterleaveCharset();
	u_setint24();
	if(GraphicsType == 1)
		speed_info ++;
	Gra_ClearScreen();
	if(PARAM1 >= (char)'A' && PARAM1 <= (char)'P') {
		if(u_getdrive() == (PARAM1 & 0x0f)) {
			AskForDisk(/*D_30A8*/"Re-Insert PLAYER Disk", /*D_309E*/"WORLD.MAP");
		} else {
			u_setdrive((unsigned char)PARAM1 & 0x0f);
		}
	}
	AskForDisk(/*D_30C8*/"Insert PLAYER disk", /*D_30BE*/"WORLD.MAP");
	if(GraphicsType != 1) {
		if(Load(/*D_30DB*/"SHAPES.EGA", 0x8000, pShapes) == -1)
			exit(3);
	}
	if(GraphicsType == 2)
		InterleaveShapes();
	if(Load(/*D_30E6*/"PARTY.SAV", sizeof(struct tParty), &Party) == -1)
		exit(3);
#if 0/*def WIN32*/
	/*for debug*/
	Party._x = 0x5b; Party._y = 0x44;
#endif
	if(Party._members == 0) {
		Gra_ClearScreen();
		txt_Y = 8;
		txt_X = 12;
		u4_puts(/*D_30F0*/"No party formed!");
		sound(8);
		u_delay(3, 0);
		u_kbflush();
		if(bp_02 == u_getdrive()) {
			AskForDisk(/*D_310B*/"Re-Insert PROGRAM Disk", /*D_3010*/"TITLE.EXE");
		} else {
			u_setdrive(bp_02);
			AskForDisk(/*D_312C*/"Insert PROGRAM disk", /*D_3122*/"TITLE.EXE");
		}
		low_clean();
		exit(4);
	}
	if(Load(/*D_3140*/"MONSTERS.SAV", sizeof(struct tNPC), &(Area._npc)) == -1)
		exit(3);
	if((File_MAP = dopen(/*D_314D*/"WORLD.MAP", 0)) == -1) {
		Console(/*D_3157*/"Can't open map file\r\n");
		exit(3);
	}
	File_TLK = 0;
	OpenGateX = MoongateX[Party._trammel];
	OpenGateY = MoongateY[Party._felucca];
	DrawGameFrame();/*draw game screen frame*/
	Gra_CR();
	spell_effect = 7;
	spell_turns = 0;
	LineOfSight = (Party._fly_light == 0);/*flying status*/
	Gallop = 0;
	CurMode = MOD_OUTDOORS;
	WindDir = DIR_N;
	SoundFlag = 1;
	dspl_Stats();
	LoadWorldWindow();
}
