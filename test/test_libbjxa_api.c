/*- Copyright (C) 2018  Dridi Boukelmoune
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

#ifdef NDEBUG
#  undef NDEBUG
#endif

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <bjxa.h>

#define ADD_TEST_CASE(name) static void test_##name(void)
#define RUN_TEST_CASE(name) test_##name()

static const char random_junk[] = "random junk";
static const char src_buf[4096];
static char dst_buf[4096];

ADD_TEST_CASE(memory_management)
{
	bjxa_decoder_t *dec;
	void *junk;

	dec = bjxa_decoder();
	assert(dec != NULL);

	junk = strdup(random_junk);
	assert(junk != NULL);

	assert(bjxa_free_decoder(NULL) == -1);
	assert(errno == EFAULT);

	assert(bjxa_free_decoder(&dec) == 0);
	assert(dec == NULL);

	assert(bjxa_free_decoder(&dec) == -1);
	assert(errno == EFAULT);

	dec = junk;
	assert(bjxa_free_decoder(&dec) == -1);
	assert(dec != NULL);
	assert(errno == EINVAL);
	dec = NULL;

	free(junk);
}

ADD_TEST_CASE(header_parsing)
{
	bjxa_decoder_t *dec;
	void *junk;

	dec = bjxa_decoder();
	assert(dec != NULL);

	junk = strdup(random_junk);
	assert(junk != NULL);

	assert(bjxa_parse_header(NULL, junk, 32) == -1);
	assert(errno == EFAULT);

	assert(bjxa_parse_header(junk, junk, 32) == -1);
	assert(errno == EINVAL);

	assert(bjxa_parse_header(dec, NULL, 32) == -1);
	assert(errno == EFAULT);

	assert(bjxa_parse_header(dec, junk, 0) == -1);
	assert(errno == ENOBUFS);

	assert(bjxa_fread_header(NULL, stdin) == -1);
	assert(errno == EFAULT);

	assert(bjxa_fread_header(junk, stdin) == -1);
	assert(errno == EINVAL);

	assert(bjxa_fread_header(dec, NULL) == -1);
	assert(errno == EFAULT);

	assert(bjxa_free_decoder(&dec) == 0);
	assert(dec == NULL);
	free(junk);
}

ADD_TEST_CASE(file_format)
{
	bjxa_decoder_t *dec;
	bjxa_format_t fmt;
	void *junk;

	dec = bjxa_decoder();
	assert(dec != NULL);

	junk = strdup(random_junk);
	assert(junk != NULL);

	assert(bjxa_decode_format(NULL, &fmt) == -1);
	assert(errno == EFAULT);

	assert(bjxa_decode_format(junk, &fmt) == -1);
	assert(errno == EINVAL);

	assert(bjxa_decode_format(dec, &fmt) == -1);
	assert(errno == EINVAL);

	assert(bjxa_decode_format(dec, NULL) == -1);
	assert(errno == EFAULT);

	assert(bjxa_free_decoder(&dec) == 0);
	assert(dec == NULL);
	free(junk);
}

ADD_TEST_CASE(decoding)
{
	bjxa_decoder_t *dec;
	bjxa_format_t fmt;
	FILE *file;
	void *junk;

	dec = bjxa_decoder();
	assert(dec != NULL);

	junk = strdup(random_junk);
	assert(junk != NULL);

	assert(bjxa_decode(NULL, dst_buf, sizeof dst_buf, src_buf,
	    sizeof src_buf) == -1);
	assert(errno == EFAULT);

	assert(bjxa_decode(junk, dst_buf, sizeof dst_buf, src_buf,
	    sizeof src_buf) == -1);
	assert(errno == EINVAL);

	assert(bjxa_decode(dec, dst_buf, sizeof dst_buf, src_buf,
	    sizeof src_buf) == -1);
	assert(errno == EINVAL);

	file = fopen("test/square-mono-4.xa", "r");
	assert(file != NULL);
	assert(bjxa_fread_header(dec, file) > 0);

	assert(bjxa_decode(dec, NULL, sizeof dst_buf, src_buf,
	    sizeof src_buf) == -1);
	assert(errno == EFAULT);

	assert(bjxa_decode(dec, dst_buf, 0, src_buf, sizeof src_buf) == -1);
	assert(errno == ENOBUFS);

	assert(bjxa_decode(dec, dst_buf, sizeof dst_buf, NULL,
	    sizeof src_buf) == -1);
	assert(errno == EFAULT);

	assert(bjxa_decode(dec, dst_buf, sizeof dst_buf, src_buf, 0) == -1);
	assert(errno == ENOBUFS);

	assert(bjxa_decode_format(dec, &fmt) == 0);
	assert(bjxa_decode(dec, dst_buf, fmt.block_size_pcm * 2,
	    src_buf, fmt.block_size_xa) == 1);
	assert(bjxa_decode(dec, dst_buf, fmt.block_size_pcm,
	    src_buf, fmt.block_size_xa * 2) == 1);

	/* decode past the last block */
	while (bjxa_decode(dec, dst_buf, fmt.block_size_pcm, src_buf,
	    fmt.block_size_xa) == 1);
	assert(errno == EPROTO);

	assert(bjxa_free_decoder(&dec) == 0);
	assert(dec == NULL);
	free(junk);
	assert(fclose(file) == 0);
}

