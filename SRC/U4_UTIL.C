/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>
#include <stdlib.h>

/*
 * Shake the screen (4 times up and down).
 */
int16 /*C_095E*/ShakeScreen()
{
	Gra_ShiftUp(); Gra_ShiftDown();
	Gra_ShiftUp(); Gra_ShiftDown();
	Gra_ShiftUp(); Gra_ShiftDown();
	Gra_ShiftUp(); Gra_ShiftDown();
}

/*
 * Give xp experience points to the character chara (max 9999).
 */
int16 /*C_097D*/XP_inc(chara, xp)
int16 chara;
int16 xp;
{
	Party.chara[chara]._XP += xp;
	if(Party.chara[chara]._XP > 9999)
		Party.chara[chara]._XP = 9999;
}

int16 /*C_09A3*/w_Pass()
{
	u4_puts("Pass\n");
}

/*
 * Give amount hit points to the character chara, up to its maximum.
 */
int16 /*C_09B1*/HP_inc(chara, amount)
int16 chara;
int16 amount;
{
	register U16 *hp = Party.chara[chara]._HP;
	if((hp[0] += amount) > hp[1])
		hp[0] = hp[1];
}

/*
 * Flash the status line of the character chara, with the "hurt" sound.
 */
int16 /*C_09D9*/FlashChara(chara)
int16 chara;
{
	Gra_InvertStatusLine(chara);
	sound(SND_HURT);
	Gra_InvertStatusLine(chara);
}

/*
 * Increase a virtue (karma points, max 99). A virtue at 0 has been
 * achieved (the party has its eighth of avatarhood) and does not change.
 */
int16 /*C_09F8*/karma_inc(karma, amount)
U16 *karma;
uint16 amount;
{
	if(*karma) {
		*karma += amount;
		if(*karma > 99)
			*karma = 99;
	}
}

/*
 * Decrease a virtue. If it had been achieved (0), the eighth is lost
 * (with a message) and the virtue starts again from 99.
 * Probable original bug: *karma is unsigned, so the "<= 0" test only catches
 * 0; a virtue lower than amount wraps around to a huge value instead of 1.
 */
int16 /*C_0A17*/karma_dec(karma, amount)
U16 *karma;
uint16 amount;
{
	if(*karma == 0) {
		*karma = 99;
		Gra_VirtueGlyph();
		/*redraw the spell effect indicator of the status area*/
		txt_X = 31;
		txt_Y = 10;
		u4_putc(spell_effect);
		u4_puts("\n Thou hast lost\n  an Eighth!\n");
	}
	*karma -= amount;
	if(*karma <= 0)
		*karma = 1;
}

/*
 * Look for an NPC/monster (Area._npc, indexes last down to 0) at (x, y).
 * Returns its index, or -1 if there is none.
 */
int16 /*C_0A58*/FindNpcAt(last, x, y)
int16 last;
unsigned char x;
unsigned char y;
{
	register int16 i;
	for(i = last; i >= 0; i--) {
		if(
			Area._npc._tile[i] != 0 &&
			Area._npc._x[i] == x &&
			Area._npc._y[i] == y
		) break;
	}
	return i;
}

/*
 * FindNpcAt for the current map: 8 monsters outdoors, 32 NPCs elsewhere.
 */
int16 /*C_0A8B*/FindLocalNpcAt(x, y)
unsigned char x;
unsigned char y;
{
	return FindNpcAt((Party._loc == 0)?7:31, x, y);
}

/*
 * a % b, or 0 when b is 0.
 */
int16 /*C_0AB1*/SafeModulo(a, b)
int16 a;
int16 b;
{
	if(b == 0)
		return 0;
	return a % b;
}

/*
 * The tile of the character chara: its class's tile, or TIL_SLEEPING
 * if it is asleep.
 */
int16 /*C_0ACF*/CharaTile(chara)
int16 chara;
{
	if(Party.chara[chara]._stat == 'S')
		return TIL_SLEEPING;
	return (Party.chara[chara]._class << 1) + TIL_MAGE;
}

