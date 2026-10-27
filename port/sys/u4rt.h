/* The port's runtime for the game's assembly replacements (port/sys): the machine's clock,
 * the keyboard and the pace of the game.
 *
 * Time. The DOS game has no clock of its own: it runs as fast as the PC goes, and its
 * animations are counted in passes of its loops, scaled by speed_info (LOW.ASM measures
 * the CPU: speed_info = iterations of a 240-cycle loop in one timer tick / 1000, so 1 on a
 * 4.77 MHz PC/XT). The only real time it reads is DOS's clock (int 21h 2Ch): the seconds
 * of u_delay and the random seed.
 *
 * The port gives the game a virtual machine "speed_info times an XT": every routine of the
 * port that stands for DOS code adds the time that code took (its 8088 cycle count, divided
 * by speed_info) to a virtual clock (vc_*), and DOS's clock is the session's start time plus
 * the virtual clock. The game's polls of the keyboard keep pace with real time: they sleep
 * while the virtual clock is ahead. So the game runs at the speed of that PC, and everything
 * it does follows from its inputs alone: the start time, speed_info and the keys of each
 * poll, which is all a recording holds (port/replay).
 */
#ifndef PORT_U4RT_H
#define PORT_U4RT_H

#include <stdint.h>

/* ns of one 8088 cycle on the PC the calibration describes (240 cycles an iteration,
 * speed_info * 1000 iterations a tick of 65536/1193182 s) */
#define XT_CYCLE_NS_X1000 228850u   /* 228.85 ns: speed_info 1 is a 4.37 MHz 8088 */

/* the speed_info the port's low_init gives (settings; recorded) */
extern uint16_t port_speed_info;

/* ---- the virtual clock ---- */
void vc_start(void);                 /* the session starts: DOS date and time (recorded) */
/* tests: the date and time the PC's clock starts from (seconds since 1970, as a local time) */
void vc_fix_clock(long long seconds);
uint64_t vc_ns(void);                /* ns since vc_start */
void vc_cycles(uint32_t cycles);     /* 8088 cycles of the game's PC have passed */
void vc_add_ns(uint64_t ns);
/* keeps pace with real time: sleeps while the virtual clock is ahead, pumps the platform
 * and presents the screen; called at each poll of the keyboard */
void vc_pace(void);
/* pumps the platform (rate-limited) and presents the screen if it changed */
void port_pump(void);
/* replay speed: 1 real time, 2, 4, 8 faster, 0 no waiting */
void vc_set_speed(int multiple);

/* DOS's clock (int 21h 2Ch / 2Ah) */
void dos_gettime(uint8_t *hour, uint8_t *minute, uint8_t *second, uint8_t *hundredths);
void dos_getdate(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *weekday);

/* ---- the keyboard (int 16h) ---- */
/* AH=01h: the key at the head of the BIOS buffer, 0 if none (each result is recorded) */
uint16_t kb_peek(void);
/* AH=00h: takes the key (waits for one if none) */
uint16_t kb_read(void);

/* the cost in 8088 cycles of the EGA writes (port/gfx), taken by vc_pace */
extern uint32_t ega_cost;

#endif
