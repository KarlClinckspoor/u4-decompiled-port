/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>

char /*D_00EE*/MsgNoEffect[] = "Hmm...No effect!\n";
char /*D_0100*/MsgNoneOwned[] = "None owned!\n";
/*for each altar room (truth, love, courage), the mask of the 4 stone
  colors (bit = index in StoneColors) that must be placed on the altar*/
char /*D_010E*/AltarStoneMasks[] = {
	(1<<6)|(1<<5)|(1<<3)|(1<<0),/*truth: white, purple, green, blue*/
	(1<<6)|(1<<4)|(1<<3)|(1<<1),/*love: white, orange, green, yellow*/
	(1<<6)|(1<<5)|(1<<4)|(1<<2) /*courage: white, purple, orange, red*/
};

/*
 * Use the stones in an altar room of the Abyss: the active character must stand
 * at the center of the room (5,5). Asks for 4 stone colors; each must be owned
 * and given only once, and together they must match the room's AltarStoneMasks.
 * Then the party gets the room's third of the Three Part Key (only once).
 * An unknown or unowned color (or an empty answer, silently) ends the use.
 */
void /*C_01E1*/USE_StonesAtAltar()
{
	int color, hole;
	char answer[12];
	char mask;/*colors given so far*/

	if(Combat._charaX[activeChara] != 5 || Combat._charaY[activeChara] != 5 || AltarRoom == -1) {
		u4_puts(MsgNoEffect);
		return;
	}
	u4_puts(/*D_0080*/"\nThere are holes for 4 stones. What colors:\n");
	mask = 0;
	for(hole = 1; hole <= 4; hole++) {
		u4_putc(hole + 'A' - 1); u4_putc(':');
		u4_gets(answer, 11);
		Gra_CR();
		for(color = 7; color >= 0; color--) {
			if(strnicmp(StoneColors[color], answer, 12) == 0)
				break;
		}
		if(color == -1 || !TST_MSK(Party.mStones, color)) {
			if(*answer != '\0')
				u4_puts(MsgNoneOwned);
			return;
		}
		if((1 << color) & mask) {
			u4_puts(/*D_00AD*/"Already used!\n");
			return;
		}
		mask |= (1 << color);
	}
	if(AltarStoneMasks[AltarRoom] != mask) {
		u4_puts(MsgNoEffect);
		return;
	}
	switch(AltarRoom) {
		case 0:
			if(TST_MSK(Party.mItems, ST_KEY_T)) {
				u4_puts(MsgNoEffect);
				return;
			}
			SET_MSK(Party.mItems, ST_KEY_T);
		break;
		case 1:
			if(TST_MSK(Party.mItems, ST_KEY_L)) {
				u4_puts(MsgNoEffect);
				return;
			}
			SET_MSK(Party.mItems, ST_KEY_L);
		break;
		case 2:
			if(TST_MSK(Party.mItems, ST_KEY_C)) {
				u4_puts(MsgNoEffect);
				return;
			}
			SET_MSK(Party.mItems, ST_KEY_C);
		break;
	}
	u4_puts(/*D_00BC*/"Thou doth find one third of the Three Part Key!\n");
}

/*the question asked at the altar of each level of the Abyss*/
char * /*D_0284*/AbyssVirtueNames[] = {
	/*D_0111*/"Truth",
	/*D_0117*/"Love",
	/*D_011C*/"Courage",
	/*D_0124*/"Truth and Love",
	/*D_0133*/"Love and Courage",
	/*D_0144*/"Courage and Truth",
	/*D_0156*/"Truth, Love and Courage",
	/*D_016E*/"\nA voice rings out:  What Virtue exists independently of Truth, Love and Courage"
};

/*
 * "Use stone(s)". In the Abyss, on an altar (dungeon cell 0xb0) of level z,
 * the party must name the virtue of that level (Names[NAME_VIRTUES + z]) and
 * then use the stone of the same index (StoneColors[z]): the altar becomes a
 * ladder down (0x20), or on the last level the game ends (EndGame).
 * In an altar room, see USE_StonesAtAltar.
 * Note: anywhere else it prints "No place to Use them!" but still goes on
 * to USE_StonesAtAltar, which then usually adds "Hmm...No effect!".
 */
