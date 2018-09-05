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

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bjxa.h"

#define ALLOC_OBJ(o, m) \
	do { \
		errno = 0; \
		(o) = calloc(1, sizeof *(o)); \
		if ((o) != NULL) \
			(o)->magic = (m); \
	} while (0)

#define FREE_OBJ(o) \
	do { \
		(void)memset((o), 0, sizeof(*(o))); \
		free(o); \
		(o) = NULL; \
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

struct bjxa_decoder {
	uint32_t		magic;
#define BJXA_DECODER_MAGIC	0x234ec0c2
};

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
