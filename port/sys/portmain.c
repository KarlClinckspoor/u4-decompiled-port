/* replaces ULTIMA.COM and the C start-up of the two programs: the port's main. It finds
 * the game, sets up the files, opens the window, then runs the programs as ULTIMA.COM
 * does: TITLE.EXE (the introduction, the menu, a new party), and when it ends with a drive
 * code ('0', '1' or a drive letter), AVATAR.EXE with that code at the end of its command
 * line. AVATAR.EXE's session is recorded (the black box) or replayed; TITLE.EXE's part is
 * not, so a recording starts from the saved game the game reads.
 *
 *   u4 [--data DIR] [--home DIR] [--replay REC] [--record DIR] [--no-recording]
 *      [--speed N] [--hidden] [--scale N] [--fullscreen] [--no-aspect] [--no-integer]
 *      [--no-audio] [--screenshot FILE] [--settings] [--title|--game] [--help]
 */
#include "session.h"
#include "dosfs.h"
#include "u4rt.h"
#include "../replay/replay.h"
#include "../platform/plat.h"
#include "../platform/hostfs.h"
#include "../ui/overlay.h"
#include "../ui/settings.h"
#include "../gfx/ega.h"

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void game_main(void);           /* AVATAR.EXE's main (SRC/U4_MAIN.C) */
void T_game_main(void);         /* TITLE.EXE's (SRC-TITLE/TITLE_0.C) */
void low_set_command_tail(const uint8_t *tail, int len);
void T_low_set_command_tail(const uint8_t *tail, int len);
uint64_t port_game_digest(void);
void port_exit(int code);

static uint8_t font[2048];
static int quit_after_replay = 1;
static const char *screenshot_path;
static int hidden;
static uint32_t run_ms;          /* --run-ms: end the session after this long (tests) */
static uint32_t fast_base;       /* the virtual clock restarts with each program */
static int fast;                 /* --fast: no pacing; --keys and --run-ms in the PC's time */

/* the time the test options go by */
static uint32_t test_ms(void)
{
	return fast ? (uint32_t)(vc_ns() / 1000000u) + fast_base : plat_real_ms();
}

enum { RUN_NONE, RUN_TITLE, RUN_AVATAR };
static int running;
static int ending;              /* the port ends the session (not a program's exit) */
static jmp_buf title_done;

/* files that mark the game folder */
static const char *const marks[] = {"WORLD.MAP", "SHAPES.EGA", "CHARSET.EGA", "EGA.DRV", "TITLE.EXE", 0};

static int is_game_folder(const char *dir)
{
	int i;
	if (!dir || !dir[0] || !hostfs_is_dir(dir))
		return 0;
	if (dosfs_init(dir, plat_home_dir()) != 0)
		return 0;
	for (i = 0; marks[i]; i++)
		if (!dosfs_exists(marks[i]))
			return 0;
	return 1;
}

static const char *find_game(const char *given)
{
	static const char *const guesses[] = {
		".", "gamedata", "ULTIMA4", "Ultima 4", "..", "../gamedata",
#ifdef _WIN32
		"C:\\GOG Games\\Ultima 4", "C:\\Program Files (x86)\\GOG Galaxy\\Games\\Ultima 4",
		"C:\\Program Files (x86)\\GOG.com\\Ultima 4", "C:\\GOG Games\\Ultima IV",
		"C:\\ULTIMA4",
#endif
		0};
	static char buf[1100];
	const char *env = getenv("U4_DATA");
	int i;
	if (given)
		return is_game_folder(given) ? given : 0;
	if (env && is_game_folder(env))
		return env;
	if (settings.data[0] && is_game_folder(settings.data))
		return settings.data;
	for (i = 0; guesses[i]; i++)
		if (is_game_folder(guesses[i]))
			return guesses[i];
	for (i = 0; guesses[i]; i++) {
		hostfs_join(buf, sizeof buf, plat_base_dir(), guesses[i]);
		if (is_game_folder(buf))
			return buf;
	}
	return 0;
}

