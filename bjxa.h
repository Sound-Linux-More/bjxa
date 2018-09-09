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

typedef struct bjxa_decoder bjxa_decoder_t;

typedef struct {
	uint32_t	blocks;
	uint8_t		block_size_pcm;
	uint8_t		block_size_xa;
	uint16_t	samples_rate;
	uint8_t		sample_bits;
	uint8_t		channels;
} bjxa_format_t;

bjxa_decoder_t * bjxa_decoder(void);
int bjxa_free_decoder(bjxa_decoder_t **);

ssize_t bjxa_parse_header(bjxa_decoder_t *, void *, size_t);
ssize_t bjxa_fread_header(bjxa_decoder_t *, FILE *);

int bjxa_decode_format(bjxa_decoder_t *, bjxa_format_t *);
int bjxa_decode(bjxa_decoder_t *, void *, size_t, const void *, size_t);
