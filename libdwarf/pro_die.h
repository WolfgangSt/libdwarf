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

	pro_die.h   
	$Revision: 1.9 $    $Date: 1999/04/20 16:43:39 $    

	Header file for producer die related information. This file is included 
	in pro_section.c. 

*/


/* 
	This struct holds the abbreviation table, before they are written 
	on disk. Holds a linked list of abbreviations, each consisting of
	a bitmap for attributes and a bitmap for forms
*/
typedef struct Dwarf_P_Abbrev_s 	*Dwarf_P_Abbrev;

struct Dwarf_P_Abbrev_s {
	Dwarf_Unsigned 	abb_idx;      /* index of abbreviation */
	Dwarf_Tag 	abb_tag;      /* tag of die */
	Dwarf_Ubyte 	abb_children; /* if children are present */
	Dwarf_ufixed   *abb_attrs;    /* holds names of attrs */
	Dwarf_ufixed   *abb_forms;    /* forms of attributes */
	int 		abb_n_attr;   /* num of attrs = 
						# of forms */
	Dwarf_P_Abbrev 	abb_next;
};

/* used in pro_section.c */

int _dwarf_pro_add_AT_fde (Dwarf_P_Debug dbg, Dwarf_P_Die die, 
    Dwarf_Unsigned offset, Dwarf_Error *error);

int _dwarf_pro_add_AT_stmt_list (Dwarf_P_Debug dbg, Dwarf_P_Die first_die, 
    Dwarf_Error *error);

int _dwarf_pro_add_AT_macro_info(Dwarf_P_Debug dbg, Dwarf_P_Die first_die,
    Dwarf_Unsigned offset, Dwarf_Error *error);
