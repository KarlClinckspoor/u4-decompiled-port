/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "title.h"

#include <malloc.h>
#include <stdlib.h>

int16 /*D_00C0*/MonsterAnimDelay = 0;
int16 /*D_00C2*/TileAnimDelay = 1;
int16 /*D_00C4*/ViewScriptPos = -1;
int16 /*D_00C6*/ViewFrameWait = 0;

/*Base tile of each of the view's 15 actors (the low nibble of a ViewScript command):
 moongate, pirate ship, ship, 2 sleeping people, 2 rogues, fighter, mage, sea serpent,
 daemon, dragon, missile, hit flash, magic bolt. The script adds the frame / direction.*/
unsigned char /*D_00C8*/ViewBaseTiles[] = {0x40,0x80,0x10,0x38,0x38,0xC8,0xC8,0x24,0x20,0x88,0xF0,0xF8,0x4D,0x4F,0x4E};

/*====---- _BSS ----====*/
static unsigned char D_3A24[19 * 5];
static uint16 D_3A84;
/*The statics keep their address-like names: Microsoft C 4.00 lays out a module's
 uninitialised statics in the order of its symbol table, which depends on the names.
 Renaming them would move them; the readable names are macros.
 ViewTiles: the 19x5 tiles of the current frame of the view (background + actors).
 ViewRunning: set while ShowView runs; makes AnimateMonsters redraw at every call.*/
#define ViewTiles D_3A24
#define ViewRunning D_3A84

/*
 * Animate the two creatures drawn in the top corners of the title screen: when its delay
 * runs out, draw the next frame of each one's sequence (if MonstersShown). Called from
 * u_delay and from u_kbhit when no key is waiting, so the creatures move during every
 * wait; while the view runs (ViewRunning) it draws at every call.
 * Also picks the text cursor's blink rate.
 */
int16 /*C_019A*/AnimateMonsters()
{
	uint16 frame;

	if(ViewRunning)
		MonsterAnimDelay = 0;
	if(MonsterAnimDelay --)
		return;
	MonsterAnimDelay = ((GraphicsType == 1)?120:200) * speed_info;
	if(MonstersShown) {
		cursor_delay = 200;

		frame = Monster1Seq[Monster1Pos++]; Monster1Pos &= 0x7f;
		Gra_Blit(6, 32, Monster1FrameX[frame], MonsterFrameY[frame], pAnim, 0, -1, 0);
		frame = Monster2Seq[Monster2Pos++]; Monster2Pos &= 0x3f;
		Gra_Blit(6, 32, Monster2FrameX[frame], MonsterFrameY[frame], pAnim, 0, -1, 34);
	} else {
		cursor_delay = 233;
	}
}

/*
 * Print character c at the text cursor and advance it; a backspace (8) moves the cursor
 * back and blanks that position.
 */
int16 /*C_027A*/u4_putc(c)
unsigned char c;
{
	if(c == 8) {
		txt_X --;
		Gra_PutChar(' ');
	} else {
		Gra_PutChar(c);
		txt_X ++;
	}
}

/*
 * Busy-wait count * speed_info iterations, unless a key is waiting (then return at once).
 */
int16 /*C_02A3*/DelayUnlessKey(count)
int32 count;
{
	if(u_kbhit())
		return;
	count *= (int32)speed_info;
	while(-- count) BUSY_PASS;
}

/*
 * Draw Lord British's signature dot by dot from SignatureDots (x, y pairs ending with 0,
 * y counted from the bottom); each dot is 2 pixels wide. With slowly set, pause after each dot so the signature looks
 * handwritten, then wait a moment at the end.
 */
int16 /*C_02D1*/DrawSignature(slowly)
uint16 slowly;
{
	register int16 dot;

	for(dot = 0; SignatureDots[dot]; ) {
		Gra_Dot(191 - SignatureDots[dot+1], SignatureDots[dot] + 20, 3);
		Gra_Dot(191 - SignatureDots[dot+1], SignatureDots[dot] + 21, 3);
		dot += 2;
		if(slowly)
			DelayUnlessKey((int32)440);
	}
	if(slowly)
		u_delay(1, 1);
}

/*
 * Animate the view's tiles: two-frame actors (sea creatures 0x84-0x8e, player classes
 * 0x20-0x2e, townsfolk 0x50-0x5e) switch frame and four-frame monsters (0x90 and up)
 * step to their next frame, each 3 times out of 4; the pirate ship (0x80) keeps the
 * direction the script gave it; other actors show their base tile. Every few calls
 * (depending on speed_info) also animate the water, lava and field tiles and the flags.
 */
