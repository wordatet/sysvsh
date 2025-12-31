/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:setbrk.c	1.8.3.1"

/*
 *	UNIX shell
 */

#include	"defs.h"
#include    <stdlib.h>

unsigned char*
setbrk(incr)
int incr;
{
    /* 
     * In a malloc-based shell, setbrk is mostly obsolete.
     * We'll use sh_alloc to satisfy callers, but they really 
     * should be refactored.
     */
	unsigned char *a = (unsigned char *)malloc(incr);
    if (!a)
        error(nospace);

	brkend = a + incr;
	return(a);
}
