/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>

/*----------------------------------------*/

/*Hawkwind's advice: [karma level][virtue], the levels being karma < 20, < 40, < 60,
 < 99 and >= 99 (karma 0 means the virtue was already elevated)*/
char * /*D_3F0C*/SeerSayings[][8] = {
	{/* < 20*/
		/*D_316E*/"Thou art a thief and a scoundrel. Thou may not ever become an Avatar!",
		/*D_31B4*/"Thou art a cold and cruel brute.  Thou shouldst go to prison for thy crimes!",
		/*D_3201*/"Thou art a coward, thou dost flee from the hint of danger!",
		/*D_323C*/"Thou art an unjust wretch. Thou are a fulsome meddler!",
		/*D_3273*/"Thou art a self-serving Tufthunter. Thou deservest not my help, yet I grant it!",
		/*D_32C3*/"Thou art a cad and a bounder. Thy presence is an affront. Thou art low as a slug!",
		/*D_3315*/"Thy spirit is weak and feeble. Thou dost not strive for Perfection!",
		/*D_3359*/"Thou art proud and vain. All other virtue in thee is a loss!",
	}, {/* < 40*/
		/*D_3396*/"Thou art not an honest soul. Thou must live a more honest life to be an Avatar!",
		/*D_33E6*/"Thou dost kill where there is no need and give too little unto others!",
		/*D_342D*/"Thou dost not display a great deal of Valor. Thou dost flee before the need!",
		/*D_347A*/"Thou art cruel and unjust. In time thou will suffer for thy crimes!",
		/*D_34BE*/"Thou dost need to think more of the life of others and less of thy own!",
		/*D_3506*/"Thou dost not fight with honor but with malice and deceit!",
		/*D_3541*/"Thou dost not take time to care about thy inner being, a must to be an Avatar!",
		/*D_3590*/"Thou art too proud of thy little deeds. Humility is the root of all Virtue!",
	}, {/* < 60*/
		/*D_35DC*/"Thou hast made little progress on the paths of Honesty. Strive to prove thy worth!",
		/*D_362F*/"Thou hast not shown thy compassion well. Be more kind unto others!",
		/*D_3672*/"Thou art not yet a valiant warrior.  Fight to defeat evil and prove thyself!",
		/*D_36BF*/"Thou hast not proven thyself to be just. Strive to do justice unto all things!",
		/*D_370E*/"Thy sacrifice is small. Give of thy life's blood so that others may live.",
		/*D_3758*/"Thou dost need to show thyself to be more honorable.  The path lies before thee!",
		/*D_37A9*/"Strive to know and master more of thine inner being. Meditation lights the path!",
		/*D_37FA*/"Thy progress on this path is most uncertain. Without Humility thou art empty!",
	}, {/* < 99*/
		/*D_3848*/"Thou dost seem to be an honest soul.  Continued honesty will reward thee!",
		/*D_3892*/"Thou dost show thy compassion well.  Continued goodwill should be thy guide!",
		/*D_38DF*/"Thou art showing Valor in the face of danger. Strive to become yet more so!",
		/*D_392B*/"Thou dost seem fair and just. Strive to uphold Justice even more sternly!",
		/*D_3975*/"Thou art giving of thyself in some ways. Seek ye now to find yet more!",
		/*D_39BC*/"Thou dost seem to be Honorable in nature.  Seek to bring Honor upon others as well!",
		/*D_3A10*/"Thou art doing well on the path to inner sight continue to seek the inner light!",
		/*D_3A61*/"Thou dost seem a humble soul.  Thou art setting strong stones to build virtues upon!",
	}, {/* >= 99*/
		/*D_3AB6*/"Thou art truly an honest soul. Seek ye now to reach Elevation!",
		/*D_3AF5*/"Compassion is a virtue that thou hast shown well.  Seek ye now Elevation!",
		/*D_3B3F*/"Thou art a truly valiant warrior. Seek ye now Elevation in the virtue of valor!",
		/*D_3B8F*/"Thou art just and fair.  Seek ye now the Elevation!",
		/*D_3BC3*/"Thou art giving and good.  Thy self-sacrifice is great.  Seek now Elevation!",
		/*D_3C10*/"Thou hast proven thyself to be Honorable. Seek ye now for the Elevation!",
		/*D_3C59*/"Spirituality is in thy nature. Seek ye now the Elevation!",
		/*D_3C93*/"Thy Humility shines bright upon thy being. Seek ye now for Elevation!"
	}
};

static unsigned char D_913A;
static unsigned char D_913B, D_913C;
static uint16 D_913E;
/*The statics keep their address-like names: Microsoft C 4.00 lays out a module's
 uninitialised statics in the order of its symbol table, which depends on the names.
 Renaming them would move them; the readable names are macros.
 InnCost, InnBedX, InnBedY: price and bed position of the room taken at the inn.
 PubDrinks: number of ales ordered during this visit to the pub (at most 3).*/
#define InnCost D_913A
#define InnBedX D_913B
#define InnBedY D_913C
#define PubDrinks D_913E

/*
 * SHOP #10 - Hawkwind the Seer (Lord British's castle). Only speaks to a conscious
 * Avatar. For each virtue named (its first 4 letters are enough), tells how far the
 * Avatar is on that path; "none", "bye" or an empty line ends. Leaving gives
 * 3 Spirituality, once per 100 moves (shared with meditation).
 */
int16 /*C_C922*/SHP_hawkwind()
{
	int16 karmaLevel, virtueIdx;
	char input[16];

	if(!isCharaConscious(0)) {
		u4_puts(/*D_3CD9*/"The Seer says: I will speak only with ");
		u4_puts(Party.chara[0]._name);
		u4_puts(/*D_3D00*/".\nReturn when ");
		u4_puts(Party.chara[0]._name);
		u4_puts(/*D_3D0F*/" is revived!\n");
		return;
	}
	u4_puts(/*D_3D1D*/"\n\nWelcome, ");
	u4_puts(Party.chara[0]._name);
	u4_puts(/*D_3D29*/"\nI am Hawkwind, Seer of Souls. I see that which is within thee and drives thee to deeds of good or evil...\n");
	u_kbflush();
	u_kbread();
	input[0] = 0;
	while(1) {
		u4_puts(input[0] == 0?
			/*D_3D95*/"\nFor what path dost thou seek enlightenment?\n":
			/*D_3DC3*/"\nHawkwind asks: What other path seeks clarity?\n"
		);
		u4_gets(input, 15);
		if(
			input[0] == 0 ||
			strnicmp(input, /*D_3DF3*/"none", 16) == 0 ||
			strnicmp(input, /*D_3DF8*/"bye", 16) == 0
		) break;
		for(virtueIdx = 7; virtueIdx >= 0; virtueIdx --) {
			if(strnicmp(input, Names[NAME_VIRTUES + virtueIdx], 4) == 0)
				break;
		}
		if(virtueIdx == -1) {
			u4_puts(/*D_3DFC*/"\nHe says: That is not a subject for enlightenment.\n");
			continue;
		}
		Gra_CR();
		if(*(pKarmas[virtueIdx]) == 0) {
			u4_puts(/*D_3E30*/"He says:\nThou hast become a partial Avatar in that attribute. Thou need not my insights.\n");
			continue;
		}
		if(*(pKarmas[virtueIdx]) < 20) {
			karmaLevel = 0;
		} else  if(*(pKarmas[virtueIdx]) < 40) {
			karmaLevel = 1;
		} else  if(*(pKarmas[virtueIdx]) < 60) {
			karmaLevel = 2;
		} else  if(*(pKarmas[virtueIdx]) < 99) {
			karmaLevel = 3;
		} else {
			karmaLevel = 4;
		}
		Gra_CR();
		u4_puts(SeerSayings[karmaLevel][virtueIdx]);
		Gra_CR();
		if(karmaLevel == 4) {
			u4_puts(/*D_3E8A*/"\nGo to the Shrine and meditate for three Cycles!\n");
			u_kbflush();
			u_kbread();
		}
	}
	u4_puts(/*D_3EBC*/"\nHawkwind says: Fare thee well and may thou complete the Quest of the Avatar!\n");
	if(Party._moves / 100 != Party._lastMeditate) {
		karma_inc(&(Party._spiri), 3);
		Party._lastMeditate = Party._moves / 100;
	}
}

