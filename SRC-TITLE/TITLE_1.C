/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

/*thanks to Fenyx4 for the variable
 name issue fix*/

#include "title.h"

#include <malloc.h>
#include <stdlib.h>
#include <string.h>

/*====---- _BSS ----====*/
/*As in TITLE_0.C, the statics keep placeholder names: Microsoft C 4.00 lays out a
 module's uninitialised statics in the order of its symbol table, which depends on the
 names, so renaming them would move them. The readable names are the macros below.
 They hold the new character being created:
   PlayerDrive      drive of the PLAYER disk (1 = A:)
   lastVirtue       the virtue chosen in the last dilemma (0 = Honesty .. 7 = Humility),
                    which decides the class, the start position and the party leader
   M_or_F           'M' or 'F'
   tmp_str/dex/int  strength, dexterity, intelligence being built
   pPicture         the current intro picture
   curQuestionIndex number of dilemmas answered (0..7)
   player_name      the name typed
   tmp_karma        the 8 karma values being built*/
/*D_6938*/static uint16 static_A;
/*D_693A*/static int16 static_B;
/*D_693C*/static int16 static_C;
/*D_693E*/static unsigned char static_D;
/*D_6940*/static void far *static_E;
/*D_6944*/static unsigned char static_F;
/*D_6946*/static uint16 static_G;
/*D_6948*/static char static_H[20];
/*D_695C*/static unsigned char static_I;
/*D_695E*/static unsigned char static_J[8];

#define PlayerDrive static_A
#define lastVirtue static_B
#define M_or_F static_C
#define tmp_str static_D
#define pPicture static_E
#define tmp_dex static_F
#define curQuestionIndex static_G
#define player_name static_H
#define tmp_int static_I
#define tmp_karma static_J
/*====---- ----====*/


#if 0
#include "joe.h"
#endif

#define STR(i) (Texts[(i) - 1])

