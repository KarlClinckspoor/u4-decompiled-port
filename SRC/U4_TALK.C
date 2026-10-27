/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>

typedef tHandler_tlk();
typedef tHandler_tlk *pHandler_tlk;

TLK_name();
TLK_look();
TLK_job();
TLK_health();
TLK_special1();
TLK_special2();
TLK_join();
TLK_give();

char /*D_2A7A*/MsgNoResponse[] = "Funny, no response!\n";

/*the topics townsfolk answer to (only the first 4 letters count): the word and its
  handler ("bye" has none, "" ends the list). Entries 5 and 6 get the citizen's own
  two keywords (TalkStrings[10] and [11]) in TalkToCitizen.*/
struct {
	char *_00;
	pHandler_tlk _02;
} /*D_2A90*/TalkKeywords[] = {
	{/*D_2A3E*/"bye",    0},
	{/*D_2A42*/"name",   TLK_name},
	{/*D_2A47*/"look",   TLK_look},
	{/*D_2A4C*/"job",    TLK_job},
	{/*D_2A50*/"health", TLK_health},
	{0,                  TLK_special1},
	{0,                  TLK_special2},
	{/*D_2A57*/"join",   TLK_join},
	{/*D_2A5C*/"give",   TLK_give},
	{/*D_2A61*/"",       0}
};

/*the strings of the current citizen's .TLK record, from TLK_Parse:
	0 name
	1 pronoun
	2 description ("look")
	3 job
	4 health
	5 answer to the personal keyword 1
	6 answer to the personal keyword 2
	7 yes/no question
	8 answer to Yes
	9 answer to No
	10 personal keyword 1
	11 personal keyword 2
  The record starts with 3 bytes: TalkData[0] = index in TalkKeywords of the topic
  after which the yes/no question is asked, TalkData[1] = nonzero if the question
  tests humility, TalkData[2] = chance (out of 256) that the citizen turns away.*/
static char * D_8CCE[12];
#define TalkStrings D_8CCE/*see the note on statics below*/

/*
 * Ask the citizen's yes/no question and print the answer. For a humility question,
 * Yes (bragging) costs 5 humility and No gives 10 (at most once every 16 moves).
 */
/*C_A163*/TLK_Question()
{
	char answer[4];

	Gra_CR();
	u_kbread();
	u4_puts(TalkStrings[7]);
	u4_puts(/*D_2A62*/"\n\nYou say: ");
	do {
		u4_gets(answer, 4);
		Gra_CR();
		if(answer[0] == 0)
			break;
		u4_toupper2(answer[0]);
		if(answer[0] != 'N' && answer[0] != 'Y')
			u4_puts(/*D_2A6E*/"Yes or no!\n");
	} while(answer[0] != 'N' && answer[0] != 'Y');
	if(answer[0] == 0)
		return;
	if(answer[0] == 'Y') {
		if(TalkData[1])
			karma_dec(&(Party._humil), 5);
		u4_puts(TalkStrings[8]);
	} else {
		if(TalkData[1] && (Party._moves >> 4) != Party._lastTalk)
			karma_inc(&(Party._humil), 10);
		u4_puts(TalkStrings[9]);
	}
	Party._lastTalk = (Party._moves >> 4);
	Gra_CR();
}

/*
 * Print text (if not null) and a new line.
 * The original declares only text but most calls pass an extra number first (3, 4,
 * 5, 6), which the code keeps: with the pascal convention the last argument is the
 * one nearest to the frame, so the function still reads text; the extra word stays
 * on the stack until the caller's epilogue resets SP. The WIN32 build declares the extra parameter.
 */
#ifdef WIN32
/*C_A22D*/TLK_Say(unused, text)
int16 unused;
char *text;
#else
/*C_A22D*/TLK_Say(text)
char *text;
#endif
{
	if(!text)
		return;
	u4_puts(text);
	Gra_CR();
}

/*the citizen's first personal keyword*/
/*C_A245*/TLK_special1()
{
#ifdef WIN32
	TLK_Say(6, TalkStrings[5]);
#else
	TLK_Say(TalkStrings[5]);
#endif
}

/*the citizen's second personal keyword*/
/*C_A253*/TLK_special2()
{
#ifdef WIN32
	TLK_Say(7, TalkStrings[6]);
#else
	TLK_Say(TalkStrings[6]);
#endif
}

/*"name"*/
/*C_A261*/TLK_name()
{
	u4_puts(TalkStrings[1]);
	u4_puts(/*D_2AB8*/" says: I am ");
	u4_puts(TalkStrings[0]);
	Gra_CR();
}

