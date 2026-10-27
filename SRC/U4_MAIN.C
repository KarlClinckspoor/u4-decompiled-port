/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * The game: initialization (GameInit); if the game was saved in a dungeon,
 * the dungeon loop (DNG_main) until the party leaves it, else the party
 * restarts on the world map. Then the main loop: read a command key (or pass
 * after a while), run the command and end the turn (EndOfTurn) unless the
 * command took no time (TurnDone 0). PartyDeath and EndGame longjmp back
 * to GameLoopJmp, at the top of the loop.
 */
void cdecl /*C_191E*/main()
{
	register uint16 cmd;
	int16 prevCmd;

	low_init();
	GameInit();
	if(Party._loc >= LOC_DUNGEONS && Party._loc <= LOC_ABYSS) {
		CurMode = MOD_DUNGEON;
		if(Load("DNGMAP.SAV", sizeof(tMap8x8x8), &(Area._map)) == -1)
			exit(3);
		File_DNG = dopen(DungeonFiles[Party._loc - LOC_DUNGEONS], 0);
		if(setjmp(GameLoopJmp) == 0)
			DNG_main();
	} else {
		Party._loc = 0;
	}
	u_kbflush();
	prevCmd = 0;
	for(;;) {
		setjmp(GameLoopJmp);
		RefreshView();
		TileEffects();
		if(CheckPartyAlive()) {
			u4_putc(0x10);/*the prompt*/
			u_delay(25, 1);/*no key for a while: pass*/
			cmd = u_kbhit()?u_kbread():KBD_SPACE;
			if(u4_isupper((unsigned char)cmd))
				cmd = (cmd & 0xff00) | u4_lower((unsigned char)cmd);
			switch(cmd) {
				case KBD_SPACE: w_Pass(); break;
				case KBD_UP_:
				case KBD_UP: CMDDIR_Up(); break;
				case KBD_LEFT_:
				case KBD_LEFT: CMDDIR_Left(); break;
				case KBD_DOWN_:
				case KBD_DOWN: CMDDIR_Down(); break;
				case KBD_RIGHT_:
				case KBD_RIGHT: CMDDIR_Right(); break;
				case KBD_A: CMD_Attack(); break;
				case KBD_B: CMD_Board(); break;
				case KBD_C: CMD_Cast(); break;
				case KBD_D: CMD_Descend(); break;
				case KBD_E: CMD_Enter(); break;
				case KBD_F: CMD_Fire(); break;
				case KBD_G: CMD_Get(); break;
				case KBD_H: CMD_HoleUp(); break;
				case KBD_I: CMD_Ignite(); break;
				case KBD_J: CMD_Jimmy(); break;
				case KBD_K: CMD_Klimb(); break;
				case KBD_L: CMD_Locate(); break;
				case KBD_M: CMD_Mix(); break;
				case KBD_N: CMD_NewOrder(); break;
				case KBD_O: CMD_Open(); break;
				case KBD_P: CMD_Peer(); break;
				case KBD_Q: CMD_Quit(); break;
				case KBD_R: CMD_Ready(); break;
				case KBD_S: CMD_Search(); break;
				case KBD_T: CMD_Talk(); break;
				case KBD_U: CMD_Use(); break;
				case KBD_V: CMD_Volume(); break;
				case KBD_W: CMD_Wear(); break;
				case KBD_X: CMD_X_it(); break;
				case KBD_Y: CMD_Yell(); break;
				case KBD_Z: CMD_Ztats(); break;
				case KBD_CTRL_S:
					/*Alt-Z then Ctrl-S: cheat; else a bad command*/
					if(prevCmd == KBD_ALT_Z) {
						CheatShowKarma();
						break;
					}
				default:
					u4_puts("Bad command!\n");
					sound(SND_BAD_COMMAND);
					TurnDone = 0;
				break;
			}
		} else {
			u4_puts("Zzzzz\n");
		}
		if(TurnDone != 0) {
			EndOfTurn();
		} else {
			RefreshView();
			TurnDone = 1;
		}
		prevCmd = cmd;
	}
}

/*
 * Cheat (Alt-Z, Ctrl-S): print the 8 virtues' karma points,
 * 2 digits each (00 for an achieved virtue).
 */
/*C_1C21*/CheatShowKarma()
{
	register int16 virtue;

	Gra_CR();
	for(virtue = 0; virtue < 8; virtue++)
		u4_putl(*(pKarmas[virtue]), 2, '0');
	Gra_CR();
}

/*
 * What happens at the end of each turn: the ship's hull slowly repairs,
 * sleepers may wake up, the poisoned lose 2 hit points, the party eats
 * (starving costs 2 hit points to everyone), magic points come back, the
 * creatures move (unless the party is flying the balloon), spells wear off,
 * open doors close.
 */
/*C_1C53*/EndOfTurn()
{
	register int16 chara;
	register struct tChara *pChara;

	CheckPartyAlive();
	/*restore some hull*/
	if(Party._ship < 50 && U4_RND1(3) == 0)
		Party._ship ++;
	/*the asleep may wake up ('S' => 'G'), the poisoned take 2 hits*/
	for(pChara = Party.chara, chara = 0; chara < Party._members; pChara++, chara++) {
		if(pChara->_stat == 'S' && U4_RND1(7) == 0) {
			pChara->_stat = 'G';
		} else if(pChara->_stat == 'P') {
			hitChara(chara, 2);
			Gra_InvertStatusLine(chara); sound(SND_HIT); Gra_InvertStatusLine(chara);
		}
	}
	/*each character eats 1 food*/
	if(food_dec(Party._members)) {
		u4_puts("\nStarving!!!\n");
		for(chara = 0; chara < Party._members; chara ++) {
			if(isCharaAlive(chara))
				hitChara(chara, 2);
		}
		chara = Party._members;
		while(--chara >= 0)
			Gra_InvertStatusLine(chara);
		sound(SND_HIT);
		chara = Party._members;
		while(--chara >= 0)
			Gra_InvertStatusLine(chara);
	}
	MP_recover();
	if(Party._fly_light == 0) {
		VortexContacts();
		MoveNpcs();
		SpawnMonsters();
		VortexContacts();
	}
	Party._moves ++;
	if(spell_turns) {
		if(--spell_turns == 0)
			spell_effect = 7;/*no effect*/
	}
	dspl_Stats();
	/*automatic door closing*/
	AutoCloseDoor();
}