char * /*D_2EE6*/Texts[0x43] = {
/*0x01*//*D_0424*/
"Entrusted to deliver an uncounted purse\n\
of gold, thou dost meet a poor beggar.\n\
Dost thou A) deliver the gold knowing\n\
the Trust in thee was well placed, or\n\
B) show Compassion, giving the beggar a\n\
coin, knowing it won't be missed?",

/*0x02*//*D_0509*/
"Thou has been prohibited by thy absent\n\
Lord from joining thy friends in a close\n\
pitched battle. Dost thou A) refrain, so\n\
thou may Honesty claim obedience, or\n\
B) show Valor, and aid thy comrades,\n\
knowing thou may deny it later?",

/*0x03*//*D_05EC*/
"A merchant owes thy friend money, now\n\
long past due.  Thou dost see the same\n\
merchant drop a purse of gold. Dost thou\n\
A) Honestly return the purse intact, or\n\
B) Justly give thy friend a portion of\n\
the gold first?",

/*0x04*//*D_06C1*/
"Thee and thy friend are valiant but\n\
penniless warriors.  Thou both go out to\n\
slay a mighty dragon.  Thy friend thinks\n\
he slew it; thee did.  When asked, dost\n\
thou A) Truthfully claim the gold, or\n\
B) Allow thy friend the large reward?",

/*0x05*//*D_07AB*/
"Thou art sworn to protect thy Lord at\n\
any cost, yet thou know he hath\n\
committed a crime.  Authorities ask thee\n\
of the affair.  Dost thou A) break thine\n\
oath by Honestly speaking, or B) uphold\n\
Honor by silently keeping thine oath?",

/*0x06*//*D_0891*/
"Thy friend seeks admittance to thy\n\
Spiritual order. Thou art asked to vouch\n\
for his purity of Spirit, of which thou\n\
art unsure.  Dost thou A) Honestly\n\
express thy doubt, or B) Vouch for him\n\
hoping for his Spiritual improvement?",

/*0x07*//*D_0975*/
"Thy Lord mistakenly believes he slew a\n\
dragon.  Thou hast proof that thy lance\n\
felled the beast.  When asked, dost thou\n\
A) Honestly claim the kill and the\n\
prize, or B) Humbly permit thy Lord his\n\
belief?",

/*0x08*//*D_0A40*/
"Thou dost manage to disarm thy mortal\n\
enemy in a duel.  He is at thy mercy.\n\
Dost thou A) show Compassion by\n\
permitting him to yield, or B) slay him\n\
as expected of a Valiant duelist?",

/*0x09*//*D_0AF6*/
"After 20 years thou hast found the\n\
slayer of thy best friends.  The villain\n\
proves to be a man who provides the sole\n\
support for a young girl.  Dost thou\n\
A) spare him in Compassion for the girl,\n\
or B) slay him in the name of Justice?",

/*0x0a*//*D_0BE0*/
"Thee and thy friends have been routed\n\
and ordered to retreat.  In defiance of\n\
thy orders, dost thou A) stop in\n\
Compassion to aid a wounded companion,\n\
or B) Sacrifice thyself to slow the\n\
pursuing enemy, so others may escape?",

/*0x0b*//*D_0CC0*/
"Thou art sworn to uphold a Lord who\n\
participates in the forbidden torture of\n\
prisoners.  Each night their cries of\n\
pain reach thee.  Dost thou A) show\n\
Compassion by reporting the deeds, or\n\
B) Honor thy oath and ignore the deeds?",

/*0x0c*//*D_0DA5*/
"Thou hast been taught to preserve all\n\
life as sacred. A man lies fatally stung\n\
by a venomous serpent.  He pleads for a\n\
merciful death.  Dost thou A) show\n\
Compassion and end his pain, or B) heed\n\
thy Spiritual beliefs and refuse?",

/*0x0d*//*D_0E89*/
"As one of the King's Guard, thy Captain\n\
has asked that one amongst you visit\n\
a hospital to cheer the children with\n\
tales of thy valiant deeds.  Dost thou\n\
A) Show thy Compassion and play the\n\
braggart, or B) Humbly let another go?",

/*0x0e*//*D_0F6E*/
"Thou hast been sent to secure a needed\n\
treaty with a distant Lord.  Thy host is\n\
agreeable to the proposal but insults\n\
thy country at dinner.  Dost thou\n\
A) Valiantly bear the slurs, or\n\
B) Justly rise and demand an apology?",

/*0x0f*//*D_104C*/
"A mighty knight accosts thee and demands\n\
thy food.  Dost thou A) Valiantly refuse\n\
and engage the knight, or B) Sacrifice\n\
thy food unto the hungry knight?",

/*0x10*//*D_10E6*/
"During battle thou art ordered to guard\n\
thy commander's empty tent.  The battle\n\
goes poorly and thou dost yearn to aid\n\
thy fellows.  Dost thou A) Valiantly\n\
enter the battle to aid thy companions,\n\
or B) Honor thy post as guard?",

/*0x11*//*D_11C9*/
"A local bully pushes for a fight.  Dost\n\
thou A) Valiantly trounce the rogue, or\n\
B) Decline, knowing in thy Spirit that\n\
no lasting good will come of it?",

/*0x12*//*D_1261*/
"Although a teacher of music, thou art\n\
a skillful wrestler.  Thou hast been\n\
asked to fight in a local championship.\n\
Dost thou A) accept the invitation and\n\
Valiantly fight to win, or B) Humbly\n\
decline knowing thou art sure to win?",

/*0x13*//*D_1346*/
"During a pitched battle, thou dost see\n\
a fellow desert his post, endangering\n\
many.  As he flees, he is set upon by\n\
several enemies. Dost thou A) Justly let\n\
him fight alone, or B) Risk Sacrificing\n\
thine own life to aid him?",

/*0x14*//*D_1425*/
"Thou hast sworn to do thy Lord's\n\
bidding in all.  He covets a piece of\n\
land and orders the owner removed.  Dost\n\
thou A) serve Justice refusing to act,\n\
thus being disgraced, or B) Honor thine\n\
oath and unfairly evict the landowner?",

/*0x15*//*D_150B*/
"Thou dost believe that virtue resides\n\
in all people.  Thou dost see a rogue\n\
steal from thy Lord.  Dost thou A) call\n\
him to Justice, or B) personally try to\n\
sway him back to the Spiritual path of\n\
good?",

/*0x16*//*D_15D4*/
"Unwitnessed, thou hast slain a great\n\
dragon in self defense.  A poor warrior\n\
claims the offered reward.  Dost thou\n\
A) Justly step forward to claim the\n\
reward, or B) Humbly go about life,\n\
secure in thy self esteem?",

/*0x17*//*D_16AA*/
"Thou art a bounty hunter sworn to return\n\
an alleged murder.  After his capture\n\
thou believest him to be innocent.  Dost\n\
thou A) Sacrifice thy sizable bounty\n\
for thy belief, or B) Honor thy oath to\n\
return him as thou hast promised?",

/*0x18*//*D_1791*/
"Thou hast spent thy life in charitable\n\
and righteous work.  Thine uncle the\n\
innkeeper lies ill and asks you to take\n\
over his tavern.  Dost thou A) Sacrifice\n\
thy life of purity to aid thy kin, or\n\
B) decline & follow thy Spirit's call?",

/*0x19*//*D_187B*/
"Thou art an elderly, wealthy eccentric.\n\
Thy end is near. Dost thou A) donate all\n\
thy wealth to feed hundreds of starving\n\
children, and receive public adulation,\n\
or B) Humbly live out thy life, willing\n\
thy fortune to thy heirs?",

/*0x1a*//*D_195E*/
"In thy youth thou pledged to marry thy\n\
sweetheart.  Now thou art on a sacred\n\
quest in distant lands.  Thy sweetheart\n\
asks thee to keep thy vow.  Dost thou\n\
A) Honor thy pledge to wed, or B) follow\n\
thy Spiritual crusade?",

/*0x1b*//*D_1A39*/
"Thou art at a crossroads in thy life.\n\
Dost thou A) Choose the Honorable life\n\
of a Paladin, striving for Truth and\n\
Courage, or B) Choose the Humble life\n\
of a Shepherd, and a world of simplicity\n\
and peace?",

/*0x1c*//*D_1B05*/
"Thy parents wish thee to become an\n\
apprentice. Two positions are available.\n\
Dost thou A) Become an acolyte in the\n\
Spiritual order, or B) Become an\n\
assistant to a humble village cobbler?",


/*0x1d*//*D_1BBF*/
"  The day is warm, yet there is a\n\
cooling breeze.  The latest in a series\n\
of personal crises seems insurmountable.\n\
You are being pulled apart in all\n\
directions.",

/*0x1e*//*D_1C60*/
"Yet this afternoon walk in the country-\n\
side slowly brings relaxation to your\n\
harried mind.  The soil and strain of\n\
modern high-tech living begins to wash\n\
off in layers. That willow tree near the\n\
stream looks comfortable and inviting.",

/*0x1f*//*D_1D4B*/
"The buzz of dragonflies and the whisper\n\
of the willow's swaying branches bring\n\
a deep peace.  Searching inward for\n\
tranquility and happiness, you close\n\
your eyes.",

/*0x20*//*D_1DEE*/
"A high-pitched cascading sound like\n\
crystal wind-chimes impinges on your\n\
floating awareness.  As you open your\n\
eyes, you see a shimmering blueness rise\n\
from the ground.  The sound seems to be\n\
emanating from this glowing portal.",

/*0x21*//*D_1ED2*/
"It is difficult to look at the blueness.\n\
Light seems to bend and distort around\n\
it, while the sound waves become so\n\
intense, they appear to become visible.",

/*0x22*//*D_1F6E*/
"The portal hangs there for a moment;\n\
then, with the rush of an imploding\n\
vacuum, it sinks into the ground.\n\
Something remains suspended in mid-air\n\
for a moment before falling to earth\n\
with a heavy thud.",

/*0x23*//*D_2038*/
"Somewhat shaken by this vision, you\n\
rise to your feet to investigate.  A\n\
crude circle of stones surrounds the\n\
spot where the portal appeared. There is\n\
something glinting in the grass.",

/*0x24*//*D_20F0*/
"You pick up an amulet shaped like a\n\
cross with a loop at the top.  It is an\n\
Ankh, the sacred symbol of life and\n\
rebirth.  But this could not have made\n\
the thud, so you look again and find\n\
a large book wrapped in thick cloth!",

/*0x25*//*D_21D1*/
"With trembling hands you unwrap the\n\
book.  Behold, the cloth is a map, and\n\
within lies not one book, but two.  The\n\
map is of a land strange to you, and the\n\
style speaks of ancient cartography.",

/*0x26*//*D_2292*/
"The script on the cover of the first\n\
book is arcane but readable.  The title\n\
is:\n\
        The History of Britannia\n\
               as told by\n\
            Kyle the Younger",

/*0x27*//*D_233B*/
"The other book is disturbing to look at.\n\
Its small cover appears to be fashioned\n\
out of some sort of leathery hide, but\n\
from what creature is uncertain.  The\n\
reddish-black skin radiates an intense\n\
aura suggestive of ancient power.",

/*0x28*//*D_2422*/
"The tongue of the title is beyond your\n\
ken.  You dare not open the book and\n\
disturb whatever sleeps within.  You\n\
decide to peruse the History.  Settling\n\
back under the willow tree, you open the\n\
book.",

/*0x29*//*D_24EA*/
"\n\
\n\
    (You read the Book of History)",

/*0x2a*//*D_250F*/
"\n\
\n\
(No, really! Read the Book of History!)",

/*0x2b*//*D_2539*/
"Closing the book, you again pick up\n\
the Ankh.  As you hold it, you begin to\n\
hear a hauntingly familiar, lute-like\n\
sound wafting over a nearby hill.  Still\n\
clutching the strange artifacts, you\n\
rise unbidden and climb the slope.",

/*0x2c*//*D_261C*/
"In the valley below you see what appears\n\
to be a fair.  It seems strange that you\n\
came that way earlier and noticed\n\
nothing.  As you mull this over, your\n\
feet carry you down towards the site.",

/*0x2d*//*D_26DC*/
"This is no ordinary travelling\n\
carnival, but a Renaissance Fair.  The\n\
pennants on the tent tops blow briskly\n\
in the late afternoon breeze.",

/*0x2e*//*D_2767*/
"The ticket taker at the RenFair's gate\n\
starts to ask you for money, but upon\n\
spotting your Ankh says, \"Welcome,\n\
friend.  Enter in peace and find your\n\
path.\"",

/*0x2f*//*D_2804*/
"The music continues to pull you\n\
forward amongst the merchants and\n\
vendors.  Glimpses of fabulous treasures\n\
can be seen in some of the shadowy\n\
booths.",

/*0x30*//*D_289A*/
"These people are very happy.  They seem\n\
to glow with an inner light.  Some look\n\
up as you pass and smile, but you cannot\n\
stop - the music compels you to move\n\
onward through the crowd.",

/*0x31*//*D_2952*/
"Through the gathering dusk you see a\n\
secluded gypsy wagon sitting off in the\n\
woods.  The music seems to emanate from\n\
the wagon.  As you draw near, a woman's\n\
voice weaves into the music, saying:\n\
\"You may approach, O seeker.\"",

/*0x32*//*D_2A32*/
"You enter to find an old gypsy sitting\n\
in a small curtained room.  She wears an\n\
Ankh around her neck. In front of her is\n\
a round table covered in deep green\n\
velvet.  The room smells so heavily of\n\
incense that you feel dizzy.",

/*0x33*//*D_2B13*/
"Seeing the Ankh, the ancient gypsy\n\
smiles and warns you never to part with\n\
it.  \"We have been waiting such a long\n\
time, but at last you have come.  Sit\n\
here and I shall read the path of your\n\
future.\"",

/*0x34*//*D_2BDB*/
"Upon the table she places a curious\n\
wooden object like an abacus but without\n\
beads.  In her hands she holds eight\n\
unusual cards.  \"Let us begin the\n\
casting.\"",

/*0x35*//*D_2C79*/"The gypsy places the first two cards\n",
/*0x36*//*D_2C9F*/"The gypsy places two more of the cards\n",
/*0x37*//*D_2CC7*/"The gypsy places the last two cards\n",
/*0x38*//*D_2CEC*/"upon the table.  They are the cards of\n",

/*0x39*//*D_2D14*/"Honesty",
/*0x3a*//*D_2D1C*/"Compassion",
/*0x3b*//*D_2D27*/"Valor",
/*0x3c*//*D_2D2D*/"Justice",
/*0x3d*//*D_2D35*/"Sacrifice",
/*0x3e*//*D_2D3F*/"Honor",
/*0x3f*//*D_2D45*/"Spirituality",
/*0x40*//*D_2D52*/"Humility",
/*0x41*//*D_2D5B*/"",

/*0x42*//*D_2D5C*/
"With the final choice, the incense\n\
swells up around you.  The gypsy speaks\n\
as if from a great distance, her voice\n\
growing fainter with each word:\n\
\"So be it!  Thy path is chosen!\"",

/*0x43*//*D_2E0F*/
"There is a moment of intense, wrenching\n\
vertigo.  As you open your eyes, a\n\
voice whispers within your mind, \"Seek\n\
the counsel of thy sovereign.\"  After a\n\
moment, the spinning subsides, and you\n\
open your eyes to..."
};

