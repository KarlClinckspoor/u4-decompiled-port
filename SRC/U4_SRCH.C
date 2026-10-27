/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

/*Thanks to Fenyx4 for the
 comment on the rune names*/

#include "u4.h"

#include <stdlib.h>

char /*D_27A6*/MsgNothingHere[] = "Nothing Here!\n";

/*"You find..." & karma+=5*/
/*C_8D4B*/SearchFound()
{
	u4_puts(/*D_2798*/"You find...\n");
	karma_inc(&(Party._honor), 5);
	Party._lastFound = Party._moves & 0xf0;
}

/*get random amount of reagent (max 99)*/
/*C_8D6D*/AddReagent(bp04)
int bp04;
{
	Party._reagents[bp04] += U4_RND1(7) + 2;
	if(Party._reagents[bp04] > 99) {
		Party._reagents[bp04] = 99;
		u4_puts(/*D_27B5*/"Dropped some!\n");
		sound(1);
	}
}

/*C_8DAA*/FindMandrake()
{
	if((Party._trammel | Party._felucca) || (Party._moves & 0xf0) == Party._lastFound) {
		u4_puts(MsgNothingHere);
		return;
	}
	SearchFound();
	u4_puts(/*D_27C4*/"Mandrake Root!\n");
	AddReagent(7);
}

/*C_8DE0*/FindNightshade()
{
	if((Party._trammel | Party._felucca) || (Party._moves & 0xf0) == Party._lastFound) {
		u4_puts(MsgNothingHere);
		return;
	}
	SearchFound();
	u4_puts(/*D_27D4*/"Nightshade!\n");
	AddReagent(6);
}

/*C_8E16*/FindBell()
{
	if(TST_MSK(Party.mItems, ST_BELL)) {
		u4_puts(MsgNothingHere);
		return;
	}
	SET_MSK(Party.mItems, ST_BELL);
	SearchFound();
	u4_puts(/*D_27E1*/"The Bell of Courage!\n");
	XP_inc(0, 400);
}

/*C_8E46*/FindHorn()
{
	if(TST_MSK(Party.mItems, ST_HORN)) {
		u4_puts(MsgNothingHere);
		return;
	}
	SET_MSK(Party.mItems, ST_HORN);
	SearchFound();
	u4_puts(/*D_27F7*/"A Silver Horn!\n");
	XP_inc(0, 400);
}

/*C_8E77*/FindWheel()
{
	if(TST_MSK(Party.mItems, ST_WHEEL)) {
		u4_puts(MsgNothingHere);
		return;
	}
	SET_MSK(Party.mItems, ST_WHEEL);
	SearchFound();
	u4_puts(/*D_2807*/"The Wheel from the H.M.S. Cape!\n");
	XP_inc(0, 400);
}

/*C_8EA8*/FindSkull()
{
	if(
		TST_MSK(Party.mItems, ST_SKULL) ||
		(Party._trammel | Party._felucca) ||
		TST_MSK(Party.mItems, ST_CAST_SKULL)
	) {
		u4_puts(MsgNothingHere);
		return;
	}
	SET_MSK(Party.mItems, ST_SKULL);
	SearchFound();
	u4_puts(/*D_2828*/"The Skull of Mondain the Wizard!\n");
	XP_inc(0, 400);
}

/*C_8EE8*/FindBlackStone()
{
	if(
		TST_MSK(Party.mStones, 7) ||
		(Party._trammel | Party._felucca)
	) {
		u4_puts(MsgNothingHere);
		return;
	}
	SET_MSK(Party.mStones, 7);
	SearchFound();
	u4_puts(/*D_284A*/"The Black Stone!\n");
	XP_inc(0, 200);
}

/*C_8F21*/FindWhiteStone()
{
	if(TST_MSK(Party.mStones, 6)) {
		u4_puts(MsgNothingHere);
		return;
	}
	SET_MSK(Party.mStones, 6);
	SearchFound();
	u4_puts(/*D_285C*/"The White Stone!\n");
	XP_inc(0, 200);
}

/*C_8F51*/FindBook()
{
	if(TST_MSK(Party.mItems, ST_BOOK)) {
		u4_puts(MsgNothingHere);
		return;
	}
	SET_MSK(Party.mItems, ST_BOOK);
	SearchFound();
	u4_puts(/*D_286E*/"The Book of Truth!\n");
	XP_inc(0, 400);
}

/*C_8F81*/FindCandle()
{
	if(TST_MSK(Party.mItems, ST_CANDLE)) {
		u4_puts(MsgNothingHere);
		return;
	}
	SET_MSK(Party.mItems, ST_CANDLE);
	SearchFound();
	u4_puts(/*D_2882*/"The Candle of Love!\n");
	XP_inc(0, 400);
}

