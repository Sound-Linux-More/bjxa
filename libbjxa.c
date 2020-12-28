/*-
 * Copyright (C) 2018-2020  Dridi Boukelmoune
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

#include "config.h"

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
			errno = EFAULT; \
			return (-1); \
		} \
	} while (0)

#define CHECK_OBJ(o, m) \
	do { \
		CHECK_PTR(o); \
		if ((o)->magic != (m)) { \
			errno = EINVAL; \
			return (-1); \
		} \
	} while (0)

#define TAKE_OBJ(o, p, m) \
	do { \
		CHECK_PTR(p); \
		CHECK_OBJ(*(p), (m)); \
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

static uint32_t
mread_le(const uint8_t **buf, unsigned bits)
{
	unsigned n = 0;
	uint32_t res = 0;

	assert(buf != NULL);
	assert(*buf != NULL);
	assert(bits > 0);
	assert(bits % 8 == 0);

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

	assert(buf != NULL);
	assert(*buf != NULL);
	assert(bits > 0);
	assert(bits % 8 == 0);

	while (bits > 0) {
		**buf = (uint8_t)val;
		val >>= 8;
		*buf += 1;
		bits -= 8;
	}
}

/* magic strings */

static int
mgets(const uint8_t **buf, const char *str)
{
	unsigned len;

	assert(buf != NULL);
	assert(*buf != NULL);
	assert(str != NULL);

	len = strlen(str);
	assert(len > 0);
	if (memcmp(*buf, str, len)) {
		errno = EPROTO;
		return (-1);
	}

	*buf += len;
	return (0);
}

static void
mputs(uint8_t **buf, const char *str)
{
	size_t len;

	assert(str != NULL);
	assert(buf != NULL);
	assert(*buf != NULL);

	len = strlen(str);
	assert(len > 0);
	(void)memcpy(*buf, str, len);
	*buf += len;
}

/* data structures */

#define BJXA_BLOCK_SAMPLES	32
#define BJXA_BLOCK_STEREO	64

typedef uint8_t	bjxa_inflate_f(bjxa_decoder_t *, int16_t *, const uint8_t *);
typedef void	bjxa_deflate_f(uint8_t *, const int16_t *);

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
	bjxa_format_t		fmt[1];
};

struct bjxa_encoder {
	uint32_t		magic;
#define BJXA_ENCODER_MAGIC	0xac12f1dd
	uint32_t		data_len;
	uint32_t		samples;
	uint16_t		samples_rate;
	uint8_t			block_size;
	uint8_t			bits;
	uint8_t			channels;
	bjxa_channel_t		channel_state[2];
	bjxa_deflate_f		*deflate_cb;
	bjxa_format_t		fmt[1];
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

bjxa_encoder_t *
bjxa_encoder(void)
{
	bjxa_encoder_t *enc;

	ALLOC_OBJ(enc, BJXA_ENCODER_MAGIC);
	return (enc);
}

int
bjxa_free_encoder(bjxa_encoder_t **encp)
{
	bjxa_encoder_t *enc;

	TAKE_OBJ(enc, encp, BJXA_ENCODER_MAGIC);
	FREE_OBJ(enc);
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
		*dst = (int16_t)(*src << 8);
		dst += chan;
		src++;
	}

	return (profile);
}

/* deflate XA blocks */

static void
bjxa_deflate_4bits(uint8_t *dst, const int16_t *src)
{
	unsigned n;

	for (n = BJXA_BLOCK_SAMPLES; n > 0; n -= 2) {
		*dst = ((uint16_t)*src >> 8) & 0xf0; src++;
		*dst |= (uint16_t)*src >> 12;        src++;
		dst++;
	}
}

static void
bjxa_deflate_6bits(uint8_t *dst, const int16_t *src)
{
	unsigned n;
	uint32_t samples;

	for (n = BJXA_BLOCK_SAMPLES; n > 0; n -= 4) {
		samples  = (uint32_t)((uint16_t)*src >> 10) << 18; src++;
		samples |= (uint32_t)((uint16_t)*src >> 10) << 12; src++;
		samples |= (uint32_t)((uint16_t)*src >> 10) << 6;  src++;
		samples |= (uint32_t)((uint16_t)*src >> 10);       src++;

		assert(samples >> 24 == 0);
		dst[0] = samples >> 16;
		dst[1] = (samples >> 8) & 0xff;
		dst[2] = samples & 0xff;
		dst += 3;
	}
}

