/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*C_93A5*/DNG_MonsterCanMove(bp08, bp06, bp04)
unsigned char bp08;
unsigned char bp06;
int bp04;
{
	unsigned char bp_02;

	bp_02 = Area._map.x8x8x8[Party._z][bp06][bp08];
	if((bp_02 & 0x0f) == 0) {
		if(
			(bp_02 & 0xe0) != 0x80 &&
			(bp_02 & 0xf0) != 0xa0 &&
			(bp_02 & 0xf0) != 0xd0 &&
			(bp_02 & 0xf0) != 0xf0
		) {
			if(U4_RND1(7) == 0 || Area._npc._old_x[bp04] != bp08 || Area._npc._old_y[bp04] != bp06)
				return 1;
		}
	}
	return 0;
}

/*C_9414*/DNG_MoveMonsters()
{
	int loc_A, loc_B, loc_C, loc_D;
	unsigned char loc_E, loc_F;
	
	for(loc_B = 31; loc_B >= 0; loc_B --) {/*end:C_95A1*/
		if(
			Area._npc._tile[loc_B] != 0 &&
			Area._npc._var[loc_B] == Party._z &&
			Area._npc._tile[loc_B] != TIL_REAPER &&
			Area._npc._tile[loc_B] != TIL_MIMIC
		) {
			if(Party._x != Area._npc._x[loc_B] || Party._y != Area._npc._y[loc_B]) {
				for(loc_D = 7; loc_D >= 0; loc_D --) {/*end:C_94CB*/
					loc_A = u4_sign((char)u_rand_a());
					loc_C = u4_sign((char)u_rand_a());
					if(U4_RND1(1) && loc_A != 0) {
						loc_E = (Area._npc._x[loc_B] + loc_A) & 7;
						loc_F = Area._npc._y[loc_B];
						if(DNG_MonsterCanMove(loc_E, loc_F, loc_B))
							break;
					}
					if(loc_C) {
						loc_E = Area._npc._x[loc_B];
						loc_F = (Area._npc._y[loc_B] + loc_C) & 7;
						if(DNG_MonsterCanMove(loc_E, loc_F, loc_B))
							break;
					}
					loc_E = (Area._npc._x[loc_B] + loc_A) & 7;
					loc_F = Area._npc._y[loc_B];
					if(DNG_MonsterCanMove(loc_E, loc_F, loc_B))
						break;
				}
				if(loc_D != -1) {
					Area._map.x8x8x8[Party._z][loc_F][loc_E] |= (Area._npc._tile[loc_B] - TIL_RAT) / 4 + 1;
					Area._map.x8x8x8[Party._z][Area._npc._y[loc_B]][Area._npc._x[loc_B]] &= 0xf0;
					Area._npc._old_x[loc_B] = Area._npc._x[loc_B];
					Area._npc._old_y[loc_B] = Area._npc._y[loc_B];
					Area._npc._x[loc_B] = loc_E;
					Area._npc._y[loc_B] = loc_F;
				}
			}
		}
	}
	RefreshView();
}

/*C_95AA*/DNG_SpawnMonsters()
{
	int loc_A;
	unsigned char loc_B, loc_C, loc_D;

	for(loc_A = Party._z * 4 + 1; loc_A >= 0; loc_A --) {
		if((loc_A >> 2) < Party._z)
			break;
		if(Area._npc._tile[loc_A] == 0) {
			loc_B = U4_RND1(7);
			loc_D = U4_RND1(7);
			if(Party._x != loc_B && Party._y != loc_D) {
				if(Area._map.x8x8x8[Party._z][loc_D][loc_B] == 0) {
					loc_C = (U4_RND1(3) + Party._z) * 4 + TIL_RAT;
					if(loc_C != TIL_MIMIC) {
						Area._npc._tile[loc_A] = loc_C;
						Area._npc._x[loc_A] = Area._npc._old_x[loc_A] = loc_B;
						Area._npc._y[loc_A] = Area._npc._old_y[loc_A] = loc_D;
						Area._npc._var[loc_A] = Party._z;
						Area._map.x8x8x8[Party._z][loc_D][loc_B] = ((loc_C - 0x90) >> 2) + 1;
					}
				}
			}
		}
	}
}

