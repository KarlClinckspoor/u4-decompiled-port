/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*spells m.p. cost*/
char /*D_208C*/SpellMPCost[] = {
	 5,/*Awaken*/
	15,/*Blink*/
	 5,/*Cure*/
	20,/*Dispel*/
	10,/*Energy*/
	15,/*Fireball*/
	40,/*Gate*/
	10,/*Heal*/
	20,/*Iceball*/
	30,/*Jinx*/
	25,/*Kill*/
	 5,/*Light*/
	 5,/*Magic Misl*/
	20,/*Negate*/
	 5,/*Open*/
	15,/*Protection*/
	20,/*Quickness*/
	45,/*Resurrect*/
	15,/*Sleep*/
	30,/*Tremor*/
	15,/*Undead*/
	15,/*View*/
	10,/*Winds*/
	15,/*X-it*/
	10,/*Y-up*/
	 5 /*Z-down*/
};

static int /*D_8CCC*/CurSpell;/*current spell*/

/*isUndead?*/
/*C_636D*/IsUndead(bp04)
int bp04;
{
	if(bp04 == TIL_GHOST || bp04 == TIL_PHANTOM || bp04 == TIL_SKELETON || bp04 == TIL_LICHE)
		return 1;
	return 0;
}

/*C_6399*/w_Failed()
{
	u4_puts(/*D_20A6*/"Failed!\n");
	hit_tile = 0;
	sound(8);
}

/*try "cast"*/
/*
	.consume m.p
	.check 'N'egate
*/
/*C_63B4*/SpellStart()
{
	Party.chara[activeChara]._MP -= SpellMPCost[CurSpell];
	sound(10, SpellMPCost[CurSpell]);
	Gra_InvertView(); sound(9, CurSpell+0x60); Gra_InvertView();
	if(spell_effect == 'N') {
		w_Failed();
		return 0;
	}
	return 1;
}

/*assert outdoors only*/
/*C_6409*/SpellOutdoorsOnly()
{
	if(CurMode != MOD_OUTDOORS) {
		u4_puts(/*D_20AF*/"Outdoors Only!\n");
		w_Failed();
		return 0;
	}
	return 1;
}

/*assert combat only*/
/*C_6428*/SpellCombatOnly()
{
	if(CurMode < MOD_COMBAT) {
		u4_puts(/*D_20BF*/"Combat Only!\n");
		w_Failed();
		return 0;
	}
	return 1;
}

/*assert dungeon only*/
/*C_6447*/SpellDungeonOnly()
{
	if(CurMode != MOD_DUNGEON) {
		u4_puts(/*D_20CD*/"Dungeon Only!\n");
		w_Failed();
		return 0;
	}
	return 1;
}
 

/*magic projectile ?*/
/*C_6466*/SpellMissile(bp04)
int bp04;
{
	register int loc_C, loc_D;
	int loc_A, loc_B;

	if(!SpellCombatOnly())
		return;
	AskDir(/*D_20DC*/"Dir: ", &loc_A, &loc_B);
	if(!(loc_A|loc_B))
		return;
	if(!SpellStart())
		return;
	hit_tile = bp04;
	hit_x = Combat._charaX[activeChara];
	hit_y = Combat._charaY[activeChara];
	loc_C = -1;
	do {
		if(!COM_CheckHitable(loc_A, loc_B))
			break;
		DrawCombat();
	} while((loc_C = COM_GetFighterId(hit_x, hit_y)) == -1);
	if(loc_C == -1) {
		w_Failed();
		return;
	}
	sound(6);
	switch(hit_tile) {
		case TIL_MISSILE: loc_D = U4_RND3(64) | 16; break;
		case TIL_MAGIC_BOLT: loc_D = U4_RND3(224) | 32; break;
		case TIL_HIT_FLASH: loc_D = U4_RND3(128) | 24; break;
		default: loc_D = 224 | 8;
	}
	hit_tile = 0;
	COM_DoDamage(loc_C, activeChara, loc_D);
}


