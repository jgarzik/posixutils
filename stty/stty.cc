
/*
 * Copyright 2008-2009 Red Hat, Inc.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#ifdef __APPLE__
#include <sys/termios.h>
#endif
#include <libpu.h>

#define COMPACT_PFX "PUSTTY1:"

enum {
	stty_cfl		= 0,
	stty_ispeed		= 1,
	stty_ospeed		= 2,
	stty_ifl		= 3,
	stty_ofl		= 4,
	stty_lfl		= 5,
	stty_cchar		= 6,
};

static const struct stty_param {
	const char		*name;		/* name on cmd line */
	int			ptype;		/* type of param */
	tcflag_t		val;		/* value to set */
	tcflag_t		val_clear;	/* value to clear */
} params[] = {
	/*
	 * control flags
	 */
	{ "parenb", stty_cfl, PARENB, PARENB },
	{ "parodd", stty_cfl, PARODD, PARODD },

	{ "cs5", stty_cfl, CS5, CSIZE },
	{ "cs6", stty_cfl, CS6, CSIZE },
	{ "cs7", stty_cfl, CS7, CSIZE },
	{ "cs8", stty_cfl, CS8, CSIZE },

	{ "ispeed", stty_ispeed, },
	{ "ospeed", stty_ospeed, },

	{ "hupcl", stty_cfl, HUPCL, HUPCL },
	/* FIXME: support "hup", alias for "hupcl" */

	{ "cstopb", stty_cfl, CSTOPB, CSTOPB },
	{ "cread", stty_cfl, CREAD, CREAD },
	{ "clocal", stty_cfl, CLOCAL, CLOCAL },

	/*
	 * input flags
	 */
	{ "ignbrk", stty_ifl, IGNBRK, IGNBRK },
	{ "brkint", stty_ifl, BRKINT, BRKINT },
	{ "ignpar", stty_ifl, IGNPAR, IGNPAR },
	{ "parmrk", stty_ifl, PARMRK, PARMRK },
	{ "inpck", stty_ifl, INPCK, INPCK },
	{ "istrip", stty_ifl, ISTRIP, ISTRIP },
	{ "inlcr", stty_ifl, INLCR, INLCR },
	{ "igncr", stty_ifl, IGNCR, IGNCR },
	{ "icrnl", stty_ifl, ICRNL, ICRNL },
	{ "ixon", stty_ifl, IXON, IXON },
	{ "ixany", stty_ifl, IXANY, IXANY },
	{ "ixoff", stty_ifl, IXOFF, IXOFF },

	/*
	 * output flags
	 */
	{ "opost", stty_ofl, OPOST, OPOST },
	{ "ocrnl", stty_ofl, OCRNL, OCRNL },
	{ "onocr", stty_ofl, ONOCR, ONOCR },
	{ "onlret", stty_ofl, ONLRET, ONLRET },
	{ "ofill", stty_ofl, OFILL, OFILL },
	{ "ofdel", stty_ofl, OFDEL, OFDEL },

	{ "cr0", stty_ofl, CR0, CRDLY },
	{ "cr1", stty_ofl, CR1, CRDLY },
	{ "cr2", stty_ofl, CR2, CRDLY },
	{ "cr3", stty_ofl, CR3, CRDLY },

	{ "nl0", stty_ofl, NL0, NLDLY },
	{ "nl1", stty_ofl, NL1, NLDLY },

	/* FIXME: support "tabs", alias for "tab0" */
	{ "tab0", stty_ofl, TAB0, TABDLY },
	{ "tab1", stty_ofl, TAB1, TABDLY },
	{ "tab2", stty_ofl, TAB2, TABDLY },
	{ "tab3", stty_ofl, TAB3, TABDLY },

	{ "bs0", stty_ofl, BS0, BSDLY },
	{ "bs1", stty_ofl, BS1, BSDLY },

	{ "ff0", stty_ofl, FF0, FFDLY },
	{ "ff1", stty_ofl, FF1, FFDLY },

	{ "vt0", stty_ofl, VT0, VTDLY },
	{ "vt1", stty_ofl, VT1, VTDLY },

	/*
	 * local modes
	 */
	{ "isig", stty_lfl, ISIG, ISIG },
	{ "icanon", stty_lfl, ICANON, ICANON },
	{ "iexten", stty_lfl, IEXTEN, IEXTEN },
	{ "echo", stty_lfl, ECHO, ECHO },
	{ "echoe", stty_lfl, ECHOE, ECHOE },
	{ "echok", stty_lfl, ECHOK, ECHOK },
	{ "echonl", stty_lfl, ECHONL, ECHONL },
	{ "noflsh", stty_lfl, NOFLSH, NOFLSH },
	{ "tostop", stty_lfl, TOSTOP, TOSTOP },

	/*
	 * control characters
	 */
	{ "eof", stty_cchar, VEOF },
	{ "eol", stty_cchar, VEOL },
	{ "erase", stty_cchar, VERASE },
	{ "intr", stty_cchar, VINTR },
	{ "kill", stty_cchar, VKILL },
	{ "quit", stty_cchar, VQUIT },
	{ "susp", stty_cchar, VSUSP },
	{ "start", stty_cchar, VSTART },
	{ "stop", stty_cchar, VSTOP },

	/* FIXME: handle multi-param aliases...
		evenp
		parity
		oddp
		-parity, -evenp, -oddp
		raw, -raw, cooked
		nl, -nl
		ek,
		sane
	 */
};