/*
 * Print a string at the text cursor; '\n' goes to the start of the next line.
 */
/*C_261D*/u4_puts(str)
char *str;
{
	char c;

	while(c = *str) {
		if(c == '\n') {
			txt_Y ++;
			txt_X = 0;
		} else {
			u4_putc(*str);
		}
		str ++;
	}
}

/*
 * Read a line of text into buf (at most size - 1 characters, printable ASCII only) until
 * Enter; Backspace, Ctrl+Backspace and Left erase. Leading and trailing spaces are then
 * removed (key is reused as an index for that).
 * Probable original bug: when the line is empty or only spaces, the final loop decrements
 * len past 0 and reads (and clears while it finds spaces) the bytes before buf.
 */
/*C_2656*/u4_gets(buf, size)
register char *buf;
uint16 size;
{
	register uint16 len;
	uint16 key;

	len = 0;
	do {
		key = u_kbread();
		switch(key) {
			case KBD_BS: case KBD_CTRL_BS: case KBD_LEFT:
				if(len == 0) {
					sound_1();
				} else {
					u4_putc(8);
					len--;
					buf[len] = ' ';
				}
			break;
			default:
				key &= 0x00ff;
				if(size - 1 == len || key < 0x20 || key >= 0x80) {
					sound_1();
				} else {
					buf[len] = key;
					u4_putc(key);
					len ++;
				}
			break;
			case KBD_ENTER:
				buf[len] = 0;
			break;
		}
	} while(key != KBD_ENTER);
	key = 0;
	while(buf[key++] == ' ');
	if(buf[--key]) {
		len = 0;
		while(buf[key])
			buf[len++] = buf[key++];
	} else {
		len = key;
	}
	do
		buf[len] = 0;
	while(buf[--len] == ' ');
}

