/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*spells' magic points cost*/
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

static int16 /*D_8CCC*/CurSpell;/*current spell*/

/*Is tile an undead monster (ghost, phantom, skeleton, liche)?*/
int16 /*C_636D*/IsUndead(tile)
int16 tile;
{
	if(tile == TIL_GHOST || tile == TIL_PHANTOM || tile == TIL_SKELETON || tile == TIL_LICHE)
		return 1;
	return 0;
}

/*Print "Failed!".*/
int16 /*C_6399*/w_Failed()
{
	u4_puts(/*D_20A6*/"Failed!\n");
	hit_tile = 0;
	sound(SND_FAIL);
}

/*
 * Cast CurSpell: spend its magic points, with sound and flash. Returns 0
 * (failed) when Negate is in effect, 1 otherwise.
 */
int16 /*C_63B4*/SpellStart()
{
	Party.chara[activeChara]._MP -= SpellMPCost[CurSpell];
	sound(SND_CAST, SpellMPCost[CurSpell]);
	Gra_InvertView(); sound(SND_MAGIC, CurSpell+0x60); Gra_InvertView();
	if(spell_effect == 'N') {
		w_Failed();
		return 0;
	}
	return 1;
}

/*Returns 1 if outdoors, else fails and returns 0.*/
int16 /*C_6409*/SpellOutdoorsOnly()
{
	if(CurMode != MOD_OUTDOORS) {
		u4_puts(/*D_20AF*/"Outdoors Only!\n");
		w_Failed();
		return 0;
	}
	return 1;
}

/*Returns 1 if in combat, else fails and returns 0.*/
int16 /*C_6428*/SpellCombatOnly()
{
	if(CurMode < MOD_COMBAT) {
		u4_puts(/*D_20BF*/"Combat Only!\n");
		w_Failed();
		return 0;
	}
	return 1;
}

/*Returns 1 if in a dungeon, else fails and returns 0.*/
int16 /*C_6447*/SpellDungeonOnly()
{
	if(CurMode != MOD_DUNGEON) {
		u4_puts(/*D_20CD*/"Dungeon Only!\n");
		w_Failed();
		return 0;
	}
	return 1;
}
 

/*
 * Combat spell shooting a missile showing as tile in a direction, until it
 * hits a foe or something that stops it. The damage depends on the tile: magic
 * missile 16-63, iceball 32-255, fireball 24-127, kill (whirlpool tile) 232.
 */
int16 /*C_6466*/SpellMissile(tile)
int16 tile;
{
	register int16 foe, points;
	int16 dx, dy;

	if(!SpellCombatOnly())
		return;
	AskDir(/*D_20DC*/"Dir: ", &dx, &dy);
	if(!(dx|dy))
		return;
	if(!SpellStart())
		return;
	hit_tile = tile;
	hit_x = Combat._charaX[activeChara];
	hit_y = Combat._charaY[activeChara];
	foe = -1;
	do {
		if(!COM_MissileStep(dx, dy))
			break;
		DrawCombat();
	} while((foe = COM_GetFighterId(hit_x, hit_y)) == -1);
	if(foe == -1) {
		w_Failed();
		return;
	}
	sound(SND_HIT);
	switch(hit_tile) {
		case TIL_MISSILE: points = U4_RND3(64) | 16; break;
		case TIL_MAGIC_BOLT: points = U4_RND3(224) | 32; break;
		case TIL_HIT_FLASH: points = U4_RND3(128) | 24; break;
		default: points = 224 | 8;
	}
	hit_tile = 0;
	COM_DoDamage(foe, activeChara, points);
}


/*Awaken: wake up a sleeping party member.*/
int16 /*C_6558*/SPL_Awaken()
{
	register int16 member;

	member = AskChara(/*D_20E2*/"Who:\x12\x12\b");
	if(member < 0)
		return;
	if(!SpellStart())
		return;
	if(Party.chara[member]._stat != 'S') {
		w_Failed();
		return;
	}
	Party.chara[member]._stat = 'G';
	if(CurMode >= MOD_COMBAT) {
		CharaAnimTile[member] = Fighters._chtile[member] = CharaTile(member);
	}
}

