/* 
  Copyright (C) 2000,2004,2005,2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007 Sun Microsystems, Inc. All rights reserved.

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


$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/print_die.c,v 1.51 2006/04/01 16:20:21 davea Exp $ */
#include "globals.h"
#include "dwarf_names.h"
#include "esb.h"		/* For flexible string buffer. */
#include "makename.h"		/* Non-duplicating string table. */

static void get_attr_value(Dwarf_Debug dbg, Dwarf_Half tag,
			   Dwarf_Attribute attrib, char **srcfiles,
			   Dwarf_Signed cnt, struct esb_s *esbp);
static void print_attribute(Dwarf_Debug dbg, Dwarf_Die die,
			    Dwarf_Half attr,
			    Dwarf_Attribute actual_addr,
			    boolean print_information, char **srcfiles,
			    Dwarf_Signed cnt);
static void get_location_list(Dwarf_Debug dbg, Dwarf_Die die,
			      Dwarf_Attribute attr, struct esb_s *esbp);
static int tag_attr_combination(Dwarf_Half tag, Dwarf_Half attr);

/* esb_base is static so gets initialized to zeros.  
   It is not thread-safe or
   safe for multiple open producer instances for
   but that does not matter here in dwarfdump.
*/
static struct esb_s esb_base;	

static int indent_level = 0;
static boolean local_symbols_already_began = FALSE;

typedef string(*encoding_type_func) (Dwarf_Debug dbg, Dwarf_Half val);

Dwarf_Off fde_offset_for_cu_low = DW_DLV_BADOFFSET;
Dwarf_Off fde_offset_for_cu_high = DW_DLV_BADOFFSET;

/* Dwarf_Half list_of_attrs[] */
/*#include "at_list.i" unreferenced */

#define DIE_STACK_SIZE 300
static Dwarf_Die die_stack[DIE_STACK_SIZE];

#define PUSH_DIE_STACK(x) { die_stack[indent_level] = x; }
#define POP_DIE_STACK { die_stack[indent_level] = 0; }

#include "_tag_tree_table.c"

/*
   Look only at valid table entries
   The check here must match the building-logic in
   tag_tree.c
   And must match the tags defined in dwarf.h
*/
#define MAX_CHECKED_TAG_ID 0x35
static int
tag_tree_combination(Dwarf_Half tag_parent, Dwarf_Half tag_child)
{
    if (tag_parent > 0 && tag_parent <= MAX_CHECKED_TAG_ID
	&& tag_child > 0 && tag_child <= MAX_CHECKED_TAG_ID) {
	return ((tag_tree_combination_table[tag_parent]
		 [tag_child / 0x20]
		 & (1 << (tag_child % 0x20))) > 0 ? TRUE : FALSE);
    } else
	return (FALSE);
}

/* recursively follow the die tree */
extern void
print_die_and_children(Dwarf_Debug dbg, Dwarf_Die in_die_in,
		       char **srcfiles, Dwarf_Signed cnt)
{
    Dwarf_Die child;
    Dwarf_Die sibling;
    Dwarf_Error err;
    int tres;
    int cdres;
    Dwarf_Die in_die = in_die_in;

    for (;;) {
	PUSH_DIE_STACK(in_die);

	if (check_tag_tree) {
	    tag_tree_result.checks++;
	    if (indent_level == 0) {
		Dwarf_Half tag;

		tres = dwarf_tag(in_die, &tag, &err);
		if (tres != DW_DLV_OK) {
		    tag_tree_result.errors++;
		    DWARF_CHECK_ERROR
			("Tag-tree root is not DW_TAG_compile_unit")
		} else if (tag == DW_TAG_compile_unit) {
		    /* OK */
		} else {
		    tag_tree_result.errors++;
		    DWARF_CHECK_ERROR
			("tag-tree root is not DW_TAG_compile_unit")
		}
	    } else {
		Dwarf_Half tag_parent, tag_child;
		int pres;
		int cres;
		char *ctagname = "<child tag invalid>";
		char *ptagname = "<parent tag invalid>";

		pres =
		    dwarf_tag(die_stack[indent_level - 1], &tag_parent,
			      &err);
		cres = dwarf_tag(in_die, &tag_child, &err);
		if (pres != DW_DLV_OK)
		    tag_parent = 0;
		if (cres != DW_DLV_OK)
		    tag_child = 0;
		if (cres != DW_DLV_OK || pres != DW_DLV_OK) {
		    if (cres == DW_DLV_OK) {
			ctagname = get_TAG_name(dbg, tag_child);
		    }
		    if (pres == DW_DLV_OK) {
			ptagname = get_TAG_name(dbg, tag_parent);
		    }
		    DWARF_CHECK_ERROR3(ptagname,
				       ctagname,
				       "Tag-tree relation is not standard..");
		} else if (tag_tree_combination(tag_parent, tag_child)) {
		    /* OK */
		} else {
		    DWARF_CHECK_ERROR3(get_TAG_name(dbg, tag_parent),
				       get_TAG_name(dbg, tag_child),
				       "tag-tree relation is not standard.");
		}
	    }
	}

	/* here to pre-descent processing of the die */
	print_one_die(dbg, in_die, info_flag, srcfiles, cnt);

	cdres = dwarf_child(in_die, &child, &err);
	/* child first: we are doing depth-first walk */
	if (cdres == DW_DLV_OK) {
	    indent_level++;
	    if(indent_level >= DIE_STACK_SIZE ) {
	        print_error(dbg,
                  "compiled in DIE_STACK_SIZE limit exceeded",
                  DW_DLV_OK,err);
	    }
	    print_die_and_children(dbg, child, srcfiles, cnt);
	    indent_level--;
	    if (indent_level == 0)
		local_symbols_already_began = FALSE;
	    dwarf_dealloc(dbg, child, DW_DLA_DIE);
	} else if (cdres == DW_DLV_ERROR) {
	    print_error(dbg, "dwarf_child", cdres, err);
	}

	cdres = dwarf_siblingof(dbg, in_die, &sibling, &err);
	if (cdres == DW_DLV_OK) {
	    /* print_die_and_children(dbg, sibling, srcfiles, cnt); We
	       loop around to actually print this, rather than
	       recursing. Recursing is horribly wasteful of stack
	       space. */
	} else if (cdres == DW_DLV_ERROR) {
	    print_error(dbg, "dwarf_siblingof", cdres, err);
	}

	/* Here do any post-descent (ie post-dwarf_child) processing of 
	   the in_die. */

	POP_DIE_STACK;
	if (in_die != in_die_in) {
	    /* Dealloc our in_die, but not the argument die, it belongs 
	       to our caller. Whether the siblingof call worked or not. 
	     */
	    dwarf_dealloc(dbg, in_die, DW_DLA_DIE);
	}
	if (cdres == DW_DLV_OK) {
	    /* Set to process the sibling, loop again. */
	    in_die = sibling;
	} else {
	    /* We are done, no more siblings at this level. */

	    break;
	}
    }				/* end for loop on siblings */
    return;
}