/*
 * Clear the text area, print text at its top line and wait for a key.
 */
/*C_271D*/ShowTextPage(text)
char *text;
{
	Gra_ClearText();
	txt_X = 0;
	txt_Y = 19;
	u4_puts(text);
	u_kbread();
}

/*
 * Intro: the moongate rises from the ground in front of the tree (busy-wait delays).
 */
/*C_273E*/MoongateRises()
{
	int16 step, delay;

	for(step = 1; step <= 23; step++) {
		Gra_Blit(3,      step, 0, 152, pPicture, 92 - step, -1, 9);
		Gra_Blit(3, 24 - step, 9,  68, pPicture, 68,         -1, 9);
		for(delay = 1; delay <= 10000; delay ++);
	}
	Gra_Blit(3, 24, 0, 152, pPicture, 68, -1, 9);
}

/*
 * Intro: the moongate sinks back into the ground.
 */
/*C_27E0*/MoongateSinks()
{
	int16 step, delay;

	for(step = 1; step <= 23; step ++) {
		Gra_Blit(3,      step, 3, 152, pPicture,         68, -1, 9);
		Gra_Blit(3, 24 - step, 0, 152, pPicture, step + 68, -1, 9);
		for(delay = 1; delay <= 10000; delay ++);
	}
	Gra_Blit(3, 24, 3, 152, pPicture, 68, -1, 9);
}