/*----------------------------------------*/

/*Shop tables are indexed by ShopId, which comes from a per-town table such as this one:
 [Party._loc - 1] gives the shop number + 1, or 0 for a town without such a shop.*/
unsigned char /*D_4170*/ReagentShopIds[16] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 3, 4, 0, 0};

char * /*D_4180*/ReagentShopNames[] = {
	/*D_3F5C*/"Magical Herbs",
	/*D_3F6A*/"Herbs and Spice",
	/*D_3F7A*/"The Magics",
	/*D_3F85*/"Magic Mentar"
};

char * /*D_4188*/ReagentShopKeepers[] = {
	/*D_3F92*/"Margot",
	/*D_3F99*/"Sasha",
	/*D_3F9F*/"Shiela",
	/*D_3FA6*/"Shannon"
};

/*price of each reagent (Sulfurous Ash .. Black Pearl) in each shop*/
unsigned char /*D_4190*/ReagentPrices[][6] = {
	{2, 5, 6, 3, 6, 9},
	{2, 4, 9, 6, 4, 8},
	{3, 4, 2, 9, 6, 7},
	{6, 7, 9, 9, 9, 1}
};

/*
 * SHOP #5 - reagents. The player chooses a reagent and an amount, then says how much
 * he pays: the shopkeeper is blind, so paying less than the price is accepted but
 * costs Honesty, Justice and Honor (4, or a third of the missing gold when 12 or more).
 * A paid purchase then gives 2 points in each of these, even after underpaying.
 * At most 99 of each reagent.
 */
int16 /*C_CAF6*/SHP_reagent()
{
	char reply;
	int16 amount, reagent, paid, shortBy;

	ShopId = ReagentShopIds[Party._loc - 1] - 1;
	u4_puts(/*D_3FAE*/"A blind woman turns to you and says: Welcome to ");
	u4_puts(ReagentShopNames[ShopId]);
	Gra_ClearStatus();
	ZtatsReagents();
	u4_puts(/*D_3FDF*/"\n\nI am ");
	u4_puts(ReagentShopKeepers[ShopId]);
	u4_puts(/*D_3FE7*/"\nAre you in need of Reagents?\x12\x12\b");
	reply = AskY_N();
	if(reply == 'Y') {
		u4_puts(/*D_4008*/"Very well,\n");
		do {
			u4_puts(/*D_4014*/"I have\n\
A-Sulfurous Ash\n\
B-Ginseng\n\
C-Garlic\n\
D-Spider Silk\n\
E-Blood Moss\n\
F-Black Pearl\n"
			);
			reagent = AskLetter(/*D_4068*/"Your Interest:\x12\x12\b", 'A', 'F');
			reagent -= 'A';
			if(reagent < 0)
				break;
			u4_puts(/*D_407A*/"\nVery well, we sell ");
			u4_puts(Names[NAME_REAGENTS + reagent]);
			u4_puts(/*D_408F*/" for ");
			u4_putl(ReagentPrices[ShopId][reagent], 1, '0');
			u4_puts(/*D_4095*/"gp. How many would you like?\x12\x12\x12\b\b");
			amount = AskInt(2);
			if(amount > 0) {
				u4_puts(/*D_40B7*/"\nVery good, that will be ");
				u4_putl((int32)amount * ReagentPrices[ShopId][reagent], 1, '0');
				u4_puts(/*D_40D1*/"gp. You pay:\x12\x12\x12\x12\b\b\b");
				paid = AskInt(3);
				if(paid <= 0) {
					if(paid < 0)
						sound(SND_BEEP);
				} else {
					if(paid < amount * ReagentPrices[ShopId][reagent]) {
						shortBy = amount * ReagentPrices[ShopId][reagent] - paid;
						shortBy = (shortBy < 12)?4:(shortBy / 3);
						karma_dec(&(Party._hones), shortBy);
						karma_dec(&(Party._justi), shortBy);
						karma_dec(&(Party._honor), shortBy);
					}
					if(paid > Party._gold) {
						u4_puts(/*D_40E5*/"It seems you have not the gold!\n");
					} else {
						karma_inc(&(Party._hones), 2);
						karma_inc(&(Party._justi), 2);
						karma_inc(&(Party._honor), 2);
						Party._gold -= paid; dspl_Gold();
						Party._reagents[reagent] += amount;
						if(Party._reagents[reagent] > 99)
							Party._reagents[reagent] = 99;
						ZtatsReagents();
						u4_puts(/*D_4106*/"\nVery good.\n");
					}
				}
			} else {
				u4_puts(/*D_4113*/"\nI see, then\n");
			}
			u4_puts(/*D_4121*/"Anything else?\x12\x12\b");
			reply = AskY_N();
		} while(reply == 'Y');
	}
	Gra_ClearStatus();
	ZtatsClearTitle();
	dspl_Stats();
	Gra_CR();
	u4_puts(ReagentShopKeepers[ShopId]);
	u4_puts(/*D_4133*/" says: Perhaps another time then.... and slowly turns away.\n");
}

/*----------------------------------------*/

char * /*D_46A2*/WeaponShopNames[] = {
	/*D_41A8*/"Windsor Weaponry",
	/*D_41B9*/"Willard's Weaponry",
	/*D_41CC*/"The Iron Works",
	/*D_41DB*/"Duelling Weapons",
	/*D_41EC*/"Hook's Arms",
	/*D_41F8*/"Village Arms"
};

char * /*D_46AE*/WeaponShopKeepers[] = {
	/*D_4205*/"Winston",
	/*D_420D*/"Willard",
	/*D_4215*/"Peter",
	/*D_421B*/"Jumar",
	/*D_4221*/"Hook",
	/*D_4226*/"Wendy"
};

/*the 4 weapons (WPN_*) each weapon shop sells*/
unsigned char /*D_46BA*/WeaponShopStock[][4] = {
	{1,  2,  3,  6},
	{5,  6,  8, 10},
	{4, 10, 11, 12},
	{4,  5,  6,  7},
	{8,  9, 13, 14},
	{2,  3,  7,  9}
};

/*price of each weapon (WPN_*); selling gives half of it*/
int16 /*D_46D2*/WeaponPrices[] = {
	   0,
	  20,
	   2,
	  25,
	 100,
	 225,
	 300,
	 250,
	 600,
	   5,
	 350,
	1500,
	2500,
	2000,
	5000,

	7000/*WPN_MYSTIC: never offered, but WeaponShopSell accepts 'P'*/
};

unsigned char /*D_46F2*/WeaponShopIds[16] = {0, 0, 0, 0, 0, 1, 2, 0, 3, 4, 0, 0, 0, 5, 6, 0};

/*sales pitch of each weapon, indexed by weapon - 1*/
char * /*D_4702*/WeaponPitches[] = {
	/*D_422C*/"We are the only staff makers in Britannia, yet sell them for only 20gp.\n",
	/*D_4275*/"We sell the most deadly of daggers, a bargain at only 2gp each.\n",
	/*D_42B6*/"Our slings are made from only the finest gut and leather, 'Tis yours for 25gp.\n",
	/*D_4306*/"These maces have a hardened shaft and a 5lb head fairly priced at 100gp.\n",
	/*D_4350*/"Notice the fine workmanship on this axe, you'll agree 225gp is a good price.\n",
	/*D_439E*/"The fine work on these swords will be the dread of thy foes, for 300gp.\n",
	/*D_43E7*/"Our bows are made of finest yew, and the arrows willow, a steal at 250gp.\n",
	/*D_4432*/"Crossbows made by Iolo the Bard are the finest in the world, yours for 600gp.\n",
	/*D_4481*/"Flasks of oil make great weapons and creates a wall of flame too. 5gp each.\n",
	/*D_44CE*/"A Halberd is a mighty weapon to attack over obstacles; a must and only 350gp.\n",
	/*D_451D*/"This magical axe can be thrown at thy enemy and will then return all for 1500gp.\n",
	/*D_456F*/"Magical swords such as these are rare indeed I will part with one for 2500gp.\n",
	/*D_45BE*/"A magical bow will keep thy enemies far away or dead! A must for 2000gp!\n",
	/*D_4608*/"This magic wand casts mighty blue bolts to strike down thy foes, 5000gp.\n"
};

