/* replaces LOW.ASM (AVATAR.EXE) and its copy in TITLE.EXE (compiled again with
 * PORT_TITLE and the title's names): the program's start-up and end on the PC.
 *
 *   low_init   the BIOS equipment word (int 11h), the video mode to restore, the far PSP pointer,
 *              a 16 KB block for Tandy's patch, the critical error vector, the random seed
 *              from DOS's clock and date, and speed_info: the iterations of a 240-cycle
 *              loop during one timer tick, / 1000 (at least 1).
 *   low_clean  restores the video mode and the vectors.
 *   low_gra    the graphics adapter: 1 CGA, 2 EGA, 3 Tandy, -1 none.
 *
 * Port: the PC is an EGA PC/AT-class machine with one hard disk; speed_info is the port's
 * (settings, recorded: u4rt.h).
 */
#include "u4rt.h"
#include "../replay/replay.h"

#ifdef PORT_TITLE
#define low_set_command_tail T_low_set_command_tail
#endif

#include <stdlib.h>
#include <string.h>

extern uint16_t speed_info;         /* the BSS block (port/sys/bss_*.c) */
extern uint16_t equip_flags;
extern void *patch_tandy;
extern uint8_t seed_128[16];

/* the PSP: 80h holds the command line's length, 81h.. its text (struct tPSP) */
static uint8_t psp[256];
void *far_psp = psp;                /* D_0078 */

/* the command line the program was started with (ULTIMA.COM passes TITLE.EXE's exit code
 * to AVATAR.EXE this way) */
void low_set_command_tail(const uint8_t *tail, int len)
{
	memset(psp + 0x80, 0, 0x80);
	if (len > 0x7E)
		len = 0x7E;
	psp[0x80] = (uint8_t)len;
	memcpy(psp + 0x81, tail, (size_t)len);
	psp[0x81 + len] = 0x0D;
}

int low_init(void)
{
	uint8_t h, m, s, hs, mo, d, wd;
	uint16_t y;

	/* int 11h: one floppy drive (bits 6-7: drives - 1), a math chip no, 80x25 colour */
	equip_flags = 0x0021;
	vc_cycles(2000);
	patch_tandy = malloc(0x4000);	/* 400h paragraphs (dfree'd on an EGA) */
	/* the seed: DX = seconds:hundredths, CX = hour:minute (int 21h 2Ch), then DX =
	 * month:day, CX = year (2Ah) */
	dos_gettime(&h, &m, &s, &hs);
	seed_128[0] = hs;
	seed_128[1] = s;
	seed_128[2] = m;
	seed_128[3] = h;
	memcpy(seed_128 + 8, seed_128, 4);
	dos_getdate(&y, &mo, &d, &wd);
	seed_128[4] = d;
	seed_128[5] = mo;
	seed_128[6] = (uint8_t)y;
	seed_128[7] = (uint8_t)(y >> 8);
	memcpy(seed_128 + 12, seed_128 + 4, 4);
	/* the calibration lasts one to two ticks */
	vc_add_ns(54925416u + 54925416u / 2);
	speed_info = (uint16_t)rp_value(RP_MISC, port_speed_info);
	if (speed_info == 0)
		speed_info = 1;
	port_speed_info = speed_info;	/* a replay's */
	return speed_info;
}

int low_clean(void)
{
	return 0;
}

int low_gra(void)
{
	return 2;
}