int16 /*C_034D*/AnimateViewTiles()
{
	register int16 slot;

	for(slot = 0; slot < 32; slot ++) {
		register int16 tile;

		tile = Area._npc._tile[slot];
		if(
			(tile >= 0x84 && tile <= 0x8e) ||
			(tile >= 0x20 && tile <= 0x2e) ||
			(tile >= 0x50 && tile <= 0x5e)
		) {
			/*the carry: the tile is below the top of its range (the last comparison)*/
			if(RND2_CARRY(tile != 0x8e && tile != 0x2e && tile != 0x5e) u_rand_b() < 0xc0)
				Area._npc._gtile[slot] = ((Area._npc._gtile[slot] & 1) | tile) ^ 1;
		} else if(tile >= 0x90) {
			if(u_rand_b() < 0xc0)
				Area._npc._gtile[slot] = ((Area._npc._gtile[slot] + 1) & 3) | tile;
		} else if(tile != 0x80) {
			Area._npc._gtile[slot] = tile;
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

/*
 * Advance the view's script (ViewScript) by one frame, unless the current frame must still
 * be held (ViewFrameWait). Script commands, one byte with the actor in the low nibble:
 *   0x00-0x6f: place the actor; the high nibble is its row and the next byte holds its
 *              column (low 5 bits) and the frame added to its base tile (high 3 bits);
 *   0x7x:      remove the actor;
 *   0x8x:      end of frame: build ViewTiles from the background and the actors, and
 *              hold this frame for x more calls;
 *   0x9x-0xfx: end of script, start it again.
 */
int16 /*C_041A*/StepViewScript()
{
	int16 idx;

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
				for(idx = 0; idx < 19 * 5; idx ++)
					ViewTiles[idx] = ViewBackground[idx];
				for(idx = 0; idx < 32; idx ++) {
					if(Area._npc._gtile[idx])
						ViewTiles[19 * Area._npc._y[idx] + Area._npc._x[idx]] = Area._npc._gtile[idx];
				}
				ViewFrameWait = ViewScript[ViewScriptPos] & 0xf;
			return;
			case 0x90: case 0xa0: case 0xb0: case 0xc0: case 0xd0: case 0xe0: case 0xf0:
				ViewScriptPos = -1;
			break;
		}
	}
}

/*
 * Run the animated view (the scene below the title) until a key is pressed; the key is
 * then read and discarded. Each frame is drawn several times depending on speed_info.
 */
int16 /*C_05A4*/ShowView()
{
	register int16 repeat;

	ViewRunning = 1;
	cursor_enabled = 0;
	while(!u_kbhit()) {
		AnimateViewTiles();
		StepViewScript();
		MonstersShown = 0;
		Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		for(repeat = speed_info / 2; repeat && !u_kbhit(); repeat --)
			Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		AnimateViewTiles();
		StepViewScript();
		Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		for(repeat = speed_info / 2; repeat && !u_kbhit(); repeat --)
			Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		MonstersShown = 1;
	}
	ViewRunning = 0;
	u_kbread();
}

/*
 * The opening sequence: the signature, "and", a line, "Origin Systems Inc.", "present",
 * the "Ultima IV" logo, "Quest of the Avatar", the view opening like a scroll, and the
 * two creatures rising into the top corners. A key press skips the slow parts.
 * temp is used both as a column and as a creature's frame.
 */
int16 /*C_068C*/TitleSequence()
{
	register uint16 step, redraw;
	int16 temp;

	u_delay(1, 1);
	/*"lord british"*/
	DrawSignature(1);
	/*"and"*/
	Gra_Blit(4, 4, 19, 17, pTitle, 17, -1, 19);
	u_delay(1, 1);
	/*draw line*/
	for(step = 86; step < 238; step++) {
		Gra_Dot(31, step, 2);
		DelayUnlessKey((int32)320);
	}
	/*"origin systems inc."*/
	for(step = 1; step < 10; step ++) {
		Gra_Blit(21, step, 9, 21, pTitle, 30 - step, -1, 9);
		DelayUnlessKey((int32)5000);
	}
	u_delay(1, 1);
	/* */
	if(!u_kbhit()) {
		/*"present"*/
		for(step = 1; step <= 5; step++) {
			Gra_Blit(15, step, 14, 5 - step, pTitle, 33, -1, 14);
			DelayUnlessKey((int32)5000);
		}
		u_delay(1, 1);
		/*"Ultima iv"*/
		for(step = 0; step < 57; step ++) {
			if(u_kbhit())
				step = 56;
			Gra_BlitReveal(30, 45, 5, 34, pTitle, 34, step, 5);
			if(speed_info > 4)
				Gra_BlitReveal(30, 45, 5, 34, pTitle, 34, step, 5);
		}
		u_delay(1, 1);
	} else {
		Gra_Blit(30, 45,  5, 34, pTitle, 34, -1,  5);
		Gra_Blit(15,  5, 14,  0, pTitle, 33, -1, 14);
	}
	/*"quest of the avatar"*/
	for(step = 1; step <= 6; step ++) {
		Gra_Blit(33, step, 3,      81, pTitle, 86 - step, -1, 3);
		Gra_Blit(33, step, 3, 93 - step, pTitle,      86, -1, 3);
		DelayUnlessKey((int32)4000);
	}
	/*open view like scroll*/
	for(step = 1; step <= 20; step ++) {
		if(u_kbhit())
			step = 20;
		AnimateViewTiles();
		Gra_DrawViewOver(5, 19, ViewBackground, 104, pTitle, 2);
		if(speed_info > 4)
			Gra_DrawViewOver(5, 19, ViewBackground, 104, pTitle, 2);
		temp = 20 - step;
		Gra_Blit(step * 2, 96, temp, 96, pTitle, 96, -1, temp);
		DelayUnlessKey((int32)680);
		AnimateViewTiles();
		Gra_DrawViewOver(5, 19, ViewBackground, 104, pTitle, 2);
	}
	_ffree(pTitle);

	/*the 2 "monsters"*/
/*C_0978:*/
	if((pAnim = (void far *)_fmalloc((GraphicsType == 1)?0x4000:0x8000)) == 0)
		exit(0x37);
	Gra_LoadPicture((GraphicsType == 1)?/*D_00D7*/"ANIMATE.PIC":/*D_00E3*/"ANIMATE.EGA", pAnim);
	for(step = 1; step <= 32; step++) {
/*C_09C4:*/
		if(u_kbhit())
			step = 32;

		/*the "2 monsters"*/
		temp = Monster1Seq[Monster1Pos++]; Monster1Pos &= 0x7f;
		Gra_Blit(6, step, Monster1FrameX[temp], MonsterFrameY[temp] - step + 32, pAnim, 0, -1, 0);
		temp = Monster2Seq[Monster2Pos++]; Monster2Pos &= 0x3f;
		Gra_Blit(6, step, Monster2FrameX[temp], MonsterFrameY[temp] - step + 32, pAnim, 0, -1, 34);
		/*the "view"*/
		AnimateViewTiles();
		StepViewScript();
		Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		for(redraw = speed_info / 2; redraw && !u_kbhit(); redraw --)
			Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		AnimateViewTiles();
		StepViewScript();
		Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
		for(redraw = speed_info / 2; redraw && !u_kbhit(); redraw --)
			Gra_DrawView(5, 19, ViewTiles, 104, 0, 0, 2);
	}
}

/*
 * Print the string str at text position (row, col).
 */
int16 /*C_0B1E*/PutsAt(row, col, str)
int16 row;
int16 col;
register char *str;
{
	txt_Y = row;
	txt_X = col;
	while(*str)
		u4_putc(*str++);
}

/*
 * Draw the title menu (Return to the view, Journey Onward, Initiate New Game) and put
 * the blinking cursor after "Options:".
 */
int16 /*C_0B45*/ShowMenu()
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

/*
 * Redraw the whole title screen without animation after the new game creation (which
 * used the screen): reload the tiles and pictures, draw the logo and the two creatures,
 * then the menu. temp is used both as a column and as a creature's frame.
 */
int16 /*C_0BCA*/RedrawTitle()
{
	int16 temp;

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
	for(temp = 86; temp < 238; temp ++)
		Gra_Dot(31, temp, 2);
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

	temp = Monster1Seq[Monster1Pos++]; Monster1Pos &= 0x7f;
	Gra_Blit(6, 32, Monster1FrameX[temp], MonsterFrameY[temp], pAnim, 0, -1, 0);
	temp = Monster2Seq[Monster2Pos++]; Monster2Pos &= 0x3f;
	Gra_Blit(6, 32, Monster2FrameX[temp], MonsterFrameY[temp], pAnim, 0, -1, 34);

	MonstersShown = 1;
	ShowMenu();
}

/*
 * TITLE.EXE entry point. The command line (in the PSP) may give the graphics card as its
 * first one-letter argument (C = CGA, E = EGA, T = Tandy) and whether there are two
 * floppy drives as its second (0 or 1); otherwise they are detected. Loads the character set
 * and tiles, plays the opening sequence, then runs the menu: R shows the view again,
 * I creates a new game (NewGame), J exits with '0' or '1' (one or two drives) as the exit
 * code, for the batch file that runs AVATAR.EXE.
 */
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
	if(GraphicsType == (uint16)(-1)) {
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
		int16 key;

/*C_10E8:*/
		key = u_kbread();
		if(u4_isupper((unsigned char)key))
			key = (key & 0xff00) | u4_lower((unsigned char)key);
		if((unsigned char)key > ' ' && ((unsigned char)key & 0x80) == 0)
			Gra_PutChar((unsigned char)key);
		switch(key) {
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
