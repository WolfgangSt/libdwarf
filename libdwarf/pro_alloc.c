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


	This is the allocator and deallocator for
	dwarf producer.

	Part of this interface is defined in the public interface.
	Part is hidden in libdwarf.

	Goals: 
		Fast allocation
	 	no leakage on dwarf_finish() kinds of activities
		match libdwarf documented semantics and space
			rules
		For producer, when dwarf_transform_to_disk_form()
			is about finished, wants
		 	all trees to disappear leaving only
			byte streams.

	Producer special case:
		After transform_to_disk_form is called we want
		to have freed all die, attributes etc and just
		leave the transformed byte streams and whatever
		else is needed to support them.
		So we invent a special free operation to do this.

	Issue: What to do if we discover problems in the arena when
		deallocating?  These return nothing....

	We will allocate space in blocks attached to a Dwarf_Debug,
        or, if no such thing present, to a special chain.
        We can clear the special chain when the last Dwarf_Debug
	is freed.

	This version is radically incomplete and meets
	none of the goals above.
	Hopefully this dreadfully incomplete implementation
        is not a problem for producer clients.

	pro_alloc.c
	$Revision: 1.6 $   $Date: 1999/06/22 16:33:34 $
	

*/

#include "config.h"
#include "dwarf_incl.h"
#include <stdlib.h>
#ifdef HAVE_BSTRING_H
#include <bstring.h>
#endif

/*
	The allocator wants to know which region
	this is to be in so it can allocate the new space
	with respect to the right region.
*/
/*ARGSUSED*/
Dwarf_Ptr _dwarf_p_get_alloc (
    Dwarf_P_Debug 	dbg,
    Dwarf_Unsigned 	size
)
{
    void *sp;

    sp = malloc(size);
    bzero(sp,(int)size);
    return sp;
}


/*ARGSUSED*/
void dwarf_p_dealloc(void *space, Dwarf_Unsigned typ)
{
	free(space);
	return;
}


/* Essentially a stub for now. */
/*ARGSUSED*/
void
_dwarf_p_dealloc (
    Dwarf_P_Debug       dbg,
    Dwarf_Small         *ptr
)
{
    dwarf_p_dealloc(ptr,DW_DLA_STRING);
}
