/* The hooks of ega.h for the test shims (ega_shim.c, tgfx_shim.c): they record what the
 * code asked for, and serve files from shim_datadir (names without regard to case). */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

uint32_t ega_cost;          /* defined by sys/vclock.c in the port */
const char *shim_datadir = ".";
int shim_noise_calls, shim_noise_last = -2;
int shim_exit_calls, shim_exit_code = -1;
int shim_reads, shim_frees;
long shim_wait_total;
char shim_last_name[256];

void shim_reset_hooks(void)
{
	shim_noise_calls = 0; shim_noise_last = -2;
	shim_exit_calls = 0; shim_exit_code = -1;
	shim_reads = shim_frees = 0;
	shim_wait_total = 0;
	shim_last_name[0] = 0;
}

void ega_speaker_noise(int toggles) { shim_noise_calls++; shim_noise_last = toggles; }
void ega_fatal_exit(int code) { shim_exit_calls++; shim_exit_code = code; }
void ega_busy_wait(long iterations) { shim_wait_total += iterations; }
void ega_free_file(uint8_t *data) { shim_frees++; free(data); }

/* the file's name looked up without regard to case in shim_datadir */
int ega_read_file(const char *name, uint8_t **data, int *size)
{
	DIR *d;
	struct dirent *e;
	char path[1024];
	FILE *f;
	long n;
	snprintf(shim_last_name, sizeof shim_last_name, "%s", name);
	shim_reads++;
	if (!(d = opendir(shim_datadir)))
		return -1;
	path[0] = 0;
	while ((e = readdir(d)))
		if (!strcasecmp(e->d_name, name)) {
			snprintf(path, sizeof path, "%s/%s", shim_datadir, e->d_name);
			break;
		}
	closedir(d);
	if (!path[0] || !(f = fopen(path, "rb")))
		return -1;
	fseek(f, 0, SEEK_END);
	n = ftell(f);
	fseek(f, 0, SEEK_SET);
	*data = malloc(n ? n : 1);
	*size = (int)fread(*data, 1, n, f);
	fclose(f);
	return 0;
}