static struct termios ti;

static int usage(void)
{
	fprintf(stderr,
"stty [-a | -g]\n"
"	or\n"
"stty operands...\n");
	return 1;
}

static int stty_push_ti(void)
{
	if (tcsetattr(STDIN_FILENO, TCSANOW, &ti)) {
		perror("stty(tcsetattr)");
		return 1;
	}

	return 0;
}

static int stty_show(void)
{
	return 1;	/* FIXME: implement stty settings display */
}

static int stty_show_compact(void)
{
	printf(COMPACT_PFX "%x:%x:%x:%x:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u\n",
	       ti.c_iflag,
	       ti.c_oflag,
	       ti.c_cflag,
	       ti.c_lflag,
	       ti.c_ispeed,
	       ti.c_ospeed,
	       ti.c_cc[VEOF],
	       ti.c_cc[VEOF],
	       ti.c_cc[VERASE],
	       ti.c_cc[VINTR],
	       ti.c_cc[VKILL],
	       ti.c_cc[VQUIT],
	       ti.c_cc[VSUSP],
	       ti.c_cc[VSTART],
	       ti.c_cc[VSTOP]);
	return 0;
}

static int stty_set_compact(const char *settings)
{
	int rc, c_cc[20], i;

	rc = sscanf(settings,
		    COMPACT_PFX "%x:%x:%x:%x:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u\n",
		    &ti.c_iflag,
		    &ti.c_oflag,
		    &ti.c_cflag,
		    &ti.c_lflag,
		    &ti.c_ispeed,
		    &ti.c_ospeed,
		    &c_cc[VEOF],
		    &c_cc[VEOF],
		    &c_cc[VERASE],
		    &c_cc[VINTR],
		    &c_cc[VKILL],
		    &c_cc[VQUIT],
		    &c_cc[VSUSP],
		    &c_cc[VSTART],
		    &c_cc[VSTOP]);
	if (rc != 15) {
		fprintf(stderr, _("stty: invalid compact settings format\n"));
		return 1;
	}

	for (i = 0; i <= 16; i++)
		ti.c_cc[i] = (unsigned char) c_cc[i];

	return stty_push_ti();
}

static const struct stty_param *last_param;

static int param_apply(const struct stty_param *param, const char *setting,
		       bool set_val)
{
	switch (param->ptype) {
	case stty_cfl:
		ti.c_cflag &= ~param->val_clear;
		if (set_val)
			ti.c_cflag |= param->val;
		break;
	case stty_ifl:
		ti.c_iflag &= ~param->val_clear;
		if (set_val)
			ti.c_iflag |= param->val;
		break;
	case stty_ofl:
		ti.c_oflag &= ~param->val_clear;
		if (set_val)
			ti.c_oflag |= param->val;
		break;
	case stty_lfl:
		ti.c_lflag &= ~param->val_clear;
		if (set_val)
			ti.c_lflag |= param->val;
		break;

	case stty_ispeed:
	case stty_ospeed:
	case stty_cchar:
		last_param = param;
		return -1;	/* obtain next arg */

	default:
		return 1;
	}

	return 0;
}

static int param_cchar(const struct stty_param *param, const char *setting)
{
	return 1;		/* FIXME: interpret control char setting */
}