#define SPACE(x) { register int i; for (i=0;i<x;i++) putchar(' '); }


/* print info about die */
void
print_one_die(Dwarf_Debug dbg, Dwarf_Die die, boolean print_information,
	      char **srcfiles, Dwarf_Signed cnt)
{
    Dwarf_Signed i;
    Dwarf_Off offset, overall_offset;
    string tagname;
    Dwarf_Half tag;
    Dwarf_Signed atcnt;
    Dwarf_Attribute *atlist;
    int tres;
    int ores;
    int atres;

    tres = dwarf_tag(die, &tag, &err);
    if (tres != DW_DLV_OK) {
	print_error(dbg, "accessing tag of die!", tres, err);
    }
    tagname = get_TAG_name(dbg, tag);
    ores = dwarf_dieoffset(die, &overall_offset, &err);
    if (ores != DW_DLV_OK) {
	print_error(dbg, "dwarf_dieoffset", ores, err);
    }
    ores = dwarf_die_CU_offset(die, &offset, &err);
    if (ores != DW_DLV_OK) {
	print_error(dbg, "dwarf_die_CU_offset", ores, err);
    }

    if (!dst_format && print_information) {
	if (indent_level == 0) {
	    if (dense)
		printf("\n");
	    else {
		printf
		    ("\nCOMPILE_UNIT<header overall offset = %llu>:\n",
		     overall_offset - offset);
	    }
	} else if (local_symbols_already_began == FALSE &&
		   indent_level == 1 && !dense) {
	    printf("\nLOCAL_SYMBOLS:\n");
	    local_symbols_already_began = TRUE;
	}
	if (dense) {
            if (show_global_offsets) {
	        if (indent_level == 0) {
		    printf("<%d><%llu+%llu GOFF=%llu><%s>", indent_level,
		       overall_offset - offset, offset,
                       overall_offset, tagname);
	        } else {
		    printf("<%d><%llu GOFF=%llu><%s>", indent_level, 
                       offset, overall_offset, tagname);
	        }
            } else {
	        if (indent_level == 0) {
		    printf("<%d><%llu+%llu><%s>", indent_level,
		       overall_offset - offset, offset, tagname);
	        } else {
		    printf("<%d><%llu><%s>", indent_level, offset, tagname);
	        }
	    }
	} else {
            if (show_global_offsets) {
	        printf("<%d><%5llu GOFF=%llu>\t%s\n", indent_level, offset,
                    overall_offset, tagname);
            } else {
	        printf("<%d><%5llu>\t%s\n", indent_level, offset, tagname);
	    }
	}
    }

    atres = dwarf_attrlist(die, &atlist, &atcnt, &err);
    if (atres == DW_DLV_ERROR) {
	print_error(dbg, "dwarf_attrlist", atres, err);
    } else if (atres == DW_DLV_NO_ENTRY) {
	/* indicates there are no attrs.  It is not an error. */
	atcnt = 0;
    }


    for (i = 0; i < atcnt; i++) {
	Dwarf_Half attr;
	int ares;

	ares = dwarf_whatattr(atlist[i], &attr, &err);
	if (ares == DW_DLV_OK) {
	    print_attribute(dbg, die, attr,
			    atlist[i],
			    print_information, srcfiles, cnt);
	} else {
	    print_error(dbg, "dwarf_whatattr entry missing", ares, err);
	}
    }

    for (i = 0; i < atcnt; i++) {
	dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
    }
    if (atres == DW_DLV_OK) {
	dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
    }

    if (dense && print_information) {
	printf("\n\n");
    }
    return;
}