/*
 * Returns 1 if the tile is not an evil creature: attacking it costs karma,
 * letting it flee is rewarded. These are TIL_DEEP_WATER..TIL_WALL (people,
 * ...), TIL_SEAHORSE, TIL_RAT, TIL_BAT, TIL_SPIDER, TIL_INSECTS and
 * TIL_PYTHON; else returns 0.
 */
int16 /*C_0AFE*/IsNonEvil(tile)
unsigned char tile;
{
	if(tile < TIL_PIRATE_W ||
		tile == TIL_SEAHORSE || tile == TIL_RAT ||
		tile == TIL_BAT || tile == TIL_SPIDER ||
		tile == TIL_INSECTS || tile == TIL_PYTHON
	) return 1;
	return 0;
}

/*
 * Print a text in the text window (columns 24-39), with word wrap: a word
 * that does not fit on the line goes to the next one. Every 13 lines,
 * waits for a key so that the text can be read.
 * Returns the number of characters printed.
 */
int16 /*C_0B38*/u4_puts(txt)
register char *txt;
{
	register int16 i;
	int16 lines;/*lines since the last pause*/
	int16 wordLen;/*characters left in the current word*/

	lines = wordLen = 0;
	i = 0;
	while(txt[i]) {
		if(wordLen-- == 0) {
			/*start of a word: measure it*/
			char c;
			for(wordLen = 0; c = txt[i + wordLen]; wordLen ++) {
				if(c == '\n' || c == ' ')
					break;
			}
			if(wordLen + txt_X > 40 && txt_X != 24) {
				if(lines++ == 12) {
					u_kbflush();
					if(txt_X == 39)
						while(!u_kbhit());
					u_kbread();
					lines = 0;
				}
				Gra_CR();
			}
		}
		if(txt[i] == '\n' && lines++ == 12) {
			u_kbflush();
			if(txt_X == 39)
				while(!u_kbhit());
			u_kbread();
			lines = 0;
		}
		u4_putc(txt[i]);
		i++;
	}
	return i;
}

/*
 * Print a number right-aligned on width characters, padded on the left
 * with the character pad.
 * Probable original bug: a negative value prints the '-' and the absolute
 * value, but then goes on with the negative value (no return) and prints
 * padding and garbage after it. The game only prints positive values.
 */
int16 /*C_0C03*/u4_putl(value, width, pad)
int32 value;
int16 width;
char pad;
{
	if(value < 0) {
		u4_putc('-');
		u4_putl(-value, width - 1, pad);
	}
	if(value < 10) {
		if(width > 1) {
			u4_putc(pad);
			u4_putl(value, width - 1, pad);
		} else {
			u4_putc(value + '0');
		}
	} else {
		u4_putl(value / 10, width - 1, pad);
		u4_putc((value % 10) + '0');
	}
}

/*
 * Print a character in the text window, handling backspace and newline;
 * goes to the next line when the line is full (a space at the end of a
 * line is dropped).
 */
int16 /*C_0C9F*/u4_putc(c)
unsigned char c;
{
	switch(c) {
		case '\b':
			if(txt_X > 1) {
				txt_X --;
				Gra_PutChar(' ');
			}
		break;
		case '\n':
			Gra_CR();
		break;
		case ' ':
			if(txt_X <= 39) {
				Gra_PutChar(c);
				txt_X ++;
			}
		break;
		default:
			if(txt_X > 39)
				Gra_CR();
			Gra_PutChar(c);
			txt_X ++;
	}
}

/*
 * Redraw the status area: for each character its number, name, hit points
 * and state; then the food, the spell effect and the ship's hull (on a
 * ship) or the gold. The text cursor is kept.
 */
