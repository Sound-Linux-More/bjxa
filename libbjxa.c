/*-
 * Copyright (C) 2018  Dridi Boukelmoune
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bjxa.h"

/* miniobj.h-inspired macros */

#define ALLOC_OBJ(o, m) \
	do { \
		errno = 0; \
		(o) = calloc(1, sizeof *(o)); \
		if ((o) != NULL) \
			(o)->magic = (m); \
	} while (0)

#define INIT_OBJ(o, m) \
	do { \
		(void)memset((o), 0, sizeof(*(o))); \
		(o)->magic = (m); \
	} while (0)

#define FREE_OBJ(o) \
	do { \
		(void)memset((o), 0, sizeof(*(o))); \
		free(o); \
		(o) = NULL; \
	} while (0)

#define CHECK_PTR(p) \
	do { \
		if ((p) == NULL) { \
			errno = EINVAL; \
			return (-1); \
		} \
	} while (0)

#define CHECK_OBJ(o, m) \
	do { \
		if ((o) == NULL || (o)->magic != (m)) { \
			errno = EINVAL; \
			return (-1); \
		} \
	} while (0)

#define TAKE_OBJ(o, p, m) \
	do { \
		if ((p) == NULL || *(p) == NULL || (*(p))->magic != (m)) { \
			errno = EINVAL; \
			return (-1); \
		} \
		o = *p; \
		*p = NULL; \
	} while (0)

#define VALID_OBJ(o, m) ((o) != NULL && (o)->magic == (m))

/* error handling */

#define BJXA_TRY(res) \
	do { \
		if ((res) < 0) \
			return (-1); \
	} while (0)

#define BJXA_COND_CHECK(cond, err) \
	do { \
		if (!(cond)) { \
			errno = (err); \
			return (-1); \
		} \
	} while (0)

#define BJXA_BUFFER_CHECK(cond) BJXA_COND_CHECK((cond), ENOBUFS)
#define BJXA_PROTO_CHECK(cond) BJXA_COND_CHECK((cond), EPROTO)

/* little endian */

static inline uint32_t
mread_le(const uint8_t **buf, unsigned bits)
{
	unsigned n = 0;
	uint32_t res = 0;

	assert(buf != NULL);
	assert(*buf != NULL);

	while (n < bits) {
		res |= (uint32_t)(**buf << n);
		*buf += 1;
		n += 8;
	}

	return (res);
}

static uint8_t
mread_le8(const uint8_t **buf)
{
	uint32_t res;

	/* NB: I know that byte order doesn't make sense for a single octet
	 * but this is for consistency.
	 */
	res = mread_le(buf, 8);
	assert(res <= UINT8_MAX);
	return ((uint8_t)res);
}

static uint16_t
mread_le16(const uint8_t **buf)
{
	uint32_t res;

	res = mread_le(buf, 16);
	assert(res <= UINT16_MAX);
	return ((uint16_t)res);
}

static uint32_t
mread_le32(const uint8_t **buf)
{

	return (mread_le(buf, 32));
}

static void
mwrite_le(uint8_t **buf, uint32_t val, unsigned bits)
{
	unsigned n = 0;

	assert(buf != NULL);
	assert(*buf != NULL);

	while (n < bits) {
		**buf = (uint8_t)val;
		val >>= 8;
		*buf += 1;
		n += 8;
	}
}

static void
mputs(uint8_t **buf, const char *str)
{
	size_t len;

	assert(str != NULL);
	assert(buf != NULL);
	assert(*buf != NULL);

	len = strlen(str);
	(void)memcpy(*buf, str, len);
	*buf += len;
}

/* data structures */

#define BJXA_HEADER_MAGIC	0x3144574b
#define BJXA_HEADER_SIZE	32
#define BJXA_BLOCK_SAMPLES	32

typedef uint8_t bjxa_inflate_f(bjxa_decoder_t *, int16_t *, const uint8_t *);

typedef struct {
	int16_t			prev[2];
} bjxa_channel_t;

