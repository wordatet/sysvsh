/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:macro.c	1.21.5.1"
/*
 * UNIX shell
 */

#include	"defs.h"
#include	"sym.h"
#include	<wait.h>
#include	<string.h>

static unsigned char	quote;	/* used locally */
static unsigned char	quoted;	/* used locally */
static int getch();


static void
copyto(endch, trimflag)
int trimflag;  /* flag to check if argument will be trimmed */
register int	endch;
{
	register int	c;
	register int 	d;
	register unsigned char *pc;

	while ((c = getch(endch, trimflag)) != endch && c != EOF && c)
		if (quote) {
			if(c == '\\') { /* don't interpret next character */
				pushstak(c);
				d = readc_raw();
				if(!escchar(d)) { /* both \ and following
						     character are quoted if next
						     character is not $, `, ", or \*/
					pushstak('\\'); 
					pushstak('\\'); 
					pc = readw(d); 
					/* push entire multibyte char */
					while(d = *pc++)
						pushstak(d);
				} else
					pushstak(d);
			} else { /* push escapes onto stack to quote characters */
				pc = readw(c); 
				pushstak('\\');
				while(c = *pc++)
					pushstak(c);
			}
		} else if(c == '\\') {
			c = readc_raw(); /* get character to be escaped */
			pushstak('\\');
			pushstak(c);
		} else
			pushstak(c);
	zerostak();
	if (c != endch && c != EOF && c != 0)
		error(badsub);
}

static
skipto(endch)
register int	endch;
{
	/*
	 * skip chars up to }
	 */
	register int	c;

	while ((c = readc_raw()) != EOF && c != endch)
	{
		switch (c)
		{
		case SQUOTE:
			skipto(SQUOTE);
			break;

		case DQUOTE:
			skipto(DQUOTE);
			break;

		case DOLLAR:
			if (readc_raw() == BRACE)
				skipto('}');
		}
	}
	if (c != endch)
		error(badsub);
}

static
int getch(endch, trimflag)
int	endch;
int trimflag; /* flag to check if an argument is going to be trimmed, here document
		 output is never trimmed
	 */
{
	register int	d;
	int atflag;  /* flag to check if $@ has already been seen within double 
		        quotes */
retry:
	d = readc_raw();
	if (!subchar(d))
		return(d);

	if (d == DOLLAR)
	{
		int c;

		if ((c = readc_raw(), dolchar(c)))
		{
			struct namnod *n = (struct namnod *)NIL;
			int		dolg = 0;
			BOOL		bra;
			BOOL		nulflg;
			long			rel;
			register unsigned char	*argp, *v;
			unsigned char		idb[2];
			unsigned char		*id = idb;

			if (bra = (c == BRACE))
				c = readc_raw();
			if (letter(c))
			{
				rel = relstak();
				while (alphanum(c))
				{
					pushstak(c);
					c = readc_raw();
				}
				zerostak();
				argp = absstak(rel);
				n = lookup(argp);
				setstak(rel);
				if (n->namflg & N_FUNCTN)
					error(badsub);
				v = n->namval;
				id = (unsigned char *)n->namid;
				peekc = (c == EOF ? EOF : c | MARK);
			}
			else if (digchar(c))
			{
				*id = c;
				idb[1] = 0;
				if (astchar(c))
				{
					if(c == '@' && !atflag && quote) {
						quoted--;
						atflag = 1;
					}
					dolg = 1;
					c = '1';
				}
				c -= '0';
				v = ((c == 0) ? cmdadr : ((int)c <= dolc) ? dolv[c] : (dolg = 0, (unsigned char *)0));
			}
			else if (c == '$')
				v = pidadr;
			else if (c == '!')
				v = pcsadr;
			else if (c == '#')
			{
				itos(dolc);
				v = numbuf;
			}
			else if (c == '?')
			{
				itos(retval);
				v = numbuf;
			}
			else if (c == '-')
				v = flagadr;
			else if (bra)
				error(badsub);
			else
				goto retry;
			c = readc_raw();
			if (c == ':' && bra)	/* null and unset fix */
			{
				nulflg = 1;
				c = readc_raw();
			}
			else
				nulflg = 0;
			if (!defchar(c) && bra)
				error(badsub);
			argp = 0;
			if (bra)
			{
				if (c != '}')
				{
					rel = relstak();
					if ((v == 0 || (nulflg && *v == 0)) ^ (setchar(c)))
						copyto('}', trimflag);
					else
						skipto('}');
					argp = absstak(rel);
				}
			}
			else
			{
				peekc = (c == EOF ? EOF : c | MARK);
				c = 0;
			}
			if (v && (!nulflg || *v))
			{

				if (c != '+')
				{
					for (;;)
					{
						if (*v == 0 && quote) {
							pushstak('\\');
							pushstak('\0');
						}
						else
							while (c = *v++) {
								if(quote || (c == '\\' && trimflag)) {
									register int length;
									wchar_t l;
									pushstak('\\');
									pushstak(c);
									length = mbtowc(&l, (char *)v - 1, MULTI_BYTE_MAX);
									while(--length > 0)
										pushstak(*v++);
								}
								else
									pushstak(c);
								
							}

						if (dolg == 0 || (++dolg > dolc))
							break;
						else /* $* and $@ expansion */
						{
							v = dolv[dolg];
							if(*id == '*' && quote)
/* push quoted space so that " $* " will not be broken into separate arguments */
								pushstak('\\');
							pushstak(' ');
						}
					}
				}
			}
			else if (argp)
			{
				if (c == '?') {
					if(trimflag)
						trim(argp);
					failed(id, *argp ? argp : (unsigned char *)badparam);
				}
				else if (c == '=')
				{
					if (n)
					{
						int strlngth = staktop - stakbot;
						unsigned char *savptr = fixstak();
						unsigned char *newargp;
					/*
					 * copy word onto stack, trim it, and then
					 * do assignment 
					 */
						usestak();
						while(c = *argp++) {
							if(c == '\\' && trimflag) {
								c = *argp++;
								if(!c)
									continue;
							}
							pushstak(c);
						}
						newargp = fixstak();
						assign(n, newargp);
						tdystak(savptr);
						memcpy(stakbot, savptr, strlngth);
						staktop = stakbot + strlngth;
					}
					else
						error(badsub);
				}
			}
			else if (flags & setflg)
				failed(id, unset);
			goto retry;
		}
		else
			peekc = (c == EOF ? EOF : c | MARK);
	}
	else if (d == endch)
		return(d);
	else if (d == SQUOTE)
	{
		comsubst(trimflag);
		goto retry;
	}
	else if (d == DQUOTE && trimflag)
	{
		if(!quote) {
			atflag = 0;
			quoted++;
		}
		quote ^= QUOTE;
		goto retry;
	}
	return(d);
}

