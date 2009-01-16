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

    dwarf_global.h

    $Revision: 1.6 $		$Date: 1999/03/05 22:00:05 $
*/


typedef struct Dwarf_Global_Context_s	*Dwarf_Global_Context;

/* 
    This struct contains header information for a set of pubnames.
    Essentially, they contain the context for a set of pubnames 
    belonging to a compilation-unit.


    This is also used for the sgi-specific
    weaknames, typenames, varnames, funcnames data:
    the structs for those are incomplete and
    instances of this are used instead.

*/
struct Dwarf_Global_Context_s {

        /* 
	    Length in .debug_pubnames of a set of pubnames 
	    for a compilation-unit. 
            Dwarf_Word			pu_length;
	    The value is not made available outside
	    libdwarf and not used inside,
	    so no need to record it.
	*/

    /* for this context, size of a length. 4 or 8 */
    unsigned char               pu_length_size;
    /* for this CU, size of the extension 0 except for
	dwarf2 extension 64bit, in which case is 4.
    */
    unsigned char               pu_extension_size;

	/* 
	    Offset into .debug_info of the compilation-unit
	    header (not DIE) for this set of pubnames.
	*/
    Dwarf_Off			pu_offset_of_cu_header;

	/* Size of compilation-unit that these pubnames are in. */
    Dwarf_Unsigned		pu_info_length;

    Dwarf_Debug                   pu_dbg;
};


/* This struct contains information for a single pubname. */
struct Dwarf_Global_s {

	/* 
	    Offset from the start of the corresponding compilation-unit
	    of the DIE for the given pubname CU.
	*/
    Dwarf_Off			gl_named_die_offset_within_cu;

	/* Points to the given pubname. */
    Dwarf_Small			*gl_name;

	/* Context for this pubname. */
    Dwarf_Global_Context	gl_context;
};

int _dwarf_internal_get_pubnames_like_data(
    Dwarf_Debug         dbg,
    Dwarf_Small         *section_data_ptr,
    Dwarf_Unsigned      section_length,
    Dwarf_Global        **globals,
    Dwarf_Signed       *return_count,
    Dwarf_Error         *error,
    int                 allocation_code,
    int                 length_err_num,
    int                 version_err_num
);

