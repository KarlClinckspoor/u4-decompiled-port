/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>
#include <stdlib.h>

/*
 * (Q)uit & save: save the party, and the monsters outdoors, or the monsters
 * and the map in a dungeon. Not possible in townes, castles or shrines.
 * Despite the name, the game goes on.
 */
int16 /*C_6F29*/CMD_Quit()
{
	u4_puts(/*D_21A2*/"Quit & Save...\n");
	u4_putl(Party._moves, 1, '0');
	u4_puts(/*D_21B2*/" moves\n");
	if(Party._loc) {
		if(Party._loc < LOC_DUNGEONS || Party._loc > LOC_ABYSS) {
			u4_puts(/*D_21BA*/"Not Here!\n");
			return;
		}
	}
	if(Save(/*D_21C5*/"PARTY.SAV", sizeof(struct tParty), &Party) == -1)
		exit(3);
	if(Party._loc == 0) {
		if(Save(/*D_21CF*/"MONSTERS.SAV", sizeof(struct tNPC), &(Area._npc)) == -1)
			exit(3);
	}
	if(Party._loc < LOC_DUNGEONS || Party._loc > LOC_ABYSS)
		return;
	if(Save(/*D_21DC*/"MONSTERS.SAV", sizeof(struct tNPC), &(Area._npc)) == -1)
		exit(3);
	if(Save(/*D_21E9*/"DNGMAP.SAV", sizeof(tMap8x8x8), &(Area._map)) == -1)
		exit(3);
}

char /*D_21F4*/MsgMustLead[] = ", You must lead!\n";

/*Exchange the characters *a and *b.*/
int16 /*C_6FF9*/SwapCharas(a, b)
struct tChara *a;
struct tChara *b;
{
	struct tChara tmp;
	memcpy(&tmp, a, sizeof(struct tChara));
	memcpy(a, b, sizeof(struct tChara));
	memcpy(b, &tmp, sizeof(struct tChara));
}

/*(N)ew order: exchange two party members; the Avatar (#1) must stay first.*/
int16 /*C_7034*/CMD_NewOrder()
{
	register struct tChara *pa, *pb;
	struct tChara dummy;/*not used*/
	int16 ia, ib;

	u4_puts(/*D_2206*/"New Order!\n");
	if((ia = AskChara(/*D_2212*/"Exchange #\x12\x12\b")) < 0)
		return;
	pa = &(Party.chara[ia]);
	if(pa == &(Party.chara[0])) {
		u4_puts(Party.chara[0]._name);
		u4_puts(MsgMustLead);
		return;
	}
	if((ib = AskChara(/*D_2220*/"    with #\x12\x12\b")) < 0)
		return;
	pb = &(Party.chara[ib]);
	if(pb == &(Party.chara[0])) {
		u4_puts(Party.chara[0]._name);
		u4_puts(MsgMustLead);
		return;
	}
	if(pa == pb) {
		w_What();
		return;
	}
	SwapCharas(pa, pb);
	Gra_ClearStatus();
	dspl_Stats();
}

/*(V)olume: toggle the sound effects.*/
int16 /*C_70AD*/CMD_Volume()
{
	u4_puts(/*D_222E*/"Volume O");
	u4_puts((SoundFlag ^= 1)?/*D_2237*/"n!\n":/*D_223B*/"ff!\n");
}
