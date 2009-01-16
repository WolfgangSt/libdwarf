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

    dwarf_util.c
    $Revision: 1.37 $   $Date: 1999/07/21 23:18:49 $

    Dwarf utility functions.
*/

#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include "dwarf_die_deliv.h"



/*
    Given a form, and a pointer to the bytes encoding 
    a value of that form, val_ptr, this function returns
    the length, in bytes, of a value of that form.
    When using this function, check for a return of 0
    a recursive DW_FORM_INDIRECT value.
*/
Dwarf_Unsigned
_dwarf_get_size_of_val (
    Dwarf_Debug		dbg,
    Dwarf_Unsigned      form,
    Dwarf_Small     	*val_ptr,
    int                 v_length_size
)
{
    Dwarf_Unsigned      length;
    Dwarf_Word		leb128_length;
    Dwarf_Unsigned      form_indirect;
    Dwarf_Unsigned	ret_value;

    switch (form) {

	default :			/* Handles form = 0. */
	    return(form);

        case DW_FORM_addr:  
            return(dbg->de_pointer_size);

        case DW_FORM_ref_addr:   
            return(v_length_size);

	case DW_FORM_block1:
	    return(*(Dwarf_Small *)val_ptr + 1);

        case DW_FORM_block2:
	    READ_UNALIGNED(dbg,ret_value, Dwarf_Unsigned,
			val_ptr, sizeof(Dwarf_Half));
	    return(ret_value + sizeof(Dwarf_Half));

        case DW_FORM_block4:
	    READ_UNALIGNED(dbg,ret_value, Dwarf_Unsigned,
			val_ptr, sizeof(Dwarf_ufixed));
	    return(ret_value + sizeof(Dwarf_ufixed));

    
        case DW_FORM_data1:
            return(1);

        case DW_FORM_data2:
            return(2);

        case DW_FORM_data4:
            return(4);

        case DW_FORM_data8:
            return(8);

        case DW_FORM_string:
            return(strlen((char *)val_ptr) + 1);

        case DW_FORM_block:
            length = _dwarf_decode_u_leb128(val_ptr,&leb128_length);
            return(length + leb128_length);

        case DW_FORM_flag:
            return(1);

        case DW_FORM_indirect:
            form_indirect = _dwarf_decode_u_leb128(val_ptr,
			&leb128_length);
            if (form_indirect == DW_FORM_indirect) return(0);
            return(leb128_length + _dwarf_get_size_of_val(dbg, 
			form_indirect,
	        val_ptr+leb128_length,v_length_size));

        case DW_FORM_ref1:
	    return(1);

	case DW_FORM_ref2:
	    return(2);

	case DW_FORM_ref4:
	    return(4);

	case DW_FORM_ref8:
	    return(8);

        case DW_FORM_sdata: 
            _dwarf_decode_s_leb128(val_ptr,&leb128_length);
            return(leb128_length);

        case DW_FORM_strp:
            return(v_length_size);

        case DW_FORM_udata: 
            _dwarf_decode_u_leb128(val_ptr,&leb128_length);
            return(leb128_length);
    }
}


