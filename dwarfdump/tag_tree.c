/* 
  Copyright (C) 2000,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan



$Header: /plroot/cmplrs.src/v7.4.4m/.RCS/PL/dwarfdump/RCS/tag_tree.c,v 1.5 2004/10/28 22:26:58 davea Exp $ */
#include <dwarf.h>
#include <stdio.h>
#include <stdlib.h> /* For exit() declaration etc. */
#include <errno.h> /* For errno declaration. */


/*  The following is the magic token used to
    distinguish real tags/attrs from group-delimiters.
    Blank lines have been eliminated by an awk script.
*/
#define MAGIC_TOKEN_VALUE 0xffffffff

/* Expected input format

0xffffffff
value of a tag
value of a standard tag that may be a child ofthat tag
...
0xffffffff
value of a tag
value of a standard tag that may be a child ofthat tag
...
0xffffffff
...

No blank lines or commentary allowed, no symbols, just numbers.


*/


/* We don't need really long lines: the input file is simple. */
#define MAX_LINE_SIZE 1000

/* 1 more than the higest number in the DW_TAG defines. */
#define TABLE_SIZE 0x41

/* Enough entries to have a bit for each standard legal tag. */
#define COLUMN_COUNT 2

/* Bits per 'int' to mark legal attrs. */
#define BITS_PER_WORD 32



#define TABLE_SIZE 0x41

static unsigned int tag_tree_combination_table[TABLE_SIZE][COLUMN_COUNT];
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
	"0x2f DW_TAG_template_type_parameter", 
	"0x30 DW_TAG_template_value_parameter", 
	"0x31 DW_TAG_thrown_type", 
	"0x32 DW_TAG_try_block", 
	"0x33 DW_TAG_variant_part", 
	"0x34 DW_TAG_variable", 
	"0x35 DW_TAG_volatile_type", 
        "0x36 DW_TAG_dwarf_procedure",
        "0x37 DW_TAG_restrict_type",
        "0x38 DW_TAG_interface_type",
        "0x39 DW_TAG_namespace",
        "0x3a DW_TAG_imported_module",
        "0x3b DW_TAG_unspecified_type",
        "0x3c DW_TAG_partial_unit",
        "0x3d DW_TAG_imported_unit",
        "0x3e", /* was DW_TAG_mutable_type, removed from DWARF3f. */
        "0x3f DW_TAG_condition",
        "0x40 DW_TAG_shared_type",
};

static int linecount = 0;
static char line_in[MAX_LINE_SIZE];

#define IS_EOF 1
#define NOT_EOF 0

#define MAX_LINE_SIZE 1000


static void
bad_line_input(char *msg)
{
  fprintf(stderr,
        "tag_tree table build failed %s, line %d: \"%s\"  \n",
                msg,linecount,line_in);
  exit(1);

}
static void
trim_newline(char *line,int max)
{
    char *end = line + max -1;
    for(;  *line && (line < end)  ; ++line) {
        if(*line == '\n') {
            /* Found newline, drop it */
            *line = 0;
            return;
        }
    }
       
    return;
}


/* Reads a value from the text table. 
   Exits  with non-zero status 
   if the table is erroneous in some way. 
*/
static int
read_value(unsigned int *outval)
{
    char *res = 0;
    FILE *file = stdin;
    unsigned long lval;
    char * strout = 0;

    ++linecount;
    *outval = 0;
    res = fgets(line_in, sizeof(line_in), file);
    if(res == 0) {
	if(ferror(file)) {
	    fprintf(stderr,"tag_attr: Error reading table, %d lines read\n",
		linecount);
	    exit(1);
	}
	if(feof(file)) {
	  return IS_EOF;
	}
	/* impossible */
	fprintf(stderr,"tag_attr: Impossible error reading table, "
		"%d lines read\n",
		linecount);
	exit(1);
    }
    trim_newline(line_in,sizeof(line_in));
    errno = 0;
    lval = strtoul(line_in,&strout,0);
    if(strout == line_in) {
	bad_line_input("bad number input!");
    }
    if( errno != 0) {
	int myerr = errno;
	fprintf(stderr,"tag_attr errno %d\n",myerr);
	bad_line_input("invalid number on line");
    }
    *outval = (int)lval;
    return NOT_EOF;
}


int
main ()
{
    int i;
    unsigned int num;
    int input_eof;
    

    input_eof = read_value(&num); /* 0xffffffff */
    if(IS_EOF == input_eof) {   
        bad_line_input("Empty input file");
    }
    if(num != MAGIC_TOKEN_VALUE) {
        bad_line_input("Expected 0xffffffff");
    }

    while (! feof(stdin)) {
        unsigned int tag;
        input_eof = read_value(&tag);
        if(IS_EOF == input_eof) {
           /* Reached normal eof */
           break;
        }
        if( tag >= TABLE_SIZE ) {
          bad_line_input("tag value exceeds table size");
        }
        input_eof = read_value(&num);
        if(IS_EOF == input_eof) {
          bad_line_input("Not terminated correctly..");
        }

	while (num != 0xffffffff) {
	    int idx = num / BITS_PER_WORD;
	    int bit = num % BITS_PER_WORD;
            if(idx >= COLUMN_COUNT) {
                bad_line_input("too many TAGs: table incomplete.");
            }

	    tag_tree_combination_table[tag][idx] |= (1 << bit);
            input_eof = read_value(&num);
            if(IS_EOF == input_eof) {
              bad_line_input("Not terminated correctly.");
            }
	}
    }
    printf("static unsigned int tag_tree_combination_table [ ][%d] = {\n",
		COLUMN_COUNT);
    for (i = 0; i < TABLE_SIZE; i ++) {
	printf("/* %-37s*/\n", tag_name[i]);
	printf("    { %#.8x, %#.8x},\n", 
	       tag_tree_combination_table[i][0], 
	       tag_tree_combination_table[i][1]);
    }
    printf("};\n");
    return(0);
}