/*
 * The story told before the gypsy's reading: text pages 0x1d to 0x34 (STR) under the
 * matching pictures (tree, moongate, portal, fair outside and inside, wagon, gypsy,
 * abacus), one page per key press. Pictures are loaded one page ahead and shown with
 * the page that needs them.
 */
/*C_2883*/Introduction()
{
	uint16 page;

	if((pPicture = _fmalloc((GraphicsType == 1)?0x4000:0x8000)) == 0)
		exit(0x5e);
	page = 0x1d;
	Gra_LoadPicture((GraphicsType == 1)?/*D_2F6C*/"tree.pic":/*D_2F75*/"tree.ega", pPicture);
	Gra_ClearScreen();
	Gra_Blit(40, 152, 0, 0, pPicture, 0, -1, 0);
	while(page != 0x35) {
		Gra_ClearText();
		txt_X = 0;
		txt_Y = 19;
		u4_puts(STR(page++));
		switch(page - 1) {
			case 0x20:
				MoongateRises();/*moongate animation #1*/
			break;
			case 0x22:
				MoongateSinks();/*moongate animation #2*/
				Gra_LoadPicture((GraphicsType == 1)?/*D_2F7E*/"portal.pic":/*D_2F89*/"portal.ega", pPicture);
			break;
			case 0x23:
				Gra_Blit(40, 152, 0, 0, pPicture, 0, -1, 0);
				Gra_LoadPicture((GraphicsType == 1)?/*D_2F94*/"tree.pic":/*D_2F9D*/"tree.ega", pPicture);
			break;
			case 0x28:
				Gra_Blit(40, 152, 0, 0, pPicture, 0, -1, 0);
				Gra_LoadPicture((GraphicsType == 1)?/*D_2FA6*/"outside.pic":/*D_2FB2*/"outside.ega", pPicture);
			break;
			case 0x2c:
				Gra_Blit(40, 152, 0, 0, pPicture, 0, -1, 0);
				Gra_LoadPicture((GraphicsType == 1)?/*D_2FBE*/"inside.pic":/*D_2FC9*/"inside.ega", pPicture);
			break;
			case 0x2e:
				Gra_Blit(40, 152, 0, 0, pPicture, 0, -1, 0);
				Gra_LoadPicture((GraphicsType == 1)?/*D_2FDE*/"wagon.pic":/*D_2FD4*/"wagon.ega", pPicture);
			break;
			case 0x31:
				Gra_Blit(40, 152, 0, 0, pPicture, 0, -1, 0);
				Gra_LoadPicture((GraphicsType == 1)?/*D_2FE8*/"gypsy.pic":/*D_2FF2*/"gypsy.ega", pPicture);
			break;
			case 0x32:
				Gra_Blit(40, 152, 0, 0, pPicture, 0, -1, 0);
				Gra_LoadPicture((GraphicsType == 1)?/*D_2FFC*/"abacus.pic":/*D_3007*/"abacus.ega", pPicture);
			break;
			case 0x34:
				Gra_Blit(40, 152, 0, 0, pPicture, 0, -1, 0);
			break;
		}
		u_kbflush();
		u_kbread();
	}
}

