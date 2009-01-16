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

*/

#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include "dwarf_weaks.h"
#include "dwarf_global.h"

int
dwarf_get_weaks (
    Dwarf_Debug		dbg,
    Dwarf_Weak		**weaks,
    Dwarf_Signed        *ret_weak_count,
    Dwarf_Error		*error
)
{
       return _dwarf_internal_get_pubnames_like_data(
                dbg,
		dbg->de_debug_weaknames,
		dbg->de_debug_weaknames_size,
                (Dwarf_Global **)weaks, /* type punning,
                        Dwarf_Type is never a completed  type */
                ret_weak_count,
                error,
                DW_DLA_WEAK_CONTEXT,
                DW_DLE_DEBUG_WEAKNAMES_LENGTH_BAD,
                DW_DLE_DEBUG_WEAKNAMES_VERSION_ERROR);

}



int
dwarf_weakname (
    Dwarf_Weak		weak_in,
    char **	     ret_name,
    Dwarf_Error		*error
)
{
    Dwarf_Global weak = (Dwarf_Global)weak_in;
    if (weak == NULL) {
	_dwarf_error(NULL, error, DW_DLE_WEAK_NULL); 
	return(DW_DLV_ERROR);
    }
    *ret_name = (char *)(weak->gl_name);
    return DW_DLV_OK;
}


int
dwarf_weak_die_offset (
    Dwarf_Weak		weak_in,
    Dwarf_Off          *weak_off,
    Dwarf_Error		*error
)
{
    Dwarf_Global weak = (Dwarf_Global)weak_in;
    return dwarf_global_die_offset(weak,
		weak_off,error);
}


int
dwarf_weak_cu_offset (
    Dwarf_Weak		weak_in,
    Dwarf_Off          *weak_off,
    Dwarf_Error		*error
)
{
    Dwarf_Global weak = (Dwarf_Global)weak_in;
    return dwarf_global_cu_offset(weak,
		weak_off,
		error);
}


int
dwarf_weak_name_offsets (
    Dwarf_Weak		weak_in,
    char    **          weak_name,
    Dwarf_Off		*die_offset,
    Dwarf_Off		*cu_offset,
    Dwarf_Error		*error
)
{
    Dwarf_Global weak = (Dwarf_Global)weak_in;
    return dwarf_global_name_offsets(
		weak,
		weak_name,
		die_offset,
		cu_offset,
		error);
}
