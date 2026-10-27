/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>
#include <stdlib.h>

/*empty functions, never called*/
int16 /*C_C445*/Unused1() {}
int16 /*C_C44A*/Unused2() {}
int16 /*C_C44F*/Unused3() {}

/*nonzero if the machine has (or the command line says) two floppy drives*/
static int16 /*D_9138*/TwoDrives;

/*
 * Make sure file fname can be found in the current drive; if not, show message
 * ("Insert ... disk") and wait until the player presses the letter of a drive that
 * has it (B only when there are two drives; other keys beep).
 */
int16 /*C_C454*/AskForDisk(message, fname)
char *message;
char *fname;
{
	int16 key;

	if(!u_fexist(fname)) {
		CurMode = MOD_VISION;
		txt_Y = 10;
		txt_X = (uint16)(40 - strlen(message)) >> 1;
		u4_puts(message);
		txt_Y ++;
		txt_X = 18;
		u4_puts(/*D_3024*/"and");
		txt_Y ++;
		txt_X = 11;
		u4_puts(/*D_3028*/"press drive letter");
		while(!u_kbhit());
		do {
			key = (unsigned char)u_kbread();
			u4_toupper(key);
			if(key != 'B' || TwoDrives != 0) {
				if(key >= 'A' && key <= 'P') {
					key &= 0xf;/*drive number: A = 1 ... O = 15, P = 0*/
					u_setdrive(key);
				}
			}
			if(u_getdrive() == (uint16)(key) && u_fexist(fname))
				return;
			sound(SND_BEEP);
			while(!u_kbhit());
		} while(1);
	}
}

/*the command line tail in the PSP: its length, then " C 1"-like arguments*/
#define NPARAM (far_psp->_80[0])
#define PARAM0 (far_psp->_80[2])
#define PARAM1 (far_psp->_80[4])

/*
 * Game start-up. The command line (written by TITLE.EXE) gives the graphics type
 * (C=CGA, E=EGA, T=Tandy, else autodetect) and the drives (0 = one drive, 1 = two,
 * A-P = the drive holding the player disk). Allocates and loads the charset and
 * shapes, loads the graphics driver, asks for the player disk, loads PARTY.SAV
 * (back to the title program if no party was formed), MONSTERS.SAV and opens
 * WORLD.MAP, then initializes the game state and draws the screen.
 * GraphicsType: 1 CGA, 2 EGA, 3 Tandy, -1 none found.
 */
int16 /*C_C51C*/GameInit()
{
	int16 programDrive, copyCheck;

	u4_toupper(PARAM1);
	u4_toupper(PARAM0);
	/*number of floppy drives: BIOS equipment word bits 6-7 = drives - 1*/
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
	/*graphics type*/
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
	programDrive = u_getdrive();
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
	copyCheck = u_diskcheck();/*copy protection check? result unused*/
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
		sound(SND_FAIL);
		u_delay(3, 0);
		u_kbflush();
		if((uint16)(programDrive) == u_getdrive()) {
			AskForDisk(/*D_310B*/"Re-Insert PROGRAM Disk", /*D_3010*/"TITLE.EXE");
		} else {
			u_setdrive(programDrive);
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
