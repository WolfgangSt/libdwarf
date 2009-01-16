/*
Copyright (c) 1994-9 Silicon Graphics, Inc.

    Permission to use, copy, modify, distribute, and sell this software and 
    its documentation for any purpose is hereby granted without fee, provided
    that (i) the above copyright notice and this permission notice appear in
    all copies of the software and related documentation, and (ii) the name
    "Silicon Graphics" or any other trademark of Silicon Graphics, Inc.  
    may not be used in any advertising or publicity relating to the software
    without the specific, prior written permission of Silicon Graphics, Inc.

    THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
    WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  

    IN NO EVENT SHALL SILICON GRAPHICS, INC. BE LIABLE FOR ANY SPECIAL, 
    INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
    OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
    WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
    LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
    OF THIS SOFTWARE.


	pro_error.c



	$Revision: 1.4 $ $Date: 1999/06/22 16:33:35 $

*/

#include "config.h"
#include "libdwarfdefs.h"
#ifdef HAVE_ELF_H
#include <elf.h>
#endif
#ifdef HAVE_LIBELF_H
#include <libelf.h>
#else
#ifdef HAVE_LIBELF_LIBELF_H
#include <libelf/libelf.h>
#endif
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include "pro_incl.h"

extern char* _dwarf_errmsgs[];

/* 
    This function performs error handling as described in the 
    libdwarf consumer document section 3.  Dbg is the Dwarf_P_debug
    structure being processed.  Error is a pointer to the pointer
    to the error descriptor that will be returned.  Errval is an
    error code listed in dwarf_error.h.
*/
void
_dwarf_p_error (
    Dwarf_P_Debug     	dbg,
    Dwarf_Error     	*error,
    Dwarf_Word      	errval
)
{
    Dwarf_Error     errptr;

    /* Allow NULL dbg on entry, since sometimes that can happen and
       we want to report the upper-level error, not this one.
    */
    if ((Dwarf_Sword)errval < 0)
	printf("ERROR VALUE: %d - %s\n",errval, _dwarf_errmsgs[-errval-1]);
    if (error != NULL) {
        errptr = (Dwarf_Error)
	    _dwarf_p_get_alloc(dbg, sizeof(struct Dwarf_Error_s));
        if (errptr == NULL) {
            fprintf(stderr,"Could not allocate Dwarf_Error structure\n");
            abort();
        }
        errptr->er_errval = (Dwarf_Sword)errval;
        *error = errptr;
        return;
    }

    if (dbg != NULL && dbg->de_errhand != NULL) {
        errptr = (Dwarf_Error)
	    _dwarf_p_get_alloc(dbg, sizeof(struct Dwarf_Error_s));
        if (errptr == NULL) {
            fprintf(stderr,"Could not allocate Dwarf_Error structure\n");
            abort();
        }
        errptr->er_errval = (Dwarf_Sword)errval;
        dbg->de_errhand(errptr,dbg->de_errarg);
        return;
    }

    abort();
}