/* Encodings have undefined signedness. Accept either
   signedness.  The values are small (they are defined
   in the DWARF specification), so the
   form the compiler uses (as long as it is
   a constant value) is a non-issue.

   If string_out is non-NULL, construct a string output, either
   an error message or the name of the encoding.
   The function pointer passed in is to code generated
   by a script at dwarfdump build time. The code for
   the val_as_string function is generated
   from dwarf.h.  See <build dir>/dwarf_names.c

   If string_out is non-NULL then attr_name and val_as_string
   must also be non-NULL.

*/
int
get_small_encoding_integer_and_name(Dwarf_Debug dbg,
				    Dwarf_Attribute attrib,
				    Dwarf_Unsigned * uval_out,
				    char *attr_name,
				    string * string_out,
				    encoding_type_func val_as_string,
				    Dwarf_Error * err)
{
    Dwarf_Unsigned uval = 0;
    char buf[100];		/* The strings are small. */
    int vres = dwarf_formudata(attrib, &uval, err);

    if (vres != DW_DLV_OK) {
	Dwarf_Signed sval = 0;

	vres = dwarf_formsdata(attrib, &sval, err);
	if (vres != DW_DLV_OK) {
	    if (string_out != 0) {
		snprintf(buf, sizeof(buf),
			 "%s has a bad form.", attr_name);
		*string_out = makename(buf);
	    }
	    return vres;
	}
	*uval_out = (Dwarf_Unsigned) sval;
    } else {
	*uval_out = uval;
    }
    if (string_out)
	*string_out = val_as_string(dbg, (Dwarf_Half) uval);

    return DW_DLV_OK;

}




/*
 * We need a 32-bit signed number here, but there's no portable
 * way of getting that.  So use __uint32_t instead.  It's supplied
 * in a reliable way by the autoconf infrastructure.
 */

static void
get_FLAG_BLOCK_string(Dwarf_Debug dbg, Dwarf_Attribute attrib)
{
    int fres = 0;
    Dwarf_Block *tempb = 0;
    int i = 0;
    char * p = 0;
    __uint32_t * array = 0;
    Dwarf_Unsigned array_len = 0;
    int output_lines = 0;
    int output_chars = 0;
    __uint32_t * array_ptr;
    Dwarf_Unsigned array_remain = 0;
    char linebuf[100];

    esb_empty_string(&esb_base);
    esb_append(&esb_base, "\n");

    /* first get compressed block data */
    fres = dwarf_formblock (attrib,&tempb, &err);
    if (fres != DW_DLV_OK) {
	print_error(dbg,"DW_FORM_blockn cannot get block\n",fres,err);
	return;
    }

    /* uncompress block into int array */
    array = dwarf_uncompress_integer_block(dbg,
			   1, /* 'true' (meaning signed ints)*/
			   32, /* bits per unit */
			   tempb->bl_data,
			   tempb->bl_len,
			   &array_len, /* len of out array */
			   &err);
    if (array == (void*) DW_DLV_BADOFFSET) {
	print_error(dbg,"DW_AT_SUN_func_offsets cannot uncompress data\n",0,err);
	return;
    }
    if (array_len == 0) {
	print_error(dbg,"DW_AT_SUN_func_offsets has no data\n",0,err);
	return;
    }
    
    /* fill in string buffer */
    array_remain = array_len;
    array_ptr = array;
    while (array_remain > 8) {
	/* print a full line */
	/* if you touch this string, update the magic number 78 below! */
	snprintf(linebuf, sizeof(linebuf), 
		"\n  %8x %8x %8x %8x %8x %8x %8x %8x",
		array_ptr[0],		array_ptr[1],
		array_ptr[2],		array_ptr[3],
		array_ptr[4],		array_ptr[5],
		array_ptr[6],		array_ptr[7]);
	array_ptr += 8;
	array_remain -= 8;
	esb_append(&esb_base, linebuf);
    }

    /* now do the last line */
    if (array_remain > 0) {
	esb_append(&esb_base, "\n ");
	while (array_remain > 0) {
	    snprintf(linebuf, sizeof(linebuf), " %8x", *array_ptr);
	    array_remain--;
	    array_ptr++;
	    esb_append(&esb_base, linebuf);
	}
    }
    
    /* free array buffer */
    dwarf_dealloc_uncompressed_block(dbg, array);

}

