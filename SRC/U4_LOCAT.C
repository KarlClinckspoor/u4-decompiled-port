/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*----------------------------------------
LOCATE
----------------------------------------*/

/*
 * (L)ocate position with a sextant: the latitude/longitude are given as letters,
 * the high and low nibbles of the party's y/x.
 */
/*C_755E*/CMD_Locate()
{
	u4_puts(/*D_22EA*/"Locate position with ");
	if(Party._sextants == 0) {
		w_What();
		return;
	}
	u4_puts(/*D_2300*/"sextant\n Latitude: ");
	u4_putc((Party._y >> 4) + 'A');
	u4_putc('\'');
	u4_putc((Party._y & 0xf) + 'A');
	u4_puts(/*D_2314*/"\"\nLongitude: ");
	u4_putc((Party._x >> 4) + 'A');
	u4_putc('\'');
	u4_putc((Party._x & 0xf) + 'A');
	u4_putc('"');
	Gra_CR();
}

/*----------------------------------------
READY WEAPON / WEAR ARMOUR
----------------------------------------*/

/*which classes may use each weapon/armour: bit 7 (0x80) for a mage, down to
 bit 0 for a shepherd (0x80 >> _class)*/
unsigned char /*D_2334*/ClassMasks[] = {
	/*weapons class masks*/
	0xFF,0xFF,0xFF,0xFF,0x7F,0x6F,0x6F,0x7E,0x7E,0xFF,0x2C,0x0C,0x2E,0x5E,0xD0,0xFF,
	/*D_2344*//*armor class masks*/
	0xFF,0xFF,0x7F,0x2C,0x2C,0x24,0x04,0xFF
};

/*
 * "A <class> may NOT use (a) <item>": item indexes ClassMasks, i.e. a weapon, or
 * 0x10 + an armour (the armour names follow the weapon names in Names[]);
 * article: print the "a" (for weapons).
 */
/*C_75DC*/w_MayNotUse(chara, item, article)
int chara;
int item;
int article;
{
	u4_puts(/*D_2322*/"\nA ");
	u4_puts(Names[NAME_CLASSES + (int)Party.chara[chara]._class]);
	u4_puts(/*D_2326*/" may NOT use ");
	if(article)
		u4_putc('a');
	Gra_CR();
	u4_puts(Names[NAME_WEAPONS + item]);
	Gra_CR();
	sound(SND_BEEP);
}

/*
 * (R)eady a weapon (in combat: for the active character). The weapon in hand goes
 * back to the party's stock (at most 99); hands (A) can always be used, otherwise
 * the class must be allowed (ClassMasks).
 */
/*C_7631*/CMD_Ready()
{
	register int wielder, weapon;
	int oldWeapon;

	if(CurMode >= MOD_COMBAT) {
		u4_puts(/*D_234C*/"Ready Weapon!\n");
		wielder = activeChara;
	} else {
		u4_puts(/*D_235B*/"Ready a weapon\n");
		if((wielder = AskChara(/*D_236B*/"for:\x12\x12\b")) < 0)
			return;
	}
	Gra_ClearStatus();
	ZtatsWeapons();
	txt_Y = 23; txt_X = 24;
	weapon = AskLetter(/*D_2373*/"Weapon:\x12\x12\b", 'A', 'P');
	Gra_ClearStatus();
	ZtatsClearTitle();
	dspl_Stats();
	if(CurMode >= MOD_COMBAT)
		Gra_InvertStatusLine(activeChara);
	if(weapon < 0)
		return;
	weapon -= 'A';
	if(weapon != 0 && Party._weapons[weapon] == 0) {
		w_NoneLeft();
		return;
	}
	if(weapon != 0) {
		if(((0x80 >> Party.chara[wielder]._class) & ClassMasks[weapon]) == 0) {
			w_MayNotUse(wielder, weapon, 1);
			return;
		}
	}
	if((oldWeapon = Party.chara[wielder]._weapon) != 0) {
		if(++Party._weapons[oldWeapon] > 99)
			Party._weapons[oldWeapon] = 99;
	}
	if(weapon != 0) 
		Party._weapons[weapon] --;
	Party.chara[wielder]._weapon = weapon;
	u4_puts(Names[NAME_WEAPONS + weapon]);
	Gra_CR();
}

/*
 * (W)ear armour: same as CMD_Ready for armours; skin (A) means no armour.
 */
/*C_7732*/CMD_Wear()
{
	register int chara, armor;
	int oldArmor;

	u4_puts(/*D_237E*/"Wear Armour\n");
	if((chara = AskChara(/*D_238B*/"for:\x12\x12\b")) < 0)
		return;
	Gra_ClearStatus();
	ZtatsArmour();
	txt_Y = 23; txt_X = 24;
	armor = AskLetter(/*D_2393*/"Armour:\x12\x12\b", 'A', 'H');
	Gra_ClearStatus();
	ZtatsClearTitle();
	dspl_Stats();
	if(CurMode >= MOD_COMBAT)
		Gra_InvertStatusLine(activeChara);
	if(armor < 0)
		return;
	armor -= 'A';
	if(armor != 0 && Party._armors[armor] == 0) {
		w_NoneLeft();
		return;
	}
	if(armor != 0) {
		if(((0x80 >> Party.chara[chara]._class) & ClassMasks[armor + 0x10]) == 0) {
			w_MayNotUse(chara, armor + 0x10, 0);
			return;
		}
	}
	if((oldArmor = Party.chara[chara]._armor) != 0) {
		if(++Party._armors[oldArmor] > 99)
			Party._armors[oldArmor] = 99;
	}
	if(armor != 0)
		Party._armors[armor] --;
	Party.chara[chara]._armor = armor;
	u4_puts(Names[NAME_ARMOURS + armor]);
	Gra_CR();
}