#define X(speed) \
	case (speed): return B##speed;

static unsigned int speed_to_bits(unsigned int speed)
{
	switch (speed) {
	case 0: return B0;
	case 50: return B50;
	case 75: return B75;
	case 110: return B110;
	case 134: return B134;
	case 150: return B150;
	case 200: return B200;
	case 300: return B300;
	case 600: return B600;
	case 1200: return B1200;
	case 1800: return B1800;
	case 2400: return B2400;
	case 4800: return B4800;
	case 9600: return B9600;
	case 19200: return B19200;
	case 38400: return B38400;
	case 57600: return B57600;
	case 115200: return B115200;
	case 230400: return B230400;

#ifdef __linux__
	case 460800: return B460800;
	case 500000: return B500000;
	case 576000: return B576000;
	case 921600: return B921600;
	case 1000000: return B1000000;
	case 1152000: return B1152000;
	case 1500000: return B1500000;
	case 2000000: return B2000000;
	case 2500000: return B2500000;
	case 3000000: return B3000000;
	case 3500000: return B3500000;
	case 4000000: return B4000000;
#endif

	default:
		break;
	}

	return 0xffffffffU;
}

#undef X

static int param_speed(unsigned int speed, bool is_input)
{
	unsigned int bits = speed_to_bits(speed);
	int rc;

	if (bits == 0xffffffffU)
		return 1;

	if (is_input)
		rc = cfsetispeed(&ti, bits);
	else
		rc = cfsetospeed(&ti, bits);

	if (rc < 0)
		return 1;

	return 0;
}

static int stty_set(const char *setting)
{
	bool set_val = true;
	unsigned int speed, i;

	if (!setting) {
		fprintf(stderr, _("stty: '%s' missing required argument\n"),
			last_param ? last_param->name : "(null)");
		return 1;
	}

	if (last_param) {
		const struct stty_param *param;
		unsigned int speed;

		param = last_param;
		last_param = NULL;

		switch (param->ptype) {
		case stty_ispeed:
			if (sscanf(setting, "%u", &speed) == 1)
				return param_speed(speed, true);
			break;
		case stty_ospeed:
			if (sscanf(setting, "%u", &speed) == 1)
				return param_speed(speed, false);
			break;
		case stty_cchar:
			return param_cchar(param, setting);
		}

		goto err_out;
	}

	if (*setting == '-') {
		set_val = false;
		setting++;
	}

	for (i = 0; i < ARRAY_SIZE(params); i++) {
		if (!strcmp(setting, params[i].name))
			return param_apply(&params[i], setting, set_val);
	}

	if (sscanf(setting, "%u", &speed) == 1) {
		if (param_speed(speed, true))
			return 1;
		if (param_speed(speed, false))
			return 1;
		return 0;
	}

err_out:
	fprintf(stderr, _("stty: invalid argument '%s'\n"), setting);
	return 1;
}

static int is_help_arg(const char *arg)
{
	if (!strcmp(arg, "-h") ||
	    !strcmp(arg, "--help") ||
	    !strcmp(arg, "-v") ||
	    !strcmp(arg, "-V") ||
	    !strcmp(arg, "-H") ||
	    !strcmp(arg, "-?"))
		return 1;

	return 0;
}

static bool is_compact_form(const char *arg)
{
	return (strncmp(arg, COMPACT_PFX, strlen(COMPACT_PFX)) == 0);
}

int main (int argc, char *argv[])
{
	int i, rc;

	pu_init();

	if ((argc == 2) && (is_help_arg(argv[1])))
		return usage();

	if (tcgetattr(STDIN_FILENO, &ti)) {
		perror("stty(tcgetattr)");
		return 1;
	}

	if (argc == 1)
		return stty_show();
	else if ((argc == 2) && (!strcmp(argv[1], "-a")))
		return stty_show();

	else if ((argc == 2) && (!strcmp(argv[1], "-g")))
		return stty_show_compact();
	else if ((argc == 2) && (is_compact_form(argv[1])))
		return stty_set_compact(argv[1]);

	for (i = 1; i < argc; i++) {
		rc = stty_set(argv[i]);
		if (rc > 0)
			return rc;
		if (rc < 0) {
			i++;
			rc = stty_set(i >= argc ? NULL : argv[i]);
			if (rc)
				return rc;
		}
	}

	return stty_push_ti();
}