int16 /*C_0CF7*/dspl_Stats()
{
	register int16 i;
	register struct tChara *pChara;
	int16 member;
	int16 prevCol, prevRow;

	Gra_VirtueGlyph();
	prevRow = txt_Y;
	prevCol = txt_X;
	for(
		member = 0, pChara = &(Party.chara[0]);
		(uint16)(member) < Party._members;
		pChara++, member++
	) {
		txt_Y = member + 1;
		txt_X = 24;
		u4_putc(member + '1'); u4_putc('-');
		for(i = 0; pChara->_name[i] && i < 8; i++)
			u4_putc(pChara->_name[i]);
		txt_X = 35;
		u4_putl(pChara->_HP[0], 3, ' ');
		u4_putc(pChara->_stat);
	}
	txt_X = 24;
	txt_Y = 10;
	u4_putc('F'); u4_putc(':');
	u4_putl(Party._food / 100, 4, '0');
	u4_putc(' ');
	u4_putc(spell_effect);
	if(Party._tile < TIL_HORSE_W) {
		u4_puts(" SHP:");
		u4_putl(Party._ship, 2, '0');
	} else {
		u4_puts(" G:");
		u4_putl(Party._gold, 4, '0');
	}
	txt_Y = prevRow;
	txt_X = prevCol;
}

/*
 * Redraw the gold in the status area (the text cursor is kept).
 */
int16 /*C_0E0E*/dspl_Gold() {
	int16 prevCol, prevRow;

	prevCol = txt_X;
	prevRow = txt_Y;
	txt_X = 35;
	txt_Y = 10;
	u4_putl(Party._gold, 4, '0');
	txt_Y = prevRow;
	txt_X = prevCol;
}

/*
 * Returns 1 if the character chara exists and is alive: state 'G' (good),
 * 'P' (poisoned) or 'S' (asleep).
 */
int16 /*C_0E4E*/isCharaAlive(chara)
int16 chara;
{
	if((uint16)(chara) >= Party._members)
		return 0;
	switch(Party.chara[chara]._stat) {
		case 'G': case 'P': case 'S': return 1;
		default: return 0;
	}
}

/*
 * Returns 1 if the character chara exists and can act: state 'G' or 'P'.
 */
int16 /*C_0E82*/isCharaConscious(chara)
int16 chara;
{
	if((uint16)(chara) >= Party._members)
		return 0;
	switch(Party.chara[chara]._stat) {
		case 'G': case 'P': return 1;
		default: return 0;
	}
}

/*
 * The whole party is dead: after the "All is Dark" sequence, Lord British
 * resurrects it in his castle (LCB_2.ULT), with full hit points, but without
 * weapons and armour, with 200 gold and 200 food. Does not return: jumps
 * back to the main loop.
 */
int16 /*C_0EB1*/PartyDeath()
{
	int16 i;

	for(i = Party._members - 1; i >= 0; i --) {
		Party.chara[i]._stat = 'D';
		Party.chara[i]._HP[0] = 0;
	}
	Gra_ClearStatus();
	dspl_Stats();
	u_delay(5, 0);
	Gra_ClearView();
	CurMode = MOD_VISION;
	u4_puts("\n\n\nAll is Dark...\n");
	u_delay(5, 0);
	u4_puts("\nBut wait...\n");
	u_delay(5, 0);
	u4_puts("Where am I?...\n");
	u_delay(5, 0);
	u4_puts("Am I dead?...\n");
	u_delay(5, 0);
	u4_puts("Afterlife?...\n");
	u_delay(5, 0);
	u4_puts("You hear:\n");
	txt_X = (uint16)((12 - (uint16)strlen(Party.chara[0]._name))) / 2 + 25;
	u4_puts(Party.chara[0]._name);
	Gra_CR();
	u_delay(5, 0);
	u4_puts("I feel motion...\n");
	u_delay(5, 0);
	Party._tile = TIL_AVATAR;
	/*in Lord British's castle, and outside of it on the world map*/
	Party._x = 0x13; Party._y = 0x08;
	Party.out_x = 0x56;
	Party.out_y = 0x6c;

	if(File_TLK) {
		dclose(File_TLK);
		File_TLK = 0;
	}
	if(File_DNG) {
		dclose(File_DNG);
		File_TLK = 0;/*probable original bug: File_DNG is not reset*/
	}
	if(Party._loc == 0) {
		if(Save("OUTMONST.SAV", sizeof(struct tNPC), &(Area._npc)) == -1)
			exit(3);
	}
	if(Load("LCB_2.ULT", sizeof(struct tArea), &Area) == -1)
		exit(3);
	File_TLK = dopen("LCB.TLK", 0);
	CurMode = MOD_BUILDING;
	Party._loc = LOC_LCB;
	Party._fly_light = 0;
	SavedMode = MOD_OUTDOORS;
	spell_turns = 0;
	hit_tile = 0;
	spell_effect = 7;
	LineOfSight = 1;
	Gallop = 0;
	RefreshView();
	UpdateMoons();
	u4_puts("\n\nLord British says: I have pulled thy spirit and some possessions from the void.  Be more careful in the future!\n");
	for(i = Party._members - 1; i >= 0; i --) {
		Party.chara[i]._stat = 'G';
		Party.chara[i]._HP[0] = Party.chara[i]._HP[1];
	}
	for(i = 15; i >= 0; i --)
		Party._weapons[i] = 0;
	for(i = 7; i >= 0; i --)
		Party._armors[i] = 0;
	Party._food = 20099L;
	Party._gold = 200;
	dspl_Stats();
	u_kbflush();
	longjmp(GameLoopJmp, -1);
}