/*
 * Buy count weapons of the given kind if the party has the gold (at most 99 kept).
 * Probable original bug: price * count is computed in 16 bits and wraps for the
 * expensive weapons (14 wands: 5000 * 14 = 70000 becomes 4464 gp).
 */
int16 /*C_CD1D*/WeaponShopDeal(weapon, count)
int16 weapon;
int16 count;
{
	if(WeaponPrices[weapon] * count > Party._gold) {
		u4_puts(/*D_4652*/"I fear you have not the funds, perhaps something else.\n");
		return;
	}
	Party._gold -= WeaponPrices[weapon] * count; dspl_Gold();
	Party._weapons[weapon] += count;
	if(Party._weapons[weapon] > 99)
		Party._weapons[weapon] = 99;
	ZtatsWeapons();
	Gra_CR();
	u4_puts(WeaponShopKeepers[ShopId]);
	u4_puts(/*D_468A*/" says: A fine choice!\n");
}

/*
 * Weapon shop, buying: list the shop's 4 weapons, ask which (letter of the weapon,
 * beeping until it is one of the shop's) and how many, until no more is wanted.
 * entry indexes the shop's stock; choice is the weapon, then the Y/N answer.
 */
int16 /*C_CD80*/WeaponShopBuy()
{
	int16 amount, entry, choice;

	Gra_CR();
	u4_puts(/*D_471E*/"\n\nVery Good!\n\n");
	do {
		u4_puts(/*D_472D*/"We have:\n");
		for(entry = 0; entry < 4; entry ++) {
			u4_putc(WeaponShopStock[ShopId][entry] + 'A');
			u4_putc('-');
			u4_puts(Names[NAME_WEAPONS + WeaponShopStock[ShopId][entry]]);
			u4_putc('s');
			Gra_CR();
		}


		do {
			choice = AskLetter(/*D_4737*/"Your Interest?\x12\x12\b", 'B', 'O');
			choice -= 'A';
			if(choice <= 0)
				break;
			for(entry = 0; entry < 4; entry ++) {
				if(WeaponShopStock[ShopId][entry] == choice)
					break;
			}
			if(entry == 4)
				sound(SND_BEEP);
		} while(entry == 4);
		if(choice < 0)
			break;
		if(WeaponPrices[choice] > Party._gold) {
			u4_puts(/*D_4749*/"You have not the funds for even one!\n");
		} else {
			Gra_CR();
			u4_puts(WeaponPitches[choice-1]);
			Gra_CR();
			u4_puts(/*D_476F*/"How many would you like?\x12\x12\x12\b\b");
			amount = AskInt(2);
			if(amount <= 0) {
				if(amount == 0)
					u4_puts(/*D_478D*/"\nToo bad.\n");
				else
					sound(SND_BEEP);
			} else {
				WeaponShopDeal(choice, amount);
			}
		}
		u4_puts(/*D_4798*/"\nAnything else?\x12\x12\b");
		choice = (char)AskY_N();
	} while(choice == 'Y');
	Gra_CR();
}

/*a += b, but not beyond c*/
#define clamp(a, b, c) if((a)+(b) > (c)) (a) = (c); else (a) += (b)

/*
 * Weapon shop, selling: any weapon the party owns (letter B to P, so even the mystic
 * weapons), for half its price; the party's gold is capped at 9999.
 * Probable original bug: when the first weapon asked for is not owned, the loop test
 * reads answer before anything was stored in it.
 */
int16 /*C_CEBE*/WeaponShopSell()
{
	int16 amount, weapon;
	char answer;

	u4_puts(/*D_47AB*/"\nExcellent! Which wouldst\n");
	weapon = -1;
	do {
		txt_Y = 23;
		txt_X = 24;
		weapon = AskLetter(/*D_47C6*/"You sell:\x12\x12\b", 'B', 'P');
		weapon -= 'A';
		Gra_CR();
		if(weapon <= 0)
			break;
		if(Party._weapons[weapon] == 0) {
			u4_puts(/*D_47D3*/"Thou dost not own that. What else might\n");
		} else {
			if(Party._weapons[weapon] > 1) {
				u4_puts(/*D_47FC*/"How many ");
				u4_puts(Names[NAME_WEAPONS + weapon]);
				u4_puts(/*D_4806*/"s would you wish to sell?\x12\x12\x12\b\b");
				amount = AskInt(2);
				if(amount <= 0) {
					if(amount == 0)
						u4_puts(/*D_4825*/"\nToo bad.\n");
					else
						sound(SND_BEEP);
					break;
				}
				if(Party._weapons[weapon] < amount) {
					u4_puts(/*D_4830*/"You don't have that many swine!\n");
					break;
				}
				u4_puts(/*D_4851*/"I will give you ");
				u4_putl(((int32)amount * WeaponPrices[weapon]) >> 1, 1, '0');
				u4_puts(/*D_4862*/"gp for ");
				if(amount > 1)
					u4_puts(/*D_486A*/"them.");
				else
					u4_puts(/*D_4870*/"it.");
			} else {
				amount = 1;
				u4_puts(/*D_4874*/"I will give you ");
				u4_putl((int32)WeaponPrices[weapon] >> 1, 1, '0');
				u4_puts(/*D_4885*/"gp for that ");
				u4_puts(Names[NAME_WEAPONS + weapon]);
			}
			u4_puts(/*D_4892*/"\nDeal?\x12\x12\b");
			answer = AskY_N();
			if(answer < 'N')
				break;
			if(answer == 'N') {
				u4_puts(/*D_489C*/"Hmmph. What else would\n");
			} else {
				Party._weapons[weapon] -= amount;
				clamp(Party._gold, (((int32)amount * WeaponPrices[weapon]) >> 1), 9999); dspl_Gold();
				ZtatsWeapons();
				u4_puts(/*D_48B4*/"\nFine! What else?\n");
			}
		}
	} while(answer == 'Y' || answer == 'N');
}

/*
 * SHOP #1 - weapons: welcome, then Buy or Sell (Space, Enter or Escape leave).
 */
int16 /*C_D085*/SHP_weapon()
{
	char choice;

	Gra_ClearStatus();
	ZtatsWeapons();
	ShopId = WeaponShopIds[Party._loc - 1] - 1;
	u4_puts(/*D_48C7*/"\n\n\n\nWelcome to ");
	u4_puts(WeaponShopNames[ShopId]);
	u4_puts(/*D_48D7*/"\n\n");
	u4_puts(WeaponShopKeepers[ShopId]);
	u4_puts(/*D_48DA*/" says: Welcome friend! Art thou here to Buy or Sell? ");
	do {
		switch(choice = u_kbread()) {
			case 'B': case 'b': choice = 'b'; break;
			case 'S': case 's': choice = 's'; break;
			case ' ': case '\n': case '\r': case 0x1b:
				choice = 0;
				Gra_CR();
			break;
			default:
				choice = -1;
				sound(SND_BEEP);
		}
	} while(choice == -1);
	if(choice) {
		u4_putc(choice);
		Gra_CR();
		if(choice == 'b')
			WeaponShopBuy();
		else
			WeaponShopSell();
	}
	ZtatsClearTitle();
	Gra_ClearStatus();
	dspl_Stats();
	Gra_CR();
	u4_puts(WeaponShopKeepers[ShopId]);
	u4_puts(/*D_4910*/" says: Fare thee well!\n");
}

/*----------------------------------------*/

char * /*D_4BAE*/ArmourShopNames[] = {
	/*D_4928*/"Windsor Armour",
	/*D_4937*/"Valiant's Armour",
	/*D_4948*/"Duelling Armour",
	/*D_4958*/"Light Armour",
	/*D_4965*/"Basic Armour"
};

char * /*D_4BB8*/ArmourShopKeepers[] = {
	/*D_4972*/"Winston",
	/*D_497A*/"Valiant",
	/*D_4982*/"Jean",
	/*D_4987*/"Pierre",
	/*D_498E*/"Limpy",

	/*D_4994*/"Big John"/*no sixth armour shop: unused (as the last row of ArmourShopStock)*/
};