static void
print_attribute(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attr,
		Dwarf_Attribute attr_in,
		boolean print_information,
		char **srcfiles, Dwarf_Signed cnt)
{
    Dwarf_Attribute attrib = 0;
    Dwarf_Unsigned uval = 0;
    string atname = 0;
    string valname = 0;
    int tres = 0;
    Dwarf_Half tag = 0;

    atname = get_AT_name(dbg, attr);

    /* the following gets the real attribute, even in the face of an 
       incorrect doubling, or worse, of attributes */
    attrib = attr_in;
    /* do not get attr via dwarf_attr: if there are (erroneously) 
       multiple of an attr in a DIE, dwarf_attr will not get the
       second, erroneous one and dwarfdump will print the first one
       multiple times. Oops. */

    tres = dwarf_tag(die, &tag, &err);
    if (tres == DW_DLV_ERROR) {
	tag = 0;
    } else if (tres == DW_DLV_NO_ENTRY) {
	tag = 0;
    } else {
	/* ok */
    }
    if (check_attr_tag) {
	char *tagname = "<tag invalid>";

	attr_tag_result.checks++;
	if (tres == DW_DLV_ERROR) {
	    attr_tag_result.errors++;
	    DWARF_CHECK_ERROR3(tagname,
			       get_AT_name(dbg, attr),
			       "check the tag-attr combination.");
	} else if (tres == DW_DLV_NO_ENTRY) {
	    attr_tag_result.errors++;
	    DWARF_CHECK_ERROR3(tagname,
			       get_AT_name(dbg, attr),
			       "check the tag-attr combination..")
	} else if (tag_attr_combination(tag, attr)) {
	    /* OK */
	} else {
	    attr_tag_result.errors++;
	    tagname = get_TAG_name(dbg, tag);
	    DWARF_CHECK_ERROR3(tagname,
			       get_AT_name(dbg, attr),
			       "check the tag-attr combination")
	}
    }

    switch (attr) {
    case DW_AT_language:
	get_small_encoding_integer_and_name(dbg, attrib, &uval,
					    "DW_AT_language", &valname,
					    get_LANG_name, &err);
	break;
    case DW_AT_accessibility:
	get_small_encoding_integer_and_name(dbg, attrib, &uval,
					    "DW_AT_accessibility",
					    &valname, get_ACCESS_name,
					    &err);
	break;
    case DW_AT_visibility:
	get_small_encoding_integer_and_name(dbg, attrib, &uval,
					    "DW_AT_visibility",
					    &valname, get_VIS_name,
					    &err);
	break;
    case DW_AT_virtuality:
	get_small_encoding_integer_and_name(dbg, attrib, &uval,
					    "DW_AT_virtuality",
					    &valname,
					    get_VIRTUALITY_name, &err);
	break;
    case DW_AT_identifier_case:
	get_small_encoding_integer_and_name(dbg, attrib, &uval,
					    "DW_AT_identifier",
					    &valname, get_ID_name,
					    &err);
	break;
    case DW_AT_inline:
	get_small_encoding_integer_and_name(dbg, attrib, &uval,
					    "DW_AT_inline", &valname,
					    get_INL_name, &err);
	break;
    case DW_AT_encoding:
	get_small_encoding_integer_and_name(dbg, attrib, &uval,
					    "DW_AT_encoding", &valname,
					    get_ATE_name, &err);
	break;
    case DW_AT_ordering:
	get_small_encoding_integer_and_name(dbg, attrib, &uval,
					    "DW_AT_ordering", &valname,
					    get_ORD_name, &err);
	break;
    case DW_AT_calling_convention:
	get_small_encoding_integer_and_name(dbg, attrib, &uval,
					    "DW_AT_calling_convention",
					    &valname, get_CC_name,
					    &err);
	break;
    case DW_AT_discr_list:	/* DWARF3 */
	get_small_encoding_integer_and_name(dbg, attrib, &uval,
					    "DW_AT_discr_list",
					    &valname, get_DSC_name,
					    &err);
	break;
    case DW_AT_location:
    case DW_AT_data_member_location:
    case DW_AT_vtable_elem_location:
    case DW_AT_string_length:
    case DW_AT_return_addr:
    case DW_AT_use_location:
    case DW_AT_static_link:
    case DW_AT_frame_base:
	/* value is a location description or location list */
	esb_empty_string(&esb_base);
	get_location_list(dbg, die, attrib, &esb_base);
	valname = esb_get_string(&esb_base);
	break;
    case DW_AT_SUN_func_offsets:
	get_FLAG_BLOCK_string(dbg, attrib);
	valname = esb_get_string(&esb_base);
	break;
    case DW_AT_SUN_cf_kind:
	{
	    Dwarf_Half kind;
	    Dwarf_Unsigned tempud;
	    Dwarf_Error err;
	    int wres;
	    wres = dwarf_formudata (attrib,&tempud, &err);
	    if(wres == DW_DLV_OK) {
		kind = tempud;
		valname = get_ATCF_name(dbg, kind);
	    } else if (wres == DW_DLV_NO_ENTRY) {
		valname = "?";
	    } else {
		print_error(dbg,"Cannot get formudata....",wres,err);
		valname = "??";
	    }
	}
	break;
    case DW_AT_upper_bound:
	{
	    Dwarf_Half theform;
	    int rv;
	    rv = dwarf_whatform(attrib,&theform,&err);
	    /* depending on the form and the attribute, process the form */
	    if(rv == DW_DLV_ERROR) {
		print_error(dbg, "dwarf_whatform cannot find attr form",
			    rv, err);
	    } else if (rv == DW_DLV_NO_ENTRY) {
		break;
	    }

	    switch (theform) {
	    case DW_FORM_block1:
		get_location_list(dbg, die, attrib, &esb_base);
		valname = esb_get_string(&esb_base);
		break;
	    default:
		esb_empty_string(&esb_base);
		get_attr_value(dbg, tag, attrib, srcfiles, cnt, &esb_base);
		valname = esb_get_string(&esb_base);
		break;
	    }
	    break;
	}
    default:
	esb_empty_string(&esb_base);
	get_attr_value(dbg, tag, attrib, srcfiles, cnt, &esb_base);
	valname = esb_get_string(&esb_base);
	break;
    }
    if (print_information) {
	if (dense)
	    printf(" %s<%s>", atname, valname);
	else
	    printf("\t\t%-28s%s\n", atname, valname);
    }
}