/*
 * Returns 1 if at least one character is conscious, 0 if the living ones
 * are all asleep. If the whole party is dead, calls PartyDeath (no return).
 */
int16 /*C_10FD*/CheckPartyAlive()
{
	register int16 i;

	for(i = Party._members - 1; i >= 0; i --) {
		if(isCharaAlive(i)) {
			for(; i >= 0; i--) {
				if(isCharaConscious(i))
					return 1;
			}
			return 0;
		}
	}
	PartyDeath();
}

/*
 * The character chara takes damage hit points.
 * Returns 0 if it survives, 1 if it dies.
 */
int16 /*C_1135*/hitChara(chara, damage)
uint16 damage;
int16 chara;
{
	register struct tChara *pChara = &(Party.chara[chara]);
	if(pChara->_HP[0] >= damage) {
		pChara->_HP[0] -= damage;
		dspl_Stats();
		return 0;
	}
	/*character dies*/
	pChara->_HP[0] = 0;
	pChara->_stat = 'D';
	if(CurMode >= MOD_COMBAT)
		CharaAnimTile[chara] = Fighters._chtile[chara] = TIL_SLEEPING;
	dspl_Stats();
	return 1;
}

int16 /*C_1188*/w_What()
{
	u4_puts("What?\n");
	u_kbflush();
}

int16 /*C_1199*/w_Cant_t()
{
	u4_puts("Can't!\n");
	u_kbflush();
}

int16 /*C_11AA*/w_NotHere()
{
	u4_puts("Not Here!\n");
	u_kbflush();
}

int16 /*C_11BB*/w_Disabled()
{
	u4_puts("Disabled!\n");
	u_kbflush();
}

int16 /*C_11CC*/w_OnlyOnFoot()
{
	u4_puts("Only on foot!\n");
	u_kbflush();
}

int16 /*C_11DD*/w_SlowProgress()
{
	u4_puts("Slow progress!\n");
}

int16 /*C_11EB*/w_NoneLeft()
{
	u4_puts("None left!\n");
}

/*
 * Ask (prompt) for a letter between lo and hi, repeating the question with
 * a beep until one is typed. Returns it (upper case), -1 for Enter/Space,
 * -2 for Escape.
 */
int16 /*C_11F9*/AskLetter(prompt, lo, hi)
char *prompt;
char lo;
char hi;
{
	register int16 key;

	key = -1;
	do {
		if(key != -1)
			sound(SND_BEEP);
		u4_puts(prompt);
		key = u_kbread();
		if(key == KBD_ENTER || key == KBD_ESC || key == KBD_SPACE) {
			if(key == KBD_ENTER || key == KBD_SPACE)
				key = -1;
			else
				key = -2;
			Gra_CR();
			break;
		}
		key &= 0xff;
		u4_toupper2(key);
		if(key > ' ' && key < 0x7f)
			u4_putc(key);
		Gra_CR();
		if(lo <= key && key <= hi)
			break;
	} while(key >= 0);

	return key;
}

