/* replaces U4_SOUND.ASM (AVATAR.EXE's sound effects), TITLE.EXE's BEEP.ASM (sound_1) and
 * the speaker crackle of EGA.DRV's screen shake: the PC speaker, driven by hand.
 *
 * DOS: each effect toggles bit 1 of port 61h (the speaker's cone) in busy loops, so its
 * pitch and length are those of the loops on the CPU it runs on. Most loops are scaled by
 * speed_info to sound alike on every PC; a few are not (the step's click), and some call
 * u_rand_a, which moves the game's random generator.
 *
 * Port: each routine is played out in 8088 cycles, exactly as its loops run (the same loop
 * counts, 8088 timings per instruction): a list of the instants the cone moves. The time
 * the effect takes goes on the virtual clock (the game waits through it, as in DOS) and the
 * list goes to the audio, which renders the cone's movements as a 1-bit waveform (box-
 * filtered to the sample rate, a DC blocker after). The random generator is called as in
 * DOS whether or not the audio plays. With the game's own sound flag off (the V command)
 * sound() does nothing, as in DOS.
 */
#include "u4rt.h"
#include "../platform/plat.h"
#include "../replay/replay.h"

#include <stdlib.h>
#include <string.h>

extern uint16_t speed_info;         /* AVATAR's */
extern uint16_t T_speed_info;       /* TITLE's */
int u_rand_a(void);

char SoundFlag = 1;        /* D_06A6 (AVATAR's _SoundFlag): the V(olume) command toggles it */

/* ---- the audio side: a queue of half-periods ---- */
#define QMAX 65536
static uint32_t q_ns[QMAX];         /* how long the cone stays */
static uint8_t q_level[QMAX];       /* at this level (0, 1) */
static volatile int q_head, q_tail;
static uint64_t seg_left_ns;        /* of the segment at q_head */
static double dc_x, dc_y;
static int volume = 50;             /* 0..100 */
static int audio_on = 1;

void sound_set_volume(int v, int on)
{
	volume = v;
	audio_on = on;
}

