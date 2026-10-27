/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

/*how each tile (0x00-0x7F) is drawn on the gem map: one of the 4x4 dot patterns
  of ShowAreaMap, 0 = left black*/
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

/*character drawn on the dungeon gem map for each kind of dungeon tile (tile >> 4)*/
int16 /*D_2FF2*/DngMapChars[] = {0x20, 0x06, 0x05, 0x04, 0x24, 0x20, 0x20, 0x0F, 0x54, 0x46, 0x5E, 0x00, 0x0E, 0x0E, 0x02, 0x03};

/*screen coordinate of dot d (0-3) of map cell n: each cell is 4x4 dots, the map
  starts 8 cells from the corner of the view. Gra_Dot takes (y, x, colour).*/
#define MK_COORD(n, d) (((n)+8)*4 + (d))
/*draw dot (dy, dx) of the current cell (row, col)*/
#define PT(dy,dx,c) Gra_Dot(MK_COORD(row,(dy)),MK_COORD(col,(dx)),(c));
/*colour 1 (green?)*/
#define PT_1(dy,dx) PT((dy),(dx),1)
/*colour 2 (blue?)*/
#define PT_2(dy,dx) PT((dy),(dx),2)
/*colour 3 (white)*/
#define PT_3(dy,dx) PT((dy),(dx),3)

static unsigned char D_8CFC[600];
static unsigned char D_8F54[22][22];
/*The statics keep their address-like names: Microsoft C 4.00 lays out a module's
 uninitialised statics in the order of its symbol table, which depends on the names.
 Renaming them would move them; the readable names are macros.*/
/*ShowDungeonMap's flood fill: stack of (screen y, screen x, map y, map x) entries*/
#define FloodStack D_8CFC
/*ShowDungeonMap's flood fill: screen positions already drawn*/
#define FloodSeen D_8F54

/*
 * Gem view of a 32x32 map (town, castle, or the part of the world around the
 * party): each tile is drawn as a 4x4 dot pattern. If showParty, the party's
 * position blinks (a moving bar) until a key is pressed.
 */
int16 /*C_B9EF*/ShowAreaMap(showParty)
uint16 showParty;
{
	int16 row, savedMode, col;

	savedMode = CurMode; CurMode = MOD_VISION;
	Gra_ClearView();
	for(row = 0; row < 32; row++) {
	for(col = 0; col < 32; col++) {
		if(Area._map.x32x32[row][col] < 0x80) {
			switch(GemMapStyle[Area._map.x32x32[row][col]]) {
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
				case 5:/*dungeon/town*/
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
					PT_3(2*(col&1),2);
				break;
			}
		} else {/*creatures and people: drawn like case 5*/
			PT_3(1,1);
			PT_3(2,1);
			PT_3(1,2);
			PT_3(2,2);
		}
	}
	}
	/*col is now the blinking counter*/
	col = 0;
	u_kbflush();
	for(; !u_kbhit(); col++) {
		if(showParty) {
			Gra_DotXor(MK_COORD(WinPos.y,0), MK_COORD(WinPos.x,col&3), 3);
			Gra_DotXor(MK_COORD(WinPos.y,1), MK_COORD(WinPos.x,col&3), 3);
			Gra_DotXor(MK_COORD(WinPos.y,2), MK_COORD(WinPos.x,col&3), 3);
			Gra_DotXor(MK_COORD(WinPos.y,3), MK_COORD(WinPos.x,col&3), 3);
		}
	}
	u_kbread();
	CurMode = savedMode;
}
#undef PT
#undef MK_COORD

/*
 * Gem view of the current dungeon level: starting from the party (in the middle of
 * a 22x22 character area), flood-fill outwards through everything that is not a wall,
 * printing a character for each tile reached. The 8x8 level wraps around, so it is
 * repeated until the edges of the area.
 */
int16 /*C_C23B*/ShowDungeonMap()
{
	int16 y, h;
	register uint16 sp;
	int16 v, prevMode, x;

	prevMode = CurMode; CurMode = MOD_VISION;
	Gra_ClearView();
	txt_Y = 21; do { txt_X = 21; do {
		FloodSeen[txt_Y][txt_X] = 0;
	} while(--txt_X >= 0); } while(--txt_Y >= 0);

	sp = 0;
	FloodStack[sp++] = txt_Y = 11;
	FloodStack[sp++] = txt_X = 11;
	FloodStack[sp++] = Party._y;
	FloodStack[sp++] = Party._x;
	FloodSeen[10][10] = 1;
	Gra_PutChar(0x01);/*the party*/
	do {
		x = FloodStack[--sp];
		y = FloodStack[--sp];
		txt_X = FloodStack[--sp] - 1;
		txt_Y = FloodStack[--sp] - 1;
		/*the 8 neighbours: h and v are the horizontal and vertical offsets*/
		for(h = -1; h < 2; txt_X ++, txt_Y -= 3, h++) {
			for(v = -1; v < 2; txt_Y ++, v++) {
				if((h | v)) {
					int16 nextX, nextY;
					nextX = (h + x) & 7;
					nextY = (v + y) & 7;
					if(
						txt_Y < 1 || txt_Y > 22 ||
						txt_X < 1 || txt_X > 22 ||
						FloodSeen[txt_Y-1][txt_X-1]
					) continue;
					Gra_PutChar(DngMapChars[Area._map.x8x8x8[Party._z][nextY][nextX] >> 4]);
					if((Area._map.x8x8x8[Party._z][nextY][nextX] & 0xf0) != 0xf0) {
						FloodStack[sp++] = txt_Y;
						FloodStack[sp++] = txt_X;
						FloodStack[sp++] = nextY;
						FloodStack[sp++] = nextX;
						/*overflow: wraps to the bottom (and if it ends at 0
						  the loop stops)*/
						if(sp >= 600)
							sp = 0;
						FloodSeen[txt_Y-1][txt_X-1] = 1;
					}
				}
			}
		}
	} while(sp);
	txt_Y = 23;
	txt_X = 24;
	u_kbflush();
	u_kbread();
	CurMode = prevMode;
	Redraw3D = 1;
}

/*
 * Gem view (also the View spell): the dungeon map in dungeons, else the 32x32 map with
 * the party's position.
 */
int16 /*C_C403*/ShowMap()
{
	if(Party._loc >= LOC_DUNGEONS) {
		ShowDungeonMap();
	} else {
		ShowAreaMap(1);
	}
}

/*
 * (P)eer at a gem: uses one gem to show the map.
 */
int16 /*C_C41D*/CMD_Peer()
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