/*C_9683*/CharaAtHit()
{
	register int si;

	for(si = Party._members; --si >= 0;  ) {
		if(isCharaAlive(si) && hit_x == Combat._charaX[si] && hit_y == Combat._charaY[si])
			return si;
	}
	return -1;
}

/*C_96B9*/FoeHitsChara(bp06, bp04)
int bp06;
int bp04;
{
	unsigned bp_02;

	Gra_InvertStatusLine(bp06);
	DrawCombat();
	sound(7);
	hit_tile = 0;
	DrawCombat();
	bp_02 = U4_RND4(MonsterMaxHP[MonsterIndex((unsigned char)Fighters._tile[bp04])] >> 2);
	bp_02 = (bp_02 >> 4) * 10 + (bp_02 % 10);
	Gra_InvertStatusLine(bp06);
	if(hitChara(bp06, bp_02) == 0) {
		dspl_Stats();
		return 0;
	}
	u4_puts(Party.chara[bp06]._name);
	u4_puts(/*D_29CA*/" is Killed!\n");
	karma_inc(&(Party._sacri), 1);
	dspl_Stats();
	CheckPartyAlive();

	return 1;
}

/*C_9764*/CharaStruck(bp04)
int bp04;
{
	Gra_InvertStatusLine(bp04);
	DrawCombat();
	sound(7);
	Gra_InvertStatusLine(bp04);
	hit_tile = 0;
}

/*monster projectile?*/
/*C_978C*/FoeShoot(bp0a, bp08, bp06, bp04)
unsigned char bp0a;/*type*/
int bp08;/*monster id*/
int bp06;
int bp04;
{
	register int si;

	hit_tile = bp0a;
	hit_x = Combat._npcX[bp08];
	hit_y = Combat._npcY[bp08];
	sound(3);
	si = -1;
	do {
		if(COM_CheckHitable(bp06, bp04) == 0)
			break;
		DrawCombat();
	} while((si = CharaAtHit()) == -1);
	if(si == -1) {
		if(hit_tile == TIL_LAVA)
			Combat_MAP(hit_y, hit_x) = TIL_LAVA;
		hit_tile = 0;
		return;
	}
	Gra_CR();
	u4_puts(Party.chara[si]._name);
	u4_putc(' ');
	switch(hit_tile) {
		case TIL_POISON_FIELD:
			u4_puts(/*D_29D7*/"Poisoned!\n");
			CharaStruck(si);
			if(U4_RND1(1) || Party.chara[si]._stat != 'G')
				u4_puts(/*D_29E2*/"Failed.\n");
			else
				Party.chara[si]._stat = 'P';
		break;
		case TIL_ENERGY_FIELD:
			u4_puts(/*D_29EB*/"Electrified!\n");
			FoeHitsChara(si, bp08);
		break;
		case TIL_FIRE_FIELD:
			u4_puts(/*D_29F9*/"Fiery Hit!\n");
			FoeHitsChara(si, bp08);
		break;
		case TIL_SLEEP_FIELD:
			u4_puts(/*D_2A05*/"Slept!\n");
			CharaStruck(si);
			if(U4_RND1(1) || Party.chara[si]._stat != 'G')
				u4_puts(/*D_2A0D*/"Failed.\n");
			else {
				Party.chara[si]._stat = 'S';
				Fighters._chtile[si] = TIL_SLEEPING;
			}
		break;
		case TIL_LAVA:
			u4_puts(/*D_2A16*/"Lava Hit!\n");
			FoeHitsChara(si, bp08);
		break;
		case TIL_MAGIC_BOLT:
			u4_puts(/*D_2A21*/"Magical Hit!\n");
			FoeHitsChara(si, bp08);
		break;
		default:
			u4_puts(/*D_2A2F*/"Hit!\n");
			FoeHitsChara(si, bp08);
	}
	hit_tile = 0;
	dspl_Stats();
}

