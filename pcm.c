/*
sn76489 - sn76489 emulator for vgm
Written in 2012 by <Ahmet Inan> <xdsopl@googlemail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "pcm.h"
#include "alsa.h"
#include "wav.h"

void close_pcm(pcm_t *pcm)
{
	pcm->close(pcm);
	free(pcm);
}

void info_pcm(pcm_t *pcm)
{
	pcm->info(pcm);
}

int rate_pcm(pcm_t *pcm)
{
	return pcm->rate(pcm);
}

int channels_pcm(pcm_t *pcm)
{
	return pcm->channels(pcm);
}

int read_pcm(pcm_t *pcm, short *buff, int frames)
{
	return pcm->rw(pcm, buff, frames);
}

int write_pcm(pcm_t *pcm, short *buff, int frames)
{
	return pcm->rw(pcm, buff, frames);
}

int open_pcm_read(pcm_t **p, char *name)
{
	if (strstr(name, "plug:") == name || strstr(name, "plughw:") == name || strstr(name, "hw:") == name || strstr(name, "default") == name)
		return open_alsa_read(p, name);
	if (strstr(name, ".wav") == (name + (strlen(name) - strlen(".wav"))))
		return open_wav_read(p, name);
	return 0;
}

int open_pcm_write(pcm_t **p, char *name, int rate, int channels, float seconds)
{
	if (strstr(name, "plug:") == name || strstr(name, "plughw:") == name || strstr(name, "hw:") == name || strstr(name, "default") == name)
		return open_alsa_write(p, name, rate, channels, seconds);
	if (strstr(name, ".wav") == (name + (strlen(name) - strlen(".wav"))))
		return open_wav_write(p, name, rate, channels, seconds);
	return 0;
}