/*C_6558*/SPL_Awaken()
{
	register int si;

	si = AskChara(/*D_20E2*/"Who:\x12\x12\b");
	if(si < 0)
		return;
	if(!SpellStart())
		return;
	if(Party.chara[si]._stat != 'S') {
		w_Failed();
		return;
	}
	Party.chara[si]._stat = 'G';
	if(CurMode >= MOD_COMBAT) {
		CharaAnimTile[si] = Fighters._chtile[si] = CharaTile(si);
	}
}

/*C_65AA*/SPL_Blink()
{
	int loc_A, loc_B;
	unsigned char loc_C, loc_D;

	if(Party._tile >= TIL_HORSE_W && Party._tile != TIL_BALLOON) {
		if(!SpellOutdoorsOnly())
			return;
		AskDir(/*D_20EA*/"Dir: ", &loc_A, &loc_B);
		if(!(loc_A|loc_B))
			return;
		if(!SpellStart())
			return;
		if((Party._x & Party._y) < 0xc0) {
			loc_C = WinPos.x;
			loc_D = WinPos.y;
			while(loc_C < 32 && loc_D < 32) {
				loc_C += loc_A;
				loc_D += loc_B;
			}
			do {
				loc_C -= loc_A;
				loc_D -= loc_B;
			} while((loc_C != WinPos.x || loc_D != WinPos.y) && !IsWalkable(Area._map.x32x32[loc_D][loc_C]));
			if(WinPos.x != loc_C || WinPos.y != loc_D) {
				Party._x += loc_C - WinPos.x;
				Party._y += loc_D - WinPos.y;
				LoadWorldWindow();
				return;
			}
		}
	}
	w_Failed();
}

/*C_669B*/SPL_Cure()
{
	register int si;

	if((si = AskChara(/*D_20F0*/"Who:\x12\x12\b")) < 0)
		return;
	if(!SpellStart())
		return;
	if(Party.chara[si]._stat != 'P') {
		w_Failed();
		return;
	}
	Party.chara[si]._stat = 'G';
}

/*C_66DA*/SPL_Dispel()
{
	int loc_A, loc_B;
	unsigned char *loc_C;

	if(CurMode == MOD_DUNGEON) {
		if(!SpellStart())
			return;
		loc_C = &(Area._map.x8x8x8[Party._z][DNG_Y_p(Party._y, Party._dir)][DNG_X_p(Party._x, Party._dir)]);
		if((*loc_C & 0xf0) != 0xa0) {
			w_Failed();
			return;
		}
		*loc_C = 0;
		return;
	}
	if(CurMode <= MOD_BUILDING && Party._fly_light) {
		w_Failed();
		return;
	}
	AskDir(/*D_20F8*/"Dir: ", &loc_A, &loc_B);
	if(!(loc_A|loc_B))
		return;
	if(!SpellStart())
		return;
	if(CurMode >= MOD_COMBAT) {
		if(Combat._charaY[activeChara] + loc_B >= 11 || Combat._charaX[activeChara] + loc_A >= 11) {
			w_Failed();
			return;
		}
		loc_C = &(Combat_MAP(Combat._charaY[activeChara]+loc_B, Combat._charaX[activeChara]+loc_A));
		if(*loc_C < TIL_POISON_FIELD || *loc_C > TIL_SLEEP_FIELD) {
			w_Failed();
			return;
		}
		*loc_C = Combat_MAP(Combat._charaY[activeChara], Combat._charaX[activeChara]);
		return;
	}
	if(CurMode == MOD_OUTDOORS) {
		loc_C = &(Area._map.x32x32[WinPos.y+loc_B][WinPos.x+loc_A]);
	} else {
		if(Party._y + loc_B >= 32 || Party._x + loc_A >= 32) {
			w_Failed();
			return;
		}
		loc_C = &(Area._map.x32x32[Party._y + loc_B][Party._x + loc_A]);
	}
	if(*loc_C < TIL_POISON_FIELD || *loc_C > TIL_SLEEP_FIELD) {
		w_Failed();
		return;
	}
	*loc_C = tile_cur;
}