unsigned char *
macro(as)
unsigned char	*as;
{
	/*
	 * Strip "" and do $ substitution
	 * Leaves result on top of stack
	 */
	register BOOL	savqu = quoted;
	register unsigned char	savq = quote;
	struct filehdr	fb;

	push(&fb);
	estabf(as);
	usestak();
	quote = 0;
	quoted = 0;
	copyto(0, 1);
	pop();
	if (quoted && (stakbot == staktop)) {
		pushstak('\\');
		pushstak('\0');
/*
 * above is the fix for *'.c' bug
 */
	}
	quote = savq;
	quoted = savqu;
	return(fixstak());
}
/* Save file descriptor for command substitution */
int savpipe = -1;

comsubst(trimflag)
int trimflag; /* used to determine if argument will later be trimmed */
{
	/*
	 * command substn
	 */
	struct fileblk	cb;
	register int	d;
	int strlngth = staktop - stakbot;
	register unsigned char *oldstaktop;
	unsigned char *savptr = fixstak();

	usestak();
	while ((d = readc_raw()) != EOF && d != SQUOTE) {
		if(d == '\\') {
			d = readc_raw();
			if(!escchar(d) || (d == '"' && !quote))
		/* trim quotes for `, \, or " if command substitution is within
		   double quotes */
				pushstak('\\');
		}
		pushstak(d);
	}
	{
		register unsigned char	*argc;

		argc = fixstak(); 
		push(&cb);
		estabf(argc);  /* read from string */
	}
	{
		register struct trenod *t;
		int		pv[2];

		/*
		 * this is done like this so that the pipe
		 * is open only when needed
		 */
	 	t = makefork(FPOU, cmd(EOFSYM, MTFLG | NLFLG ));
		chkpipe(pv);
		savpipe = pv[OTPIPE];
		initf(pv[INPIPE]); /* read from pipe */
		execute(t, XEC_NOSTOP, (int)(flags & errflg), 0, pv);
		close(pv[OTPIPE]);
		savpipe = -1;
	}
	tdystak(savptr);
	memcpy(stakbot, savptr, strlngth);
	oldstaktop = staktop = stakbot + strlngth;
	while ((d = readc_raw()) != EOF) {
		if(quote || (d == '\\' && trimflag)) {
			register unsigned char *rest;
			/* quote output from command subst. if within double 
			   quotes or backslash part of output */
			rest = readw(d);
			pushstak('\\');
			while(d = *rest++)
			/* Pick up all of multibyte character */
				pushstak(d);
		}
		else
			pushstak(d);
	}
	{
		extern pid_t parent;
		int stat;
		register rc;
		while (waitpid(parent,&stat,0) != parent)
			continue;
		if (WIFEXITED(stat))
			rc = WEXITSTATUS(stat);
		else
			rc = (WTERMSIG(stat) | SIGFLG);
		if (rc && (flags & errflg))
			exitsh(rc);
		exitval = rc;
		flags |= eflag;
		exitset();
	}
	while (oldstaktop != staktop)
	{ /* strip off trailing newlines from command substitution only */
		if ((*--staktop) != NL)
		{
			++staktop;
			break;
		} else if(quote)
			staktop--; /* skip past backslashes if quoting */ 
	}
	pop();
}

#define CPYSIZ	512

subst(in, ot)
int	in, ot;
{
	register int	c;
	struct fileblk	fb;
	unsigned char	buffer[CPYSIZ];
	register int	pos = 0;

	push(&fb);
	initf(in);
	/*
	 * DQUOTE used to stop it from quoting
	 */
	while ((c = (getch(DQUOTE, 0))) != EOF)
	{
		if(c == '\\') {
			c = readc_raw();
			if(!escchar(c) || c == '"') {
				buffer[pos++] = '\\';
				if (pos == CPYSIZ) {
					write(ot, buffer, pos);
					pos = 0;
				}
			}
		}
		buffer[pos++] = c;
		if (pos == CPYSIZ)
		{
			write(ot, buffer, pos);
			pos = 0;
		}
	}
	if (pos > 0)
		write(ot, buffer, pos);
	pop();
}

flush(ot)
{
	/* This function is now mostly obsolete but kept for compatibility */
}