ADD_TEST_CASE(riff_header_dumping)
{
	bjxa_decoder_t *dec;
	FILE *file;
	void *junk;

	dec = bjxa_decoder();
	assert(dec != NULL);

	junk = strdup(random_junk);
	assert(junk != NULL);

	assert(bjxa_fwrite_riff_header(NULL, stdout) == -1);
	assert(errno == EFAULT);

	assert(bjxa_fwrite_riff_header(junk, stdout) == -1);
	assert(errno == EINVAL);

	assert(bjxa_fwrite_riff_header(dec, stdout) == -1);
	assert(errno == EINVAL);

	assert(bjxa_dump_riff_header(NULL, dst_buf, sizeof dst_buf) == -1);
	assert(errno == EFAULT);

	assert(bjxa_dump_riff_header(junk, dst_buf, sizeof dst_buf) == -1);
	assert(errno == EINVAL);

	assert(bjxa_dump_riff_header(dec, dst_buf, sizeof dst_buf) == -1);
	assert(errno == EINVAL);

	file = fopen("test/square-mono-4.xa", "r");
	assert(file != NULL);
	assert(bjxa_fread_header(dec, file) > 0);

	assert(bjxa_fwrite_riff_header(dec, NULL) == -1);
	assert(errno == EFAULT);

	assert(bjxa_fwrite_riff_header(dec, stdin) == -1);
	assert(errno == EBADF);

	assert(bjxa_dump_riff_header(dec, NULL, sizeof dst_buf) == -1);
	assert(errno == EFAULT);

	assert(bjxa_dump_riff_header(dec, dst_buf, 0) == -1);
	assert(errno == ENOBUFS);

	assert(bjxa_free_decoder(&dec) == 0);
	assert(dec == NULL);
	free(junk);
	assert(fclose(file) == 0);
}

ADD_TEST_CASE(pcm_samples_dumping)
{
	const void *src = src_buf;
	void *dst = dst_buf;

	assert(bjxa_dump_pcm(NULL, src, 32) == -1);
	assert(errno == EFAULT);

	assert(bjxa_dump_pcm(dst, NULL, 32) == -1);
	assert(errno == EFAULT);

	assert(bjxa_dump_pcm(dst, src, 0) == -1);
	assert(errno == ENOBUFS);

	assert(bjxa_dump_pcm(dst, src, 31) == -1);
	assert(errno == ENOBUFS);

	assert(bjxa_fwrite_pcm(NULL, 32, stdout) == -1);
	assert(errno == EFAULT);

	assert(bjxa_fwrite_pcm(src, 0, stdout) == -1);
	assert(errno == ENOBUFS);

	assert(bjxa_fwrite_pcm(src, 31, stdout) == -1);
	assert(errno == ENOBUFS);

	assert(bjxa_fwrite_pcm(src, 32, NULL) == -1);
	assert(errno == EFAULT);

	assert(bjxa_fwrite_pcm(src, 32, stdin) == -1);
	assert(errno == EBADF);
}

int
main(void)
{
	const char *srcdir;

	srcdir = getenv("SRCDIR");
	assert(srcdir != NULL);
	assert(chdir(srcdir) == 0);

	RUN_TEST_CASE(memory_management);
	RUN_TEST_CASE(header_parsing);
	RUN_TEST_CASE(file_format);
	RUN_TEST_CASE(decoding);
	RUN_TEST_CASE(riff_header_dumping);
	RUN_TEST_CASE(pcm_samples_dumping);
	return (EXIT_SUCCESS);
}
