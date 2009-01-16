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

int
dwarf_get_str (
    Dwarf_Debug		dbg,
    Dwarf_Off		offset,
    char		**string,
    Dwarf_Signed	*returned_str_len,
    Dwarf_Error		*error
)
{

    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL);
	return(DW_DLV_ERROR);
    }

    if (dbg->de_debug_str == NULL) {
	return(DW_DLV_NO_ENTRY);
    }

    if (offset >= dbg->de_debug_str_size) {
	_dwarf_error(dbg, error, DW_DLE_DEBUG_STR_OFFSET_BAD);
	return(DW_DLV_ERROR);
    }

    if (string == NULL) {
	_dwarf_error(dbg, error, DW_DLE_STRING_PTR_NULL);
	return(DW_DLV_ERROR);
    }
    *string = (char *)dbg->de_debug_str + offset;

    *returned_str_len = (strlen(*string));
    return DW_DLV_OK;
}
