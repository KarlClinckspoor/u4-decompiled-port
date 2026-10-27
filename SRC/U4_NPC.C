/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

char /*D_1FD8*/MsgAttackedBy[] = "\nAttacked by ";

/*can npc move?*/
/*C_4E94*/NpcCanMoveTo(bp0a, bp08, bp06, bp04)
unsigned bp0a;
unsigned char bp08;/*new x*/
unsigned char bp06;/*new y*/
unsigned char bp04;
{
	register int si, di;

	if(CurMode == MOD_BUILDING)
		if(bp08 >= 32 || bp06 >= 32)
			return 0;
	di = Area._npc._tile[bp0a];
	if(IsSlowTerrain(bp04))
		return 0;
	if(di != TIL_WHIRLPOOL && di != TIL_TWISTER) {
		for(si = 31; si >= 0; si --) {
			if(
				Area._npc._tile[si] &&
				Area._npc._x[si] == bp08 &&
				Area._npc._y[si] == bp06 &&
				Area._npc._tile[si] != TIL_CHEST
			) return 0;
		}
		if(Party._x == bp08 && Party._y == bp06)
			return 0;
	}
	if(
		di != TIL_PIRATE_W &&
		bp08 == Area._npc._old_x[bp0a] &&
		bp06 == Area._npc._old_y[bp0a] &&
		U4_RND1(3) == 0
	) return 0;
	if(di < TIL_PIRATE_W)
		return IsWalkable(bp04);
	if(di < TIL_TWISTER)
		return (di == TIL_PIRATE_W)?(bp04 <= TIL_WATER):(bp04 <= TIL_SHALLOWS);
	if(di == TIL_TWISTER || di == TIL_BAT || di == TIL_INSECTS || di == TIL_DAEMON || di == TIL_DRAGON || di == TIL_BALRON)
		return (bp04 <= TIL_SHALLOWS) || IsWalkable(bp04);
	if(di == TIL_ZORN || di == TIL_GHOST)
		return (bp04 >= TIL_SWAMP) && (bp04 != TIL_ENERGY_FIELD);
	return IsWalkable(bp04);
}

/*C_4FBE*/u4_abs(bp04)
int bp04;
{
	return (bp04 < 0)?-bp04:bp04;
}

/*test parameter's sign
<  0 -> -1
== 0 ->  0
>  0 ->  1*/
/*C_4FD7*/u4_sign(bp04)
int bp04;
{
	return (bp04 < 0)?-1:(bp04 > 0)?1:0;
}

/*C_4FF8*/NpcMoveInTown(si/*bp08*/, bp06, bp04)
register int si;
unsigned char bp06;
unsigned char bp04;
{
	if((unsigned)bp06 < 32 || (unsigned)bp04 < 32) {
		Area._npc._old_x[si] = Area._npc._x[si];
		Area._npc._x[si] = bp06;
		Area._npc._old_y[si] = Area._npc._y[si];
		Area._npc._y[si] = bp04;
	}
}

/*C_5036*/NpcMoveTo(si/*bp08*/, bp06, bp04)
register int si;
unsigned char bp06;
unsigned char bp04;
{
	Area._npc._old_x[si] = Area._npc._x[si];
	Area._npc._x[si] = bp06;
	Area._npc._old_y[si] = Area._npc._y[si];
	Area._npc._y[si] = bp04;
}

/*NPC attempts move[MOD_OUTDOORS]*/
/*C_5062*/NpcStepOutdoors(bp0a, bp08, bp06, bp04)
int bp0a;
int bp08;
int bp06;
int bp04;
{
	unsigned char loc_A, loc_B;
	unsigned char loc_C, loc_D;

	loc_B = Area._npc._x[bp0a]; loc_A = (bp08 + loc_B) - (WinBlock.x << 4);
	loc_D = Area._npc._y[bp0a]; loc_C = (bp06 + loc_D) - (WinBlock.y << 4);
	if(loc_A  > 31 || loc_C > 31) {
		if(bp04 >= 0)
			NpcStepOutdoors(bp0a, u4_sign((char)u_rand_a()), (char)u4_sign(u_rand_a()), bp04 - 1);
		return;
	}
	if(U4_RND1(1) && bp08 && NpcCanMoveTo(bp0a, loc_B + bp08, loc_D, Area._map.x32x32[loc_C-bp06][loc_A])) {
		NpcMoveTo(bp0a, loc_B + bp08, loc_D);
		return;
	}
	if(bp06 && NpcCanMoveTo(bp0a, loc_B, loc_D + bp06, Area._map.x32x32[loc_C][loc_A - bp08])) {
		NpcMoveTo(bp0a, loc_B, loc_D + bp06);
		return;
	}
	if(NpcCanMoveTo(bp0a, loc_B + bp08, loc_D, Area._map.x32x32[loc_C - bp06][loc_A])) {
		NpcMoveTo(bp0a, loc_B + bp08, loc_D);
		return;
	}
	if(bp04 >= 0)
		NpcStepOutdoors(bp0a, u4_sign((char)u_rand_a()), u4_sign((char)u_rand_a()), bp04 - 1);
}

