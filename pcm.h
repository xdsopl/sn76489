/*
sn76489 - sn76489 emulator for vgm
Written in 2012 by <Ahmet Inan> <xdsopl@googlemail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef PCM_H
#define PCM_H

typedef struct pcm {
	void (*close)(struct pcm *);
	void (*info)(struct pcm *);
	int (*rate)(struct pcm *);
	int (*channels)(struct pcm *);
	int (*rw)(struct pcm *, short *, int);
} pcm_t;

void close_pcm(pcm_t *);
void info_pcm(pcm_t *);
int rate_pcm(pcm_t *);
int channels_pcm(pcm_t *);
int read_pcm(pcm_t *, short *, int);
int write_pcm(pcm_t *, short *, int);
int open_pcm_read(pcm_t **, char *);
int open_pcm_write(pcm_t **, char *, int, int, float);

#endif

