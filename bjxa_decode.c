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
decode_loop(bjxa_decoder_t *dec, FILE *in, FILE *out)
{
	bjxa_format_t fmt;
	void *buf_pcm, *buf_xa;
	uint32_t pcm_block;
	int ret = 0;

	if (bjxa_fread_header(dec, in) < 0) {
		perror("bjxa_fread_header");
		return (-1);
	}

	if (bjxa_decode_format(dec, &fmt) < 0) {
		perror("bjxa_decode_format");
		return (-1);
	}

	if (bjxa_fwrite_riff_header(dec, out) < 0) {
		perror("bjxa_fwrite_riff_header");
		return (-1);
	}

	/* allocate space for exactly one block */
	buf_pcm = malloc(fmt.block_size_pcm);
	buf_xa = malloc(fmt.block_size_xa);

	if (buf_pcm == NULL || buf_xa == NULL) {
		perror("malloc");
		ret = -1;
	}

	while (fmt.blocks > 0 && ret == 0) {
		if (fread(buf_xa, fmt.block_size_xa, 1, in) != 1) {
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
