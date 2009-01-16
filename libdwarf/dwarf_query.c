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
#include "dwarf_die_deliv.h"

int dwarf_get_address_size(Dwarf_Debug dbg,
        Dwarf_Half *ret_addr_size,
        Dwarf_Error *error)
{
	Dwarf_Half address_size;
	if(dbg == 0) {
	    _dwarf_error(NULL,error,DW_DLE_DBG_NULL);
            return(DW_DLV_ERROR);
	}
	/* length size same as address size */
	address_size = dbg->de_pointer_size;
	*ret_addr_size = address_size;
	return DW_DLV_OK;
}

int
dwarf_dieoffset (
    Dwarf_Die       die,
    Dwarf_Off      *ret_offset,
    Dwarf_Error     *error
)
{
    CHECK_DIE(die, DW_DLV_ERROR)

    *ret_offset = (die->di_debug_info_ptr - 
	die->di_cu_context->cc_dbg->de_debug_info);
    return DW_DLV_OK;
}


/*
    This function returns the offset of
    the die relative to the start of its
    compilation-unit rather than .debug_info.
    Returns DW_DLV_ERROR on error.
*/
int
dwarf_die_CU_offset (
    Dwarf_Die	    die,
    Dwarf_Off       *cu_off,
    Dwarf_Error	    *error
)
{
    Dwarf_CU_Context	cu_context;

    CHECK_DIE(die, DW_DLV_ERROR)
    cu_context = die->di_cu_context;

    *cu_off = (die->di_debug_info_ptr - cu_context->cc_dbg->de_debug_info -
	cu_context->cc_debug_info_offset);
    return DW_DLV_OK;
}


int
dwarf_tag (
    Dwarf_Die       die,
    Dwarf_Half      *tag,
    Dwarf_Error     *error
)
{
    CHECK_DIE(die, DW_DLV_ERROR)

  
    *tag =(die->di_abbrev_list->ab_tag);
    return DW_DLV_OK;
}


int
dwarf_attrlist (
    Dwarf_Die         die,
    Dwarf_Attribute   **attrbuf,
    Dwarf_Signed      *attrcnt,
    Dwarf_Error       *error
)
{
    Dwarf_Word              attr_count = 0;
    Dwarf_Word	            i;
    Dwarf_Half              attr;
    Dwarf_Half		    attr_form;
    Dwarf_Byte_Ptr          abbrev_ptr;
    Dwarf_Abbrev_List	    abbrev_list;
    Dwarf_Attribute	    new_attr;
    Dwarf_Attribute	    head_attr = NULL, curr_attr;
    Dwarf_Attribute	    *attr_ptr;
    Dwarf_Debug		    dbg;
    Dwarf_Byte_Ptr	    info_ptr;

    CHECK_DIE(die, DW_DLV_ERROR)
    dbg = die->di_cu_context->cc_dbg;

    abbrev_list = _dwarf_get_abbrev_for_code(die->di_cu_context, 
	die->di_abbrev_list->ab_code);
    if (abbrev_list == NULL) {
	_dwarf_error(dbg, error, DW_DLE_DIE_ABBREV_BAD); 
	return(DW_DLV_ERROR);
    }
    abbrev_ptr = abbrev_list->ab_abbrev_ptr;

    info_ptr = die->di_debug_info_ptr;
    SKIP_LEB128_WORD(info_ptr)

    do {
	Dwarf_Unsigned utmp2;
        DECODE_LEB128_UWORD(abbrev_ptr, utmp2)
	attr = (Dwarf_Half)utmp2;
        DECODE_LEB128_UWORD(abbrev_ptr, utmp2)
	attr_form = (Dwarf_Half)utmp2;

	if (attr != 0) {
            new_attr = (Dwarf_Attribute)_dwarf_get_alloc(dbg, DW_DLA_ATTR, 1);
	    if (new_attr == NULL) {
	        _dwarf_error(dbg,error,DW_DLE_ALLOC_FAIL);
	        return(DW_DLV_ERROR);
	    }

	    new_attr->ar_attribute = attr;
	    new_attr->ar_attribute_form = attr_form;
	    new_attr->ar_cu_context = die->di_cu_context;
	    new_attr->ar_debug_info_ptr = info_ptr;

	    info_ptr += _dwarf_get_size_of_val(dbg, attr_form, info_ptr,
			die->di_cu_context->cc_length_size);

	    if (head_attr == NULL)
	        head_attr = curr_attr = new_attr;
	    else {
	        curr_attr->ar_next = new_attr;
	        curr_attr = new_attr;
	    }
            attr_count++;
	}
    } while (attr != 0 || attr_form != 0);

    if (attr_count == 0) {
	*attrbuf = NULL;
	*attrcnt = 0;
	return(DW_DLV_NO_ENTRY);
    }

    attr_ptr = (Dwarf_Attribute *)
	_dwarf_get_alloc(dbg, DW_DLA_LIST, attr_count);
    if (attr_ptr == NULL) {
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return(DW_DLV_ERROR);
    }

    curr_attr = head_attr;
    for (i = 0; i < attr_count; i++) {
	*(attr_ptr + i) = curr_attr;
	curr_attr = curr_attr->ar_next;
    }

    *attrbuf = attr_ptr;
    *attrcnt = attr_count;
    return(DW_DLV_OK);
}