/*"look"*/
/*C_A280*/TLK_look()
{
	u4_puts(/*D_2AC5*/"You see ");
	TLK_Say(3, TalkStrings[2]);
}

/*"job"*/
/*C_A299*/TLK_job()
{
	TLK_Say(4, TalkStrings[3]);
}

/*"health"*/
/*C_A2AB*/TLK_health()
{
	TLK_Say(5, TalkStrings[4]);
}

/*for "Thou art not ... enough", indexed like the towns (= the virtues)*/
char * /*D_2BB2*/VirtueAdjectives[] = {
	/*D_2ACE*/"honest",
	/*D_2AD5*/"compassionate",
	/*D_2AE3*/"valiant",
	/*D_2AEB*/"just",
	/*D_2AF0*/"sacrificial",
	/*D_2AFC*/"honorable",
	/*D_2B06*/"spiritual",
	/*D_2B10*/"humble"
};

static uint16 D_8CE6;
/*The statics keep their address-like names: Microsoft C 4.00 lays out a module's
 uninitialised statics in the order of its symbol table, which depends on the names.
 Renaming them would move them; the readable names are macros.*/
/*index in Area._npc of the citizen being talked to, for the TLK_ handlers*/
#define TalkNpc D_8CE6

/*
 * "join": only the companion of each of the 8 virtue towns (talk record 1) can join,
 * and not in the town of the Avatar's own class (that class is the Avatar's). Needs
 * the town's virtue at 40 or more (or 0 = partial Avatar) and max hit points of at
 * least 100 per member after joining. NPC #31 is removed from the town map (not
 * TalkNpc: the companion is presumably always the last NPC of its town) and the
 * companion's character, found in the roster by its class, is moved to the first
 * free party slot.
 */
/*C_A2BD*/TLK_join()
{
	int16 chara;

	if(
		(uint16)Area._npc._tlkidx[TalkNpc] != 1 ||
		(Party._loc - LOC_TOWNS) >= 8 ||
		(Party._loc - LOC_TOWNS) == Party.chara[0]._class
	) {
		u4_puts(TalkStrings[1]);
		u4_puts(/*D_2B17*/" says: I cannot join thee.\n");
		return;
	}
	if(*pKarmas[Party._loc - LOC_TOWNS] < 40 && *pKarmas[Party._loc - LOC_TOWNS] != 0) {
		u4_puts(/*D_2B33*/"Thou art not ");
		u4_puts(VirtueAdjectives[Party._loc - LOC_TOWNS]);
		u4_puts(/*D_2B41*/" enough for me to join thee.\n");
		return;
	}
	if(100 * Party._members + 100 > Party.chara[0]._HP[1]) {
		u4_puts(/*D_2B5F*/"Thou art not experienced enough for me to join thee.\n");
		return;
	}
	u4_puts(/*D_2B95*/"I am honored to join thee!\n");
	Area._npc._tile[31] =
	Area._npc._gtile[31] =
	Area._npc._var[31] =
	Area._npc._tlkidx[31] = 0;
	RefreshView();
	for(chara = 7; chara >= 0; chara --) {
		if(Party.chara[chara]._class == (Party._loc - LOC_TOWNS))
			break;
	}
	if(chara != -1)
		SwapCharas(&(Party.chara[Party._members]), &(Party.chara[chara]));
	Party._members ++;
	Gra_ClearStatus();
	dspl_Stats();
}

/*
 * "give": only beggars accept gold; giving some gives 2 compassion (at most once
 * every 16 moves). The final TLK_Say(6, 0) prints nothing.
 */
/*C_A3A2*/TLK_give()
{
	int16 amount;

	if(Area._npc._tile[TalkNpc] != TIL_BEGGAR) {
		u4_puts(TalkStrings[1]);
		u4_puts(/*D_2BC2*/" says: I do not need thy gold.  Keep it!\n");
		return;
	}
	u4_puts(/*D_2BEC*/"How much?\x12\x12\x12\b\b");
	if((amount = AskInt(2)) > 0) {
		if(Party._gold < amount) {
			u4_puts(/*D_2BFB*/"Thou hast not that much gold!\n");
		} else {
			Party._gold -= amount;
			dspl_Stats();
			u4_puts(TalkStrings[1]);
			u4_puts(/*D_2C1A*/" says: Oh, Thank thee! I shall never forget thy kindness!\n");
			if((Party._moves >> 4) != Party._lastTalk)
				karma_inc(&(Party._compa), 2);
			Party._lastTalk = Party._moves >> 4;
		}
	}
	TLK_Say(6, 0);
}