/*C_98E4*/FoeCanMoveTo(bp08, bp06, bp04)
unsigned char bp08;
unsigned char bp06;
int bp04;
{
	unsigned char bp_02;
	int bp_04;

	bp_02 = Combat_MAP(bp06, bp08);
	if(IsSlowTerrain(bp_02))
		return 0;
	if(bp08 >= 11 || bp06 >= 11)
		return ((unsigned char)Fighters._HP[bp04] < 24);
	if(Fighters._x[bp04] == bp08 && Fighters._y[bp04] == bp06 && U4_RND1(3))
		return 0;

	for(bp_04 = 15; bp_04 >= 0; bp_04 --)
		if(Fighters._tile[bp_04] && Combat._npcX[bp_04] == bp08 && Combat._npcY[bp_04] == bp06)
			return 0;
	for(bp_04 = 7; bp_04 >= 0; bp_04 --)
		if(Fighters._chtile[bp_04] && Combat._charaX[bp_04] == bp08 && Combat._charaY[bp_04] == bp06)
			return 0;

	if((unsigned char)Fighters._tile[bp04] < 0x80)
		return IsWalkable(Combat_MAP(bp06, bp08));
	if((unsigned char)Fighters._tile[bp04] < 0x8e)
		return (bp_02 < 3);
	switch((unsigned char)Fighters._gtile[bp04]) {
		case TIL_INSECTS:
		case TIL_TWISTER:
		case TIL_BAT:
		case TIL_DAEMON:
		case TIL_DRAGON:
		case TIL_BALRON:
			return (bp_02 < 3) || IsWalkable(bp_02);
		case TIL_GHOST:
		case TIL_ZORN:
			return (bp_02 >= 3) && (bp_02 != 0x45);
		default:
			return IsWalkable(bp_02);
	}
}

unsigned char /*C_9A41*/FoeMissile(bp04)
unsigned char bp04;
{
	switch(bp04) {
		case TIL_MAGE:
		case TIL_LORD_BRITISH:
		case TIL_SEAHORSE:
		case TIL_EVIL_MAGE:
		case TIL_LICHE:
		case TIL_DAEMON:
			return TIL_MAGIC_BOLT;/*magic sphere*/
		case TIL_NIXIE:
		case TIL_TROLL:
			return TIL_MISSILE;/*Missile*/
		case TIL_SQUID:
			return TIL_ENERGY_FIELD;/*energy field*/
		case TIL_SERPENT:
		case TIL_HYDRA:
		case TIL_DRAGON:
			return TIL_HIT_FLASH;/*flash*/
		case TIL_SPIDER:
		case TIL_MIMIC:
		case TIL_PYTHON:
			return TIL_POISON_FIELD;/*poison field*/
		case TIL_GAZER:
			return TIL_SLEEP_FIELD;/*sleep field*/
		case TIL_ETTIN:
		case TIL_CYCLOPS:
			return TIL_ROCKS;/*rocks*/
		case TIL_LAVA_LIZARD:
			return TIL_LAVA;/*lava*/
		case TIL_REAPER:
		case TIL_BALRON:
			return U4_RND1(3) + TIL_POISON_FIELD;/*[poison|energy|fire] field*/
		default:
			return 0;
	}
}

/*C_9B03*/CheckJinx(_x, _y)
unsigned char _x;
unsigned char _y;
{
	int bp_02;

	if(spell_effect == 'J') {
		if((bp_02 = COM_GetFighterId(_x, _y)) != -1) {
			hit_tile = TIL_HIT_FLASH;
			hit_x = _x;
			hit_y = _y;
			DrawCombat();
			hit_tile = 0;
			sound(6);
			COM_DoDamage(bp_02, -1, U4_RND1(0x3f));
			DrawCombat();
			return 1;
		}
	}
	return 0;
}

/*steal food*/
/*C_9B6B*/StealFood()
{
	Party._food = u4_max(Party._food - 2500, 0L);
	dspl_Stats();
	sound(8);
}

/*steal gold*/
/*C_9BA6*/StealGold()
{
	register int si;

	if(U4_RND1(3) == 0) {
		si = U4_RND1(0x3f);
		Party._gold = u4_max(Party._gold - si, 0);
		dspl_Stats();
		sound(8);
	}
}