/*
 * Ask (prompt) for a character number. With a single character, answers
 * 1 by itself. Returns the character's index (0-based), -1 if none was
 * chosen, -2 for '0'.
 */
int16 /*C_1287*/AskChara(prompt)
char *prompt;
{
	register int16 key;

	if(Party._members == 1) {
		u4_puts(prompt);
		u4_puts("1\n");
		return 0;
	}
	key = AskLetter(prompt, '0', '0' + Party._members);
	if(key == '0')
		return -2;
	if(key < 0)
		return -1;

	return key - '1';
}

/*
 * Ask (prompt) for a direction (arrow keys) into *dx, *dy. Space, Escape,
 * Backspace or Enter give no direction (0, 0); any other key beeps and
 * asks again.
 */
int16 /*C_12D6*/AskDir(prompt, dx, dy)
char *prompt;
register int16 *dx;
register int16 *dy;
{
	uint16 valid, key;

	*dx = *dy = 0;
	do {
		valid = 1;
		u4_puts(prompt);
		key = u_kbread();
		switch(key) {
			case KBD_SPACE: case KBD_ESC: case KBD_BS: case KBD_ENTER:
			break;
			case KBD_UP:
				*dy = -1;
				u4_puts("North");
			break;
			case KBD_DOWN:
				*dy = 1;
				u4_puts("South");
			break;
			case KBD_RIGHT:
				*dx = 1;
				u4_puts("East");
			break;
			case KBD_LEFT:
				*dx = -1;
				u4_puts("West");
			break;
			default:
				key &= 0xff;
				if(key >= ' ' && key < 0x7f)
					u4_putc(key);
				Gra_CR();
				sound(SND_BEEP);
				valid = 0;
		}
	} while(!valid);
	Gra_CR();
}

/*
 * Consume amount food. Returns 1 if there was not enough (food becomes 0),
 * else 0.
 */
int16 /*C_138B*/food_dec(amount)
int16 amount;
{
	Party._food -= amount;
	if(Party._food >= 0)
		return 0;
	Party._food = 0;

	return 1;
}

/*
 * Each living character recovers 1 magic point, up to the maximum of its
 * class: mage 2*int, bard/paladin/ranger int, druid 3/2*int, tinker int/2,
 * others 0 (max 99).
 */
int16 /*C_13B6*/MP_recover()
{
	register struct tChara *p;
	int16 chara;

	for(chara = Party._members - 1; chara >= 0; chara --) {
		if(isCharaAlive(chara)) {
			register int16 maxMP;
			p = &(Party.chara[chara]);
			switch(p->_class) {
				case CLASS_MAGE: maxMP = p->_int * 2; break;
				case CLASS_BARD:
				case CLASS_PALADIN:
				case CLASS_RANGER: maxMP = p->_int; break;
				case CLASS_DRUID: maxMP = (p->_int / 2) + p->_int; break;
				case CLASS_TINKER: maxMP = p->_int / 2; break;
				default: maxMP = 0;
			}
			if(maxMP > 99)
				maxMP = 99;
			p->_MP ++;
			if(p->_MP > (uint16)(maxMP))
				p->_MP = maxMP;
		}
	}
}

/*
 * Read a line of text (at most size - 1 characters) into buf, with echo;
 * Backspace, Delete and Left erase. Leading and trailing spaces are removed.
 * Probable original bug: for an empty answer the final loop reads buf[-1]
 * (and clears the bytes before buf as long as they are spaces).
 */
