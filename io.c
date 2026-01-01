/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:io.c	1.10.4.1"

/*
 * UNIX shell
 */

#include	"defs.h"
#include	"dup.h"
#include	<fcntl.h>

short topfd;

/* ========	input output and file copying ======== */

initf(fd)
int	fd;
{
	register struct fileblk *f = standin;

	f->fdes = fd;
	f->fsiz = ((flags & oneflg) == 0 ? SH_BUFSIZ : 1);
	f->fnxt = f->fend = f->fbuf;
	f->feval = 0;
	f->flin = 1;
	f->feof = FALSE;
}

estabf(s)
register unsigned char *s;
{
	register struct fileblk *f;

	(f = standin)->fdes = -1;
	f->fend = length(s) + (f->fnxt = s);
	f->flin = 1;
	return(f->feof = (s == 0));
}

push(af)
struct fileblk *af;
{
	register struct fileblk *f;

	(f = af)->fstak = standin;
	f->feof = 0;
	f->feval = 0;
	standin = f;
}

pop()
{
	register struct fileblk *f;

	if ((f = standin)->fstak)
	{
		if (f->fdes >= 0)
			close(f->fdes);
		standin = f->fstak;
		return(TRUE);
	}
	else
		return(FALSE);
}

struct tempblk *tmpfptr;

pushtemp(fd,tb)
	int fd;
	struct tempblk *tb;
{
	tb->fdes = fd;
	tb->fstak = tmpfptr;
	tmpfptr = tb;
}

poptemp()
{
	if (tmpfptr)
	{
		close(tmpfptr->fdes);
		tmpfptr = tmpfptr->fstak;
		return(TRUE);
	}
	else
		return(FALSE);
}
	
chkpipe(pv)
int	*pv;
{
	if (pipe(pv) < 0 || pv[INPIPE] < 0 || pv[OTPIPE] < 0)
		error(piperr);
}

chkopen(idf)
unsigned char *idf;
{
	register int	rc;

	if ((rc = open((char *)idf, 0)) < 0)
		failed(idf, badopen);
	else
		return(rc);
}

sh_rename(f1, f2)
register int	f1, f2;
{
	if (f1 != f2)
	{
		(void) dup2(f1, f2);
		close(f1);
		if (f2 == 0)
			ioset |= 1;
	}
}

create(s)
unsigned char *s;
{
	register int	rc;

	if ((rc = open((char *)s, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0)
		failed(s, badcreate);
	else
		return(rc);
}

tmpfil(tb)
	struct tempblk *tb;
{
	int fd;

	itos(serial++);
	movstr(numbuf, sh_tmpnam);
	fd = create(tmpout);
	pushtemp(fd,tb);
	return(fd);
}

/*
 * set by trim
 */
extern BOOL		nosubst;
#define			CPYSIZ		512

#define HEREDOC_BUFLEN 1024

void
copy(ioparg)
struct ionod	*ioparg;
{
	register struct ionod	*iop;
	int	c;
	unsigned char	*ends;
	int		fd;
	int		i;
	int		stripflg;
	unsigned char line[HEREDOC_BUFLEN];
	unsigned char *clinep;

	if (iop = ioparg)
	{
		struct tempblk tb;

		copy(iop->iolst);
		ends = mactrim(iop->ioname);
		stripflg = iop->iofile & IOSTRIP;
		if (nosubst)
			iop->iofile &= ~IODOC;
		fd = tmpfil(&tb);

		if (fndef)
			iop->ioname = make(tmpout);
		else
			iop->ioname = cpystak(tmpout);

		iop->iolst = iotemp;
		iotemp = iop;

		if (stripflg)
		{
			iop->iofile &= ~IOSTRIP;
			while (*ends == '\t')
				ends++;
		}
		
		clinep = line;
		for (;;)
		{
			chkpr();
			if (nosubst)
			{
				c = readc();
				if (stripflg)
					while (c == '\t')
						c = readc();

				while (!eolchar(c))
				{
					if (clinep < line + HEREDOC_BUFLEN - 1)
						*clinep++ = c;
					c = readc();
				}
			}
			else
			{
				c = nextc();
				if (stripflg)
					while (c == '\t')
						c = nextc();
				
				while (!eolchar(c))
				{
					if (clinep < line + HEREDOC_BUFLEN - 1)
						*clinep++ = c;
					if(c == '\\') {
						c = readc();
						if (clinep < line + HEREDOC_BUFLEN - 1)
							*clinep++ = c;
					}
					c = nextc();
				}
			}

			*clinep = 0;
			if (eof || eq(line, ends))
			{
				if ((i = clinep - line) > 0)
					write(fd, line, i);
				break;
			}
			else
			{
				if (clinep < line + HEREDOC_BUFLEN - 1)
					*clinep++ = NL;
			}

			/* If buffer full, flush it */
			if ((i = clinep - line) >= HEREDOC_BUFLEN - MULTI_BYTE_MAX - 2)
			{
				write(fd, line, i);
				clinep = line;
			}
		}

		poptemp();		/* pushed in tmpfil -- bug fix for problem
					   deleting in-line scripts */
	}
}


link_iodocs(i)
	struct ionod	*i;
{
	while(i)
	{
		sh_free(i->iolink);

		itos(serial++);
		movstr(numbuf, sh_tmpnam);
		i->iolink = make(tmpout);
		link(i->ioname, i->iolink);

		i = i->iolst;
	}
}


swap_iodoc_nm(i)
	struct ionod	*i;
{
	while(i)
	{
		sh_free(i->ioname);
		i->ioname = i->iolink;
		i->iolink = 0;

		i = i->iolst;
	}
}


savefd(fd)
	int fd;
{
	register int	f;

	f = fcntl(fd, F_DUPFD, 10);
	return(f);
}


restore(last)
	register int	last;
{
	register int 	i;
	register int	dupfd;

	for (i = topfd - 1; i >= last; i--)
	{
		if ((dupfd = fdmap[i].dup_fd) > 0)
			sh_rename(dupfd, fdmap[i].org_fd);
		else
			close(fdmap[i].org_fd);
	}
	topfd = last;
}
