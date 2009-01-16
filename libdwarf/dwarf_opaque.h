/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU Lesser General Public 
  License along with this program; if not, write the Free Software 
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, 
  USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/



struct Dwarf_Die_s {
	/* 
	    Points to the start of the portion corresponding 
	    to this Die in the .debug_info section.  
	*/
    Dwarf_Byte_Ptr       di_debug_info_ptr;

    Dwarf_Abbrev_List	 di_abbrev_list;

	/* Points to cu context for this die.  */
    Dwarf_CU_Context	 di_cu_context;
};

struct Dwarf_Attribute_s {
    Dwarf_Half          ar_attribute;		/* Attribute Value. */
    Dwarf_Half          ar_attribute_form;	/* Attribute Form. */
    Dwarf_CU_Context	ar_cu_context;
    Dwarf_Small		*ar_debug_info_ptr;
    Dwarf_Attribute	ar_next;
};

/*
    This structure provides the context for a compilation unit.  
    Thus, it contains the Dwarf_Debug, cc_dbg, that this cu
    belongs to.  It contains the information in the compilation 
    unit header, cc_length, cc_version_stamp, cc_abbrev_offset,
    and cc_address_size, in the .debug_info section for that cu.  
    In addition, it contains the count, cc_count_cu, of the cu 
    number of that cu in the list of cu's in the .debug_info.  
    The count starts at 1, ie cc_count_cu is 1 for the first cu, 
    2 for the second and so on.  This struct also contains a 
    pointer, cc_abbrev_table, to a list of pairs of abbrev code 
    and a pointer to the start of that abbrev 
    in the .debug_abbrev section.

    Each die will also contain a pointer to such a struct to 
    record the context for that die.  
    
    **Updated by dwarf_next_cu_header in dwarf_die_deliv.c
*/
struct Dwarf_CU_Context_s {
    Dwarf_Debug			cc_dbg;
    Dwarf_Word			cc_length;
    Dwarf_Small			cc_length_size;
    Dwarf_Small			cc_extension_size;
    Dwarf_Half			cc_version_stamp;
    Dwarf_Sword			cc_abbrev_offset;
    Dwarf_Small			cc_address_size;
    Dwarf_Word			cc_debug_info_offset;
    Dwarf_Byte_Ptr		cc_last_abbrev_ptr;
    Dwarf_Hash_Table		cc_abbrev_hash_table;
    Dwarf_CU_Context		cc_next;
    unsigned char               cc_offset_length;
};


struct Dwarf_Debug_s {
    Elf                 *de_elf;

    Dwarf_Unsigned      de_access;
    Dwarf_Handler       de_errhand;
    Dwarf_Ptr           de_errarg;
    
	/* 
	    Context for the compilation_unit just
	    read by a call to dwarf_next_cu_header.
	    **Updated by dwarf_next_cu_header in
	    dwarf_die_deliv.c
	*/
    Dwarf_CU_Context	de_cu_context;

        /*
	    Points to linked list of CU Contexts
	    for the CU's already read.  These are
	    only CU's read by dwarf_next_cu_header().
	*/
    Dwarf_CU_Context    de_cu_context_list;

        /* 
	    Points to the last CU Context added to 
	    the list by dwarf_next_cu_header(). 
	*/
    Dwarf_CU_Context    de_cu_context_list_end;

	/*
	    This is the list of CU contexts read for
	    dwarf_offdie().  These may read ahead of
	    dwarf_next_cu_header().
	*/
    Dwarf_CU_Context	de_offdie_cu_context;
    Dwarf_CU_Context	de_offdie_cu_context_end;

	/* Offset of last byte of last CU read. */
    Dwarf_Word		de_info_last_offset;

	/* 
	    Number of bytes in the length,
	    and offset field in various .debug_* 
	    sections.  Check that this is in agreement 
	    with the address size in those sections.  
	    This field is dervied from the Elf header.
	    4 for mips32, ia32,  8 for MIPS -64

	    For the dwarf2 32->64 extension, this
	    is no  longer applicable, as 
	    the extension-64 method makes 64bit length_size
	    be specific to each compilation unit.
	*/

