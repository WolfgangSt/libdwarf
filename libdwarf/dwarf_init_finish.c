/*

  Copyright (C) 2000, 2002 Silicon Graphics, Inc.  All Rights Reserved.

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



#include "config.h"
#include "dwarf_incl.h"
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

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "dwarf_incl.h"

#define DWARF_DBG_ERROR(dbg,errval,retval) \
     _dwarf_error(dbg, error, errval); return(retval);

#define FALSE	0
#define TRUE	1

#ifdef HAVE_ELF64_GETEHDR
extern Elf64_Ehdr *elf64_getehdr(Elf *);
#endif
#ifdef HAVE_ELF64_GETSHDR
extern Elf64_Shdr *elf64_getshdr(Elf_Scn *);
#endif


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
_dwarf_setup(Dwarf_Debug dbg, Elf * elf, Dwarf_Error * error)
{
    Elf32_Ehdr *ehdr32;

#ifdef HAVE_ELF64_GETEHDR
    Elf64_Ehdr *ehdr64;
#endif
    Elf32_Shdr *shdr32;

#ifdef HAVE_ELF64_GETSHDR
    Elf64_Shdr *shdr64;
#endif
    Elf_Scn *scn;
    Elf_Data *data;
    char *scn_name;
    char *ehdr_ident;
    int is_64bit;
    int foundDwarf;

    foundDwarf = FALSE;
    dbg->de_elf = elf;

    dbg->de_assume_string_in_bounds = _dwarf_assume_string_bad;

    if ((ehdr_ident = elf_getident(elf, NULL)) == NULL) {
	DWARF_DBG_ERROR(dbg, DW_DLE_ELF_GETIDENT_ERROR, DW_DLV_ERROR);
    }

    is_64bit = (ehdr_ident[EI_CLASS] == ELFCLASS64);


    dbg->de_same_endian = 1;
    dbg->de_copy_word = memcpy;
#ifdef WORDS_BIGENDIAN
    if (ehdr_ident[EI_DATA] == ELFDATA2LSB) {
	dbg->de_same_endian = 0;
	dbg->de_copy_word = _dwarf_memcpy_swap_bytes;
    }
#else /* little endian */
    if (ehdr_ident[EI_DATA] == ELFDATA2MSB) {
	dbg->de_same_endian = 0;
	dbg->de_copy_word = _dwarf_memcpy_swap_bytes;
    }
#endif /* !WORDS_BIGENDIAN */

    /* The following de_length_size is later modified in this routine
       in the important cases. To allow cygnus 32bit-offset dwarf2. */
    dbg->de_length_size = is_64bit ? 8 : 4;
    dbg->de_pointer_size = is_64bit ? 8 : 4;


#ifdef HAVE_ELF64_GETEHDR
    if (is_64bit) {
	ehdr64 = elf64_getehdr(elf);
	if (ehdr64 == NULL) {
	    DWARF_DBG_ERROR(dbg, DW_DLE_ELF_GETEHDR_ERROR,
			    DW_DLV_ERROR);
	}
	if (ehdr64->e_machine != EM_MIPS) {
	    /* MIPS/IRIX makes pointer size and length size 8 for -64.
	       Other platforms make length 4 always. */
	    /* 4 here supports 32bit-offset dwarf2, as emitted by
	       cygnus tools, and the dwarfv2.1 64bit extension setting. */
	    dbg->de_length_size = 4;
	}
    } else
#endif
    {
	ehdr32 = elf32_getehdr(elf);
	if (ehdr32 == NULL) {
	    DWARF_DBG_ERROR(dbg, DW_DLE_ELF_GETEHDR_ERROR,
			    DW_DLV_ERROR);
	}
    }

    scn = NULL;
    while ((scn = elf_nextscn(elf, scn)) != NULL) {

#ifdef HAVE_ELF64_GETSHDR
	if (is_64bit) {
	    shdr64 = elf64_getshdr(scn);
	    if (shdr64 == NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_ELF_GETSHDR_ERROR,
				DW_DLV_ERROR);
	    }
	    if ((scn_name = elf_strptr(elf, ehdr64->e_shstrndx,
				       shdr64->sh_name))
		== NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_ELF_STRPTR_ERROR,
				DW_DLV_ERROR);
	    }
	} else