/*screen row of the abacus's bead rows, one per question*/
int16 /*D_3012*/AbacusRowY[] = {0x18,0x28,0x36,0x44,0x52,0x62,0x70};

/*
 * Draw a bead on the gypsy's abacus: in the column of the virtue, on the row of the
 * question, with a different bead picture for the chosen and the discarded virtue.
 */
/*C_2B2A*/DrawBead(discarded, virtue, row)
uint16 discarded;/*0: chosen virtue, 1: discarded virtue*/
uint16 virtue;/*virtue index 0..7*/
uint16 row;/*question index 0..6*/
{
	Gra_Blit(1, 12, (discarded == 0)?1:3, 187, pPicture, AbacusRowY[row], -1, virtue + 16);
}

char * /*D_307E*/CardPictures[] = {
	/*D_3020*/"HONCOM.EGA",
	/*D_302B*/"VALJUS.EGA",
	/*D_3036*/"SACHONOR.EGA",
	/*D_3043*/"SPIRHUM.EGA",

	/*D_304F*/"HONCOM.PIC",
	/*D_305A*/"VALJUS.PIC",
	/*D_3065*/"SACHONOR.PIC",
	/*D_3072*/"SPIRHUM.PIC"
};

/*
 * Draw the card of a virtue on the table, on the left (side 0) or right (side 1). Each
 * picture holds two cards side by side (HONCOM = Honesty and Compassion, ...), so the
 * card is the left or right half of picture virtue / 2; the CGA pictures come after
 * the EGA ones in CardPictures.
 */
/*C_2B6D*/DrawCard(virtue, side)
uint16 virtue;/*virtue index 0..7*/
uint16 side;/*0: left, 1: right*/
{
	void far *pic;

	virtue += ((GraphicsType == 1)?1:0) * 8;
	if((pic = _fmalloc((GraphicsType == 1)?0x4000:0x8000)) == 0)
		exit(0x5d);
	Gra_LoadPicture(CardPictures[virtue/2], pic);
	Gra_Blit(12, 124, ((virtue&1) == 0)?1:27, 12, pic, 12, -1, (side == 0)?1:27);
	_ffree(pic);
}

/*attribute bonus given by each virtue when chosen*/
unsigned char /*D_30B2*/VirtueStr[] = {0, 0, 3, 0, 1, 1, 1, 0};/*str*/
unsigned char /*D_30BA*/VirtueDex[] = {0, 3, 0, 1, 1, 0, 1, 0};/*dex*/
unsigned char /*D_30C2*/VirtueInt[] = {3, 0, 0, 1, 0, 1, 1, 0};/*int*/

/*text index of the dilemmas: STR(DilemmaBase[a] + b) is the dilemma between
 virtues a < b (the comment at the start of each line is the value)*/
unsigned char /*D_30CA*/DilemmaBase[] = {
	/*0x00*/0x01 - 1,/*Honesty vs {Compassion|...|Humility}*/
	/*0x06*/0x08 - 2,/*Compassion vs {Valor|...|Humility}*/
	/*0x0B*/0x0e - 3,/*Valor vs {Justice|...|Humility}*/
	/*0x0F*/0x13 - 4,/*Justice vs {Sacrifice|...|Humility}*/
	/*0x12*/0x17 - 5,/*Sacrifice vs {Honor|...|Humility}*/
	/*0x14*/0x1a - 6,/*Honor vs {Spirituality|Humility}*/
	/*0x15*/0x1c - 7 /*Spirituality vs Humility*/
};

/*
 * The gypsy's reading: 7 dilemmas between two virtues, a tournament. The 8 virtues are
 * paired at random for 4 questions, then the 4 winners for 2 questions, then the last 2.
 * flags[virtue]: 0 still in this round, 1 already chosen in this round, 0xff discarded.
 * chosen/discarded: the two virtues drawn, the lower one first (it is answer A); they are
 * swapped if B is answered. Each chosen virtue gets 5 karma, its attribute bonus and a
 * bead; the last one chosen (lastVirtue) decides the class. n is a loop index, a swap
 * temporary and the key read.
 */
