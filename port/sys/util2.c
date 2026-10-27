/* replaces UTIL2.ASM (AVATAR.EXE, SRC/) and its copy in TITLE.EXE (SRC-TITLE/; this file
 * is compiled a second time with PORT_TITLE and the title's names, port/title/trename.h):
 * the delay with the spinning cursor, the two random generators, the keyboard, the drives.
 *
 * DOS's clock and keyboard are the port's (u4rt.h). Each routine adds to the virtual clock
 * the time its DOS code took on the PC (8088 cycles: the BIOS and DOS calls included), so
 * that the game keeps the pace it had there.
 */
#include "u4rt.h"
#include "dosfs.h"

#include <string.h>

/* the game's (C) */
#ifdef PORT_TITLE
#define u_rand_b_carry T_u_rand_b_carry
#define u4_rnd1 T_u4_rnd1
void AnimateMonsters(void);
#define REFRESH() AnimateMonsters()
#else
void RefreshView(void);
#define REFRESH() RefreshView()
extern uint16_t CurMode;
#endif
void Gra_PutChar(int c);
extern uint8_t Party[];
extern uint16_t speed_info;

/* UTIL2.ASM's data: public ones (the BSS block, port/sys/bss_*.c, holds seed_128 and Area) */
uint16_t cursor_delay;              /* D_0672: extra cursor countdown per refresh */
static uint16_t cursor_delay_old;   /* D_0674 */
uint16_t cursor_enabled = 0xFFFF;   /* D_0676: the spinning cursor is shown */
extern uint8_t seed_128[16];        /* D_8732 */
static uint8_t cursor_state;        /* D_8C42: 0..3, the cursor's frame */
static uint16_t cursor_count;       /* D_8C43 */
int u_rand_b_carry;

/* ---- the 8088 cycles the DOS code takes (the BIOS and DOS calls with it) ---- */
#define COST_RAND_A 600     /* u_rand_a: 14 "adc/stosb/loop" and the counter */
#define COST_KBHIT 250      /* int 16h AH=01h */
#define COST_GETTIME 450    /* int 21h AH=2Ch */
#define COST_LOOP 40

/* what the game's refresh costs besides its EGA writes (its C: the line of sight, the
 * dungeon's walls, ...) */
static uint32_t refresh_cost(void)
{
#ifdef PORT_TITLE
	return 150;	/* AnimateMonsters: a countdown (its drawing counts itself) */
#else
	switch (CurMode) {
	case 0: return 30;          /* MOD_VISION: returns at once */
	case 1: return 60000;       /* outdoors: wind, balloon, the line of sight */
	case 2: return 50000;       /* a town */
	case 3: return 40000;       /* a dungeon: the 3D view */
	default: return 25000;      /* combat */
	}
#endif
}

static void refresh(void)
{
	REFRESH();
	vc_cycles(refresh_cost());
}

static uint8_t dos_seconds(void)
{
	uint8_t h, m, s, hs;
	vc_cycles(COST_GETTIME);
	dos_gettime(&h, &m, &s, &hs);
	return s;
}

/* C_1771: the 128-bit generator: seed_128[1..14] each add their right neighbour with carry,
 * from the end, then seed_128[0]; then the whole is incremented as a 128-bit number.
 * Returns seed_128[0]. */
int u_rand_a(void)
{
	unsigned al, cf = 0;
	int di;
	vc_cycles(COST_RAND_A);
	al = seed_128[15];
	for (di = 14; di >= 1; di--) {
		al = al + seed_128[di] + cf;
		cf = al >> 8;
		al &= 0xFF;
		seed_128[di] = (uint8_t)al;
	}
	seed_128[0] = (uint8_t)(seed_128[0] + al + cf);
	for (di = 15; di >= 0; di--)
		if (++seed_128[di] != 0)
			break;
	return seed_128[0];
}

/* the port's U4_RND1 (U4.H): the operand is computed before the call */
int u4_rnd1(int a)
{
	return u_rand_a() & a;
}

/* C_17A2: the small generator on seed_128[0..3]; its first "adc" adds the caller's carry
 * (u_rand_b_carry, set by RND2_CARRY in the game's C; 0 otherwise) */