static void
bjxa_deflate_8bits(uint8_t *dst, const int16_t *src)
{
	unsigned n;

	for (n = BJXA_BLOCK_SAMPLES; n > 0; n--) {
		*dst = ((uint16_t)*src) >> 8;
		dst++;
		src++;
	}
}

/* XA header */

ssize_t
bjxa_parse_header(bjxa_decoder_t *dec, const void *src, size_t len)
{
	bjxa_decoder_t tmp;
	uint32_t pad, blocks, max_samples, loop;
	const uint8_t *buf;
	uint8_t bits;

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(src);
	BJXA_BUFFER_CHECK(len >= BJXA_HEADER_SIZE_XA);

	buf = src;

	INIT_OBJ(&tmp, BJXA_DECODER_MAGIC);
	BJXA_TRY(mgets(&buf, "KWD1"));
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

	assert((uintptr_t)buf - (uintptr_t)src == BJXA_HEADER_SIZE_XA);

	BJXA_PROTO_CHECK(tmp.data_len > 0);
	BJXA_PROTO_CHECK(tmp.samples > 0);
	BJXA_PROTO_CHECK(tmp.samples_rate > 0);
	BJXA_PROTO_CHECK(bits == 4 || bits == 6 || bits == 8);
	BJXA_PROTO_CHECK(tmp.channels == 1 || tmp.channels == 2);

	tmp.block_size = bits * 4 + 1;
	blocks = tmp.data_len / tmp.block_size;
	max_samples = (BJXA_BLOCK_SAMPLES * tmp.data_len) /
	    (tmp.block_size * tmp.channels);
	BJXA_PROTO_CHECK(blocks * tmp.block_size == tmp.data_len);
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

	BJXA_PROTO_CHECK(bjxa_decode_format(&tmp, tmp.fmt) == 0);

	(void)memcpy(dec, &tmp, sizeof tmp);
	return (BJXA_HEADER_SIZE_XA);
}

ssize_t
bjxa_fread_header(bjxa_decoder_t *dec, FILE *file)
{
	uint8_t buf[BJXA_HEADER_SIZE_XA];
	ssize_t ret;

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(file);

	if (fread(buf, sizeof buf, 1, file) != 1) {
		if (feof(file))
			errno = EIO;
		return (-1);
	}

	ret = bjxa_parse_header(dec, buf, sizeof buf);
	if (ret < 0) {
		assert(errno != EINVAL);
		assert(errno != ENOBUFS);
	}
	return (ret);
}

ssize_t
bjxa_dump_header(bjxa_encoder_t *enc, void *dst, size_t len)
{
	uint8_t *hdr;

	CHECK_OBJ(enc, BJXA_ENCODER_MAGIC);
	CHECK_PTR(dst);
	BJXA_BUFFER_CHECK(len >= BJXA_HEADER_SIZE_XA);
	BJXA_COND_CHECK(enc->data_len > 0, EINVAL);

	hdr = dst;
	mputs(&hdr, "KWD1");
	mwrite_le(&hdr, enc->data_len, 32);
	mwrite_le(&hdr, enc->samples, 32);
	mwrite_le(&hdr, enc->samples_rate, 16);
	mwrite_le(&hdr, enc->bits, 8);
	mwrite_le(&hdr, enc->channels, 8);
	mwrite_le(&hdr, 0, 32); /* loop */
	mwrite_le(&hdr, 0, 16); /* prev */
	mwrite_le(&hdr, 0, 16); /* prev */
	mwrite_le(&hdr, 0, 16); /* prev */
	mwrite_le(&hdr, 0, 16); /* prev */
	mwrite_le(&hdr, 0, 32); /* pad */

	return (BJXA_HEADER_SIZE_XA);
}

