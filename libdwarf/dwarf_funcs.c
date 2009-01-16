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
#include "dwarf_funcs.h"
#include "dwarf_global.h"

int
dwarf_get_funcs (
    Dwarf_Debug		dbg,
    Dwarf_Func		**funcs,
    Dwarf_Signed	* ret_func_count,
    Dwarf_Error		*error
)
{
       return _dwarf_internal_get_pubnames_like_data(
                dbg,
		dbg->de_debug_funcnames,
		dbg->de_debug_funcnames_size,
                (Dwarf_Global **)funcs, /* type punning,
                        Dwarf_Type is never a completed  type */
                ret_func_count,
                error,
                DW_DLA_FUNC_CONTEXT,
                DW_DLE_DEBUG_FUNCNAMES_LENGTH_BAD,
                DW_DLE_DEBUG_FUNCNAMES_VERSION_ERROR);

}


int
dwarf_funcname (
    Dwarf_Func		func_in,
    char         **     ret_name,
    Dwarf_Error		*error
)
{
    Dwarf_Global func = (Dwarf_Global)func_in;
    if (func == NULL)
	{_dwarf_error(NULL, error, DW_DLE_FUNC_NULL); return(DW_DLV_ERROR);}

    *ret_name = (char *)(func->gl_name);
    return DW_DLV_OK;
}

int
dwarf_func_die_offset (
    Dwarf_Func		func_in,
    Dwarf_Off      *    return_offset,
    Dwarf_Error		*error
)
{
    Dwarf_Global func = (Dwarf_Global)func_in;
    return dwarf_global_die_offset(func,
			return_offset,	error);
}


int
dwarf_func_cu_offset (
    Dwarf_Func		func_in,
    Dwarf_Off      *    return_offset,
    Dwarf_Error		*error
)
{
    Dwarf_Global func = (Dwarf_Global)func_in;
    return dwarf_global_cu_offset(func,
		return_offset, error);
}


int
dwarf_func_name_offsets (
    Dwarf_Func		func_in,
    char **              ret_func_name,
    Dwarf_Off		*die_offset,
    Dwarf_Off		*cu_die_offset,
    Dwarf_Error		*error
)
{
    Dwarf_Global func = (Dwarf_Global)func_in;
    return dwarf_global_name_offsets(func,
		ret_func_name,
		die_offset,
		cu_die_offset,
		error);
}
