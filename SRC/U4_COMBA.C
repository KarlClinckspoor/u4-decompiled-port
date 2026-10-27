/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*Index of the foe (0-15) at (x, y) on the combat map, or -1.*/
/*C_59D5*/COM_GetFighterId(x, y)
unsigned char x;
unsigned char y;
{
	register int16 fighter;

	for(fighter = 15; fighter >= 0; fighter --)
		if(Fighters._tile[fighter] && Combat._npcX[fighter] == x && Combat._npcY[fighter] == y)
			break;
	return fighter;
}

/*Is every party member dead?*/
/*C_5A04*/IsPartyWipedOut()
{
	register int16 member;

	for(member = Party._members - 1; member >= 0; member --)
		if(isCharaAlive(member))
			return 0;
	return 1;
}

/*
 * Is the combat over: no party member left on the field, or no foe left
 * (in a dungeon room, only the first condition counts).
 */
/*C_5A28*/IsCombatEnded()
{
	register int16 who;

	for(who = 7; who >= 0; who --) {
		if(Fighters._chtile[who] != 0)
			break;
	}
	if(who == -1)
		return 1;

	if(CurMode == MOD_COM_ROOM) {
		goto C_5A5F;
	}

	for(who = 15; who >= 0; who--) {
		if(Fighters._tile[who] != 0) {
C_5A5F:
			return 0;
		}
	}
	return 1;
}

/*COM_EndCharaTurn();
COM_Attack();*/

/*
 * The combat loop: each conscious party member on the field gets a command,
 * then the foes act, until the combat is over. Ctrl-S right after Alt-Z (the
 * last key of the previous round) shows the karmas.
 */
/*C_5A6B*/COM_main()
{
	register uint16 cmd;
	uint16 last_key;

	u_kbflush();
	last_key = 0;
	RoomExitX = RoomExitY = 0;
	do {
		for(activeChara = 0; /*C_5A88:*/activeChara < Party._members && !IsCombatEnded(); activeChara++) {
/*C_5A9E*/
			if(Fighters._chtile[activeChara] && isCharaConscious(activeChara)) {
				CursorBlink = 4;
				Gra_InvertStatusLine(activeChara);
				activeCharaX = Combat._charaX[activeChara];
				activeCharaY = Combat._charaY[activeChara];
				u4_puts(Party.chara[activeChara]._name);
				u4_puts(/*D_1FE6*/" with ");
				u4_puts(Names[NAME_WEAPONS + Party.chara[activeChara]._weapon]);
				Gra_CR();
				u4_putc(0x10);
				CursorBlink = 2;
				DrawCombat();
				u_delay(25, 1);
				cmd = u_kbhit()?u_kbread():KBD_SPACE;
				if(u4_isupper((unsigned char)cmd))
					cmd = (cmd & 0xff00) | u4_lower((unsigned char)cmd);
				switch(cmd) {
					case KBD_SPACE: w_Pass(); break;
					case KBD_UP_:
					case KBD_UP: COM_North(); break;
					case KBD_LEFT_:
					case KBD_LEFT: COM_West(); break;
					case KBD_DOWN_:
					case KBD_DOWN: COM_South(); break;
					case KBD_RIGHT_:
					case KBD_RIGHT: COM_East(); break;
					case KBD_A: COM_Attack(); break;/*(A)ttack(combat)*/
					case KBD_C: CMD_Cast(); break;
					case KBD_G: COM_GetChest(); break;/*(G)et chest(combat)*/
					case KBD_R: CMD_Ready(); break;
					case KBD_U: CMD_Use(); break;
					case KBD_V: CMD_Volume(); break;
					case KBD_Z: CMD_Ztats(); break;
					case KBD_B:
					case KBD_D:
					case KBD_E:
					case KBD_F:
					case KBD_H:
					case KBD_I:
					case KBD_J:
					case KBD_K:
					case KBD_L:
					case KBD_M:
					case KBD_N:
					case KBD_O:
					case KBD_P:
					case KBD_Q:
					case KBD_S:
					case KBD_T:
					case KBD_W:
					case KBD_X:
					case KBD_Y: w_NotHere(); break;
					case KBD_CTRL_S:
						if(last_key == KBD_ALT_Z) {
							CheatShowKarma();
							break;
						}
					default:
						u4_puts(/*D_1FED*/"Bad command\n");
						sound(SND_BAD_COMMAND);
						Gra_InvertStatusLine(activeChara);
						activeChara --;
					continue;
				}
				Gra_InvertStatusLine(activeChara);
				activeCharaX = -1;
				dspl_Stats();
			}
			if(!TurnDone) {
				TurnDone = 1;
				activeChara --;
			} else {
				COM_EndCharaTurn();
			}
/*C_5C09*/
		}
/*C_5CEF:*/
		COM_FoesTurn();
		last_key = cmd;
	} while(!IsCombatEnded());
	if(!IsPartyWipedOut())
		CombatEnd();
	TurnDone = 1;
}

