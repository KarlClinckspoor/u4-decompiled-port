/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

char /*D_1FD8*/MsgAttackedBy[] = "\nAttacked by ";

/*
 * Can npc move to (x, y), whose map tile is tile? Never onto slow terrain,
 * nor (except whirlpools and twisters) onto another npc (chests excepted) or
 * the party; 1 time out of 4 not back to its previous square (pirate ships
 * excepted). Then by kind: pirate ships sail on water, sea monsters swim up to
 * the shallows, flyers go on water or land, zorns and ghosts go anywhere but
 * water and energy fields, the others walk.
 */
int16 /*C_4E94*/NpcCanMoveTo(npc, x, y, tile)
uint16 npc;
unsigned char x;
unsigned char y;
unsigned char tile;
{
	register int16 other, npc_tile;

	if(CurMode == MOD_BUILDING)
		if(x >= 32 || y >= 32)
			return 0;
	npc_tile = Area._npc._tile[npc];
	if(IsSlowTerrain(tile))
		return 0;
	if(npc_tile != TIL_WHIRLPOOL && npc_tile != TIL_TWISTER) {
		for(other = 31; other >= 0; other --) {
			if(
				Area._npc._tile[other] &&
				Area._npc._x[other] == x &&
				Area._npc._y[other] == y &&
				Area._npc._tile[other] != TIL_CHEST
			) return 0;
		}
		if(Party._x == x && Party._y == y)
			return 0;
	}
	if(
		npc_tile != TIL_PIRATE_W &&
		x == Area._npc._old_x[npc] &&
		y == Area._npc._old_y[npc] &&
		U4_RND1(3) == 0
	) return 0;
	if(npc_tile < TIL_PIRATE_W)
		return IsWalkable(tile);
	if(npc_tile < TIL_TWISTER)
		return (npc_tile == TIL_PIRATE_W)?(tile <= TIL_WATER):(tile <= TIL_SHALLOWS);
	if(npc_tile == TIL_TWISTER || npc_tile == TIL_BAT || npc_tile == TIL_INSECTS || npc_tile == TIL_DAEMON || npc_tile == TIL_DRAGON || npc_tile == TIL_BALRON)
		return (tile <= TIL_SHALLOWS) || IsWalkable(tile);
	if(npc_tile == TIL_ZORN || npc_tile == TIL_GHOST)
		return (tile >= TIL_SWAMP) && (tile != TIL_ENERGY_FIELD);
	return IsWalkable(tile);
}

/*Absolute value of value.*/
int16 /*C_4FBE*/u4_abs(value)
int16 value;
{
	return (value < 0)?-value:value;
}

/*Sign of value: -1, 0 or 1.*/
int16 /*C_4FD7*/u4_sign(value)
int16 value;
{
	return (value < 0)?-1:(value > 0)?1:0;
}

/*
 * Move npc to (x, y) in a towne, remembering its previous square.
 * Probable original bug: the bounds check uses || where && was meant, so it
 * only refuses squares off the map on both axes.
 */
int16 /*C_4FF8*/NpcMoveInTown(npc, x, y)
register int16 npc;
unsigned char x;
unsigned char y;
{
	if((uint16)x < 32 || (uint16)y < 32) {
		Area._npc._old_x[npc] = Area._npc._x[npc];
		Area._npc._x[npc] = x;
		Area._npc._old_y[npc] = Area._npc._y[npc];
		Area._npc._y[npc] = y;
	}
}

/*Move npc to (x, y), remembering its previous square.*/
int16 /*C_5036*/NpcMoveTo(npc, x, y)
register int16 npc;
unsigned char x;
unsigned char y;
{
	Area._npc._old_x[npc] = Area._npc._x[npc];
	Area._npc._x[npc] = x;
	Area._npc._old_y[npc] = Area._npc._y[npc];
	Area._npc._y[npc] = y;
}

/*
 * Outdoors, try to move npc one square in direction (dx, dy), along one axis
 * only: the horizontal step first half of the time, then the vertical one,
 * then the horizontal one. If no step is possible or the destination is out of
 * the loaded map window, retry in a random direction, up to tries more times.
 * Note: the first random retry never gets a negative dy (the (char) cast is
 * on the sign instead of the random number), so it is biased southward.
 */