/*C_0311*/USE_Stones() {
	int color;

	if(Party.mStones) {
		char answer[14];
		if(CurMode != MOD_COM_ROOM && CurMode != MOD_DUNGEON)
			u4_puts(/*D_01BF*/"No place to Use them!\n");
		if(CurMode == MOD_DUNGEON) {
			if(tile_cur != 0xb0 || Party._loc != LOC_ABYSS) {
				u4_puts(MsgNoEffect);
				return;
			}
			if(Party._z != 7)
				u4_puts(/*D_01D6*/"\nAs thou doth approach, a voice rings out: What virtue dost stem from ");
			u4_puts(AbyssVirtueNames[Party._z]);
			u4_puts(/*D_021D*/"?\n\n");
			u4_gets(answer, 13);
			Gra_CR();
			if(strnicmp(answer, Names[NAME_VIRTUES + Party._z], 14)) {
bad_answer:
				if(answer[0])
					u4_puts(MsgNoEffect);
				return;
			}
			u4_puts(/*D_0221*/"\nThe Voice says: Use thy Stone.\n\nColor:\n");
			u4_gets(answer, 11);
			Gra_CR();
			color = 7;
			while(
				strnicmp(answer, StoneColors[color], 12) &&
				--color >= 0
			);
			if(color == -1)
				goto bad_answer;
			if(!TST_MSK(Party.mStones, color)) {
				u4_puts(/*D_024A*/"\nYou have none!\n\n");
				return;
			}
			if(Party._z != color) {
				u4_puts(MsgNoEffect);
				return;
			}
			if(Party._z == 7) {
				EndGame();/*last phase of game*/
				return;
			}
			/*the altar becomes a ladder down*/
			Area._map.x8x8x8[Party._z][Party._y][Party._x] = 0x20;
			u4_puts(/*D_025C*/"\nThe altar changes before thyne eyes!\n");
		} else {
			USE_StonesAtAltar();
		}
	} else {
		u4_puts(MsgNoneOwned);
	}
}

/*
 * "Use key(s)": the parts of the Three Part Key cannot be used directly
 * (the key is used at the end of the game, in EndGame).
 */
/*C_044C*/USE_Keys() {
	if(
		((Party.mItems >> ST_KEY_T) & 1) |
		((Party.mItems >> ST_KEY_L) & 1) |
		((Party.mItems >> ST_KEY_C) & 1)
	)
		u4_puts("No place to Use them!\n");
	else
		u4_puts(MsgNoneOwned);
}

/*
 * "Use bell": first step of opening the Abyss, only on the entrance of the
 * Abyss on the world map (0xe9, 0xe9).
 */
/*C_0487*/USE_Bell() {
	if(!TST_MSK(Party.mItems, ST_BELL)) {
		u4_puts(MsgNoneOwned);
		return;
	}
	if(Party._loc != 0 || Party._x != 0xe9 || Party._y != 0xe9) {
		u4_puts(MsgNoEffect);
		return;
	}
	SET_MSK(Party.mItems, ST_USE_BELL);
	u4_puts("The Bell rings on and on!\n");
}

/*
 * "Use book": second step of opening the Abyss (after the bell),
 * on the entrance of the Abyss.
 */
/*C_04C0*/USE_Book() {
	if(!TST_MSK(Party.mItems, ST_BOOK)) {
		u4_puts(MsgNoneOwned);
		return;
	}
	if(Party._loc != 0 || Party._x != 0xe9 || Party._y != 0xe9 ||
		!TST_MSK(Party.mItems, ST_USE_BELL)
	) {
		u4_puts(MsgNoEffect);
		return;
	}
	SET_MSK(Party.mItems, ST_USE_BOOK);
	u4_puts("The words resonate with the ringing!\n");
}

/*
 * "Use candle": third and last step of opening the Abyss (after the book),
 * on the entrance of the Abyss.
 */
/*C_0501*/USE_Candle() {
	if(!TST_MSK(Party.mItems, ST_CANDLE)) {
		u4_puts(MsgNoneOwned);
		return;
	}
	if(Party._loc != 0 || Party._x != 0xe9 || Party._y != 0xe9 ||
		!TST_MSK(Party.mItems, ST_USE_BOOK)
	) {
		u4_puts(MsgNoEffect);
		return;
	}
	SET_MSK(Party.mItems, ST_USE_CANDLE);
	u4_puts("As you light the Candle the Earth Trembles!\n");
	sound(SND_HIT);
	ShakeScreen();
}

/*
 * "Use horn": on the world map only, for 10 turns (spell_effect 1), the
 * daemons guarding the entrance of the Abyss do not appear (SpawnDaemons).
 */
/*C_0553*/USE_Horn() {
	if(!TST_MSK(Party.mItems, ST_HORN)) {
		u4_puts(MsgNoneOwned);
		return;
	}
	if(Party._loc != 0) {
		u4_puts(MsgNoEffect);
		return;
	}
	u4_puts("The Horn sounds an eerie tone!\n");
	spell_effect = 1;
	spell_turns = 10;
	dspl_Stats();
}

/*
 * "Use wheel": on the world map, aboard a ship whose hull is at 50 (full),
 * the hull strength becomes 99.
 */