/*the armours (ARM_*) each armour shop sells, 0 for an empty slot*/
unsigned char /*D_4BC4*/ArmourShopStock[][4] = {
	{1, 2, 3, 0},
	{3, 4, 5, 6},
	{1, 3, 5, 0},
	{1, 2, 0, 0},
	{1, 2, 3, 0},

	{0, 0, 0, 0}
};

/*price of each armour (ARM_*); selling gives half of it*/
int16 /*D_4BDC*/ArmourPrices[] = {
	   0,
	  50,
	 200,
	 600,
	2000,
	4000,
	7000,

	9000/*ARM_MYSTIC: never offered, but ArmourShopSell accepts 'H'*/
};

unsigned char /*D_4BEC*/ArmourShopIds[16] = {0, 0, 0, 0, 0, 1, 2, 0, 0, 3, 0, 0, 4, 5, 0, 0};

/*sales pitch of each armour, indexed by armour - 1*/
char * /*D_4BFC*/ArmourPitches[] = {
	/*D_499D*/"Cloth Armour is good for a tight budget, Fairly priced at 50gp.\n",
	/*D_49DE*/"Leather Armour is both supple and strong, and costs a mere 200gp.\nA Bargain!\n",
	/*D_4A2C*/"Chain Mail is the armour used by more warriors than all others. Ours costs 600gp.\n",
	/*D_4A7F*/"Full Plate armour is the ultimate in non-magical armour. Get yours for 2000gp.\n",
	/*D_4ACF*/"Magic Armour is rare and expensive.  This chain sells for 4000gp.\n",
	/*D_4B12*/"Magic Plate Armour is the best known protection. Only we have it. Cost: 7000gp.\n"
};

/*
 * Buy count armours of the given kind if the party has the gold (at most 99 kept).
 * Probable original bug: as in WeaponShopDeal, price * count wraps in 16 bits.
 */
int16 /*C_D16D*/ArmourShopDeal(armour, count)
int16 armour;
int16 count;
{
	if(ArmourPrices[armour] * count > Party._gold) {
		u4_puts(/*D_4B63*/"You don't have enough gold.\nMaybe something cheaper?\n");
		return;
	}
	Party._gold -= ArmourPrices[armour] * count; dspl_Gold();
	Party._armors[armour] += count;
	if(Party._armors[armour] > 99)
		Party._armors[armour] = 99;
	ZtatsArmour();
	Gra_CR();
	u4_puts(ArmourShopKeepers[ShopId]);
	u4_puts(/*D_4B99*/" says: Good choice!\n");
}

/*
 * Armour shop, buying: as WeaponShopBuy, with the shop's (up to 4) armours.
 */
int16 /*C_D1D0*/ArmourShopBuy()
{
	int16 amount, entry, choice;


	u4_puts(/*D_4C08*/"\n\n\nWell then,\n");
	do {
		u4_puts(/*D_4C17*/"We've got:\n");
		for(entry = 0; entry < 4; entry ++) {
			if(ArmourShopStock[ShopId][entry]) {
				u4_putc(ArmourShopStock[ShopId][entry] + 'A');
				u4_putc(' ');
				u4_puts(Names[NAME_ARMOURS + ArmourShopStock[ShopId][entry]]);
				Gra_CR();
			}
		}
		Gra_CR();
		do {
			choice = AskLetter(/*D_4C23*/"What'll it be?\x12\x12\b", 'B', 'G');
			choice -= 'A';
			if(choice <= 0)
				break;
			for(entry = 0; entry < 4; entry ++) {
				if(ArmourShopStock[ShopId][entry] == choice)
					break;
			}
			if(entry == 4)
				sound(SND_BEEP);
		} while(entry == 4);
		if(choice < 0)
			break;
		if(ArmourPrices[choice] > Party._gold) {
			u4_puts(/*D_4C35*/"What you try'n to pull? You can't pay!\n");
		} else {
			Gra_CR();
			u4_puts(ArmourPitches[choice-1]);
			Gra_CR();
			u4_puts(/*D_4C5D*/"How many would you like?\x12\x12\x12\b\b");
			amount = AskInt(2);
			if(amount <= 0) {

				u4_puts(/*D_4C7B*/"\nToo bad.\n");


			} else {
				ArmourShopDeal(choice, amount);
			}
		}
		u4_puts(/*D_4C86*/"\nAnything else?\x12\x12\b");
		choice = (char)AskY_N();
	} while(choice == 'Y');

}

/*
 * Armour shop, selling: any armour the party owns (letter B to H, so even the mystic
 * robes), for half its price; the party's gold is capped at 9999.
 * Probable original bug: as in WeaponShopSell, answer can be tested before being set.
 */
int16 /*C_D2F8*/ArmourShopSell()
{
	int16 amount, armour;
	char answer;

	u4_puts(/*D_4C99*/"\nWhat will\n");
	armour = -1;
	do {
		txt_Y = 23;
		txt_X = 24;
		armour = AskLetter(/*D_4CA5*/"You sell:\x12\x12\b", 'B', 'H');
		armour -= 'A';
		Gra_CR();
		if(armour <= 0)
			break;
		if(Party._armors[armour] == 0) {
			u4_puts(/*D_4CB2*/"Come on, you don't own any.\n");
		} else {
			if(Party._armors[armour] > 1) {
				u4_puts(/*D_4CCF*/"How many ");
				u4_puts(Names[NAME_ARMOURS + armour]);
				u4_puts(/*D_4CD9*/"s would you like to sell?\x12\x12\x12\b\b");
				amount = AskInt(2);
				if(amount <= 0) {

					u4_puts(/*D_4CF8*/"\nToo bad.\n");


					break;
				}
				if(Party._armors[armour] < amount) {
					u4_puts(/*D_4D03*/"You don't have that many swine!\n");
					break;
				}
				u4_puts(/*D_4D24*/"I'll give ya\n");
				u4_putl(((int32)amount * ArmourPrices[armour]) >> 1, 1, '0');
				u4_puts(/*D_4D32*/"gp for ");
				if(amount > 1)
					u4_puts(/*D_4D3A*/"them.");
				else
					u4_puts(/*D_4D40*/"it.");
			} else {
				amount = 1;
				u4_puts(/*D_4D44*/"I will give you\n");
				u4_putl((int32)ArmourPrices[armour] >> 1, 1, '0');
				u4_puts(/*D_4D55*/"gp for that ");
				u4_puts(Names[NAME_ARMOURS + armour]);
			}
			u4_puts(/*D_4D62*/"\nDeal?\x12\x12\b");
			answer = AskY_N();
			if(answer < 'N')
				break;
			if(answer == 'N') {
				u4_puts(/*D_4D6C*/"Harumph. What else would\n");
			} else {
				Party._armors[armour] -= amount;
				clamp(Party._gold, (((int32)amount * ArmourPrices[armour]) >> 1), 9999); dspl_Gold();
				ZtatsArmour();
				u4_puts(/*D_4D86*/"\nFine! What else?\n");
			}
		}
	} while(answer == 'Y' || answer == 'N');
}

/*
 * SHOP #2 - armour: welcome, then Buy or Sell (Space, Enter or Escape leave).
 */
int16 /*C_D4AE*/SHP_armour()
{
	char choice;

	Gra_ClearStatus();
	ZtatsArmour();
	ShopId = ArmourShopIds[Party._loc - 1] - 1;
	u4_puts(/*D_4D99*/"\n\n\n\nWelcome to\n");
	u4_puts(ArmourShopNames[ShopId]);
	u4_puts(/*D_4DA9*/"\n\n");
	u4_puts(ArmourShopKeepers[ShopId]);
	u4_puts(/*D_4DAC*/" says: Welcome friend! Want to Buy or Sell? ");
	do {
		switch(choice = u_kbread()) {
			case 'B': case 'b': choice = 'b'; break;
			case 'S': case 's': choice = 's'; break;
			case ' ': case '\n': case '\r': case 0x1b:
				choice = 0;
				Gra_CR();
			break;
			default:
				choice = -1;
				sound(SND_BEEP);
		}
	} while(choice == -1);
	if(choice) {
		u4_putc(choice);
		Gra_CR();
		if(choice == 'b')
			ArmourShopBuy();
		else
			ArmourShopSell();
	}
	ZtatsClearTitle();
	Gra_ClearStatus();
	dspl_Stats();
	Gra_CR();
	u4_puts(ArmourShopKeepers[ShopId]);
	u4_puts(/*D_4DD9*/" says: Good Bye.\n\0");/*the extra 0 reproduces the original data layout*/
}