int16 /*C_1445*/u4_gets(buf, size)
register char *buf;
uint16 size;
{
	register int16 len;
	uint16 key;

	len = 0;
	do {
		key = u_kbread();
		switch(key) {
			case KBD_BS:
			case KBD_CTRL_BS:
			case KBD_LEFT:
				if(len == 0) {
					sound(SND_BEEP);
				} else {
					u4_putc(8);
					len --;
					buf[len] = ' ';
				}
			break;
			default:
				key &= 0xff;
				if((uint16)(size - 1) == (uint16)(len) || key < ' ' || key >= 0x80) {
					sound(SND_BEEP);
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

	/*remove the leading spaces (key is now an index)*/
	key = 0;
	while(buf[key++] == ' ');
	key --;
	if(buf[key]) {
		len = 0;
		while(buf[key])
			buf[len++] = buf[key++];
	} else {
		len = key;
	}
	/*remove the trailing spaces*/
	do {
		buf[len] = 0;
	} while(LOCAL_AT(buf, --len) == ' ');
}

/*
 * The name of the creature or person shown by a tile.
 */
char * /*C_1513*/CreatureName(tile)
unsigned char tile;
{
	if(tile >= TIL_PIRATE_W) {
		if(tile >= TIL_RAT)
			return Names[NAME_MONSTERS + (tile - TIL_PIRATE_W)/4 - (TIL_RAT - TIL_PIRATE_W)/4];
		else
			return Names[NAME_CREATURES + (tile - TIL_PIRATE_W)/2];
	}
	if(tile == TIL_SLEEPING)
		return Names[NAME_VILLAGER];
	if(tile < TIL_MAGE || tile >= TIL_LETTER_A || (tile >= TIL_30 && tile < TIL_GUARD))
		return Names[NAME_PHANTOM];/*"Phantom"*/
	return Names[NAME_CLASSES + (tile & 0x1f)/2];
}

/*
 * Damage to the whole party (missiles, fields, lava, whirlpools, ...): each
 * living character has one chance in two to take 10-24 hit points. On a
 * ship, the hull loses 10 instead and the ship sinks when it goes below 0.
 * Probable original bug: Party._ship is unsigned, so "< 0" is never true:
 * the ship never sinks here and the hull wraps around.
 */
int16 /*C_1584*/DamageAllParty()
{
	register int16 i;
	int16 damage;

	for(i = Party._members - 1; i >= 0; i --)
		Gra_InvertStatusLine(i);
	sound(SND_HIT);
	ShakeScreen();
	for(i = Party._members - 1; i >= 0; i --)
		Gra_InvertStatusLine(i);
	if(CurMode >= MOD_COMBAT || Party._tile > TIL_SHIP_S) {
		/*normal case*/
		for(i = Party._members - 1; i >= 0; i --) {
			if(U4_RND1(1) && isCharaAlive(i)) {
				damage = U4_RND3(15) + 10;
				if(CurMode < MOD_COMBAT || Fighters._chtile[i])
					hitChara(i, damage);
			}
		}
	} else {
		/*on ship*/
		if((Party._ship -= 10) < 0) {
			Party._ship = 0;
			for(i = Party._members; --i >= 0; )
				Gra_InvertStatusLine(i);
			dspl_Stats();
			u4_puts("Thy Ship Sinks!\n");
			PartyDeath();
		}
	}
	dspl_Stats();
}

/*
 * Wait for Y or N (or Space, Escape, Enter), beeping at other keys.
 * Returns the key (upper case).
 */
int16 /*C_162F*/AskY_N()
{
	int16 key;

	key = -1;
	do {
		if(key != -1)
			sound(SND_BEEP);
		key = u_kbread() & 0x7f;
		u4_toupper(key);
	} while(
		key != 'N' && key != 'Y' &&
		key != ' ' && key != 0x1b && key != '\r'
	);
	if(key == 'Y' || key == 'N')
		u4_putc(key);
	Gra_CR();
	return (char)key;
}

/*
 * Read a number of at most digits characters. Returns it, or -1 for an
 * empty answer.
 */
int32 /*C_169C*/AskInt(digits)
int16 digits;
{
	char answer[20];

	u4_gets(answer, digits + 1);
	Gra_CR();
	return answer[0]?atol(answer):-1L;
}