static int
_dwarf_print_one_locdesc(Dwarf_Debug dbg,
			 Dwarf_Locdesc * llbuf,
			 struct esb_s *string_out)
{

    Dwarf_Locdesc *locd;
    Dwarf_Half no_of_ops = 0;
    string op_name;
    int i;
    char small_buf[100];


    if (verbose || llbuf->ld_from_loclist) {
	snprintf(small_buf, sizeof(small_buf), "<lowpc=0x%llx>",
		 (unsigned long long) llbuf->ld_lopc);
	esb_append(string_out, small_buf);


	snprintf(small_buf, sizeof(small_buf), "<highpc=0x%llx>",
		 (unsigned long long) llbuf->ld_hipc);
	esb_append(string_out, small_buf);
	if (verbose) {
	    snprintf(small_buf, sizeof(small_buf),
		     "<from %s offset 0x%llx>",
		     llbuf->
		     ld_from_loclist ? ".debug_loc" : ".debug_info",
		     (unsigned long long) llbuf->ld_section_offset);
	    esb_append(string_out, small_buf);

	}
    }


    locd = llbuf;
    no_of_ops = llbuf->ld_cents;
    for (i = 0; i < no_of_ops; i++) {
	Dwarf_Small op;
	Dwarf_Unsigned opd1, opd2;

	/* local_space_needed is intended to be 'more than big enough'
	   for a short group of loclist entries.  */
	char small_buf[100];

	if (i > 0)
	    esb_append(string_out, " ");

	op = locd->ld_s[i].lr_atom;
	if (op > DW_OP_nop) {
	    print_error(dbg, "dwarf_op unexpected value", DW_DLV_OK,
			err);
	    return DW_DLV_ERROR;
	}
	op_name = get_OP_name(dbg, op);
	esb_append(string_out, op_name);

	opd1 = locd->ld_s[i].lr_number;
	if (op >= DW_OP_breg0 && op <= DW_OP_breg31) {
	    snprintf(small_buf, sizeof(small_buf),
		     "%+lld", (Dwarf_Signed) opd1);
	    esb_append(string_out, small_buf);
	} else {
	    switch (op) {
	    case DW_OP_addr:
		snprintf(small_buf, sizeof(small_buf), " %#llx", opd1);
		esb_append(string_out, small_buf);
		break;
	    case DW_OP_const1s:
	    case DW_OP_const2s:
	    case DW_OP_const4s:
	    case DW_OP_const8s:
	    case DW_OP_consts:
	    case DW_OP_skip:
	    case DW_OP_bra:
	    case DW_OP_fbreg:
		snprintf(small_buf, sizeof(small_buf),
			 " %lld", (Dwarf_Signed) opd1);
		esb_append(string_out, small_buf);
		break;
	    case DW_OP_const1u:
	    case DW_OP_const2u:
	    case DW_OP_const4u:
	    case DW_OP_const8u:
	    case DW_OP_constu:
	    case DW_OP_pick:
	    case DW_OP_plus_uconst:
	    case DW_OP_regx:
	    case DW_OP_piece:
	    case DW_OP_deref_size:
	    case DW_OP_xderef_size:
		snprintf(small_buf, sizeof(small_buf), " %llu", opd1);
		esb_append(string_out, small_buf);
		break;
	    case DW_OP_bregx:
		snprintf(small_buf, sizeof(small_buf), "%llu", opd1);
		esb_append(string_out, small_buf);



		opd2 = locd->ld_s[i].lr_number2;
		snprintf(small_buf, sizeof(small_buf),
			 "%+lld", (Dwarf_Signed) opd2);
		esb_append(string_out, small_buf);

		break;
	    default:
		break;
	    }
	}
    }

    return DW_DLV_OK;
}

/* Fill buffer with location lists 
   Buffer esbp expands as needed.
*/
 /*ARGSUSED*/ static void
get_location_list(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Attribute attr,
		  struct esb_s *esbp)
{
    Dwarf_Locdesc *llbuf = 0;
    Dwarf_Locdesc **llbufarray = 0;
    Dwarf_Signed no_of_elements;
    Dwarf_Error err;
    int i;
    int lres = 0;
    int llent = 0;


    if (use_old_dwarf_loclist) {

	lres = dwarf_loclist(attr, &llbuf, &no_of_elements, &err);
	if (lres == DW_DLV_ERROR)
	    print_error(dbg, "dwarf_loclist", lres, err);
	if (lres == DW_DLV_NO_ENTRY)
	    return;

	_dwarf_print_one_locdesc(dbg, llbuf, esbp);
	dwarf_dealloc(dbg, llbuf->ld_s, DW_DLA_LOC_BLOCK);
	dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);
	return;
    }

    lres = dwarf_loclist_n(attr, &llbufarray, &no_of_elements, &err);
    if (lres == DW_DLV_ERROR)
	print_error(dbg, "dwarf_loclist", lres, err);
    if (lres == DW_DLV_NO_ENTRY)
	return;

    for (llent = 0; llent < no_of_elements; ++llent) {
	char small_buf[100];

	llbuf = llbufarray[llent];

	if (!dense && llbuf->ld_from_loclist) {
	    if (llent == 0) {
		snprintf(small_buf, sizeof(small_buf),
			 "<loclist with %ld entries follows>",
			 (long) no_of_elements);
		esb_append(esbp, small_buf);
	    }
	    esb_append(esbp, "\n\t\t\t");
	    snprintf(small_buf, sizeof(small_buf), "[%2d]", llent);
	    esb_append(esbp, small_buf);
	}
	lres = _dwarf_print_one_locdesc(dbg, llbuf, esbp);
	if (lres == DW_DLV_ERROR) {
	    return;
	} else {
	    /* DW_DLV_OK so we add follow-on at end, else is
	       DW_DLV_NO_ENTRY (which is impossible, treat like
	       DW_DLV_OK). */
	}
    }
    for (i = 0; i < no_of_elements; ++i) {
	dwarf_dealloc(dbg, llbufarray[i]->ld_s, DW_DLA_LOC_BLOCK);
	dwarf_dealloc(dbg, llbufarray[i], DW_DLA_LOCDESC);
    }
    dwarf_dealloc(dbg, llbufarray, DW_DLA_LIST);
}

