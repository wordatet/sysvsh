/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:blok.c	1.9.4.1"
/*
 *	UNIX shell
 */

#include	"defs.h"
#include <stdlib.h>

#undef alloc
#undef free


/*
 *	storage allocator
 *	(standard malloc wrapper)
 */

void *
sh_alloc(nbytes)
	size_t nbytes;
{
	void *p = malloc(nbytes);
	if (!p)
		error(nospace);
	return p;
}

void
sh_free(ap)
	void *ap;
{
	if (ap)
		free(ap);
}

void addblok() {}

/* stubs for old internal functions if needed, but better to remove callers */
void chkbptr() {}
void chkmem() {}