/*
 * Point TalkStrings at the 12 consecutive zero-terminated strings that start at text
 * (a .TLK record after its 3 bytes), and lower the case of the description's first
 * letter.
 */
/*C_A443*/TLK_Parse(text)
register char *text;
{
	register char **str;

	for(str = &(TalkStrings[0]); str != &(TalkStrings[12]); ) {
		*(str++) = text;
		while(*(text++));
	}
	u4_tolower(TalkStrings[2][0]);
}

/*
 * Remove the trailing spaces of string str.
 */
/*C_A47F*/TrimRight(str)
char *str;
{
	char *last;

	for(last = strchr(str, 0) - 1; last + 1 != str && *last == ' '; )
		*(last--) = 0;
}

/*
 * Conversation with townsperson #npc: load its .TLK record, describe it, then answer
 * topics until "bye", an empty line, or the citizen joins the party. Before each
 * topic the citizen may turn away (TalkData[2]); if very unwilling, he attacks.
 */
/*C_A4B4*/TalkToCitizen(npc)
int16 npc;
{
	int16 done, members;

	dlseek(File_TLK, (Area._npc._tlkidx[npc] - 1) * 0x120);
	dread(File_TLK, TalkData, 0x120);
	members = Party._members;
	CombatFoe = Area._npc._tile[npc];
	TLK_Parse(TalkData+3);
	/*the personal keywords 1 & 2*/
	TalkKeywords[5]._00 = TalkStrings[10];
	TalkKeywords[6]._00 = TalkStrings[11];
	TrimRight(TalkKeywords[5]._00);
	TrimRight(TalkKeywords[6]._00);
	/* */
	u4_puts(/*D_2C55*/"\nYou meet ");
	TLK_Say(3, TalkStrings[2]);
	/*randomly says his name*/
	if(U4_RND1(1)) {
		Gra_CR();
		u4_puts(TalkStrings[1]);
		u4_puts(/*D_2C60*/" says: I am ");
		u4_puts(TalkStrings[0]);
		Gra_CR();
	}
	done = 0;
	do {
		register int16 kw;
		char interest[12];

		u4_puts(/*D_2C6D*/"\nYour Interest:\n");
		u4_gets(interest, 11);
		Gra_CR();
		if(interest[0] == 0)
			break;
		/*kw is first a random number*/
		if((kw = u_rand_a()) < TalkData[2]) {
			if(TalkData[2] - kw >= 0x40) {
				/*he/she gets upset: says the name unless it starts with
				  "a " or "the ", and becomes hostile*/
				if(strnicmp(TalkStrings[0], /*D_2C7E*/"a ", 2) && strnicmp(TalkStrings[0], /*D_2C81*/"the ", 4))
					u4_puts(TalkStrings[0]);
				else
					u4_puts(TalkStrings[1]);
				u4_puts(/*D_2C86*/" says: On guard! Fool!\n");
				Area._npc._var[npc] = 0xff;
			} else {
				u4_puts(TalkStrings[1]);
				u4_puts(/*D_2C9E*/" turns away!\n\n");
			}
			return;
		}
		for(kw = 0; TalkKeywords[kw]._00[0]; kw++) {
			if(strnicmp(TalkKeywords[kw]._00, interest, 4) == 0) {
				if(TalkKeywords[kw]._02) {
					Gra_CR();
					TalkNpc = npc;
					(*(TalkKeywords[kw]._02))();
					if(Party._members != members)
						return;/*looks like he/she has joined*/
				} else {
					done = 1;
				}
				if(!done) {
					if(TalkData[0] == kw)
						TLK_Question();
				}
				break;
			}
		}
		if(TalkKeywords[kw]._00[0] == 0)
			u4_puts(/*D_2CAD*/"That I cannot help thee with.\n");
	} while(done == 0);
	u4_puts(/*D_2CCC*/"\nBye.\n");
}

/*for each location (Party._loc - 1) and shop (index in ShopHandlers), the row (y)
  of the shop's counter sign; 0 = none*/
