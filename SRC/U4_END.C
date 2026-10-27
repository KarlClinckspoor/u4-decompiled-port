/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>

/*the questions at the end of the Abyss: the 8 virtues (answers in
  Names[NAME_VIRTUES + i]), then the 3 principles (PrincipleWords)*/
char * /*D_0BDA*/EndQuestions[] = {
	/*D_09BE*/"What dost thou possess if all may rely upon your every word?",
	/*D_09FB*/"What quality compels one to share in the journeys of others?",
	/*D_0A38*/"What answers when great deeds are called for?",
	/*D_0A66*/"What should be the same for Lord and Serf alike?",
	/*D_0A97*/"What is loath to place the self above aught else?",
	/*D_0AC9*/"What shirks no duty?",
	/*D_0ADE*/"What, in knowing the true self, knows all?",
	/*D_0B09*/"What is that which Serfs are born with but Nobles must strive to obtain?",
	/*D_0B52*/"If all else is imaginary, this is real...",
	/*D_0B7C*/"What plunges to the depths, while soaring on the heights?",
	/*D_0BB6*/"What turns not away from any peril?"
};

/*
 * Wait a while, or until a key is pressed (the key is then read).
 */
/*C_2F7E*/WaitKeyOrTimeout()
{
	u_delay(15, 1);
	if(u_kbhit())
		u_kbread();
}

/*where ExpelToSurface puts the party: 0-10 near the town or castle of the
  virtue or principle of the failed question, 11 near Lord British's castle
  (the last question), 12 the entrance of the Abyss*/
unsigned char /*D_0BF0*/ExpelX[] = {0xE7,0x53,0x23,0x3B,0x9E,0x69,0x17,0xBA,0xD8,0x1D,0x91,0x59,0xE9};
unsigned char /*D_0BFE*/ExpelY[] = {0x88,0x69,0xDD,0x2C,0x15,0xB7,0x81,0xAC,0x6A,0x30,0xF3,0x6A,0xE9};

/*the party's size before EndGame (where the Avatar is alone)*/
static unsigned /*D_8CCA*/SavedPartySize;

/*
 * Failure at the end of the game: the party is put back together on the
 * world map at the place number place (see ExpelX). Does not return:
 * jumps back to the main loop.
 */
/*C_2F9D*/ExpelToSurface(place)
unsigned place;
{
	u_delay(5, 0);
	Gra_CR();
	Party._x = ExpelX[place];
	Party._y = ExpelY[place];
	Party._members = SavedPartySize;
	Party._loc = 0;
	CurMode = MOD_OUTDOORS;
	spell_turns = 0;
	hit_tile = 0;
	spell_effect = 7;
	LineOfSight = 1;
	Gallop = 0;
	Party._fly_light = 0;
	LoadWorldWindow();
	dspl_Stats();
	u_kbflush();
	longjmp(GameLoopJmp, -1);
}

/*
 * Refused at the door of the Codex chamber: back to the entrance of the Abyss.
 */
/*C_3010*/PassageDenied()
{
	u4_puts(/*D_0C0B*/"\nPassage is not granted.\n");
	ExpelToSurface(12);
}

/*
 * The end: the Codex is revealed and the final texts are shown. The game
 * then hangs forever (the player has to reboot).
 */