static void
formx_unsigned(Dwarf_Unsigned u, struct esb_s *esbp)
{
     char small_buf[40];
     snprintf(small_buf, sizeof(small_buf),
      "%llu", (unsigned long long)u);
     esb_append(esbp, small_buf);

}
static void
formx_signed(Dwarf_Signed u, struct esb_s *esbp)
{
     char small_buf[40];
     snprintf(small_buf, sizeof(small_buf),
      "%lld", (long long)u);
     esb_append(esbp, small_buf);
}
/* We think this is an integer. Figure out how to print it.
   In case the signedness is ambiguous (such as on 
   DW_FORM_data1 (ie, unknown signedness) print two ways.
*/
static int
formxdata_print_value(Dwarf_Attribute attrib, struct esb_s *esbp,
	Dwarf_Error * err)
{
    Dwarf_Signed tempsd = 0;
    Dwarf_Unsigned tempud = 0;
    int sres = 0;
    int ures = 0;
    Dwarf_Error serr = 0;
    ures = dwarf_formudata(attrib, &tempud, err);
    sres = dwarf_formsdata(attrib, &tempsd, &serr);
    if(ures == DW_DLV_OK) {
      if(sres == DW_DLV_OK) {
	if(tempud == tempsd) {
	   /* Data is the same value, so makes no difference which
		we print. */
	   formx_unsigned(tempud,esbp);
	} else {
	   formx_unsigned(tempud,esbp);
	   esb_append(esbp,"(as signed = ");
	   formx_signed(tempsd,esbp);
	   esb_append(esbp,")");
        }
      } else if (sres == DW_DLV_NO_ENTRY) {
	formx_unsigned(tempud,esbp);
      } else /* DW_DLV_ERROR */{
	formx_unsigned(tempud,esbp);
      }
      return DW_DLV_OK;
    } else  if (ures == DW_DLV_NO_ENTRY) {
      if(sres == DW_DLV_OK) {
	formx_signed(tempsd,esbp);
	return sres;
      } else if (sres == DW_DLV_NO_ENTRY) {
	return sres;
      } else /* DW_DLV_ERROR */{
	*err = serr;
        return sres;
      }
    } 
    /* else ures ==  DW_DLV_ERROR */ 
    if(sres == DW_DLV_OK) {
	formx_signed(tempsd,esbp);
    } else if (sres == DW_DLV_NO_ENTRY) {
	return ures;
    } 
    /* DW_DLV_ERROR */
    return ures;
}