/*C_6882*/SPL_Energy()
{
	unsigned char *loc_D;
	int loc_B;
	int loc_A, loc_C, loc_E;

	u4_puts(/*D_20FE*/"Energy type? ");
	loc_B = (unsigned char)u_kbread();
/*bad hack:I must force the redefinition of u4_islower
int order to get the compiler to provide
the original bytecode*/
#define u4_islower(c) ((c) >= (int)'a' && (c) <= (int)'z')
	u4_toupper(loc_B);
	switch(loc_B) {
		case 'P'/*oison*/:    loc_E = TIL_POISON_FIELD; break;
		case 'L'/*ightning*/: loc_E = TIL_ENERGY_FIELD; break;
		case 'F'/*ire*/:      loc_E = TIL_FIRE_FIELD; break;
		case 'S'/*leep*/:     loc_E = TIL_SLEEP_FIELD; break;
		default: loc_E = -1;
	}
	if(loc_E == -1) {
		w_Failed();
		return;
	}
	u4_putc(loc_B);
	Gra_CR();
	if(CurMode == MOD_DUNGEON) {
		if(!SpellStart())
			return;
		loc_D = &(Area._map.x8x8x8[Party._z][DNG_Y_p(Party._y, Party._dir)][DNG_X_p(Party._x, Party._dir)]);
		if(*loc_D)
			w_Failed();
		*loc_D = (loc_E&3) | 0xa0;
		return;
	}
	if(CurMode >= MOD_COMBAT) {
		AskDir(/*D_210C*/"Dir: ", &loc_A, &loc_C);
		if(!(loc_A|loc_C))
			return;
		if(!SpellStart())
			return;
		if(Combat._charaY[activeChara] + loc_C < 11 && Combat._charaX[activeChara] + loc_A < 11) {
			if(IsWalkable(*(loc_D = &(Combat_MAP(loc_C + Combat._charaY[activeChara], loc_A + Combat._charaX[activeChara]))))) {
				*loc_D = loc_E;
				return;
			}
		}
	}
	w_Failed();
}

/*C_69D7*/SPL_Fireball() {
	SpellMissile(TIL_HIT_FLASH);
}

/*C_69E5*/SPL_Gate()
{
	register int si;

	if(Party._tile < TIL_HORSE_W || Party._tile == TIL_BALLOON) {
		w_Failed();
		return;
	}
	if(!SpellOutdoorsOnly())
		return;
	si = AskLetter(/*D_2112*/"To Phase:\x12\x12\b", '0', '8');
	if(si < 0 || si == (int)'0')
		return;
	if(!SpellStart())
		return;
	si -= '1';
	Party._x = MoongateX[si];
	Party._y = MoongateY[si];
	LoadWorldWindow();
}

/*C_6A40*/SPL_Heal()
{
	register int si;

	if((si = AskChara(/*D_211F*/"Who?\x12\x12\b")) < 0)
		return;
	if(!SpellStart())
		return;
	if(!isCharaAlive(si)) {
		w_Failed();
		return;
	}
	HP_inc(si, U4_RND3(25) + 75);
}

/*C_6A82*/SPL_Iceball()
{
	SpellMissile(TIL_MAGIC_BOLT);
}

/*C_6A90*/SPL_Jinx()
{
	if(!SpellStart())
		return;
	spell_effect = 'J';
	spell_turns = 10;
}

/*C_6AA9*/SPL_Kill()
{
	SpellMissile(TIL_WHIRLPOOL);
}