/*C_3025*/Victory()
{
	u_delay(2, 0);
	ShakeScreen();
	ShakeScreen();
	ShakeScreen();
	ShakeScreen();
	u_delay(3, 0);
	dfree(pShapes);
	Gra_RevealCodex();/*the infinity symbol*/
	u_delay(3, 0);
	u4_puts(/*D_0C25*/"\n\nThe boundless knowledge of the Codex of Ultimate Wisdom is revealed unto thee.");
	WaitKeyOrTimeout();
	u4_puts(/*D_0C76*/"\n\nThe voice says: Thou hast proven thyself to be truly good in nature.");
	WaitKeyOrTimeout();
	u4_puts(/*D_0CBD*/"\n\nThou must know that thy quest to become an Avatar is the endless quest of a lifetime.");
	WaitKeyOrTimeout();
	u4_puts(/*D_0D15*/"\n\nAvatarhood is a living gift.  It must always and forever be nurtured to flourish.");
	WaitKeyOrTimeout();
	u4_puts(/*D_0D69*/"\n\nFor if thou dost stray from the paths of virtue, thy way may be lost forever.");
	WaitKeyOrTimeout();
	u4_puts(/*D_0DB9*/"\n\nReturn now unto thine own world. Live there as an example to thy people, as our memory of thy gallant deeds serves us.");
	WaitKeyOrTimeout();
	Gra_ClearView();
	u4_puts(/*D_0E32*/"\n\nAs the sound of the voice trails off, darkness seems to rise around you. There is a moment of intense, wrenching vertigo.");
	WaitKeyOrTimeout();
	Gra_LoadPicture((GraphicsType == 1)?/*D_0EAE*/"stoncrcl.pic":/*D_0EBB*/"stoncrcl.ega", 0, 0);
	u4_puts(/*D_0EC8*/"\n\nYou open your eyes to a familiar circle of stones.  You wonder of your recent adventures.");
	WaitKeyOrTimeout();
	u4_puts(/*D_0F24*/"\n\nIt seems a time and place very distant.  You wonder if it really happened. Then you realize that in your hand you hold The Ankh.");
	WaitKeyOrTimeout();
	u4_puts(/*D_0FA7*/"\n\nYou walk away from the circle, knowing that you can always return from whence you came, since you now know the secret of the gates.");
	WaitKeyOrTimeout();
	u4_puts(/*D_102D*/"\n\nCONGRATULATIONS!\n   Thou hast\n   completed\n   ULTIMA IV\n  Quest of the\n    AVATAR\n  in ");
	u4_putl(Party._moves, 0, '0');
	u4_puts(/*D_1087*/"\n turns! Report\n thy feat unto\nLord British at\nOrigin Systems!");
	while(1);
}

/*
 * Ask the question up to 3 times. Returns 1 if the reply matches answer
 * (case ignored), 0 after 3 wrong replies.
 */
/*C_310F*/AskThrice(question, answer)
char *question;
char *answer;
{
	char reply[16];

	u4_puts(question);
	Gra_CR();
	Gra_CR();
	u4_gets(reply, 15);
	Gra_CR();
	if(strnicmp(reply, answer, 15) != 0) {
		u_delay(1, 0);
		u4_puts(/*D_10C6*/"\nThy thoughts are not pure.\nI ask again.\n");
		u_delay(2, 0);

		u4_puts(question);
		Gra_CR();
		Gra_CR();
		u4_gets(reply, 15);
		Gra_CR();
		if(strnicmp(reply, answer, 15) != 0) {
			u_delay(1, 0);
			u4_puts(/*D_10F0*/"\nThy thoughts are not pure.\nI ask again.\n");
			u_delay(2, 0);

			u4_puts(question);
			Gra_CR();
			Gra_CR();
			u4_gets(reply, 15);
			Gra_CR();
			if(strnicmp(reply, answer, 15) != 0)
				return 0;
		}
	}
	return 1;
}

/*the answers to the 3 last questions, and the names of their pictures*/
char * /*D_161A*/PrincipleWords[] = {
	/*D_111A*/"truth",
	/*D_1120*/"love",
	/*D_1125*/"courage"
};

/*the names of the pictures shown after each virtue's question*/
char * /*D_1620*/VirtuePictures[] = {
	/*D_112D*/"honesty",
	/*D_1135*/"compassn",
	/*D_113E*/"valor",
	/*D_1144*/"justice",
	/*D_114C*/"sacrific",
	/*D_1155*/"honor",
	/*D_115B*/"spirit",
	/*D_1162*/"humility"
};

/*
 * The end of the game, in the Codex chamber at the bottom of the Abyss (after
 * using the black stone, see USE_Stones). The Avatar, alone, needs the
 * Three Part Key, the Word of Passage, a party of 8 and all the virtues
 * achieved; then must answer the 11 questions of the virtues and principles
 * and the last one (infinity). Any failure expels the party to the surface
 * (ExpelToSurface); success ends the game (Victory).
 */
