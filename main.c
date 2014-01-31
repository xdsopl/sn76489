/*
sn76489 - sn76489 emulator for vgm
Written in 2012 by <Ahmet Inan> <xdsopl@googlemail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "pcm.h"
#include "math.h"
#include "mmap_file.h"

static uint16_t divider[4];
static uint16_t attenuation[16];
static uint16_t counter[4];
static uint16_t volume[4];
static uint8_t flipflop[4];

int16_t sn76489()
{
	for (uint8_t i = 0; i < 3; i++)
		flipflop[i] ^= counter[i] ? 0 : 1;

	static uint16_t noise = 1 << 15;
	noise = counter[3] ? noise : (noise >> 1) | (((noise >> 3) ^ noise) << 15);
	flipflop[3] ^= noise & 1 ? 1 : 0;

	for (uint8_t i = 0; i < 4; i++)
		counter[i] = counter[i] ? counter[i] - 1 : divider[i];

	int16_t tmp = 0;
	for (uint8_t i = 0; i < 4; i++)
		tmp += flipflop[i] ? volume[i] : -volume[i];

	return tmp >> 1;
}

uint16_t parse_vgm(uint8_t *vgm, uint32_t size)
{
	static uint32_t pos = 0x40;
	static uint32_t loop_offset = 0;
	static uint32_t loop_samples = 0;
	static uint8_t loop_count = 0;
	static int init = 0;
	if (!init) {
		loop_offset = *(uint32_t *)(vgm+0x1C) + 0x1C;
		loop_samples = *(uint32_t *)(vgm+0x20);
		init = 1;
	}

	static int quit = 0;
	static uint32_t samples = 0;
	if (loop_samples && samples >= loop_samples) {
		pos = loop_offset;
		samples = 0;
		if (++loop_count > 2)
			quit = 1;
	}
	if (quit)
		return 0;
	uint16_t wait = 0;
	while (!wait && !quit && pos < size) {
		switch (vgm[pos]) {
			case 0x50:
				pos++;
				uint8_t byte = vgm[pos];
				static uint8_t addr = 0;
				static uint8_t type = 0;
				uint8_t first = byte & 128;
				if (first) {
					addr = (byte >> 5) & 3;
					type = (byte >> 4) & 1;
				}
				if (type) {
					volume[addr] = attenuation[byte & 0x0f];
				} else if (addr == 3) {
					divider[3] = 32 << (byte & 3);
				} else if (first) {
					divider[addr] = (divider[addr] & 0x3f0) | (byte & 0x0f);
				} else {
					divider[addr] = (divider[addr] & 0x0f) | ((byte & 0x3f) << 4);
				}
				pos++;
				break;
			case 0x61:
				pos++;
				wait = (vgm[pos+1] << 8) + vgm[pos];
				pos += 2;
				break;
			case 0x62:
				pos++;
				wait = 735;
				break;
			case 0x63:
				pos++;
				wait = 882;
				break;
			case 0x66:
				pos++;
				quit = 1;
				break;
			case 0x4f:
				pos += 2;
				break;
			case 0x51:
				pos += 3;
				break;
			case 0x70:
				pos++;
				wait = 1;
				break;
			case 0x71:
				pos++;
				wait = 2;
				break;
			case 0x72:
				pos++;
				wait = 3;
				break;
			case 0x73:
				pos++;
				wait = 4;
				break;
			case 0x74:
				pos++;
				wait = 5;
				break;
			case 0x75:
				pos++;
				wait = 6;
				break;
			case 0x76:
				pos++;
				wait = 7;
				break;
			case 0x77:
				pos++;
				wait = 8;
				break;
			case 0x78:
				pos++;
				wait = 9;
				break;
			case 0x79:
				pos++;
				wait = 10;
				break;
			case 0x7A:
				pos++;
				wait = 11;
				break;
			case 0x7B:
				pos++;
				wait = 12;
				break;
			case 0x7C:
				pos++;
				wait = 13;
				break;
			case 0x7D:
				pos++;
				wait = 14;
				break;
			case 0x7E:
				pos++;
				wait = 15;
				break;
			case 0x7F:
				pos++;
				wait = 16;
				break;
			default:
				fprintf(stderr, "unknown cmd at 0x%x: 0x%x\n", pos, vgm[pos]);
				fflush(stderr);
				pos++;
				break;
		}
	}
	if (loop_samples && pos >= loop_offset)
		samples += wait;
	if (quit || pos >= size)
		return 0;
	return wait;
}
int main(int argc, char **argv)
{
	char *name = "default";
	float time = 180;
	if (argc == 3) {
		name = argv[2];
		time = 180;
	}
	if (argc < 2)
		return 1;
	pcm_t *pcm;
	if (!open_pcm_write(&pcm, name, 48000, 1, time))
		return 1;

	void *inp;
	size_t size;
	if (!mmap_file_ro(&inp, argv[1], &size))
		return 1;
	uint8_t *vgm = (uint8_t *)inp;
	uint32_t loop_offset = *(uint32_t *)(vgm+0x1C) + 0x1C;
	uint32_t loop_samples = *(uint32_t *)(vgm+0x20);
	uint32_t clock_rate = *(uint32_t *)(vgm+0x0C);
	fprintf(stderr, "loop offset: 0x%x\n", loop_offset);
	fprintf(stderr, "loop samples: 0x%x\n", loop_samples);
	fprintf(stderr, "clock rate: %d\n", clock_rate);

	int rate = rate_pcm(pcm);
	int channels = channels_pcm(pcm);
	int samples = 1024;
	short *buff = (short *)malloc(sizeof(short)*channels*samples);
	info_pcm(pcm);

	for (int i = 0; i < 4; i++)
		volume[i] = 0;

	for (int i = 0; i < 15; i++)
		attenuation[i] = 8000 * powf(10, -0.1 * i);

	attenuation[15] = 0;

#if 0
	for (int i = 0; i < 16; i++)
		fprintf(stderr, "%d ", attenuation[i]);
	fprintf(stderr, "\n");
#endif

	uint32_t wait = 0;
	int64_t clock = 0;
	int64_t frame = 0;
	static int hist_div[4][1024];
	static int hist_wait[65536];
	int min[4] = { INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX };
	int max[4] = { INT32_MIN, INT32_MIN, INT32_MIN, INT32_MIN };
	int quit = 0;
	do {
		for (int sample = 0; sample < samples; sample++) {
			if (!wait) {
				wait = parse_vgm(vgm, size);
				if (!wait) {
					quit = 1;
					break;
				}
				hist_wait[wait]++;
				wait = (rate * wait) / 44100;
			} else {
				wait--;
			}
			for (int i = 0; i < 4; i++) {
				if (!divider[i])
					continue;
				hist_div[i][divider[i]]++;
				min[i] = min[i] < divider[i] ? min[i] : divider[i];
				max[i] = max[i] > divider[i] ? max[i] : divider[i];
			}
			int16_t tmp = 0;
			for (;clock * rate * 16 < frame * clock_rate; clock++)
				tmp = sn76489();
			frame++;
			for (int channel = 0; channel < channels; channel++)
				buff[channels * sample + channel] = tmp;
		}
	} while (!quit && write_pcm(pcm, buff, samples));

	munmap_file(inp, size);
	close_pcm(pcm);

	static int symbols_div[4];
	for (int i = 0; i < 4; i++)
		for (int k = 0; k < 1024; k++)
			if (hist_div[i][k]) symbols_div[i]++;

	for (int i = 0; i < 4; i++)
		fprintf(stderr, "min[%d]: %d, max[%d]: %d\n", i, min[i], i, max[i]);

	for (int i = 0; i < 4; i++)
		fprintf(stderr, "symbols_div[%d]: %d\n", i, symbols_div[i]);

	for (int i = 0; i < 1024; i++)
		if (hist_div[3][i])
			fprintf(stderr, "hist_div[3][%d]: %d\n", i, hist_div[3][i]);

	for (int i = 0; i < 65536; i++)
		if (hist_wait[i])
			fprintf(stderr, "hist_wait[%d]: %d\n", i, hist_wait[i]);
	return 0;
}
