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

	pro_finish.c 
	$Revision: 1.9 $    $Date: 1999/06/22 16:33:36 $    
	$Source: /isms/cmplrs.src/osprey1.0/libdwarf/RCS/pro_finish.c,v $

	exit function
*/

#include "config.h"
#include "libdwarfdefs.h"
#include "pro_incl.h"

/*---------------------------------------------------------------
	This routine deallocates all memory, and does some 
	finishing up
-----------------------------------------------------------------*/
/*ARGSUSED*/
Dwarf_Unsigned
dwarf_producer_finish(Dwarf_P_Debug dbg, Dwarf_Error *error)
{
	if(dbg->de_version_magic_number != PRO_VERSION_MAGIC) {
           DWARF_P_DBG_ERROR(dbg, DW_DLE_IA, DW_DLV_NOCOUNT);
        }

	dwarf_p_dealloc((void *)dbg, 0);
	return 0;
}
