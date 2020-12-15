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
	    "Available actions:\n"
	    "\n"
	    "  help\n"
	    "    Show this message and exit.\n"
	    "\n"
	    "  decode [<xa file> [<wav file>]]\n"
	    "    Read an XA file and convert it into a WAV file.\n"
	    "\n"
	    "  encode [--bits <4|6|8>] [wav file> [<xa file>]]\n"
	    "    Read a WAV file and convert it into an XA file.\n"
	    "    The default number of bits per sample, when left\n"
	    "    unspecified is 6.\n"
	    "\n",
	    progname);
}

static noreturn int
cmd_fail(const char *msg)
{

	fprintf(stderr, "bjxa: %s\n", msg);
	usage(stderr);
	exit(EXIT_FAILURE);
}

static int
open_files(int argc, char * const *argv)
{

	if (argc == 0)
		return (0);

	if (strcmp("-", *argv) && freopen(*argv, "r", stdin) == NULL) {
		perror("Error");
		return (-1);
	}

	argc--;
	argv++;

	if (argc == 0)
		return (0);

	if (strcmp("-", *argv) && freopen(*argv, "w", stdout) == NULL) {
		perror("Error");
		return (-1);
	}

	return (0);
}

int
main(int argc, char * const *argv)
{

	int bits = -1;

	progname = *argv;
	argc--;
	argv++;

	if (argc == 0)
		cmd_fail("Missing an action");

	if (!strcmp("help", *argv)) {
		usage(stdout);
		return (EXIT_SUCCESS);
	}
	else if (!strcmp("decode", *argv)) {
		argc--;
		argv++;
		if (argc > 2)
			cmd_fail("Too many arguments");
		if (open_files(argc, argv) < 0 || decode(stdin, stdout) < 0)
			return (EXIT_FAILURE);
	}
	else if (!strcmp("encode", *argv)) {
		argc--;
		argv++;
		if (argc > 0 && !strcmp("--bits", *argv)) {
			argc--;
			argv++;
			if (argc == 0)
				cmd_fail("Missing number of bits per sample");
			if (strlen(*argv) == 1)
				bits = **argv - '0';
			if (bits != 4 && bits != 6 && bits != 8)
				cmd_fail("Invalid number of bits per sample");
			argc--;
			argv++;
		}
		else {
			bits = 6;
		}
		assert(bits == 4 || bits == 6 || bits == 8);
		if (argc > 2)
			cmd_fail("Too many arguments");
		if (open_files(argc, argv) < 0)
			return (EXIT_FAILURE);
		fprintf(stderr, "bjxa: Encoding not implemented\n");
		return (EXIT_FAILURE);
	}
	else {
		cmd_fail("Unknown action");
	}

	return (EXIT_SUCCESS);
}