/*
    This function takes a die, and an attr, and returns
    a pointer to the start of the value of that attr in
    the given die in the .debug_info section.  The form
    is returned in *attr_form.

    Returns NULL on error, or if attr is not found.
    However, *attr_form is 0 on error, and positive 
    otherwise.
*/
static Dwarf_Byte_Ptr
_dwarf_get_value_ptr (
    Dwarf_Die		die,
    Dwarf_Half		attr,
    Dwarf_Half		*attr_form
)
{
    Dwarf_Byte_Ptr          abbrev_ptr;
    Dwarf_Abbrev_List	    abbrev_list;
    Dwarf_Half	            curr_attr;
    Dwarf_Half	            curr_attr_form;
    Dwarf_Byte_Ptr	    info_ptr;

    abbrev_list = _dwarf_get_abbrev_for_code(die->di_cu_context, 
	die->di_abbrev_list->ab_code);
    if (abbrev_list == NULL) {
	*attr_form = 0;
	return(NULL);
    }
    abbrev_ptr = abbrev_list->ab_abbrev_ptr;

    info_ptr = die->di_debug_info_ptr;
    SKIP_LEB128_WORD(info_ptr)

    do {
	Dwarf_Unsigned utmp3;
        DECODE_LEB128_UWORD(abbrev_ptr, utmp3)
	curr_attr = (Dwarf_Half)utmp3;
	DECODE_LEB128_UWORD(abbrev_ptr, utmp3)
	curr_attr_form = (Dwarf_Half)utmp3;

        if (curr_attr == attr) {
	    *attr_form = curr_attr_form;
	    return(info_ptr);
	}

        info_ptr += _dwarf_get_size_of_val(die->di_cu_context->cc_dbg, 
            curr_attr_form, info_ptr,
		die->di_cu_context->cc_length_size);
    } while (curr_attr != 0 || curr_attr_form != 0);

    *attr_form = 1;
    return(NULL);
}


int
dwarf_diename (
    Dwarf_Die       die,
    char     **     ret_name,
    Dwarf_Error     *error
)
{
    Dwarf_Half	            attr_form;
    Dwarf_Debug		    dbg;
    Dwarf_Byte_Ptr	    info_ptr;
    Dwarf_Unsigned          string_offset;

    CHECK_DIE(die, DW_DLV_ERROR)

    info_ptr = _dwarf_get_value_ptr(die, DW_AT_name, &attr_form);
    if (info_ptr == NULL) {
	if (attr_form == 0)  {
	    _dwarf_error(die->di_cu_context->cc_dbg, error, DW_DLE_DIE_BAD);
	    return(DW_DLV_ERROR);
	}
	return DW_DLV_NO_ENTRY;
    }

    if (attr_form == DW_FORM_string){
	 *ret_name = (char *)(info_ptr);
	 return DW_DLV_OK;
    }

    dbg = die->di_cu_context->cc_dbg;
    if (attr_form != DW_FORM_strp) {
	_dwarf_error(dbg, error, DW_DLE_ATTR_FORM_BAD);
	return(DW_DLV_ERROR);
    }

    READ_UNALIGNED(dbg,string_offset,Dwarf_Unsigned,
	 info_ptr, dbg->de_length_size);

    if (dbg->de_debug_str == NULL) {
	_dwarf_error(dbg, error, DW_DLE_DEBUG_STR_NULL); 
	return(DW_DLV_ERROR);
    }

    if (string_offset >= dbg->de_debug_str_size) {
	_dwarf_error(dbg, error, DW_DLE_STRING_OFFSET_BAD); 
	return(DW_DLV_ERROR);
    }

    *ret_name = (char *)(dbg->de_debug_str + string_offset);
    return DW_DLV_OK;
}