/*NPC attempts move[MOD_BUILDING]*/
/*C_51A7*/NpcStepInTown(bp0a, bp08, bp06, bp04)
int bp0a;
int bp08;
int bp06;
int bp04;/*# of tries*/
{
	unsigned bp_02, bp_04;

	bp_02 = Area._npc._x[bp0a];
	bp_04 = Area._npc._y[bp0a];
	/*Area._map.x32x32 is accessed without any out-of-bounds control.
Fortunately, there is some control in NpcCanMoveTo so any faultly-accessed
value will be discarded anyway.
Though, there seem to be a problem with 64-bit builds.
An exception is raised when (bp08+bp_02) or (bp06+bp_04) are negative.
Masking the sums should fix the problem:
	Area._map.x32x32[bp_04][(bp08+bp_02)&0x1f]
	Area._map.x32x32[(bp06+bp_04)&0x1f][bp_02]
(issue raised by @plaidpants)*/
	if(U4_RND1(1) && bp08) {
		if(NpcCanMoveTo(bp0a, bp08+bp_02, bp_04, Area._map.x32x32[bp_04][bp08+bp_02]))
			NpcMoveInTown(bp0a, bp08+bp_02, bp_04);
		return;
	}
	if(bp06) {
		if(NpcCanMoveTo(bp0a, bp_02, bp06+bp_04, Area._map.x32x32[bp06+bp_04][bp_02]))
			NpcMoveInTown(bp0a, bp_02, bp06+bp_04);
		return;
	}
	if(NpcCanMoveTo(bp0a, bp08+bp_02, bp_04, Area._map.x32x32[bp_04][bp08+bp_02])) {
		NpcMoveInTown(bp0a, bp08+bp_02, bp_04);
		return;
	}
	if(Area._npc._var[bp0a] != 0x80 && bp04 >= 0)
		NpcStepInTown(bp0a, u4_sign((char)u_rand_a()), u4_sign((char)u_rand_a()), bp04 - 1);
}

/*move NPCs[MOD_BUILDING]*/
/*C_5293*/MoveTownNpcs()
{
	int bp_02, bp_04;

	{register int si; for(si = 31; si >= 0; si --) {
		if(Area._npc._tile[si] && Area._npc._var[si]) {
			if((char)Area._npc._var[si] >= 0) {
				/*random mode*/
				if(U4_RND1(1))
					NpcStepInTown(si, u4_sign((char)u_rand_a()), u4_sign((char)u_rand_a()), 2);
			} else {
				/*following mode*/
				bp_02 = u4_sign((char)(Party._x - Area._npc._x[si]));
				bp_04 = u4_sign((char)(Party._y - Area._npc._y[si]));
				if(u4_abs((char)(Party._x - Area._npc._x[si])) + u4_abs((char)(Party._y - Area._npc._y[si])) >= 2)
					NpcStepInTown(si, bp_02, bp_04, 2);
				else if(Area._npc._var[si] == 0xff) {
					/*agressive mode*/
					/*-- attacked by --*/
					RefreshView();
					u4_puts(MsgAttackedBy); u4_puts(CreatureName(Area._npc._tile[si]));
					Gra_CR();
					StartCombat(si);
				}
			}
		}
	}}
}

/*creature/party distance ?*/
/*C_535B*/NpcDistance(bp0c, bp0a, bp08, bp06, bp04)
int bp0c;/*index*/
int *bp0a;/*x sign*/
int *bp08;/*y sign*/
int *bp06;/*x distance*/
int *bp04;/*y distance*/
{
	*bp04 = (char)(Party._y - Area._npc._y[bp0c]);
	*bp08 = u4_sign(*bp04);
	*bp06 = (char)(Party._x - Area._npc._x[bp0c]);
	*bp0a = u4_sign(*bp06);
	return u4_abs(*bp06) + u4_abs(*bp04);
}