#endif
	{
	    if ((shdr32 = elf32_getshdr(scn)) == NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_ELF_GETSHDR_ERROR, 0);
	    }
	    if ((scn_name = elf_strptr(elf, ehdr32->e_shstrndx,
				       shdr32->sh_name)) == NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_ELF_STRPTR_ERROR,
				DW_DLV_ERROR);
	    }
	}

	if (strncmp(scn_name, ".debug_", 7)
	    && strcmp(scn_name, ".eh_frame")
	    )
	    continue;

	else if (strcmp(scn_name, ".debug_info") == 0) {
	    if (dbg->de_debug_info != NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_INFO_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL ||
		data->d_size == 0) {
		/* Know no reason to allow empty debug_info section */
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_INFO_NULL,
				DW_DLV_ERROR);
	    }
	    dbg->de_debug_info = data->d_buf;
	    dbg->de_debug_info_size = data->d_size;
	    foundDwarf = TRUE;
/*
	    Here we'll try to determine if this is
            true 64 bit (sgi) or cygnus 32bit-offset (pure dwarf2 v2.0.0)
	    by looking for the dwarf version number (which is 2).

	    If de_length_size winds up 4 we can still read extensions,
	    per the dwarf2 offset extensions rule, added to dwarf2
	    in late 1999.
*/
	    if (is_64bit && data->d_size > 10) {
		Dwarf_Half v1;
		Dwarf_Half v64;
		Dwarf_Unsigned length;

		READ_UNALIGNED(dbg, length, Dwarf_Unsigned,
			       dbg->de_debug_info,
			       sizeof(Dwarf_Unsigned));
		READ_UNALIGNED(dbg, v1, Dwarf_Half,
			       dbg->de_debug_info + 4,
			       sizeof(Dwarf_Half));
		READ_UNALIGNED(dbg, v64, Dwarf_Half,
			       dbg->de_debug_info + 8,
			       sizeof(Dwarf_Half));
		if (length == DISTINGUISHED_VALUE) {
		    /* Using the dwarf2 1999 offset extension facility.
		       This field is not used in this case */
		    dbg->de_length_size = 4;
		} else if (v1 == 2) {
		    if (v64 == 2) {
			/* Ambiguous! */
			/* Assume we have it right already. */
		    } else {
			/* Assume is cygnus 32bit-offset, otherwise
			   this is junk anyway. */
			dbg->de_length_size = 4;
		    }
		} else if (v64 == 2) {
		    /* Definitely sgi true 64 bit */
		    dbg->de_length_size = 8;
		} else {
		    /* Impossible: wrong version. */
		    /* Error caught later. */
		}

	    }
	}

	else if (strcmp(scn_name, ".debug_abbrev") == 0) {
	    if (dbg->de_debug_abbrev != NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_ABBREV_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL ||
		data->d_size == 0) {
		/* Know no reason to allow empty debug_abbrev section */
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_ABBREV_NULL,
				DW_DLV_ERROR);
	    }
	    dbg->de_debug_abbrev = data->d_buf;
	    dbg->de_debug_abbrev_size = data->d_size;
	}

	else if (strcmp(scn_name, ".debug_aranges") == 0) {
	    if (dbg->de_debug_aranges != NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_ARANGES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_ARANGES_NULL,
				DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_aranges = data->d_buf;
	    dbg->de_debug_aranges_size = data->d_size;
	}

	else if (strcmp(scn_name, ".debug_line") == 0) {
	    if (dbg->de_debug_line != NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_LINE_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_LINE_NULL, DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_line = data->d_buf;
	    dbg->de_debug_line_size = data->d_size;
	}

	else if (strcmp(scn_name, ".debug_frame") == 0) {
	    if (dbg->de_debug_frame != NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_FRAME_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_FRAME_NULL, DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_frame = data->d_buf;
	    dbg->de_debug_frame_size = data->d_size;
	    foundDwarf = TRUE;
	} else if (strcmp(scn_name, ".eh_frame") == 0) {
	    /* gnu egcs-1.1.2 data */
	    if (dbg->de_debug_frame != NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_FRAME_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_FRAME_NULL, DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_frame_eh_gnu = data->d_buf;
	    dbg->de_debug_frame_size_eh_gnu = data->d_size;
	    foundDwarf = TRUE;
	}

	else if (strcmp(scn_name, ".debug_loc") == 0) {
	    if (dbg->de_debug_loc != NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_LOC_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_LOC_NULL,
				DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_loc = data->d_buf;
	    dbg->de_debug_loc_size = data->d_size;
	}


	else if (strcmp(scn_name, ".debug_pubnames") == 0) {
	    if (dbg->de_debug_pubnames != NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_PUBNAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_PUBNAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_pubnames = data->d_buf;
	    dbg->de_debug_pubnames_size = data->d_size;
	}

	else if (strcmp(scn_name, ".debug_str") == 0) {
	    if (dbg->de_debug_str != NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_STR_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_STR_NULL,
				DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_str = data->d_buf;
	    dbg->de_debug_str_size = data->d_size;
	}

	else if (strcmp(scn_name, ".debug_funcnames") == 0) {
	    if (dbg->de_debug_funcnames != NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_FUNCNAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_FUNCNAMES_NULL,
				DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_funcnames = data->d_buf;
	    dbg->de_debug_funcnames_size = data->d_size;
	}

	else if (strcmp(scn_name, ".debug_typenames") == 0) {
	    if (dbg->de_debug_typenames != NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_TYPENAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_TYPENAMES_NULL,
				DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_typenames = data->d_buf;
	    dbg->de_debug_typenames_size = data->d_size;
	}

	else if (strcmp(scn_name, ".debug_varnames") == 0) {
	    if (dbg->de_debug_varnames != NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_VARNAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_VARNAMES_NULL,
				DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_varnames = data->d_buf;
	    dbg->de_debug_varnames_size = data->d_size;
	}

	else if (strcmp(scn_name, ".debug_weaknames") == 0) {
	    if (dbg->de_debug_weaknames != NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_WEAKNAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_WEAKNAMES_NULL,
				DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_weaknames = data->d_buf;
	    dbg->de_debug_weaknames_size = data->d_size;
	} else if (strcmp(scn_name, ".debug_macinfo") == 0) {
	    if (dbg->de_debug_macinfo != NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_MACINFO_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if ((data = elf_getdata(scn, 0)) == NULL) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_MACINFO_NULL,
				DW_DLV_ERROR);
	    }
	    if (data->d_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_macinfo = data->d_buf;
	    dbg->de_debug_macinfo_size = data->d_size;
	}
    }
    if (foundDwarf) {
	return DW_DLV_OK;
    }

    return (DW_DLV_NO_ENTRY);
}


/*
    The basic dwarf initializer function for consumers.
    Return NULL on error.
*/
int
dwarf_init(int fd,
	   Dwarf_Unsigned access,
	   Dwarf_Handler errhand,
	   Dwarf_Ptr errarg, Dwarf_Debug * ret_dbg, Dwarf_Error * error)
{
    Dwarf_Debug dbg;
    struct stat fstat_buf;
    Elf *elf;
    int res;
    Elf_Cmd what_kind_of_elf_read;

    dbg = _dwarf_get_debug();
    if (dbg == NULL) {
	DWARF_DBG_ERROR(dbg, DW_DLE_DBG_ALLOC, DW_DLV_ERROR);
    }
    dbg->de_errhand = errhand;
    dbg->de_errarg = errarg;

    if (fstat(fd, &fstat_buf) != 0) {
	DWARF_DBG_ERROR(dbg, DW_DLE_FSTAT_ERROR, DW_DLV_ERROR);
    }
    if (!S_ISREG(fstat_buf.st_mode)) {
	DWARF_DBG_ERROR(dbg, DW_DLE_FSTAT_MODE_ERROR, DW_DLV_ERROR);
    }

    if (access != DW_DLC_READ) {
	DWARF_DBG_ERROR(dbg, DW_DLE_INIT_ACCESS_WRONG, DW_DLV_ERROR);
    }
    dbg->de_access = access;

    elf_version(EV_CURRENT);
    /* changed to mmap request per bug 281217. 6/95 */
#ifdef HAVE_ELF_C_READ_MMAP
    /* ELF_C_READ_MMAP is an SGI IRIX specific enum value from IRIX
       libelf.h meaning read but use mmap */
    what_kind_of_elf_read = ELF_C_READ_MMAP;
#else
    /* ELF_C_READ is a portable value */
    what_kind_of_elf_read  = ELF_C_READ;
#endif

    if ((elf = elf_begin(fd, what_kind_of_elf_read, 0)) == NULL) {
	DWARF_DBG_ERROR(dbg, DW_DLE_ELF_BEGIN_ERROR, DW_DLV_ERROR);
    }

    dbg->de_elf_must_close = 1;
    if ((res = _dwarf_setup(dbg, elf, error)) != DW_DLV_OK) {
	free(dbg);
	return (res);
    }

    /* call cannot fail: no malloc or free involved */
    _dwarf_setup_debug(dbg);

    *ret_dbg = dbg;
    return (DW_DLV_OK);
}


/*
    The alternate dwarf setup call for consumers
*/
int
dwarf_elf_init(Elf * elf_file_pointer,
	       Dwarf_Unsigned access,
	       Dwarf_Handler errhand,
	       Dwarf_Ptr errarg,
	       Dwarf_Debug * ret_dbg, Dwarf_Error * error)
{
    Dwarf_Debug dbg;
    int res;

    dbg = _dwarf_get_debug();
    if (dbg == NULL) {
	DWARF_DBG_ERROR(dbg, DW_DLE_DBG_ALLOC, DW_DLV_ERROR);
    }
    dbg->de_errhand = errhand;
    dbg->de_errarg = errarg;

    if (access != DW_DLC_READ) {
	DWARF_DBG_ERROR(dbg, DW_DLE_INIT_ACCESS_WRONG, DW_DLV_ERROR);
    }
    dbg->de_access = access;

    dbg->de_elf_must_close = 0;
    if ((res = _dwarf_setup(dbg, elf_file_pointer, error)) != DW_DLV_OK) {
	free(dbg);
	return (res);
    }

    /* this call cannot fail: allocates nothing, releases nothing */
    _dwarf_setup_debug(dbg);

    *ret_dbg = dbg;
    return (DW_DLV_OK);
}


/*
	Frees all memory that was not previously freed
	by dwarf_dealloc.
	Aside from certain categories.
*/
int
dwarf_finish(Dwarf_Debug dbg, Dwarf_Error * error)
{
    int res = DW_DLV_OK;
    if(dbg->de_elf_must_close) {
	/* Must do this *before* _dwarf_free_all_of_one_debug()
	   as that zeroes out dbg contents
	*/
	elf_end(dbg->de_elf);
    }

    res = _dwarf_free_all_of_one_debug(dbg);
    if (res == DW_DLV_ERROR) {
	DWARF_DBG_ERROR(dbg, DW_DLE_DBG_ALLOC, DW_DLV_ERROR);
    }

    return res;


}


/*
    This function returns the Elf * pointer
    associated with a Dwarf_Debug.
*/
int
dwarf_get_elf(Dwarf_Debug dbg, Elf ** elf, Dwarf_Error * error)
{
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    *elf = dbg->de_elf;
    return (DW_DLV_OK);
}
