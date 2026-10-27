/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "title.h"

#include <malloc.h>
#include <stdlib.h>

int /*D_00C0*/MonsterAnimDelay = 0;
int /*D_00C2*/TileAnimDelay = 1;
int /*D_00C4*/ViewScriptPos = -1;
int /*D_00C6*/ViewFrameWait = 0;

/*base tiles for view*/
unsigned char /*D_00C8*/ViewBaseTiles[] = {0x40,0x80,0x10,0x38,0x38,0xC8,0xC8,0x24,0x20,0x88,0xF0,0xF8,0x4D,0x4F,0x4E};

/*====---- _BSS ----====*/
static unsigned char D_3A24[19 * 5];
static unsigned D_3A84;
/*The statics keep their address-like names: Microsoft C 4.00 lays out a module's
 uninitialised statics in the order of its symbol table, which depends on the names.
 Renaming them would move them; the readable names are macros.*/
#define ViewTiles D_3A24
#define ViewRunning D_3A84

/*animate the 2 "monsters"*/
/*callback for u_delay*/
/*C_019A*/AnimateMonsters()
{
	unsigned bp_02;

	if(ViewRunning)
		MonsterAnimDelay = 0;
	if(MonsterAnimDelay --)
		return;
	MonsterAnimDelay = ((GraphicsType == 1)?120:200) * speed_info;
	if(MonstersShown) {
		cursor_rate = 200;

		bp_02 = Monster1Seq[Monster1Pos++]; Monster1Pos &= 0x7f;
		Gra_Blit(6, 32, Monster1FrameX[bp_02], MonsterFrameY[bp_02], pAnim, 0, -1, 0);
		bp_02 = Monster2Seq[Monster2Pos++]; Monster2Pos &= 0x3f;
		Gra_Blit(6, 32, Monster2FrameX[bp_02], MonsterFrameY[bp_02], pAnim, 0, -1, 34);
	} else {
		cursor_rate = 233;
	}
}

/*C_027A*/u4_putc(bp04)
unsigned char bp04;
{
	if(bp04 == 8) {
		txt_X --;
		Gra_PutChar(' ');
	} else {
		Gra_PutChar(bp04);
		txt_X ++;
	}
}

/*C_02A3*/DelayUnlessKey(bp04)
long bp04;
{
	if(u_kbhit())
		return;
	bp04 *= (long)speed_info;
	while(-- bp04);
}

/*handwriting?*/
/*C_02D1*/DrawSignature(bp04)
unsigned bp04;
{
	register int si;

	for(si = 0; SignatureDots[si]; ) {
		Gra_Dot(191 - SignatureDots[si+1], SignatureDots[si] + 20, 3);
		Gra_Dot(191 - SignatureDots[si+1], SignatureDots[si] + 21, 3);
		si += 2;
		if(bp04)
			DelayUnlessKey((long)440);
	}
	if(bp04)
		u_delay(1, 1);
}

/*tiles animation ?*/
/*C_034D*/AnimateViewTiles()
{
	register int si;

	for(si = 0; si < 32; si ++) {
		register int di;

		di = Area._npc._tile[si];
		if(
			(di >= 0x84 && di <= 0x8e) ||
			(di >= 0x20 && di <= 0x2e) ||
			(di >= 0x50 && di <= 0x5e)
		) {
			if(u_rand_b() < 0xc0)
				Area._npc._gtile[si] = ((Area._npc._gtile[si] & 1) | di) ^ 1;
		} else if(di >= 0x90) {
			if(u_rand_b() < 0xc0)
				Area._npc._gtile[si] = ((Area._npc._gtile[si] + 1) & 3) | di;
		} else if(di != 0x80) {
			Area._npc._gtile[si] = di;
		}
	}
	if(--TileAnimDelay)
		return;
	TileAnimDelay = (speed_info == 1)?1:(speed_info - 1);
	Gra_AnimFlow(0x00);
	Gra_AnimFlow(0x01);
	Gra_AnimFlow(0x02);
	Gra_AnimFlow(0x4c);
	Gra_AnimFlow(0x44);
	Gra_AnimFlow(0x46);
	Gra_AnimFlow(0x47);
	Gra_AnimFlow(0x45);
	Gra_AnimFlags();
}