int16 /*C_5062*/NpcStepOutdoors(npc, dx, dy, tries)
int16 npc;
int16 dx;
int16 dy;
int16 tries;
{
	unsigned char map_col, cur_col;
	unsigned char map_row, cur_row;

	cur_col = Area._npc._x[npc]; map_col = (dx + cur_col) - (WinBlock.x << 4);
	cur_row = Area._npc._y[npc]; map_row = (dy + cur_row) - (WinBlock.y << 4);
	if(map_col  > 31 || map_row > 31) {
		if(tries >= 0)
			NpcStepOutdoors(npc, u4_sign((char)u_rand_a()), (char)u4_sign(u_rand_a()), tries - 1);
		return;
	}
	if(U4_RND1(1) && dx && NpcCanMoveTo(npc, cur_col + dx, cur_row, Area._map.x32x32[map_row-dy][map_col])) {
		NpcMoveTo(npc, cur_col + dx, cur_row);
		return;
	}
	if(dy && NpcCanMoveTo(npc, cur_col, cur_row + dy, Area._map.x32x32[map_row][map_col - dx])) {
		NpcMoveTo(npc, cur_col, cur_row + dy);
		return;
	}
	if(NpcCanMoveTo(npc, cur_col + dx, cur_row, Area._map.x32x32[map_row - dy][map_col])) {
		NpcMoveTo(npc, cur_col + dx, cur_row);
		return;
	}
	if(tries >= 0)
		NpcStepOutdoors(npc, u4_sign((char)u_rand_a()), u4_sign((char)u_rand_a()), tries - 1);
}

/*
 * In a towne, try to move npc one square in direction (dx, dy), along one
 * axis only: half of the time the horizontal step, else the vertical one,
 * else (dy == 0) the horizontal one. When that last one fails, retry in a
 * random direction, up to tries more times (not for _var 0x80 npcs).
 */
int16 /*C_51A7*/NpcStepInTown(npc, dx, dy, tries)
int16 npc;
int16 dx;
int16 dy;
int16 tries;
{
	uint16 npc_x, npc_y;

	npc_x = Area._npc._x[npc];
	npc_y = Area._npc._y[npc];
	/*Area._map.x32x32 is accessed without any out-of-bounds control.
Fortunately, there is some control in NpcCanMoveTo so any faultly-accessed
value will be discarded anyway.
Though, there seem to be a problem with 64-bit builds.
An exception is raised when (dx+npc_x) or (dy+npc_y) are negative.
Masking the sums should fix the problem:
	Area._map.x32x32[npc_y][(dx+npc_x)&0x1f]
	Area._map.x32x32[(dy+npc_y)&0x1f][npc_x]
(issue raised by @plaidpants)*/
	if(U4_RND1(1) && dx) {
		if(NpcCanMoveTo(npc, dx+npc_x, npc_y, Area._map.x32x32[npc_y][dx+npc_x]))
			NpcMoveInTown(npc, dx+npc_x, npc_y);
		return;
	}
	if(dy) {
		if(NpcCanMoveTo(npc, npc_x, dy+npc_y, Area._map.x32x32[dy+npc_y][npc_x]))
			NpcMoveInTown(npc, npc_x, dy+npc_y);
		return;
	}
	if(NpcCanMoveTo(npc, dx+npc_x, npc_y, Area._map.x32x32[npc_y][dx+npc_x])) {
		NpcMoveInTown(npc, dx+npc_x, npc_y);
		return;
	}
	if(Area._npc._var[npc] != 0x80 && tries >= 0)
		NpcStepInTown(npc, u4_sign((char)u_rand_a()), u4_sign((char)u_rand_a()), tries - 1);
}

/*
 * Move the towne's npcs according to their _var: 0 stays, positive wanders at
 * random (half of the time), negative follows the party, and 0xff attacks
 * when next to it.
 */
int16 /*C_5293*/MoveTownNpcs()
{
	int16 dx, dy;

	{register int16 npc; for(npc = 31; npc >= 0; npc --) {
		if(Area._npc._tile[npc] && Area._npc._var[npc]) {
			if((char)Area._npc._var[npc] >= 0) {
				/*random mode*/
				if(U4_RND1(1))
					NpcStepInTown(npc, u4_sign((char)u_rand_a()), u4_sign((char)u_rand_a()), 2);
			} else {
				/*following mode*/
				dx = u4_sign((char)(Party._x - Area._npc._x[npc]));
				dy = u4_sign((char)(Party._y - Area._npc._y[npc]));
				if(u4_abs((char)(Party._x - Area._npc._x[npc])) + u4_abs((char)(Party._y - Area._npc._y[npc])) >= 2)
					NpcStepInTown(npc, dx, dy, 2);
				else if(Area._npc._var[npc] == 0xff) {
					/*agressive mode*/
					/*-- attacked by --*/
					RefreshView();
					u4_puts(MsgAttackedBy); u4_puts(CreatureName(Area._npc._tile[npc]));
					Gra_CR();
					StartCombat(npc);
				}
			}
		}
	}}
}