uint8_t u_rand_b(void)
{
	unsigned al = seed_128[3] + seed_128[2] + (u_rand_b_carry ? 1u : 0u);
	u_rand_b_carry = 0;
	vc_cycles(60);
	seed_128[2] = (uint8_t)al;
	seed_128[1] ^= (uint8_t)al;	/* xor clears the carry */
	al = (uint8_t)(seed_128[1] + seed_128[0]);
	al = (al >> 1 | al << 7) & 0xFF;
	seed_128[0] = (uint8_t)al;
	seed_128[3] = (uint8_t)al;
	return (uint8_t)al;
}

/* C_17C5: one poll. Calls u_rand_a, counts the turn (Party._tick += 1), then: a key
 * waiting returns 1 (Ctrl+C and the zero key are taken and dropped); else the view is
 * refreshed and it returns 0. */
int16_t u_kbhit(void)
{
	uint32_t tick;
	uint16_t key;
	u_rand_a();
	memcpy(&tick, Party, 4);
	tick++;
	memcpy(Party, &tick, 4);
	vc_cycles(COST_KBHIT + COST_LOOP);
	vc_pace();
	key = kb_peek();
	if (key) {
		if (key != 0x2E03)
			return 1;
		kb_read();
	}
	refresh();
	return 0;
}

/* C_16CD: waits until the clock's second is `seconds` later (DOS's clock: up to a second
 * less). With `interrupt`, a key ends the wait and the cursor spins at (txt_X, txt_Y)
 * meanwhile, one frame every (cursor_delay + 1) * speed_info refreshes. The view is
 * refreshed all the time. */
void u_delay(int seconds, int interrupt)
{
	uint16_t target;
	int enabled = (uint16_t)interrupt & cursor_enabled;

	cursor_count = 1;
	cursor_delay_old = cursor_delay;
	target = (uint16_t)(dos_seconds() + (uint16_t)seconds);
	if (target >= 60) {
		target -= 60;
		if (!interrupt)
			goto refresh_;
		if (u_kbhit())
			return;	/* C_176C: the cursor is not erased */
	}
	for (;;) {
		/* C_1701 */
		if (interrupt && u_kbhit())
			break;
refresh_:
		/* C_1710 */
		refresh();
		if (enabled) {
			int reload = 1;
			if (cursor_delay_old == cursor_delay) {
				reload = 0;
				if (--cursor_count == 0) {
					cursor_state = (uint8_t)((cursor_state - 1) & 3);
					Gra_PutChar(cursor_state + 0x1C);
					reload = 1;
				}
			}
			if (reload) {
				/* C_173D */
				cursor_count = (uint16_t)((1u + cursor_delay) * speed_info);
				cursor_delay_old = cursor_delay;
			}
		}
		vc_cycles(COST_LOOP);
		/* C_1751 */
		if (target == dos_seconds())
			break;
	}
	/* C_175D */
	if (enabled)
		Gra_PutChar(' ');
}

/* C_17F4: waits for a key (a delay of 0x8081 seconds, ended by the key) and reads it */
int16_t u_kbread(void)
{
	u_delay(0x8081, 0x8081);
	vc_cycles(COST_KBHIT);
	return (int16_t)kb_read();
}

/* C_1804: drops the keys waiting */
void u_kbflush(void)
{
	while (u_kbhit())
		u_kbread();
}

/* C_1814: the current drive (1 = A:): the game folder is drive C: */
uint8_t u_getdrive(void)
{
	return 3;
}

/* C_181D: selects a drive (DOS 0Eh): the port has only C: */
int u_setdrive(uint16_t drive)
{
	(void)drive;
	return 0;
}

/* C_182F: 1 if the file can be opened */
int u_fexist(char *name)
{
	return dosfs_exists(name);
}

/* C_184F: reads 16 sectors of track 9 of the floppy (the original disk's mark): 0 if they
 * read, -1 if not. The result is not used; there is no floppy. */
int u_diskcheck(void)
{
	return (uint8_t)0xFF;
}

/* C_18A2: installs the critical error handler (int 24h): the port's files report their
 * errors to the game as DOS's do, with no prompt */
int u_setint24(void)
{
	return 0;
}
