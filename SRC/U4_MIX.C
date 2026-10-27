/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*reagent bits in a recipe: reagent r is 0x80 >> r*/
#define RG_ash        (0x80>>0)
#define RG_ginseng    (0x80>>1)
#define RG_garlic     (0x80>>2)
#define RG_spiderSilk (0x80>>3)
#define RG_bloodMoss  (0x80>>4)
#define RG_blackPearl (0x80>>5)
#define RG_nightshade (0x80>>6)
#define RG_mandrake   (0x80>>7)

/*the reagents of each spell's mixture*/
unsigned char /*D_277E*/SpellRecipes[] = {
	/*A*/RG_ginseng|RG_garlic,
	/*B*/RG_spiderSilk|RG_bloodMoss,
	/*C*/RG_ginseng|RG_garlic,
	/*D*/RG_ash|RG_garlic|RG_blackPearl,
	/*E*/RG_ash|RG_spiderSilk|RG_blackPearl,
	/*F*/RG_ash|RG_blackPearl,
	/*G*/RG_ash|RG_blackPearl|RG_mandrake,
	/*H*/RG_ginseng|RG_spiderSilk,
	/*I*/RG_blackPearl|RG_mandrake,
	/*J*/RG_blackPearl|RG_nightshade|RG_mandrake,
	/*K*/RG_blackPearl|RG_nightshade,
	/*L*/RG_ash,
	/*M*/RG_ash|RG_blackPearl,
	/*N*/RG_ash|RG_garlic|RG_mandrake,
	/*O*/RG_ash|RG_bloodMoss,
	/*P*/RG_ash|RG_ginseng|RG_garlic,
	/*Q*/RG_ash|RG_ginseng|RG_bloodMoss,
	/*R*/RG_ash|RG_ginseng|RG_garlic|RG_spiderSilk|RG_bloodMoss|RG_mandrake,
	/*S*/RG_ginseng|RG_spiderSilk,
	/*T*/RG_ash|RG_bloodMoss|RG_mandrake,
	/*U*/RG_ash|RG_garlic,
	/*V*/RG_nightshade|RG_mandrake,
	/*W*/RG_ash|RG_bloodMoss,
	/*X*/RG_ash|RG_spiderSilk|RG_bloodMoss,
	/*Y*/RG_spiderSilk|RG_bloodMoss,
	/*Z*/RG_spiderSilk|RG_bloodMoss
};

/*
 * (M)ix reagents: choose a spell, then add reagents one by one; Enter mixes them
 * (a success if they are exactly the spell's recipe, the reagents are used up
 * either way), Escape gives the reagents back and quits; the loop goes on with
 * another spell until Escape.
 * Note: choosing a reagent the party has none left of ends the mixture without
 * giving back the reagents already added: they are lost (probable original bug).
 */
/*C_8C08*/CMD_Mix()
{
	register int n;
	unsigned char saved[8];
	int  reagent, spellNo, cancel;
	char mask;

	cancel = 0;
	do {
		Gra_ClearStatus();
		ZtatsClearTitle();
		ZtatsMixtures();
		txt_Y = 23;
		txt_X = 24;
		u4_puts(/*D_270C*/"Mix Reagents\n");
		if((spellNo = AskLetter(/*D_271A*/"For Spell:\x12\x12\b", 'A', 'Z')) < 0)
			break;
		spellNo -= 'A';
		u4_puts(Names[NAME_SPELLS + spellNo]);
		Gra_CR();
		Gra_ClearStatus();
		ZtatsClearTitle();
		for(n = 7; n >= 0; n --)
			saved[n] = Party._reagents[n];
		mask = 0;
		while(1) {
			ZtatsReagents();
			txt_Y = 23;
			txt_X = 24;
			if((reagent = AskLetter(/*D_2728*/"Reagent:\x12\x12\b", 'A', 'H')) == -2) {
				for(n = 7; n >= 0; n --)
					Party._reagents[n] = saved[n];
				cancel = 1;
				break;
			}
			if(reagent == -1) {
				if(mask == 0) {
					u4_puts(/*D_2734*/"\nNothing mixed!\n");
				} else {
					u4_puts(/*D_2745*/"\nYou mix the Reagents, and...\n");
					reagent = 0;
					if(SpellRecipes[spellNo] == mask) {
						u4_puts(/*D_2764*/"Success!\n\n");
						Party._mixtures[spellNo] ++;
						if(Party._mixtures[spellNo] > 99)
							Party._mixtures[spellNo] = 99;
					} else {
						u4_puts(/*D_276F*/"It Fizzles!\n\n");
					}
				}
				break;
			}
			reagent -= 'A';
			if(Party._reagents[reagent] == 0) {
				w_NoneLeft();
				break;
			}
			mask |= 0x80 >> reagent;
			Party._reagents[reagent] --;
		}
	} while(cancel == 0);
	Gra_ClearStatus();
	ZtatsClearTitle();
	dspl_Stats();
}