int
dwarf_hasattr (
    Dwarf_Die       die,
    Dwarf_Half      attr,
    Dwarf_Bool      *return_bool,
    Dwarf_Error     *error
)
{
    Dwarf_Half	        attr_form;

    CHECK_DIE(die, DW_DLV_ERROR)

    if (_dwarf_get_value_ptr(die, attr, &attr_form) == NULL) {
	if (attr_form == 0)  {
	    _dwarf_error(die->di_cu_context->cc_dbg, error, DW_DLE_DIE_BAD);
	    return(DW_DLV_ERROR);
	}
        *return_bool = false;
	return DW_DLV_OK;
    }

    *return_bool = (true);
    return DW_DLV_OK;
}


int
dwarf_attr (
    Dwarf_Die       die,
    Dwarf_Half      attr,
    Dwarf_Attribute *    ret_attr,
    Dwarf_Error     *error
)
{
    Dwarf_Half              attr_form;
    Dwarf_Attribute         attrib;
    Dwarf_Byte_Ptr	    info_ptr;
    Dwarf_Debug		    dbg;

    CHECK_DIE(die, DW_DLV_ERROR)
    dbg = die->di_cu_context->cc_dbg;

    info_ptr = _dwarf_get_value_ptr(die, attr, &attr_form);
    if (info_ptr == NULL) {
	if (attr_form == 0) {
		_dwarf_error(dbg, error, DW_DLE_DIE_BAD); 
        	return(DW_DLV_ERROR);
	}
	return DW_DLV_NO_ENTRY;
    }

    attrib = (Dwarf_Attribute)_dwarf_get_alloc(dbg, DW_DLA_ATTR, 1);
    if (attrib == NULL) {
	_dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return(DW_DLV_ERROR);
    }

    attrib->ar_attribute = attr;
    attrib->ar_attribute_form = attr_form;
    attrib->ar_cu_context = die->di_cu_context;
    attrib->ar_debug_info_ptr = info_ptr;
    *ret_attr = (attrib);
    return DW_DLV_OK;
}


int
dwarf_lowpc (
    Dwarf_Die		die,
    Dwarf_Addr    *return_addr,
    Dwarf_Error		*error
)
{
    Dwarf_Addr		ret_addr;
    Dwarf_Byte_Ptr	info_ptr;
    Dwarf_Half		attr_form;
    Dwarf_Debug         dbg;

    CHECK_DIE(die, DW_DLV_ERROR)

    dbg = die->di_cu_context->cc_dbg;
    info_ptr = _dwarf_get_value_ptr(die, DW_AT_low_pc, &attr_form);
    if ((info_ptr == NULL && attr_form == 0) ||
	(info_ptr != NULL && attr_form != DW_FORM_addr)) {
	_dwarf_error(dbg, error, DW_DLE_DIE_BAD);
	return(DW_DLV_ERROR);
    }
    if (info_ptr == NULL) {
	return(DW_DLV_NO_ENTRY);
    }
    

    READ_UNALIGNED(dbg,ret_addr,Dwarf_Addr,
	 info_ptr, dbg->de_pointer_size);

    *return_addr = ret_addr;
    return(DW_DLV_OK);
}


int
dwarf_highpc (
    Dwarf_Die		die,
    Dwarf_Addr    *return_addr,
    Dwarf_Error		*error
)
{
    Dwarf_Addr		ret_addr;
    Dwarf_Byte_Ptr	info_ptr;
    Dwarf_Half		attr_form;
    Dwarf_Debug         dbg;

    CHECK_DIE(die, DW_DLV_ERROR)

    dbg = die->di_cu_context->cc_dbg;
    info_ptr = _dwarf_get_value_ptr(die, DW_AT_high_pc, &attr_form);
    if ((info_ptr == NULL && attr_form == 0) ||
	(info_ptr != NULL && attr_form != DW_FORM_addr)) {
	_dwarf_error(dbg, error, DW_DLE_DIE_BAD);
	return(DW_DLV_ERROR);
    }
    if (info_ptr == NULL) {
	return(DW_DLV_NO_ENTRY);
    }

    READ_UNALIGNED(dbg,ret_addr, Dwarf_Addr,
	info_ptr, 
	dbg->de_pointer_size);

    *return_addr = ret_addr;
    return(DW_DLV_OK);
}


