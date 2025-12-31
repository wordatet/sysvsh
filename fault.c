/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sh:fault.c	1.13.22.1"
/*
 * UNIX shell
 */

#include	"defs.h"

static void fault();
static BOOL sleeping = 0;
static unsigned char *trapcom[MAXTRAP];
static BOOL trapflg[MAXTRAP];
static void (*sigval[MAXTRAP])();

void siginit()
{
	static int init = 0;
	int i;
	if (init) return;
	for (i = 0; i < MAXTRAP; i++) {
		trapflg[i] = 0;
		sigval[i] = 0;
	}
	sigval[SIGHUP] = done;
	sigval[SIGINT] = fault;
	sigval[SIGQUIT] = fault;
	sigval[SIGILL] = done;
	sigval[SIGTRAP] = done;
#ifdef SIGIOT
	sigval[SIGIOT] = done;
#endif
#ifdef SIGEMT
	sigval[SIGEMT] = done;
#endif
	sigval[SIGFPE] = done;
	sigval[SIGBUS] = done;
	sigval[SIGSEGV] = fault;
	sigval[SIGSYS] = done;
	sigval[SIGPIPE] = done;
	sigval[SIGALRM] = done;
	sigval[SIGTERM] = fault;
	sigval[SIGUSR1] = done;
	sigval[SIGUSR2] = done;
	sigval[SIGXCPU] = done;
	sigval[SIGXFSZ] = done;
	sigval[SIGVTALRM] = done;
	sigval[SIGPROF] = done;
	/* SIGCHLD (17 on Linux) is intentionally 0 to be ignored by stdsigs() default loop */
	init = 1;
}


static int
ignoring(i)
register int i;
{
	struct sigaction act;
	if (trapflg[i] & SIGIGN)
		return 1;
	sigaction(i, 0, &act);
	if (act.sa_handler == SIG_IGN) {
		trapflg[i] |= SIGIGN;
		return 1;
	}
	return 0;
}

static void
clrsig(i)
int	i;
{
	if (trapcom[i] != 0) {
		sh_free(trapcom[i]);
		trapcom[i] = 0;
	}
	if (trapflg[i] & SIGMOD) {
		trapflg[i] &= ~(SIGMOD | SIGIGN);
		handle(i, sigval[i]);
	}
}

void 
done(sig)
{
	register unsigned char	*t;

	if (t = trapcom[0])
	{
		trapcom[0] = 0;
		execexp(t, 0);
		sh_free(t);
	}
	else
		chktrap();

	rmtemp(0);
	rmfunctmp();

#ifdef ACCT
	doacct();
#endif
	(void)endjobs(0);
	if (sig) {
		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, sig);
		sigprocmask(SIG_UNBLOCK, &set, 0);
		handle(sig, SIG_DFL);
		kill(mypid, sig);
	}
	exit(exitval);
}

static void 
fault(sig)
register int	sig;
{
	register int flag;
	
	switch (sig) {
/*	case SIGSEGV:
			if (setbrk(brkincr) == -1)
				error(nospace);
			return; */
		case SIGSEGV:
		case SIGBUS:
				done(sig);
				return;
		case SIGALRM:
				break;
	}

	if (trapcom[sig])
		flag = TRAPSET;
	else if (flags & subsh)
		done(sig);
	else
		flag = SIGSET;

	trapnote |= flag;
	trapflg[sig] |= flag;
}

int
handle(sig, func)
	int sig; 
	void (*func)();
{
	struct sigaction act, oact;
	if (func == SIG_IGN && (trapflg[sig] & SIGIGN))
		return 0;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = func;
	sigaction(sig, &act, &oact);
	if (func == SIG_IGN)
		trapflg[sig] |= SIGIGN;
	return (func != oact.sa_handler);
}

void
stdsigs()
{
	register int	i;
	siginit();

	for (i = 1; i < MAXTRAP; i++) {
		if (sigval[i] == 0)
			continue;
		if (i != SIGSEGV && ignoring(i))
			continue;
		handle(i, sigval[i]);
	}
}

void
oldsigs()
{
	register int	i;
	register unsigned char	*t;

	i = MAXTRAP;
	while (i--)
	{
		t = trapcom[i];
		if (t == 0 || *t)
			clrsig(i);
		trapflg[i] = 0;
	}
	trapnote = 0;
}

/*
 * check for traps
 */

void
chktrap()
{
	register int	i = MAXTRAP;
	register unsigned char	*t;

	trapnote &= ~TRAPSET;
	while (--i)
	{
		if (trapflg[i] & TRAPSET)
		{
			trapflg[i] &= ~TRAPSET;
			if (t = trapcom[i])
			{
				int	savxit = exitval;
				execexp(t, 0);
				exitval = savxit;
				exitset();
			}
		}
	}
}

systrap(argc,argv)
int argc;
char **argv;
{
	int sig;

	if (argc == 1) {
		for (sig = 0; sig < MAXTRAP; sig++) {
			if (trapcom[sig]) {
				prn_buff(sig);
				prs_buff(colon);
				prs_buff(trapcom[sig]);
				prc_buff(NL);
			}
		}
	} else {
		char *cmd = *argv, *a1 = *(argv+1);
		BOOL noa1;
		noa1 = (str2sig(a1,&sig) == 0);
		if (noa1 == 0)
			++argv;
		while (*++argv) {
			if (str2sig(*argv,&sig) < 0 ||
			  sig >= MAXTRAP || sig < MINTRAP || 
			  sig == SIGSEGV)
				failure(cmd, badtrap);
			else if (noa1)
				clrsig(sig);
                        else if (*a1) {
				if (trapflg[sig] & SIGMOD || !ignoring(sig)) {
					handle(sig, fault);
					trapflg[sig] |= SIGMOD;
					replace(&trapcom[sig], a1);
				}
			} else if (handle(sig, SIG_IGN)) {
				trapflg[sig] |= SIGMOD;
				replace(&trapcom[sig], a1);
			}
		}
	}
}

sh_sleep(ticks)
int ticks;
{
	sigset_t set, oset;
	struct sigaction act, oact;


	/*
	 * add SIGALRM to mask
	 */

	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	sigprocmask(SIG_BLOCK, &set, &oset);

	/*
	 * catch SIGALRM
	 */

	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = fault;
	sigaction(SIGALRM, &act, &oact);

	/*
	 * start alarm and wait for signal
	 */

	alarm(ticks);
	sleeping = 1;
	sigsuspend(&oset);
	sleeping = 0;

	/*
	 * reset alarm, catcher and mask
	 */

	alarm(0); 
	sigaction(SIGALRM, &oact, NULL);
	sigprocmask(SIG_SETMASK, &oset, 0);

}