/*
 * End of the active character's turn: effects of the tile it stands on,
 * removal if dead, maybe another turn with Quickness (1 time out of 2), maybe
 * waking up (1 time out of 8), and food consumption.
 */
/*C_5D14*/COM_EndCharaTurn()
{
	if(Fighters._chtile[activeChara] == 0)
		return;
	if(isCharaConscious(activeChara))
		TileEffects();
	if(!isCharaAlive(activeChara)) {
		Fighters._chtile[activeChara] = 0;
	} else if(isCharaConscious(activeChara) && spell_effect == 'Q' && U4_RND1(1)) {
		/*Quickness!*/
		activeChara --;
		return;
	} else if(Party.chara[activeChara]._stat == 'S' && !U4_RND1(7)) {
		Party.chara[activeChara]._stat = 'G';
		CharaAnimTile[activeChara] = Fighters._chtile[activeChara] = CharaTile(activeChara);
		dspl_Stats();
	}
	food_dec(1);
}

/*
 * Inflict damage to foe fighter (Lord British cannot be killed). If it is
 * killed, party member chara (unless -1) gets experience (max HP / 16 + 1);
 * otherwise print its state ("Fleeing!" being under 24 HP whatever its max).
 */
/*C_5DAB*/COM_DoDamage(fighter, chara, damage)
register int16 fighter;
int16 chara;
unsigned char damage;
{
	u4_puts(CreatureName(Fighters._tile[fighter]));
	u4_putc(' ');
	if(Fighters._tile[fighter] != TIL_LORD_BRITISH && (Fighters._HP[fighter] -= damage) < 0) {
		u4_puts(/*D_1FFA*/"Killed!\n");
		if(chara != -1) {
			register int16 xp;

			xp = MonsterMaxHP[MonsterIndex(Fighters._tile[fighter])] / 16 + 1;
			XP_inc(chara, xp);
			u4_puts(/*D_2003*/"Exp. ");
			u4_putl(xp, 1, ' ');
			Gra_CR();
		}
		Fighters._tile[fighter] = 0;
	} else {
		int16 hp, mark75, mark25, mark50;

		mark25 = mark50 = MonsterMaxHP[MonsterIndex(Fighters._tile[fighter])] / 2;/*50% HP*/
		mark25 >>= 1;/*25% HP*/
		mark75 = mark25 + mark50;/*75% HP*/
		hp = Fighters._HP[fighter];
		if(hp < 24) {
			u4_puts(/*D_2009*/"Fleeing!\n");
		} else if(hp < mark25) {
			u4_puts(/*D_2013*/"Critical!\n");
		} else {
			Gra_CR();
			if(hp < mark50) {
				u4_puts(/*D_201E*/"Heavily ");
			} else if(hp < mark75) {
				u4_puts(/*D_2027*/"Lightly ");
			} else {
				u4_puts(/*D_2030*/"Barely ");
			}
			u4_puts(/*D_2038*/"Wounded!\n");
		}
	}
}

/*
 * Move the blow position (hit_x, hit_y) one square in direction (dx, dy),
 * unless it would leave the map or enter a square that stops missiles
 * (secret doors, and unwalkable tiles other than water, shores, ship rails
 * and creatures). Returns 1 if it moved.
 */
