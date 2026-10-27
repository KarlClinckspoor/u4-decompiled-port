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

/*
 * Something was found: Honor +5, and remember the time (moves & 0xf0) so that the
 * reagents cannot be found again before a while (see FindMandrake).
 */
/*C_8D4B*/SearchFound()
{
	u4_puts(/*D_2798*/"You find...\n");
	karma_inc(&(Party._honor), 5);
	Party._lastFound = Party._moves & 0xf0;
}

/*
 * Add 2 to 9 of reagent reagent (at most 99).
 */
/*C_8D6D*/AddReagent(reagent)
int16 reagent;
{
	Party._reagents[reagent] += U4_RND1(7) + 2;
	if(Party._reagents[reagent] > 99) {
		Party._reagents[reagent] = 99;
		u4_puts(/*D_27B5*/"Dropped some!\n");
		sound(SND_BEEP);
	}
}

/*
 * Mandrake root: only when both moons are new, and not twice in the same period of
 * 16 moves.
 */
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

/*
 * Nightshade: same conditions as the mandrake.
 */
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

/*
 * The Bell of Courage (once).
 */
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

/*
 * The Silver Horn (once).
 */
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

/*
 * The ship's wheel (once).
 */
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

/*
 * The Skull of Mondain: only when both moons are new, and not if it was already
 * found or destroyed.
 */
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

/*
 * The black stone (bit 7 of mStones): only when both moons are new.
 */
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

/*
 * The white stone (bit 6 of mStones).
 */
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

/*
 * The Book of Truth (once).
 */
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

/*
 * The Candle of Love (once).
 */
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

/*
 * The telescope (in the Lycaeum): show the map of one of the 16 towns/castles, then
 * reload the current one.
 */
/*C_8FB1*/UseTelescope()
{
	int16 town;

	u4_puts(/*D_2897*/"You see a knob on the Telescope marked A-P\n");
	if((town = AskLetter(/*D_28C3*/"You Select:\x12\x12\b", 'A', 'P')) < 0)
		return;
	town -= 'A';
	if(Load(TownFiles[town], sizeof(tMap32x32), &(Area._map)) == -1)
		exit(3);
	ShowAreaMap(0);
	if(Load(TownFiles[Party._loc - 1], sizeof(tMap32x32), &(Area._map)) == -1)
		exit(3);
}

/*
 * Mystic armour, 8 of them: only when the party has none and all eight virtues
 * are elevated (karma 0).
 */
/*C_9027*/FindMysticArmour()
{
	if(
		Party._armors[ARM_MYSTIC] |
		Party._hones | Party._compa |
		Party._valor | Party._justi |
		Party._sacri | Party._honor |
		Party._spiri | Party._humil
	) {
		u4_puts(MsgNothingHere);
		return;
	}
	Party._armors[ARM_MYSTIC] = 8;
	SearchFound();
	u4_puts(/*D_28D2*/"Mystic Armour!\n");
	XP_inc(0, 400);
}

/*
 * Mystic weapons: same conditions as the mystic armour.
 */
/*C_9076*/FindMysticWeapons()
{
	if(
		Party._weapons[WPN_MYSTIC] |
		Party._hones | Party._compa |
		Party._valor | Party._justi |
		Party._sacri | Party._honor |
		Party._spiri | Party._humil
	) {
		u4_puts(MsgNothingHere);
		return;
	}
	Party._weapons[WPN_MYSTIC] = 8;
	SearchFound();
	u4_puts(/*D_28E2*/"Mystic Weapons!\n");
	XP_inc(0, 400);
}

/*where each virtue's rune is (location, bit in mRunes), in the order of the
 virtues*/
unsigned char /*D_2904*/RuneTowns[][2] = {
	{LOC_MOONGLOW,0x01},
	{LOC_BRITAIN,0x02},
	{LOC_JHELOM,0x04},
	{LOC_YEW,0x08},
	{LOC_MINOC,0x10},
	{LOC_TRINSIC,0x20},
	{LOC_LCB,0x40},
	{LOC_PAWS,0x80}
};

/*
 * The rune of the virtue of the current location (once).
 */
/*C_90C5*/FindRune()
{
	register int16 virtue;

	for(virtue = 7; virtue >= 0; virtue --)
		if(RuneTowns[virtue][0] == Party._loc)
			break;
	if(RuneTowns[virtue][1] & Party.mRunes) {
		u4_puts(MsgNothingHere);
		return;
	}
	Party.mRunes |= RuneTowns[virtue][1];
	SearchFound();
	u4_puts(/*D_28F3*/"The rune of ");
	u4_puts(Names[NAME_VIRTUES + virtue]);
	u4_puts(/*D_2900*/"!\n");
	XP_inc(0, 100);
}

typedef s_handler();
typedef s_handler *pS_handler;

/*what can be found where*/
struct tSearchSpot {
	unsigned char _00, _01, _02;/*location (0: outdoors), x, y*/
	pS_handler _03;/*what is found*/
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
	{LOC_LYCAEUM,0x06,0x06,FindBook},/*The Book of Truth!*/
	{LOC_COVE,0x16,0x01,FindCandle},/*The Candle of Love!*/
	{LOC_LYCAEUM,0x16,0x03,UseTelescope},/*use Telescope*/
	{LOC_EMPATH,0x16,0x04,FindMysticArmour},/*Mystic Armour!*/
	{LOC_SERPENT,0x08,0x0F,FindMysticWeapons},/*Mystic Weapons!*/
	{LOC_MOONGLOW,0x08,0x06,FindRune},/*rune of Honesty*/
	{LOC_BRITAIN,0x19,0x01,FindRune},/*rune of Compassion*/
	{LOC_JHELOM,0x1E,0x1E,FindRune},/*rune of Valor*/
	{LOC_YEW,0x0D,0x06,FindRune},/*rune of Justice*/
	{LOC_MINOC,0x1C,0x1E,FindRune},/*rune of Sacrifice*/
	{LOC_TRINSIC,0x02,0x1D,FindRune},/*rune of Honor*/
	{LOC_LCB,0x11,0x08,FindRune},/*rune of Spirituality*/
	{LOC_PAWS,0x1D,0x1D,FindRune},/*rune of Humility*/
	{0x00,0x00,0x00,0}
};

/*
 * (S)earch: look for the party's location and position in SearchSpots (not
 * possible while flying the balloon).
 */
/*C_913A*/CMD_Search()
{
	register struct tSearchSpot *spot;

	u4_puts(/*D_2914*/"Search...\n");
	if(CurMode <= MOD_BUILDING && Party._fly_light != 0) {
		w_DriftOnly();
		return;
	}
	for(spot = SearchSpots; spot->_03; spot ++) {
		if(spot->_00 == Party._loc && spot->_01 == Party._x && spot->_02 == Party._y)
			break;
	}
	if(spot->_03 == 0) {
		u4_puts(MsgNothingHere);
	} else {
		(*spot->_03)();
	}
}
