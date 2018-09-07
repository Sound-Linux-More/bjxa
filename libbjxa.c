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

/* error handling */

#define BJXA_TRY(res) \
	do { \
		if ((res) < 0) \
			return (-1); \
	} while (0)

#define BJXA_PROTO_CHECK(cond) \
	do { \
		if (!(cond)) { \
			errno = EPROTO; \
			return (-1); \
		} \
	} while (0)

/* little endian */

static inline int
fread_le(FILE *file, uint32_t *res, unsigned bits)
{
	unsigned n = 0;
	uint8_t u8;

	assert(file != NULL);
	assert(res != NULL);
	errno = 0;
	*res = 0;

	while (n < bits && fread(&u8, 1, 1, file) == 1) {
		*res |= (uint32_t)(u8 << n);
		n += 8;
	}

	if (n < bits) {
		if (feof(file))
			errno = ENODATA;
		return (-1);
	}

	return (0);
}

static int
fread_le8(FILE *file, uint8_t *res)
{
	uint32_t tmp;

	assert(file != NULL);
	assert(res != NULL);

	/* NB: I know that byte order doesn't make sense for a single octet
	 * but this makes the caller side cleaner, consistent, and shares the
	 * error handling with the more legitimate functions.
	 */
	if (fread_le(file, &tmp, 8) < 0)
		return (-1);

	assert(tmp <= UINT8_MAX);
	*res = (uint8_t)tmp;
	return (0);
}

static int
fread_le16(FILE *file, uint16_t *res)
{
	uint32_t tmp;

	assert(file != NULL);
	assert(res != NULL);

	if (fread_le(file, &tmp, 16) < 0)
		return (-1);

	assert(tmp <= UINT16_MAX);
	*res = (uint16_t)tmp;
	return (0);
}

static int
fread_le32(FILE *file, uint32_t *res)
{

	assert(file != NULL);
	assert(res != NULL);

	return (fread_le(file, res, 32));
}

/* data structures */

#define BJXA_HEADER_MAGIC	0x3144574b
#define BJXA_HEADER_SIZE	32

typedef struct {
	uint16_t	prev[2];
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

/* XA header */

ssize_t
bjxa_parse_header(bjxa_decoder_t *dec, void *ptr, size_t len)
{
	FILE *file;
	ssize_t ret;

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(ptr);

	if (len < BJXA_HEADER_SIZE) {
		errno = ENOBUFS;
		return (-1);
	}

	errno = 0;
	file = fmemopen(ptr, len, "r");
	if (file == NULL) {
		assert(errno != EINVAL);
		return (-1);
	}

	/* NB: ISO C99 and POSIX.1 for some obscure and really hard to justify
	 * reasons decided not to specify functions for big and little endian
	 * conversions. This is especially hard since there is also neither
	 * standardized or portable way to know the endianness of a host. But
	 * of course your average libc has functions such as ntohs that
	 * definitely prove that your average libc could ship with include
	 * files advertising macros about the host byte order. In the absence
	 * of such functions or macros, screw efficiency, and treat the memory
	 * buffer as a file where it should really have been the other way
	 * around with a single read followed by byte order adjustments.
	 */
	ret = bjxa_fread_header(dec, file);
	if (ret < 0) {
		assert(errno != EINVAL);
		assert(errno != ENODATA);
	}
	(void)fclose(file);
	return (ret);
}

ssize_t
bjxa_fread_header(bjxa_decoder_t *dec, FILE *file)
{
	bjxa_decoder_t tmp;
	uint32_t magic, pad, data_len;
	uint8_t bits;

	CHECK_OBJ(dec, BJXA_DECODER_MAGIC);
	CHECK_PTR(file);

	INIT_OBJ(&tmp, BJXA_DECODER_MAGIC);
	BJXA_TRY(fread_le32(file, &magic));
	BJXA_TRY(fread_le32(file, &tmp.data_len));
	BJXA_TRY(fread_le32(file, &tmp.samples));
	BJXA_TRY(fread_le16(file, &tmp.samples_rate));
	BJXA_TRY(fread_le8(file, &bits));
	BJXA_TRY(fread_le8(file, &tmp.channels));
	BJXA_TRY(fread_le16(file, &tmp.channel_state[0].prev[0]));
	BJXA_TRY(fread_le16(file, &tmp.channel_state[0].prev[1]));
	BJXA_TRY(fread_le16(file, &tmp.channel_state[1].prev[0]));
	BJXA_TRY(fread_le16(file, &tmp.channel_state[1].prev[1]));
	BJXA_TRY(fread_le32(file, &pad));

	BJXA_PROTO_CHECK(magic == BJXA_HEADER_MAGIC);
	BJXA_PROTO_CHECK(bits == 4 || bits == 6 || bits == 8);
	BJXA_PROTO_CHECK(tmp.channels == 1 || tmp.channels == 2);
	BJXA_PROTO_CHECK((tmp.samples & 0x1f) == 0);

	tmp.block_size = bits * 4 + 1;
	data_len = (tmp.samples >> 5) * tmp.channels * tmp.block_size;

	BJXA_PROTO_CHECK(tmp.data_len == data_len);

	(void)memcpy(dec, &tmp, sizeof tmp);
	return (BJXA_HEADER_SIZE);
}