/*
 * Blink: outdoors, on foot or on horseback, jump in a direction to the
 * farthest walkable square of the loaded map window. Not when both coordinates
 * are 0xc0 or more (the Abyss' corner of the world).
 */
int16 /*C_65AA*/SPL_Blink()
{
	int16 dx, dy;
	unsigned char dest_x, dest_y;

	if(Party._tile >= TIL_HORSE_W && Party._tile != TIL_BALLOON) {
		if(!SpellOutdoorsOnly())
			return;
		AskDir(/*D_20EA*/"Dir: ", &dx, &dy);
		if(!(dx|dy))
			return;
		if(!SpellStart())
			return;
		if((Party._x & Party._y) < 0xc0) {
			dest_x = WinPos.x;
			dest_y = WinPos.y;
			while(dest_x < 32 && dest_y < 32) {
				dest_x += dx;
				dest_y += dy;
			}
			do {
				dest_x -= dx;
				dest_y -= dy;
			} while((dest_x != WinPos.x || dest_y != WinPos.y) && !IsWalkable(Area._map.x32x32[dest_y][dest_x]));
			if(WinPos.x != dest_x || WinPos.y != dest_y) {
				Party._x += dest_x - WinPos.x;
				Party._y += dest_y - WinPos.y;
				LoadWorldWindow();
				return;
			}
		}
	}
	w_Failed();
}

/*Cure: cure a poisoned party member.*/
int16 /*C_669B*/SPL_Cure()
{
	register int16 member;

	if((member = AskChara(/*D_20F0*/"Who:\x12\x12\b")) < 0)
		return;
	if(!SpellStart())
		return;
	if(Party.chara[member]._stat != 'P') {
		w_Failed();
		return;
	}
	Party.chara[member]._stat = 'G';
}

/*
 * Dispel a field: in a dungeon, the one ahead; elsewhere, the one in the given
 * direction, replaced by the tile under the caster (not while flying).
 */
int16 /*C_66DA*/SPL_Dispel()
{
	int16 dx, dy;
	unsigned char *p_field;

	if(CurMode == MOD_DUNGEON) {
		if(!SpellStart())
			return;
		p_field = &(Area._map.x8x8x8[Party._z][DNG_Y_p(Party._y, Party._dir)][DNG_X_p(Party._x, Party._dir)]);
		if((*p_field & 0xf0) != 0xa0) {
			w_Failed();
			return;
		}
		*p_field = 0;
		return;
	}
	if(CurMode <= MOD_BUILDING && Party._fly_light) {
		w_Failed();
		return;
	}
	AskDir(/*D_20F8*/"Dir: ", &dx, &dy);
	if(!(dx|dy))
		return;
	if(!SpellStart())
		return;
	if(CurMode >= MOD_COMBAT) {
		if(Combat._charaY[activeChara] + dy >= 11 || Combat._charaX[activeChara] + dx >= 11) {
			w_Failed();
			return;
		}
		p_field = &(Combat_MAP(Combat._charaY[activeChara]+dy, Combat._charaX[activeChara]+dx));
		if(*p_field < TIL_POISON_FIELD || *p_field > TIL_SLEEP_FIELD) {
			w_Failed();
			return;
		}
		*p_field = Combat_MAP(Combat._charaY[activeChara], Combat._charaX[activeChara]);
		return;
	}
	if(CurMode == MOD_OUTDOORS) {
		p_field = &(Area._map.x32x32[WinPos.y+dy][WinPos.x+dx]);
	} else {
		if(Party._y + dy >= 32 || Party._x + dx >= 32) {
			w_Failed();
			return;
		}
		p_field = &(Area._map.x32x32[Party._y + dy][Party._x + dx]);
	}
	if(*p_field < TIL_POISON_FIELD || *p_field > TIL_SLEEP_FIELD) {
		w_Failed();
		return;
	}
	*p_field = tile_cur;
}