unsigned char /*D_2CD4*/ShopRows[][8] = {
	{0x00,0x00,0x00,0x00,0x00,0x1A,0x00,0x00},/*LB*/

	{0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00},/*lycaeum*/
	{0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00},/*empath*/
	{0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00},/*serpent*/

	{0x00,0x00,0x0E,0x00,0x1A,0x1B,0x02,0x00},/*MOONGLOW*/
	{0x03,0x07,0x06,0x02,0x00,0x1D,0x0C,0x00},/*BRITAIN*/
	{0x09,0x05,0x00,0x13,0x00,0x06,0x1A,0x00},/*JHELOM*/
	{0x00,0x00,0x18,0x00,0x00,0x19,0x00,0x00},/*YEW.ULT*/
	{0x1C,0x00,0x00,0x00,0x00,0x00,0x03,0x00},/*MINOC*/
	{0x14,0x18,0x00,0x02,0x00,0x00,0x03,0x00},/*TRINSIC*/
	{0x00,0x00,0x11,0x00,0x04,0x1B,0x0D,0x00},/*SKARA*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*MAGINCIA*/

	{0x00,0x04,0x1A,0x05,0x07,0x00,0x00,0x00},/*paws*/
	{0x0B,0x11,0x00,0x19,0x08,0x00,0x00,0x07},/*den*/
	{0x14,0x00,0x00,0x16,0x00,0x00,0x15,0x1A},/*vesper*/
	{0x00,0x00,0x00,0x00,0x00,0x1A,0x00,0x00} /*cove*/
};

extern SHP_weapon();
extern SHP_armour();
extern SHP_food();
extern SHP_pub();
extern SHP_reagent();
extern SHP_healer();
extern SHP_inn();
extern SHP_guild();
extern SHP_horse();
extern SHP_hawkwind();

/*the shops; 8 and 9 have no row in ShopRows (see EnterShop)*/
pHandler_tlk /*D_2D54*/ShopHandlers[] = {
	SHP_weapon,
	SHP_armour,
	SHP_food,
	SHP_pub,
	SHP_reagent,
	SHP_healer,
	SHP_inn,
	SHP_guild,

	SHP_horse,
	SHP_hawkwind
};

/*
 * Enter the shop whose counter sign is on row y of the current town (the column
 * is not checked). The Paws horse seller and Hawkwind in Lord British's castle are
 * special cases.
 */
/*C_A686*/EnterShop(y)
unsigned char y;
{
	register int16 shop;

	for(shop = 7; shop >= 0;shop--) {
		if(ShopRows[Party._loc - 1][shop] == y)
			break;
	}
	if(shop == -1 && y == 0x18 && Party._loc == LOC_PAWS)
		shop = 8;/*patch! paws' horses*/
	if(shop == -1 && y == 0x19 && Party._loc == LOC_LCB)
		shop = 9;/*patch! LB's hawkwind*/
	if(shop == -1) {
		u4_puts(MsgNoResponse);
		return;
	}
	(*(ShopHandlers[shop]))();
}

/*
 * (T)alk in a direction (h, v): to the person there, or across a counter (a letter
 * tile) to a merchant, which enters the shop of that row; Lord British has his own
 * conversation.
 */
/*C_A6F3*/CMD_Talk() {
	unsigned char y, x;
	int16 h, npcAt, v;

	u4_puts(/*D_2D68*/"Talk\n");
	if(Party._fly_light != 0) {
		w_DriftOnly();
		return;
	}
	AskDir(/*D_2D6E*/"Dir: ", &h, &v);
	if(!(h | v))
		return;
	x = Party._x + h;
	y = Party._y + v;
	npcAt = FindLocalNpcAt(x, y);
	if(x > 31 || y > 31 || Party._loc == 0) {
		u4_puts(MsgNoResponse);
		return;
	}
	if(
		Area._map.x32x32[y][x] >= TIL_LETTER_A &&
		Area._map.x32x32[y][x] <= TIL_BLACK
	) {/*a letter (counter sign): the merchant is behind it*/
		x += h;
		y += v;
		if(x > 31 || y > 31) {
			u4_puts(MsgNoResponse);
			return;
		}
		if((npcAt = FindLocalNpcAt(x, y)) == -1 || Area._npc._tile[npcAt] != TIL_MERCHANT) {
			u4_puts(MsgNoResponse);
			return;
		}
		EnterShop(y - v);
		return;
	}
	if(npcAt == -1 || Area._npc._tlkidx[npcAt] == 0) {
		u4_puts(MsgNoResponse);
		return;
	}
	if(Area._npc._tile[npcAt] == TIL_LORD_BRITISH) {
		TalkToLB();
		return;
	}
	TalkToCitizen(npcAt);
}