/*C_041A*/StepViewScript()
{
	int bp_02;

	if(ViewFrameWait) {
		ViewFrameWait --;
		return;
	}
	while(1) {
		switch(ViewScript[++ViewScriptPos] & 0xf0) {
			case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60:
				Area._npc._y[ViewScript[ViewScriptPos] & 0xf] = ViewScript[ViewScriptPos] >> 4;
				Area._npc._x[ViewScript[ViewScriptPos] & 0xf] = ViewScript[ViewScriptPos + 1] & 0x1f;
				Area._npc._gtile[ViewScript[ViewScriptPos] & 0xf] = ViewBaseTiles[ViewScript[ViewScriptPos] & 0xf] + (ViewScript[ViewScriptPos + 1] >> 5);
				Area._npc._tile[ViewScript[ViewScriptPos] & 0xf] = ((ViewScript[ViewScriptPos] & 0xf) == 1)?
					ViewBaseTiles[ViewScript[ViewScriptPos] & 0xf]:
					Area._npc._gtile[ViewScript[ViewScriptPos] & 0xf]
				;
				ViewScriptPos++;
			break;
			case 0x70:/**/
				Area._npc._gtile[ViewScript[ViewScriptPos] & 0xf] =
				Area._npc._tile[ViewScript[ViewScriptPos] & 0xf] = 0;
			break;
			case 0x80:/*end frame*/
				for(bp_02 = 0; bp_02 < 19 * 5; bp_02 ++)
					ViewTiles[bp_02] = ViewBackground[bp_02];
				for(bp_02 = 0; bp_02 < 32; bp_02 ++) {
					if(Area._npc._gtile[bp_02])
						ViewTiles[19 * Area._npc._y[bp_02] + Area._npc._x[bp_02]] = Area._npc._gtile[bp_02];
				}
				ViewFrameWait = ViewScript[ViewScriptPos] & 0xf;
			return;
			case 0x90: case 0xa0: case 0xb0: case 0xc0: case 0xd0: case 0xe0: case 0xf0:
				ViewScriptPos = -1;
			break;
		}
	}
}

/*the "view"*/
/*C_05A4*/ShowView()
{
	register int si;

	ViewRunning = 1;
	cursor_enabled = 0;
	while(!u_kbhit()) {
		AnimateViewTiles();
		StepViewScript();
		MonstersShown = 0;
		Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		for(si = speed_info / 2; si && !u_kbhit(); si --)
			Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		AnimateViewTiles();
		StepViewScript();
		Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		for(si = speed_info / 2; si && !u_kbhit(); si --)
			Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		MonstersShown = 1;
	}
	ViewRunning = 0;
	u_kbread();
}

/*logo + splash + animation*/
/*C_068C*/TitleSequence()
{
	register unsigned loc_A, loc_B;
	int loc_C;

	u_delay(1, 1);
	/*"lord british"*/
	DrawSignature(1);
	/*"and"*/
	Gra_Blit(4, 4, 19, 17, pTitle, 17, -1, 19);
	u_delay(1, 1);
	/*draw line*/
	for(loc_A = 86; loc_A < 238; loc_A++) {
		Gra_Dot(31, loc_A, 2);
		DelayUnlessKey((long)320);
	}
	/*"origin systems inc."*/
	for(loc_A = 1; loc_A < 10; loc_A ++) {
		Gra_Blit(21, loc_A, 9, 21, pTitle, 30 - loc_A, -1, 9);
		DelayUnlessKey((long)5000);
	}
	u_delay(1, 1);
	/* */
	if(!u_kbhit()) {
		/*"present"*/
		for(loc_A = 1; loc_A <= 5; loc_A++) {
			Gra_Blit(15, loc_A, 14, 5 - loc_A, pTitle, 33, -1, 14);
			DelayUnlessKey((long)5000);
		}
		u_delay(1, 1);
		/*"Ultima iv"*/
		for(loc_A = 0; loc_A < 57; loc_A ++) {
			if(u_kbhit())
				loc_A = 56;
			Gra_BlitReveal(30, 45, 5, 34, pTitle, 34, loc_A, 5);
			if(speed_info > 4)
				Gra_BlitReveal(30, 45, 5, 34, pTitle, 34, loc_A, 5);
		}
		u_delay(1, 1);
	} else {
		Gra_Blit(30, 45,  5, 34, pTitle, 34, -1,  5);
		Gra_Blit(15,  5, 14,  0, pTitle, 33, -1, 14);
	}
	/*"quest of the avatar"*/
	for(loc_A = 1; loc_A <= 6; loc_A ++) {
		Gra_Blit(33, loc_A, 3,      81, pTitle, 86 - loc_A, -1, 3);
		Gra_Blit(33, loc_A, 3, 93 - loc_A, pTitle,      86, -1, 3);
		DelayUnlessKey((long)4000);
	}
	/*open view like scroll*/
	for(loc_A = 1; loc_A <= 20; loc_A ++) {
		if(u_kbhit())
			loc_A = 20;
		AnimateViewTiles();
		Gra_DrawViewOver(5, 19, ViewBackground, 104, pTitle, 2);
		if(speed_info > 4)
			Gra_DrawViewOver(5, 19, ViewBackground, 104, pTitle, 2);
		loc_C = 20 - loc_A;
		Gra_Blit(loc_A * 2, 96, loc_C, 96, pTitle, 96, -1, loc_C);
		DelayUnlessKey((long)680);
		AnimateViewTiles();
		Gra_DrawViewOver(5, 19, ViewBackground, 104, pTitle, 2);
	}
	_ffree(pTitle);

	/*the 2 "monsters"*/
/*C_0978:*/
	if((pAnim = (void far *)_fmalloc((GraphicsType == 1)?0x4000:0x8000)) == 0)
		exit(0x37);
	Gra_LoadPicture((GraphicsType == 1)?/*D_00D7*/"ANIMATE.PIC":/*D_00E3*/"ANIMATE.EGA", pAnim);
	for(loc_A = 1; loc_A <= 32; loc_A++) {
/*C_09C4:*/
		if(u_kbhit())
			loc_A = 32;

		/*the "2 monsters"*/
		loc_C = Monster1Seq[Monster1Pos++]; Monster1Pos &= 0x7f;
		Gra_Blit(6, loc_A, Monster1FrameX[loc_C], MonsterFrameY[loc_C] - loc_A + 32, pAnim, 0, -1, 0);
		loc_C = Monster2Seq[Monster2Pos++]; Monster2Pos &= 0x3f;
		Gra_Blit(6, loc_A, Monster2FrameX[loc_C], MonsterFrameY[loc_C] - loc_A + 32, pAnim, 0, -1, 34);
		/*the "view"*/
		AnimateViewTiles();
		StepViewScript();
		Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		for(loc_B = speed_info / 2; loc_B && !u_kbhit(); loc_B --)
			Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		AnimateViewTiles();
		StepViewScript();
		Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		for(loc_B = speed_info / 2; loc_B && !u_kbhit(); loc_B --)
			Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
	}
}

