/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*
 * Can dungeon monster npc move to (x, y) on the party's level? The square must hold
 * no monster, and not be a trap or fountain (0x80-0x9F), a field, a room or a wall;
 * going back to its previous square is allowed only one time in 8.
 */
/*C_93A5*/DNG_MonsterCanMove(x, y, npc)
unsigned char x;
unsigned char y;
int npc;
{
	unsigned char tile;

	tile = Area._map.x8x8x8[Party._z][y][x];
	if((tile & 0x0f) == 0) {
		if(
			(tile & 0xe0) != 0x80 &&
			(tile & 0xf0) != 0xa0 &&
			(tile & 0xf0) != 0xd0 &&
			(tile & 0xf0) != 0xf0
		) {
			if(U4_RND1(7) == 0 || Area._npc._old_x[npc] != x || Area._npc._old_y[npc] != y)
				return 1;
		}
	}
	return 0;
}

/*
 * Move the monsters of the party's level in the dungeon, at random (they do not
 * chase the party), except the reapers, the mimics and a monster on the party's
 * square. Each one makes up to 8 tries with a random direction (incH, incV: -1, 0
 * or 1), horizontally (1 time in 2), vertically, then horizontally. The monster's
 * number is kept in the low nibble of the map's tile.
 */
/*C_9414*/DNG_MoveMonsters()
{
	int incH, npc, incV, retry;
	unsigned char toX, toY;
	
	for(npc = 31; npc >= 0; npc --) {
		if(
			Area._npc._tile[npc] != 0 &&
			Area._npc._var[npc] == Party._z &&
			Area._npc._tile[npc] != TIL_REAPER &&
			Area._npc._tile[npc] != TIL_MIMIC
		) {
			if(Party._x != Area._npc._x[npc] || Party._y != Area._npc._y[npc]) {
				for(retry = 7; retry >= 0; retry --) {
					incH = u4_sign((char)u_rand_a());
					incV = u4_sign((char)u_rand_a());
					if(U4_RND1(1) && incH != 0) {
						toX = (Area._npc._x[npc] + incH) & 7;
						toY = Area._npc._y[npc];
						if(DNG_MonsterCanMove(toX, toY, npc))
							break;
					}
					if(incV) {
						toX = Area._npc._x[npc];
						toY = (Area._npc._y[npc] + incV) & 7;
						if(DNG_MonsterCanMove(toX, toY, npc))
							break;
					}
					toX = (Area._npc._x[npc] + incH) & 7;
					toY = Area._npc._y[npc];
					if(DNG_MonsterCanMove(toX, toY, npc))
						break;
				}
				if(retry != -1) {
					Area._map.x8x8x8[Party._z][toY][toX] |= (Area._npc._tile[npc] - TIL_RAT) / 4 + 1;
					Area._map.x8x8x8[Party._z][Area._npc._y[npc]][Area._npc._x[npc]] &= 0xf0;
					Area._npc._old_x[npc] = Area._npc._x[npc];
					Area._npc._old_y[npc] = Area._npc._y[npc];
					Area._npc._x[npc] = toX;
					Area._npc._y[npc] = toY;
				}
			}
		}
	}
	RefreshView();
}

/*
 * Maybe create monsters on the party's level of the dungeon: level z uses NPC
 * slots 4z and 4z+1. A monster appears in a free slot at a random empty corridor
 * square that is in neither the party's row nor its column (&&: probably meant
 * "not on the party's square"); it is one of the 4 monsters from TIL_RAT + 4z on
 * (no mimics).
 */
/*C_95AA*/DNG_SpawnMonsters()
{
	int npc;
	unsigned char spawnCol, monster, spawnRow;

	for(npc = Party._z * 4 + 1; npc >= 0; npc --) {
		if((npc >> 2) < Party._z)
			break;
		if(Area._npc._tile[npc] == 0) {
			spawnCol = U4_RND1(7);
			spawnRow = U4_RND1(7);
			if(Party._x != spawnCol && Party._y != spawnRow) {
				if(Area._map.x8x8x8[Party._z][spawnRow][spawnCol] == 0) {
					monster = (U4_RND1(3) + Party._z) * 4 + TIL_RAT;
					if(monster != TIL_MIMIC) {
						Area._npc._tile[npc] = monster;
						Area._npc._x[npc] = Area._npc._old_x[npc] = spawnCol;
						Area._npc._y[npc] = Area._npc._old_y[npc] = spawnRow;
						Area._npc._var[npc] = Party._z;
						Area._map.x8x8x8[Party._z][spawnRow][spawnCol] = ((monster - TIL_RAT) >> 2) + 1;
					}
				}
			}
		}
	}
}

