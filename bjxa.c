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
#include <string.h>
#include <unistd.h>

#include "bjxa.h"
#include "bjxa_priv.h"

static const char *progname;

static void
usage(FILE* file)
{

	fprintf(file,
	    "Usage: %s <action> [args...]\n"
	    "\n"
	    "Available action:\n"
	    "\n"
	    "  help\n"
	    "    Show this message and exit.\n"
	    "\n"
	    "  decode [<xa file> [<wav file>]]\n"
	    "    Read an XA file and convert it into a WAV file.\n"
	    "\n",
	    progname);
}

int
main(int argc, char * const *argv)
{

	progname = *argv;
	argc--;
	argv++;

	if (argc == 0) {
		fprintf(stderr, "bjxa: Missing an action\n");
		usage(stderr);
		return (EXIT_FAILURE);
	}

	if (!strcmp("help", *argv)) {
		usage(stdout);
		return (EXIT_SUCCESS);
	}
	else if (!strcmp("decode", *argv)) {
		argc--;
		argv++;
		if (decode(stdin, stdout) < 0)
			return (EXIT_FAILURE);
	}
	else {
		fprintf(stderr, "bjxa: Unknown action\n");
		usage(stderr);
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