/* Fill buffer with attribute value.
   We pass in tag so we can try to do the right thing with
   broken compiler DW_TAG_enumerator 

   We append to esbp's buffer.

*/
static void
get_attr_value(Dwarf_Debug dbg, Dwarf_Half tag, Dwarf_Attribute attrib,
	       char **srcfiles, Dwarf_Signed cnt, struct esb_s *esbp)
{
    Dwarf_Half theform;
    string temps;
    Dwarf_Block *tempb;
    Dwarf_Signed tempsd = 0;
    Dwarf_Unsigned tempud = 0;
    int i;
    Dwarf_Half attr;
    Dwarf_Off off;
    Dwarf_Die die_for_check;
    Dwarf_Half tag_for_check;
    Dwarf_Bool tempbool;
    Dwarf_Addr addr = 0;
    int fres;
    int bres;
    int wres;
    int dres;
    Dwarf_Half direct_form = 0;
    char small_buf[100];


    fres = dwarf_whatform(attrib, &theform, &err);
    /* depending on the form and the attribute, process the form */
    if (fres == DW_DLV_ERROR) {
	print_error(dbg, "dwarf_whatform cannot find attr form", fres,
		    err);
    } else if (fres == DW_DLV_NO_ENTRY) {
	return;
    }

    dwarf_whatform_direct(attrib, &direct_form, &err);
    /* ignore errors in dwarf_whatform_direct() */


    switch (theform) {
    case DW_FORM_addr:
	bres = dwarf_formaddr(attrib, &addr, &err);
	if (bres == DW_DLV_OK) {
	    snprintf(small_buf, sizeof(small_buf), "%#llx",
		     (unsigned long long) addr);
	    esb_append(esbp, small_buf);
	} else {
	    print_error(dbg, "addr formwith no addr?!", bres, err);
	}
	break;
    case DW_FORM_ref_addr:
	/* DW_FORM_ref_addr is not accessed thru formref: ** it is an
	   address (global section offset) in ** the .debug_info
	   section. */
	bres = dwarf_global_formref(attrib, &off, &err);
	if (bres == DW_DLV_OK) {
	    snprintf(small_buf, sizeof(small_buf),
		     "<global die offset %llu>",
		     (unsigned long long) off);
	    esb_append(esbp, small_buf);
	} else {
	    print_error(dbg,
			"DW_FORM_ref_addr form with no reference?!",
			bres, err);
	}
	break;
    case DW_FORM_ref1:
    case DW_FORM_ref2:
    case DW_FORM_ref4:
    case DW_FORM_ref8:
    case DW_FORM_ref_udata:
	bres = dwarf_formref(attrib, &off, &err);
	if (bres != DW_DLV_OK) {
	    print_error(dbg, "ref formwith no ref?!", bres, err);
	}
	/* do references inside <> to distinguish them ** from
	   constants. In dense form this results in <<>>. Ugly for
	   dense form, but better than ambiguous. davea 9/94 */
	snprintf(small_buf, sizeof(small_buf), "<%llu>", off);
	esb_append(esbp, small_buf);
	if (check_type_offset) {
	    wres = dwarf_whatattr(attrib, &attr, &err);
	    if (wres == DW_DLV_ERROR) {

	    } else if (wres == DW_DLV_NO_ENTRY) {
	    }
	    if (attr == DW_AT_type) {
		dres = dwarf_offdie(dbg, cu_offset + off,
				    &die_for_check, &err);
		type_offset_result.checks++;
		if (dres != DW_DLV_OK) {
		    type_offset_result.errors++;
		    DWARF_CHECK_ERROR
			("DW_AT_type offset does not point to type info")
		} else {
		    int tres2;

		    tres2 =
			dwarf_tag(die_for_check, &tag_for_check, &err);
		    if (tres2 == DW_DLV_OK) {
			switch (tag_for_check) {
			case DW_TAG_array_type:
			case DW_TAG_class_type:
			case DW_TAG_enumeration_type:
			case DW_TAG_pointer_type:
			case DW_TAG_reference_type:
			case DW_TAG_string_type:
			case DW_TAG_structure_type:
			case DW_TAG_subroutine_type:
			case DW_TAG_typedef:
			case DW_TAG_union_type:
			case DW_TAG_ptr_to_member_type:
			case DW_TAG_set_type:
			case DW_TAG_subrange_type:
			case DW_TAG_base_type:
			case DW_TAG_const_type:
			case DW_TAG_file_type:
			case DW_TAG_packed_type:
			case DW_TAG_thrown_type:
			case DW_TAG_volatile_type:
			    /* OK */
			    break;
			default:
			    type_offset_result.errors++;
			    DWARF_CHECK_ERROR
				("DW_AT_type offset does not point to type info")
				break;
			}
			dwarf_dealloc(dbg, die_for_check, DW_DLA_DIE);
		    } else {
			type_offset_result.errors++;
			DWARF_CHECK_ERROR
			    ("DW_AT_type offset does not exist")
		    }
		}
	    }
	}
	break;
    case DW_FORM_block:
    case DW_FORM_block1:
    case DW_FORM_block2:
    case DW_FORM_block4:
	fres = dwarf_formblock(attrib, &tempb, &err);
	if (fres == DW_DLV_OK) {
	    for (i = 0; i < tempb->bl_len; i++) {
		snprintf(small_buf, sizeof(small_buf), "%02x",
			 *(i + (unsigned char *) tempb->bl_data));
		esb_append(esbp, small_buf);
	    }
	    dwarf_dealloc(dbg, tempb, DW_DLA_BLOCK);
	} else {
	    print_error(dbg, "DW_FORM_blockn cannot get block\n", fres,
			err);
	}
	break;
    case DW_FORM_data1:
    case DW_FORM_data2:
    case DW_FORM_data4:
    case DW_FORM_data8:
	fres = dwarf_whatattr(attrib, &attr, &err);
	if (fres == DW_DLV_ERROR) {
	    print_error(dbg, "FORM_datan cannot get attr", fres, err);
	} else if (fres == DW_DLV_NO_ENTRY) {
	    print_error(dbg, "FORM_datan cannot get attr", fres, err);
	} else {
	    switch (attr) {
	    case DW_AT_ordering:
	    case DW_AT_byte_size:
	    case DW_AT_bit_offset:
	    case DW_AT_bit_size:
	    case DW_AT_inline:
	    case DW_AT_language:
	    case DW_AT_visibility:
	    case DW_AT_virtuality:
	    case DW_AT_accessibility:
	    case DW_AT_address_class:
	    case DW_AT_calling_convention:
	    case DW_AT_discr_list:	/* DWARF3 */
	    case DW_AT_encoding:
	    case DW_AT_identifier_case:
	    case DW_AT_MIPS_loop_unroll_factor:
	    case DW_AT_MIPS_software_pipeline_depth:
	    case DW_AT_decl_column:
	    case DW_AT_decl_file:
	    case DW_AT_decl_line:
	    case DW_AT_start_scope:
	    case DW_AT_byte_stride:
	    case DW_AT_bit_stride:
	    case DW_AT_count:
	    case DW_AT_stmt_list:
	    case DW_AT_MIPS_fde:
		wres = get_small_encoding_integer_and_name(dbg,
							   attrib,
							   &tempud,
							   /* attrname */
		    (char *) NULL,
							   /* err_string 
							    */ 
							   (char **)
							   NULL,
							   (encoding_type_func) 0,
							   &err);

		if (wres == DW_DLV_OK) {
		    snprintf(small_buf, sizeof(small_buf), "%llu",
			     tempud);
		    esb_append(esbp, small_buf);
		    if (attr == DW_AT_decl_file) {
			if (srcfiles && tempud > 0 && tempud <= cnt) {
			    /* added by user request */
			    /* srcfiles is indexed starting at 0, but
			       DW_AT_decl_file defines that 0 means no
			       file, so tempud 1 means the 0th entry in
			       srcfiles, thus tempud-1 is the correct
			       index into srcfiles.  */
			    char *fname = srcfiles[tempud - 1];

			    esb_append(esbp, " ");
			    esb_append(esbp, fname);
			}
		    }
		} else {
		    print_error(dbg, "Cannot get encoding attribute ..",
				wres, err);
		}
		break;
	    case DW_AT_const_value:
		wres = formxdata_print_value(attrib,esbp, &err);
		if(wres == DW_DLV_OK){
		    /* String appended already. */
		} else if (wres == DW_DLV_NO_ENTRY) {
		    /* nothing? */
		} else {
		   print_error(dbg,"Cannot get DW_AT_const_value ",wres,err);
		}
  
		
		break;
	    case DW_AT_upper_bound:
	    case DW_AT_lower_bound:
	    default:
		wres = formxdata_print_value(attrib,esbp, &err);
		if (wres == DW_DLV_OK) {
		    /* String appended already. */
		} else if (wres == DW_DLV_NO_ENTRY) {
		    /* nothing? */
		} else {
		    print_error(dbg, "Cannot get formsdata..", wres,
				err);
		}
		break;
	    }
	}
	if (cu_name_flag) {
	    if (attr == DW_AT_MIPS_fde) {
		if (fde_offset_for_cu_low == DW_DLV_BADOFFSET) {
		    fde_offset_for_cu_low
			= fde_offset_for_cu_high = tempud;
		} else if (tempud < fde_offset_for_cu_low) {
		    fde_offset_for_cu_low = tempud;
		} else if (tempud > fde_offset_for_cu_high) {
		    fde_offset_for_cu_high = tempud;
		}
	    }
	}
	break;
    case DW_FORM_sdata:
	wres = dwarf_formsdata(attrib, &tempsd, &err);
	if (wres == DW_DLV_OK) {
	    snprintf(small_buf, sizeof(small_buf), "%lld", tempsd);
	    esb_append(esbp, small_buf);
	} else if (wres == DW_DLV_NO_ENTRY) {
	    /* nothing? */
	} else {
	    print_error(dbg, "Cannot get formsdata..", wres, err);
	}
	break;
    case DW_FORM_udata:
	wres = dwarf_formudata(attrib, &tempud, &err);
	if (wres == DW_DLV_OK) {
	    snprintf(small_buf, sizeof(small_buf), "%llu", tempud);
	    esb_append(esbp, small_buf);
	} else if (wres == DW_DLV_NO_ENTRY) {
	    /* nothing? */
	} else {
	    print_error(dbg, "Cannot get formudata....", wres, err);
	}
	break;
    case DW_FORM_string:
    case DW_FORM_strp:
	wres = dwarf_formstring(attrib, &temps, &err);
	if (wres == DW_DLV_OK) {
	    esb_append(esbp, temps);
	} else if (wres == DW_DLV_NO_ENTRY) {
	    /* nothing? */
	} else {
	    print_error(dbg, "Cannot get formstr/p....", wres, err);
	}

	break;
    case DW_FORM_flag:
	wres = dwarf_formflag(attrib, &tempbool, &err);
	if (wres == DW_DLV_OK) {
	    if (tempbool) {
		snprintf(small_buf, sizeof(small_buf), "yes(%d)",
			 tempbool);
		esb_append(esbp, small_buf);
	    } else {
		snprintf(small_buf, sizeof(small_buf), "no");
		esb_append(esbp, small_buf);
	    }
	} else if (wres == DW_DLV_NO_ENTRY) {
	    /* nothing? */
	} else {
	    print_error(dbg, "Cannot get formflag/p....", wres, err);
	}
	break;
    case DW_FORM_indirect:
	/* We should not ever get here, since the true form was
	   determined and direct_form has the DW_FORM_indirect if it is
	   used here in this attr. */
	esb_append(esbp, get_FORM_name(dbg, theform));
	break;
    default:
	print_error(dbg, "dwarf_whatform unexpected value", DW_DLV_OK,
		    err);
    }
    if (verbose && direct_form && direct_form == DW_FORM_indirect) {
	char *form_indir = " (used DW_FORM_indirect) ";

	esb_append(esbp, form_indir);
    }
}

#include "_tag_attr_table.c"

static int
tag_attr_combination(Dwarf_Half tag, Dwarf_Half attr)
{
    if (attr > 0 && attr < 0x60) {
	return ((tag_attr_combination_table[tag][attr / 0x20]
		 & (1 << (attr % 0x20))) > 0 ? TRUE : FALSE);
    } else if (attr == DW_AT_MIPS_fde) {
	/* no check now */
	return (TRUE);
    } else
	return (FALSE);
}
