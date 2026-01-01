/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:stak.c	1.8.3.1"

/*
 * UNIX shell
 */

#include	"defs.h"
#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#define STAK_MAX_SIZE (256 * 1024 * 1024) /* 256MB Virtual Limit */

static unsigned char *stak_buffer = NULL;
static size_t stak_size = 0;

void
stak_ensure(needed)
size_t needed;
{
    if (stak_buffer == NULL) {
        /* Reserve a large virtual region but let the OS handle actual paging */
        stak_buffer = (unsigned char *)mmap(NULL, STAK_MAX_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (stak_buffer == MAP_FAILED) {
            perror("sh: mmap stack");
            exit(1);
        }
        stakbas = stakbot = staktop = stak_buffer;
        brkend = stak_buffer + STAK_MAX_SIZE;
        stak_size = STAK_MAX_SIZE;
    }

    if (staktop + needed > brkend) {
        error(nostack);
    }
}

void
stak_init()
{
    stak_ensure(0);
}

void pushstak(c)
unsigned char c;
{
    if (staktop >= brkend)
        stak_ensure(1);
    *staktop++ = c;
}

unsigned char *
getstak(asize)			/* allocate requested stack */
int	asize;
{
	register unsigned char	*oldstak;
	register int	size;

	size = round(asize, BYTESPERWORD);
    stak_ensure(size);

	oldstak = stakbot;
	staktop = stakbot += size;
	return(oldstak);
}

/*
 * set up stack for local use
 * should be followed by `endstak'
 */
unsigned char *
locstak()
{
    stak_ensure(BRKINCR);

	return(stakbot);
}

unsigned char *
savstak()
{
	assert(staktop == stakbot);
	return(stakbot);
}

unsigned char *
endstak(argp)		/* tidy up after `locstak' */
register unsigned char	*argp;
{
	register unsigned char	*oldstak;

	*argp++ = 0;
	oldstak = stakbot;
	stakbot = staktop = (unsigned char *)round(argp, BYTESPERWORD);
	return(oldstak);
}

tdystak(x)		/* try to bring stack back to x */
register unsigned char	*x;
{
	stak_ensure(0);
	while ((unsigned char *)stakbsy > x)
	{
		struct blk *t = stakbsy;
		stakbsy = stakbsy->word;
		sh_free(t);
	}
	staktop = stakbot = (unsigned char *)max((intptr_t)x, (intptr_t)stakbas);
	rmtemp(x);
}

stakchk()
{
}

unsigned char *
cpystak(x)
unsigned char	*x;
{
	return(endstak(movstr(x, locstak())));
}