/*C_31F4*/EndGame()
{
	int i;
	char answer[16];

	SavedPartySize = Party._members;
	Party._members = 1;
	CurMode = MOD_VISION;
	Gra_ClearStatus();
	dspl_Stats();
	Gra_ClearView();
	u4_puts(/*D_116B*/"\n\n\nThere is a sudden darkness, and you find yourself alone in an empty chamber.\n");
	u_delay(4, 0);
	Gra_LoadPicture((GraphicsType == 1)?/*D_11BC*/"key7.pic":/*D_11C5*/"key7.ega", 0, 0);
	if(
		!TST_MSK(Party.mItems, ST_KEY_C) |
		!TST_MSK(Party.mItems, ST_KEY_T) |
		!TST_MSK(Party.mItems, ST_KEY_L)
	) {
		u4_puts(/*D_11CE*/"\nThou dost not have the Key of Three Parts.\n");
		ExpelToSurface(12);
	}
	u4_puts(/*D_11FB*/"\nYou use your Key of Three Parts.\n");
	u_delay(3, 0);
	u4_puts(/*D_121E*/"\nA voice rings out:\n");
	if(!AskThrice(/*D_123D*/"\"What is the Word of Passage?\"", /*D_1233*/"veramocor"))
		PassageDenied();
	if(SavedPartySize != 8) {
		u4_puts(/*D_125C*/"\nThou hast not proved thy leadership in all eight virtues.\n");
		u_delay(8, 0);
		PassageDenied();
	}
	for(i = 7; i >= 0; i --) {
		if(*(pKarmas[i])) {
			u4_puts(/*D_1298*/"\nThou art not ready.\n");
			PassageDenied();
		}
	}
	u4_puts(/*D_12AE*/"\nPassage is granted.\n");
	u_delay(5, 0);
	Gra_ClearView();
	for(i = 0; i < 11; i ++) {
		char picture[20];

		u_delay(2, 0);
		u4_puts(/*D_12C4*/"\n\nThe voice asks:\n\n");
		u_delay(2, 0);
		if(!AskThrice(EndQuestions[i], (i<=7)?Names[NAME_VIRTUES + i]:PrincipleWords[i-8])) {
			u_delay(1, 0);
			u4_puts(/*D_12D8*/"\nThy quest is not yet complete.\n");
			ExpelToSurface(i);
		}
		strcpy(picture, (i >= 8)?PrincipleWords[i-8]:VirtuePictures[i]);
		strcat(picture, (GraphicsType == 1)?/*D_12F9*/".pic":/*D_12FE*/".ega");
		Gra_LoadPicture(picture, 0, 0);
		if(i == 7) {
			u_delay(3, 0);
			u4_puts(/*D_1303*/"\nThou art well versed in the virtues of the Avatar.\n");
			u_delay(5, 0);
		}
	}
	u4_puts(/*D_1338*/"\n\nThe floor rumbles beneath your feet.\n");
	ShakeScreen();
	ShakeScreen();
	u_delay(5, 0);
	u4_puts(/*D_1360*/"\nAbove the din, the voice asks:\n\nIf all eight virtues of the Avatar combine into and are derived from the Three Principles of Truth, Love and Courage...");
	WaitKeyOrTimeout();
	u4_puts(/*D_13F9*/"\n\nThen what is the one thing which encompasses and is the whole of all undeniable Truth, unending Love, and unyielding Courage?\n\n");
	u4_gets(answer, 15);
	Gra_CR();
	if(strnicmp(answer, /*D_147B*/"infinity", 16) != 0) {
		u4_puts(/*D_1484*/"\nThy thoughts are not pure.\nI ask again.\n");
		u_delay(3, 0);
		u4_puts(/*D_14AE*/"\n\nWhat is the one thing which encompasses and is the whole of all undeniable Truth, unending Love, and unyielding Courage?\n\n");
		u4_gets(answer, 15);
		Gra_CR();
		if(strnicmp(answer, /*D_152B*/"infinity", 16) != 0) {
			u4_puts(/*D_1534*/"\nThy thoughts are not pure.\nI ask again.\n");
			u_delay(3, 0);
			u4_puts(/*D_155E*/"\n\nWhat is the one thing which encompasses and is the whole of all undeniable Truth, unending Love, and unyielding Courage?\n\n");
			u4_gets(answer, 15);
			Gra_CR();
			if(strnicmp(answer, /*D_15DB*/"infinity", 16) != 0) {
				u_delay(1, 0);
				u4_puts(/*D_15E4*/"\nThou dost not know the true nature of the Universe.\n");
				ExpelToSurface(11);
			}
		}
	}
	Victory();
}
