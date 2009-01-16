/* 
Copyright (c) 1998,1999 Silicon Graphics, Inc.

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


$Header: /isms/cmplrs.src/osprey1.0/dwarfdump/RCS/tag_attr.c,v 1.3 1999/03/05 21:59:55 davea Exp $ */
#include <dwarf.h>
#include <stdio.h>

static unsigned int tag_attr_combination_table[0x36][3];
static char *tag_name[ ] = {
	"0x00", 
	"0x01 DW_TAG_array_type", 
	"0x02 DW_TAG_class_type", 
	"0x03 DW_TAG_entry_point", 
	"0x04 DW_TAG_enumeration_type", 
	"0x05 DW_TAG_formal_parameter", 
	"0x06", 
	"0x07", 
	"0x08 DW_TAG_imported_declaration", 
	"0x09", 
	"0x0a DW_TAG_label", 
	"0x0b DW_TAG_lexical_block", 
	"0x0c", 
	"0x0d DW_TAG_member", 
	"0x0e", 
	"0x0f DW_TAG_pointer_type", 
	"0x10 DW_TAG_reference_type", 
	"0x11 DW_TAG_compile_unit", 
	"0x12 DW_TAG_string_type", 
	"0x13 DW_TAG_structure_type", 
	"0x14", 
	"0x15 DW_TAG_subroutine_type", 
	"0x16 DW_TAG_typedef", 
	"0x17 DW_TAG_union_type", 
	"0x18 DW_TAG_unspecified_parameters", 
	"0x19 DW_TAG_variant", 
	"0x1a DW_TAG_common_block", 
	"0x1b DW_TAG_common_inclusion", 
	"0x1c DW_TAG_inheritance", 
	"0x1d DW_TAG_inlined_subroutine", 
	"0x1e DW_TAG_module", 
	"0x1f DW_TAG_ptr_to_member_type", 
	"0x20 DW_TAG_set_type", 
	"0x21 DW_TAG_subrange_type", 
	"0x22 DW_TAG_with_stmt", 
	"0x23 DW_TAG_access_declaration", 
	"0x24 DW_TAG_base_type", 
	"0x25 DW_TAG_catch_block", 
	"0x26 DW_TAG_const_type", 
	"0x27 DW_TAG_constant", 
	"0x28 DW_TAG_enumerator", 
	"0x29 DW_TAG_file_type", 
	"0x2a DW_TAG_friend", 
	"0x2b DW_TAG_namelist", 
	"0x2c DW_TAG_namelist_item", 
	"0x2d DW_TAG_packed_type", 
	"0x2e DW_TAG_subprogram", 
	"0x2f DW_TAG_template_type_param", 
	"0x30 DW_TAG_template_value_param", 
	"0x31 DW_TAG_thrown_type", 
	"0x32 DW_TAG_try_block", 
	"0x33 DW_TAG_variant_part", 
	"0x34 DW_TAG_variable", 
	"0x35 DW_TAG_volatile_type", 
};


int
main ()
{
    int i;
    int num;
    scanf("%x\n", &num); /* 0xffffffff */
    while (! feof(stdin)) {
	int tag;
	scanf("%x\n", &tag);
	scanf("%x\n", &num);
	while (num != 0xffffffff) {
	    int idx = num / 0x20;
	    int bit = num % 0x20;
	    tag_attr_combination_table[tag][idx] |= (1 << bit);
	    scanf("%x\n", &num);
	}
    }
    printf("static unsigned int tag_attr_combination_table [ ][3] = {\n");
    for (i = 0; i < 0x36; i ++) {
	printf("/* %-37s*/\n", tag_name[i]);
	printf("    { %#.8x, %#.8x, %#.8x},\n", 
	       tag_attr_combination_table[i][0], 
	       tag_attr_combination_table[i][1], 
	       tag_attr_combination_table[i][2]);
    }
    printf("};\n");
    return(0);
}