/*
 * Energy field of the type asked (Poison, Lightning, Fire, Sleep): in a
 * dungeon, ahead; in combat, on a walkable square in the given direction.
 * Probable original bug: in a dungeon, when the square ahead is not empty it
 * says "Failed!" but still replaces it with the field.
 */
int16 /*C_6882*/SPL_Energy()
{
	unsigned char *p_square;
	int16 typed;
	int16 dx, dy, field;

	u4_puts(/*D_20FE*/"Energy type? ");
	typed = (unsigned char)u_kbread();
/*bad hack:I must force the redefinition of u4_islower
in order to get the compiler to provide
the original bytecode*/
#define u4_islower(c) ((c) >= (int16)'a' && (c) <= (int16)'z')
	u4_toupper(typed);
	switch(typed) {
		case 'P'/*oison*/:    field = TIL_POISON_FIELD; break;
		case 'L'/*ightning*/: field = TIL_ENERGY_FIELD; break;
		case 'F'/*ire*/:      field = TIL_FIRE_FIELD; break;
		case 'S'/*leep*/:     field = TIL_SLEEP_FIELD; break;
		default: field = -1;
	}
	if(field == -1) {
		w_Failed();
		return;
	}
	u4_putc(typed);
	Gra_CR();
	if(CurMode == MOD_DUNGEON) {
		if(!SpellStart())
			return;
		p_square = &(Area._map.x8x8x8[Party._z][DNG_Y_p(Party._y, Party._dir)][DNG_X_p(Party._x, Party._dir)]);
		if(*p_square)
			w_Failed();
		*p_square = (field&3) | 0xa0;
		return;
	}
	if(CurMode >= MOD_COMBAT) {
		AskDir(/*D_210C*/"Dir: ", &dx, &dy);
		if(!(dx|dy))
			return;
		if(!SpellStart())
			return;
		if(Combat._charaY[activeChara] + dy < 11 && Combat._charaX[activeChara] + dx < 11) {
			if(IsWalkable(*(p_square = &(Combat_MAP(dy + Combat._charaY[activeChara], dx + Combat._charaX[activeChara]))))) {
				*p_square = field;
				return;
			}
		}
	}
	w_Failed();
}

/*Fireball: missile spell.*/
int16 /*C_69D7*/SPL_Fireball() {
	SpellMissile(TIL_HIT_FLASH);
}

/*Gate: outdoors, not on a ship or the balloon, go to the moongate of a phase (1-8).*/
int16 /*C_69E5*/SPL_Gate()
{
	register int16 phase;

	if(Party._tile < TIL_HORSE_W || Party._tile == TIL_BALLOON) {
		w_Failed();
		return;
	}
	if(!SpellOutdoorsOnly())
		return;
	phase = AskLetter(/*D_2112*/"To Phase:\x12\x12\b", '0', '8');
	if(phase < 0 || phase == (int16)'0')
		return;
	if(!SpellStart())
		return;
	phase -= '1';
	Party._x = MoongateX[phase];
	Party._y = MoongateY[phase];
	LoadWorldWindow();
}

/*Heal: give 75-99 hit points to a living party member.*/
int16 /*C_6A40*/SPL_Heal()
{
	register int16 member;

	if((member = AskChara(/*D_211F*/"Who?\x12\x12\b")) < 0)
		return;
	if(!SpellStart())
		return;
	if(!isCharaAlive(member)) {
		w_Failed();
		return;
	}
	HP_inc(member, U4_RND3(25) + 75);
}

/*Iceball: missile spell.*/
int16 /*C_6A82*/SPL_Iceball()
{
	SpellMissile(TIL_MAGIC_BOLT);
}

/*Jinx: for 10 turns, the foes may hit each other (see CheckJinx).*/
int16 /*C_6A90*/SPL_Jinx()
{
	if(!SpellStart())
		return;
	spell_effect = 'J';
	spell_turns = 10;
}

