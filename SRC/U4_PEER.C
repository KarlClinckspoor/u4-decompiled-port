/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

unsigned char /*D_2F72*/GemMapStyle[] = {
	0x0C,0x0B,0x0A,0x01,0x01,0x09,0x02,0x08,0x07,0x05,0x05,0x05,0x05,0x04,0x04,0x04,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x04,0x05,0x04,0x04,0x05,0x05,0x06,0x06,0x05,
	0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,
	0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x05,0x05,0x07,0x06,0x06,0x05,0x00,0x03,0x04,
	0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x07,0x03,0x03,0x03,0x03,0x03,0x03,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
	0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x06,0x00,0x07
};

int /*D_2FF2*/DngMapChars[] = {0x20, 0x06, 0x05, 0x04, 0x24, 0x20, 0x20, 0x0F, 0x54, 0x46, 0x5E, 0x00, 0x0E, 0x0E, 0x02, 0x03};

#define MK_COORD(a, b) (((a)+8)*4 + (b))
#define PT(x,y,c) Gra_Dot(MK_COORD(loc_A,(x)),MK_COORD(loc_C,(y)),(c));
/*green?*/
#define PT_1(x,y) PT((x),(y),1)
/*blue?*/
#define PT_2(x,y) PT((x),(y),2)
/*white?*/
#define PT_3(x,y) PT((x),(y),3)

static unsigned char D_8CFC[600];
static unsigned char D_8F54[22][22];
/*The statics keep their address-like names: Microsoft C 4.00 lays out a module's
 uninitialised statics in the order of its symbol table, which depends on the names.
 Renaming them would move them; the readable names are macros.*/
#define FloodStack D_8CFC
#define FloodSeen D_8F54

/*C_B9EF*/ShowTownMap(bp04/*display player pos*/)
unsigned bp04;
{
	int loc_A, loc_B, loc_C;

	loc_B = CurMode; CurMode = MOD_VISION;
	Gra_ClearView();
	for(loc_A = 0; loc_A < 32; loc_A++) {
	for(loc_C = 0; loc_C < 32; loc_C++) {
		if(Area._map.x32x32[loc_A][loc_C] < 0x80) {
			switch(GemMapStyle[Area._map.x32x32[loc_A][loc_C]]) {
				case 1:/*grass*/
					PT_1(0,1);
					PT_1(1,3);
					PT_1(2,1);
					PT_1(3,3);
				break;
				case 2:/*forest*/
					PT_1(0,3);
					PT_1(1,1);
					PT_1(2,3);
					PT_1(3,1);
					PT_1(0,1);
					PT_1(1,3);
					PT_1(2,1);
					PT_1(3,3);
				break;
				case 3:/*lava*/
					PT_2(0,0);
					PT_2(1,0);
					PT_2(2,0);
					PT_2(3,0);
					PT_2(0,2);
					PT_2(1,2);
					PT_2(2,2);
					PT_2(3,2);
				break;
				case 4:/*castle/bridge*/
					PT_3(0,0);
					PT_3(0,1);
					PT_3(0,2);
					PT_3(0,3);
					PT_3(3,0);
					PT_3(3,1);
					PT_3(3,2);
					PT_3(3,3);
				break;
				case 5:/*dungeon/town*//*C_BCA0:*/
					PT_3(1,1);
					PT_3(2,1);
					PT_3(1,2);
					PT_3(2,2);
				break;
				case 6:/*shrine*/
					PT_3(1,0);
					PT_3(2,0);
					PT_3(1,3);
					PT_3(2,3);
					PT_3(0,0);
					PT_3(0,1);
					PT_3(0,2);
					PT_3(0,3);
					PT_3(3,0);
					PT_3(3,1);
					PT_3(3,2);
					PT_3(3,3);
				break;
				case 7:/*mountain*/
					PT_3(1,1);
					PT_3(2,1);
					PT_3(1,2);
					PT_3(2,2);
					PT_3(1,0);
					PT_3(2,0);
					PT_3(1,3);
					PT_3(2,3);
					PT_3(0,0);
					PT_3(0,1);
					PT_3(0,2);
					PT_3(0,3);
					PT_3(3,0);
					PT_3(3,1);
					PT_3(3,2);
					PT_3(3,3);
				break;
				case 8:/*hill*/
					PT_3(0,0);
					PT_3(1,0);
					PT_3(0,1);
					PT_3(1,1);
					PT_3(2,2);
					PT_3(3,2);
					PT_3(2,3);
					PT_3(3,3);
				break;
				case 9:/*bush*/
					PT_1(0,1);
					PT_1(1,1);
					PT_1(2,1);
					PT_1(0,3);
					PT_1(2,3);
					PT_1(3,3);
				break;
				case 10:/*shallow water*/
					PT_2(0,0);
					PT_2(2,0);
					PT_2(1,2);
					PT_2(3,2);
				break;
				case 11:/*water*/
					PT_2(0,0);
					PT_2(2,2);
				break;
				case 12:/*deep water*/
					PT_3(2*(loc_C&1),2);
				break;
			}
		} else {/*goto C_BCA0;*/
			PT_3(1,1);
			PT_3(2,1);
			PT_3(1,2);
			PT_3(2,2);
		}
/*C_BD04*/
	}
	}
/*C_BD1C*/
	loc_C = 0;
	u_kbflush();
/*C_C174*/
	for(; !u_kbhit(); loc_C++) {
		if(bp04) {
			Gra_DotXor(MK_COORD(WinPos.y,0), MK_COORD(WinPos.x,loc_C&3), 3);
			Gra_DotXor(MK_COORD(WinPos.y,1), MK_COORD(WinPos.x,loc_C&3), 3);
			Gra_DotXor(MK_COORD(WinPos.y,2), MK_COORD(WinPos.x,loc_C&3), 3);
			Gra_DotXor(MK_COORD(WinPos.y,3), MK_COORD(WinPos.x,loc_C&3), 3);
		}
	}
	u_kbread();
	CurMode = loc_B;
}
#undef PT
#undef MK_COORD

