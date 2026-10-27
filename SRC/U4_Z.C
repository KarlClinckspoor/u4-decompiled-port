/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>

/*Blank the status area (columns 24-38) from the current line down to line 8.*/
int16 /*C_45B5*/ZtatsClearRest()
{
	for( ;txt_Y <= 8; txt_Y ++) {
		txt_X = 24;
		u4_puts(/*D_18C6*/"               ");
	}
}

/*
 * Print str centered in the status area (plus shift columns) on line txt_Y;
 * on the top line (the frame), between the two title brackets 0x10 and 0x11.
 */
int16 /*C_45D6*/ZtatsTitle(str, shift)
register char *str;
uint16 shift;
{
	register int16 len;

	len = strlen(str);
	if(len > 12)
		len = 12;
	len -= (len & 1);

	txt_X = (len>=8) + ((12 - len)  / 2) + shift + 25;
	if(txt_Y == 0) {
		txt_X --;
		u4_putc(0x10);
	}
	u4_puts(str);
	if(txt_Y == 0)
		u4_putc(0x11);
}

/*Redraw the frame over the status area's title (top line); the cursor is preserved.*/
int16 /*C_4649*/ZtatsClearTitle()
{
	uint16 orig_y, orig_x;

	orig_y = txt_Y;
	orig_x = txt_X;
	txt_Y = 0;
	txt_X = 24;
	do
		Gra_PutChar('\r');
	while(++txt_X < 39);
	txt_Y = orig_y;
	txt_X = orig_x;
}

/*
 * Ztats page of party member #member: name, sex, class, status, magic points,
 * level (max HP / 100), attributes, hit points, experience, weapon and armour.
 */
int16 /*C_4689*/ZtatsChara(member)
int16 member;
{
	register struct tChara *chara;

	chara = &(Party.chara[member]);
	txt_Y = 0; ZtatsTitle(chara->_name, 0);
	txt_X = 24;
	txt_Y = 1; u4_putc(chara->_sex);
	txt_Y = 1; ZtatsTitle(Names[NAME_CLASSES + chara->_class], 0);
	txt_X = 38; u4_putc(chara->_stat);
	txt_X = 25;
	txt_Y = 3; u4_puts(/*D_18D6*/"MP:"); u4_putl(chara->_MP, 2, '0');
	txt_X = 32; u4_puts(/*D_18DA*/"LV:"); u4_putc((chara->_HP[1]/100)+'0');
	txt_X = 24;
	txt_Y = 4; u4_puts(/*D_18DE*/"STR:"); u4_putl(chara->_str, 2, '0');
	txt_X = 32; u4_puts(/*D_18E3*/"HP:"); u4_putl(chara->_HP[0], 4, '0');
	txt_X = 24;
	txt_Y = 5; u4_puts(/*D_18E7*/"DEX:"); u4_putl(chara->_dex, 2, '0');
	txt_X = 32; u4_puts(/*D_18EC*/"HM:"); u4_putl(chara->_HP[1], 4, '0');
	txt_X = 24;
	txt_Y = 6; u4_puts(/*D_18F0*/"INT:"); u4_putl(chara->_int, 2, '0');
	txt_X = 32; u4_puts(/*D_18F5*/"EX:"); u4_putl(chara->_XP, 4, '0');
	txt_Y = 7;
	txt_X = 24; u4_puts(/*D_18F9*/"W:"); u4_puts(Names[NAME_WEAPONS + chara->_weapon]);
	txt_Y = 8;
	txt_X = 24; u4_puts(/*D_18FC*/"A:"); u4_puts(Names[NAME_ARMOURS + chara->_armor]);
}

/*
 * Ztats page of the weapons owned (hands always shown): letter, count and
 * code, in columns of 8 lines.
 */
int16 /*C_4832*/ZtatsWeapons()
{
	register int16 weapon;

	txt_Y = 0;
	ZtatsTitle(/*D_18FF*/"Weapons", 0);
	txt_Y = 1;
	txt_X = 24;
	u4_puts(/*D_1907*/"A-Hands");
	txt_Y = 2;
	txt_X = 24;
	weapon = 1;
	do {
		if(Party._weapons[weapon]) {
			u4_putc(weapon+'A');
			u4_putl(Party._weapons[weapon], 2, '-');
			u4_putc('-');
			u4_puts(Names[NAME_WEAPON_CODES + weapon]);
			txt_X = (txt_X - 1) & ~7;
			if(++txt_Y == 9) {
				txt_Y = 1;
				txt_X += 8;
			}
		}
	} while(++weapon < 0x10);
	while(txt_X < 40) {
		u4_puts(/*D_190F*/"       ");
		txt_X = (txt_X - 1) & ~7;
		if(++txt_Y == 9) {
			txt_Y = 1;
			txt_X += 8;
		}
	}
}