/*contact attack?*/
/*C_9BE5*/FoeMelee(bp06, bp04)
int bp06;
int bp04;
{
	hit_x = Combat._charaX[bp04];
	hit_y = Combat._charaY[bp04];
	hit_tile = TIL_MISSILE;
	DrawCombat();
	sound(5);
	hit_tile = 0;
	if((spell_effect == 'P' && U4_RND1(1)) || ArmorDefense[Party.chara[bp04]._armor] >= u_rand_a()) {
		DrawCombat();
	} else if(FoeHitsChara(bp04, bp06) == 0) {
		return 1;
	}
	return 0;
}

/*C_9C56*/FoeFlees(bp04)
int bp04;
{
	Gra_CR();
	u4_puts(CreatureName((unsigned char)Fighters._tile[bp04]));
	u4_puts(/*D_2A35*/" Flees!\n");
	if(IsNonEvil((unsigned char)Fighters._tile[bp04])) {
		karma_inc(&(Party._compa), 1);
		karma_inc(&(Party._justi), 1);
	}
	sound(8);
	Fighters._tile[bp04] = Fighters._HP[bp04] = 0;
}

/*move enemies during fight*/
/*C_9CBC*/FoeAct(bp08, bp06, bp04)
int bp08;/*monster id*/
int bp06;/*chara id*/
int bp04;
{
	int loc_A, loc_B, loc_C;
	unsigned char loc_D, loc_E;

	loc_A = u4_sign((unsigned char)Combat._charaX[bp06] - (unsigned char)Combat._npcX[bp08]);
	loc_C = u4_sign((unsigned char)Combat._charaY[bp06] - (unsigned char)Combat._npcY[bp08]);
	/*Mimic*/
	if((unsigned char)Fighters._tile[bp08] == TIL_MIMIC) {
		Fighters._gtile[bp08] = TIL_CHEST;
		if(bp04 >= 5)
			goto C_9D51;
		Fighters._gtile[bp08] = TIL_MIMIC;
	}
	/*attempt projectile*/
	if(U4_RND1(3) == 0) {
		int bp_0c;

		if((bp_0c = FoeMissile((unsigned char)Fighters._tile[bp08])) != 0) {
			if(bp_0c != TIL_MAGIC_BOLT || spell_effect != 'N') {
				FoeShoot(bp_0c, bp08, loc_A, loc_C);
				RefreshView();
				return;
			}
		}
	}
	/* */
C_9D51:
	/*balron or reaper*/
	if(U4_RND1(3) == 0) {
		if((unsigned char)Fighters._tile[bp08] == TIL_BALRON || (unsigned char)Fighters._tile[bp08] == TIL_REAPER) {
			if(spell_effect != 'N') {
				RefreshView();
				Gra_InvertView(); sound(9, 0x80); Gra_InvertView();
				for(loc_B = Party._members - 1; loc_B >= 0; loc_B --) {
					if(Fighters._chtile[loc_B] != 0 && Party.chara[loc_B]._stat == 'G' && U4_RND1(1)) {
						Party.chara[loc_B]._stat = 'S';
						Fighters._chtile[loc_B] = TIL_SLEEPING;
					}
				}
				dspl_Stats();
				return;
			}
		}
	}
	/* */
	if((unsigned char)Fighters._HP[bp08] < 24) {
		loc_A = -loc_A;
		loc_C = -loc_C;
C_9DE5:
		if((unsigned char)Fighters._tile[bp08] == TIL_MIMIC || (unsigned char)Fighters._tile[bp08] == TIL_REAPER)
			return;
	} else if(bp04 >= 2) {
		goto C_9DE5;
	} else {
		/*contact?*/
		switch((unsigned char)Fighters._tile[bp08]) {
			case TIL_GREMLIN: StealFood(); break;
			case TIL_ROGUE: StealGold(); break;
		}
		FoeMelee(bp08, bp06);/*contact attack?*/
		return;
	}
	/* */
	for(loc_B = 2; loc_B > 0; loc_B --) {
		if(U4_RND1(1) && loc_A != 0) {
			loc_D = (unsigned char)Combat._npcX[bp08] + loc_A;
			loc_E = (unsigned char)Combat._npcY[bp08];
			if(CheckJinx(loc_D, loc_E))
				return;
			if(FoeCanMoveTo(loc_D, loc_E, bp08))
				break;
		}
		if(loc_C != 0) {
			loc_D = (unsigned char)Combat._npcX[bp08];
			loc_E = (unsigned char)Combat._npcY[bp08] + loc_C;
			if(CheckJinx(loc_D, loc_E))
				return;
			if(FoeCanMoveTo(loc_D, loc_E, bp08))
				break;
		}
		if(loc_A != 0) {
			loc_D = (unsigned char)Combat._npcX[bp08] + loc_A;
			loc_E = (unsigned char)Combat._npcY[bp08];
			if(CheckJinx(loc_D, loc_E))
				return;
			if(FoeCanMoveTo(loc_D, loc_E, bp08))
				break;
		}
		loc_A = u4_sign((char)u_rand_a());
		loc_C = u4_sign((char)u_rand_a());
	}
	if(loc_B != 0) {
		Fighters._x[bp08] = Combat._npcX[bp08];
		Fighters._y[bp08] = Combat._npcY[bp08];
		Combat._npcX[bp08] = loc_D;
		Combat._npcY[bp08] = loc_E;
		if(loc_D >= 11 || loc_E >= 11) {
			FoeFlees(bp08);/*... Flees*/
			return;
		}
		DrawCombat();
		return;
	}
	if(bp04 < 2)
		FoeMelee(bp08, bp06);/*contact attack?*/
}