/*C_0B1E*/PutsAt(bp08, bp06, si/*bp04*/)
int bp08;
int bp06;
register char *si;
{
	txt_Y = bp08;
	txt_X = bp06;
	while(*si)
		u4_putc(*si++);
}

/*C_0B45*/ShowMenu()
{
	Gra_ClearMenu();
	PutsAt(14,  2, /*D_00EF*/"In another world, in a time to come.");
	PutsAt(16, 15, /*D_0114*/"Options:");
	PutsAt(17, 11, /*D_011D*/"Return to the view");
	PutsAt(18, 11, /*D_0130*/"Journey Onward");
	PutsAt(19, 11, /*D_013F*/"Initiate New Game");
	PutsAt(21,  3, /*D_0151*/"Conversion by James Van Artsdalen");
	PutsAt(22,  5, /*D_0173*/"\x9 Copyright 1987 Lord British");
	txt_X = 24;
	txt_Y = 16;
	cursor_enabled = 1;
}

/*C_0BCA*/RedrawTitle()
{
	int bp_02;

	Gra_ClearScreen();
	if((pShapes = _fmalloc((GraphicsType == 1)?0x4000:0x8000)) == 0)
		exit(0x3a);
	if(GraphicsType == 1) {
		if(Load(/*D_0191*/"SHAPES.CGA", 0x4000, pShapes) == -1)
			exit(5);
	} else {
		LoadCompressed(/*D_019C*/"SHAPES.EGZ", pShapes);
		InterleaveShapes();
	}

	if((pTitle = _fmalloc((GraphicsType == 1)?0x4000:0x8000)) == 0)
		exit(0x3b);
	Gra_LoadPicture((GraphicsType == 1)?/*D_01A7*/"TITLE.PIC":/*D_01B1*/"TITLE.EGA", pTitle);
	DrawSignature(0);
	Gra_Blit(4, 4, 19, 17, pTitle, 17, -1, 19);
	for(bp_02 = 86; bp_02 < 238; bp_02 ++)
		Gra_Dot(31, bp_02, 2);
	Gra_Blit(21,  9,  9, 21, pTitle, 21, -1, 9);
	Gra_Blit(30, 45,  5, 34, pTitle, 34, -1, 5);
	Gra_Blit(15,  5, 14,  0, pTitle, 33, -1, 14);
	Gra_Blit(33,  6,  3, 81, pTitle, 80, -1, 3);
	Gra_Blit(33,  6,  3, 87, pTitle, 86, -1, 3);
	Gra_Blit(40, 96,  0, 96, pTitle, 96, -1, 0);
	_ffree(pTitle);

	if((pAnim = _fmalloc((GraphicsType == 1)?0x4000:0x8000)) == 0)
		exit(0x3b);
	Gra_LoadPicture((GraphicsType == 1)?/*D_01BB*/"ANIMATE.PIC":/*D_01C7*/"ANIMATE.EGA", pAnim);

	bp_02 = Monster1Seq[Monster1Pos++]; Monster1Pos &= 0x7f;
	Gra_Blit(6, 32, Monster1FrameX[bp_02], MonsterFrameY[bp_02], pAnim, 0, -1, 0);
	bp_02 = Monster2Seq[Monster2Pos++]; Monster2Pos &= 0x3f;
	Gra_Blit(6, 32, Monster2FrameX[bp_02], MonsterFrameY[bp_02], pAnim, 0, -1, 34);

	MonstersShown = 1;
	ShowMenu();
}