static void write_screenshot(const char *path)
{
	/* a 320x200 8-bit BMP of what is on the screen */
	static uint8_t px[320 * 200], pal[768];
	FILE *f;
	uint8_t hdr[54 + 1024];
	int y, i;
	uint32_t size = 54 + 1024 + 320 * 200;
	if (plat_video_capture(px, pal) != 0 || !(f = fopen(path, "wb")))
		return;
	memset(hdr, 0, sizeof hdr);
	hdr[0] = 'B'; hdr[1] = 'M';
	hdr[2] = (uint8_t)size; hdr[3] = (uint8_t)(size >> 8); hdr[4] = (uint8_t)(size >> 16);
	hdr[10] = (uint8_t)(54 + 1024); hdr[11] = (uint8_t)((54 + 1024) >> 8);
	hdr[14] = 40; hdr[18] = 0x40; hdr[19] = 1; hdr[22] = 200; hdr[26] = 1; hdr[28] = 8;
	hdr[47] = 1;	/* 256 colours */
	for (i = 0; i < 256; i++) {
		hdr[54 + i * 4 + 2] = (uint8_t)(pal[i * 3] << 2 | pal[i * 3] >> 4);
		hdr[54 + i * 4 + 1] = (uint8_t)(pal[i * 3 + 1] << 2 | pal[i * 3 + 1] >> 4);
		hdr[54 + i * 4 + 0] = (uint8_t)(pal[i * 3 + 2] << 2 | pal[i * 3 + 2] >> 4);
	}
	fwrite(hdr, 1, sizeof hdr, f);
	for (y = 199; y >= 0; y--)
		fwrite(px + y * 320, 1, 320, f);
	fclose(f);
}

/* Called by the programs' exit (compat.h renames exit to port_exit). TITLE.EXE's exit
 * returns to main, which goes on as ULTIMA.COM does. */
void port_exit(int code)
{
	if (running == RUN_TITLE && !plat_quit_requested() && !ending)
		longjmp(title_done, (code & 0xFF) + 1);
	if (rp_mode == RP_REPLAY || rp_finished())
		plat_log("replay: %u input reads, %u of %u checkpoints compared, %s", rp_call_count(),
			rp_checks_compared(), rp_checks_total(), rp_diverged() ? "DIFFERENT from the recording" : "all identical");
	if (screenshot_path)
		write_screenshot(screenshot_path);
	if (running == RUN_AVATAR && code == 4 && rp_mode != RP_REPLAY && !ending && !hidden) {
		/* "No party formed!": ULTIMA.COM runs TITLE.EXE again */
		plat_log("AVATAR.EXE found no party: back to the title");
		session_end();
		session_restart_menu();
	}
	if (code != 0 && !ending && rp_mode != RP_REPLAY) {
		char msg[200];
		snprintf(msg, sizeof msg, "%s ended with code %d (%s).", running == RUN_AVATAR ? "AVATAR.EXE" : "The port",
			code, code == 3 ? "a file could not be read" : code == 0x49 ? "the graphics driver is missing" : "an error");
		plat_log("%s", msg);
		if (!hidden && code != 4)
			plat_message(msg);
	}
	session_end();
	plat_shutdown();
	exit(rp_diverged() ? 3 : code);
}

/* Replays: when the recording runs out, stop (tests) or let the player take over. */
static void replay_watch(void)
{
	static int reported;
	if (rp_mode == RP_REPLAY && rp_finished() && !reported) {
		reported = 1;
		if (quit_after_replay)
			port_exit(0);
		session_take_over();
	}
}

static void pump_hook_chain(void);
static void test_options(void);

/* --keys "MS:KEY MS:KEY ...": keys typed at those times (tests; the recording has them as
 * the game read them). KEY: a character, or up down left right enter esc space bksp tab,
 * f1..f12, ^x for Ctrl+x. */
static const char *key_script;
static const char scan_row[] = "1234567890-=\0\0qwertyuiop[]\0\0asdfghjkl;'`\0\\zxcvbnm,./";