/*
    This function returns a pointer to a Dwarf_Abbrev_List_s
    struct for the abbrev with the given code.  It puts the
    struct on the appropriate hash table.  It also adds all
    the abbrev between the last abbrev added and this one to
    the hash table.  In other words, the .debug_abbrev section
    is scanned sequentially from the top for an abbrev with
    the given code.  All intervening abbrevs are also put 
    into the hash table.

    This function hashes the given code, and checks the chain
    at that hash table entry to see if a Dwarf_Abbrev_List_s
    with the given code exists.  If yes, it returns a pointer
    to that struct.  Otherwise, it scans the .debug_abbrev
    section from the last byte scanned for that CU till either
    an abbrev with the given code is found, or an abbrev code
    of 0 is read.  It puts Dwarf_Abbrev_List_s entries for all
    abbrev's read till that point into the hash table.  The
    hash table contains both a head pointer and a tail pointer
    for each entry.

    Returns NULL on error.
*/
Dwarf_Abbrev_List
_dwarf_get_abbrev_for_code (
    Dwarf_CU_Context	cu_context,
    Dwarf_Word		code
)
{
    Dwarf_Debug		dbg = cu_context->cc_dbg;
    Dwarf_Hash_Table	hash_table = cu_context->cc_abbrev_hash_table;
    Dwarf_Word		hash_num;
    Dwarf_Abbrev_List	hash_abbrev_list;
    Dwarf_Abbrev_List	abbrev_list;
    Dwarf_Byte_Ptr	abbrev_ptr;
    Dwarf_Half		abbrev_code, abbrev_tag;
    Dwarf_Half		attr_name, attr_form;

    hash_num = code % ABBREV_HASH_TABLE_SIZE;
    for (hash_abbrev_list = hash_table[hash_num].at_head; 
	hash_abbrev_list != NULL && hash_abbrev_list->ab_code != code; 
        hash_abbrev_list = hash_abbrev_list->ab_next);
    if (hash_abbrev_list != NULL)
        return(hash_abbrev_list);

    abbrev_ptr = cu_context->cc_last_abbrev_ptr != NULL ? 
	cu_context->cc_last_abbrev_ptr : 
	dbg->de_debug_abbrev + cu_context->cc_abbrev_offset;

	/* End of abbrev's for this cu, since abbrev code is 0. */
    if (*abbrev_ptr == 0)  {
		return(NULL);
    }

    do {
	Dwarf_Unsigned utmp;
        DECODE_LEB128_UWORD(abbrev_ptr, utmp)
	abbrev_code = (Dwarf_Half)utmp;
	DECODE_LEB128_UWORD(abbrev_ptr, utmp)
	abbrev_tag = (Dwarf_Half)utmp;

        abbrev_list = (Dwarf_Abbrev_List)
	    _dwarf_get_alloc(cu_context->cc_dbg, DW_DLA_ABBREV_LIST, 1);
        if (abbrev_list == NULL) return(NULL);

	hash_num = abbrev_code % ABBREV_HASH_TABLE_SIZE;
	if (hash_table[hash_num].at_head == NULL) {
	    hash_table[hash_num].at_head = 
		hash_table[hash_num].at_tail = abbrev_list;
	}
	else {
	    hash_table[hash_num].at_tail->ab_next = abbrev_list;
	    hash_table[hash_num].at_tail = abbrev_list;
	}

        abbrev_list->ab_code = abbrev_code;
	abbrev_list->ab_tag = abbrev_tag;

	abbrev_list->ab_has_child = *(abbrev_ptr++);
        abbrev_list->ab_abbrev_ptr = abbrev_ptr;

	do {
	    Dwarf_Unsigned utmp3;
	    DECODE_LEB128_UWORD(abbrev_ptr, utmp3)
	    attr_name = (Dwarf_Half)utmp3;
	    DECODE_LEB128_UWORD(abbrev_ptr, utmp3)
	    attr_form = (Dwarf_Half)utmp3;
        } while (attr_name != 0 && attr_form != 0);

    } while (*abbrev_ptr != 0 && abbrev_code != code);

    cu_context->cc_last_abbrev_ptr = abbrev_ptr;
    return(abbrev_code == code ? abbrev_list : NULL);
}


/* return 1 if string ends before 'endptr' else
** return 0 meaning string is not properly terminated.
** Presumption is the 'endptr' pts to end of some dwarf section data.
*/
int _dwarf_string_valid(void *startptr, void *endptr)
{

	char *start = startptr;
	char *end = endptr;

	while(start < end) {
		if (*start == 0) {
			return 1; /* OK! */
		}
		++start;
		++end;
	}
	return 0; /* FAIL! bad string! */
}

/*
  A byte-swapping version of memcpy
  for cross-endian use.
  Only 2,4,8 should be lengths passed in.
*/
void * _dwarf_memcpy_swap_bytes(void *s1, const void *s2,size_t len)
{
  void *orig_s1 = s1;
  unsigned char *targ = (unsigned char *)s1;
  unsigned char *src  = (unsigned char *)s2;

  if(len == 4) {
	targ[3] = src[0];
	targ[2] = src[1];
	targ[1] = src[2];
	targ[0] = src[3];
  } else if (len == 8) {
	targ[7] = src[0];
	targ[6] = src[1];
	targ[5] = src[2];
	targ[4] = src[3];
	targ[3] = src[4];
	targ[2] = src[5];
	targ[1] = src[6];
	targ[0] = src[7];
  } else if(len == 2) {
	targ[1] = src[0];
	targ[0] = src[1];
  }
/* should NOT get below here: is not the intended use */
  else if(len == 1) {
	targ[0] = src[0];
  } else {
	memcpy(s1,s2,len);
  } 

  return orig_s1;
}


/*
  This calculation used to be sprinkled all over.
  Now brought to one place.

  We try to accurately compute the size of a cu header
  given a known cu header location ( an offset in .debug_info).

*/
/* ARGSUSED */
Dwarf_Unsigned 
_dwarf_length_of_cu_header(Dwarf_Debug dbg, Dwarf_Unsigned offset) 
{
	int local_length_size = 0;
	int local_extension_size = 0;
        Dwarf_Unsigned length = 0;
	Dwarf_Small *cuptr = dbg->de_debug_info    + offset;

        READ_AREA_LENGTH(dbg,length,Dwarf_Unsigned,
          cuptr,local_length_size,local_extension_size);

	return
	    local_extension_size +  /* initial extesion, if present */
            local_length_size +       /* Size of cu length field. */
            sizeof(Dwarf_Half) +        /* Size of version stamp field. */
            local_length_size +       /* Size of abbrev offset field. */
            sizeof(Dwarf_Small);        /* Size of address size field. */

}

/*
	Pretend we know nothing about the CU
	and just roughly compute the result. 
*/
Dwarf_Unsigned 
_dwarf_length_of_cu_header_simple(Dwarf_Debug dbg) 
{
	return
            dbg->de_length_size +       /* Size of cu length field. */
            sizeof(Dwarf_Half) +        /* Size of version stamp field. */
            dbg->de_length_size +       /* Size of abbrev offset field. */
            sizeof(Dwarf_Small);        /* Size of address size field. */
}