struct bjxa_decoder {
	uint32_t		magic;
#define BJXA_DECODER_MAGIC	0x234ec0c2
	uint32_t		data_len;
	uint32_t		samples;
	uint16_t		samples_rate;
	uint8_t			block_size;
	uint8_t			channels;
	bjxa_channel_t		channel_state[2];
	bjxa_inflate_f		*inflate_cb;
};

/* memory management */

bjxa_decoder_t *
bjxa_decoder(void)
{
	bjxa_decoder_t *dec;

	ALLOC_OBJ(dec, BJXA_DECODER_MAGIC);
	return (dec);
}

int
bjxa_free_decoder(bjxa_decoder_t **decp)
{
	bjxa_decoder_t *dec;

	TAKE_OBJ(dec, decp, BJXA_DECODER_MAGIC);
	FREE_OBJ(dec);
	return (0);
}

/* inflate XA blocks */

static uint8_t
bjxa_inflate_4bits(bjxa_decoder_t *dec, int16_t *dst, const uint8_t *src)
{
	unsigned n, chan;
	uint8_t profile;

	assert(VALID_OBJ(dec, BJXA_DECODER_MAGIC));
	assert(dec->channels == 1 || dec->channels == 2);

	chan = dec->channels;
	profile = *src;
	src++;

	for (n = BJXA_BLOCK_SAMPLES; n > 0; n -= 2) {
		*dst = (int16_t)((src[0] & 0xf0) << 8);  dst += chan;
		*dst = (int16_t)((src[0] & 0x0f) << 12); dst += chan;
		src++;
	}

	return (profile);
}

static uint8_t
bjxa_inflate_6bits(bjxa_decoder_t *dec, int16_t *dst, const uint8_t *src)
{
	unsigned n, chan;
	uint32_t samples;
	uint8_t profile;

	assert(VALID_OBJ(dec, BJXA_DECODER_MAGIC));
	assert(dec->channels == 1 || dec->channels == 2);

	chan = dec->channels;
	profile = *src;
	src++;

	for (n = BJXA_BLOCK_SAMPLES; n > 0; n -= 4) {
		samples = (uint32_t)(src[0] << 16) | (uint32_t)(src[1] << 8) |
		    src[2];

		*dst = (int16_t)((samples & 0x00fc0000) >> 8);  dst += chan;
		*dst = (int16_t)((samples & 0x0003f000) >> 2);  dst += chan;
		*dst = (int16_t)((samples & 0x00000fc0) << 4);  dst += chan;
		*dst = (int16_t)((samples & 0x0000003f) << 10); dst += chan;

		src += 3;
	}

	return (profile);
}

static uint8_t
bjxa_inflate_8bits(bjxa_decoder_t *dec, int16_t *dst, const uint8_t *src)
{
	unsigned n, chan;
	uint8_t profile;

	assert(VALID_OBJ(dec, BJXA_DECODER_MAGIC));
	assert(dec->channels == 1 || dec->channels == 2);

	chan = dec->channels;
	profile = *src;
	src++;

	for (n = BJXA_BLOCK_SAMPLES; n > 0; n--) {
		*dst = (int16_t)(src[0] << 8);
		dst += chan;
		src++;
	}

	return (profile);
}

/* XA header */

