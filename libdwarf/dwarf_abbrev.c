/*
    dwarf_abbrev.c

    $Revision: 1.19 $		$Date: 1999/07/21 21:29:38 $
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
#include "dwarf_abbrev.h"

int
dwarf_get_abbrev (
    Dwarf_Debug		dbg,
    Dwarf_Unsigned	offset,
    Dwarf_Abbrev        *returned_abbrev,
    Dwarf_Unsigned	*length,
    Dwarf_Unsigned	*abbr_count,
    Dwarf_Error		*error
)
{
    Dwarf_Small		*abbrev_ptr;
    Dwarf_Small		*abbrev_section_end;
    Dwarf_Half		attr;
    Dwarf_Half		attr_form;
    Dwarf_Abbrev	ret_abbrev;
    Dwarf_Unsigned     labbr_count = 0;
    Dwarf_Unsigned      utmp;


    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL); 
	return(DW_DLV_ERROR);
    }

    if (offset >= dbg->de_debug_abbrev_size)  {
	return(DW_DLV_NO_ENTRY);
    }

    ret_abbrev = (Dwarf_Abbrev)_dwarf_get_alloc(dbg, DW_DLA_ABBREV, 1);
    if (ret_abbrev == NULL) {
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL); 
        return(DW_DLV_ERROR);
    }
    ret_abbrev->ab_dbg = dbg;
    if(returned_abbrev == 0 || abbr_count == 0 ) {
	_dwarf_error(dbg, error, DW_DLE_DEBUG_ABBREV_NULL);
        return(DW_DLV_ERROR);
    }


    *abbr_count = 0;
    if (length != NULL) *length = 1;

    abbrev_ptr = dbg->de_debug_abbrev + offset;
    abbrev_section_end = dbg->de_debug_abbrev + dbg->de_debug_abbrev_size;

    DECODE_LEB128_UWORD(abbrev_ptr, utmp);
    ret_abbrev->ab_code = (Dwarf_Word)utmp;
    if (ret_abbrev->ab_code == 0) {
	*returned_abbrev = ret_abbrev;
        *abbr_count = 0;
	return(DW_DLV_OK);
    }

    DECODE_LEB128_UWORD(abbrev_ptr,  utmp);
    ret_abbrev->ab_tag = utmp;
    ret_abbrev->ab_has_child = *(abbrev_ptr++);
    ret_abbrev->ab_abbrev_ptr = abbrev_ptr;

    do {
	Dwarf_Unsigned utmp2;
        DECODE_LEB128_UWORD(abbrev_ptr, utmp2)
	attr = (Dwarf_Half)utmp2;
        DECODE_LEB128_UWORD(abbrev_ptr, utmp2)
	attr_form = (Dwarf_Half)utmp2;

	if ( attr != 0) (labbr_count)++;

    } while (abbrev_ptr < abbrev_section_end && (attr != 0 || attr_form != 0));

    if (abbrev_ptr > abbrev_section_end) {
	_dwarf_error(dbg, error, DW_DLE_ABBREV_DECODE_ERROR); 
	return(DW_DLV_ERROR);
    }

    if (length != NULL) 
	*length = abbrev_ptr - dbg->de_debug_abbrev - offset;

    *returned_abbrev = ret_abbrev;
    *abbr_count = labbr_count;
    return(DW_DLV_OK);
}

int
dwarf_get_abbrev_code (
    Dwarf_Abbrev	abbrev,
    Dwarf_Unsigned         *returned_code,
    Dwarf_Error		*error
)
{
    if (abbrev == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_ABBREV_NULL); 
	return(DW_DLV_ERROR);
    }
    
    *returned_code = abbrev->ab_code;
    return(DW_DLV_OK);
}

int
dwarf_get_abbrev_tag (
    Dwarf_Abbrev	abbrev,
    Dwarf_Half         *returned_tag,
    Dwarf_Error		*error
)
{
    if (abbrev == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_ABBREV_NULL); 
	return(DW_DLV_ERROR);
    }
    
    *returned_tag = abbrev->ab_tag;
    return(DW_DLV_OK);
}


int
dwarf_get_abbrev_children_flag (
    Dwarf_Abbrev	abbrev,
    Dwarf_Signed       *returned_flag,
    Dwarf_Error		*error
)
{
    if (abbrev == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_ABBREV_NULL); 
	return(DW_DLV_ERROR);
    }
    
    *returned_flag = abbrev->ab_has_child;
    return(DW_DLV_OK);
}


int
dwarf_get_abbrev_entry (
    Dwarf_Abbrev	abbrev,
    Dwarf_Signed	index,
    Dwarf_Half         *returned_attr_num,
    Dwarf_Signed	*form,
    Dwarf_Off		*offset,
    Dwarf_Error		*error
)
{
    Dwarf_Byte_Ptr	abbrev_ptr;
    Dwarf_Byte_Ptr	abbrev_end;
    Dwarf_Byte_Ptr	mark_abbrev_ptr;
    Dwarf_Half		attr;
    Dwarf_Half		attr_form;

    if (index < 0)
	return(DW_DLV_NO_ENTRY);

    if (abbrev == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DWARF_ABBREV_NULL); 
	return(DW_DLV_ERROR);
    }
    
    if (abbrev->ab_code == 0) {
	return(DW_DLV_NO_ENTRY);
    }

    if (abbrev->ab_dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL);
	return(DW_DLV_ERROR);
    }

    abbrev_ptr = abbrev->ab_abbrev_ptr;
    abbrev_end = 
	abbrev->ab_dbg->de_debug_abbrev + abbrev->ab_dbg->de_debug_abbrev_size;

    for (attr = 1 ,attr_form = 1; 
	index >= 0 && abbrev_ptr < abbrev_end && (attr != 0 || attr_form != 0); 
	index--) {
	Dwarf_Unsigned utmp4;
	mark_abbrev_ptr = abbrev_ptr;
        DECODE_LEB128_UWORD(abbrev_ptr, utmp4)
	attr = (Dwarf_Half)utmp4;
        DECODE_LEB128_UWORD(abbrev_ptr, utmp4)
	attr_form = (Dwarf_Half)utmp4;
    } 

    if (abbrev_ptr >= abbrev_end) {
	_dwarf_error(abbrev->ab_dbg, error, DW_DLE_ABBREV_DECODE_ERROR);
	return(DW_DLV_ERROR);
    }

    if (index >=  0) {
	return(DW_DLV_NO_ENTRY);
    }

    if (form != NULL) *form = attr_form;
    if (offset != NULL) 
	*offset = mark_abbrev_ptr - abbrev->ab_dbg->de_debug_abbrev;

    *returned_attr_num = (attr);
    return DW_DLV_OK;
}
