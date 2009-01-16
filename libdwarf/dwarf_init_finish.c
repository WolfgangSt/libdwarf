/*

  Copyright (C) 2000,2002,2003,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2008 Arxan Technologies, Inc.  All Rights Reserved.

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
  Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston MA 02110-1301,
  USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/

#include "config.h"
#include "dwarf_incl.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "dwarf_incl.h"
#include "malloc_check.h"

#define DWARF_DBG_ERROR(dbg,errval,retval) \
     _dwarf_error(dbg, error, errval); return(retval);

#define FALSE 0
#define TRUE  1



/* This static is copied to the dbg on dbg init
   so that the static need not be referenced at
   run time, preserving better locality of
   reference.
   Value is 0 means do the string check.
   Value non-zero means do not do the check.
*/
static Dwarf_Small _dwarf_assume_string_bad;


int
dwarf_set_stringcheck(int newval)
{
    int oldval = _dwarf_assume_string_bad;

    _dwarf_assume_string_bad = newval;
    return oldval;
}


/*
    Given an Elf ptr, set up dbg with pointers
    to all the Dwarf data sections.
    Return NULL on error.

    This function is also responsible for determining
    whether the given object contains Dwarf information
    or not.  The test currently used is that it contains
    either a .debug_info or a .debug_frame section.  If 
    not, it returns DW_DLV_NO_ENTRY causing dwarf_init() also to 
    return DW_DLV_NO_ENTRY.  Earlier, we had thought of using only 
    the presence/absence of .debug_info to test, but we 
    added .debug_frame since there could be stripped objects 
    that have only a .debug_frame section for exception 
    processing.
    DW_DLV_NO_ENTRY or DW_DLV_OK or DW_DLV_ERROR
*/
static int
_dwarf_setup(Dwarf_Debug dbg, Dwarf_Error * error)
{
    const char *scn_name = 0;
    int foundDwarf = 0;
    struct Dwarf_Obj_Access_Interface_s * obj = 0;

    Dwarf_Endianness endianness;

    Dwarf_Unsigned section_size = 0;
    Dwarf_Unsigned section_count = 0;
    Dwarf_Half section_index = 0;
    Dwarf_Addr section_addr = 0;

    foundDwarf = FALSE;

    dbg->de_assume_string_in_bounds = _dwarf_assume_string_bad;

    dbg->de_same_endian = 1;
    dbg->de_copy_word = memcpy;
    obj = dbg->de_obj_file;
    endianness = obj->methods->get_byte_order(obj->object);
#ifdef WORDS_BIGENDIAN
    dbg->de_big_endian_object = 1;
    if (endianness == DW_OBJECT_LSB ) {
        dbg->de_same_endian = 0;
        dbg->de_big_endian_object = 0;
        dbg->de_copy_word = _dwarf_memcpy_swap_bytes;
    }
#else /* little endian */
    dbg->de_big_endian_object = 0;
    if (endianness == DW_OBJECT_MSB ) {
        dbg->de_same_endian = 0;
        dbg->de_big_endian_object = 1;
        dbg->de_copy_word = _dwarf_memcpy_swap_bytes;
    }
#endif /* !WORDS_BIGENDIAN */


    /* The following de_length_size is Not Too Significant. Only used
       one calculation, and an approximate one at that. */
    dbg->de_length_size = obj->methods->get_length_size(obj->object);
    dbg->de_pointer_size = obj->methods->get_pointer_size(obj->object);

    section_count = obj->methods->get_section_count(obj->object);

    /* We can skip index 0 when considering ELF files, but not other
       object types. */
    for (section_index = 0; section_index < section_count;
         ++section_index) {
        
        struct Dwarf_Obj_Access_Section_s doas;
        Dwarf_Error section_error;
        int res;
        int err;

        res = obj->methods->get_section_info(obj->object, 
                                             section_index, 
                                             &doas, &err);
        if(res == DW_DLV_ERROR){
          DWARF_DBG_ERROR(dbg, err, DW_DLV_ERROR);
        }

        section_addr = doas.addr;
        section_size = doas.size;
        scn_name = doas.name;

        if (strncmp(scn_name, ".debug_", 7)
            && strcmp(scn_name, ".eh_frame")
            )
            continue;

        else if (strcmp(scn_name, ".debug_info") == 0) {
            if (dbg->de_debug_info != NULL) {
                DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_INFO_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* Know no reason to allow empty debug_info section */
                DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_INFO_NULL,
                                DW_DLV_ERROR);
            }
            foundDwarf = TRUE;
            dbg->de_debug_info_index = section_index;
            dbg->de_debug_info_size = section_size;
        }

        else if (strcmp(scn_name, ".debug_abbrev") == 0) {
            if (dbg->de_debug_abbrev != NULL) {
                DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_ABBREV_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* Know no reason to allow empty debug_abbrev section */
                DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_ABBREV_NULL,
                                DW_DLV_ERROR);
            }
            dbg->de_debug_abbrev_index = section_index;
            dbg->de_debug_abbrev_size = section_size;
        }

        else if (strcmp(scn_name, ".debug_aranges") == 0) {
            if (dbg->de_debug_aranges_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_ARANGES_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_aranges_index = section_index;
            dbg->de_debug_aranges_size = section_size;
        }

        else if (strcmp(scn_name, ".debug_line") == 0) {
            if (dbg->de_debug_line_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_LINE_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_line_index = section_index;
            dbg->de_debug_line_size = section_size;
        }

        else if (strcmp(scn_name, ".debug_frame") == 0) {
            if (dbg->de_debug_frame_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_FRAME_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_frame_index = section_index;
            dbg->de_debug_frame_size = section_size;
            foundDwarf = TRUE;
        } else if (strcmp(scn_name, ".eh_frame") == 0) {
            /* gnu egcs-1.1.2 data */
            if (dbg->de_debug_frame_eh_gnu_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_FRAME_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_frame_eh_gnu_index = section_index;
            dbg->de_debug_frame_size_eh_gnu = section_size;
            dbg->de_debug_frame_eh_addr = section_addr;
            foundDwarf = TRUE;
        }

        else if (strcmp(scn_name, ".debug_loc") == 0) {
            if (dbg->de_debug_loc_index != 0) {
                DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_LOC_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_loc_index = section_index;
            dbg->de_debug_loc_size = section_size;
        }


        else if (strcmp(scn_name, ".debug_pubnames") == 0) {
            if (dbg->de_debug_pubnames_index != 0) {
                DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_PUBNAMES_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_pubnames_index = section_index;
            dbg->de_debug_pubnames_size = section_size;
        }

        else if (strcmp(scn_name, ".debug_str") == 0) {
            if (dbg->de_debug_str_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_STR_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_str_index = section_index;
            dbg->de_debug_str_size = section_size;
        }

        else if (strcmp(scn_name, ".debug_funcnames") == 0) {
            if (dbg->de_debug_funcnames_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_FUNCNAMES_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_funcnames_index = section_index;
            dbg->de_debug_funcnames_size = section_size;
        }

        else if (strcmp(scn_name, ".debug_typenames") == 0) {
            /* SGI IRIX-only, created years before DWARF3. Content
               essentially identical to .debug_pubtypes.  */
            if (dbg->de_debug_typenames_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_TYPENAMES_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_typenames_index = section_index;
            dbg->de_debug_typenames_size = section_size;
        } else if (strcmp(scn_name, ".debug_pubtypes") == 0) {
            /* Section new in DWARF3.  */
            if (dbg->de_debug_pubtypes_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_PUBTYPES_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_pubtypes_index = section_index;
            dbg->de_debug_pubtypes_size = section_size;
        }

        else if (strcmp(scn_name, ".debug_varnames") == 0) {
            if (dbg->de_debug_varnames_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_VARNAMES_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_varnames_index = section_index;
            dbg->de_debug_varnames_size = section_size;
        }

        else if (strcmp(scn_name, ".debug_weaknames") == 0) {
            if (dbg->de_debug_weaknames_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_WEAKNAMES_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_weaknames_index = section_index;
            dbg->de_debug_weaknames_size = section_size;
        } else if (strcmp(scn_name, ".debug_macinfo") == 0) {
            if (dbg->de_debug_macinfo_index != 0) {
                DWARF_DBG_ERROR(dbg,
                                DW_DLE_DEBUG_MACINFO_DUPLICATE,
                                DW_DLV_ERROR);
            }
            if (section_size == 0) {
                /* a zero size section is just empty. Ok, no error */
                continue;
            }
            dbg->de_debug_macinfo_index = section_index;
            dbg->de_debug_macinfo_size = section_size;
        }
    }
    if (foundDwarf) {
        return DW_DLV_OK;
    }
    return (DW_DLV_NO_ENTRY);
}


/*
    Use a Dwarf_Obj_Access_Interface to kick things off. All other 
    init routines eventually use this one.
    The returned Dwarf_Debug contains a copy of *obj
    the callers copy of *obj may be freed whenever the caller
    wishes.
*/
int 
dwarf_object_init(Dwarf_Obj_Access_Interface* obj, Dwarf_Handler errhand,
               Dwarf_Ptr errarg, Dwarf_Debug* ret_dbg, 
               Dwarf_Error* error)
{
    Dwarf_Debug dbg = 0;
    int setup_result = DW_DLV_OK;

    dbg = _dwarf_get_debug();
    if (dbg == NULL) {
        DWARF_DBG_ERROR(dbg, DW_DLE_DBG_ALLOC, DW_DLV_ERROR);
    }
    dbg->de_errhand = errhand;
    dbg->de_errarg = errarg;
    dbg->de_frame_rule_initial_value = DW_FRAME_REG_INITIAL_VALUE;
    dbg->de_frame_reg_rules_entry_count = DW_FRAME_LAST_REG_NUM;

    dbg->de_obj_file = obj;

    setup_result = _dwarf_setup(dbg, error);
    if (setup_result != DW_DLV_OK) {
        /* The status we want to return  here is of _dwarf_setup,
           not of the  _dwarf_free_all_of_one_debug(dbg) call. 
           So use a local status variable for the free.  */
        int freeresult = _dwarf_free_all_of_one_debug(dbg);
        if (freeresult == DW_DLV_ERROR) {
            DWARF_DBG_ERROR(dbg, DW_DLE_DBG_ALLOC, DW_DLV_ERROR);
        }
        dwarf_malloc_check_complete("After Final free");
        return setup_result;
    }

    /* This call cannot fail: allocates nothing, releases nothing */
    _dwarf_setup_debug(dbg);


    *ret_dbg = dbg;
    return DW_DLV_OK;    
}


/*
    A finish routine that is completely unaware of ELF.

    Frees all memory that was not previously freed by
    dwarf_dealloc.
    Aside frmo certain categories.
 */
int 
dwarf_object_finish(Dwarf_Debug dbg, Dwarf_Error * error)
{
    int res = DW_DLV_OK;

    res = _dwarf_free_all_of_one_debug(dbg);
    if (res == DW_DLV_ERROR) {
        DWARF_DBG_ERROR(dbg, DW_DLE_DBG_ALLOC, DW_DLV_ERROR);
    }
    dwarf_malloc_check_complete("After Final free");

    return res;  
}


/*
    Load the ELF section with the specified index and set the
    pointer pointed to by section_data to the memory where it
    was loaded.
 */
int
_dwarf_load_section(Dwarf_Debug dbg,
    Dwarf_Half section_index,
    Dwarf_Small ** section_data, Dwarf_Error * error)
{
    int res;
    int err;

    /* check to see if the section is already loaded */
    if (*section_data != NULL) {
        return DW_DLV_OK;
    }

    {
        struct Dwarf_Obj_Access_Interface_s *o = dbg->de_obj_file; 
        res = o->methods->load_section(
                  o->object, section_index, 
                  section_data, &err);
        if(res == DW_DLV_ERROR){
            DWARF_DBG_ERROR(dbg, err, DW_DLV_ERROR);
        }
    }

    return res;
}

/* This is a hack so clients can verify offsets.
   Added April 2005 so that debugger can detect broken offsets
   (which happened in an IRIX  -64 executable larger than 2GB
    using MIPSpro 7.3.1.3 compilers. A couple .debug_pubnames
    offsets were wrong.).
*/
int
dwarf_get_section_max_offsets(Dwarf_Debug dbg,
    Dwarf_Unsigned * debug_info_size,
    Dwarf_Unsigned * debug_abbrev_size,
    Dwarf_Unsigned * debug_line_size,
    Dwarf_Unsigned * debug_loc_size,
    Dwarf_Unsigned * debug_aranges_size,
    Dwarf_Unsigned * debug_macinfo_size,
    Dwarf_Unsigned * debug_pubnames_size,
    Dwarf_Unsigned * debug_str_size,
    Dwarf_Unsigned * debug_frame_size,
    Dwarf_Unsigned * debug_ranges_size,
    Dwarf_Unsigned * debug_typenames_size)
{
    *debug_info_size = dbg->de_debug_info_size;
    *debug_abbrev_size = dbg->de_debug_abbrev_size;
    *debug_line_size = dbg->de_debug_line_size;
    *debug_loc_size = dbg->de_debug_loc_size;
    *debug_aranges_size = dbg->de_debug_aranges_size;
    *debug_macinfo_size = dbg->de_debug_macinfo_size;
    *debug_pubnames_size = dbg->de_debug_pubnames_size;
    *debug_str_size = dbg->de_debug_str_size;
    *debug_frame_size = dbg->de_debug_frame_size;
    *debug_ranges_size = 0;  /* Not yet supported. */
    *debug_typenames_size = dbg->de_debug_typenames_size;


    return DW_DLV_OK;
}
