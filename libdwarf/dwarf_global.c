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
#include "dwarf_global.h"

int
dwarf_get_globals (
    Dwarf_Debug		dbg,
    Dwarf_Global	**globals,
    Dwarf_Signed       *return_count,
    Dwarf_Error		*error
)
{


	return _dwarf_internal_get_pubnames_like_data(
		dbg,
		dbg->de_debug_pubnames,
		dbg->de_debug_pubnames_size,
		globals,
		return_count,
		error,
		DW_DLA_GLOBAL_CONTEXT,
		DW_DLE_PUBNAMES_LENGTH_BAD,
		DW_DLE_PUBNAMES_VERSION_ERROR);

}


/* Sweeps the complete  section. 
*/
int
_dwarf_internal_get_pubnames_like_data(
    Dwarf_Debug         dbg,
    Dwarf_Small         *section_data_ptr,
    Dwarf_Unsigned      section_length,
    Dwarf_Global        **globals,
    Dwarf_Signed       *return_count,
    Dwarf_Error         *error,
    int                 allocation_code,
    int                 length_err_num,
    int                 version_err_num
)

{


    Dwarf_Small			*pubnames_like_ptr;



	/* 
	    Points to the context for the current set of global names,
	    and contains information to identify the compilation-unit
	    that the set refers to.
	*/
    Dwarf_Global_Context	pubnames_context;

    Dwarf_Half			version;

	/* 
	    Offset from the start of compilation-unit 
	    for the current global.
	*/
    Dwarf_Off			die_offset_in_cu;

    Dwarf_Unsigned		global_count = 0;

	/* Points to the current global read. */
    Dwarf_Global		global;

	/* 
	    Used to chain the Dwarf_Global_s structs for creating
	    contiguous list of pointers to the structs.
	*/
    Dwarf_Chain			curr_chain, prev_chain, head_chain = NULL;

	/* Points to contiguous block of Dwarf_Global's to be returned. */
    Dwarf_Global		*ret_globals;

	/* Temporary counter. */
    Dwarf_Unsigned		i;

    


    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL); 
	return(DW_DLV_ERROR);
    }

    if (section_data_ptr == NULL) {
	return(DW_DLV_NO_ENTRY);
    }

    pubnames_like_ptr = section_data_ptr;
    do {
        Dwarf_Unsigned length;
	int local_extension_size;
	int local_length_size;

	pubnames_context = (Dwarf_Global_Context)
	    _dwarf_get_alloc(dbg, allocation_code, 1);
	if (pubnames_context == NULL) {
	    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL); 
	    return(DW_DLV_ERROR);
	}
	/* READ_AREA_LENGTH updates pubnames_like_ptr for consumed bytes*/
	READ_AREA_LENGTH(dbg,length,Dwarf_Unsigned,
            pubnames_like_ptr,local_length_size,local_extension_size);
        pubnames_context->pu_length_size = local_length_size;
        pubnames_context->pu_extension_size = local_extension_size;
        pubnames_context->pu_dbg = dbg;


	READ_UNALIGNED(dbg,version, Dwarf_Half,
		pubnames_like_ptr, sizeof(Dwarf_Half));
	pubnames_like_ptr += sizeof(Dwarf_Half);
	if (version != CURRENT_VERSION_STAMP) {
	    _dwarf_error(dbg, error, version_err_num);
	    return(DW_DLV_ERROR);
	}

	/* offset of CU header in debug section */
	READ_UNALIGNED(dbg,pubnames_context->pu_offset_of_cu_header, Dwarf_Off,
		pubnames_like_ptr, 
	    pubnames_context->pu_length_size);
	pubnames_like_ptr += pubnames_context->pu_length_size;

	
	READ_UNALIGNED(dbg,pubnames_context->pu_info_length,Dwarf_Unsigned,
			 pubnames_like_ptr, 
	    pubnames_context->pu_length_size);
	pubnames_like_ptr += pubnames_context->pu_length_size;

	if (pubnames_like_ptr > (section_data_ptr +
	    section_length)) {
	    _dwarf_error(dbg, error, length_err_num);
	    return(DW_DLV_ERROR);
	}

	/* read initial  offset (of DIE within CU) of a pubname,
           final entry is not a pair, just a zero offset */
	READ_UNALIGNED(dbg,die_offset_in_cu,Dwarf_Off, 
			pubnames_like_ptr, pubnames_context->pu_length_size);
	pubnames_like_ptr += pubnames_context->pu_length_size;

	/* loop thru pairs. DIE off with CU followed by string  */
	while (die_offset_in_cu != 0) {

	    /* Already read offset, pubnames_like_ptr now points to
		the string
	    */
            global = (Dwarf_Global)_dwarf_get_alloc(dbg, DW_DLA_GLOBAL, 1);
	    if (global == NULL) {
		_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL); 
		return(DW_DLV_ERROR);
	    }
	    global_count++;

	    global->gl_context = pubnames_context;

            global->gl_named_die_offset_within_cu = die_offset_in_cu;

	    global->gl_name = pubnames_like_ptr;
	    pubnames_like_ptr = pubnames_like_ptr + 
		strlen((char *)pubnames_like_ptr) + 1;


	    /* finish off current entry chain */
	    curr_chain = (Dwarf_Chain)_dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
	    if (curr_chain == NULL) {
		_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
		return(DW_DLV_ERROR);
	    }

		/* Put current global on singly_linked list. */
	    curr_chain->ch_item = (Dwarf_Global)global;
	    if (head_chain == NULL)
		head_chain = prev_chain = curr_chain;
	    else {
		prev_chain->ch_next = curr_chain;
		prev_chain = curr_chain;
	    }

	    /* read offset for the *next* entry */
	    READ_UNALIGNED(dbg,die_offset_in_cu, Dwarf_Off,
		pubnames_like_ptr, pubnames_context->pu_length_size);
	
	    pubnames_like_ptr +=  pubnames_context->pu_length_size;
	    if (pubnames_like_ptr > (section_data_ptr + 
		section_length)) {
		_dwarf_error(dbg, error, length_err_num);
		return(DW_DLV_ERROR);
	    }
	}

    } while 
	(pubnames_like_ptr < (section_data_ptr + section_length));
    
	/* Points to contiguous block of Dwarf_Global's. */
    ret_globals = (Dwarf_Global *)
	_dwarf_get_alloc(dbg, DW_DLA_LIST, global_count);
    if (ret_globals == NULL) 
	{_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL); return(DW_DLV_ERROR);}

	/* 
	    Store pointers to Dwarf_Global_s structs in
	    contiguous block, and deallocate the chain.
	*/
    curr_chain = head_chain;
    for (i = 0; i < global_count; i++) {
	*(ret_globals + i) = curr_chain->ch_item;
	prev_chain = curr_chain;
	curr_chain = curr_chain->ch_next;
	dwarf_dealloc(dbg, prev_chain, DW_DLA_CHAIN);
    }

    *globals = ret_globals;
    *return_count = (global_count);
    return DW_DLV_OK;
}