/*C_6AB7*/SPL_Light()
{
	if(!SpellDungeonOnly())
		return;
	if(!SpellStart())
		return;
	Party._fly_light += 100;
	if(CurMode == MOD_DUNGEON)
		DNG_LookAround();
}

/*C_6ADB*/SPL_MagicMisl()
{
	SpellMissile(TIL_MISSILE);
}
  
/*C_6AE9*/SPL_Negate()
{
	if(!SpellStart())
		return;
	spell_effect = 'N';
	spell_turns = 10;
}

/*C_6B02*/SPL_Open()
{
	if(!SpellStart())
		return;
	if(CurMode == MOD_OUTDOORS && Party._tile == TIL_BALLOON) {
		w_Failed();
		return;
	}
	if(CurMode >= MOD_COMBAT) {
		COM_OpenChest();
	} else {
		OpenChest(-1);/*100% chance evasion*/
	}
}

/*C_6B36*/SPL_Protection()
{
	if(!SpellStart())
		return;
	spell_effect = 'P';
	spell_turns = 10;
}

/*C_6B4F*/SPL_Quickness()
{
	if(!SpellStart())
		return;
	spell_effect = 'Q';
	spell_turns = 10;
}

/*C_6B68*/SPL_Resurrect()
{
	register int si;

	if(CurMode >= MOD_COMBAT) {
		w_Failed();
		return;
	}
	si = AskChara(/*D_2127*/"Who:\x12\x12\b");
	if(si < 0)
		return;
	if(!SpellStart())
		return;
	if(Party.chara[si]._stat != 'D') {
		w_Failed();
		return;
	}
	Party.chara[si]._stat = 'G';
}

/*C_6BAE*/SPL_Sleep()
{
	register int si;

	if(!SpellCombatOnly())
		return;
	if(!SpellStart())
		return;
	for(si = 15; si >= 0; si--)
		if(Fighters._tile[si] && IsUndead(Fighters._tile[si]) == 0 && Fighters._tile[si] != TIL_BALRON && Fighters._HP[si] < u_rand_a())
			Fighters._sleeping[si] = 1;
}

/*C_6BF8*/SPL_Tremor()
{
	register int si;

	if(!SpellCombatOnly())
		return;
	if(!SpellStart())
		return;
	ShakeScreen();
	hit_tile = TIL_HIT_FLASH;
	for(si = 15; si >= 0; si --) {
		if(Fighters._tile[si] != 0 && Fighters._HP[si] < 192) {
			if(U4_RND1(1)) {
				if(U4_RND1(1))
					Fighters._HP[si] = 23;
				continue;
			}
			hit_x = Combat._npcX[si];
			hit_y = Combat._npcY[si];
			DrawCombat();
			sound(6);
			COM_DoDamage(si, activeChara, 0xff);
		}
	}
	hit_tile = 0;
}

/*C_6C71*/SPL_Undead()
{
	register int si;

	if(!SpellCombatOnly())
		return;
	if(!SpellStart())
		return;
	for(si = 15; si >= 0; si--)
		if(IsUndead(Fighters._tile[si]) && U4_RND1(1) && Fighters._HP[si] > 23)
			Fighters._HP[si] = 23;
}

/*C_6CB2*/SPL_View()
{
	if(!SpellStart())
		return;
	ShowMap();
}


/*C_6CC3*/SPL_Winds()
{
	int bp_02, bp_04;

	if(!SpellOutdoorsOnly())
		return;
	AskDir(/*D_212F*/"From Dir: ", &bp_02, &bp_04);
	if(!(bp_02|bp_04))
		return;
	if(!SpellStart())
		return;
	if(bp_02) {
		if(bp_02 == 1)
			WindDir = DIR_E;
		else
			WindDir = DIR_W;
	} else {
		if(bp_04 == 1)
			WindDir = DIR_S;
		else
			WindDir = DIR_N;
	}
}

/*C_6D22*/SPL_X_it()
{
	if(!SpellDungeonOnly())
		return;
	if(!SpellStart())
		return;
	Party._z = -1;
}

