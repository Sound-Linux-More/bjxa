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
decode_header(bjxa_decoder_t *dec, FILE *in, FILE *out, bjxa_format_t *fmt)
{

	if (bjxa_fread_header(dec, in) < 0) {
		perror("bjxa_fread_header");
		return (-1);
	}

	if (bjxa_decode_format(dec, fmt) < 0) {
		perror("bjxa_decode_format");
		return (-1);
	}

	if (bjxa_fwrite_riff_header(dec, out) < 0) {
		perror("bjxa_fwrite_riff_header");
		return (-1);
	}

	return (0);
}

#ifdef BJXA_SINGLE_PASS
static int
decode_loop(bjxa_decoder_t *dec, FILE *in, FILE *out)
{
	bjxa_format_t fmt;
	void *buf_pcm, *buf_xa;
	uint32_t xa_len;
	int ret = 0;

	if (decode_header(dec, in, out, &fmt) < 0)
		return (-1);

	/* allocate space for the whole stream */
	xa_len = fmt.block_size_xa * fmt.blocks;
	buf_pcm = malloc(fmt.data_len_pcm);
	buf_xa = malloc(xa_len);

	if (buf_pcm == NULL || buf_xa == NULL) {
		perror("malloc");
		ret = -1;
	}

	if (ret == 0 && fread(buf_xa, xa_len, 1, in) != 1) {
		if (feof(in))
			fprintf(stderr, "fread: End of file\n");
		else
			perror("fread");
		ret = -1;
	}

	if (ret == 0 && bjxa_decode(dec, buf_pcm, fmt.data_len_pcm, buf_xa,
	    xa_len) != (int)fmt.blocks) {
		perror("bjxa_decode");
		ret = -1;
	}

	if (ret == 0 && bjxa_fwrite_pcm(buf_pcm, fmt.data_len_pcm, out) < 0) {
		perror("bjxa_fwrite_pcm");
		ret = -1;
	}

	free(buf_pcm);
	free(buf_xa);
	return (ret);
}
#else /* BJXA_SINGLE_PASS */
static int
decode_loop(bjxa_decoder_t *dec, FILE *in, FILE *out)
{
	bjxa_format_t fmt;
	void *buf_pcm, *buf_xa;
	uint32_t pcm_block;
	int ret = 0;

	if (decode_header(dec, in, out, &fmt) < 0)
		return (-1);

	/* allocate space for exactly one block */
	buf_pcm = malloc(fmt.block_size_pcm);
	buf_xa = malloc(fmt.block_size_xa);

	if (buf_pcm == NULL || buf_xa == NULL) {
		perror("malloc");
		ret = -1;
	}

	while (fmt.blocks > 0 && ret == 0) {
		if (fread(buf_xa, fmt.block_size_xa, 1, in) != 1) {
			if (feof(in))
				fprintf(stderr, "fread: End of file\n");
			else
				perror("fread");
			ret = -1;
			break;
		}

		if (bjxa_decode(dec, buf_pcm, fmt.block_size_pcm, buf_xa,
		    fmt.block_size_xa) != 1) {
			perror("bjxa_decode");
			ret = -1;
			break;
		}

		pcm_block = fmt.block_size_pcm;
		assert(fmt.data_len_pcm > 0);
		if (pcm_block > fmt.data_len_pcm)
			pcm_block = fmt.data_len_pcm;

		if (bjxa_fwrite_pcm(buf_pcm, pcm_block, out) < 0) {
			perror("bjxa_fwrite_pcm");
			ret = -1;
			break;
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
decode(FILE *in, FILE *out)
{
	bjxa_decoder_t *dec;
	int status = 0;

	dec = bjxa_decoder();
	if (dec == NULL) {
		perror("bjxa_decoder");
		return (-1);
	}

	if (decode_loop(dec, in, out) < 0)
		status = -1;

	if (bjxa_free_decoder(&dec) < 0) {
		perror("bjxa_free_decoder");
		status = -1;
	}

	return (status);
}