ssize_t
bjxa_fwrite_header(bjxa_encoder_t *enc, FILE *file)
{
	uint8_t buf[BJXA_HEADER_SIZE_XA];

	CHECK_OBJ(enc, BJXA_ENCODER_MAGIC);
	CHECK_PTR(file);
	if (bjxa_dump_header(enc, buf, sizeof buf) < 0) {
		assert(errno != ENOBUFS);
		return (-1);
	}

	if (fwrite(buf, sizeof buf, 1, file) != 1)
		return (-1);

	return (BJXA_HEADER_SIZE_XA);
}

/* decode XA blocks */

static const int16_t gain_factor[][2] = {
	{  0,    0},
	{240,    0},
	{460, -208},
	{392, -220},
	{488, -240},
};

static int
bjxa_decode_inflated(bjxa_decoder_t *dec, int16_t *dst, uint8_t profile,
    unsigned chan)
{
	bjxa_channel_t *state;
	int32_t gain, sample;
	int16_t ranged, k0, k1;
	uint8_t range, factor;
	unsigned samples, step;

	assert(chan == 0 || chan == 1);

	samples = BJXA_BLOCK_SAMPLES;
	step = dec->channels;
	factor = profile >> 4;
	range = profile & 0x0f;

	BJXA_PROTO_CHECK(factor < 5);

	state = &dec->channel_state[chan];
	k0 = gain_factor[factor][0];
	k1 = gain_factor[factor][1];

	while (samples > 0) {
		/* compute sample */
		ranged = *dst >> range;
		gain = (state->prev[0] * k0) + (state->prev[1] * k1);
		sample = ranged + gain / 256;

		/* clamp sample */
		if (sample < INT16_MIN)
			sample = INT16_MIN;
		if (sample > INT16_MAX)
			sample = INT16_MAX;

		/* propagate sample */
		*dst = (int16_t)sample;
		state->prev[1] = state->prev[0];
		state->prev[0] = *dst;

		dst += step;
		samples--;
	}

	return (0);
}

int
bjxa_decode_format(bjxa_decoder_t *dec, bjxa_format_t *fmt)
{

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(fmt);
	BJXA_COND_CHECK(dec->block_size != 0, EINVAL);

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
	bjxa_format_t *fmt;
	const uint8_t *src_ptr;
	int16_t *dst_ptr, dst_buf[BJXA_BLOCK_STEREO];
	uint8_t profile, pcm_block;
	int blocks = 0;

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(dst);
	CHECK_PTR(src);
	fmt = dec->fmt;
	BJXA_COND_CHECK(fmt->sample_bits == 16, EINVAL);
	BJXA_PROTO_CHECK(fmt->blocks > 0);

	BJXA_BUFFER_CHECK(dst_len >= fmt->block_size_pcm);
	BJXA_BUFFER_CHECK(src_len >= fmt->block_size_xa);

	pcm_block = fmt->block_size_pcm;
	if (pcm_block > fmt->data_len_pcm)
		pcm_block = (uint8_t)fmt->data_len_pcm;

	dst_ptr = dst;
	src_ptr = src;

	while (fmt->blocks > 0 && dst_len >= pcm_block &&
	    src_len >= fmt->block_size_xa) {

		assert(pcm_block > 0);
		profile = dec->inflate_cb(dec, dst_buf, src_ptr);
		BJXA_TRY(bjxa_decode_inflated(dec, dst_buf, profile, 0));

		src_ptr += dec->block_size;
		src_len -= dec->block_size;

		if (dec->channels == 2) {
			profile = dec->inflate_cb(dec, dst_buf + 1, src_ptr);
			BJXA_TRY(bjxa_decode_inflated(dec, dst_buf + 1,
			    profile, 1));
			src_ptr += dec->block_size;
			src_len -= dec->block_size;
		}

		memcpy(dst_ptr, dst_buf, pcm_block);

		dst_ptr += pcm_block / sizeof *dst_ptr;
		dst_len -= pcm_block;
		blocks++;

		fmt->data_len_pcm -= pcm_block;
		fmt->blocks--;
		if (pcm_block > fmt->data_len_pcm)
			pcm_block = (uint8_t)fmt->data_len_pcm;
	}

	return (blocks);
}

/* encode XA blocks */