/*C_53AF*/ShipSail(si/*bp08*/, bp06, bp04)
register int si;
unsigned char bp06;
unsigned char bp04;
{
	if(NpcCanMoveTo(si, bp06, bp04, Area._map.x32x32[bp04-WinBlock.y*16][bp06-WinBlock.x*16])) {
		if(WindAllows(Area._npc._gtile[si] & 3)) {
			Area._npc._old_x[si] = Area._npc._x[si];
			Area._npc._x[si] = bp06;
			Area._npc._old_y[si] = Area._npc._y[si];
			Area._npc._y[si] = bp04;
		}
	} else {
		Area._npc._gtile[si] = ((u4_sign((char)u_rand_a()) + Area._npc._gtile[si]) & 3) | TIL_PIRATE_W;
	}
}

/*C_5443*/ShipSteer(si/*bp04*/)
register int si;
{
	int loc_A, loc_B, loc_C, loc_D;

	if(NpcDistance(si, &loc_A, &loc_B, &loc_C, &loc_D) > 5 || U4_RND1(3) == 0) {
		if(u4_abs(loc_C) < u4_abs(loc_D)) {
			if(loc_D >= 0)
				Area._npc._gtile[si] = TIL_PIRATE_S;
			else
				Area._npc._gtile[si] = TIL_PIRATE_N;
		} else {
			if(loc_C >= 0)
				Area._npc._gtile[si] = TIL_PIRATE_E;
			else
				Area._npc._gtile[si] = TIL_PIRATE_W;
		}
	} else if(Area._npc._gtile[si] == TIL_PIRATE_W) {
		ShipSail(si, Area._npc._x[si] - 1, Area._npc._y[si]);
	} else if(Area._npc._gtile[si] == TIL_PIRATE_N) {
		ShipSail(si, Area._npc._x[si], Area._npc._y[si] - 1);
	} else if(Area._npc._gtile[si] == TIL_PIRATE_E) {
		ShipSail(si, Area._npc._x[si]+1, Area._npc._y[si]);
	} else {
		ShipSail(si, Area._npc._x[si], Area._npc._y[si]+1);
	}
}

/*C_5500*/ShipMove(si/*bp08*/, bp06, bp04)
register int si;
int bp06;
int bp04;
{
	if(Area._npc._gtile[si] == TIL_PIRATE_W) {
		if(bp06 < 0) {
			ShipSail(si, Area._npc._x[si]+bp06, Area._npc._y[si]);
			return;
		}
	} else if(Area._npc._gtile[si] == TIL_PIRATE_N) {
		if(bp04 < 0) {
			ShipSail(si, Area._npc._x[si], Area._npc._y[si]+bp04);
			return;
		}
	} else if(Area._npc._gtile[si] == TIL_PIRATE_E) {
		if(bp06 > 0) {
			ShipSail(si, Area._npc._x[si]+bp06, Area._npc._y[si]);
			return;
		}
	} else {
		if(bp04 > 0) {
			ShipSail(si, Area._npc._x[si], Area._npc._y[si]+bp04);
			return;
		}
	}
	ShipSteer(si);
}

/*C_5569*/NpcShoot(bp0a, bp08, bp06, bp04)
int bp0a;
unsigned char bp08;
unsigned char bp06;
unsigned char bp04;
{
	unsigned char loc_E, loc_A;
	int loc_D, loc_C, loc_B;

	loc_E = Area._npc._x[bp0a] + bp08;
	loc_A = Area._npc._y[bp0a] + bp06;
	sound(3);
	for(loc_C = 1; loc_C <= 3; loc_E += bp08, loc_A += bp06, loc_C++) {
		if(Party._x == loc_E && Party._y == loc_A)
			return 1;
		if((loc_B = FindNpcAt(31, loc_E, loc_A)) != -1) {
			sound(6);
			if(loc_B >= 8 || U4_RND1(3) == 0) {
				Area._npc._tile[loc_B] = Area._npc._gtile[loc_B] = 0;
			}
			return 0;
		}
		{
			register unsigned char *si;

			si = &(Area._map.x32x32[loc_A - (WinBlock.y*16)][loc_E - (WinBlock.x*16)]);
			loc_D = *si;
			*si = bp04;
			RefreshView();
			*si = loc_D;
		}
	}
	return 0;
}