static void mix(int16_t *out, int frames, int rate)
{
	double sample_ns = 1e9 / rate;
	int i;
	for (i = 0; i < frames; i++) {
		double need = sample_ns, high = 0, x;
		int active = q_head != q_tail;
		while (need > 0 && q_head != q_tail) {
			double take;
			if (seg_left_ns == 0)
				seg_left_ns = q_ns[q_head];
			take = (double)seg_left_ns < need ? (double)seg_left_ns : need;
			if (q_level[q_head])
				high += take;
			need -= take;
			seg_left_ns -= (uint64_t)take;
			if (seg_left_ns == 0 || (double)seg_left_ns < 1)
				q_head = (q_head + 1) % QMAX, seg_left_ns = 0;
		}
		x = active ? (high / sample_ns) * 2.0 - 1.0 : dc_x;
		/* DC blocker: the cone resting on one side is silence */
		dc_y = x - dc_x + 0.995 * dc_y;
		dc_x = x;
		{
			int v = out[i] + (int)(dc_y * volume * 60);
			out[i] = (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
		}
	}
}

/* ---- the routines, in 8088 cycles ---- */
static uint64_t t;                  /* cycles since the effect started */
static uint64_t t_last;             /* the last movement */
static int level;                   /* the cone (port 61h bit 1) */
static int al;                      /* bit 1 of AL, the next value OUT writes */
static uint16_t speed;              /* the speed_info of the program playing */
static uint32_t *seg_ns;
static uint8_t *seg_level;
static int nseg, segcap;

static void begin(void)
{
	t = t_last = 0;
	level = 0;
	al = 0;		/* "and al,not 3": the routines start from the cone at rest */
	nseg = 0;
}

static void wait(uint64_t cycles) { t += cycles; }

static void push_seg(uint64_t cycles, int lvl)
{
	if (nseg == segcap) {
		segcap = segcap ? segcap * 2 : 4096;
		seg_ns = realloc(seg_ns, (size_t)segcap * sizeof *seg_ns);
		seg_level = realloc(seg_level, (size_t)segcap);
	}
	seg_ns[nseg] = (uint32_t)(cycles * XT_CYCLE_NS_X1000 / 1000u / (port_speed_info ? port_speed_info : 1));
	seg_level[nseg] = (uint8_t)lvl;
	nseg++;
}

/* the cone moves */
static void edge(void)
{
	push_seg(t - t_last, level);
	t_last = t;
	level ^= 1;
}

/* OUT 61h,AL: the cone goes where bit 1 of AL says (it moves only if that differs) */
static void out61(void)
{
	t += 10;
	if (al != level)
		edge();
}

/* "out 61h,al / xor al,2": the loops' step */
static void toggle(void)
{
	out61();
	t += 4;
	al ^= 1;
}

/* the routine's end: port 61h as it was (the cone at rest) */
static void restore(void)
{
	al = 0;
	out61();
}

/* the effect is over: the game waited this long; the audio gets it */
static void end(void)
{
	int i;
	restore();
	push_seg(t - t_last + 1, level);
	vc_cycles((uint32_t)(t > 0xFFFFFFFFu ? 0xFFFFFFFFu : t));
	if (!audio_on || (rp_mode == RP_REPLAY && !rp_finished()))
		return;
	plat_audio_lock();
	for (i = 0; i < nseg; i++) {
		int next = (q_tail + 1) % QMAX;
		if (next == q_head)
			break;
		q_ns[q_tail] = seg_ns[i];
		q_level[q_tail] = seg_level[i];
		q_tail = next;
	}
	plat_audio_unlock();
}

/* "dec reg / jnz" n times (0 is 65536) */
#define DEC_JNZ 19
static uint64_t loops(uint32_t n, uint32_t per)
{
	n &= 0xFFFF;
	return (uint64_t)(n ? n : 0x10000) * per;
}

static void snd_step(void)                  /* C_1D69: one click */
{
	al ^= 1;	/* xor al,2 before the first out */
	out61();
	wait(loops(0x32, DEC_JNZ));
	al ^= 1;
	out61();
}

static void snd_blocked(void)               /* C_1D82: 8 periods of a low tone */
{
	uint16_t dx = (uint16_t)(0xCA * speed);
	int cx;
	wait(130);
	for (cx = 0x10; cx > 0; cx--) {
		toggle();
		wait(loops(dx, 3 + 14 + 15 + 12 + 12 + 16));	/* dec, pushf, push, pop, popf, jnz */
		wait(17);
	}
	restore();
}

static void snd_error(void)                 /* C_1DA8: 24 periods, then the low tone */
{
	uint16_t dx = (uint16_t)(0xE0 * speed);
	int cx;
	wait(130);
	for (cx = 0x30; cx > 0; cx--) {
		toggle();
		wait(loops(dx, DEC_JNZ));
		wait(17);
	}
	restore();
	snd_blocked();
}

static void snd_magic(uint8_t pitch)        /* C_1DCD: a pitch sweep down and up */
{
	uint16_t h = (uint16_t)((speed >> 1) + (speed & 1));
	uint16_t stop = (uint16_t)(0x1B * h), dx = (uint16_t)((uint8_t)h * pitch), cx = h;
	int ah;
	wait(250);
	do {
		for (ah = 0x30; ah > 0; ah--) {
			wait(loops(dx, DEC_JNZ));
			toggle();
			wait(loops(cx, DEC_JNZ));
			toggle();
			wait(20);
		}
		dx = (uint16_t)(dx - h);
		cx = (uint16_t)(cx + h);
	} while (cx != stop);
	do {
		for (ah = 0x30; ah > 0; ah--) {
			wait(loops(dx, DEC_JNZ));
			toggle();
			wait(loops(cx, DEC_JNZ));
			toggle();
			wait(20);
		}
		dx = (uint16_t)(dx + h);
		cx = (uint16_t)(cx - h);
	} while (cx != 0);
}

static void snd_hurt(void)                  /* C_1E53: 255 random half-periods */
{
	int bh;
	for (bh = 0xFF; bh > 0; bh--) {
		uint16_t dx = (uint16_t)(((u_rand_a() & 0x7F) | 0x40) * (speed & 0xFF));
		dx = (uint16_t)((dx >> 1) | 1);
		wait(110);
		wait(loops(dx, 3 + 3 + 3 + 16));	/* nop, nop, dec, jnz */
		toggle();
	}
}

static void snd_hit(void)                   /* C_1E7F: 255 random half-periods, shorter */
{
	int bh;
	for (bh = 0xFF; bh > 0; bh--) {
		uint16_t cx = (uint16_t)((u_rand_a() & 0x7F) | 1);
		cx = (uint16_t)((uint16_t)(cx * 3u) * speed);
		cx = (uint16_t)((cx >> 2) | 1);
		wait(200);
		wait(loops(cx, 17));	/* loop $ */
		toggle();
	}
}

static void snd_shoot(void)                 /* C_1EB3: rising period, 251 half-periods */
{
	unsigned bl;
	for (bl = 5; bl <= 0xFF; bl++) {
		uint16_t cx;
		toggle();
		cx = (uint16_t)(((speed & 0xFF) * bl) >> 1);
		wait(90);
		wait(loops(cx, 3 + 3 + 3 + 16));
	}
}

static void snd_swing(void)                 /* C_1EFD: the attack's whoosh */
{
	uint8_t bl = 0;
	do {
		uint16_t cx;
		toggle();
		cx = (uint16_t)(((speed & 0xFF) * bl) >> 1);
		wait(90);
		wait(loops(cx, 3 + 3 + 3 + 16));
		bl--;
	} while (bl & 0x80);
}

static void snd_failed(void)                /* C_1F22: falling period */
{
	unsigned bl;
	for (bl = 0x80; bl > 0; bl--) {
		uint16_t ax = (uint16_t)((speed & 0xFF) * bl);
		uint16_t cx = (uint16_t)((ax >> 1) + (ax & 1));
		toggle();
		wait(100);
		wait(loops(cx, 3 + 3 + 3 + 16));
	}
}

static void snd_foe_attack(void)            /* C_1F4A: rising period */
{
	unsigned bl;
	for (bl = 0x80; bl <= 0xFF; bl++) {
		uint16_t cx = (uint16_t)(((speed & 0xFF) * bl) >> 1);
		toggle();
		wait(90);
		wait(loops(cx, 3 + 12 + 16));	/* dec, 4 nops, jnz */
	}
}

static void snd_cast(uint8_t count)         /* C_1F73: count bursts of 20 periods */
{
	unsigned cl = count ? count : 0x100;
	for (; cl > 0; cl--) {
		uint8_t dl = (uint8_t)((u_rand_a() & 0x3F) + 0x40);
		uint16_t dx = (uint16_t)((uint16_t)((speed & 0xFF) * dl) << 1);
		int ch;
		wait(120);
		for (ch = 0x28; ch > 0; ch--) {
			wait(loops(dx, DEC_JNZ));
			toggle();
		}
	}
}

static void snd_vortex(int down)            /* C_1FA3, C_1FCC: the whirlpool, the twister */
{
	unsigned cx = down ? 0xC0 : 0x40;
	for (;;) {
		int ah;
		for (ah = 0x14; ah > 0; ah--) {
			uint16_t bx = (uint16_t)(cx * speed);
			wait(130);
			wait(loops(bx, 3 + 3 + 16));	/* nop, dec, jnz */
			toggle();
		}
		if (down) {
			if (--cx <= 0x40)
				break;
		} else if (++cx >= 0xC0)
			break;
	}
}

/* C_1D47: sound(effect, a, b): a is the magic's pitch (9) or the casting's length (10) */
int sound(int effect, int a, int b)
{
	(void)b;
	if (!SoundFlag || (unsigned)(effect & 0xFF) >= 13)
		return 0;
	speed = speed_info;
	begin();
	switch (effect & 0xFF) {
	case 0: snd_step(); break;
	case 1: snd_blocked(); break;
	case 2: snd_error(); break;
	case 3: snd_shoot(); break;
	case 4: snd_swing(); break;
	case 5: snd_foe_attack(); break;
	case 6: snd_hit(); break;
	case 7: snd_hurt(); break;
	case 8: snd_failed(); break;
	case 9: snd_magic((uint8_t)a); break;
	case 10: snd_cast((uint8_t)a); break;
	case 11: snd_vortex(0); break;
	case 12: snd_vortex(1); break;
	}
	end();
	return 0;
}

/* TITLE.EXE's BEEP.ASM: the same low tone (without a sound flag) */
int T_sound_1(void)
{
	speed = T_speed_info;
	begin();
	snd_blocked();
	end();
	return 0;
}

/* the cycles the next noise spreads over (0: EGA.DRV's shake); TITLE.EXE's logo sets it */
uint64_t ega_noise_span;

/* EGA.DRV's shake (ega.c), TITLE.EXE's logo (tgfx.c): the cone moved `toggles` times while
 * the screen was drawn (-1: the sound flag was off). Its time is already on the clock (the
 * drawing's). */
void ega_speaker_noise(int toggles)
{
	int i;
	uint64_t span = ega_noise_span ? ega_noise_span : 173u * (22u * 25u + 60u), step;
	if (toggles <= 0)
		return;
	speed = port_speed_info;
	begin();
	step = span / (uint64_t)(toggles + 1);
	for (i = 0; i < toggles; i++) {
		wait(step);
		edge();
	}
	t = span;
	push_seg(t - t_last + 1, level);
	if (!audio_on || (rp_mode == RP_REPLAY && !rp_finished()))
		return;
	plat_audio_lock();
	for (i = 0; i < nseg; i++) {
		int next = (q_tail + 1) % QMAX;
		if (next == q_head)
			break;
		q_ns[q_tail] = seg_ns[i];
		q_level[q_tail] = seg_level[i];
		q_tail = next;
	}
	plat_audio_unlock();
}

void sound_init(void)
{
	plat_set_music(mix);
}