static void
bjxa_encode_inflated(bjxa_encoder_t *enc, int16_t *dst, const int16_t *src,
    uint8_t *profile, unsigned chan, unsigned pcm_block)
{
	unsigned n, samples, step;

	assert(chan == 0 || chan == 1);
	assert(pcm_block > 0);

	step = enc->channels;
	samples = pcm_block / (step * sizeof *src);
	assert(pcm_block % samples == 0);
	assert(samples <= BJXA_BLOCK_SAMPLES);

	*profile = 0;
	for (n = 0; n < samples; n++) {
		*dst = *src;
		src += step;
		dst++;
	}

	while (samples < BJXA_BLOCK_SAMPLES) {
		*dst = 0;
		dst++;
		samples++;
	}
}

int
bjxa_encode_init(bjxa_encoder_t *enc, bjxa_format_t *fmt, uint8_t bits)
{
	bjxa_encoder_t tmp;

	CHECK_OBJ(enc, BJXA_ENCODER_MAGIC);
	CHECK_PTR(fmt);
	BJXA_COND_CHECK(fmt->sample_bits == 16, EINVAL);
	BJXA_COND_CHECK(bits == 4 || bits == 6 || bits == 8, EINVAL);

	INIT_OBJ(&tmp, BJXA_ENCODER_MAGIC);
	tmp.bits = bits;
	tmp.channels = fmt->channels;
	BJXA_PROTO_CHECK(tmp.channels == 1 || tmp.channels == 2);

	tmp.samples = fmt->data_len_pcm / (tmp.channels * sizeof(int16_t));
	tmp.samples_rate = fmt->samples_rate;
	BJXA_PROTO_CHECK(tmp.samples > 0);
	BJXA_PROTO_CHECK(tmp.samples_rate > 0);
	BJXA_PROTO_CHECK(fmt->data_len_pcm % tmp.samples == 0);

	if (bits == 4)
		tmp.deflate_cb = bjxa_deflate_4bits;
	else if (bits == 6)
		tmp.deflate_cb = bjxa_deflate_6bits;
	else
		tmp.deflate_cb = bjxa_deflate_8bits;

	tmp.block_size = bits * 4 + 1;
	fmt->block_size_xa = tmp.block_size * tmp.channels;
	fmt->block_size_pcm = BJXA_BLOCK_SAMPLES * tmp.channels *
	    sizeof(int16_t);
	fmt->blocks = tmp.samples / BJXA_BLOCK_SAMPLES;
	tmp.data_len = fmt->blocks * tmp.block_size * tmp.channels;
	if (tmp.samples % BJXA_BLOCK_SAMPLES != 0) {
		tmp.data_len += tmp.block_size * tmp.channels;
		fmt->blocks++;
	}

	memcpy(tmp.fmt, fmt, sizeof *fmt);
	memcpy(enc, &tmp, sizeof tmp);
	return (0);
}

int
bjxa_encode_format(bjxa_encoder_t *enc, bjxa_format_t *fmt)
{

	CHECK_OBJ(enc, BJXA_ENCODER_MAGIC);
	CHECK_PTR(fmt);
	BJXA_COND_CHECK(enc->block_size != 0, EINVAL);

	fmt->data_len_pcm = enc->samples * enc->channels * sizeof(int16_t);
	fmt->samples_rate = enc->samples_rate;
	fmt->sample_bits = enc->bits;
	fmt->channels = enc->channels;
	fmt->block_size_xa = enc->block_size * enc->channels;
	fmt->block_size_pcm = BJXA_BLOCK_SAMPLES * enc->channels *
	    sizeof(int16_t);
	fmt->blocks = enc->data_len / fmt->block_size_xa;

	assert(fmt->blocks * fmt->block_size_xa == enc->data_len);

	return (0);
}