/*C_5EA2*/COM_MissileStep(dx, dy)
unsigned char dx;
unsigned char dy;
{
	unsigned char x, y;
	register int16 tile;

	x = dx + hit_x;
	y = dy + hit_y;
	if(x > 10 || y > 10)
		return 0;
	tile = Combat_MAP(y, x);
	if(tile == TIL_SECRET_DOOR)
		return 0;
	if(tile <= TIL_PIRATE_W && tile > TIL_SHALLOWS && tile != TIL_SHIP_RAIL)
		if(tile < TIL_SHORE_1 || tile > TIL_SHORE_4)
			if(!IsWalkable(tile))
				return 0;

	hit_x = x;
	hit_y = y;
	return 1;
}

/*
 * After a blow at range squares, a thrown magic axe flies back to the active
 * character.
 */
/*C_5F26*/DrawAxeReturn(range)
int16 range;
{
	int16 dy, dx;

	if(Party.chara[activeChara]._weapon != WPN_MAGIC_AXE || range < 2)
		return;
	dx = u4_sign(Combat._charaX[activeChara] - hit_x);
	dy = u4_sign(Combat._charaY[activeChara] - hit_y);
	hit_tile = TIL_MISSILE;
	while(range) {
		COM_MissileStep(dx, dy);
		DrawCombat();
		range --;
	}
	hit_tile = 0;
}

/*
 * The blow (at range squares, 0 for a hand to hand blow) missed. Flaming oil
 * still sets a fire field where it lands (but not on water).
 */
/*C_5F9D*/w_missed(range)
int16 range;
{
	if(range == 0)
		sound(SND_ATTACK);
	hit_tile = 0;
	u4_puts(/*D_2042*/"Missed!\n");
	if(Party.chara[activeChara]._weapon == WPN_OIL && hit_x < 11 && hit_y < 11 && Combat_MAP(hit_y, hit_x) >= TIL_SWAMP)
		Combat_MAP(hit_y, hit_x) = TIL_FIRE_FIELD;
	DrawCombat();
	DrawAxeReturn(range);
}

/*
 * The active character's blow reaches foe fighter at range squares. In the
 * Abyss only the magic weapons work. A dexterity under 40 may miss. The damage
 * is random, modulo strength + weapon damage (at most 255).
 */
/*C_6012*/TryDamage(fighter, range)
int16 fighter;
int16 range;
{
	register char *p_ground;
	register struct tChara *chara;
	int16 damage;
	
	chara = &(Party.chara[activeChara]);
	if(Party._loc == LOC_ABYSS && chara->_weapon < WPN_MAGIC_AXE) {
		/*abyss without good weapons*/
		w_missed(range);
		return;
	}
	hit_tile = (chara->_weapon == WPN_WAND)?TIL_MAGIC_BOLT:TIL_HIT_FLASH;
	/*dexterity test*/
	if(chara->_dex < 40 && U4_RND1(0xff) > chara->_dex + 0x80) {
		w_missed(range);
		return;
	}
	/*success*/
	p_ground = &(Combat_MAP(Combat._npcY[fighter], Combat._npcX[fighter]));
	if(chara->_weapon == WPN_OIL && *p_ground >= TIL_SWAMP)
		*p_ground = TIL_FIRE_FIELD;
	/*damage points*/
	damage = chara->_str;
	damage += WeaponDamage[chara->_weapon];
	damage = SafeModulo(U4_RND1(0xff), u4_min(damage, 0xff));
	DrawCombat();
	sound(SND_HIT);
	hit_tile = 0;
	COM_DoDamage(fighter, activeChara, damage);
	DrawAxeReturn(range);
}

/*
 * The active character shoots (or throws) in direction (dx, dy): up to 11
 * squares, or the range asked for flaming oil. Daggers and oil are used up.
 */