/*C_2C12*/GypsyReading()
{
	int16 n;
	unsigned char chosen, discarded;
	unsigned char flags[8];

	tmp_str = tmp_int = tmp_dex = 15;
	curQuestionIndex = 0;
	for(n = 7; n >= 0; n--) {
		flags[n] = 0;
		tmp_karma[n] = 50;
	}
	do {
		if(curQuestionIndex == 4 || curQuestionIndex == 6) {
			for(n = 7; n >= 0; n --)
				if(flags[n] < 0x80)
					flags[n] = 0;
		}
		Gra_ClearText();
		txt_X = 0;
		txt_Y = 19;
		do
			chosen = u_rand_a() & 7;
		while(flags[chosen&0xff]);
		do
			discarded = u_rand_a() & 7;
		while(flags[discarded&0xff] || discarded == chosen);
		if(discarded < chosen) {
			/*swap(chosen,discarded);*/
			n = chosen; chosen = discarded; discarded = n;
		}
		if(curQuestionIndex == 0)
			u4_puts(/*D_2F4E*/STR(0x35));
		else if(curQuestionIndex == 6)
			u4_puts(/*D_2F52*/STR(0x37));
		else
			u4_puts(/*D_2F50*/STR(0x36));
		u4_puts(/*D_2F54*/STR(0x38));
		DrawCard(chosen, 0);
		u4_puts(STR(0x39 + chosen));
		u4_puts(/*D_308E*/" and ");
		DrawCard(discarded, 1);
		u4_puts(STR(0x39 + discarded));
		u4_puts(/*D_3094*/".  She says\n\"Consider this:\"");
		u_kbflush();
		u_kbread();
		Gra_ClearText();
		txt_X = 0;
		txt_Y = 19;
		u4_puts(STR(DilemmaBase[chosen] + discarded));
		do {
			n = u_kbread();
			if(u4_isupper((unsigned char)n))
				n += 'a' - 'A';
		} while(n != KBD_A && n != KBD_B);
		if(n == KBD_B) {
			/*swap(chosen,discarded);*/
			n = chosen; chosen = discarded; discarded = n;
		}
		/*chosen virtue*/
		lastVirtue = chosen;
		flags[lastVirtue] = 1;
		tmp_karma[lastVirtue] += 5;
		tmp_str += VirtueStr[lastVirtue];
		tmp_dex += VirtueDex[lastVirtue];
		tmp_int += VirtueInt[lastVirtue];
		DrawBead(0, lastVirtue, curQuestionIndex);
		/*discarded virtue*/
		flags[discarded] = 0xff;
		DrawBead(1, discarded, curQuestionIndex);
	} while(++curQuestionIndex != 7);
	ShowTextPage(/*D_2F68*/STR(0x42));
	ShowTextPage(/*D_2F6A*/STR(0x43));
}

/*start position on the world map for each virtue (by the town of that virtue)*/
unsigned char /*D_30DC*/StartX[] = {0xE7,0x53,0x23,0x3B,0x9E,0x69,0x17,0xBA};
unsigned char /*D_30E4*/StartY[] = {0x88,0x69,0xDD,0x2C,0x15,0xB7,0x81,0xAB};

uint16 /*D_30EC*/*pKarmas[] = {
	&(Party._hones),
	&(Party._compa),
	&(Party._valor),
	&(Party._justi),
	&(Party._sacri),
	&(Party._honor),
	&(Party._spiri),
	&(Party._humil)
};

/*
 * Build the new party from PARTY.NEW: the start position and karmas, the character of
 * the chosen class (lastVirtue) swapped into the first slot and given the player's name,
 * attributes and sex; one member only, and the NPC table of the world map is cleared.
 */
/*C_2E04*/CreateParty()
{
	int16 idx;
	struct tChara temp;

	if(Load(/*D_30D1*/"PARTY.NEW", sizeof(struct tParty), &Party) == -1)
		exit(3);
	Party._x = StartX[lastVirtue];
	Party._y = StartY[lastVirtue];
	Party._members = 1;
	for(idx = 7; idx >= 0; idx --)
		*(pKarmas[idx]) = tmp_karma[idx];
	/*-- swap(&(Party.chara[lastVirtue]), &(Party.chara[0])) --*/
	memcpy(&temp, &(Party.chara[lastVirtue]), sizeof(struct tChara));
	memcpy(&(Party.chara[lastVirtue]), &(Party.chara[0]), sizeof(struct tChara));
	memcpy(&(Party.chara[0]), &temp, sizeof(struct tChara));
	/*-- --*/
	strcpy(Party.chara[0]._name, player_name);
	Party.chara[0]._str = tmp_str;
	Party.chara[0]._int = tmp_int;
	Party.chara[0]._dex = tmp_dex;
	Party.chara[0]._sex = (M_or_F == 'M')?0x0b:0x0c;
	for(idx = 31; idx >= 0; idx --) {
		Area._npc._gtile[idx] =
		Area._npc._x[idx] =
		Area._npc._y[idx] =
		Area._npc._tile[idx] =
		Area._npc._old_x[idx] =
		Area._npc._old_y[idx] =
		Area._npc._var[idx] =
		Area._npc._tlkidx[idx] = 0;
	}
}