/*C_058C*/USE_Wheel() {
	if(!TST_MSK(Party.mItems, ST_WHEEL)) {
		u4_puts(MsgNoneOwned);
		return;
	}
	if(Party._loc != 0 || Party._tile > TIL_SHIP_S || Party._ship != 50) {
		u4_puts(MsgNoEffect);
		return;
	}
	u4_puts("Once mounted, the Wheel glows with a blue light!\n");
	Party._ship = 99;
	dspl_Stats();
}

/*
 * "Use skull". Anywhere but on the entrance of the Abyss, it removes every
 * monster and townsperson from the map (and from the fight), except Lord
 * British, at the cost of 5 points in each virtue. On the entrance of the
 * Abyss, it is cast into the Abyss for +10 in each virtue.
 * Either way the skull is lost.
 */
/*C_05CE*/USE_Skull() {
	int i;

	if(!TST_MSK(Party.mItems, ST_SKULL)) {
		u4_puts(MsgNoneOwned);
		return;
	}
	if(Party._loc != 0 || Party._x != 0xe9 || Party._y != 0xe9) {
		u4_puts("\nYou hold the evil Skull of Mondain the Wizard aloft....\n");
		sound(SND_HIT); ShakeScreen();
		Gra_InvertView();
		sound(SND_HIT); ShakeScreen();
		Gra_InvertView();
		sound(SND_HIT); ShakeScreen();
		if(CurMode >= MOD_COMBAT) {
			for(i = 31; i >= 0; i --) {
				if(Area._npc._tile[i] != TIL_LORD_BRITISH)
					Area._npc._tile[i] = Area._npc._gtile[i] = 0;
			}
			for(i = 15; i >= 0; i --) {
				if(Fighters._tile[i] != TIL_LORD_BRITISH)
					Fighters._tile[i] = Fighters._gtile[i] = 0;
			}
		} else {
			for(i = (Party._loc == 0)?7:31; i >= 0; i --) {
				if(Area._npc._tile[i] != TIL_LORD_BRITISH)
					Area._npc._tile[i] = Area._npc._gtile[i] = 0;
			}
		}
		RefreshView();
		karma_dec((char *)&(Party._hones), 5);
		karma_dec((char *)&(Party._compa), 5);
		karma_dec((char *)&(Party._valor), 5);
		karma_dec((char *)&(Party._justi), 5);
		karma_dec((char *)&(Party._sacri), 5);
		karma_dec((char *)&(Party._honor), 5);
		karma_dec((char *)&(Party._spiri), 5);
		karma_dec((char *)&(Party._humil), 5);
		dspl_Stats();
	} else {
		u4_puts("\nYou cast the Skull of Mondain into the Abyss!\n");
		SET_MSK(Party.mItems, ST_CAST_SKULL);
		karma_inc((char *)&(Party._hones), 10);
		karma_inc((char *)&(Party._compa), 10);
		karma_inc((char *)&(Party._valor), 10);
		karma_inc((char *)&(Party._justi), 10);
		karma_inc((char *)&(Party._sacri), 10);
		karma_inc((char *)&(Party._honor), 10);
		karma_inc((char *)&(Party._spiri), 10);
		karma_inc((char *)&(Party._humil), 10);
		sound(SND_HIT); ShakeScreen();
		Gra_InvertView();
		sound(SND_HIT); ShakeScreen();
		Gra_InvertView();
		sound(SND_HIT); ShakeScreen();
	}
	RST_MSK(Party.mItems, ST_SKULL);
}

/*the usable items: name typed by the player, handler*/
struct {
	char *_00;
	void (*_02)();
} /*D_0434*/UseItems[] = {
	{/*D_03D5*/"stone",  USE_Stones},
	{/*D_03DB*/"stones", USE_Stones},
	{/*D_03E2*/"bell",   USE_Bell},
	{/*D_03E7*/"book",   USE_Book},
	{/*D_03EC*/"candle", USE_Candle},
	{/*D_03F3*/"key",    USE_Keys},
	{/*D_03F7*/"keys",   USE_Keys},
	{/*D_03FC*/"horn",   USE_Horn},
	{/*D_0401*/"wheel",  USE_Wheel},
	{/*D_0407*/"skull",  USE_Skull},
	{/*D_040D*/"",       0}
};

/*
 * The (U)se command: asks for an item name and calls its handler.
 */
/*C_07AE*/CMD_Use() {
	register int item;
	char answer[12];

	u4_puts("Use which item:\n");
	u4_gets(answer, 11);
	Gra_CR();
	for(item = 0; UseItems[item]._00[0]; item++) {
		if(strnicmp(UseItems[item]._00, answer, 12) == 0) {
			(*(UseItems[item]._02))();
			return;
		}
	}
	u4_puts("Not a Usable item!\n");
}