/*Kill: missile spell (shown with the whirlpool tile).*/
int16 /*C_6AA9*/SPL_Kill()
{
	SpellMissile(TIL_WHIRLPOOL);
}

/*Light: in a dungeon, 100 turns of light.*/
int16 /*C_6AB7*/SPL_Light()
{
	if(!SpellDungeonOnly())
		return;
	if(!SpellStart())
		return;
	Party._fly_light += 100;
	if(CurMode == MOD_DUNGEON)
		DNG_LookAround();
}

/*Magic missile: missile spell.*/
int16 /*C_6ADB*/SPL_MagicMisl()
{
	SpellMissile(TIL_MISSILE);
}
  
/*Negate: no magic for 10 turns.*/
int16 /*C_6AE9*/SPL_Negate()
{
	if(!SpellStart())
		return;
	spell_effect = 'N';
	spell_turns = 10;
}

/*
 * Open a chest (not in the balloon); outside combat, its trap is always
 * evaded, in combat the caster faces it.
 */
int16 /*C_6B02*/SPL_Open()
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
		OpenChest(-1);/*no character: the trap is always evaded*/
	}
}

/*Protection: for 10 turns, half of the foes' blows are parried.*/
int16 /*C_6B36*/SPL_Protection()
{
	if(!SpellStart())
		return;
	spell_effect = 'P';
	spell_turns = 10;
}

/*Quickness: for 10 turns (extra turns in combat, see COM_EndCharaTurn).*/
int16 /*C_6B4F*/SPL_Quickness()
{
	if(!SpellStart())
		return;
	spell_effect = 'Q';
	spell_turns = 10;
}

/*Resurrect: bring a dead party member back to life (not in combat).*/
int16 /*C_6B68*/SPL_Resurrect()
{
	register int16 member;

	if(CurMode >= MOD_COMBAT) {
		w_Failed();
		return;
	}
	member = AskChara(/*D_2127*/"Who:\x12\x12\b");
	if(member < 0)
		return;
	if(!SpellStart())
		return;
	if(Party.chara[member]._stat != 'D') {
		w_Failed();
		return;
	}
	Party.chara[member]._stat = 'G';
}

/*Sleep: in combat, put to sleep the foes (but undead and balrons) with less hit points than a random number.*/
int16 /*C_6BAE*/SPL_Sleep()
{
	register int16 foe;

	if(!SpellCombatOnly())
		return;
	if(!SpellStart())
		return;
	for(foe = 15; foe >= 0; foe--)
		if(Fighters._tile[foe] && IsUndead(Fighters._tile[foe]) == 0 && (unsigned char)(Fighters._tile[foe]) != TIL_BALRON && Fighters._HP[foe] < u_rand_a())
			Fighters._sleeping[foe] = 1;
}

/*
 * Tremor: in combat, each foe under 192 hit points is either killed (1 time
 * out of 2), or set to 23 hit points (1 time out of 4: this can heal a weak
 * one), or unharmed.
 */
int16 /*C_6BF8*/SPL_Tremor()
{
	register int16 foe;

	if(!SpellCombatOnly())
		return;
	if(!SpellStart())
		return;
	ShakeScreen();
	hit_tile = TIL_HIT_FLASH;
	for(foe = 15; foe >= 0; foe --) {
		if(Fighters._tile[foe] != 0 && Fighters._HP[foe] < 192) {
			if(U4_RND1(1)) {
				if(U4_RND1(1))
					Fighters._HP[foe] = 23;
				continue;
			}
			hit_x = Combat._npcX[foe];
			hit_y = Combat._npcY[foe];
			DrawCombat();
			sound(SND_HIT);
			COM_DoDamage(foe, activeChara, 0xff);
		}
	}
	hit_tile = 0;
}

