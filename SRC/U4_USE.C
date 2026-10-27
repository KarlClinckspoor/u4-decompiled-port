/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>

char /*D_00EE*/MsgNoEffect[] = "Hmm...No effect!\n";
char /*D_0100*/MsgNoneOwned[] = "None owned!\n";
char /*D_010E*/AltarStoneMasks[] = {/*colors masks*/
	(1<<6)|(1<<5)|(1<<3)|(1<<0),/*truth*/
	(1<<6)|(1<<4)|(1<<3)|(1<<1),/*love*/
	(1<<6)|(1<<5)|(1<<4)|(1<<2) /*courage*/
};

void /*C_01E1*/USE_StonesAtAltar()
{
	int bp_10, bp_02;
	char bp_12[12];
	char bp_04;

	if(Combat._charaX[activeChara] != 5 || Combat._charaY[activeChara] != 5 || AltarRoom == -1) {
		u4_puts(MsgNoEffect);
		return;
	}
	u4_puts(/*D_0080*/"\nThere are holes for 4 stones. What colors:\n");
	bp_04 = 0;
	for(bp_02 = 1; bp_02 <= 4; bp_02++) {
		u4_putc(bp_02 + 'A' - 1); u4_putc(':');
		u4_gets(bp_12, 11);
		Gra_CR();
		for(bp_10 = 7; bp_10 >= 0; bp_10--) {
			if(strnicmp(StoneColors[bp_10], bp_12, 12) == 0)
				break;
		}
		if(bp_10 == -1 || !TST_MSK(Party.mStones, bp_10)) {
			if(*bp_12 != '\0')
				u4_puts(MsgNoneOwned);
			return;
		}
		if((1 << bp_10) & bp_04) {
			u4_puts(/*D_00AD*/"Already used!\n");
			return;
		}
		bp_04 |= (1 << bp_10);
	}
	if(AltarStoneMasks[AltarRoom] != bp_04) {
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

/*use stone(s)*/
/*C_0311*/USE_Stones() {
	int bp_02;

	if(Party.mStones) {
		char bp_10[14];
		if(CurMode != MOD_COM_ROOM && CurMode != MOD_DUNGEON)
			u4_puts(/*D_01BF*/"No place to Use them!\n");
		if(CurMode == MOD_DUNGEON) {
			if(tile_cur != 0xb0 || Party._loc != 0x18) {
				u4_puts(MsgNoEffect);
				return;
			}
			if(Party._z != 7)
				u4_puts(/*D_01D6*/"\nAs thou doth approach, a voice rings out: What virtue dost stem from ");
			u4_puts(AbyssVirtueNames[Party._z]);
			u4_puts(/*D_021D*/"?\n\n");
			u4_gets(bp_10, 13);
			Gra_CR();
			if(strnicmp(bp_10, Names[151 + Party._z], 14)) {
C_03A1:
				if(bp_10[0])
					u4_puts(MsgNoEffect);
				return;
			}
			u4_puts(/*D_0221*/"\nThe Voice says: Use thy Stone.\n\nColor:\n");
			u4_gets(bp_10, 11);
			Gra_CR();
			bp_02 = 7;
			while(
				strnicmp(bp_10, StoneColors[bp_02], 12) &&
				--bp_02 >= 0
			);
			if(bp_02 == -1)
				goto C_03A1;
			if(!TST_MSK(Party.mStones, bp_02)) {
				u4_puts(/*D_024A*/"\nYou have none!\n\n");
				return;
			}
			if(Party._z != bp_02) {
				u4_puts(MsgNoEffect);
				return;
			}
			if(Party._z == 7) {
				EndGame();/*last phase of game*/
				return;
			}
			Area._map.x8x8x8[Party._z][Party._y][Party._x] = 0x20;
			u4_puts(/*D_025C*/"\nThe altar changes before thyne eyes!\n");
		} else {
			USE_StonesAtAltar();
		}
	} else {
		u4_puts(MsgNoneOwned);
	}
}

/*use key(s)*/
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

/*use bell*/
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

/*use book*/
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

/*use candle*/
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
	sound(6);
	ShakeScreen();
}

/*use horn*/
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

/*use wheel*/
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

/*use skull*/
/*C_05CE*/USE_Skull() {
	int bp_02;

	if(!TST_MSK(Party.mItems, ST_SKULL)) {
		u4_puts(MsgNoneOwned);
		return;
	}
	if(Party._loc != 0 || Party._x != 0xe9 || Party._y != 0xe9) {
		u4_puts("\nYou hold the evil Skull of Mondain the Wizard aloft....\n");
		sound(6); ShakeScreen();
		Gra_InvertView();
		sound(6); ShakeScreen();
		Gra_InvertView();
		sound(6); ShakeScreen();
		if(CurMode >= MOD_COMBAT) {
			for(bp_02 = 31; bp_02 >= 0; bp_02 --) {
				if(Area._npc._tile[bp_02] != TIL_LORD_BRITISH)
					Area._npc._tile[bp_02] = Area._npc._gtile[bp_02] = 0;
			}
			for(bp_02 = 15; bp_02 >= 0; bp_02 --) {
				if(Fighters._tile[bp_02] != TIL_LORD_BRITISH)
					Fighters._tile[bp_02] = Fighters._gtile[bp_02] = 0;
			}
		} else {
			for(bp_02 = (Party._loc == 0)?7:31; bp_02 >= 0; bp_02 --) {
				if(Area._npc._tile[bp_02] != TIL_LORD_BRITISH)
					Area._npc._tile[bp_02] = Area._npc._gtile[bp_02] = 0;
			}
		}
/*C_06B2:*/
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
		sound(6); ShakeScreen();
		Gra_InvertView();
		sound(6); ShakeScreen();
		Gra_InvertView();
		sound(6); ShakeScreen();
	}
	RST_MSK(Party.mItems, ST_SKULL);
}

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

/*C_07AE*/CMD_Use() {
	register int si;
	char bp_0e[12];

	u4_puts("Use which item:\n");
	u4_gets(bp_0e, 11);
	Gra_CR();
	for(si = 0; UseItems[si]._00[0]; si++) {
		if(strnicmp(UseItems[si]._00, bp_0e, 12) == 0) {
			(*(UseItems[si]._02))();
			return;
		}
	}
	u4_puts("Not a Usable item!\n");
}
