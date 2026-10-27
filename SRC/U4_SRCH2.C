/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*----------------------------------------
MODULE SEARCH[dungeon]
----------------------------------------*/

char * /*D_2E04*/MsgHmmmNoEffect[] = {/*D_2DF0*/"\nHmmm--No Effect!\n"};

/*
 * Hurt character #chara for 100 * hundreds hit points, flashing its status line
 * and playing the "hurt" sound three times.
 */
/*C_B730*/HurtChara(chara, hundreds)
int chara;
int hundreds;
{
	Gra_InvertStatusLine(chara);
	sound(SND_HURT);
	sound(SND_HURT);
	sound(SND_HURT);
	Gra_InvertStatusLine(chara);
	hitChara(chara, 100 * hundreds);
}

/*
 * Add amount to the characteristic (strength, dexterity or intelligence) *stat,
 * capped at 50.
 */
/*C_B770*/AddStat(stat, amount)
int *stat;
int amount;
{
	*stat += amount;
	*stat = (*stat > 50)?50:*stat;
}

/*damage (in hundreds of hit points) of the magic ball of each dungeon*/
int /*D_2E6E*/BallDamage[] = {2,2,2,4,4,4,6,6};
/*characteristics raised by the magic ball of each dungeon: [0] strength,
  [1] dexterity, [2] intelligence; nonzero = +5*/
int /*D_2E7E*/BallStats[][8] = {
	{0,0,5,0,5,5,5,0},
	{0,5,0,5,5,0,5,0},
	{5,0,0,5,0,5,5,0}
};

/*
 * Search found a magic ball: a chosen character touches it, takes damage and gains
 * +5 in the characteristic(s) of this dungeon's ball. The ball is removed from the map.
 */
/*C_B795*/TouchBall()
{
	int chara;

	u4_puts(/*D_2E06*/"\nYou find a Magical Ball...\n");
	if((chara = AskChara(/*D_2E23*/"Who touches?\x12\x12\b")) < 0)
		return;
	if(!isCharaConscious(chara)) {
		u4_puts(/*D_2E33*/"\nDisabled!\n");
		return;
	}
	Area._map.x8x8x8[Party._z][Party._y][Party._x] = 0;
	HurtChara(chara, BallDamage[Party._loc - LOC_DUNGEONS]);
	/*-- which characteristic(s) ? --*/
	if(BallStats[0][Party._loc - LOC_DUNGEONS]) {
		AddStat(&(Party.chara[chara]._str), 5);
		u4_puts(/*D_2E3F*/"Strength + 5\n");
	}
	if(BallStats[1][Party._loc - LOC_DUNGEONS]) {
		AddStat(&(Party.chara[chara]._dex), 5);
		u4_puts(/*D_2E4D*/"Dexterity + 5\n");
	}
	if(BallStats[2][Party._loc - LOC_DUNGEONS]) {
		AddStat(&(Party.chara[chara]._int), 5);
		u4_puts(/*D_2E5C*/"Intelligence + 5\n");
	}
}

/*
 * Search found a fountain: a chosen character drinks from it. The kind of fountain is
 * the low nibble of the tile: 0 nothing, 1 heals fully, 2 hurts, 3 cures poison,
 * 4 poisons (and hurts).
 */
/*C_B863*/DrinkFountain()
{
	int chara;

	u4_puts(/*D_2EAE*/"You find a Fountain.\n");
	if((chara = AskChara(/*D_2EC4*/"Who drinks?\x12\x12\b")) < 0)
		return;
	if(!isCharaConscious(chara)) {
		u4_puts(/*D_2ED3*/"\nDisabled!\n");
		return;
	}
	switch(tile_cur & 0xf) {
		case 0:	u4_puts(MsgHmmmNoEffect[0]); break;
		case 1:
			if(Party.chara[chara]._HP[0] == Party.chara[chara]._HP[1]) {
				u4_puts(MsgHmmmNoEffect[0]);
			} else {
				u4_puts(/*D_2EDF*/"\nAhh-Refreshing!\n");
				Party.chara[chara]._HP[0] = Party.chara[chara]._HP[1];
			}
		break;
		case 2:
			u4_puts(/*D_2EF1*/"\nBleck--Nasty!\n");
			HurtChara(chara, 1);
		return;
		case 3:
			if(Party.chara[chara]._stat != 'P') {
				u4_puts(MsgHmmmNoEffect[0]);
			} else {
				Party.chara[chara]._stat = 'G';
				u4_puts(/*D_2F01*/"\nHmm--Delicious!\n");
			}
		break;
		case 4:
			if(Party.chara[chara]._stat == 'P') {
				u4_puts(MsgHmmmNoEffect[0]);
			} else {
				Party.chara[chara]._stat = 'P';
				u4_puts(/*D_2F13*/"\nArgh-Choke-Gasp!\n");
				HurtChara(chara, 1);
			};
		break;
	}
}

/*
 * Search found an altar with a stone: gives this dungeon's stone once (Hythloth and
 * the Abyss have none), with +5 honor and 200 experience to the first character.
 */
/*C_B93F*/FindStone()
{
	if(
		Party._loc == LOC_HYTHLOTH || Party._loc == LOC_ABYSS ||
		TST_MSK(Party.mStones, Party._loc - LOC_DUNGEONS)
	) {
		u4_puts(/*D_2F26*/"\nYou find Nothing!\n");
		return;
	}
	SET_MSK(Party.mStones, Party._loc - LOC_DUNGEONS);
	u4_puts(/*D_2F3A*/"\nYou find the ");
	u4_puts(StoneColors[Party._loc - LOC_DUNGEONS]);
	u4_puts(/*D_2F49*/" stone!\n");
	karma_inc(&(Party._honor), 5);
	XP_inc(0, 200);
}

/*
 * (S)earch in a dungeon: the high nibble of the current tile tells what is here
 * (0x70 magic ball, 0x90 fountain, 0xB0 altar with a stone).
 */
/*C_B9B2*/DNG_Search()
{
	u4_puts(/*D_2F52*/"Search...\n");
	switch(tile_cur & 0xf0) {
		case 0x70: TouchBall(); break;
		case 0x90: DrinkFountain(); break;
		case 0xB0: FindStone(); break;
		default: u4_puts(/*D_2F5D*/"\nYou find Nothing!\n");
	}
}
