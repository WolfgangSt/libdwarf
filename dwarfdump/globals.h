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


$Header: /isms/cmplrs.src/osprey1.0/dwarfdump/RCS/globals.h,v 1.18 1999/06/22 16:33:19 davea Exp $ */
#ifndef globals_INCLUDED
#define globals_INCLUDED

#include "config.h"

/* We want __uint32_t and __uint64_t and __int32_t __int64_t
   properly defined but not duplicated, since duplicate typedefs
   are not legal C.
*/
/*
 HAVE___UINT32_T
 HAVE___UINT64_T will be set by configure if
 our 4 types are predefined in compiler
*/


#if (!defined(HAVE___UINT32_T)) && defined(HAVE_SGIDEFS_H)
#include <sgidefs.h> /* sgidefs.h defines them */
#define HAVE___UINT32_T 1
#define HAVE___UINT64_T 1
#endif



#if (!defined(HAVE___UINT32_T)) && defined(HAVE_SYS_TYPES_H) && defined(HAVE___UINT32_T_IN_SYS_TYPES_H)
#  include <sys/types.h>
/* we assume __[u]int32_t and __[u]int64_t defined 
   since __uint32_t defined in the sys/types.h in use */
#define HAVE___UINT32_T 1
#define HAVE___UINT64_T 1
#endif

#ifndef HAVE___UINT32_T
typedef int __int32_t;
typedef unsigned  __uint32_t;
#define HAVE___UINT32_T 1
#endif
#ifndef HAVE___UINT64_T
typedef long long __int64_t;
typedef unsigned long long  __uint64_t;
#define HAVE___UINT64_T 1
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_ELF_H
#include <elf.h>
#endif
#ifdef HAVE_LIBELF_H
#include <libelf.h>
#else
#ifdef HAVE_LIBELF_LIBELF_H
#include <libelf/libelf.h>
#endif
#endif
#include <dwarf.h>
#include <libdwarf.h>
/* DST* names are sgi-centric and not generally useful,
   so they are just stubbed out in the following defines.
*/
#define DST_FILE_IDX int
#define DST_DIR_IDX  int
#define DST_INFO_IDX int

typedef char * string;
typedef int boolean;
#ifndef FALSE
#define FALSE 0
#endif 
#ifndef TRUE
#define TRUE 1
#endif

/* size of attrib_buffer, defined in print_die.c */
#define ATTRIB_BUFSIZ 999

typedef struct {
    int checks;
    int errors;
} Dwarf_Check_Result;

extern int verbose;
extern boolean dense;
extern boolean ellipsis;
extern boolean dst_format;

extern boolean check_pubname_attr;
extern boolean check_attr_tag;
extern boolean check_tag_tree;
extern boolean check_type_offset;

extern Dwarf_Check_Result abbrev_code_result;
extern Dwarf_Check_Result pubname_attr_result;
extern Dwarf_Check_Result reloc_offset_result;
extern Dwarf_Check_Result attr_tag_result;
extern Dwarf_Check_Result tag_tree_result;
extern Dwarf_Check_Result type_offset_result;

extern boolean info_flag;

extern char cu_name[ ];
extern boolean cu_name_flag;
extern Dwarf_Unsigned cu_offset;
extern Dwarf_Off fde_offset_for_cu_low;
extern Dwarf_Off fde_offset_for_cu_high;

extern DST_FILE_IDX file_name_idx;
extern DST_DIR_IDX  incl_dir_idx;
extern DST_INFO_IDX null_idx;

extern int check_error;
extern Dwarf_Error err;
extern void print_error (Dwarf_Debug dbg, string msg,int res, Dwarf_Error err);

extern void print_line_numbers_this_cu (Dwarf_Debug dbg, Dwarf_Die in_die);
extern void print_frames (Dwarf_Debug dbg);
extern void print_pubnames (Dwarf_Debug dbg);
extern void print_macinfo (Dwarf_Debug dbg);
extern void print_locs (Dwarf_Debug dbg);
extern void print_abbrevs (Dwarf_Debug dbg);
extern void print_strings (Dwarf_Debug dbg);
extern void print_aranges (Dwarf_Debug dbg);
extern void print_relocinfo (Dwarf_Debug dbg);
extern void print_static_funcs(Dwarf_Debug dbg);
extern void print_static_vars(Dwarf_Debug dbg);
extern void print_types(Dwarf_Debug dbg);
extern void print_weaknames(Dwarf_Debug dbg);
extern void print_exception_tables(Dwarf_Debug dbg);
extern string get_fde_proc_name(Dwarf_Debug dbg, Dwarf_Addr low_pc);


#define DWARF_CHECK_ERROR(str) {\
	printf("*** DWARF CHECK: %s ***\n", str);\
	check_error ++; \
}

#define DWARF_CHECK_ERROR2(str1, str2) {\
	printf("*** DWARF CHECK: %s: %s ***\n", str1, str2);\
	check_error ++; \
}

#endif /* globals_INCLUDED */