int
bjxa_encode(bjxa_encoder_t *enc, void *dst, size_t dst_len, const void *src,
    size_t src_len)
{
	bjxa_format_t *fmt;
	const int16_t *src_ptr;
	uint8_t *dst_ptr;
	int16_t enc_buf[BJXA_BLOCK_SAMPLES];
	uint8_t profile, pcm_block;
	int blocks = 0;

	CHECK_OBJ(enc, BJXA_ENCODER_MAGIC);
	CHECK_PTR(dst);
	CHECK_PTR(src);
	fmt = enc->fmt;
	BJXA_COND_CHECK(fmt->sample_bits == 16, EINVAL);
	BJXA_PROTO_CHECK(fmt->blocks > 0);

	BJXA_BUFFER_CHECK(dst_len >= fmt->block_size_xa);
	BJXA_BUFFER_CHECK(src_len >= fmt->block_size_pcm);

	pcm_block = fmt->block_size_pcm;
	if (pcm_block > fmt->data_len_pcm)
		pcm_block = (uint8_t)fmt->data_len_pcm;

	dst_ptr = dst;
	src_ptr = src;

	while (fmt->blocks > 0 && dst_len >= fmt->block_size_xa &&
	    src_len >= pcm_block) {

		assert(pcm_block > 0);
		bjxa_encode_inflated(enc, enc_buf, src_ptr, &profile,
		    0, pcm_block);
		*dst_ptr = profile;
		enc->deflate_cb(dst_ptr + 1, enc_buf);

		dst_ptr += enc->block_size;
		dst_len -= enc->block_size;

		if (enc->channels == 2) {
			bjxa_encode_inflated(enc, enc_buf, src_ptr + 1,
			    &profile, 1, pcm_block);
			*dst_ptr = profile;
			enc->deflate_cb(dst_ptr + 1, enc_buf);
			dst_ptr += enc->block_size;
			dst_len -= enc->block_size;
		}

		src_ptr += pcm_block / sizeof *src_ptr;
		src_len -= pcm_block;
		blocks++;

		fmt->data_len_pcm -= pcm_block;
		fmt->blocks--;
		if (pcm_block > fmt->data_len_pcm)
			pcm_block = (uint8_t)fmt->data_len_pcm;
	}

	return (blocks);
}

/* WAVE file format */

#define WAVE_HEADER_LEN	16
#define WAVE_FORMAT_PCM	1

ssize_t
bjxa_parse_riff_header(bjxa_format_t *fmt, const void *src, size_t len)
{
	bjxa_format_t tmp;
	uint32_t riff_data, wave_hdr, wave_rate, wave_bytes, wave_data;
	uint16_t wave_fmt, wave_chan, wave_block, wave_bits;
	const uint8_t *buf;

	CHECK_PTR(fmt);
	CHECK_PTR(src);
	BJXA_BUFFER_CHECK(len >= BJXA_HEADER_SIZE_RIFF);

	buf = src;

	BJXA_TRY(mgets(&buf, "RIFF"));
	riff_data = mread_le32(&buf);
	BJXA_TRY(mgets(&buf, "WAVEfmt "));
	wave_hdr = mread_le32(&buf);
	wave_fmt = mread_le16(&buf);
	wave_chan = mread_le16(&buf);
	wave_rate = mread_le32(&buf);
	wave_bytes = mread_le32(&buf);
	wave_block = mread_le16(&buf);
	wave_bits = mread_le16(&buf);
	BJXA_TRY(mgets(&buf, "data"));
	wave_data = mread_le32(&buf);

	assert((uintptr_t)buf - (uintptr_t)src == BJXA_HEADER_SIZE_RIFF);

	BJXA_PROTO_CHECK(riff_data >= BJXA_HEADER_SIZE_RIFF - 8 + wave_data);
	BJXA_PROTO_CHECK(wave_hdr == WAVE_HEADER_LEN);
	BJXA_PROTO_CHECK(wave_fmt == WAVE_FORMAT_PCM);
	BJXA_PROTO_CHECK(wave_chan == 1 || wave_chan == 2);
	BJXA_PROTO_CHECK(wave_rate > 0 && wave_rate < UINT16_MAX);
	BJXA_PROTO_CHECK(wave_block == wave_chan * sizeof(uint16_t));
	BJXA_PROTO_CHECK(wave_bytes == wave_rate * wave_block);
	BJXA_PROTO_CHECK(wave_data % wave_block == 0);
	BJXA_PROTO_CHECK(wave_bits == 16);

	memset(&tmp, 0, sizeof tmp);
	tmp.data_len_pcm = wave_data;
	tmp.samples_rate = wave_rate;
	tmp.sample_bits = 16;
	tmp.channels = wave_chan;
	(void)memcpy(fmt, &tmp, sizeof tmp);

	return (BJXA_HEADER_SIZE_RIFF);
}