/*Undead: in combat, 1 time out of 2 bring each undead foe down to 23 hit points.*/
int16 /*C_6C71*/SPL_Undead()
{
	register int16 foe;

	if(!SpellCombatOnly())
		return;
	if(!SpellStart())
		return;
	for(foe = 15; foe >= 0; foe--)
		if(IsUndead(Fighters._tile[foe]) && U4_RND1(1) && Fighters._HP[foe] > 23)
			Fighters._HP[foe] = 23;
}

/*View: show the map.*/
int16 /*C_6CB2*/SPL_View()
{
	if(!SpellStart())
		return;
	ShowMap();
}


/*Winds: outdoors, make the wind blow from the given direction.*/
int16 /*C_6CC3*/SPL_Winds()
{
	int16 dx, dy;

	if(!SpellOutdoorsOnly())
		return;
	AskDir(/*D_212F*/"From Dir: ", &dx, &dy);
	if(!(dx|dy))
		return;
	if(!SpellStart())
		return;
	if(dx) {
		if(dx == 1)
			WindDir = DIR_E;
		else
			WindDir = DIR_W;
	} else {
		if(dy == 1)
			WindDir = DIR_S;
		else
			WindDir = DIR_N;
	}
}

/*X-it: leave the dungeon.*/
int16 /*C_6D22*/SPL_X_it()
{
	if(!SpellDungeonOnly())
		return;
	if(!SpellStart())
		return;
	Party._z = -1;
}

/*
 * Y-up: in a dungeon (not the Abyss), go up a level to a random empty square
 * (32 tries); from the first level, leave the dungeon.
 */
int16 /*C_6D3D*/SPL_Y_up()
{
	int16 tries;
	unsigned char dest_x, dest_y;

	if(!SpellDungeonOnly())
		return;
	if(!SpellStart())
		return;
	if(Party._loc == LOC_ABYSS) {
		w_Failed();
		return;
	}
	if(--Party._z < 0)
		return;
	for(tries = 32; tries > 0; tries --) {
		dest_x = U4_RND1(7);
		dest_y = U4_RND1(7);
		if(Area._map.x8x8x8[Party._z][dest_y][dest_x] == 0)
			break;
	}
	if(tries == 0) {
		Party._z ++;
		w_Failed();
		return;
	}
	Party._x = dest_x;
	Party._y = dest_y;
}

/*
 * Z-down: in a dungeon (not the Abyss, not from the last level), go down a
 * level to a random empty square (32 tries).
 */
int16 /*C_6DC1*/SPL_Z_down()
{
	int16 tries;
	unsigned char dest_x, dest_y;

	if(!SpellDungeonOnly())
		return;
	if(!SpellStart())
		return;
	if(Party._loc  == LOC_ABYSS || Party._z == 7) {
		w_Failed();
		return;
	}
	Party._z ++;
	for(tries = 32; tries > 0; tries --) {
		dest_x = U4_RND1(7);
		dest_y = U4_RND1(7);
		if(Area._map.x8x8x8[Party._z][dest_y][dest_x] == 0)
			break;
	}
	if(tries == 0) {
		Party._z --;
		w_Failed();
		return;
	}
	Party._x = dest_x;
	Party._y = dest_y;
}

typedef spell_handler();
typedef spell_handler *pSpell_handler;

/*the spells, A to Z*/
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

/*
 * (C)ast: choose the caster (outside combat) and the spell. The mixture is
 * used up even when the magic points are too low.
 */
int16 /*C_6E4A*/CMD_Cast()
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
	u4_puts(Names[NAME_SPELLS + CurSpell]); u4_puts(/*D_215C*/"!\n");
	/*-- check mixture --*/
	if(Party._mixtures[CurSpell] == 0) {
		w_NoneLeft();
		return;
	}
	Party._mixtures[CurSpell] --;
	/*-- check m.p --*/
	if(Party.chara[activeChara]._MP < (uint16)(SpellMPCost[CurSpell])) {
		u4_puts(/*D_215F*/"M.P. too low!\n");
		w_Failed();
		return;
	}
	/*-- cast spell --*/
	(*SpellHandlers[CurSpell])();
	dspl_Stats();
}
