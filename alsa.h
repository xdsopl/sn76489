/*
sn76489 - sn76489 emulator for vgm
Written in 2012 by <Ahmet Inan> <xdsopl@googlemail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef ALSA_H
#define ALSA_H
#include "pcm.h"
int open_alsa_read(pcm_t **, char *);
int open_alsa_write(pcm_t **, char *, int, int, float);
#endif