/*
 * The living character at (hit_x, hit_y) on the combat map, or -1.
 */
/*C_9683*/CharaAtHit()
{
	register int chara;

	for(chara = Party._members; --chara >= 0;  ) {
		if(isCharaAlive(chara) && hit_x == Combat._charaX[chara] && hit_y == Combat._charaY[chara])
			return chara;
	}
	return -1;
}

/*
 * Foe foe hits character chara: damage from 0 to a quarter of the foe's base HP,
 * then oddly "converted" as if it were BCD: (d >> 4) * 10 + d % 10 (probably a
 * leftover of a BCD version; with d % 16 it would have been a real conversion).
 * A killed character raises Sacrifice.
 * Returns 1 if the character was killed.
 */
/*C_96B9*/FoeHitsChara(chara, foe)
int chara;
int foe;
{
	unsigned damage;

	Gra_InvertStatusLine(chara);
	DrawCombat();
	sound(SND_HURT);
	hit_tile = 0;
	DrawCombat();
	damage = U4_RND4(MonsterMaxHP[MonsterIndex((unsigned char)Fighters._tile[foe])] >> 2);
	damage = (damage >> 4) * 10 + (damage % 10);
	Gra_InvertStatusLine(chara);
	if(hitChara(chara, damage) == 0) {
		dspl_Stats();
		return 0;
	}
	u4_puts(Party.chara[chara]._name);
	u4_puts(/*D_29CA*/" is Killed!\n");
	karma_inc(&(Party._sacri), 1);
	dspl_Stats();
	CheckPartyAlive();

	return 1;
}

/*
 * Character chara is struck without damage (a missile with a special effect).
 */
/*C_9764*/CharaStruck(chara)
int chara;
{
	Gra_InvertStatusLine(chara);
	DrawCombat();
	sound(SND_HURT);
	Gra_InvertStatusLine(chara);
	hit_tile = 0;
}

/*
 * Foe foe shoots missile (a tile, see FoeMissile) in direction (dx, dy): the missile
 * flies until it hits a character or an obstacle. A missed lava ball leaves lava
 * where it stops. The poison and sleep missiles fail one time in 2 or on
 * characters not in good health; the others hurt.
 */
/*C_978C*/FoeShoot(missile, foe, dx, dy)
unsigned char missile;
int foe;
int dx;
int dy;
{
	register int chara;

	hit_tile = missile;
	hit_x = Combat._npcX[foe];
	hit_y = Combat._npcY[foe];
	sound(SND_SHOOT);
	chara = -1;
	do {
		if(COM_MissileStep(dx, dy) == 0)
			break;
		DrawCombat();
	} while((chara = CharaAtHit()) == -1);
	if(chara == -1) {
		if(hit_tile == TIL_LAVA)
			Combat_MAP(hit_y, hit_x) = TIL_LAVA;
		hit_tile = 0;
		return;
	}
	Gra_CR();
	u4_puts(Party.chara[chara]._name);
	u4_putc(' ');
	switch(hit_tile) {
		case TIL_POISON_FIELD:
			u4_puts(/*D_29D7*/"Poisoned!\n");
			CharaStruck(chara);
			if(U4_RND1(1) || Party.chara[chara]._stat != 'G')
				u4_puts(/*D_29E2*/"Failed.\n");
			else
				Party.chara[chara]._stat = 'P';
		break;
		case TIL_ENERGY_FIELD:
			u4_puts(/*D_29EB*/"Electrified!\n");
			FoeHitsChara(chara, foe);
		break;
		case TIL_FIRE_FIELD:
			u4_puts(/*D_29F9*/"Fiery Hit!\n");
			FoeHitsChara(chara, foe);
		break;
		case TIL_SLEEP_FIELD:
			u4_puts(/*D_2A05*/"Slept!\n");
			CharaStruck(chara);
			if(U4_RND1(1) || Party.chara[chara]._stat != 'G')
				u4_puts(/*D_2A0D*/"Failed.\n");
			else {
				Party.chara[chara]._stat = 'S';
				Fighters._chtile[chara] = TIL_SLEEPING;
			}
		break;
		case TIL_LAVA:
			u4_puts(/*D_2A16*/"Lava Hit!\n");
			FoeHitsChara(chara, foe);
		break;
		case TIL_MAGIC_BOLT:
			u4_puts(/*D_2A21*/"Magical Hit!\n");
			FoeHitsChara(chara, foe);
		break;
		default:
			u4_puts(/*D_2A2F*/"Hit!\n");
			FoeHitsChara(chara, foe);
	}
	hit_tile = 0;
	dspl_Stats();
}