cdecl /*C_0EAA*/main()
{
	cursor_enabled = 0;
	low_init();

	u4_toupper(far_psp->_80[4]);
	u4_toupper(far_psp->_80[2]);
	/*set drive number*/
	if(far_psp->_80[0] < 4) {
		TwoDrives = ((equip_flags & 0xc0) >> 6) != 0;
	} else if(far_psp->_80[4] == '0') {
		TwoDrives = 0;
	} else if(far_psp->_80[4] == '1') {
		TwoDrives = 1;
	} else {
		TwoDrives = ((equip_flags & 0xc0) >> 6) != 0;
	}
	/*set graphic type*/
	if(far_psp->_80[0] < 2) {
		GraphicsType = low_gra();
	} else if(far_psp->_80[2] == 'C') {
		GraphicsType = 1;
	} else if(far_psp->_80[2] == 'E') {
		GraphicsType = 2;
	} else if(far_psp->_80[2] == 'T') {
		GraphicsType = 3;
	} else {
		GraphicsType = low_gra();
	}
	if(GraphicsType == -1) {
		Console(/*D_01D3*/"I can't find a color graphics card.\r\n");
		exit(2);
	}
	/* */
	MonstersShown = 0;
	if((pCharset = _fmalloc((GraphicsType == 1)?0x800:0x1000)) == 0)
		exit(0x3c);
	if(Load((GraphicsType == 1)?/*D_01F9*/"CHARSET.CGA":/*D_0205*/"CHARSET.EGA", (GraphicsType == 1)?0x800:0x1000, pCharset) == -1)
		exit(6);
	if((pShapes = _fmalloc((GraphicsType == 1)?0x4000:0x8000)) == 0)
		exit(0x3d);
	if(GraphicsType == 1) {
		if(Load(/*D_0211*/"SHAPES.CGA", 0x4000, pShapes) == -1)
			exit(5);
	} else {
		LoadCompressed(/*D_021C*/"SHAPES.EGZ", pShapes);
	}
	Gra_Init();
	u_setint24();/*set int 24h handler*/

	if((pTitle = _fmalloc((GraphicsType == 1)?0x4000:0x8000)) == 0)
		exit(0x3e);
	Gra_LoadPicture((GraphicsType == 1)?/*D_0227*/"TITLE.PIC":/**D_0231*/"TITLE.EGA", pTitle);
	if(GraphicsType == 1)
		speed_info ++;
	ViewRunning = 0;
	TitleSequence();/*logo + splash + animation*/

	MonstersShown = 1;
/*C_10E2:*/
	ShowView();
	ShowMenu();
	do {
		int bp_02;

/*C_10E8:*/
		bp_02 = u_kbread();
		if(u4_isupper((unsigned char)bp_02))
			bp_02 = (bp_02 & 0xff00) | u4_lower((unsigned char)bp_02);
		if((unsigned char)bp_02 > ' ' && ((unsigned char)bp_02 & 0x80) == 0)
			Gra_PutChar((unsigned char)bp_02);
		switch(bp_02) {
			case KBD_R:/*(R)eturn*/
				ShowView();
				ShowMenu();
			break;
			case KBD_I:/*(I)nitiate*/
				NewGame();
				RedrawTitle();
			break;
			case KBD_J:/*(J)ourney*/
				_ffree(pAnim);
				_ffree(pShapes);
				_ffree(pCharset);
				low_clean();
				exit(TwoDrives?'1':'0');
			break;
			default:
				sound_1();
			break;
		}
	} while(1);
}