/*Ztats page of the armours owned (no armour always shown).*/
int16 /*C_48F8*/ZtatsArmour()
{
	register int16 armour;

	txt_Y = 0;
	ZtatsTitle(/*D_1917*/"Armour", 0);
	txt_Y = 1;
	txt_X = 24;
	u4_puts(/*D_191E*/"A  -No Armour");
	txt_Y = 2;
	for(armour = 1; armour < 8; armour ++) {
		if(Party._armors[armour]) {
			txt_X = 24;
			u4_putc(armour + 'A');
			u4_putl(Party._armors[armour], 2, '-');
			u4_putc('-');
			u4_puts(Names[NAME_ARMOURS + armour]);
			while(txt_X != 39)
				u4_putc(' ');
			txt_Y ++;
		}
	}
	ZtatsClearRest();
}

/*Ztats page of the torches, gems, keys and sextants (when any).*/
int16 /*C_4987*/ZtatsEquipment()
{
	txt_Y = 0;
	ZtatsTitle(/*D_192C*/"Equipment", -1);

	txt_X = 24; txt_Y = 1;
	u4_putl(Party._torches, 2, ' '); u4_puts(/*D_1936*/" Torches");

	txt_X = 24; txt_Y ++;
	u4_putl(Party._gems, 2, ' '); u4_puts(/*D_193F*/" Gems");

	txt_X = 24; txt_Y ++;
	u4_putl(Party._keys, 2, ' '); u4_puts(/*D_1945*/" Keys");

	if(Party._sextants) {
		txt_X = 24; txt_Y = 4;
		u4_putl(Party._sextants, 2, ' '); u4_puts(/*D_194B*/" Sextants");
	}
	txt_Y ++;
	ZtatsClearRest();
}

/*stones: Blue Yellow Red Green Orange Purple White Black*/
char /*D_199A*/StoneLetters[] = "BYRGOPWB";
/*runes: Honesty Compassion Valor Justice Sacrifice Honor Spirituality Humility*/
char /*D_19A4*/RuneLetters[] = "HCVJSHSH";

/*
 * Ztats page of the quest items: stones, runes, bell/book/candle ("Candl"
 * when all three, to fit), the three part key, horn, wheel and skull.
 */
int16 /*C_4A3D*/ZtatsItems()
{
	register int16 bit;

	txt_Y = 0;
	ZtatsTitle(/*D_1955*/"Items", 0);
	txt_Y = 1;
	if(Party.mStones) {
		txt_X = 24;
		u4_puts(/*D_195B*/"Stones:");
		bit = 0;
		do {
			if(TST_MSK(Party.mStones, bit))
				u4_putc(StoneLetters[bit]);
		} while(++bit < 8);
		txt_Y ++;
	}
	if(Party.mRunes) {
		txt_X = 24;
		u4_puts(/*D_1963*/"Runes:");
		bit = 0;
		do {
			if(TST_MSK(Party.mRunes, bit))
				u4_putc(RuneLetters[bit]);
		} while(++bit < 8);
		txt_Y ++;
	}
	if(TST_MSK(Party.mItems, ST_BELL) || TST_MSK(Party.mItems, ST_BOOK) || TST_MSK(Party.mItems, ST_CANDLE)) {
		txt_X = 24;
		if(TST_MSK(Party.mItems, ST_BELL))
			u4_puts(/*D_196A*/"Bell ");
		if(TST_MSK(Party.mItems, ST_BOOK))
			u4_puts(/*D_1970*/"Book ");
		if(TST_MSK(Party.mItems, ST_CANDLE)) {
			u4_puts(/*D_1976*/"Candl");
			if(!TST_MSK(Party.mItems, ST_BOOK) || !TST_MSK(Party.mItems, ST_BELL))
				u4_putc('e');
		}
		txt_Y ++;
	}
	if(TST_MSK(Party.mItems, ST_KEY_C) || TST_MSK(Party.mItems, ST_KEY_L) || TST_MSK(Party.mItems, ST_KEY_T)) {
		txt_X = 24;
		u4_puts(/*D_197C*/"3 Part Key:");
		if(TST_MSK(Party.mItems, ST_KEY_T))
			u4_putc('T');
		if(TST_MSK(Party.mItems, ST_KEY_L))
			u4_putc('L');
		if(TST_MSK(Party.mItems, ST_KEY_C))
			u4_putc('C');
		txt_Y ++;
	}
	if(TST_MSK(Party.mItems, ST_HORN)) {
		txt_X = 24;
		u4_puts(/*D_1988*/"Horn");
		txt_Y ++;
	}
	if(TST_MSK(Party.mItems, ST_WHEEL)) {
		txt_X = 24;
		u4_puts(/*D_198D*/"Wheel");
		txt_Y ++;
	}
	if(TST_MSK(Party.mItems, ST_SKULL)) {
		txt_X = 24;
		u4_puts(/*D_1993*/"Skull");
	}
}