/*----------------------------------------*/

/*
 * SHOP #9 - horses: one horse per party member, 100 gp each; the party then rides
 * (Party._tile becomes a horse).
 */
int16 /*C_D596*/SHP_horse()
{
	char answer;

	u4_puts(/*D_4DEC*/"\nWelcome friend! Can I interest thee in horses?\x12\x12\b");
	answer = AskY_N();
	if(answer != 'Y') {
		if(answer == 'N')
			u4_puts(/*D_4E1F*/"\nA shame, thou looks like thou could use a good horse!\n");
		return;
	}
	u4_puts(/*D_4E57*/"\nFor only ");
	u4_putl(Party._members, 1, '0');
	u4_puts(/*D_4E62*/"00g.p. Thou can have the best!  Wilt thou buy?\x12\x12\b");
	answer = AskY_N();
	if(answer != 'Y') {
		if(answer == 'N')
			u4_puts(/*D_4E94*/"\nA shame, thou looks like thou could use a good horse!\n");
		return;
	}
	if(100 * Party._members > Party._gold) {
		u4_puts(/*D_4ECC*/"\nIt seems thou hast not gold enough to pay!\n");
		return;
	}
	Party._gold -= 100 * Party._members; dspl_Gold();
	u4_puts(/*D_4EF9*/"\nHere, a better breed thou shalt not find ever!\n");
	Party._tile  = TIL_HORSE_W;
}

/*----------------------------------------*/

unsigned char /*D_5196*/GuildShopIds[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0};

/*torches, gems, keys, sextant: price, and number sold for that price*/
int16 /*D_51A6*/GuildPrices[] = {50, 60, 60, 900};
int16 /*D_51AE*/GuildQuantities[] = { 5,  5,  6,   1};

char * /*D_51B6*/GuildShopNames[] = {
	/*D_4F2A*/"Pirate's Guild",
	/*D_4F39*/"The Guild Shop"
};
char * /*D_51BA*/GuildShopKeepers[] = {
	/*D_4F48*/"One Eyed Willey",
	/*D_4F58*/"Long John Leary"
};

char * /*D_51BE*/GuildPitches[] = {
	/*D_4F68*/"I can give ya 5 long lasting Torches for a mere 50gp.\n",
	/*D_4F9F*/"I've got magical mapping Gems, 5 for only 60gp.\n",
	/*D_4FD0*/"Magical Keys 1 use each, a fair price at 60gp for 6.\n",
	/*D_5006*/"So...Ya want a Sextant...Well I gots one which I might part with fer 900 gold!\n"
};

/*
 * SHOP #8 - guild: torches, gems and keys, and a sextant, not on the list but
 * sold as item D. At most 99 of each.
 */
int16 /*C_D61E*/SHP_guild()
{
	int16 item;
	char reply;
	U16 *pQuantity;

	ShopId = GuildShopIds[Party._loc - 1] - 1;
	Gra_ClearStatus();
	ZtatsEquipment();
	u4_puts(/*D_5056*/"\n\n\n\nAvast ye mate! Shure ye wishes to buy from ol' ");
	u4_puts(GuildShopKeepers[ShopId]);
	u4_puts(/*D_508A*/"?\n\n\n");
	u4_puts(GuildShopKeepers[ShopId]);
	u4_puts(/*D_508F*/" says: Welcome to ");
	u4_puts(GuildShopNames[ShopId]);
	u4_puts(/*D_50A2*/"\nLike to see my goods?\x12\x12\b");
	while((reply = AskY_N()) == 'Y') {
		Gra_CR();
		u4_puts(GuildShopKeepers[ShopId]);
		u4_puts(/*D_50BC*/" says: Good Mate! Ya see I gots:\n\
A-Torches\n\
B-Magic Gems\n\
C-Magic Keys\n");
		item = AskLetter(/*D_5102*/"Wat'l it be?\x12\x12\b", 'A', 'D') - 'A';
		if(item < 0)
			break;
		Gra_CR();
		u4_puts(GuildPitches[item]);
		u4_puts(/*D_5112*/"\nWill ya buy?\x12\x12\b");
		reply = AskY_N();
		if(reply != 'Y') {
			if(reply == 'N')
				u4_puts(/*D_5123*/"\nHmmm...Grmbl...\n");
			break;
		}
		if(Party._gold < GuildPrices[item]) {
			Gra_ClearStatus();
			ZtatsClearTitle();
			dspl_Stats();
			u4_puts(/*D_5135*/"\nWhat? Can't pay! Buzz off swine!\n");
			return;
		}
		switch(item) {
			case 0: pQuantity = &(Party._torches); break;
			case 1: pQuantity = &(Party._gems); break;
			case 2: pQuantity = &(Party._keys); break;
			case 3: pQuantity = &(Party._sextants); break;
		}
		*pQuantity += GuildQuantities[item];
		if(*pQuantity > 99)
			*pQuantity = 99;
		Party._gold -= GuildPrices[item];
		u4_puts(/*D_5158*/"\nFine...fine...\n\n");
		dspl_Gold();
		ZtatsEquipment();
		Gra_CR();
		u4_puts(GuildShopKeepers[ShopId]);
		u4_puts(/*D_516A*/" says: See more?\x12\x12\b");
	}
	Gra_ClearStatus();
	ZtatsClearTitle();
	dspl_Stats();
	Gra_CR();
	u4_puts(GuildShopKeepers[ShopId]);
	u4_puts(/*D_517E*/" says:  See ya matie!\n");
}

/*----------------------------------------*/

unsigned char /*D_5484*/InnIds[16] = {0, 0, 0, 0, 1, 2, 3, 0, 4, 5, 6, 0, 0, 0, 7, 0};

/*position of the bed (in the town map) and price of the room of each inn*/
unsigned char /*D_5494*/InnBedXs[] = {0x1C, 0x1D, 0x0A, 0x02, 0x1D, 0x1C, 0x19, 0};
unsigned char /*D_549C*/InnBedYs[] = {0x06, 0x06, 0x1A, 0x06, 0x02, 0x0B, 0x17, 0};
unsigned char /*D_54A4*/InnPrices[] = {  20,   15,   10,   30,   15,    5,    1, 0};

char * /*D_54AC*/InnNames[] = {
	/*D_51C6*/"The Honest Inn",
	/*D_51D5*/"Britannia Manor",
	/*D_51E5*/"The Inn of Ends",
	/*D_51F5*/"Wayfarer's Inn",
	/*D_5204*/"Honorable Inn",
	/*D_5212*/"The Inn of the Spirits",
	/*D_5229*/"The Sleep Shop"
};

char * /*D_54BA*/InnKeepers[] = {
	/*D_5238*/"Scatu",
	/*D_523E*/"Jason",
	/*D_5244*/"Smirk",
	/*D_524A*/"Estro",
	/*D_5250*/"Zajac",
	/*D_5256*/"Tyrone",
	/*D_525D*/"Tymus"
};

char * /*D_54C8*/InnPitches[] = {
	/*D_5263*/"We have a room with 2 beds that rents for 20gp.",
	/*D_5293*/"We have a modest sized room with 1 bed for 15gp.",
	/*D_52C4*/"We have a very secure room of modest size and 1 bed for 10gp.",
	/*D_5302*/"We have three rooms available, a 1, 2 and 3 bed room for 30, 60 and 90gp each.",
	/*D_5351*/"We have a single bed room with a back door for 15g.",
	/*D_5385*/"Unfortunately, I have but only a very small room with 1 bed: worse yet, it is haunted!  If you do wish to stay it costs 5gp.",
	/*D_5402*/"All we have is that cot over there. But it is comfortable, and only 1gp."
};

/*
 * At night, the Avatar is attacked by rogues (a camp-style combat).
 */
int16 /*C_D7A8*/InnAmbush()
{
	int16 savedMode;

	u4_puts(/*D_544B*/"\nIn the middle of the night, while out for a stroll...\n");
	savedMode = CurMode;
	CurMode = MOD_COM_CAMP;
	CombatFoeTile = CombatFoe = TIL_ROGUE;
	InnCombat();
	CurMode = savedMode;
}