/*C_564B*/NpcFireAtParty(bp0a, bp08, bp06, bp04)
int bp0a;
int bp08;
int bp06;
int bp04;
{
	if(NpcShoot(bp0a, bp08, bp06, bp04) == 0) {
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

/*C_568F*/ShipAttack(bp08, bp06, bp04)
int bp08;
int bp06;
int bp04;
{
	if(Area._npc._gtile[bp08] == TIL_PIRATE_W || Area._npc._gtile[bp08] == TIL_PIRATE_E)
		if(bp06)
			ShipMove(bp08, bp06, bp04);
		else
			NpcFireAtParty(bp08, bp06, bp04, TIL_MISSILE);
	else
		if(bp04)
			ShipMove(bp08, bp06, bp04);
		else
			NpcFireAtParty(bp08, bp06, bp04, TIL_MISSILE);
}

/*C_56D3*/NpcOffWindow(bp04)
int bp04;
{
	return
		(Area._npc._x[bp04] - WinBlock.x*16) >= 32 ||
		(Area._npc._y[bp04] - WinBlock.y*16) >= 32
	;
}

/*move NPCs[MOD_OUTDOORS]*/
/*C_5712*/MoveWorldNpcs()
{
	int loc_A, loc_B, loc_C, loc_D;
	register int loc_F;

	loc_F = 7; do {
		int loc_G;
		register int loc_H;

		loc_H = Area._npc._tile[loc_F];
		if(loc_H == 0)
			continue;
		/*is out of range?*/
		if(NpcOffWindow(loc_F)) {
			Area._npc._tile[loc_F] = Area._npc._gtile[loc_F] = 0;
			continue;
		}
		/* */
		if(loc_H != TIL_WHIRLPOOL && loc_H != TIL_TWISTER) {
			loc_G = NpcDistance(loc_F, &loc_A, &loc_B, &loc_C, &loc_D);
			if(loc_G < 2) {
				/*-- attacked by --*/
				RefreshView();
				u4_puts(MsgAttackedBy); u4_puts(CreatureName(loc_H));
				Gra_CR();
				StartCombat(loc_F);
				continue;
			}
		}
		if(loc_H == TIL_PIRATE_W) {
			if(loc_G < 4)
				ShipAttack(loc_F, loc_A, loc_B);
			else
				ShipMove(loc_F, loc_A, loc_B);
			continue;
		}
		if(loc_H == TIL_SERPENT || loc_H == TIL_LAVA_LIZARD || loc_H >= TIL_HYDRA) {
			if(u4_abs(loc_C) < 5 && u4_abs(loc_D) < 5 && U4_RND1(1))
				NpcFireAtParty(loc_F, loc_A, loc_B, TIL_HIT_FLASH);
		}
		if(loc_H == TIL_WHIRLPOOL || loc_H == TIL_TWISTER || (IsNonEvil(loc_H) && U4_RND1(3)))
			NpcStepOutdoors(loc_F, u4_sign((char)u_rand_a()), u4_sign((char)u_rand_a()), 1);
		else
			NpcStepOutdoors(loc_F, loc_A, loc_B, 2);
	} while(--loc_F >= 0);
}

/*move npc?*/
/*C_5834*/MoveNpcs()
{
	if(CurMode == MOD_BUILDING)
		MoveTownNpcs();
	else if(CurMode == MOD_OUTDOORS)
		MoveWorldNpcs();
}

/*spawn creatures?*/
/*C_5851*/SpawnMonsters()
{
	register int si;
	register unsigned char *loc_B;
	unsigned char loc_C, loc_A;

	if(CurMode != MOD_OUTDOORS || U4_RND1(15))
		return;
	for(si = 3; si >= 0; si--) {
		if(Area._npc._tile[si] != 0)
			continue;
		loc_A = U4_RND1(31);
		loc_C = U4_RND1(31);
		loc_B = &(Area._map.x32x32[loc_C][loc_A]);
		if(u4_abs(loc_A - WinPos.x) >= 6 && u4_abs(loc_C - WinPos.y) >= 6) {
			if(*loc_B < TIL_SHALLOWS) {/*at sea*/
				if(U4_RND1(7) == 0) {
					Area._npc._x[si] = loc_A + WinBlock.x*16;
					Area._npc._y[si] = loc_C + WinBlock.y*16;
					Area._npc._tile[si] = Area._npc._gtile[si] = (unsigned char)TIL_PIRATE_W + (U4_RND1(7) * 2);
					if(Area._npc._gtile[si] == TIL_PIRATE_E)
						Area._npc._tile[si] = Area._npc._gtile[si] = TIL_PIRATE_W;
				}
			} else if(*loc_B >= TIL_GRASS && *loc_B < TIL_MOUNTAINS) {/*on earth*/
				unsigned bp_0a;

				Area._npc._x[si] = loc_A + WinBlock.x*16;
				Area._npc._y[si] = loc_C + WinBlock.y*16;
				if(Party._moves > 100000L)
					bp_0a = 15;
				else if(Party._moves < 10000L)
					bp_0a = 3;
				else if(Party._moves < 20000L)
					bp_0a = 7;
				else if (Party._moves >= 30000L)
					bp_0a = 15;
				else
					bp_0a = 7;
/*
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
				Area._npc._tile[si] =
				Area._npc._gtile[si] =
					U4_RND1(U4_RND1(bp_0a)) * 4 + TIL_ORC;
			}
		}
	}
}
