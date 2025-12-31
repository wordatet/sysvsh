/*
 * Compatibility shims for SVR4 shell on Linux
 * K&R style preserved
 */

#include "defs.h"
#include <dirent.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <wctype.h>

int _sys_nsig = NSIG;
char *_sh_siglist[NSIG];
char **_sys_siglist = _sh_siglist;

void sh_init()
{
	int i;
	for (i = 0; i < NSIG; i++)
		_sh_siglist[i] = strsignal(i);
}

int wisprint(c)
wchar_t c;
{
	return iswprint(c);
}

/*
 * gmatch - glob pattern matching
 * Returns non-zero if s matches pattern p
 */
gmatch(s, p)
char *s;
char *p;
{
	register int scc;
	register int c;

	if (scc = *s++)
		if ((scc &= 0177) == 0)
			scc = 0200;

	switch (c = *p++) {
	case '[':
		{
			int ok = 0;
			int lc = -1;
			int notflag = 0;

			if (*p == '!') {
				notflag = 1;
				p++;
			}

			while (c = *p++) {
				if (c == ']')
					return ok ? gmatch(s, p) : 0;
				if (c == '-' && lc >= 0 && *p != ']') {
					c = *p++;
					if (notflag) {
						if (scc < lc || scc > c)
							ok++;
					} else {
						if (lc <= scc && scc <= c)
							ok++;
					}
				} else {
					lc = c;
					if (notflag) {
						if (scc != lc)
							ok++;
					} else {
						if (scc == lc)
							ok++;
					}
				}
			}
			return 0;
		}

	case '*':
		if (!*p)
			return 1;
		s--;
		while (*s)
			if (gmatch(s++, p))
				return 1;
		return 0;

	case '?':
		return scc ? gmatch(s, p) : 0;

	case 0:
		return scc == 0;

	case '\\':
		if ((*p & 0177) != scc)
			return 0;
		p++;
		return gmatch(s, p);

	default:
		if ((c & 0177) != scc)
			return 0;
		return gmatch(s, p);
	}
}

/*
 * mbftowc - multibyte file to wide char (single-byte shim)
 */
mbftowc(s, wchar, f, peekc)
char *s;
wchar_t *wchar;
int (*f)();
int *peekc;
{
	register int c;

	if (peekc && *peekc) {
		c = *peekc;
		*peekc = 0;
	} else if (f) {
		c = (*f)();
	} else {
		return 0;
	}

	if (c == 0 || c == EOF)
		return 0;

	if (s)
		s[0] = c;
	if (wchar)
		*wchar = c & 0377;
	return 1;
}

/*
 * sig2str - convert signal number to name
 */
sig2str(sig, s)
int sig;
char *s;
{
	switch (sig) {
	case SIGHUP:	strcpy(s, "HUP"); break;
	case SIGINT:	strcpy(s, "INT"); break;
	case SIGQUIT:	strcpy(s, "QUIT"); break;
	case SIGKILL:	strcpy(s, "KILL"); break;
	case SIGTERM:	strcpy(s, "TERM"); break;
	case SIGSTOP:	strcpy(s, "STOP"); break;
	case SIGTSTP:	strcpy(s, "TSTP"); break;
	case SIGCONT:	strcpy(s, "CONT"); break;
	default:
		sprintf(s, "%d", sig);
	}
	return 0;
}

/*
 * str2sig - convert signal name to number
 */
str2sig(s, sigp)
char *s;
int *sigp;
{
	if (strcmp(s, "HUP") == 0)	{ *sigp = SIGHUP; return 0; }
	if (strcmp(s, "INT") == 0)	{ *sigp = SIGINT; return 0; }
	if (strcmp(s, "QUIT") == 0)	{ *sigp = SIGQUIT; return 0; }
	if (strcmp(s, "KILL") == 0)	{ *sigp = SIGKILL; return 0; }
	if (strcmp(s, "TERM") == 0)	{ *sigp = SIGTERM; return 0; }
	if (strcmp(s, "STOP") == 0)	{ *sigp = SIGSTOP; return 0; }
	if (strcmp(s, "TSTP") == 0)	{ *sigp = SIGTSTP; return 0; }
	if (strcmp(s, "CONT") == 0)	{ *sigp = SIGCONT; return 0; }
	*sigp = atoi(s);
	return 0;
}