ssize_t
bjxa_parse_header(bjxa_decoder_t *dec, const void *src, size_t len)
{
	bjxa_decoder_t tmp;
	uint32_t magic, pad, max_samples, loop;
	const uint8_t *buf;
	uint8_t bits;

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(src);
	BJXA_BUFFER_CHECK(len >= BJXA_HEADER_SIZE);

	buf = src;

	INIT_OBJ(&tmp, BJXA_DECODER_MAGIC);
	magic = mread_le32(&buf);
	tmp.data_len = mread_le32(&buf);
	tmp.samples = mread_le32(&buf);
	tmp.samples_rate = mread_le16(&buf);
	bits = mread_le8(&buf);
	tmp.channels = mread_le8(&buf);
	loop = mread_le32(&buf);
	tmp.channel_state[0].prev[0] = (int16_t)mread_le16(&buf);
	tmp.channel_state[0].prev[1] = (int16_t)mread_le16(&buf);
	tmp.channel_state[1].prev[0] = (int16_t)mread_le16(&buf);
	tmp.channel_state[1].prev[1] = (int16_t)mread_le16(&buf);
	pad = mread_le32(&buf);

	assert((uintptr_t)buf - (uintptr_t)src == BJXA_HEADER_SIZE);

	BJXA_PROTO_CHECK(magic == BJXA_HEADER_MAGIC);
	BJXA_PROTO_CHECK(bits == 4 || bits == 6 || bits == 8);
	BJXA_PROTO_CHECK(tmp.channels == 1 || tmp.channels == 2);

	tmp.block_size = bits * 4 + 1;
	max_samples = (BJXA_BLOCK_SAMPLES * tmp.data_len) /
	    (tmp.block_size * tmp.channels);
	BJXA_PROTO_CHECK(max_samples >= tmp.samples);
	BJXA_PROTO_CHECK(max_samples - tmp.samples < BJXA_BLOCK_SAMPLES);

	if (bits == 4)
		tmp.inflate_cb = bjxa_inflate_4bits;
	else if (bits == 6)
		tmp.inflate_cb = bjxa_inflate_6bits;
	else
		tmp.inflate_cb = bjxa_inflate_8bits;

	(void)loop;
	(void)pad;

	(void)memcpy(dec, &tmp, sizeof tmp);
	return (BJXA_HEADER_SIZE);
}

ssize_t
bjxa_fread_header(bjxa_decoder_t *dec, FILE *file)
{
	uint8_t buf[BJXA_HEADER_SIZE];
	ssize_t ret;

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(file);

	if (fread(buf, sizeof buf, 1, file) != 1) {
		if (feof(file))
			errno = ENODATA;
		return (-1);
	}

	ret = bjxa_parse_header(dec, buf, sizeof buf);
	if (ret < 0) {
		assert(errno != EINVAL);
		assert(errno != ENOBUFS);
	}
	return (ret);
}

/* decode XA blocks */

static const int16_t spread_factor[][2] = {
	{0x0000, 0x0000},
	{0x00f0, 0x0000},
	{0x01cc, 0x00d0},
	{0x0188, 0x00dc},
	{0x01e8, 0x00f0},
};

static int
bjxa_decode_inflated(bjxa_decoder_t *dec, int16_t *dst, uint8_t profile,
    unsigned chan)
{
	bjxa_channel_t *state;
	int32_t spread;
	int16_t sample, sf0, sf1;
	uint8_t shr, idx;
	unsigned len, inc;

	assert(chan == 0 || chan == 1);

	len = BJXA_BLOCK_SAMPLES;
	inc = dec->channels;
	idx = profile >> 4;
	shr = profile & 0x0f;

	BJXA_PROTO_CHECK(idx < 6);

	state = &dec->channel_state[chan];
	sf0 = spread_factor[idx][0];
	sf1 = spread_factor[idx][1];

	while (len > 0) {
		sample = *dst >> shr;
		spread = (state->prev[0] * sf0) - (state->prev[1] * sf1);

		if (spread < 0)
			spread += 0xff;

		*dst = (int16_t)(sample + spread / 256);
		state->prev[1] = state->prev[0];
		state->prev[0] = *dst;

		dst += inc;
		len--;
	}

	return (0);
}

