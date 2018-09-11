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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bjxa.h"

static int
decode(bjxa_decoder_t *dec)
{
	bjxa_format_t fmt;
	void *buf_pcm, *buf_xa;
	uint32_t pcm_block;
	int ret = 0;

	if (bjxa_fread_header(dec, stdin) < 0) {
		perror("bjxa_fread_header");
		return (-1);
	}

	if (bjxa_decode_format(dec, &fmt) < 0) {
		perror("bjxa_decode_format");
		return (-1);
	}

	/* allocate space for exactly one block */
	buf_pcm = malloc(fmt.block_size_pcm);
	buf_xa = malloc(fmt.block_size_xa);

	if (buf_pcm == NULL || buf_xa == NULL) {
		perror("malloc");
		ret = -1;
	}

	if (bjxa_fwrite_riff_header(dec, stdout) < 0) {
		perror("bjxa_fwrite_riff_header");
		ret = -1;
	}

	while (fmt.blocks > 0 && ret == 0) {
		if (fread(buf_xa, fmt.block_size_xa, 1, stdin) != 1) {
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
		if (pcm_block > fmt.data_len_pcm)
			pcm_block = fmt.data_len_pcm;

		if (fwrite(buf_pcm, pcm_block, 1, stdout) != 1) {
			perror("fwrite");
			ret = -1;
			break;
		}

		fmt.data_len_pcm -= pcm_block;
		fmt.blocks--;
	}

	assert(fmt.data_len_pcm == 0);

	free(buf_pcm);
	free(buf_xa);
	return (ret);
}

int
main(void)
{
	bjxa_decoder_t *dec;
	int status = EXIT_SUCCESS;

	dec = bjxa_decoder();
	if (dec == NULL) {
		perror("bjxa_decoder");
		return (EXIT_FAILURE);
	}

	if (decode(dec) < 0)
		status = EXIT_FAILURE;

	if (bjxa_free_decoder(&dec) < 0) {
		perror("bjxa_free_decoder");
		status = EXIT_FAILURE;
	}

	return (status);
}