static uint16_t bios_word(const char *k, size_t n)
{
	static const struct { const char *name; uint16_t w; } named[] = {
		{"up", 0x4800}, {"down", 0x5000}, {"left", 0x4B00}, {"right", 0x4D00},
		{"enter", 0x1C0D}, {"esc", 0x011B}, {"space", 0x3920}, {"bksp", 0x0E08}, {"tab", 0x0F09},
		{0, 0}};
	int i;
	if (n == 1) {
		char c = *k, lc = (char)(c >= 'A' && c <= 'Z' ? c + 32 : c);
		for (i = 0; i < (int)sizeof scan_row - 1; i++)
			if (scan_row[i] && scan_row[i] == lc)
				return (uint16_t)((i + 2) << 8 | (unsigned char)c);
		return (unsigned char)c;
	}
	for (i = 0; named[i].name; i++)
		if (strlen(named[i].name) == n && !strncmp(named[i].name, k, n))
			return named[i].w;
	if (n == 2 && k[0] == '^') {	/* ^s: Ctrl+S */
		uint16_t w = bios_word(k + 1, 1);
		return (uint16_t)((w & 0xff00) | ((w & 0x1f)));
	}
	if (n >= 2 && (k[0] == 'f' || k[0] == 'F')) {
		int f = atoi(k + 1);
		if (f >= 1 && f <= 10)
			return (uint16_t)((0x3A + f) << 8);
		if (f == 11 || f == 12)
			return (uint16_t)((0x57 + f - 11) << 8);
	}
	return 0;
}

static void run_key_script(void)
{
	while (key_script && *key_script) {
		const char *p = key_script, *colon, *end;
		unsigned long at;
		while (*p == ' ')
			p++;
		if (!*p) {
			key_script = 0;
			return;
		}
		at = strtoul(p, 0, 10);
		if (test_ms() < at)
			return;
		colon = strchr(p, ':');
		if (!colon) {
			key_script = 0;
			return;
		}
		end = strchr(colon + 1, ' ');
		if (!end)
			end = colon + 1 + strlen(colon + 1);
		plat_push_key(bios_word(colon + 1, (size_t)(end - colon - 1)));
		key_script = end;
	}
}

static void usage(void)
{
	printf("Ultima IV, native port\n\n"
		"  --data DIR          the game folder (remembered)\n"
		"  --home DIR          where settings, saved games and recordings go\n"
		"  --replay REC        play a recording (a folder, or a name in recordings/)\n"
		"  --speed N           replay speed: 1, 2, 4, 8, or 0 for as fast as possible\n"
		"  --continue          after a replay ends, keep playing (default: quit)\n"
		"  --record DIR        record this session into DIR\n"
		"  --no-recording      no black box recording this time\n"
		"  --hidden            no window (replays, tests)\n"
		"  --screenshot FILE   write the screen as a BMP when the game ends\n"
		"  --run-ms N          end the session after N ms (tests)\n"
		"  --keys SCRIPT       type keys at given times (tests): \"1500:up 2000:a\"\n"
		"  --clock TIME        the PC's clock starts at YYYYMMDD-HHMMSS (tests)\n"
		"  --fast              no pacing: the game runs as fast as it can, --keys and\n"
		"                      --run-ms count the game's own time (tests)\n"
		"  --scale N, --fullscreen, --no-aspect, --no-integer, --no-audio\n"
		"  --settings          open the settings screen before the game\n"
		"  --title, --game     start with TITLE.EXE (the introduction and its menu), or\n"
		"                      with the game when there is a party (default: the setting)\n"
		"F11 opens the settings screen in the game.\n");
}

/* the overlay's font: CHARSET.EGA's first 256 characters (8x8, two pixels a byte) in one
 * bit a pixel */
static void load_font(void)
{
	static uint8_t cs[0x2000];
	int16_t h;
	uint16_t got = 0;
	int c, r, x;
	if (dosfs_open("CHARSET.EGA", 0, &h) != 0)
		return;
	dosfs_read(h, cs, sizeof cs, &got);
	dosfs_close(h);
	for (c = 0; c < 256 && (c + 1) * 32 <= got; c++)
		for (r = 0; r < 8; r++) {
			uint8_t bits = 0;
			for (x = 0; x < 8; x++) {
				uint8_t b = cs[c * 32 + r * 4 + x / 2];
				if ((x & 1 ? b & 0x0F : b >> 4) != 0)
					bits |= (uint8_t)(0x80 >> x);
			}
			font[c * 8 + r] = bits;
		}
}