ssize_t
bjxa_fread_riff_header(bjxa_format_t *fmt, FILE *file)
{
	uint8_t buf[BJXA_HEADER_SIZE_RIFF];
	ssize_t ret;

	CHECK_PTR(fmt);
	CHECK_PTR(file);

	if (fread(buf, sizeof buf, 1, file) != 1) {
		if (feof(file))
			errno = EIO;
		return (-1);
	}

	ret = bjxa_parse_riff_header(fmt, buf, sizeof buf);
	if (ret < 0) {
		assert(errno != EINVAL);
		assert(errno != ENOBUFS);
	}
	return (ret);
}

ssize_t
bjxa_dump_riff_header(bjxa_decoder_t *dec, void *dst, size_t len)
{
	bjxa_format_t fmt;
	uint8_t *hdr;

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(dst);
	BJXA_BUFFER_CHECK(len >= BJXA_HEADER_SIZE_RIFF);
	BJXA_TRY(bjxa_decode_format(dec, &fmt));

	hdr = dst;
	mputs(&hdr, "RIFF");
	mwrite_le(&hdr, BJXA_HEADER_SIZE_RIFF - 8 + fmt.data_len_pcm, 32);
	mputs(&hdr, "WAVEfmt ");
	mwrite_le(&hdr, WAVE_HEADER_LEN, 32);
	mwrite_le(&hdr, WAVE_FORMAT_PCM, 16);
	mwrite_le(&hdr, fmt.channels, 16);
	mwrite_le(&hdr, fmt.samples_rate, 32);
	mwrite_le(&hdr, fmt.samples_rate * fmt.block_size_pcm /
	    BJXA_BLOCK_SAMPLES, 32);
	mwrite_le(&hdr, fmt.channels * fmt.sample_bits / 8, 16);
	mwrite_le(&hdr, fmt.sample_bits, 16);
	mputs(&hdr, "data");
	mwrite_le(&hdr, fmt.data_len_pcm, 32);

	assert((uintptr_t)hdr - (uintptr_t)dst == BJXA_HEADER_SIZE_RIFF);

	return (BJXA_HEADER_SIZE_RIFF);
}

ssize_t
bjxa_fwrite_riff_header(bjxa_decoder_t *dec, FILE *file)
{
	uint8_t buf[BJXA_HEADER_SIZE_RIFF];

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(file);
	if (bjxa_dump_riff_header(dec, buf, sizeof buf) < 0) {
		assert(errno != ENOBUFS);
		return (-1);
	}

	if (fwrite(buf, sizeof buf, 1, file) != 1)
		return (-1);

	return (BJXA_HEADER_SIZE_RIFF);
}

int
bjxa_dump_pcm(void *dst, const int16_t *src, size_t len)
{
	uint8_t *out;

	CHECK_PTR(dst);
	CHECK_PTR(src);
	BJXA_BUFFER_CHECK(len > 0);
	BJXA_BUFFER_CHECK((len & 1) == 0);

	out = dst;

	while (len > 0) {
		mwrite_le(&out, (uint32_t)*src, 16);
		src++;
		len -= 2;
	}

	return (0);
}

int
bjxa_fwrite_pcm(const int16_t *src, size_t len, FILE *file)
{
	int16_t buf[BJXA_BLOCK_SAMPLES];
	size_t buf_len = sizeof buf;
	int ret;

#ifdef NDEBUG
	(void)ret;
#endif

	CHECK_PTR(src);
	CHECK_PTR(file);
	BJXA_BUFFER_CHECK(len > 0);
	BJXA_BUFFER_CHECK((len & 1) == 0);

	while (len > 0) {
		if (buf_len > len)
			buf_len = len;
		ret = bjxa_dump_pcm(buf, src, buf_len);
		assert(ret == 0);
		if (fwrite(buf, buf_len, 1, file) != 1)
			return (-1);
		src += buf_len / sizeof *src;
		len -= buf_len;
	}

	return (0);
}