/*
 * Ask for a disk: show prompt, "and", "press drive letter", then wait until a drive
 * letter is pressed that selects a drive holding the file fname (B: only with two
 * drives). Beeps and waits again otherwise.
 * Probable original bug: 'P' gives drive 0 ('P' & 0xf), which never matches.
 */
/*C_2F07*/AskForDisk(prompt, fname)/*%%% hacked %%%*/
char *prompt;
char *fname;
{
	int16 key;

	Gra_ClearScreen();
	txt_Y = 10;
	txt_X = (40 - (uint16)strlen(prompt)) / 2;
	u4_puts(prompt);
	txt_Y ++;
	txt_X = 18;
	u4_puts(/*D_30FC*/"and");
	txt_Y ++;
	txt_X = 11;
	u4_puts(/*D_3100*/"press drive letter");
	while(!u_kbhit());
	do {
		key = u_kbread() & 0xff;
		u4_toupper(key);
		if(key != 'B' || TwoDrives != 0) {
			if(key >= 'A' && key <= 'P') {
				key &= 0xf;
				u_setdrive(key);
			}
		}
		if((u_getdrive() & 0xff) == key && u_fexist(fname))
			return;
		sound_1();
		while(!u_kbhit());
	} while(1);
}

/*
 * Ask for the PLAYER disk (the one holding WORLD.MAP) and write PARTY.SAV and
 * MONSTERS.SAV on it. If it went into the program's drive, ask for the PROGRAM disk
 * again (AVATAR.EXE), otherwise go back to the program's drive.
 */
/*C_2FB8*/WritePlayerDisk()
{
	uint16 programDrive;

	programDrive = u_getdrive() & 0xff;
	AskForDisk(/*D_311D*/"Insert PLAYER disk", /*D_3113*/"WORLD.MAP");
	PlayerDrive = u_getdrive() & 0xff;
	if(Save(/*D_3130*/"PARTY.SAV", sizeof(struct tParty), &Party) == -1)
		exit(3);
	if(Save(/*D_313A*/"MONSTERS.SAV", sizeof(struct tNPC), &(Area._npc)) == -1)
		exit(3);
	if(PlayerDrive == programDrive)
		AskForDisk(/*D_3152*/"Re-Insert PROGRAM disk", /*D_3147*/"AVATAR.EXE");
	else
		u_setdrive(programDrive);
}

extern /*C_0B1E*/PutsAt(int16, int16, char *);

/*
 * "Initiate New Game": ask the name and the sex, then run the introduction, the gypsy's
 * reading, create the party and write it on the PLAYER disk, and exit to DOS with the
 * player drive's letter as the exit code. An empty name or Escape returns to the title
 * screen instead (the caller redraws it, the animations having been freed).
 * Probable original bug: M_or_F holds only the character code (& 0xff), so the
 * KBD_ENTER and KBD_SPACE (scan code + character) tests never match.
 */
/*C_3030*/NewGame()
{
	Gra_ClearMenu();
	PutsAt(16, 4, /*D_3169*/"By what name shalt thou be known");
	PutsAt(17, 4, /*D_318A*/"in this world and time?");
	txt_X = 12;
	txt_Y = 19;
	u4_gets(player_name, 12);
	if(player_name[0] == 0) {
		_ffree(pAnim);
		_ffree(pShapes);
		return;
	}
	Gra_ClearMenu();
	PutsAt(17, 4, /*D_31A2*/"Art thou Male or Female? ");
	M_or_F = u_kbread() & 0xff;
	u4_toupper(M_or_F);
	while(M_or_F != 'M' && M_or_F != 'F') {
		sound_1();
		M_or_F = u_kbread() & 0xff;
		if(M_or_F == 0x1b || M_or_F == KBD_ENTER || M_or_F == KBD_SPACE) {
			_ffree(pAnim);
			_ffree(pShapes);
			return;
		}
		u4_toupper(M_or_F);
	}
	u4_putc(M_or_F);
	_ffree(pAnim);
	_ffree(pShapes);
	MonstersShown = 0;

	Introduction();
	GypsyReading();
	CreateParty();
	WritePlayerDisk();
	_ffree(pPicture);
	/*-- clean & return to dos --*/
	_ffree(pCharset);
	low_clean();
	exit('A' + PlayerDrive - 1);
}
