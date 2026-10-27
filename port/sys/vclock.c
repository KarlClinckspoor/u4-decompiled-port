/* replaces the PC's clock and keyboard as the game sees them: the virtual clock, DOS's
 * time and date, the BIOS keyboard buffer, and the pace of the game (u4rt.h). */
#include "u4rt.h"
#include "../gfx/ega.h"
#include "../platform/plat.h"
#include "../replay/replay.h"

#include <string.h>
#include <time.h>

uint16_t port_speed_info = 3;
uint32_t ega_cost;

static uint64_t vclock;          /* ns */
static uint32_t start_date;      /* year << 16 | month << 8 | day */
static uint32_t start_time;      /* hundredths of a second since midnight */
static int pace_speed = 1;
static int anchored;
static uint32_t anchor_ms;
static uint64_t anchor_vc;

static uint8_t frame[320 * 200];
static uint8_t palette[256 * 3];

void vc_set_speed(int multiple)
{
	pace_speed = multiple;
	anchored = 0;
}

static time_t fixed_clock;

void vc_fix_clock(long long seconds_since_epoch_local)
{
	fixed_clock = (time_t)seconds_since_epoch_local;
}

void vc_start(void)
{
	time_t t = fixed_clock ? fixed_clock : time(NULL);
	struct tm *tm = fixed_clock ? gmtime(&t) : localtime(&t);
	uint32_t live_date = (uint32_t)(tm->tm_year + 1900) << 16 | (uint32_t)(tm->tm_mon + 1) << 8 | (uint32_t)tm->tm_mday;
	uint32_t live_time = (uint32_t)((tm->tm_hour * 60 + tm->tm_min) * 60 + tm->tm_sec) * 100u;
	start_date = rp_value(RP_DATE, live_date);
	start_time = rp_value(RP_DATE, live_time);
	vclock = 0;
	anchored = 0;
	ega_cost = 0;
	memset(palette, 0, sizeof palette);
	plat_video_source(frame, palette);
}

uint64_t vc_ns(void)
{
	return vclock;
}

void vc_cycles(uint32_t cycles)
{
	uint16_t s = port_speed_info ? port_speed_info : 1;
	vclock += (uint64_t)cycles * XT_CYCLE_NS_X1000 / 1000u / s;
}

void vc_add_ns(uint64_t ns)
{
	vclock += ns;
}

/* the game's busy-wait loops (BUSY_PASS, portable.h): one pass of MSC's "while(--count);"
 * on a 32-bit count; the platform is pumped now and then (a long wait, or the endless
 * loop after the victory) */
void port_busy_pass(void)
{
	static uint32_t passes;
	vc_cycles(120);
	if (++passes % 256 == 0)
		vc_pace();
}

/* ---- the screen ---- */
static void present(void)
{
	int i;
	if (!ega_take_dirty())
		return;
	ega_to_chunky(frame);
	for (i = 0; i < 16; i++)
		ega_rgb6(ega_palette_regs[i], palette + i * 3);
	plat_video_dirty();
}

static uint32_t last_pump_ms = 0xFFFFFFFFu;

/* At most once a millisecond: of real time, or of the PC's time when nothing waits for
 * real time (a replay at full speed, --fast), so that what the pump does (the test options'
 * keys) happens at the same point of the game whatever the host's speed. */
void port_pump(void)
{
	uint32_t now = pace_speed == 0 ? (uint32_t)(vclock / 1000000u) : plat_ms();
	present();
	if (now != last_pump_ms) {
		last_pump_ms = now;
		plat_pump();
	}
}

void vc_pace(void)
{
	uint32_t now, due;
	uint64_t ahead;

	if (ega_cost) {
		vc_cycles(ega_cost);
		ega_cost = 0;
	}
	port_pump();
	if (pace_speed == 0 || (rp_mode == RP_REPLAY && !rp_finished() && pace_speed == 0))
		return;
	now = plat_ms();
	if (!anchored) {
		anchored = 1;
		anchor_ms = now;
		anchor_vc = vclock;
		return;
	}
	ahead = (vclock - anchor_vc) / 1000000u / (uint64_t)pace_speed;
	due = anchor_ms + (uint32_t)ahead;
	if ((int32_t)(now - due) > 250) {
		/* the host fell behind (a busy machine, a window being dragged): go on from now
		 * instead of hurrying to catch up */
		anchor_ms = now;
		anchor_vc = vclock;
		return;
	}
	while ((int32_t)(due - now) > 0 && !plat_quit_requested()) {
		uint32_t wait = due - now;
		plat_sleep_ms(wait > 10 ? 10 : wait);
		port_pump();
		now = plat_ms();
		if (!anchored)	/* the speed changed meanwhile */
			return;
	}
}

/* ---- DOS's clock ---- */
#define TICK_NS 54925416u          /* 65536 / 1193182 s */

void dos_gettime(uint8_t *hour, uint8_t *minute, uint8_t *second, uint8_t *hundredths)
{
	/* DOS counts the time in timer ticks: it moves 18.2 times a second */
	uint64_t ticks = vclock / TICK_NS;
	uint64_t hs = start_time + ticks * 549254u / 100000u;	/* a tick is 5.4925 hundredths */
	hs %= 8640000u;
	*hour = (uint8_t)(hs / 360000u);
	*minute = (uint8_t)(hs / 6000u % 60u);
	*second = (uint8_t)(hs / 100u % 60u);
	*hundredths = (uint8_t)(hs % 100u);
}

void dos_getdate(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *weekday)
{
	struct tm tm;
	memset(&tm, 0, sizeof tm);
	tm.tm_year = (int)(start_date >> 16) - 1900;
	tm.tm_mon = (int)(start_date >> 8 & 0xFF) - 1;
	tm.tm_mday = (int)(start_date & 0xFF) + (int)((start_time / 100u + vclock / 1000000000u) / 86400u);
	tm.tm_hour = 12;
	mktime(&tm);
	*year = (uint16_t)(tm.tm_year + 1900);
	*month = (uint8_t)(tm.tm_mon + 1);
	*day = (uint8_t)tm.tm_mday;
	*weekday = (uint8_t)tm.tm_wday;
}

/* ---- the keyboard ---- */
static uint16_t held;       /* the live key taken from the host, not yet read by the game */
static uint16_t head;       /* the key at the head of the buffer (the replayed one under replay) */

/* A poll that finds no key is recorded; a key stays at the head of the buffer until the
 * game reads it, so the polls in between need no record. */
uint16_t kb_peek(void)
{
	if (!head) {
		if (!held && plat_key_available())
			held = plat_key_get();
		head = (uint16_t)rp_value(RP_KEY, held);
	}
	return head;
}

uint16_t kb_read(void)
{
	uint16_t k;
	for (;;) {
		k = kb_peek();
		if (k)
			break;
		vc_cycles(300);
		vc_pace();
		if (plat_quit_requested())
			break;
	}
	head = 0;
	held = 0;
	return k;
}