/*
    Takes a die, an attribute attr, and checks if attr 
    occurs in die.  Attr is required to be an attribute
    whose form is in the "constant" class.  If attr occurs 
    in die, the value is returned.  
  Returns DW_DLV_OK, DW_DLV_ERROR, or DW_DLV_NO_ENTRY as
    appropriate. Sets the value thru the pointer return_val.
    This function is meant to do all the 
    processing for dwarf_bytesize, dwarf_bitsize, dwarf_bitoffset, 
    and dwarf_srclang.
*/
static int
_dwarf_die_attr_unsigned_constant (
    Dwarf_Die		die,
    Dwarf_Half		attr,
    Dwarf_Unsigned      *return_val,
    Dwarf_Error		*error
)
{
    Dwarf_Byte_Ptr	info_ptr;
    Dwarf_Half		attr_form;
    Dwarf_Unsigned	ret_value;
    Dwarf_Debug         dbg;

    CHECK_DIE(die, DW_DLV_ERROR)

    dbg = die->di_cu_context->cc_dbg;
    info_ptr = _dwarf_get_value_ptr(die, attr, &attr_form);
    if (info_ptr != NULL) {
        switch (attr_form) {

	    case DW_FORM_data1 :
	        *return_val =  (*(Dwarf_Small *)info_ptr);
	        return(DW_DLV_OK);

	    case DW_FORM_data2 : 
	        READ_UNALIGNED(dbg,ret_value,Dwarf_Unsigned,
		 info_ptr, sizeof(Dwarf_Shalf));
		*return_val = ret_value;
	        return(DW_DLV_OK);

	    case DW_FORM_data4 : 
	        READ_UNALIGNED(dbg,ret_value,Dwarf_Unsigned,
		 info_ptr, sizeof(Dwarf_sfixed));
		*return_val = ret_value;
	        return(DW_DLV_OK);

	    case DW_FORM_data8 :
	        READ_UNALIGNED(dbg,ret_value,Dwarf_Unsigned,
		 info_ptr, sizeof(Dwarf_Unsigned));
		*return_val = ret_value;
	        return(DW_DLV_OK);

	    case DW_FORM_udata :
	        *return_val = (_dwarf_decode_u_leb128(info_ptr, NULL));
	        return(DW_DLV_OK);

	    default :
		_dwarf_error(dbg, error, DW_DLE_DIE_BAD);
		return(DW_DLV_ERROR);
        }
    }
    if (attr_form == 0) {
	    _dwarf_error(dbg, error, DW_DLE_DIE_BAD);
	     return(DW_DLV_ERROR);
    }
    return DW_DLV_NO_ENTRY;
}


int
dwarf_bytesize (
    Dwarf_Die		die,
    Dwarf_Unsigned	* ret_size,
    Dwarf_Error		*error
)
{
   Dwarf_Unsigned luns;
   int res=_dwarf_die_attr_unsigned_constant(die, DW_AT_byte_size,&luns,error);
   *ret_size = luns;
   return res;
}


int
dwarf_bitsize (
    Dwarf_Die		die,
    Dwarf_Unsigned     *ret_size,
    Dwarf_Error		*error
)
{
   Dwarf_Unsigned luns;
   int res;
    res = _dwarf_die_attr_unsigned_constant(die, DW_AT_bit_size,&luns, error);
    *ret_size = luns;
   return res;
}


int
dwarf_bitoffset (
    Dwarf_Die		die,
    Dwarf_Unsigned     *ret_size,
    Dwarf_Error		*error
)
{
   Dwarf_Unsigned luns;
   int res;
    res= _dwarf_die_attr_unsigned_constant(die, DW_AT_bit_offset,&luns, error);
    *ret_size = luns;
   return res;
}


/* Refer section 3.1, page 21 in Dwarf Definition. */
int
dwarf_srclang (
    Dwarf_Die		die,
    Dwarf_Unsigned     *ret_size,
    Dwarf_Error		*error
)
{
   Dwarf_Unsigned luns;
   int res;
    res = _dwarf_die_attr_unsigned_constant(die, DW_AT_language,&luns, error);
    *ret_size = luns;
    return res;
}


/* Refer section 5.4, page 37 in Dwarf Definition. */
int
dwarf_arrayorder (
    Dwarf_Die		die,
    Dwarf_Unsigned     *ret_size,
    Dwarf_Error		*error
)
{
   Dwarf_Unsigned luns;
	int res;
    res =_dwarf_die_attr_unsigned_constant(die, DW_AT_ordering,&luns, error);
    *ret_size = luns;
    return res;
}

/*
	Return DW_DLV_OK if ok
	DW_DLV_ERROR if failure.

	If the die and the attr are not related the result is
	meaningless.
*/
int
dwarf_attr_offset (
    Dwarf_Die       die,
    Dwarf_Attribute attr,
    Dwarf_Off  *    offset, /* return offset thru this ptr */
    Dwarf_Error     *error
)
{
    Dwarf_Off attroff;
    CHECK_DIE(die, DW_DLV_ERROR)

    attroff = (attr->ar_debug_info_ptr - 
		die->di_cu_context->cc_dbg->de_debug_info);
    *offset = attroff;
    return DW_DLV_OK;
}