int
bjxa_decode_format(bjxa_decoder_t *dec, bjxa_format_t *fmt)
{

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(fmt);
	if (dec->block_size == 0) {
		errno = EINVAL;
		return (-1);
	}

	fmt->data_len_pcm = dec->samples * dec->channels * sizeof(int16_t);
	fmt->samples_rate = dec->samples_rate;
	fmt->sample_bits = 16;
	fmt->channels = dec->channels;
	fmt->block_size_xa = dec->block_size * dec->channels;
	fmt->block_size_pcm = BJXA_BLOCK_SAMPLES * dec->channels *
	    sizeof(int16_t);
	fmt->blocks = dec->data_len / fmt->block_size_xa;

	assert(fmt->blocks * fmt->block_size_xa == dec->data_len);

	return (0);
}

int
bjxa_decode(bjxa_decoder_t *dec, void *dst, size_t dst_len, const void *src,
    size_t src_len)
{
	bjxa_format_t fmt;
	const uint8_t *src_ptr;
	int16_t *dst_ptr;
	uint8_t profile;
	int blocks = 0;

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(dst);
	CHECK_PTR(src);
	BJXA_TRY(bjxa_decode_format(dec, &fmt));

	BJXA_BUFFER_CHECK(dst_len >= fmt.block_size_pcm);
	BJXA_BUFFER_CHECK(src_len >= fmt.block_size_xa);

	dst_ptr = dst;
	src_ptr = src;

	while (dst_len >= fmt.block_size_pcm &&
	    src_len >= fmt.block_size_xa) {

		profile = dec->inflate_cb(dec, dst_ptr, src_ptr);
		BJXA_TRY(bjxa_decode_inflated(dec, dst_ptr, profile, 0));

		src_ptr += dec->block_size;
		src_len -= dec->block_size;

		if (dec->channels == 2) {
			profile = dec->inflate_cb(dec, dst_ptr + 1, src_ptr);
			BJXA_TRY(bjxa_decode_inflated(dec, dst_ptr + 1,
			    profile, 1));
			src_ptr += dec->block_size;
			src_len -= dec->block_size;
		}

		dst_ptr += BJXA_BLOCK_SAMPLES * dec->channels;
		dst_len -= fmt.block_size_pcm;
		blocks++;
	}

	return (blocks);
}

/* WAVE file format */

#define RIFF_HEADER_LEN	44
#define WAVE_HEADER_LEN	16
#define WAVE_FORMAT_PCM	1

ssize_t
bjxa_dump_riff_header(bjxa_decoder_t *dec, void *dst, size_t len)
{
	bjxa_format_t fmt;
	uint8_t *hdr;

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(dst);
	BJXA_BUFFER_CHECK(len >= RIFF_HEADER_LEN);
	BJXA_TRY(bjxa_decode_format(dec, &fmt));

	hdr = dst;
	mputs(&hdr, "RIFF");
	mwrite_le(&hdr, RIFF_HEADER_LEN + fmt.data_len_pcm, 32);
	mputs(&hdr, "WAVEfmt ");
	mwrite_le(&hdr, WAVE_HEADER_LEN, 32);
	mwrite_le(&hdr, WAVE_FORMAT_PCM, 16);
	mwrite_le(&hdr, fmt.channels, 16);
	mwrite_le(&hdr, fmt.samples_rate, 32);
	mwrite_le(&hdr, fmt.samples_rate * fmt.sample_bits / 8, 32);
	mwrite_le(&hdr, fmt.channels * fmt.sample_bits / 8, 16);
	mwrite_le(&hdr, fmt.sample_bits, 16);
	mputs(&hdr, "data");
	mwrite_le(&hdr, fmt.data_len_pcm, 32);

	assert((uintptr_t)hdr - (uintptr_t)dst == RIFF_HEADER_LEN);

	return (RIFF_HEADER_LEN);
}

ssize_t
bjxa_fwrite_riff_header(bjxa_decoder_t *dec, FILE *file)
{
	uint8_t buf[RIFF_HEADER_LEN];

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(file);
	if (bjxa_dump_riff_header(dec, buf, sizeof buf) < 0) {
		assert(errno != ENOBUFS);
		return (-1);
	}

	if (fwrite(buf, sizeof buf, 1, file) != 1)
		return (-1);

	return (RIFF_HEADER_LEN);
}