/*
 * Vector from npc to the party: *dist_x, *dist_y and their signs *dx, *dy.
 * Returns the distance |dist_x| + |dist_y|.
 */
int16 /*C_535B*/NpcDistance(npc, dx, dy, dist_x, dist_y)
int16 npc;
int16 *dx;
int16 *dy;
int16 *dist_x;
int16 *dist_y;
{
	*dist_y = (char)(Party._y - Area._npc._y[npc]);
	*dy = u4_sign(*dist_y);
	*dist_x = (char)(Party._x - Area._npc._x[npc]);
	*dx = u4_sign(*dist_x);
	return u4_abs(*dist_x) + u4_abs(*dist_y);
}

/*
 * Pirate ship npc sails to (x, y) if it can and the wind allows its heading;
 * if it cannot go there, it turns at random.
 */
int16 /*C_53AF*/ShipSail(npc, x, y)
register int16 npc;
unsigned char x;
unsigned char y;
{
	if(NpcCanMoveTo(npc, x, y, Area._map.x32x32[y-WinBlock.y*16][x-WinBlock.x*16])) {
		if(WindAllows(Area._npc._gtile[npc] & 3)) {
			Area._npc._old_x[npc] = Area._npc._x[npc];
			Area._npc._x[npc] = x;
			Area._npc._old_y[npc] = Area._npc._y[npc];
			Area._npc._y[npc] = y;
		}
	} else {
		Area._npc._gtile[npc] = ((u4_sign((char)u_rand_a()) + Area._npc._gtile[npc]) & 3) | TIL_PIRATE_W;
	}
}

/*
 * Pirate ship npc: when farther than 5 squares (or 1 time out of 4), turn to
 * face the party along the longer axis; otherwise sail one square ahead.
 */
int16 /*C_5443*/ShipSteer(npc)
register int16 npc;
{
	int16 dx, dy, dist_x, dist_y;

	if(NpcDistance(npc, &dx, &dy, &dist_x, &dist_y) > 5 || U4_RND1(3) == 0) {
		if(u4_abs(dist_x) < u4_abs(dist_y)) {
			if(dist_y >= 0)
				Area._npc._gtile[npc] = TIL_PIRATE_S;
			else
				Area._npc._gtile[npc] = TIL_PIRATE_N;
		} else {
			if(dist_x >= 0)
				Area._npc._gtile[npc] = TIL_PIRATE_E;
			else
				Area._npc._gtile[npc] = TIL_PIRATE_W;
		}
	} else if(Area._npc._gtile[npc] == TIL_PIRATE_W) {
		ShipSail(npc, Area._npc._x[npc] - 1, Area._npc._y[npc]);
	} else if(Area._npc._gtile[npc] == TIL_PIRATE_N) {
		ShipSail(npc, Area._npc._x[npc], Area._npc._y[npc] - 1);
	} else if(Area._npc._gtile[npc] == TIL_PIRATE_E) {
		ShipSail(npc, Area._npc._x[npc]+1, Area._npc._y[npc]);
	} else {
		ShipSail(npc, Area._npc._x[npc], Area._npc._y[npc]+1);
	}
}

/*
 * Pirate ship npc: sail in direction (dx, dy) if it is heading that way,
 * else steer.
 */
int16 /*C_5500*/ShipMove(npc, dx, dy)
register int16 npc;
int16 dx;
int16 dy;
{
	if(Area._npc._gtile[npc] == TIL_PIRATE_W) {
		if(dx < 0) {
			ShipSail(npc, Area._npc._x[npc]+dx, Area._npc._y[npc]);
			return;
		}
	} else if(Area._npc._gtile[npc] == TIL_PIRATE_N) {
		if(dy < 0) {
			ShipSail(npc, Area._npc._x[npc], Area._npc._y[npc]+dy);
			return;
		}
	} else if(Area._npc._gtile[npc] == TIL_PIRATE_E) {
		if(dx > 0) {
			ShipSail(npc, Area._npc._x[npc]+dx, Area._npc._y[npc]);
			return;
		}
	} else {
		if(dy > 0) {
			ShipSail(npc, Area._npc._x[npc], Area._npc._y[npc]+dy);
			return;
		}
	}
	ShipSteer(npc);
}

