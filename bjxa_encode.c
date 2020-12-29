/*-
 * Copyright (C) 2020  Dridi Boukelmoune
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

/* begin strip */
#include "config.h"
/* end strip */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bjxa.h"

/* begin strip */
#include "bjxa_priv.h"
/* end strip */

static int
encode_header(bjxa_encoder_t *enc, FILE *in, FILE *out, bjxa_format_t *fmt,
    unsigned bits)
{

	if (bjxa_fread_riff_header(fmt, in) < 0) {
		perror("bjxa_fread_riff_header");
		return (-1);
	}

	if (bjxa_encode_init(enc, fmt, bits) < 0) {
		perror("bjxa_encode_init");
		return (-1);
	}

	if (bjxa_encode_format(enc, fmt) < 0) {
		perror("bjxa_encode_format");
		return (-1);
	}

	if (bjxa_fwrite_header(enc, out) < 0) {
		perror("bjxa_fwrite_header");
		return (-1);
	}

	return (0);
}

#ifdef BJXA_SINGLE_PASS
static int
encode_loop(bjxa_encoder_t *enc, FILE *in, FILE *out, unsigned bits)
{
	bjxa_format_t fmt;
	void *buf_pcm, *buf_xa;
	uint32_t xa_len;
	int ret = 0;

	if (encode_header(enc, in, out, &fmt, bits) < 0)
		return (-1);

	/* allocate space for the whole stream */
	xa_len = fmt.block_size_xa * fmt.blocks;
	buf_pcm = malloc(fmt.data_len_pcm);
	buf_xa = malloc(xa_len);

	if (buf_pcm == NULL || buf_xa == NULL) {
		perror("malloc");
		ret = -1;
	}

	if (ret == 0 && fread(buf_pcm, fmt.data_len_pcm, 1, in) != 1) {
		if (feof(in))
			fprintf(stderr, "fread: End of file\n");
		else
			perror("fread");
		ret = -1;
	}

	if (ret == 0 && bjxa_encode(enc, buf_xa, xa_len, buf_pcm,
	    fmt.data_len_pcm) != (int)fmt.blocks) {
		perror("bjxa_encode");
		ret = -1;
	}

	if (ret == 0 && fwrite(buf_xa, xa_len, 1, out) != 1) {
		perror("fwrite");
		ret = -1;
	}

	free(buf_pcm);
	free(buf_xa);
	return (ret);
}
#else /* BJXA_SINGLE_PASS */
static int
encode_loop(bjxa_encoder_t *enc, FILE *in, FILE *out, unsigned bits)
{
	bjxa_format_t fmt;
	void *buf_pcm, *buf_xa;
	uint32_t pcm_block;
	int ret = 0;

	if (encode_header(enc, in, out, &fmt, bits) < 0)
		return (-1);

	/* allocate space for exactly one block */
	buf_pcm = malloc(fmt.block_size_pcm);
	buf_xa = malloc(fmt.block_size_xa);

	if (buf_pcm == NULL || buf_xa == NULL) {
		perror("malloc");
		ret = -1;
	}

	pcm_block = fmt.block_size_pcm;
	assert(fmt.data_len_pcm > 0);

	while (fmt.blocks > 0 && ret == 0) {
		if (pcm_block > fmt.data_len_pcm)
			pcm_block = fmt.data_len_pcm;

		if (fread(buf_pcm, pcm_block, 1, in) != 1) {
			if (feof(in))
				fprintf(stderr, "fread: End of file\n");
			else
				perror("fread");
			ret = -1;
			break;
		}

		if (bjxa_encode(enc, buf_xa, fmt.block_size_xa, buf_pcm,
		    fmt.block_size_pcm) != 1) {
			perror("bjxa_encode");
			ret = -1;
			break;
		}

		if (ret == 0 &&
		    fwrite(buf_xa, fmt.block_size_xa, 1, out) != 1) {
			perror("fwrite");
			ret = -1;
		}

		fmt.data_len_pcm -= pcm_block;
		fmt.blocks--;
	}

	if (ret == 0)
		assert(fmt.data_len_pcm == 0);

	free(buf_pcm);
	free(buf_xa);
	return (ret);
}
#endif /* BJXA_SINGLE_PASS */

int
encode(FILE *in, FILE *out, unsigned bits)
{
	bjxa_encoder_t *enc;
	int status = 0;

	enc = bjxa_encoder();
	if (enc == NULL) {
		perror("bjxa_encoder");
		return (-1);
	}

	if (encode_loop(enc, in, out, bits) < 0)
		status = -1;

	if (bjxa_free_encoder(&enc) < 0) {
		perror("bjxa_free_encoder");
		status = -1;
	}

	return (status);
}