/*
 * A night at the inn: the party goes to the bed, the living members sleep and wake up
 * in good health (which also cures poison) with 100 to 198 more HP, magic points are
 * recovered. Then 1 time in 8 the Avatar is
 * ambushed by rogues; otherwise, in Skara Brae's inn (ShopId 5), 1 time in 4 the ghost
 * Isaac comes next to the bed.
 */
int16 /*C_D7D6*/InnNight()
{
	int16 chara;

	u_delay(1, 0);
	Party._x = InnBedX;
	Party._y = InnBedY;
	u_delay(1, 0);
	for(chara = Party._members - 1; chara >= 0; chara --) {
		if(isCharaAlive(chara))
			Party.chara[chara]._stat = 'S';
	}
	dspl_Stats();
	Party._tile = TIL_SLEEPING;
	u_delay(5, 0);
	Party._tile = TIL_AVATAR;
	for(chara = Party._members - 1; chara >= 0; chara --) {
		if(Party.chara[chara]._stat == 'S') {
			Party.chara[chara]._stat = 'G';
			HP_inc(chara, U4_RND3(50) * 2 + 100);
		}
	}
	MP_recover();
	dspl_Stats();
	if(isCharaAlive(0) && U4_RND1(7) == 0) {
		InnAmbush();
	} else if(ShopId == 5 && U4_RND1(3) == 0) {
		/*Isaac the ghost appears in skara brae*/
		Area._npc._tile[0] = Area._npc._gtile[0] = TIL_GHOST;
		Area._npc._x[0] = Party._x - 1;
		Area._npc._y[0] = Party._y;
		Area._npc._var[0] = 1;
		Area._npc._tlkidx[0] = 0x10;
	}
	u4_puts(/*D_54D6*/"\nMorning!\n");
}

/*the 3 rooms of the Wayfarer's Inn (ShopId 3): bed position and price*/
unsigned char /*D_5672*/InnRoomBedX[] = { 2, 2, 8, 0};
unsigned char /*D_5676*/InnRoomBedY[] = { 6, 2, 2, 0};
unsigned char /*D_567A*/InnRoomPrices[] = {30,60,90, 0};

/*
 * SHOP #7 - inn: not on horseback. Offers the room (or one of 3 rooms at the
 * Wayfarer's Inn), takes the gold and spends the night (InnNight).
 */
int16 /*C_D8DD*/SHP_inn()
{
	int16 room;
	char answer;

	if(Party._tile != TIL_AVATAR) {
		u4_puts(/*D_54E1*/"The Innkeeper says: Get that horse out of here!!!\n");
		return;
	}
	u4_puts(/*D_5514*/"The Innkeeper says: Welcome to ");
	ShopId = InnIds[Party._loc - 1] - 1;
	u4_puts(InnNames[ShopId]);
	u4_puts(/*D_5534*/"\n\nI am ");
	u4_puts(InnKeepers[ShopId]);
	u4_puts(/*D_553C*/".\n\nAre you in need of lodging?\x12\x12\b");
	answer = AskY_N();
	if(answer == 'Y') {
		Gra_CR();
		u4_puts(InnPitches[ShopId]);
		Gra_CR();
		if(ShopId == 3) {
			room = AskLetter(/*D_555E*/"1, 2 or 3 beds?\x12\x12\b", '1', '3');
			room -= '1';
			if(room < 0)
				return;
			InnBedX = InnRoomBedX[room];
			InnBedY = InnRoomBedY[room];
			InnCost = InnRoomPrices[room];
		} else {
			u4_puts(/*D_5571*/"\nTake it?\x12\x12\b");
			answer = AskY_N();
			if(answer != 'Y') {
				if(answer == 'N')
					u4_puts(/*D_557E*/"\nYou won't find a better deal in this towne!\n");
				return;
			}
			InnBedX = InnBedXs[ShopId];
			InnBedY = InnBedYs[ShopId];
			InnCost = InnPrices[ShopId];
		}
		if(InnCost > Party._gold) {
			u4_puts(/*D_55AC*/"\nIf you can't pay, you can't stay! Good Bye.\n");
			return;
		}
		Party._gold -= InnCost; dspl_Gold();
		u4_puts(/*D_55DA*/"\n\nVery good.  Have a pleasant night.\n");
		if(U4_RND1(3) == 0)
			u4_puts(/*D_5600*/"\nOh, and don't mind the strange noises, it's only rats!\n");
		InnNight();
	} else  {
		Gra_CR();
		u4_puts(InnKeepers[ShopId]);
		u4_puts(/*D_5639*/" says: Then you have come to the wrong place! Good Day.\n");
	}
}

/*----------------------------------------*/

unsigned char /*D_5788*/HealerIds[16] = {1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 9, 0, 0, 0, 0, 10};

char * /*D_5798*/HealerNames[] = {
	/*D_567E*/"The Royal Healer",
	/*D_568F*/"The Truth Healer",
	/*D_56A0*/"The Love Healer",
	/*D_56B0*/"Courage Healer",
	/*D_56BF*/"The Healer",
	/*D_56CA*/"Wound Healing",
	/*D_56D8*/"Heal and Health",
	/*D_56E8*/"Just Healing",
	/*D_56F5*/"The Mystic Heal",
	/*D_5705*/"The Healer Shop"
};

char * /*D_57AC*/HealerKeepers[] = {
	/*D_5715*/"Pendragon",
	/*D_571F*/"Starfire",
	/*D_5728*/"Salle'",
	/*D_572F*/"Windwalker",
	/*D_573A*/"Harmony",
	/*D_5742*/"Celest",
	/*D_5749*/"Triplet",
	/*D_5751*/"Justin",
	/*D_5758*/"Spiran",
	/*D_575F*/"Quat"
};

/*
 * Ask which party member needs help; returns its index, or a negative value.
 * Probable original bug: AskChara returns -2 for '0', but the callers only test for -1
 * and then use Party.chara[-2].
 */
int16 /*C_DA05*/HealerAskWho()
{
	register int16 chara;

	Gra_CR();
	u4_puts(HealerKeepers[ShopId]);
	u4_puts(/*D_5764*/" asks:\n");
	chara = AskChara(/*D_576C*/"Who is in need?\x12\x12\b");
	if(chara < 0)
		u4_puts(/*D_577F*/"No one?\n");
	return chara;
}

/*
 * Ask whether to pay price gold; if Yes, take it and return 1, else return 0.
 */
int16 /*C_DA3E*/HealerAskPay(price)
int16 price;
{
	char answer;

	u4_puts(/*D_57C0*/"\nWilt thou pay?\x12\x12\b");
	answer = AskY_N();
	if(answer == 'Y') {
		Party._gold -= price; dspl_Gold();
		return 1;
	}
	if(answer == 'N')
		u4_puts(/*D_57D3*/"\nThen I cannot aid thee.\n");

	return 0;
}

/*
 * The healer's spell on a party member: flash the view and its status line with
 * a magic sound.
 */
int16 /*C_DA79*/HealerSpell(chara)
int16 chara;
{
	Gra_InvertView();
	Gra_InvertStatusLine(chara);
	sound(SND_MAGIC, 0xc0);
	Gra_InvertStatusLine(chara);
	Gra_InvertView();
}

/*
 * Healer: cure poison for 100 gp, for free when the party cannot pay.
 */
int16 /*C_DAA2*/HealerCure()
{
	int16 chara;

	chara = HealerAskWho();
	if(chara == -1)
		return;
	if(Party.chara[chara]._stat != 'P') {
		u4_puts(/*D_57ED*/"Thou suffers not from Poison!\n");
		return;
	}
	u4_puts(/*D_580C*/"A curing will cost thee 100gp.\n");
	if(Party._gold < 100) {
		u4_puts(/*D_582C*/"\nI see by thy purse that thou hast not enough gold.");
		u_kbflush();
		u_delay(5, 1);
		if(u_kbhit())
			u_kbread();
		u4_puts(/*D_5860*/"\n\nI will cure thee for free, but give unto others whenever thou may!\n");
	} else {
		if(!HealerAskPay(100))
			return;
	}
	Party.chara[chara]._stat = 'G';
	HealerSpell(chara);
	dspl_Stats();
}

