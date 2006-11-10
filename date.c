/*
 * Copyright 2004-2006 Jeff Garzik <jgarzik@pobox.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef HAVE_CONFIG_H
#error missing autoconf-generated config.h.
#endif
#include "posixutils-config.h"

#define _BSD_SOURCE

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <argp.h>
#include <libpu.h>


static const char doc[] =
N_("date - print or set the system date and time");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ "utc", 'u', NULL, 0,
	  N_("Perform operations as if the TZ environment variable was set to the string \"UTC0\"") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, NULL, doc };

static int opt_utc;
static char date_buf[4096];

static void pr_formatted(const struct tm *tm, const char *fmt)
{
	size_t len;

	len = strftime(date_buf, sizeof(date_buf), fmt, tm);
	date_buf[sizeof(date_buf) - 1] = 0;

	printf("%s\n", date_buf);
}

static struct tm *fetch_localtime(void)
{
	time_t t;
	struct tm *tm;

	t = time(NULL);
	if (t == ((time_t)-1)) {
		perror("time");
		return NULL;
	}

	tm = localtime(&t);
	if (!tm) {
		perror("localtime");
		return NULL;
	}

	return tm;
}

static int output_default(void)
{
	struct tm *tm = fetch_localtime();
	if (!tm)
		return 1;
	pr_formatted(tm, "%a %b %e %H:%M:%S %Z %Y");
	return 0;
}

static int output_format(const char *fmt)
{
	struct tm *tm = fetch_localtime();
	if (!tm)
		return 1;
	fmt++;
	pr_formatted(tm, fmt);
	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'u':
		opt_utc = 1;
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static int input_date(const char *date_str)
{
	struct tm tm, *tm_lcl;
	int rc, year1 = -1, year2 = -1;
	time_t t;
	struct timeval tv;

	tm_lcl = fetch_localtime();
	memcpy(&tm, tm_lcl, sizeof(tm));

	rc = sscanf(date_str, "%02d%02d%02d%02d%02d%02d",
		    &tm.tm_mon,
		    &tm.tm_mday,
		    &tm.tm_hour,
		    &tm.tm_min,
		    &year1, &year2);
	if (rc < 4) {
		fprintf(stderr, "invalid date format '%s'\n", date_str);
		return 1;
	}

	if (rc == 5) {
		if (year1 <= 68)
			year1 += 2000;
		else
			year1 += 1900;

		tm.tm_year = year1 - 1900;
	}
	else if (rc == 6) {
		int tmp = (year1 * 100) + year2;
		tm.tm_year = tmp - 1900;
	}

	t = mktime(&tm);
	if (t == ((time_t)-1)) {
		perror("mktime");
		return 1;
	}

	rc = 0;

	tv.tv_sec = t;
	tv.tv_usec = 0;
	if (settimeofday(&tv, NULL) < 0) {
		perror("cannot set date");
		rc = 1;
	}

	rc |= output_default();

	return rc;
}

int main (int argc, char *argv[])
{
	error_t arc;
	int rc, idx = -1;
	char *old_tz = NULL;

	pu_init();

	arc = argp_parse(&argp, argc, argv, 0, &idx, NULL);
	if (arc) {
		fprintf(stderr, "argp_parse failed: %s\n", strerror(arc));
		return 1;
	}

	if (opt_utc) {
		char *s = getenv("TZ");
		if (s)
			old_tz = xstrdup(s);
		rc = setenv("TZ", "UTC0", 1);
		if (rc < 0) {
			perror("setenv");
			return 1;
		}
	}

	if (idx >= argc)
		rc = output_default();
	else if (argv[idx][0] == '+')
		rc = output_format(argv[idx]);
	else
		rc = input_date(argv[idx]);

	if (old_tz)
		setenv("TZ", old_tz, 1);

	return rc;
}