/*C_C23B*/ShowDungeonMap()
{
	int loc_A, loc_B;
	register unsigned loc_C;
	int loc_D, loc_E, loc_F;

	loc_E = CurMode; CurMode = MOD_VISION;
	Gra_ClearView();
	txt_Y = 21; do { txt_X = 21; do {
		FloodSeen[txt_Y][txt_X] = 0;
	} while(--txt_X >= 0); } while(--txt_Y >= 0);

	loc_C = 0;
	FloodStack[loc_C++] = txt_Y = 11;
	FloodStack[loc_C++] = txt_X = 11;
	FloodStack[loc_C++] = Party._y;
	FloodStack[loc_C++] = Party._x;
	FloodSeen[10][10] = 1;
	Gra_PutChar(0x01);
/*C_C2B5*/
	do {
		loc_F = FloodStack[--loc_C];
		loc_A = FloodStack[--loc_C];
		txt_X = FloodStack[--loc_C] - 1;
		txt_Y = FloodStack[--loc_C] - 1;
		for(loc_B = -1; loc_B < 2; txt_X ++, txt_Y -= 3, loc_B++) {
/*C_C2E0*/
			for(loc_D = -1; loc_D < 2; txt_Y ++, loc_D++) {
				if((loc_B | loc_D)) {
					int bp_0e, bp_10;
					bp_0e = (loc_B + loc_F) & 7;
					bp_10 = (loc_D + loc_A) & 7;
					if(
						txt_Y < 1 || txt_Y > 22 ||
						txt_X < 1 || txt_X > 22 ||
						FloodSeen[txt_Y-1][txt_X-1]
					) continue;
/*C_C347*/
					Gra_PutChar(DngMapChars[Area._map.x8x8x8[Party._z][bp_10][bp_0e] >> 4]);
					if((Area._map.x8x8x8[Party._z][bp_10][bp_0e] & 0xf0) != 0xf0) {
						FloodStack[loc_C++] = txt_Y;
						FloodStack[loc_C++] = txt_X;
						FloodStack[loc_C++] = bp_10;
						FloodStack[loc_C++] = bp_0e;
						if(loc_C >= 600)
							loc_C = 0;
						FloodSeen[txt_Y-1][txt_X-1] = 1;
					}
				}
/*C_C33E*/
			}
		}
	} while(loc_C);
	txt_Y = 23;
	txt_X = 24;
	u_kbflush();
	u_kbread();
	CurMode = loc_E;
	Redraw3D = 1;
}

/*C_C403*/ShowMap()
{
	if(Party._loc >= 0x11) {
		ShowDungeonMap();
	} else {
		ShowTownMap(1);
	}
}

/*C_C41D*/CMD_Peer()
{
	u4_puts(/*D_3012*/"Peer at ");
	if(Party._gems == 0) {
		w_What();
		return;
	}
	u4_puts(/*D_301B*/"a Gem!\n");
	Party._gems --;
	ShowMap();
}