/*use telescope*/
/*C_8FB1*/UseTelescope()
{
	int bp_02;

	u4_puts(/*D_2897*/"You see a knob on the Telescope marked A-P\n");
	if((bp_02 = AskLetter(/*D_28C3*/"You Select:\x12\x12\b", 'A', 'P')) < 0)
		return;
	bp_02 -= 'A';
	if(Load(TownFiles[bp_02], sizeof(tMap32x32), &(Area._map)) == -1)
		exit(3);
	ShowTownMap(0);
	if(Load(TownFiles[Party._loc - 1], sizeof(tMap32x32), &(Area._map)) == -1)
		exit(3);
}

/*C_9027*/FindMysticArmour()
{
	if(
		Party._armors[7] |
		Party._hones | Party._compa |
		Party._valor | Party._justi |
		Party._sacri | Party._honor |
		Party._spiri | Party._humil
	) {
		u4_puts(MsgNothingHere);
		return;
	}
	Party._armors[7] = 8;
	SearchFound();
	u4_puts(/*D_28D2*/"Mystic Armour!\n");
	XP_inc(0, 400);
}

/*C_9076*/FindMysticWeapons()
{
	if(
		Party._weapons[15] |
		Party._hones | Party._compa |
		Party._valor | Party._justi |
		Party._sacri | Party._honor |
		Party._spiri | Party._humil
	) {
		u4_puts(MsgNothingHere);
		return;
	}
	Party._weapons[15] = 8;
	SearchFound();
	u4_puts(/*D_28E2*/"Mystic Weapons!\n");
	XP_inc(0, 400);
}

unsigned char /*D_2904*/RuneTowns[][2] = {
	{0x05,0x01},
	{0x06,0x02},
	{0x07,0x04},
	{0x08,0x08},
	{0x09,0x10},
	{0x0A,0x20},
	{0x01,0x40},
	{0x0D,0x80}
};

/*C_90C5*/FindRune()
{
	register int si;

	for(si = 7; si >= 0; si --)
		if(RuneTowns[si][0] == Party._loc)
			break;
	if(RuneTowns[si][1] & Party.mRunes) {
		u4_puts(MsgNothingHere);
		return;
	}
	Party.mRunes |= RuneTowns[si][1];
	SearchFound();
	u4_puts(/*D_28F3*/"The rune of ");
	u4_puts(Names[151 + si]);
	u4_puts(/*D_2900*/"!\n");
	XP_inc(0, 100);
}

typedef s_handler();
typedef s_handler *pS_handler;

struct tSearchSpot {
	unsigned char _00, _01, _02;
	pS_handler _03;
} /*D_2920*/SearchSpots[] = {
	{0x00,0xB6,0x36,FindMandrake},/*Mandrake root*/
	{0x00,0x64,0xA5,FindMandrake},/*Mandrake root*/
	{0x00,0x2E,0x95,FindNightshade},/*Nightshade*/
	{0x00,0xCD,0x2C,FindNightshade},/*Nightshade*/
	{0x00,0xB0,0xD0,FindBell},/*The Bell of Courage!*/
	{0x00,0x2D,0xAD,FindHorn},/*'A Silver Horn!'*/
	{0x00,0x60,0xD7,FindWheel},/*The Wheel from the H.M.S. Cape!*/
	{0x00,0xC5,0xF5,FindSkull},/*The Skull of Mondain the Wizard!*/
	{0x00,0xE0,0x85,FindBlackStone},/*The Black Stone!*/
	{0x00,0x40,0x50,FindWhiteStone},/*The White Stone!*/
	{0x02,0x06,0x06,FindBook},/*The Book of Truth!*/
	{0x10,0x16,0x01,FindCandle},/*The Candle of Love!*/
	{0x02,0x16,0x03,UseTelescope},/*use Telescope*/
	{0x03,0x16,0x04,FindMysticArmour},/*Mystic Armour!*/
	{0x04,0x08,0x0F,FindMysticWeapons},/*Mystic Weapons!*/
	{0x05,0x08,0x06,FindRune},/*rune of Honesty*/
	{0x06,0x19,0x01,FindRune},/*rune of Compassion*/
	{0x07,0x1E,0x1E,FindRune},/*rune of Valor*/
	{0x08,0x0D,0x06,FindRune},/*rune of Justice*/
	{0x09,0x1C,0x1E,FindRune},/*rune of Sacrifice*/
	{0x0A,0x02,0x1D,FindRune},/*rune of Honor*/
	{0x01,0x11,0x08,FindRune},/*rune of Spirituality*/
	{0x0D,0x1D,0x1D,FindRune},/*rune of Humility*/
	{0x00,0x00,0x00,0}
};

/*C_913A*/CMD_Search()
{
	register struct tSearchSpot *si;

	u4_puts(/*D_2914*/"Search...\n");
	if(CurMode <= MOD_BUILDING && Party._fly_light != 0) {
		w_DriftOnly();
		return;
	}
	for(si = SearchSpots; si->_03; si ++) {
		if(si->_00 == Party._loc && si->_01 == Party._x && si->_02 == Party._y)
			break;
	}
	if(si->_03 == 0) {
		u4_puts(MsgNothingHere);
	} else {
		(*si->_03)();
	}
}