/*
 * Healer: restore a member's HP to the maximum for 200 gp.
 */
int16 /*C_DB29*/HealerHeal()
{
	int16 chara;

	chara = HealerAskWho();
	if(chara == -1)
		return;
	if(Party.chara[chara]._HP[0] == Party.chara[chara]._HP[1]) {
		u4_puts(/*D_58A6*/"Thou art already quite healthy!\n");
		return;
	}
	u4_puts(/*D_58C7*/"\nA healing will cost thee 200gp.\n");
	if(Party._gold < 200) {
		u4_puts(/*D_58E9*/"\nI see by thy purse that thou hast not enough gold. I cannot aid thee.\n");
		return;
	}
	if(HealerAskPay(200)) {
		Party.chara[chara]._HP[0] = Party.chara[chara]._HP[1];
		HealerSpell(chara);
		dspl_Stats();
	}
}

/*
 * Healer: bring a dead member back to life for 300 gp.
 */
int16 /*C_DB93*/HealerResurrect()
{
	int16 chara;

	chara = HealerAskWho();
	if(chara == -1)
		return;
	if(Party.chara[chara]._stat != 'D') {
		u4_puts(/*D_5931*/"Thou art not dead fool!\n");
		return;
	}
	u4_puts(/*D_594A*/"Resurrection will cost thee 300gp.\n");
	if(Party._gold < 300) {
		u4_puts(/*D_596E*/"\nI see by thy purse that thou hast not enough gold. I cannot aid thee.\n");
		return;
	}
	if(HealerAskPay(300)) {
		Party.chara[chara]._stat = 'G';
		HealerSpell(chara);
		dspl_Stats();
	}
}

/*
 * Healer: ask the Avatar to give 100 HP of blood: Yes gives 5 Sacrifice, No costs 5
 * (other answers neither). dummy is an unused local of the original.
 */
int16 /*C_DBF5*/HealerBlood()
{
	char answer;
	int16 dummy;

	u4_puts(/*D_59B6*/"Art thou willing to give 100pts of thy blood to aid others?\x12\x12\b");
	answer = AskY_N();
	if(answer == 'Y') {
		karma_inc(&(Party._sacri), 5);
		u4_puts(/*D_59F5*/"Thou art a great help.  We are in dire need!\n");
		Party.chara[0]._HP[0] -= 100;
		dspl_Stats();
		u_delay(1, 0);
	} else if(answer == 'N') {
		karma_dec(&(Party._sacri), 5);
		dspl_Stats();
	}
}

/*
 * SHOP #6 - healer: curing, healing and resurrection as long as wanted; then, if the
 * Avatar has at least 400 HP, the blood donation.
 */
int16 /*C_DC4D*/SHP_healer()
{
	int16 service;
	char answer;

	ShopId = HealerIds[Party._loc - 1] - 1;
	u4_puts(/*D_5A23*/"Welcome unto\n");
	u4_puts(HealerNames[ShopId]);
	Gra_CR();
	u4_puts(HealerKeepers[ShopId]);
	u4_puts(/*D_5A31*/" says: Peace and Joy be with you friend. Are you in need of help?\x12\x12\b");
	while((answer = AskY_N()) == 'Y') {
		Gra_CR();
		u4_puts(HealerKeepers[ShopId]);
		u4_puts(/*D_5A76*/" says: We can perform:\n\
A-Curing\n\
B-Healing\n\
C-Resurrection\n");
		service = AskLetter(/*D_5AB0*/"Your need:\x12\x12\b", 'A', 'C');
		if(service < 'A')
			break;
		switch((char)service) {
			case 'A': HealerCure(); break;
			case 'B': HealerHeal(); break;
			case 'C': HealerResurrect(); break;
		}
		Gra_CR();
		u4_puts(HealerKeepers[ShopId]);
		u4_puts(/*D_5ABE*/" asks: Do you need more help?\x12\x12\b");
	}
	if(Party.chara[0]._HP[0] >= 400)
		HealerBlood();
	dspl_Stats();
	Gra_CR();
	u4_puts(HealerKeepers[ShopId]);
	u4_puts(/*D_5ADF*/" says: May thy life be guarded by the powers of good.\n");
}

/*----------------------------------------*/

unsigned char /*D_5EE8*/PubIds[16] = {0, 0, 0, 0, 0, 1, 2, 0, 0, 3, 0, 0, 4, 5, 6, 0};

/*gold to pay for the answer about each topic of PubTopics*/
int16 /*D_5EF8*/PubTopicPrices[] = {20, 30, 10, 40, 99, 25};

char * /*D_5F04*/PubNames[] = {
	/*D_5B16*/"Jolly Spirits",
	/*D_5B24*/"The Bloody Pub",
	/*D_5B33*/"The Keg Tap",
	/*D_5B3F*/"Folley Tavern",
	/*D_5B4D*/"Captain Black Tavern",
	/*D_5B62*/"Axe 'n Ale"
};

char * /*D_5F10*/PubKeepers[] = {
	/*D_5B6D*/"Sam",
	/*D_5B71*/"Celestial",
	/*D_5B7B*/"Terran",
	/*D_5B82*/"Greg 'n Rob",
	/*D_5B8E*/"The Cap'n",
	/*D_5B98*/"Arron"
};

char * /*D_5F1C*/PubSpecialties[] = {
	/*D_5B9E*/"Lamb Chops",
	/*D_5BA9*/"Dragon Tartar",
	/*D_5BB7*/"Brown Beans",
	/*D_5BC3*/"Folley Filet",
	/*D_5BD0*/"Dog Meat Pie",
	/*D_5BDD*/"Green Granukit"
};

int16 /*D_5F28*/PubFoodPrices[] = {4,2,3,2,4,2};

/*what can be asked in a pub; "mandrake root" is the same as "mandrake". Each pub
 only answers about the topics from its own number (ShopId) on.*/
char * /*D_5F34*/PubTopics[] = {
	/*D_5BEC*/"black stone",
	/*D_5BF8*/"sextant",
	/*D_5C00*/"white stone",
	/*D_5C0C*/"mandrake",
	/*D_5C15*/"skull",
	/*D_5C1B*/"nightshade",
	/*D_5C26*/"mandrake root"
};

char * /*D_5F42*/PubAnswers[] = {
	/*D_5C34*/"Ah, the Black Stone.  Yes I've heard of it. But, the only one who knows where it lies is the wizard Merlin.\n",
	/*D_5CA1*/"For navigation a Sextant is vital...  Ask for item \"D\" in the Guild shops!\n",
	/*D_5CED*/"Hmmm, Now let me see... Yes, it was the old Hermit... Sloven!  He is tough to find, lives near Lock Lake I hear.\n",
	/*D_5D5F*/"The last person I knew that had any Mandrake was an old alchemist named Calumny.\n",
	/*D_5DB1*/"If thou must know of that evilest of all things... find the beggar Jude.  He is very very poor!\n",
	/*D_5E12*/"Of Nightshade I know but this... Seek out Virgil or thou shalt miss! Try in Trinsic!\n"
};

/*
 * Pub: buy plates of the specialty, each giving 1 food (100 units), as many as the
 * party can pay. Returns 1 to leave the pub (nothing ordered or not a single plate
 * afforded), 0 otherwise.
 */
int16 /*C_DD24*/PubFood()
{
	int16 plates, sold;

	u4_puts(/*D_5E68*/"Our specialty is ");
	u4_puts(PubSpecialties[ShopId]);
	u4_puts(/*D_5E7A*/", which costs ");
	u4_putl(PubFoodPrices[ShopId], 1, '0');
	u4_puts(/*D_5E89*/"gp.  How many plates would you like?\x12\x12\x12\b\b");
	plates = AskInt(2);
	if(plates <= 0) {
		if(plates < 0)
			sound(SND_BEEP);
		return 1;
	}
	sold = 0;
	while(plates) {
		if(Party._gold >= PubFoodPrices[ShopId]) {
			Party._gold -= PubFoodPrices[ShopId];
			Party._food = (Party._food + 100 > 999900L)?999900L:(Party._food + 100);
			dspl_Stats();
			plates --;
			sold ++;
		} else {
			if(sold == 0) {
				u4_puts(/*D_5EB3*/"\nYa cannot afford any!\n");
				return 1;
			}
			u4_puts(/*D_5ECB*/"\nYa can only afford ");
			u4_putl(sold, 1, '0');
			u4_puts(/*D_5EE0*/" plate");
			if(sold != 1)
				u4_putc('s'); 
			u4_putc('.');
			Gra_CR();
			break;
		}
	}

	return 0;
}