/*C_6D3D*/SPL_Y_up()
{
	int loc_A;
	unsigned char loc_B, loc_C;

	if(!SpellDungeonOnly())
		return;
	if(!SpellStart())
		return;
	if(Party._loc == 0x18) {
		w_Failed();
		return;
	}
	if(--Party._z < 0)
		return;
	for(loc_A = 32; loc_A > 0; loc_A --) {
		loc_B = U4_RND1(7);
		loc_C = U4_RND1(7);
		if(Area._map.x8x8x8[Party._z][loc_C][loc_B] == 0)
			break;
	}
	if(loc_A == 0) {
		Party._z ++;
		w_Failed();
		return;
	}
	Party._x = loc_B;
	Party._y = loc_C;
}

/*C_6DC1*/SPL_Z_down()
{
	int loc_A;
	unsigned char loc_B, loc_C;

	if(!SpellDungeonOnly())
		return;
	if(!SpellStart())
		return;
	if(Party._loc  == 0x18 || Party._z == 7) {
		w_Failed();
		return;
	}
	Party._z ++;
	for(loc_A = 32; loc_A > 0; loc_A --) {
		loc_B = U4_RND1(7);
		loc_C = U4_RND1(7);
		if(Area._map.x8x8x8[Party._z][loc_C][loc_B] == 0)
			break;
	}
	if(loc_A == 0) {
		Party._z --;
		w_Failed();
		return;
	}
	Party._x = loc_B;
	Party._y = loc_C;
}

typedef spell_handler();
typedef spell_handler *pSpell_handler;

pSpell_handler /*D_216E*/SpellHandlers[] = {
	SPL_Awaken,
	SPL_Blink,
	SPL_Cure,
	SPL_Dispel,
	SPL_Energy,
	SPL_Fireball,
	SPL_Gate,
	SPL_Heal,
	SPL_Iceball,
	SPL_Jinx,
	SPL_Kill,
	SPL_Light,
	SPL_MagicMisl,
	SPL_Negate,
	SPL_Open,
	SPL_Protection,
	SPL_Quickness,
	SPL_Resurrect,
	SPL_Sleep,
	SPL_Tremor,
	SPL_Undead,
	SPL_View,
	SPL_Winds,
	SPL_X_it,
	SPL_Y_up,
	SPL_Z_down
};

/*C_6E4A*/CMD_Cast()
{
	u4_puts(/*D_213A*/"Cast Spell!\n");
	if(CurMode < MOD_COMBAT) {
		if((activeChara = AskChara(/*D_2147*/"Player:\x12\x12\b")) < 0)
			return;
		if(!isCharaConscious(activeChara)) {
			w_Disabled();
			return;
		}
	}
	Gra_ClearStatus();
	ZtatsMixtures();
	txt_Y = 23;
	txt_X = 24;
	CurSpell = AskLetter(/*D_2152*/"Spell:\x12\x12\b", 'A', 'Z');
	Gra_ClearStatus();
	ZtatsClearTitle();
	dspl_Stats();
	if(CurMode >= MOD_COMBAT)
		Gra_InvertStatusLine(activeChara);
	if(CurSpell < 0)
		return;
	CurSpell -= 'A';
	u4_puts(Names[101 + CurSpell]); u4_puts(/*D_215C*/"!\n");
	/*-- check mixture --*/
	if(Party._mixtures[CurSpell] == 0) {
		w_NoneLeft();
		return;
	}
	Party._mixtures[CurSpell] --;
	/*-- check m.p --*/
	if(Party.chara[activeChara]._MP < SpellMPCost[CurSpell]) {
		u4_puts(/*D_215F*/"M.P. too low!\n");
		w_Failed();
		return;
	}
	/*-- cast spell --*/
	(*SpellHandlers[CurSpell])();
	dspl_Stats();
}