/*
 * Can foe foe move to (x, y) on the combat map? Not on slow terrain; off the map
 * only when fleeing (HP < 24); back to its previous square only 1 time in 4; not
 * onto another foe or a character. Then it depends on the foe: humans walk, the
 * sea monsters (but the twister) swim, the flying ones (insects, twister, bat,
 * daemon, dragon, balron) go over water and walkable tiles, ghosts and zorns go
 * anywhere but water and energy fields.
 * Note: the map is read before checking that (x, y) is on it.
 */
/*C_98E4*/FoeCanMoveTo(x, y, foe)
unsigned char x;
unsigned char y;
int foe;
{
	unsigned char tile;
	int i;

	tile = Combat_MAP(y, x);
	if(IsSlowTerrain(tile))
		return 0;
	if(x >= 11 || y >= 11)
		return ((unsigned char)Fighters._HP[foe] < 24);
	if(Fighters._x[foe] == x && Fighters._y[foe] == y && U4_RND1(3))
		return 0;

	for(i = 15; i >= 0; i --)
		if(Fighters._tile[i] && Combat._npcX[i] == x && Combat._npcY[i] == y)
			return 0;
	for(i = 7; i >= 0; i --)
		if(Fighters._chtile[i] && Combat._charaX[i] == x && Combat._charaY[i] == y)
			return 0;

	if((unsigned char)Fighters._tile[foe] < 0x80)
		return IsWalkable(Combat_MAP(y, x));
	if((unsigned char)Fighters._tile[foe] < TIL_TWISTER)
		return (tile < TIL_SWAMP);
	switch((unsigned char)Fighters._gtile[foe]) {
		case TIL_INSECTS:
		case TIL_TWISTER:
		case TIL_BAT:
		case TIL_DAEMON:
		case TIL_DRAGON:
		case TIL_BALRON:
			return (tile < TIL_SWAMP) || IsWalkable(tile);
		case TIL_GHOST:
		case TIL_ZORN:
			return (tile >= TIL_SWAMP) && (tile != TIL_ENERGY_FIELD);
		default:
			return IsWalkable(tile);
	}
}

/*
 * The missile (tile) a foe of tile tile can shoot, 0 for none.
 */
