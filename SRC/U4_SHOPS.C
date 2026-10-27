/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>

/*----------------------------------------*/

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
static unsigned D_913E;/*# of drinks in pub*/
/*The statics keep their address-like names: Microsoft C 4.00 lays out a module's
 uninitialised statics in the order of its symbol table, which depends on the names.
 Renaming them would move them; the readable names are macros.*/
#define InnCost D_913A
#define InnBedX D_913B
#define InnBedY D_913C
#define PubDrinks D_913E

/*SHOP #10 - The Seer*/
/*C_C922*/SHP_hawkwind()
{
	int bp_02, bp_04;
	char bp_14[16];

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
	bp_14[0] = 0;
	while(1) {
		u4_puts(bp_14[0] == 0?
			/*D_3D95*/"\nFor what path dost thou seek enlightenment?\n":
			/*D_3DC3*/"\nHawkwind asks: What other path seeks clarity?\n"
		);
		u4_gets(bp_14, 15);
		if(
			bp_14[0] == 0 ||
			strnicmp(bp_14, /*D_3DF3*/"none", 16) == 0 ||
			strnicmp(bp_14, /*D_3DF8*/"bye", 16) == 0
		) break;
		for(bp_04 = 7; bp_04 >= 0; bp_04 --) {
			if(strnicmp(bp_14, Names[151 + bp_04], 4) == 0)
				break;
		}
		if(bp_04 == -1) {
			u4_puts(/*D_3DFC*/"\nHe says: That is not a subject for enlightenment.\n");
			continue;
		}
		Gra_CR();
		if(*(pKarmas[bp_04]) == 0) {
			u4_puts(/*D_3E30*/"He says:\nThou hast become a partial Avatar in that attribute. Thou need not my insights.\n");
			continue;
		}
		if(*(pKarmas[bp_04]) < 20) {
			bp_02 = 0;
		} else  if(*(pKarmas[bp_04]) < 40) {
			bp_02 = 1;
		} else  if(*(pKarmas[bp_04]) < 60) {
			bp_02 = 2;
		} else  if(*(pKarmas[bp_04]) < 99) {
			bp_02 = 3;
		} else {
			bp_02 = 4;
		}
		Gra_CR();
		u4_puts(SeerSayings[bp_02][bp_04]);
		Gra_CR();
		if(bp_02 == 4) {
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

/*shops indexes*/
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

unsigned char /*D_4190*/ReagentPrices[][6] = {
	{2, 5, 6, 3, 6, 9},
	{2, 4, 9, 6, 4, 8},
	{3, 4, 2, 9, 6, 7},
	{6, 7, 9, 9, 9, 1}
};

/*SHOP #5 - reagents*/
/*C_CAF6*/SHP_reagent()
{
	char loc_C;
	int loc_A, loc_B, loc_D, loc_E;

	ShopId = ReagentShopIds[Party._loc - 1] - 1;
	u4_puts(/*D_3FAE*/"A blind woman turns to you and says: Welcome to ");
	u4_puts(ReagentShopNames[ShopId]);
	Gra_ClearStatus();
	ZtatsReagents();
	u4_puts(/*D_3FDF*/"\n\nI am ");
	u4_puts(ReagentShopKeepers[ShopId]);
	u4_puts(/*D_3FE7*/"\nAre you in need of Reagents?\x12\x12\b");
	loc_C = AskY_N();
	if(loc_C == 'Y') {
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
			loc_B = AskLetter(/*D_4068*/"Your Interest:\x12\x12\b", 'A', 'F');
			loc_B -= 'A';
			if(loc_B < 0)
				break;
			u4_puts(/*D_407A*/"\nVery well, we sell ");
			u4_puts(Names[93 + loc_B]);
			u4_puts(/*D_408F*/" for ");
			u4_putl(ReagentPrices[ShopId][loc_B], 1, '0');
			u4_puts(/*D_4095*/"gp. How many would you like?\x12\x12\x12\b\b");
			loc_A = AskInt(2);
			if(loc_A > 0) {
				u4_puts(/*D_40B7*/"\nVery good, that will be ");
				u4_putl((long)loc_A * ReagentPrices[ShopId][loc_B], 1, '0');
				u4_puts(/*D_40D1*/"gp. You pay:\x12\x12\x12\x12\b\b\b");
				loc_D = AskInt(3);
				if(loc_D <= 0) {
					if(loc_D < 0)
						sound(1);
				} else {
					if(loc_D < loc_A * ReagentPrices[ShopId][loc_B]) {
						loc_E = loc_A * ReagentPrices[ShopId][loc_B] - loc_D;
						loc_E = (loc_E < 12)?4:(loc_E / 3);
						karma_dec(&(Party._hones), loc_E);
						karma_dec(&(Party._justi), loc_E);
						karma_dec(&(Party._honor), loc_E);
					}
					if(loc_D > Party._gold) {
						u4_puts(/*D_40E5*/"It seems you have not the gold!\n");
					} else {
						karma_inc(&(Party._hones), 2);
						karma_inc(&(Party._justi), 2);
						karma_inc(&(Party._honor), 2);
						Party._gold -= loc_D; dspl_Gold();
						Party._reagents[loc_B] += loc_A;
						if(Party._reagents[loc_B] > 99)
							Party._reagents[loc_B] = 99;
						ZtatsReagents();
						u4_puts(/*D_4106*/"\nVery good.\n");
					}
				}
			} else {
				u4_puts(/*D_4113*/"\nI see, then\n");
			}
			u4_puts(/*D_4121*/"Anything else?\x12\x12\b");
			loc_C = AskY_N();
		} while(loc_C == 'Y');
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

unsigned char /*D_46BA*/WeaponShopStock[][4] = {
	{1,  2,  3,  6},
	{5,  6,  8, 10},
	{4, 10, 11, 12},
	{4,  5,  6,  7},
	{8,  9, 13, 14},
	{2,  3,  7,  9}
};

/*prices[weapons]*/
int /*D_46D2*/WeaponPrices[] = {
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

	7000/*what is this price for?*/
};

/*shops indexes*/
unsigned char /*D_46F2*/WeaponShopIds[16] = {0, 0, 0, 0, 0, 1, 2, 0, 3, 4, 0, 0, 0, 5, 6, 0};

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

/*finalize deal[weapon]*/
/*C_CD1D*/WeaponShopDeal(bp06, bp04)
int bp06;
int bp04;
{
	if(WeaponPrices[bp06] * bp04 > Party._gold) {
		u4_puts(/*D_4652*/"I fear you have not the funds, perhaps something else.\n");
		return;
	}
	Party._gold -= WeaponPrices[bp06] * bp04; dspl_Gold();
	Party._weapons[bp06] += bp04;
	if(Party._weapons[bp06] > 99)
		Party._weapons[bp06] = 99;
	ZtatsWeapons();
	Gra_CR();
	u4_puts(WeaponShopKeepers[ShopId]);
	u4_puts(/*D_468A*/" says: A fine choice!\n");
}

/*choose item[weapon]*/
/*C_CD80*/WeaponShopBuy()
{
	int loc_A, loc_B, loc_C;

	Gra_CR();
	u4_puts(/*D_471E*/"\n\nVery Good!\n\n");
	do {
		u4_puts(/*D_472D*/"We have:\n");
		for(loc_B = 0; loc_B < 4; loc_B ++) {
			u4_putc(WeaponShopStock[ShopId][loc_B] + 'A');
			u4_putc('-');
			u4_puts(Names[37 + WeaponShopStock[ShopId][loc_B]]);
			u4_putc('s');
			Gra_CR();
		}


		do {
			loc_C = AskLetter(/*D_4737*/"Your Interest?\x12\x12\b", 'B', 'O');
			loc_C -= 'A';
			if(loc_C <= 0)
				break;
			for(loc_B = 0; loc_B < 4; loc_B ++) {
				if(WeaponShopStock[ShopId][loc_B] == loc_C)
					break;
			}
			if(loc_B == 4)
				sound(1);
		} while(loc_B == 4);
		if(loc_C < 0)
			break;
		if(WeaponPrices[loc_C] > Party._gold) {
			u4_puts(/*D_4749*/"You have not the funds for even one!\n");
		} else {
			Gra_CR();
			u4_puts(WeaponPitches[loc_C-1]);
			Gra_CR();
			u4_puts(/*D_476F*/"How many would you like?\x12\x12\x12\b\b");
			loc_A = AskInt(2);
			if(loc_A <= 0) {
				if(loc_A == 0)
					u4_puts(/*D_478D*/"\nToo bad.\n");
				else
					sound(1);
			} else {
				WeaponShopDeal(loc_C, loc_A);
			}
		}
		u4_puts(/*D_4798*/"\nAnything else?\x12\x12\b");
		loc_C = (char)AskY_N();
	} while(loc_C == 'Y');
	Gra_CR();
}

#define clamp(a, b, c) if((a)+(b) > (c)) (a) = (c); else (a) += (b)

/*sell item[weapon]*/
/*C_CEBE*/WeaponShopSell()
{
	int loc_A, loc_B;
	char loc_C;

	u4_puts(/*D_47AB*/"\nExcellent! Which wouldst\n");
	loc_B = -1;
	do {
		txt_Y = 23;
		txt_X = 24;
		loc_B = AskLetter(/*D_47C6*/"You sell:\x12\x12\b", 'B', 'P');
		loc_B -= 'A';
		Gra_CR();
		if(loc_B <= 0)
			break;
		if(Party._weapons[loc_B] == 0) {
			u4_puts(/*D_47D3*/"Thou dost not own that. What else might\n");
		} else {
			if(Party._weapons[loc_B] > 1) {
				u4_puts(/*D_47FC*/"How many ");
				u4_puts(Names[37 + loc_B]);
				u4_puts(/*D_4806*/"s would you wish to sell?\x12\x12\x12\b\b");
				loc_A = AskInt(2);
				if(loc_A <= 0) {
					if(loc_A == 0)
						u4_puts(/*D_4825*/"\nToo bad.\n");
					else
						sound(1);
					break;
				}
				if(Party._weapons[loc_B] < loc_A) {
					u4_puts(/*D_4830*/"You don't have that many swine!\n");
					break;
				}
				u4_puts(/*D_4851*/"I will give you ");
				u4_putl(((long)loc_A * WeaponPrices[loc_B]) >> 1, 1, '0');
				u4_puts(/*D_4862*/"gp for ");
				if(loc_A > 1)
					u4_puts(/*D_486A*/"them.");
				else
					u4_puts(/*D_4870*/"it.");
			} else {
				loc_A = 1;
				u4_puts(/*D_4874*/"I will give you ");
				u4_putl((long)WeaponPrices[loc_B] >> 1, 1, '0');
				u4_puts(/*D_4885*/"gp for that ");
				u4_puts(Names[37 + loc_B]);
			}
			u4_puts(/*D_4892*/"\nDeal?\x12\x12\b");
			loc_C = AskY_N();
			if(loc_C < 'N')
				break;
			if(loc_C == 'N') {
				u4_puts(/*D_489C*/"Hmmph. What else would\n");
			} else {
				Party._weapons[loc_B] -= loc_A;
				clamp(Party._gold, (((long)loc_A * WeaponPrices[loc_B]) >> 1), 9999); dspl_Gold();
				ZtatsWeapons();
				u4_puts(/*D_48B4*/"\nFine! What else?\n");
			}
		}
	} while(loc_C == 'Y' || loc_C == 'N');
}

/*SHOP #1 - weapons*/
/*C_D085*/SHP_weapon()
{
	char bp_02;

	Gra_ClearStatus();
	ZtatsWeapons();
	ShopId = WeaponShopIds[Party._loc - 1] - 1;
	u4_puts(/*D_48C7*/"\n\n\n\nWelcome to ");
	u4_puts(WeaponShopNames[ShopId]);
	u4_puts(/*D_48D7*/"\n\n");
	u4_puts(WeaponShopKeepers[ShopId]);
	u4_puts(/*D_48DA*/" says: Welcome friend! Art thou here to Buy or Sell? ");
	do {
		switch(bp_02 = u_kbread()) {
			case 'B': case 'b': bp_02 = 'b'; break;
			case 'S': case 's': bp_02 = 's'; break;
			case ' ': case '\n': case '\r': case 0x1b:
				bp_02 = 0;
				Gra_CR();
			break;
			default:
				bp_02 = -1;
				sound(1);
		}
	} while(bp_02 == -1);
	if(bp_02) {
		u4_putc(bp_02);
		Gra_CR();
		if(bp_02 == 'b')
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

	/*D_4994*/"Big John"/*?!?who is that?!?*/
};

unsigned char /*D_4BC4*/ArmourShopStock[][4] = {
	{1, 2, 3, 0},
	{3, 4, 5, 6},
	{1, 3, 5, 0},
	{1, 2, 0, 0},
	{1, 2, 3, 0},

	{0, 0, 0, 0}
};

/*prices[armour]*/
int /*D_4BDC*/ArmourPrices[] = {
	   0,
	  50,
	 200,
	 600,
	2000,
	4000,
	7000,

	9000/*what is this price for?*/
};

/*shops indexes*/
unsigned char /*D_4BEC*/ArmourShopIds[16] = {0, 0, 0, 0, 0, 1, 2, 0, 0, 3, 0, 0, 4, 5, 0, 0};

char * /*D_4BFC*/ArmourPitches[] = {
	/*D_499D*/"Cloth Armour is good for a tight budget, Fairly priced at 50gp.\n",
	/*D_49DE*/"Leather Armour is both supple and strong, and costs a mere 200gp.\nA Bargain!\n",
	/*D_4A2C*/"Chain Mail is the armour used by more warriors than all others. Ours costs 600gp.\n",
	/*D_4A7F*/"Full Plate armour is the ultimate in non-magical armour. Get yours for 2000gp.\n",
	/*D_4ACF*/"Magic Armour is rare and expensive.  This chain sells for 4000gp.\n",
	/*D_4B12*/"Magic Plate Armour is the best known protection. Only we have it. Cost: 7000gp.\n"
};

/*finalize deal[armor]*/
/*C_D16D*/ArmourShopDeal(bp06, bp04)
int bp06;
int bp04;
{
	if(ArmourPrices[bp06] * bp04 > Party._gold) {
		u4_puts(/*D_4B63*/"You don't have enough gold.\nMaybe something cheaper?\n");
		return;
	}
	Party._gold -= ArmourPrices[bp06] * bp04; dspl_Gold();
	Party._armors[bp06] += bp04;
	if(Party._armors[bp06] > 99)
		Party._armors[bp06] = 99;
	ZtatsArmour();
	Gra_CR();
	u4_puts(ArmourShopKeepers[ShopId]);
	u4_puts(/*D_4B99*/" says: Good choice!\n");
}

/*choose item[armor]*/
/*C_D1D0*/ArmourShopBuy()
{
	int loc_A, loc_B, loc_C;


	u4_puts(/*D_4C08*/"\n\n\nWell then,\n");
	do {
		u4_puts(/*D_4C17*/"We've got:\n");
		for(loc_B = 0; loc_B < 4; loc_B ++) {
			if(ArmourShopStock[ShopId][loc_B]) {
				u4_putc(ArmourShopStock[ShopId][loc_B] + 'A');
				u4_putc(' ');
				u4_puts(Names[53 + ArmourShopStock[ShopId][loc_B]]);
				Gra_CR();
			}
		}
		Gra_CR();
		do {
			loc_C = AskLetter(/*D_4C23*/"What'll it be?\x12\x12\b", 'B', 'G');
			loc_C -= 'A';
			if(loc_C <= 0)
				break;
			for(loc_B = 0; loc_B < 4; loc_B ++) {
				if(ArmourShopStock[ShopId][loc_B] == loc_C)
					break;
			}
			if(loc_B == 4)
				sound(1);
		} while(loc_B == 4);
		if(loc_C < 0)
			break;
		if(ArmourPrices[loc_C] > Party._gold) {
			u4_puts(/*D_4C35*/"What you try'n to pull? You can't pay!\n");
		} else {
			Gra_CR();
			u4_puts(ArmourPitches[loc_C-1]);
			Gra_CR();
			u4_puts(/*D_4C5D*/"How many would you like?\x12\x12\x12\b\b");
			loc_A = AskInt(2);
			if(loc_A <= 0) {

				u4_puts(/*D_4C7B*/"\nToo bad.\n");


			} else {
				ArmourShopDeal(loc_C, loc_A);
			}
		}
		u4_puts(/*D_4C86*/"\nAnything else?\x12\x12\b");
		loc_C = (char)AskY_N();
	} while(loc_C == 'Y');

}

/*sell item[armor]*/
/*C_D2F8*/ArmourShopSell()
{
	int loc_A, loc_B;
	char loc_C;

	u4_puts(/*D_4C99*/"\nWhat will\n");
	loc_B = -1;
	do {
		txt_Y = 23;
		txt_X = 24;
		loc_B = AskLetter(/*D_4CA5*/"You sell:\x12\x12\b", 'B', 'H');
		loc_B -= 'A';
		Gra_CR();
		if(loc_B <= 0)
			break;
		if(Party._armors[loc_B] == 0) {
			u4_puts(/*D_4CB2*/"Come on, you don't own any.\n");
		} else {
			if(Party._armors[loc_B] > 1) {
				u4_puts(/*D_4CCF*/"How many ");
				u4_puts(Names[53 + loc_B]);
				u4_puts(/*D_4CD9*/"s would you like to sell?\x12\x12\x12\b\b");
				loc_A = AskInt(2);
				if(loc_A <= 0) {

					u4_puts(/*D_4CF8*/"\nToo bad.\n");


					break;
				}
				if(Party._armors[loc_B] < loc_A) {
					u4_puts(/*D_4D03*/"You don't have that many swine!\n");
					break;
				}
				u4_puts(/*D_4D24*/"I'll give ya\n");
				u4_putl(((long)loc_A * ArmourPrices[loc_B]) >> 1, 1, '0');
				u4_puts(/*D_4D32*/"gp for ");
				if(loc_A > 1)
					u4_puts(/*D_4D3A*/"them.");
				else
					u4_puts(/*D_4D40*/"it.");
			} else {
				loc_A = 1;
				u4_puts(/*D_4D44*/"I will give you\n");
				u4_putl((long)ArmourPrices[loc_B] >> 1, 1, '0');
				u4_puts(/*D_4D55*/"gp for that ");
				u4_puts(Names[53 + loc_B]);
			}
			u4_puts(/*D_4D62*/"\nDeal?\x12\x12\b");
			loc_C = AskY_N();
			if(loc_C < 'N')
				break;
			if(loc_C == 'N') {
				u4_puts(/*D_4D6C*/"Harumph. What else would\n");
			} else {
				Party._armors[loc_B] -= loc_A;
				clamp(Party._gold, (((long)loc_A * ArmourPrices[loc_B]) >> 1), 9999); dspl_Gold();
				ZtatsArmour();
				u4_puts(/*D_4D86*/"\nFine! What else?\n");
			}
		}
	} while(loc_C == 'Y' || loc_C == 'N');
}

/*SHOP #2 - armor*/
/*C_D4AE*/SHP_armor()
{
	char bp_02;

	Gra_ClearStatus();
	ZtatsArmour();
	ShopId = ArmourShopIds[Party._loc - 1] - 1;
	u4_puts(/*D_4D99*/"\n\n\n\nWelcome to\n");
	u4_puts(ArmourShopNames[ShopId]);
	u4_puts(/*D_4DA9*/"\n\n");
	u4_puts(ArmourShopKeepers[ShopId]);
	u4_puts(/*D_4DAC*/" says: Welcome friend! Want to Buy or Sell? ");
	do {
		switch(bp_02 = u_kbread()) {
			case 'B': case 'b': bp_02 = 'b'; break;
			case 'S': case 's': bp_02 = 's'; break;
			case ' ': case '\n': case '\r': case 0x1b:
				bp_02 = 0;
				Gra_CR();
			break;
			default:
				bp_02 = -1;
				sound(1);
		}
	} while(bp_02 == -1);
	if(bp_02) {
		u4_putc(bp_02);
		Gra_CR();
		if(bp_02 == 'b')
			ArmourShopBuy();
		else
			ArmourShopSell();
	}
	ZtatsClearTitle();
	Gra_ClearStatus();
	dspl_Stats();
	Gra_CR();
	u4_puts(ArmourShopKeepers[ShopId]);
	u4_puts(/*D_4DD9*/" says: Good Bye.\n\0");/*TODO:0 is for padding*/
}

/*----------------------------------------*/

/*SHOP #9 - horses*/
/*C_D596*/SHP_horse()
{
	char bp_02;

	u4_puts(/*D_4DEC*/"\nWelcome friend! Can I interest thee in horses?\x12\x12\b");
	bp_02 = AskY_N();
	if(bp_02 != 'Y') {
		if(bp_02 == 'N')
			u4_puts(/*D_4E1F*/"\nA shame, thou looks like thou could use a good horse!\n");
		return;
	}
	u4_puts(/*D_4E57*/"\nFor only ");
	u4_putl(Party._members, 1, '0');
	u4_puts(/*D_4E62*/"00g.p. Thou can have the best!  Wilt thou buy?\x12\x12\b");
	bp_02 = AskY_N();
	if(bp_02 != 'Y') {
		if(bp_02 == 'N')
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

/*shops indexes*/
unsigned char /*D_5196*/GuildShopIds[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0};

/*items prices*/
int /*D_51A6*/GuildPrices[] = {50, 60, 60, 900};
/*items quantities*/
int /*D_51AE*/GuildQuantities[] = { 5,  5,  6,   1};

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

/*SHOP #8 - guild*/
/*C_D61E*/SHP_guild()
{
	int loc_A;
	char loc_B;
	U16 *loc_C;

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
	while((loc_B = AskY_N()) == 'Y') {
		Gra_CR();
		u4_puts(GuildShopKeepers[ShopId]);
		u4_puts(/*D_50BC*/" says: Good Mate! Ya see I gots:\n\
A-Torches\n\
B-Magic Gems\n\
C-Magic Keys\n");
		loc_A = AskLetter(/*D_5102*/"Wat'l it be?\x12\x12\b", 'A', 'D') - 'A';
		if(loc_A < 0)
			break;
		Gra_CR();
		u4_puts(GuildPitches[loc_A]);
		u4_puts(/*D_5112*/"\nWill ya buy?\x12\x12\b");
		loc_B = AskY_N();
		if(loc_B != 'Y') {
			if(loc_B == 'N')
				u4_puts(/*D_5123*/"\nHmmm...Grmbl...\n");
			break;
		}
		if(Party._gold < GuildPrices[loc_A]) {
			Gra_ClearStatus();
			ZtatsClearTitle();
			dspl_Stats();
			u4_puts(/*D_5135*/"\nWhat? Can't pay! Buzz off swine!\n");
			return;
		}
		switch(loc_A) {
			case 0: loc_C = &(Party._torches); break;
			case 1: loc_C = &(Party._gems); break;
			case 2: loc_C = &(Party._keys); break;
			case 3: loc_C = &(Party._sextants); break;
		}
		*loc_C += GuildQuantities[loc_A];
		if(*loc_C > 99)
			*loc_C = 99;
		Party._gold -= GuildPrices[loc_A];
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

/*shops indexes*/
unsigned char /*D_5484*/InnIds[16] = {0, 0, 0, 0, 1, 2, 3, 0, 4, 5, 6, 0, 0, 0, 7, 0};

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

/*C_D7A8*/InnAmbush()
{
	int bp_02;

	u4_puts(/*D_544B*/"\nIn the middle of the night, while out for a stroll...\n");
	bp_02 = CurMode;
	CurMode = MOD_COM_CAMP;
	CombatFoeTile = CombatFoe = TIL_ROGUE;
	InnCombat();
	CurMode = bp_02;
}

/*C_D7D6*/InnNight()
{
	int bp_02;

	u_delay(1, 0);
	Party._x = InnBedX;
	Party._y = InnBedY;
	u_delay(1, 0);
	for(bp_02 = Party._members - 1; bp_02 >= 0; bp_02 --) {
		if(isCharaAlive(bp_02))
			Party.chara[bp_02]._stat = 'S';
	}
	dspl_Stats();
	Party._tile = TIL_SLEEPING;
	u_delay(5, 0);
	Party._tile = TIL_AVATAR;
	for(bp_02 = Party._members - 1; bp_02 >= 0; bp_02 --) {
		if(Party.chara[bp_02]._stat == 'S') {
			Party.chara[bp_02]._stat = 'G';
			HP_inc(bp_02, U4_RND3(50) * 2 + 100);
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

unsigned char /*D_5672*/InnRoomBedX[] = { 2, 2, 8, 0};
unsigned char /*D_5676*/InnRoomBedY[] = { 6, 2, 2, 0};
unsigned char /*D_567A*/InnRoomPrices[] = {30,60,90, 0};

/*SHOP #7 - inn*/
/*C_D8DD*/SHP_inn()
{
	int bp_02;
	char bp_04;

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
	bp_04 = AskY_N();
	if(bp_04 == 'Y') {
		Gra_CR();
		u4_puts(InnPitches[ShopId]);
		Gra_CR();
		if(ShopId == 3) {
			bp_02 = AskLetter(/*D_555E*/"1, 2 or 3 beds?\x12\x12\b", '1', '3');
			bp_02 -= '1';
			if(bp_02 < 0)
				return;
			InnBedX = InnRoomBedX[bp_02];
			InnBedY = InnRoomBedY[bp_02];
			InnCost = InnRoomPrices[bp_02];
		} else {
			u4_puts(/*D_5571*/"\nTake it?\x12\x12\b");
			bp_04 = AskY_N();
			if(bp_04 != 'Y') {
				if(bp_04 == 'N')
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

/*shops indexes*/
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

/*C_DA05*/HealerAskWho()
{
	register int si;

	Gra_CR();
	u4_puts(HealerKeepers[ShopId]);
	u4_puts(/*D_5764*/" asks:\n");
	si = AskChara(/*D_576C*/"Who is in need?\x12\x12\b");
	if(si < 0)
		u4_puts(/*D_577F*/"No one?\n");
	return si;
}

/*C_DA3E*/HealerAskPay(bp04)
int bp04;
{
	char bp_02;

	u4_puts(/*D_57C0*/"\nWilt thou pay?\x12\x12\b");
	bp_02 = AskY_N();
	if(bp_02 == 'Y') {
		Party._gold -= bp04; dspl_Gold();
		return 1;
	}
	if(bp_02 == 'N')
		u4_puts(/*D_57D3*/"\nThen I cannot aid thee.\n");

	return 0;
}

/*C_DA79*/HealerSpell(bp04)
int bp04;
{
	Gra_InvertView();
	Gra_InvertStatusLine(bp04);
	sound(9, 0xc0);
	Gra_InvertStatusLine(bp04);
	Gra_InvertView();
}

/*Curing*/
/*C_DAA2*/HealerCure()
{
	int bp_02;

	bp_02 = HealerAskWho();
	if(bp_02 == -1)
		return;
	if(Party.chara[bp_02]._stat != 'P') {
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
	Party.chara[bp_02]._stat = 'G';
	HealerSpell(bp_02);
	dspl_Stats();
}

/*Healing*/
/*C_DB29*/HealerHeal()
{
	int bp_02;

	bp_02 = HealerAskWho();
	if(bp_02 == -1)
		return;
	if(Party.chara[bp_02]._HP[0] == Party.chara[bp_02]._HP[1]) {
		u4_puts(/*D_58A6*/"Thou art already quite healthy!\n");
		return;
	}
	u4_puts(/*D_58C7*/"\nA healing will cost thee 200gp.\n");
	if(Party._gold < 200) {
		u4_puts(/*D_58E9*/"\nI see by thy purse that thou hast not enough gold. I cannot aid thee.\n");
		return;
	}
	if(HealerAskPay(200)) {
		Party.chara[bp_02]._HP[0] = Party.chara[bp_02]._HP[1];
		HealerSpell(bp_02);
		dspl_Stats();
	}
}

/*Resurrection*/
/*C_DB93*/HealerResurrect()
{
	int bp_02;

	bp_02 = HealerAskWho();
	if(bp_02 == -1)
		return;
	if(Party.chara[bp_02]._stat != 'D') {
		u4_puts(/*D_5931*/"Thou art not dead fool!\n");
		return;
	}
	u4_puts(/*D_594A*/"Resurrection will cost thee 300gp.\n");
	if(Party._gold < 300) {
		u4_puts(/*D_596E*/"\nI see by thy purse that thou hast not enough gold. I cannot aid thee.\n");
		return;
	}
	if(HealerAskPay(300)) {
		Party.chara[bp_02]._stat = 'G';
		HealerSpell(bp_02);
		dspl_Stats();
	}
}

/*blood donation*/
/*C_DBF5*/HealerBlood()
{
	char bp_04;
	int dummy;

	u4_puts(/*D_59B6*/"Art thou willing to give 100pts of thy blood to aid others?\x12\x12\b");
	bp_04 = AskY_N();
	if(bp_04 == 'Y') {
		karma_inc(&(Party._sacri), 5);
		u4_puts(/*D_59F5*/"Thou art a great help.  We are in dire need!\n");
		Party.chara[0]._HP[0] -= 100;
		dspl_Stats();
		u_delay(1, 0);
	} else if(bp_04 == 'N') {
		karma_dec(&(Party._sacri), 5);
		dspl_Stats();
	}
}

/*SHOP #6 - healer*/
/*C_DC4D*/SHP_healer()
{
	int bp_02;
	char bp_04;

	ShopId = HealerIds[Party._loc - 1] - 1;
	u4_puts(/*D_5A23*/"Welcome unto\n");
	u4_puts(HealerNames[ShopId]);
	Gra_CR();
	u4_puts(HealerKeepers[ShopId]);
	u4_puts(/*D_5A31*/" says: Peace and Joy be with you friend. Are you in need of help?\x12\x12\b");
	while((bp_04 = AskY_N()) == 'Y') {
		Gra_CR();
		u4_puts(HealerKeepers[ShopId]);
		u4_puts(/*D_5A76*/" says: We can perform:\n\
A-Curing\n\
B-Healing\n\
C-Resurrection\n");
		bp_02 = AskLetter(/*D_5AB0*/"Your need:\x12\x12\b", 'A', 'C');
		if(bp_02 < 'A')
			break;
		switch((char)bp_02) {
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

/*shops indexes*/
unsigned char /*D_5EE8*/PubIds[16] = {0, 0, 0, 0, 0, 1, 2, 0, 0, 3, 0, 0, 4, 5, 6, 0};

int /*D_5EF8*/PubTipPrices[] = {20, 30, 10, 40, 99, 25};

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

int /*D_5F28*/PubFoodPrices[] = {4,2,3,2,4,2};

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

/*pub food*/
/*C_DD24*/PubFood()
{
	int bp_02, bp_04;

	u4_puts(/*D_5E68*/"Our specialty is ");
	u4_puts(PubSpecialties[ShopId]);
	u4_puts(/*D_5E7A*/", which costs ");
	u4_putl(PubFoodPrices[ShopId], 1, '0');
	u4_puts(/*D_5E89*/"gp.  How many plates would you like?\x12\x12\x12\b\b");
	bp_02 = AskInt(2);
	if(bp_02 <= 0) {
		if(bp_02 < 0)
			sound(1);
		return 1;
	}
	bp_04 = 0;
	while(bp_02) {
		if(Party._gold >= PubFoodPrices[ShopId]) {
			Party._gold -= PubFoodPrices[ShopId];
			Party._food = (Party._food + 100 > 999900L)?999900L:(Party._food + 100);
			dspl_Stats();
			bp_02 --;
			bp_04 ++;
		} else {
			if(bp_04 == 0) {
				u4_puts(/*D_5EB3*/"\nYa cannot afford any!\n");
				return 1;
			}
			u4_puts(/*D_5ECB*/"\nYa can only afford ");
			u4_putl(bp_04, 1, '0');
			u4_puts(/*D_5EE0*/" plate");
			if(bp_04 != 1)
				u4_putc('s'); 
			u4_putc('.');
			Gra_CR();
			break;
		}
	}

	return 0;
}

/*pub ale*/
/*C_DE35*/PubAle()
{
	int loc_A;
	char loc_B;
	int loc_C;
	char loc_D[16];
	int loc_E;

	if(PubDrinks == 3) {
		Gra_CR();
		u4_puts(PubKeepers[ShopId]);
		u4_puts(/*D_5F4E*/" says: Sorry, you seem to have too many.  Bye!\n");
		return 1;
	}
	PubDrinks ++;
	u4_puts(/*D_5F7E*/"\nHere's a mug of our best. That'll be 2gp. You pay?\x12\x12\x12\b\b");
	loc_A = AskInt(2);
	if(loc_A <= 0) {
		if(loc_A < 0)
			sound(1);
		return 1;
	}
	if(loc_A < 2) {
		u4_puts(/*D_5FB7*/"\nWon't pay, eh. Ya scum, be gone fore ey call the guards!\n");
		return 1;
	}
	if(Party._gold < loc_A) {
		u4_puts(/*D_5FF2*/"\nIt seems that you have not the gold. Good Day!\n");
		return 1;
	}
	Party._gold -= loc_A; dspl_Gold();
	if(loc_A >= 3) {
		u4_puts(/*D_6023*/"\nWhat'd ya like to know friend?\n");
		u4_gets(loc_D, 15);
		for(loc_C = 6; loc_C >= 0; loc_C --) {
			if(strnicmp(loc_D, PubTopics[loc_C], 16) == 0)
			break;
		}
		if(loc_C == 6)
			loc_C = 3;
		if(loc_C < ShopId) {
			u4_puts(/*D_6044*/"\n'fraid I can't help ya there friend!\n");
		} else {
			while(PubTipPrices[loc_C] > loc_A) {
				u4_puts(/*D_606B*/"\nThat subject is a bit foggy, perhaps more gold will refresh my memory. You give:\x12\x12\x12\b\b");
				loc_E = AskInt(2);
				if(loc_E <= 0) {
					if(loc_A < 0)
						sound(1);
					else
						u4_puts(/*D_60C2*/"\nSorry, I could not hep ya mate!\n");
					return 1;
				}
				if(Party._gold < loc_E) {
					u4_puts(/*D_60E4*/"\nYer don't have that mate!\n");
					u4_puts(/*D_6100*/"\nSorry, I could not hep ya mate!\n");
					return 1;
				}
				Party._gold -= loc_E; dspl_Gold();
				loc_A += loc_E;
			}
			if(PubTipPrices[loc_C] <= loc_A) {
				Gra_CR();
				u4_puts(PubKeepers[ShopId]);
				u4_puts(/*D_6122*/" says: ");
				u4_puts(PubAnswers[loc_C]);
				Gra_CR();
			}
		}
	}
	u4_puts(/*D_612A*/"\nAnythin' else?\x12\x12\b");
	loc_B = AskY_N();
	if(loc_B != 'Y')
		return 1;
	return 0;
}

/*SHOP #4 - pub*/
/*C_DFAF*/SHP_pub()
{
	char bp_02;

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
			bp_02 = u_kbread();
			u4_toupper(bp_02);
			if(bp_02 == ' ' || bp_02 == 0x1b || bp_02 == '\r') {
				Gra_CR();
				return;
			}
			if(bp_02 != 'F' && bp_02 != 'A') {
				sound(1);
				goto C_E004;
			}
		}
		u4_putc(bp_02);
		Gra_CR();
		if(bp_02 == 'F') {
			if(PubFood())
				break;
		} else {
			if(PubAle())
				break;
		}
		dspl_Stats();
		u4_puts(/*D_6176*/"Here ye arr. Somethin' else?\x12\x12\b");
		bp_02 = AskY_N();
	} while(bp_02 == 'Y');
	u4_puts(/*D_6196*/"See ya mate!\n");
}

/*----------------------------------------*/

/*shops indexes*/
unsigned char /*D_636C*/FoodShopIds[16] = {0, 0, 0, 0, 1, 2, 0, 3, 0, 0, 4, 0, 5, 0, 0, 0};

int /*D_637C*/FoodPrices[] = {25, 40, 35, 20, 30};

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

/*SHOP #3 - food*/
/*C_E088*/SHP_food()
{
	char bp_02;
	char bp_04;

	ShopId = FoodShopIds[Party._loc - 1] - 1;
	u4_puts(/*D_620D*/"\n\n\n\nWelcome to ");
	u4_puts(FoodShopNames[ShopId]);
	u4_puts(/*D_621D*/"\n\n");
	u4_puts(FoodShopKeepers[ShopId]);
	u4_puts(/*D_6220*/" says: Good day, and Welcome friend.\n\nMay I interest you in some rations?\x12\x12\b");
	bp_04 = AskY_N();
	if(bp_04 != 'Y') {
		if(bp_04 == 'N')
			u4_puts(/*D_626D*/"Goodbye, Come again!\n");
		return;
	}
	u4_puts(/*D_6283*/"We have the best adventure rations, 25 for only ");
	u4_putl(FoodPrices[ShopId], 1, '0');
	u4_puts(/*D_62B4*/"gp.");
	do {
		u4_puts(/*D_62B8*/"\nHow many packs of 25 would you like?\x12\x12\x12\x12\b\b\b");
		bp_02 = AskInt(3);
		if(bp_02 <= 0) {
			u4_puts(/*D_62E5*/"\nToo bad. Maybe next time.\n");
			bp_04 = 'N';
			continue;
		}
		if(FoodPrices[ShopId] * bp_02 > Party._gold) {
			if(FoodPrices[ShopId] > Party._gold) {
				u4_puts(/*D_6301*/"You cannot afford any!\n");
				break;
			}
			u4_puts(/*D_6319*/"You can only afford ");
			u4_putl((long)Party._gold / (long)FoodPrices[ShopId], 1, '0');
			u4_puts(/*D_632E*/" packs.\n");
			bp_04 = 'Y';
			continue;
		}
		Party._food += 2500L * bp_02;
		if(Party._food > 999900L)
			Party._food = 999900L;
		Party._gold -= FoodPrices[ShopId] * bp_02;
		dspl_Stats();
		u4_puts(/*D_6337*/"\nThank you. Anything else?\x12\x12\b");
		bp_04 = AskY_N();
	} while(bp_04 == 'Y');
	u4_puts(/*D_6355*/"\nGoodbye. Come again!\n");
}