/*C_60F1*/COM_Shoot(dx, dy)
int16 dx;
int16 dy;
{
	register struct tChara *chara;
	int16 range, foe, dist;

	range = 11;
	dist = 0;
	chara = &(Party.chara[activeChara]);
	/*-- For OIL, ask range --*/
	if(chara->_weapon == WPN_OIL) {
		range = AskLetter(/*D_204B*/"Range:\x12\x12\b", '0', '9');
		range -= '0';
		if(range < 0)
			return;
	}
	/*-- if thrown weapon, decrement --*/
	if(chara->_weapon == WPN_DAGGER || chara->_weapon == WPN_OIL) {
		if(Party._weapons[chara->_weapon] == 0) {
			chara->_weapon = WPN_HANDS;
			u4_puts(/*D_2055*/"Last one!\n");
		} else {
			Party._weapons[chara->_weapon] --;
		}
	}
	/*-- --*/
	hit_tile = (chara->_weapon != WPN_WAND)?TIL_MISSILE:TIL_MAGIC_BOLT;
	sound(SND_ATTACK);
	/*note:when using flaming oil with range '0'
	 this loop is never entered, and so foe stays
	 uninitialized. This may lead to unexpected behavior
	 on the original version, or exception on modern
	 systems.
	*/
	for(; range; range--) {
		if(!COM_MissileStep(dx, dy)) {
			w_missed(dist);
			return;
		}
		dist ++;
		DrawCombat();
		if((foe = COM_GetFighterId(hit_x, hit_y)) != -1)
			break;
	}
	hit_tile = 0;
	if(foe == -1) {
		w_missed(dist);
		return;
	}
	TryDamage(foe, dist);
}

/*
 * (A)ttack in combat: hand to hand weapons hit the next square (the halberd
 * the one after); a dagger with nobody next is thrown; ranged weapons shoot.
 */
/*C_61D1*/COM_Attack()
{
	register struct tChara *chara;
	int16 dx, dy, foe;

	chara = &(Party.chara[activeChara]);
	AskDir(/*D_2060*/"Dir: ", &dx, &dy);
	if(!(dx | dy))
		return;
	hit_x = Combat._charaX[activeChara];
	hit_y = Combat._charaY[activeChara];

	if(!WeaponIsRanged[chara->_weapon]) {/*is not range weapon?*/
		if(chara->_weapon == WPN_HALBERD) {
			hit_x += dx;
			hit_y += dy;
		}
		if(COM_MissileStep(dx, dy)) {
			hit_tile = TIL_MISSILE;
			DrawCombat();
			hit_tile = 0;
			sound(SND_ATTACK);

			if((foe = COM_GetFighterId(hit_x, hit_y)) != -1) {
				TryDamage(foe, 1);
				return;
			}
			if(chara->_weapon == WPN_DAGGER) {
				COM_Shoot(dx, dy);
				return;
			}
		}
		w_missed(0);
	} else {
		COM_Shoot(dx, dy);
		return;
	}
}

/*
 * (A)ttack outside combat: start a combat with the npc in the given direction.
 * In a towne, the guards and Lord British turn hostile; attacking there, or
 * attacking a non-evil creature, costs compassion, justice and honor.
 * Odd: TIL_SHALLOWS (2) is tested as an npc tile.
 */
/*C_628F*/CMD_Attack()
{
	register int16 npc;
	int16 dx, dy;

	if(CurMode <= MOD_BUILDING && Party._fly_light != 0) {
		u4_puts(/*D_2066*/"Attack\n");
		w_DriftOnly();
		return;
	}

	AskDir(/*D_206E*/"Attack: ", &dx, &dy);
	if(!(dx|dy))
		return;
	if(
		(npc = FindLocalNpcAt(Party._x + dx, Party._y + dy)) == -1 ||
		Area._npc._tile[npc] == TIL_WHIRLPOOL ||
		Area._npc._tile[npc] == TIL_SHALLOWS ||
		Area._npc._tile[npc] == TIL_TWISTER
	) {
		u4_puts(/*D_2077*/"Nothing to Attack!\n");
		return;
	}
	if(CurMode == MOD_BUILDING) {
		register int16 guard;
		for(guard = 31; guard >= 0; guard --)
			if(Area._npc._tile[guard] == TIL_GUARD || Area._npc._tile[guard] == TIL_LORD_BRITISH) {
				Area._npc._var[guard] = 0xff;
				Area._npc._tlkidx[guard] = 0;
			}
	}
	if(CurMode == MOD_BUILDING || IsNonEvil(Area._npc._tile[npc])) {
		karma_dec(&(Party._compa), 5);
		karma_dec(&(Party._justi), 5);
		karma_dec(&(Party._honor), 5);
	}
	StartCombat(npc);
}