/*
	Given a pubnames entry (or other like section entry)
	return thru the ret_name pointer
	a pointer to the string which is the entry name.
	
*/
int
dwarf_globname (
    Dwarf_Global	glob,
    char **             ret_name,
    Dwarf_Error		*error
)
{
    if (glob == NULL)
	{_dwarf_error(NULL, error, DW_DLE_GLOBAL_NULL); return(DW_DLV_ERROR);}

    *ret_name = (char *)(glob->gl_name);
    return DW_DLV_OK;
}


/*
	Given a pubnames entry (or other like section entry)
	return thru the ret_off pointer the
	global offset of the DIE for this entry.
	The global offset is the offset within the .debug_info
	section as a whole.
*/
int
dwarf_global_die_offset (
    Dwarf_Global	global,
    Dwarf_Off          *ret_off,
    Dwarf_Error		*error
)
{
    if (global == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_NULL); 
	return(DW_DLV_ERROR);
    }

    if (global->gl_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_CONTEXT_NULL);
	return(DW_DLV_ERROR);
    }

    *ret_off = (global->gl_named_die_offset_within_cu + 
		global->gl_context->pu_offset_of_cu_header);
    return DW_DLV_OK;
}

/*
	Given a pubnames entry (or other like section entry)
	return thru the ret_off pointer the
	offset of the compilation unit header of the
        compilation unit the  entry's DIE is part of.
*/
int
dwarf_global_cu_offset (
    Dwarf_Global	global,
    Dwarf_Off          *ret_off,
    Dwarf_Error		*error
)
{
    if (global == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_NULL); 
	return(DW_DLV_ERROR);
    }

    if (global->gl_context == NULL) {
	_dwarf_error(NULL, error, DW_DLE_GLOBAL_CONTEXT_NULL);
	return(DW_DLV_ERROR);
    }

    *ret_off = (global->gl_context->pu_offset_of_cu_header);
    return DW_DLV_OK;
}

/*
  Give back the pubnames entry (or any other like section)
  name, symbol DIE offset, and the cu-DIE offset.
*/
int
dwarf_global_name_offsets (
    Dwarf_Global	global,
    char               **ret_name,
    Dwarf_Off		*die_offset,
    Dwarf_Off		*cu_die_offset,
    Dwarf_Error		*error
)
{
    Dwarf_Global_Context con;
    Dwarf_Off off;
    if (global == NULL)
	{_dwarf_error(NULL, error, DW_DLE_GLOBAL_NULL); return(DW_DLV_ERROR);}

    con = global->gl_context;
    if (con == NULL) 
	{_dwarf_error(NULL, error, DW_DLE_GLOBAL_CONTEXT_NULL); 
	return(DW_DLV_ERROR);}

    off = con->pu_offset_of_cu_header;
    if (die_offset != NULL) {
	*die_offset = global->gl_named_die_offset_within_cu + off;
    }

    if (cu_die_offset != NULL) {
	*cu_die_offset = off + 
          _dwarf_length_of_cu_header(global->gl_context->pu_dbg, off);
    }

    *ret_name = (char *)global->gl_name;
    return DW_DLV_OK;
}