/*
 * Pub: buy an ale (2 gp, at most 3 per visit). Paying 3 gp or more lets you ask
 * about a topic; the barkeeper asks for more gold until the tip reaches its price
 * (money is everything paid). Returns 1 to leave the pub, 0 to order again.
 * Probable original bug: when no more gold is given (Enter), the test for the beep
 * checks money instead of extra, so the "could not hep ya" message is always shown.
 */
int16 /*C_DE35*/PubAle()
{
	int16 money;
	char ans;
	int16 topic;
	char question[16];
	int16 extra;

	if(PubDrinks == 3) {
		Gra_CR();
		u4_puts(PubKeepers[ShopId]);
		u4_puts(/*D_5F4E*/" says: Sorry, you seem to have too many.  Bye!\n");
		return 1;
	}
	PubDrinks ++;
	u4_puts(/*D_5F7E*/"\nHere's a mug of our best. That'll be 2gp. You pay?\x12\x12\x12\b\b");
	money = AskInt(2);
	if(money <= 0) {
		if(money < 0)
			sound(SND_BEEP);
		return 1;
	}
	if(money < 2) {
		u4_puts(/*D_5FB7*/"\nWon't pay, eh. Ya scum, be gone fore ey call the guards!\n");
		return 1;
	}
	if(Party._gold < money) {
		u4_puts(/*D_5FF2*/"\nIt seems that you have not the gold. Good Day!\n");
		return 1;
	}
	Party._gold -= money; dspl_Gold();
	if(money >= 3) {
		u4_puts(/*D_6023*/"\nWhat'd ya like to know friend?\n");
		u4_gets(question, 15);
		for(topic = 6; topic >= 0; topic --) {
			if(strnicmp(question, PubTopics[topic], 16) == 0)
			break;
		}
		if(topic == 6)
			topic = 3;
		if(topic < ShopId) {
			u4_puts(/*D_6044*/"\n'fraid I can't help ya there friend!\n");
		} else {
			while(PubTopicPrices[topic] > money) {
				u4_puts(/*D_606B*/"\nThat subject is a bit foggy, perhaps more gold will refresh my memory. You give:\x12\x12\x12\b\b");
				extra = AskInt(2);
				if(extra <= 0) {
					if(money < 0)
						sound(SND_BEEP);
					else
						u4_puts(/*D_60C2*/"\nSorry, I could not hep ya mate!\n");
					return 1;
				}
				if(Party._gold < extra) {
					u4_puts(/*D_60E4*/"\nYer don't have that mate!\n");
					u4_puts(/*D_6100*/"\nSorry, I could not hep ya mate!\n");
					return 1;
				}
				Party._gold -= extra; dspl_Gold();
				money += extra;
			}
			if(PubTopicPrices[topic] <= money) {
				Gra_CR();
				u4_puts(PubKeepers[ShopId]);
				u4_puts(/*D_6122*/" says: ");
				u4_puts(PubAnswers[topic]);
				Gra_CR();
			}
		}
	}
	u4_puts(/*D_612A*/"\nAnythin' else?\x12\x12\b");
	ans = AskY_N();
	if(ans != 'Y')
		return 1;
	return 0;
}

/*
 * SHOP #4 - pub: Food or Ale, as long as wanted.
 */
int16 /*C_DFAF*/SHP_pub()
{
	char choice;

	PubDrinks = 0;
	ShopId = PubIds[Party._loc - 1] - 1;
	Gra_CR();
	u4_puts(PubKeepers[ShopId]);
	u4_puts(/*D_613D*/" says:\nWelcome to ");
	u4_puts(PubNames[ShopId]);
	Gra_CR();
	do {
		u4_puts(PubKeepers[ShopId]);
		u4_puts(/*D_6150*/" says: What'll it be, Food er Ale?\x12\x12\b");
		{
C_E004:
			choice = u_kbread();
			u4_toupper(choice);
			if(choice == ' ' || choice == 0x1b || choice == '\r') {
				Gra_CR();
				return;
			}
			if(choice != 'F' && choice != 'A') {
				sound(SND_BEEP);
				goto C_E004;
			}
		}
		u4_putc(choice);
		Gra_CR();
		if(choice == 'F') {
			if(PubFood())
				break;
		} else {
			if(PubAle())
				break;
		}
		dspl_Stats();
		u4_puts(/*D_6176*/"Here ye arr. Somethin' else?\x12\x12\b");
		choice = AskY_N();
	} while(choice == 'Y');
	u4_puts(/*D_6196*/"See ya mate!\n");
}

/*----------------------------------------*/

unsigned char /*D_636C*/FoodShopIds[16] = {0, 0, 0, 0, 1, 2, 0, 3, 0, 0, 4, 0, 5, 0, 0, 0};

/*price of a pack of 25 rations in each shop*/
int16 /*D_637C*/FoodPrices[] = {25, 40, 35, 20, 30};

char * /*D_6386*/FoodShopNames[] = {
	/*D_61A4*/"The Sage Deli",
	/*D_61B2*/"Adventure Food",
	/*D_61C1*/"The Dry Goods",
	/*D_61CF*/"Food for Thought",
	/*D_61E0*/"The Market"
};

char * /*D_6390*/FoodShopKeepers[] = {
	/*D_61EB*/"Shaman",
	/*D_61F2*/"Windrick",
	/*D_61FB*/"Donnar",
	/*D_6102*/"Mintol",
	/*D_6109*/"Max"
};

/*
 * SHOP #3 - food: packs of 25 rations (food is kept in hundredths, max 999900).
 * Probable original bug: packs is a char, so amounts of 128 or more typed (up to 999)
 * wrap around: 200 packs is read as -56 ("Too bad"), 300 as 44.
 */
int16 /*C_E088*/SHP_food()
{
	char packs;
	char answer;

	ShopId = FoodShopIds[Party._loc - 1] - 1;
	u4_puts(/*D_620D*/"\n\n\n\nWelcome to ");
	u4_puts(FoodShopNames[ShopId]);
	u4_puts(/*D_621D*/"\n\n");
	u4_puts(FoodShopKeepers[ShopId]);
	u4_puts(/*D_6220*/" says: Good day, and Welcome friend.\n\nMay I interest you in some rations?\x12\x12\b");
	answer = AskY_N();
	if(answer != 'Y') {
		if(answer == 'N')
			u4_puts(/*D_626D*/"Goodbye, Come again!\n");
		return;
	}
	u4_puts(/*D_6283*/"We have the best adventure rations, 25 for only ");
	u4_putl(FoodPrices[ShopId], 1, '0');
	u4_puts(/*D_62B4*/"gp.");
	do {
		u4_puts(/*D_62B8*/"\nHow many packs of 25 would you like?\x12\x12\x12\x12\b\b\b");
		packs = AskInt(3);
		if(packs <= 0) {
			u4_puts(/*D_62E5*/"\nToo bad. Maybe next time.\n");
			answer = 'N';
			continue;
		}
		if(FoodPrices[ShopId] * packs > Party._gold) {
			if(FoodPrices[ShopId] > Party._gold) {
				u4_puts(/*D_6301*/"You cannot afford any!\n");
				break;
			}
			u4_puts(/*D_6319*/"You can only afford ");
			u4_putl((int32)Party._gold / (int32)FoodPrices[ShopId], 1, '0');
			u4_puts(/*D_632E*/" packs.\n");
			answer = 'Y';
			continue;
		}
		Party._food += 2500L * packs;
		if(Party._food > 999900L)
			Party._food = 999900L;
		Party._gold -= FoodPrices[ShopId] * packs;
		dspl_Stats();
		u4_puts(/*D_6337*/"\nThank you. Anything else?\x12\x12\b");
		answer = AskY_N();
	} while(answer == 'Y');
	u4_puts(/*D_6355*/"\nGoodbye. Come again!\n");
}