unsigned char /*C_9A41*/FoeMissile(tile)
unsigned char tile;
{
	switch(tile) {
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

/*
 * Under the Jinx spell, a foe moving onto another foe at (x, y) hits it instead
 * (0-63 damage). Returns 1 if that happened.
 */
/*C_9B03*/CheckJinx(x, y)
unsigned char x;
unsigned char y;
{
	int victim;

	if(spell_effect == 'J') {
		if((victim = COM_GetFighterId(x, y)) != -1) {
			hit_tile = TIL_HIT_FLASH;
			hit_x = x;
			hit_y = y;
			DrawCombat();
			hit_tile = 0;
			sound(SND_HIT);
			COM_DoDamage(victim, -1, U4_RND1(0x3f));
			DrawCombat();
			return 1;
		}
	}
	return 0;
}

/*
 * A gremlin steals 25 food (2500 hundredths).
 */
/*C_9B6B*/StealFood()
{
	Party._food = u4_max(Party._food - 2500, 0L);
	dspl_Stats();
	sound(SND_FAIL);
}

/*
 * A rogue steals 0-63 gold, one time in 4.
 */
/*C_9BA6*/StealGold()
{
	register int amount;

	if(U4_RND1(3) == 0) {
		amount = U4_RND1(0x3f);
		Party._gold = u4_max(Party._gold - amount, 0);
		dspl_Stats();
		sound(SND_FAIL);
	}
}

/*
 * Foe foe attacks the adjacent character chara. It misses one time in 2 under the
 * Protection spell, or if the armour wins (ArmorDefense >= random 0-255).
 * Returns 1 if the character was hit and survived, else 0 (the result is unused).
 */
/*C_9BE5*/FoeMelee(foe, chara)
int foe;
int chara;
{
	hit_x = Combat._charaX[chara];
	hit_y = Combat._charaY[chara];
	hit_tile = TIL_MISSILE;
	DrawCombat();
	sound(SND_FOE_ATTACK);
	hit_tile = 0;
	if((spell_effect == 'P' && U4_RND1(1)) || ArmorDefense[Party.chara[chara]._armor] >= u_rand_a()) {
		DrawCombat();
	} else if(FoeHitsChara(chara, foe) == 0) {
		return 1;
	}
	return 0;
}

/*
 * Foe foe leaves the combat map; letting a non-evil foe flee raises Compassion and
 * Justice.
 */
/*C_9C56*/FoeFlees(foe)
int foe;
{
	Gra_CR();
	u4_puts(CreatureName((unsigned char)Fighters._tile[foe]));
	u4_puts(/*D_2A35*/" Flees!\n");
	if(IsNonEvil((unsigned char)Fighters._tile[foe])) {
		karma_inc(&(Party._compa), 1);
		karma_inc(&(Party._justi), 1);
	}
	sound(SND_FAIL);
	Fighters._tile[foe] = Fighters._HP[foe] = 0;
}

/*
 * The turn of foe foe, whose nearest character is chara, dist squares away:
 * - a mimic looks like a chest while the characters are 5 or more squares away
 *   (and then does not shoot);
 * - 1 time in 4 it shoots its missile if it has one (no magic bolt under Negate);
 * - 1 time in 4 a balron or reaper casts sleep on the party (not under Negate):
 *   each character in good health falls asleep 1 time in 2;
 * - with less than 24 HP it flees, else it moves toward chara, or attacks him when
 *   adjacent (gremlins and rogues steal first); mimics and reapers never move.
 *   It tries twice (on the second try in a random direction): horizontally
 *   (1 time in 2), vertically, horizontally. Moving off the map is fleeing; a
 *   fleeing foe that cannot move but is adjacent attacks.
 */
/*C_9CBC*/FoeAct(foe, chara, dist)
int foe;
int chara;
int dist;
{
	int incH, attempts, incV;
	unsigned char toX, toY;

	incH = u4_sign((unsigned char)Combat._charaX[chara] - (unsigned char)Combat._npcX[foe]);
	incV = u4_sign((unsigned char)Combat._charaY[chara] - (unsigned char)Combat._npcY[foe]);
	/*Mimic*/
	if((unsigned char)Fighters._tile[foe] == TIL_MIMIC) {
		Fighters._gtile[foe] = TIL_CHEST;
		if(dist >= 5)
			goto C_9D51;
		Fighters._gtile[foe] = TIL_MIMIC;
	}
	/*attempt projectile*/
	if(U4_RND1(3) == 0) {
		int missile;

		if((missile = FoeMissile((unsigned char)Fighters._tile[foe])) != 0) {
			if(missile != TIL_MAGIC_BOLT || spell_effect != 'N') {
				FoeShoot(missile, foe, incH, incV);
				RefreshView();
				return;
			}
		}
	}
	/* */
C_9D51:
	/*balron or reaper*/
	if(U4_RND1(3) == 0) {
		if((unsigned char)Fighters._tile[foe] == TIL_BALRON || (unsigned char)Fighters._tile[foe] == TIL_REAPER) {
			if(spell_effect != 'N') {
				RefreshView();
				Gra_InvertView(); sound(SND_MAGIC, 0x80); Gra_InvertView();
				/*attempts is just a character index here*/
				for(attempts = Party._members - 1; attempts >= 0; attempts --) {
					if(Fighters._chtile[attempts] != 0 && Party.chara[attempts]._stat == 'G' && U4_RND1(1)) {
						Party.chara[attempts]._stat = 'S';
						Fighters._chtile[attempts] = TIL_SLEEPING;
					}
				}
				dspl_Stats();
				return;
			}
		}
	}
	/* */
	if((unsigned char)Fighters._HP[foe] < 24) {
		incH = -incH;
		incV = -incV;
C_9DE5:
		if((unsigned char)Fighters._tile[foe] == TIL_MIMIC || (unsigned char)Fighters._tile[foe] == TIL_REAPER)
			return;
	} else if(dist >= 2) {
		goto C_9DE5;
	} else {
		/*contact?*/
		switch((unsigned char)Fighters._tile[foe]) {
			case TIL_GREMLIN: StealFood(); break;
			case TIL_ROGUE: StealGold(); break;
		}
		FoeMelee(foe, chara);
		return;
	}
	/* */
	for(attempts = 2; attempts > 0; attempts --) {
		if(U4_RND1(1) && incH != 0) {
			toX = (unsigned char)Combat._npcX[foe] + incH;
			toY = (unsigned char)Combat._npcY[foe];
			if(CheckJinx(toX, toY))
				return;
			if(FoeCanMoveTo(toX, toY, foe))
				break;
		}
		if(incV != 0) {
			toX = (unsigned char)Combat._npcX[foe];
			toY = (unsigned char)Combat._npcY[foe] + incV;
			if(CheckJinx(toX, toY))
				return;
			if(FoeCanMoveTo(toX, toY, foe))
				break;
		}
		if(incH != 0) {
			toX = (unsigned char)Combat._npcX[foe] + incH;
			toY = (unsigned char)Combat._npcY[foe];
			if(CheckJinx(toX, toY))
				return;
			if(FoeCanMoveTo(toX, toY, foe))
				break;
		}
		incH = u4_sign((char)u_rand_a());
		incV = u4_sign((char)u_rand_a());
	}
	if(attempts != 0) {
		Fighters._x[foe] = Combat._npcX[foe];
		Fighters._y[foe] = Combat._npcY[foe];
		Combat._npcX[foe] = toX;
		Combat._npcY[foe] = toY;
		if(toX >= 11 || toY >= 11) {
			FoeFlees(foe);
			return;
		}
		DrawCombat();
		return;
	}
	if(dist < 2)
		FoeMelee(foe, chara);
}

/*
 * The foes' turn in combat. Each awake foe (a sleeping one wakes up 1 time in 8)
 * acts against its nearest character (zorns also negate magic for 2 turns, wisps
 * teleport 1 time in 8). Then the field under each foe acts: fire field and lava
 * (except for lava lizards, daemons, hydras, dragons and balrons) and poison field
 * do 0-127 damage, a sleep field puts to sleep a living foe that fails a check
 * against its HP. Then the end of the turn: magic points, moves, spell effects
 * (7: none).
 */
/*C_9F7B*/COM_FoesTurn()
{
	int foeId, chara, nearest, bestDist;
	unsigned char posX, posY;

	RefreshView();

	for(foeId = 15; foeId >= 0; foeId --) {
		if(Fighters._tile[foeId]) {
			if((unsigned char)Fighters._sleeping[foeId] == 0 || U4_RND1(7) == 0) {
				Fighters._sleeping[foeId] = 0;
				if((unsigned char)Fighters._tile[foeId] == TIL_ZORN) {
					spell_effect = 'N';
					spell_turns = 2;
					dspl_Stats();
				} else if((unsigned char)Fighters._tile[foeId] == TIL_WISP && U4_RND1(7) == 0) {
					do {
						posX = U4_RND3(11);
						posY = U4_RND3(11);
					} while(!FoeCanMoveTo(posX, posY, foeId));
					Combat._npcX[foeId] = posX;
					Combat._npcY[foeId] = posY;
				}
				posX = Combat._npcX[foeId];
				posY = Combat._npcY[foeId];
				bestDist = 99;
				nearest = -1;
				for(chara = Party._members - 1; chara >= 0; chara --) {
					if(isCharaAlive(chara) && Fighters._chtile[chara]) {
						register int dist;/*Manhattan distance*/

						dist =
							u4_abs((unsigned char)Combat._charaX[chara] - posX) +
							u4_abs((unsigned char)Combat._charaY[chara] - posY)
						;
						if(bestDist > dist) {
							bestDist = dist;
							nearest = chara;
						}
					}
				}
				if(nearest != -1)
					FoeAct(foeId, nearest, bestDist);
			}
			switch(Combat_MAP((unsigned char)Combat._npcY[foeId], (unsigned char)Combat._npcX[foeId])) {
				case TIL_FIRE_FIELD: case TIL_LAVA:
					if((unsigned char)Fighters._tile[foeId] == TIL_LAVA_LIZARD || (unsigned char)Fighters._tile[foeId] >= TIL_DAEMON)
						break;
					/*no break*/
				case TIL_POISON_FIELD:
					sound(SND_HIT);
					COM_DoDamage(foeId, -1, U4_RND1(0x7f));
				break;
				case TIL_SLEEP_FIELD:
					if(IsUndead((unsigned char)Fighters._tile[foeId]) == 0 && (unsigned char)Fighters._HP[foeId] >= u_rand_a()) {
						Fighters._sleeping[foeId] = 1;
						sound(SND_HIT);
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