/*Ztats page of the reagents owned.*/
int16 /*C_4BC7*/ZtatsReagents()
{
	register int16 reagent;

	txt_Y = 0;
	ZtatsTitle(/*D_19AD*/"Reagents", 0);
	txt_Y = 1;
	reagent = 0;
	do {
		if(Party._reagents[reagent]) {
			txt_X = 24;
			u4_putc(reagent+'A');
			u4_putl(Party._reagents[reagent], 2, '-');
			u4_putc('-');
			u4_puts(Names[NAME_REAGENTS + reagent]);
			while(txt_X != 39)
				u4_putc(' ');
			txt_Y ++;
		}
	} while(++reagent < 8);
	ZtatsClearRest();
}

/*
 * Ztats page of the mixtures (spell letter and count), in columns of 8 lines;
 * stops when the status area is full.
 */
int16 /*C_4C42*/ZtatsMixtures()
{
	register int16 spell;

	txt_Y = 0;
	ZtatsTitle(/*D_19B6*/"Mixtures", 0);
	txt_Y = 1;
	txt_X = 24;
	spell = 0;
	do {
		if(Party._mixtures[spell]) {
			u4_putc(spell+'A');
			u4_putc('-');
			u4_putl(Party._mixtures[spell], 2, '0');
			txt_X -= 4;
			txt_Y ++;
			if(txt_Y == 9) {
				txt_Y = 1;
				txt_X += 5;
				if(txt_X >= 39)
					return;
			}
		}
	} while(++spell < 26);
}

typedef z_handler(int16);
typedef z_handler *pZ_handler;

/*the Ztats pages, by page number (0-7: party members)*/
pZ_handler /*D_19C0*/ZtatsPages[] = {
	ZtatsChara,ZtatsChara,ZtatsChara,ZtatsChara,
	ZtatsChara,ZtatsChara,ZtatsChara,ZtatsChara,
	ZtatsWeapons,
	ZtatsArmour,
	ZtatsEquipment,
	ZtatsItems,
	ZtatsReagents,
	ZtatsMixtures
};

/*
 * Show the Ztats pages starting at page (see ZtatsPages): 1-8 select a member,
 * 0 the weapons, arrows/PgUp/PgDn/Home/End browse (skipping the absent
 * members), any other key leaves.
 */
int16 /*C_4CC1*/ZtatsBrowse(page)
int16 page;
{
	uint16 orig_y, orig_x;

	orig_y = txt_Y;
	orig_x = txt_X;
	do {
		register int16 key;

		Gra_ClearStatus();
		ZtatsClearTitle();
		(*ZtatsPages[page])(page);
		while(!u_kbhit());
		switch(key = u_kbread()) {
			case KBD_1:
			case KBD_2:
			case KBD_3:
			case KBD_4:
			case KBD_5:
			case KBD_6:
			case KBD_7:
			case KBD_8:
				if((key&0xf) <= Party._members)
					page = (key&0xf) - 1;
				else
					sound(SND_BEEP);
			break;
			case KBD_RIGHT: case KBD_DOWN:
				page = (page + 1) % 14;
				if(page == Party._members)
					page = 8;
			break;
			case KBD_UP: case KBD_LEFT:
				if(page == 8)
					page = Party._members - 1;
				else
					page = (page + 13) % 14;
			break;
			case KBD_PGUP:
				if(page >= 1 && page <= 8)
					page = 0;
				else if(page > 8 && page <= 13)
					page = 8;
				else page = 13;
			break;
			case KBD_HOME:
				page = 0;
			break;
			case KBD_0:
				page = 8;
			break;
			case KBD_PGDN:
				if(page >= 0 && page <= 7)
					page = 8;
				else if(page >= 8 && page <= 12)
					page = 13;
				else
					page = 0;
			break;
			case KBD_END:
				page = 13;
			break;
			default:
				page = -1;
		}
	} while(page != -1);
	ZtatsClearTitle();
	Gra_ClearStatus();
	dspl_Stats();
	txt_Y = orig_y;
	txt_X = orig_x;
}

/*
 * Ztats: in combat, of the active character; otherwise ask for a member
 * (0 for the weapons page).
 */
int16 /*C_4E45*/CMD_Ztats()
{
	if(CurMode >= MOD_COMBAT) {
		u4_puts(/*D_19DC*/"Ztats\n");
		Gra_InvertStatusLine(activeChara);
		ZtatsBrowse(activeChara);
		Gra_InvertStatusLine(activeChara);
	} else {
		int16 member;

		member = AskChara(/*D_19E3*/"Ztats for:\x12\x12\b");
		if(member != -1)
			ZtatsBrowse((member == -2)?8:member);
	}
}