    Dwarf_Small	de_length_size;

	/* number of bytes in a pointer of the target
           in various .debug_ sections.
	   4 in 32bit, 8 in MIPS 64, ia64.
	*/
    Dwarf_Small         de_pointer_size;

	/* set at creation of a Dwarf_Debug to say
           if form_string should be checked for valid
           length at every call. 0 means do the check.
	   non-zero means do not do the check.
	*/
    Dwarf_Small         de_assume_string_in_bounds;

	/* 
	    Dwarf_Alloc_Hdr_s structs 
	    used to manage chunks that are malloc'ed for 
	    each allocation type for structs.
	*/
    struct Dwarf_Alloc_Hdr_s de_alloc_hdr[ ALLOC_AREA_REAL_TABLE_MAX];

    /* 
	These fields are used to process debug_frame
    	section.  **Updated by dwarf_get_fde_list
    	in dwarf_frame.h
    */
	/* 
	    Points to contiguous block of pointers 
	    to Dwarf_Cie_s structs.
	*/
    Dwarf_Cie           *de_cie_data;
	/* Count of number of Dwarf_Cie_s structs. */
    Dwarf_Signed        de_cie_count;
	/* 
	    Points to contiguous block of pointers
	    to Dwarf_Fde_s structs.
	*/
    Dwarf_Fde           *de_fde_data;
	/* Count of number of Dwarf_Fde_s structs. */
    Dwarf_Signed        de_fde_count;

    Dwarf_Small         *de_debug_info;
    Dwarf_Small         *de_debug_abbrev;
    Dwarf_Small         *de_debug_line;
    Dwarf_Small         *de_debug_loc;
    Dwarf_Small         *de_debug_aranges;
    Dwarf_Small         *de_debug_macinfo;
    Dwarf_Small         *de_debug_pubnames;
    Dwarf_Small         *de_debug_str;
    Dwarf_Small         *de_debug_frame;
    Dwarf_Small         *de_debug_frame_eh_gnu; /* gnu for
			the g++ eh_frame section */

    Dwarf_Small		*de_debug_funcnames;
    Dwarf_Small		*de_debug_typenames;
    Dwarf_Small		*de_debug_varnames;
    Dwarf_Small		*de_debug_weaknames;

    Dwarf_Unsigned      de_debug_info_size;
    Dwarf_Unsigned      de_debug_abbrev_size;
    Dwarf_Unsigned      de_debug_line_size;
    Dwarf_Unsigned      de_debug_loc_size;
    Dwarf_Unsigned      de_debug_aranges_size;
    Dwarf_Unsigned      de_debug_macinfo_size;
    Dwarf_Unsigned      de_debug_pubnames_size;
    Dwarf_Unsigned      de_debug_str_size;


    Dwarf_Unsigned      de_debug_frame_size;

    Dwarf_Unsigned      de_debug_frame_size_eh_gnu; /* gnu for
			the g++ eh_frame section */

    Dwarf_Unsigned	de_debug_funcnames_size;
    Dwarf_Unsigned	de_debug_typenames_size;
    Dwarf_Unsigned	de_debug_varnames_size;
    Dwarf_Unsigned	de_debug_weaknames_size;

               void * (*de_copy_word)(void *, const void *, size_t);
    unsigned char       de_same_endian;

};

typedef struct Dwarf_Chain_s	*Dwarf_Chain;
struct Dwarf_Chain_s {
    void		*ch_item;
    Dwarf_Chain		ch_next;
};

#define CURRENT_VERSION_STAMP		2

    /* Size of cu header version stamp field. */
#define CU_VERSION_STAMP_SIZE   sizeof(Dwarf_Half)

    /* Size of cu header address size field. */
#define CU_ADDRESS_SIZE_SIZE	sizeof(Dwarf_Small)

void * _dwarf_memcpy_swap_bytes(void *s1, const void *s2,size_t len);

#define ORIGINAL_DWARF_OFFSET_SIZE  4
#define DISTINGUISHED_VALUE  0xffffffff
#define DISTINGUISHED_VALUE_OFFSET_SIZE 8