/*combat ai*/
/*C_9F7B*/COM_FoesTurn()
{
	int loc_A, loc_B, loc_C, loc_F;
	unsigned char loc_D, loc_E;

	RefreshView();

	for(loc_A = 15; loc_A >= 0; loc_A --) {
		if(Fighters._tile[loc_A]) {
			if((unsigned char)Fighters._sleeping[loc_A] == 0 || U4_RND1(7) == 0) {
				Fighters._sleeping[loc_A] = 0;
				if((unsigned char)Fighters._tile[loc_A] == TIL_ZORN) {
					spell_effect = 'N';
					spell_turns = 2;
					dspl_Stats();
				} else if((unsigned char)Fighters._tile[loc_A] == TIL_WISP && U4_RND1(7) == 0) {
					do {
						loc_D = U4_RND3(11);
						loc_E = U4_RND3(11);
					} while(!FoeCanMoveTo(loc_D, loc_E, loc_A));
					Combat._npcX[loc_A] = loc_D;
					Combat._npcY[loc_A] = loc_E;
				}
				loc_D = Combat._npcX[loc_A];
				loc_E = Combat._npcY[loc_A];
				loc_F = 99;
				loc_C = -1;
				for(loc_B = Party._members - 1; loc_B >= 0; loc_B --) {
					if(isCharaAlive(loc_B) && Fighters._chtile[loc_B]) {
						register int si;

						si =
							u4_abs((unsigned char)Combat._charaX[loc_B] - loc_D) +
							u4_abs((unsigned char)Combat._charaY[loc_B] - loc_E)
						;
						if(loc_F > si) {
							loc_F = si;
							loc_C = loc_B;
						}
					}
				}
				if(loc_C != -1)
					FoeAct(loc_A, loc_C, loc_F);
			}
			switch(Combat_MAP((unsigned char)Combat._npcY[loc_A], (unsigned char)Combat._npcX[loc_A])) {
				case TIL_FIRE_FIELD: case TIL_LAVA:
					if((unsigned char)Fighters._tile[loc_A] == TIL_LAVA_LIZARD || (unsigned char)Fighters._tile[loc_A] >= TIL_DAEMON)
						break;
				case TIL_POISON_FIELD:
					sound(6);
					COM_DoDamage(loc_A, -1, U4_RND1(0x7f));
				break;
				case TIL_SLEEP_FIELD:
					if(IsUndead((unsigned char)Fighters._tile[loc_A]) == 0 && (unsigned char)Fighters._HP[loc_A] >= u_rand_a()) {
						Fighters._sleeping[loc_A] = 1;
						sound(6);
					}
				break;
			}
		}
	}
	MP_recover();
	Party._moves ++;
	if(spell_turns) {
		if(--spell_turns == 0)
			spell_effect = 7;
	}
}