/*
 * npc shoots a missile showing as tile in direction (dx, dy), up to 3
 * squares, through any terrain. Returns 1 if it reaches the party. Another npc
 * on the way stops it, and is destroyed if it is #8-#31, else 1 time out of 4.
 */
int16 /*C_5569*/NpcShotHitsParty(npc, dx, dy, tile)
int16 npc;
unsigned char dx;
unsigned char dy;
unsigned char tile;
{
	unsigned char x, y;
	int16 old_tile, step, victim;

	x = Area._npc._x[npc] + dx;
	y = Area._npc._y[npc] + dy;
	sound(SND_SHOOT);
	for(step = 1; step <= 3; x += dx, y += dy, step++) {
		if(Party._x == x && Party._y == y)
			return 1;
		if((victim = FindNpcAt(31, x, y)) != -1) {
			sound(SND_HIT);
			if(victim >= 8 || U4_RND1(3) == 0) {
				Area._npc._tile[victim] = Area._npc._gtile[victim] = 0;
			}
			return 0;
		}
		{
			register unsigned char *p_square;

			p_square = &(Area._map.x32x32[y - (WinBlock.y*16)][x - (WinBlock.x*16)]);
			old_tile = *p_square;
			*p_square = tile;
			RefreshView();
			*p_square = old_tile;
		}
	}
	return 0;
}

/*npc shoots tile in direction (dx, dy); if the party is reached, everybody is hurt.*/
int16 /*C_564B*/NpcFireAtParty(npc, dx, dy, tile)
int16 npc;
int16 dx;
int16 dy;
int16 tile;
{
	if(NpcShotHitsParty(npc, dx, dy, tile) == 0) {
		hit_tile = 0;
		return;
	}
	RefreshView();
	hit_y = hit_x = 5;
	hit_tile = TIL_HIT_FLASH;
	DrawView();
	DamageAllParty();
	hit_tile = 0;
	RefreshView();
}

/*
 * Pirate ship npc near the party (whose direction is (dx, dy)): fires a
 * broadside when the party is straight abeam, else moves.
 */
int16 /*C_568F*/ShipAttack(npc, dx, dy)
int16 npc;
int16 dx;
int16 dy;
{
	if(Area._npc._gtile[npc] == TIL_PIRATE_W || Area._npc._gtile[npc] == TIL_PIRATE_E)
		if(dx)
			ShipMove(npc, dx, dy);
		else
			NpcFireAtParty(npc, dx, dy, TIL_MISSILE);
	else
		if(dy)
			ShipMove(npc, dx, dy);
		else
			NpcFireAtParty(npc, dx, dy, TIL_MISSILE);
}

/*Is npc outside the loaded 32x32 map window?*/
int16 /*C_56D3*/NpcOffWindow(npc)
int16 npc;
{
	return
		(uint16)((Area._npc._x[npc] - WinBlock.x*16)) >= 32 ||
		(uint16)((Area._npc._y[npc] - WinBlock.y*16)) >= 32
	;
}

/*
 * Move the outdoor monsters (npcs #0-#7; those out of the map window vanish).
 * One next to the party attacks; pirate ships within 3 squares fight, else
 * chase; serpents, lava lizards, hydras, dragons and balrons within 4 squares
 * shoot fire half of the time; whirlpools, twisters and (3 times out of 4)
 * non-evil monsters wander, the others come towards the party.
 */
