
/*

 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.

 *

 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@

 * 

 * This file contains Original Code and/or Modifications of Original Code

 * as defined in and that are subject to the Apple Public Source License

 * Version 2.0 (the 'License'). You may not use this file except in

 * compliance with the License. The rights granted to you under the License

 * may not be used to create, or enable the creation or redistribution of,

 * unlawful or unlicensed copies of an Apple operating system, or to

 * circumvent, violate, or enable the circumvention or violation of, any

 * terms of an Apple operating system software license agreement.

 * 

 * Please obtain a copy of the License at

 * http://www.opensource.apple.com/apsl/ and read it before using this file.

 * 

 * The Original Code and all software distributed under the License are

 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER

 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,

 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,

 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.

 * Please see the License for the specific language governing rights and

 * limitations under the License.

 * 

 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@

 */

/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */

/*

 * Copyright (c) 1982, 1986, 1989, 1991, 1993

 *	The Regents of the University of California.  All rights reserved.

 * (c) UNIX System Laboratories, Inc.

 * All or some portions of this file are derived from material licensed

 * to the University of California by American Telephone and Telegraph

 * Co. or Unix System Laboratories, Inc. and are reproduced herein with

 * the permission of UNIX System Laboratories, Inc.

 *

 * Redistribution and use in source and binary forms, with or without

 * modification, are permitted provided that the following conditions

 * are met:

 * 1. Redistributions of source code must retain the above copyright

 *    notice, this list of conditions and the following disclaimer.

 * 2. Redistributions in binary form must reproduce the above copyright

 *    notice, this list of conditions and the following disclaimer in the

 *    documentation and/or other materials provided with the distribution.

 * 3. All advertising materials mentioning features or use of this software

 *    must display the following acknowledgement:

 *	This product includes software developed by the University of

 *	California, Berkeley and its contributors.

 * 4. Neither the name of the University nor the names of its contributors

 *    may be used to endorse or promote products derived from this software

 *    without specific prior written permission.

 *

 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND

 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE

 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE

 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE

 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL

 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS

 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)

 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT

 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY

 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF

 * SUCH DAMAGE.

 *

 *	@(#)signal.h	8.2 (Berkeley) 1/21/94

 */



#ifndef	_SYS_SIGNAL_H_

#define	_SYS_SIGNAL_H_



#include &lt;sys/cdefs.h&gt;

#include &lt;sys/appleapiopts.h&gt;



#define __DARWIN_NSIG	32	/* counting 0; could be 33 (mask is 1-32) */



#if !defined(_ANSI_SOURCE) &amp;&amp; (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))

#define NSIG	__DARWIN_NSIG

#endif



#include &lt;machine/signal.h&gt;	/* sigcontext; codes for SIGILL, SIGFPE */



#define	SIGHUP	1	/* hangup */

#define	SIGINT	2	/* interrupt */

#define	SIGQUIT	3	/* quit */

#define	SIGILL	4	/* illegal instruction (not reset when caught) */

#define	SIGTRAP	5	/* trace trap (not reset when caught) */

#define	SIGABRT	6	/* abort() */

#if  (defined(_POSIX_C_SOURCE) &amp;&amp; !defined(_DARWIN_C_SOURCE))

#define	SIGPOLL	7	/* pollable event ([XSR] generated, not supported) */

#else	/* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */

#define	SIGIOT	SIGABRT	/* compatibility */

#define	SIGEMT	7	/* EMT instruction */

#endif	/* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */

#define	SIGFPE	8	/* floating point exception */

#define	SIGKILL	9	/* kill (cannot be caught or ignored) */

#define	SIGBUS	10	/* bus error */

#define	SIGSEGV	11	/* segmentation violation */

#define	SIGSYS	12	/* bad argument to system call */

#define	SIGPIPE	13	/* write on a pipe with no one to read it */

#define	SIGALRM	14	/* alarm clock */

#define	SIGTERM	15	/* software termination signal from kill */

#define	SIGURG	16	/* urgent condition on IO channel */

#define	SIGSTOP	17	/* sendable stop signal not from tty */

#define	SIGTSTP	18	/* stop signal from tty */

#define	SIGCONT	19	/* continue a stopped process */

#define	SIGCHLD	20	/* to parent on child stop or exit */

#define	SIGTTIN	21	/* to readers pgrp upon background tty read */

#define	SIGTTOU	22	/* like TTIN for output if (tp-&gt;t_local&amp;LTOSTOP) */

#if  (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))

#define	SIGIO	23	/* input/output possible signal */

#endif

#define	SIGXCPU	24	/* exceeded CPU time limit */

#define	SIGXFSZ	25	/* exceeded file size limit */

#define	SIGVTALRM 26	/* virtual time alarm */

#define	SIGPROF	27	/* profiling time alarm */

#if  (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))

#define SIGWINCH 28	/* window size changes */

#define SIGINFO	29	/* information request */

#endif

#define SIGUSR1 30	/* user defined signal 1 */

#define SIGUSR2 31	/* user defined signal 2 */



#if defined(_ANSI_SOURCE) || __DARWIN_UNIX03 || defined(__cplusplus)

/*

 * Language spec sez we must list exactly one parameter, even though we

 * actually supply three.  Ugh!

 * SIG_HOLD is chosen to avoid KERN_SIG_* values in &lt;sys/signalvar.h&gt;

 */

#define	SIG_DFL		(void (*)(int))0

#define	SIG_IGN		(void (*)(int))1

#define	SIG_HOLD	(void (*)(int))5

#define	SIG_ERR		((void (*)(int))-1)

#else

/* DO NOT REMOVE THE COMMENTED OUT int: fixincludes needs to see them */

#define	SIG_DFL		(void (*)(/*int*/))0

#define	SIG_IGN		(void (*)(/*int*/))1

#define	SIG_HOLD	(void (*)(/*int*/))5

#define	SIG_ERR		((void (*)(/*int*/))-1)

#endif



#ifndef _ANSI_SOURCE