/* PARTY.SAV's member count (0: no party formed yet) */
static int party_members(void)
{
	int16_t h;
	uint8_t b[2] = {0, 0};
	uint16_t got;
	uint32_t pos;
	if (dosfs_open("PARTY.SAV", 0, &h) != 0)
		return 0;
	dosfs_seek(h, 0, 0x1D8, &pos);
	dosfs_read(h, b, 2, &got);
	dosfs_close(h);
	return b[0] | b[1] << 8;
}

/* TITLE.EXE; returns its exit code */
static int run_title(void)
{
	int r;
	static const uint8_t none[1] = {0};
	dosfs_init(settings.data, session_home());
	vc_start();
	T_low_set_command_tail(none, 0);
	running = RUN_TITLE;
	if (fast)
		vc_set_speed(0);
	r = setjmp(title_done);
	if (!r) {
		T_game_main();
		r = 1;	/* main returned: exit(0) */
	}
	running = RUN_NONE;
	return r - 1;
}

int main(int argc, char **argv)
{
	const char *data = 0, *home = 0, *replay = 0, *record = 0;
	int title = -1;
	int recording = -1, speed = -1, audio = 1, settings_now = 0, i;
	int scale = 0, fullscreen = -1, aspect = -1, integer = -1, first_run;
	PlatConfig cfg;
	const char *game;
	char homebuf[1100];
	uint8_t tail[4] = {0x0D, ' ', ' ', '0'};

	for (i = 1; i < argc; i++) {
		const char *a = argv[i];
#define ARG (i + 1 < argc ? argv[++i] : (usage(), exit(2), (char *)0))
		if (!strcmp(a, "--data")) data = ARG;
		else if (!strcmp(a, "--home")) home = ARG;
		else if (!strcmp(a, "--replay")) replay = ARG;
		else if (!strcmp(a, "--record")) record = ARG;
		else if (!strcmp(a, "--no-recording")) recording = 0;
		else if (!strcmp(a, "--speed")) speed = atoi(ARG);
		else if (!strcmp(a, "--continue")) quit_after_replay = 0;
		else if (!strcmp(a, "--hidden")) hidden = 1;
		else if (!strcmp(a, "--screenshot")) screenshot_path = ARG;
		else if (!strcmp(a, "--scale")) scale = atoi(ARG);
		else if (!strcmp(a, "--fullscreen")) fullscreen = 1;
		else if (!strcmp(a, "--no-aspect")) aspect = 0;
		else if (!strcmp(a, "--no-integer")) integer = 0;
		else if (!strcmp(a, "--no-audio")) audio = 0;
		else if (!strcmp(a, "--settings")) settings_now = 1;
		else if (!strcmp(a, "--title") || !strcmp(a, "--menu")) title = 1;
		else if (!strcmp(a, "--game")) title = 0;
		else if (!strcmp(a, "--keys")) key_script = ARG;
		else if (!strcmp(a, "--fast")) fast = 1;
		else if (!strcmp(a, "--clock")) {
			/* YYYYMMDD-HHMMSS */
			const char *c = ARG;
			int y, mo, d, h, mi, se;
			if (sscanf(c, "%4d%2d%2d-%2d%2d%2d", &y, &mo, &d, &h, &mi, &se) == 6) {
				long long days;
				int yy = y - (mo <= 2), era = yy / 400, yoe = yy - era * 400;
				int doy = (153 * (mo + (mo > 2 ? -3 : 9)) + 2) / 5 + d - 1;
				days = (long long)era * 146097 + yoe * 365 + yoe / 4 - yoe / 100 + doy - 719468;
				vc_fix_clock(days * 86400 + h * 3600 + mi * 60 + se);
			}
		}
		else if (!strcmp(a, "--run-ms")) run_ms = (uint32_t)atol(ARG);
		else if (!strcmp(a, "--help") || !strcmp(a, "-h")) { usage(); return 0; }
		else { fprintf(stderr, "unknown option %s\n", a); usage(); return 2; }
#undef ARG
	}

	settings_defaults();
	if (!home) {
		snprintf(homebuf, sizeof homebuf, "%s", plat_home_dir());
		home = homebuf;
	}
	hostfs_mkdirs(home);
	first_run = !settings_load(home);
	session_set_home(home);
	{
		char logp[1100];
		hostfs_join(logp, sizeof logp, home, "u4port.log");
		plat_log_to(logp);
	}
	memset(&cfg, 0, sizeof cfg);
	cfg.scale = scale ? scale : settings.scale;
	cfg.fullscreen = fullscreen >= 0 ? fullscreen : settings.fullscreen;
	cfg.aspect = aspect >= 0 ? aspect : settings.aspect;
	cfg.integer_scale = integer >= 0 ? integer : settings.integer_scale;
	cfg.audio = audio && !hidden;
	cfg.volume = settings.sound ? settings.volume : 0;
	cfg.hidden = hidden;
	cfg.title = "Ultima IV";
	if (plat_init(&cfg) != 0)
		return 1;

	game = find_game(data);
	if (!game && !hidden) {
		const char *pick;
		plat_message(data ? "Ultima IV's files are not in the folder given with --data. "
			"Choose the folder of your Ultima IV (the one with WORLD.MAP, SHAPES.EGA, ...)."
			: "Choose the folder of your Ultima IV (the one with WORLD.MAP, SHAPES.EGA, ...).");
		pick = plat_choose_folder("The folder of Ultima IV");
		if (pick && is_game_folder(pick)) {
			game = pick;
			data = pick;
		}
	}
	if (!game) {
		plat_message("Ultima IV's files were not found. Start the port with --data <the game folder> "
			"(the folder with WORLD.MAP, SHAPES.EGA, ...).");
		plat_shutdown();
		return 1;
	}
	if (data || !settings.data[0] || strcmp(settings.data, game)) {
		snprintf(settings.data, sizeof settings.data, "%s", game);
		settings_save();
	}

	/* the overlay draws with the game's own font; the screen is black until the game draws */
	dosfs_init(settings.data, home);
	load_font();
	vc_start();
	overlay_init(font);
	plat_set_pump_hook(pump_hook_chain);
	overlay_set_idle_hook(test_options);
	sound_init();
	sound_set_volume(settings.volume, settings.sound);
	port_speed_info = (uint16_t)settings.speed;

	if (!hidden && (settings_now || ((settings.settings_at_start || first_run) && !replay)))
		overlay_run_modal();

	/* TITLE.EXE, as ULTIMA.COM runs it first; it comes before the session starts, so that a
	 * recording's copy of the saved game is the one AVATAR.EXE reads. Without a party it
	 * always runs (AVATAR.EXE would send the player back). */
	if (!replay && (title > 0 || (title < 0 && settings.start_title) || party_members() == 0)) {
		int code = run_title();
		if (!((code >= '0' && code <= '1') || (code >= 'A' && code <= 'P'))) {
			if (code != 0 && !plat_quit_requested()) {
				char msg[100];
				snprintf(msg, sizeof msg, "TITLE.EXE ended with code %d.", code);
				plat_log("%s", msg);
				if (!hidden)
					plat_message(msg);
			}
			plat_shutdown();
			return code;
		}
		tail[3] = (uint8_t)code;
		ega_clear_screen();
		fast_base = test_ms();
	}

	if (session_start(home, replay, record, !replay && (recording < 0 ? settings.recording : recording),
			argv[0]) != 0)
		return 1;
	if (dosfs_init(settings.data, session_game_home()) != 0) {
		fprintf(stderr, "cannot use the game folder %s\n", settings.data);
		return 1;
	}
	if (speed >= 0)
		session_set_replay_speed(speed);
	else if (hidden)
		session_set_replay_speed(0);
	rp_set_digest(port_game_digest);
	vc_start();
	if (fast)
		vc_set_speed(0);
	/* ULTIMA.COM's command line for AVATAR.EXE: its own (empty: the CR), two spaces, and
	 * TITLE.EXE's exit code (one drive: '0') */
	low_set_command_tail(tail, 4);
	running = RUN_AVATAR;
	game_main();
	port_exit(0);
	return 0;
}

/* the overlay's pump hook (set by overlay_init) runs first, then the replay's end check */
extern void overlay_pump_hook(void);
static void test_options(void)
{
	run_key_script();
	if (run_ms && test_ms() >= run_ms) {
		ending = 1;
		port_exit(0);
	}
}

static void pump_hook_chain(void)
{
	overlay_pump_hook();
	replay_watch();
	test_options();
	if (plat_quit_requested()) {
		ending = 1;
		port_exit(0);
	}
}