int16 /*C_5712*/MoveWorldNpcs()
{
	int16 dx, dy, dist_x, dist_y;
	register int16 monster;

	monster = 7; do {
		int16 dist;
		register int16 npc_tile;

		npc_tile = Area._npc._tile[monster];
		if(npc_tile == 0)
			continue;
		/*is out of range?*/
		if(NpcOffWindow(monster)) {
			Area._npc._tile[monster] = Area._npc._gtile[monster] = 0;
			continue;
		}
		/* */
		if(npc_tile != TIL_WHIRLPOOL && npc_tile != TIL_TWISTER) {
			dist = NpcDistance(monster, &dx, &dy, &dist_x, &dist_y);
			if(dist < 2) {
				/*-- attacked by --*/
				RefreshView();
				u4_puts(MsgAttackedBy); u4_puts(CreatureName(npc_tile));
				Gra_CR();
				StartCombat(monster);
				continue;
			}
		}
		if(npc_tile == TIL_PIRATE_W) {
			if(dist < 4)
				ShipAttack(monster, dx, dy);
			else
				ShipMove(monster, dx, dy);
			continue;
		}
		if(npc_tile == TIL_SERPENT || npc_tile == TIL_LAVA_LIZARD || npc_tile >= TIL_HYDRA) {
			if(u4_abs(dist_x) < 5 && u4_abs(dist_y) < 5 && U4_RND1(1))
				NpcFireAtParty(monster, dx, dy, TIL_HIT_FLASH);
		}
		if(npc_tile == TIL_WHIRLPOOL || npc_tile == TIL_TWISTER || (IsNonEvil(npc_tile) && U4_RND1(3)))
			NpcStepOutdoors(monster, u4_sign((char)u_rand_a()), u4_sign((char)u_rand_a()), 1);
		else
			NpcStepOutdoors(monster, dx, dy, 2);
	} while(--monster >= 0);
}

/*Move the npcs (in a towne or outdoors).*/
int16 /*C_5834*/MoveNpcs()
{
	if(CurMode == MOD_BUILDING)
		MoveTownNpcs();
	else if(CurMode == MOD_OUTDOORS)
		MoveWorldNpcs();
}

/*
 * Outdoors, 1 time out of 16: in each free slot of npcs #0-#3, try to put a
 * monster on a random square of the map window at least 6 squares away from
 * the party on both axes. At sea (1 time out of 8), a sea creature (pirate
 * ships twice as likely); on grass, scrub, forest or hills, a land monster
 * from the orc up to the balron depending on the number of moves.
 */
int16 /*C_5851*/SpawnMonsters()
{
	register int16 npc;
	register unsigned char *ptile;
	unsigned char pos_y, pos_x;

	if(CurMode != MOD_OUTDOORS || U4_RND1(15))
		return;
	for(npc = 3; npc >= 0; npc--) {
		if(Area._npc._tile[npc] != 0)
			continue;
		pos_x = U4_RND1(31);
		pos_y = U4_RND1(31);
		ptile = &(Area._map.x32x32[pos_y][pos_x]);
		if(u4_abs(pos_x - WinPos.x) >= 6 && u4_abs(pos_y - WinPos.y) >= 6) {
			if(*ptile < TIL_SHALLOWS) {/*at sea*/
				if(U4_RND1(7) == 0) {
					Area._npc._x[npc] = pos_x + WinBlock.x*16;
					Area._npc._y[npc] = pos_y + WinBlock.y*16;
					Area._npc._tile[npc] = Area._npc._gtile[npc] = (unsigned char)TIL_PIRATE_W + (U4_RND1(7) * 2);
					if(Area._npc._gtile[npc] == TIL_PIRATE_E)
						Area._npc._tile[npc] = Area._npc._gtile[npc] = TIL_PIRATE_W;
				}
			} else if(*ptile >= TIL_GRASS && *ptile < TIL_MOUNTAINS) {/*on earth*/
				uint16 level_mask;

				Area._npc._x[npc] = pos_x + WinBlock.x*16;
				Area._npc._y[npc] = pos_y + WinBlock.y*16;
				if(Party._moves > 100000L)
					level_mask = 15;
				else if(Party._moves < 10000L)
					level_mask = 3;
				else if(Party._moves < 20000L)
					level_mask = 7;
				else if (Party._moves >= 30000L)
					level_mask = 15;
				else
					level_mask = 7;
/*the original code of the expression below:
:599A E8D4BD    CALL	u_rand_a
:599D 8946F4    MOV	[BP-0Ch],AX
:59A0 E8CEBD    CALL	u_rand_a
:59A3 8A4EF6    MOV	CL,[BP-0Ah]
:59A6 22C8      AND	CL,AL
:59A8 224EF4    AND	CL,[BP-0Ch]
:59AB D0E1      SHL	CL,1
:59AD D0E1      SHL	CL,1
:59AF 80C1C0    ADD	CL,0C0h
:59B2 888C428B  MOV	__8B42[SI],CL
:59B6 888CA28B  MOV	__8B42[SI+60h],CL*/
				Area._npc._tile[npc] =
				Area._npc._gtile[npc] =
					U4_RND1(U4_RND1(level_mask)) * 4 + TIL_ORC;
			}
		}
	}
}
