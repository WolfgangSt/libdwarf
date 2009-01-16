/*
  Copyright (C) 2000,2002,2003,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007 Sun Microsystems, Inc. All rights reserved.
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
#include "dwarf_elf_access.h"

#ifdef HAVE_ELF_H
#include <elf.h>
#endif
#ifdef __SGI_FAST_LIBELF
#include <libelf_sgi.h>
#else
#ifdef HAVE_LIBELF_H
#include <libelf.h>
#else
#ifdef HAVE_LIBELF_LIBELF_H
#include <libelf/libelf.h>
#endif
#endif
#endif /* !defined(__SGI_FAST_LIBELF) */

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define FALSE	0
#define TRUE	1



#ifdef __SGI_FAST_LIBELF
#else
#ifdef HAVE_ELF64_GETEHDR
extern Elf64_Ehdr *elf64_getehdr(Elf *);
#endif
#ifdef HAVE_ELF64_GETSHDR
extern Elf64_Shdr *elf64_getshdr(Elf_Scn *);
#endif
#endif /* !defined(__SGI_FAST_LIBELF) */

#ifdef __SGI_FAST_LIBELF
/*
	This function translates an elf_sgi error code into a libdwarf
	code.
 */
static int
_dwarf_error_code_from_elf_sgi_error_code(enum elf_sgi_error_type val)
{
    switch (val) {
    case ELF_SGI_ERROR_OK:
	return DW_DLE_NE;
    case ELF_SGI_ERROR_BAD_ALLOC:
	return DW_DLE_MAF;
    case ELF_SGI_ERROR_FORMAT:
	return DW_DLE_MDE;
    case ELF_SGI_ERROR_ERRNO:
	return DW_DLE_IOF;
    case ELF_SGI_ERROR_TOO_BIG:
	return DW_DLE_MOF;
    default:
	return DW_DLE_LEE;
    }
}
#endif


typedef struct {
    dwarf_elf_handle elf;
    int              is_64bit;
    Dwarf_Small      length_size;
    Dwarf_Small      pointer_size;
    Dwarf_Unsigned   section_count;
    Dwarf_Endianness endianness;
    int              libdwarf_owns_elf;
#ifdef __SGI_FAST_LIBELF
    Elf64_Ehdr ehdr;
#else
    Elf32_Ehdr *ehdr32;

#ifdef HAVE_ELF64_GETEHDR
    Elf64_Ehdr *ehdr64;
#endif

#endif /* !defined(__SGI_FAST_LIBELF) */
} dwarf_elf_object_access_internals_t;


/*
    dwarf_elf_object_access_internals_init()
 */
static int 
dwarf_elf_object_access_internals_init(void* obj_in, 
                              dwarf_elf_handle elf, 
                              int* error)
{
    dwarf_elf_object_access_internals_t*obj = 
        (dwarf_elf_object_access_internals_t*)obj_in;
#ifdef __SGI_FAST_LIBELF
    enum elf_sgi_error_type sres;
    unsigned char const *ehdr_ident = 0;
#else
    char *ehdr_ident = 0;
#endif /* !defined(__SGI_FAST_LIBELF) */
    Dwarf_Half machine = 0;
    obj->elf = elf;

#ifdef __SGI_FAST_LIBELF
    sres = elf_sgi_ehdr(elf, &obj->ehdr);
    if (sres != ELF_SGI_ERROR_OK) {
      *error = _dwarf_error_code_from_elf_sgi_error_code(sres);
      return DW_DLV_ERROR;
    }
    ehdr_ident = obj->ehdr.e_ident;
    obj->section_count = obj->ehdr.e_shnum;
    machine = obj->ehdr.e_machine;
#else
    if ((ehdr_ident = elf_getident(elf, NULL)) == NULL) {
      *error = DW_DLE_ELF_GETIDENT_ERROR;
      return DW_DLV_ERROR;
    }
#endif /* !defined(__SGI_FAST_LIBELF) */

    obj->is_64bit = (ehdr_ident[EI_CLASS] == ELFCLASS64);


    if(ehdr_ident[EI_DATA] == ELFDATA2LSB){
      obj->endianness = DW_OBJECT_LSB;
    }
    else if(ehdr_ident[EI_DATA] == ELFDATA2MSB){
      obj->endianness = DW_OBJECT_MSB;
    }

#ifdef __SGI_FAST_LIBELF
    /* We've already loaded the ELF header, so there's nothing to do
       here */
#else
#ifdef HAVE_ELF64_GETEHDR
    if (obj->is_64bit) {
	obj->ehdr64 = elf64_getehdr(elf);
	if (obj->ehdr64 == NULL) {
          *error = DW_DLE_ELF_GETEHDR_ERROR;
          return DW_DLV_ERROR;
	}
	obj->section_count = obj->ehdr64->e_shnum;
	machine = obj->ehdr64->e_machine;
    } else
#endif
    {
	obj->ehdr32 = elf32_getehdr(elf);
	if (obj->ehdr32 == NULL) {
          *error = DW_DLE_ELF_GETEHDR_ERROR;
          return DW_DLV_ERROR;
	}
	obj->section_count = obj->ehdr32->e_shnum;
	machine = obj->ehdr32->e_machine;
    }
#endif /* !defined(__SGI_FAST_LIBELF) */

    /* The following de_length_size is Not Too Significant. Only used
       one calculation, and an approximate one at that. */
    obj->length_size = obj->is_64bit ? 8 : 4;
    obj->pointer_size = obj->is_64bit ? 8 : 4;    

    if (obj->is_64bit && machine != EM_MIPS) {
	/* MIPS/IRIX makes pointer size and length size 8 for -64.
	   Other platforms make length 4 always. */
	/* 4 here supports 32bit-offset dwarf2, as emitted by cygnus
	   tools, and the dwarfv2.1 64bit extension setting. */
	obj->length_size = 4;
    }
    return DW_DLV_OK;
}

/*
    dwarf_elf_object_access_get_byte_order
 */
static
Dwarf_Endianness 
dwarf_elf_object_access_get_byte_order(void* obj_in)
{
    dwarf_elf_object_access_internals_t*obj = 
        (dwarf_elf_object_access_internals_t*)obj_in;
    return obj->endianness;
}

/*
    dwarf_elf_object_access_get_section_count()
 */
static
Dwarf_Unsigned 
dwarf_elf_object_access_get_section_count(void * obj_in)
{
    dwarf_elf_object_access_internals_t*obj = 
        (dwarf_elf_object_access_internals_t*)obj_in;
    return obj->section_count;
}


/*
    dwarf_elf_object_access_get_section()
 */
static 
int 
dwarf_elf_object_access_get_section_info(
    void* obj_in, 
    Dwarf_Half section_index, 
    Dwarf_Obj_Access_Section* ret_scn, int* error)
{
    dwarf_elf_object_access_internals_t*obj = 
        (dwarf_elf_object_access_internals_t*)obj_in;

    {
#ifdef __SGI_FAST_LIBELF
    Elf64_Shdr shdr;
    enum elf_sgi_error_type sres;
#else
    Elf32_Shdr *shdr32 = 0;

#ifdef HAVE_ELF64_GETSHDR
    Elf64_Shdr *shdr64 = 0;
#endif
    Elf_Scn *scn = 0;
#endif /* !defined(__SGI_FAST_LIBELF) */


#ifdef __SGI_FAST_LIBELF
    sres = elf_sgi_shdr(obj->elf, section_index, &shdr);
    if (sres != ELF_SGI_ERROR_OK) {
        *error = _dwarf_error_code_from_elf_sgi_error_code(sres);
        return DW_DLV_ERROR;
    }

    ret_scn->size = shdr.sh_size;
    ret_scn->addr = shdr.sh_addr;

    sres =
        elf_sgi_string(obj->elf, obj->ehdr.e_shstrndx, shdr.sh_name,
                       (char const **) &ret_scn->name);
    if (sres != ELF_SGI_ERROR_OK) {
        *error = _dwarf_error_code_from_elf_sgi_error_code(sres);
        return DW_DLV_ERROR;
    }
#else /* !defined(__SGI_FAST_LIBELF) */
    scn = elf_getscn(obj->elf, section_index);
    if (scn == NULL) {
        *error = DW_DLE_MDE;
        return DW_DLV_ERROR;
    }
#ifdef HAVE_ELF64_GETSHDR
    if (obj->is_64bit) {
        shdr64 = elf64_getshdr(scn);
        if (shdr64 == NULL) {
            *error = DW_DLE_ELF_GETSHDR_ERROR;
            return DW_DLV_ERROR;
        }

        ret_scn->size = shdr64->sh_size;
        ret_scn->addr = shdr64->sh_addr;

        ret_scn->name = elf_strptr(obj->elf, obj->ehdr64->e_shstrndx,
                                        shdr64->sh_name);
        if(ret_scn->name == NULL) {
            *error = DW_DLE_ELF_STRPTR_ERROR;
            return DW_DLV_ERROR;
	}
    } else
#endif /* HAVE_ELF64_GETSHDR */
    {
        if ((shdr32 = elf32_getshdr(scn)) == NULL) {
            *error = DW_DLE_ELF_GETSHDR_ERROR;
            return DW_DLV_ERROR;
	}

	ret_scn->size = shdr32->sh_size;
	ret_scn->addr = shdr32->sh_addr;

	ret_scn->name = elf_strptr(obj->elf, obj->ehdr32->e_shstrndx,
		                        shdr32->sh_name);
	if (ret_scn->name == NULL) {
            *error = DW_DLE_ELF_STRPTR_ERROR;
            return DW_DLV_ERROR;
	}
    }
#endif /* !defined(__SGI_FAST_LIBELF) */
    }
    return DW_DLV_OK;
}

/*
    dwarf_elf_object_access_get_length_size
 */
static
Dwarf_Small 
dwarf_elf_object_access_get_length_size(void* obj_in)
{
    dwarf_elf_object_access_internals_t*obj = 
        (dwarf_elf_object_access_internals_t*)obj_in;
    return obj->length_size;
}

/*
    dwarf_elf_object_access_get_pointer_size
 */
static
Dwarf_Small 
dwarf_elf_object_access_get_pointer_size(void* obj_in)
{
    dwarf_elf_object_access_internals_t*obj = 
        (dwarf_elf_object_access_internals_t*)obj_in;
    return obj->pointer_size;
}

/* 
    dwarf_elf_object_access_load_section
 */
static
int 
dwarf_elf_object_access_load_section(void* obj_in, 
                            Dwarf_Half section_index, 
                            Dwarf_Small** section_data, 
                            int* error)
{
    dwarf_elf_object_access_internals_t*obj = 
        (dwarf_elf_object_access_internals_t*)obj_in;
    if (section_index == 0) {
	return DW_DLV_NO_ENTRY;
    }

    {
#ifdef __SGI_FAST_LIBELF
	enum elf_sgi_error_type sres;

	sres = elf_sgi_section(obj->elf,
			       section_index, (void **) section_data);
	if (sres != ELF_SGI_ERROR_OK) {
            *error = _dwarf_error_code_from_elf_sgi_error_code(sres);
            return DW_DLV_ERROR;
	}
#else
	Elf_Scn *scn;
	Elf_Data *data;

	scn = elf_getscn(obj->elf, section_index);
	if (scn == NULL) {
	    *error = DW_DLE_MDE;
            return DW_DLV_ERROR;
	}

	/* 
	   When using libelf as a producer, section data may be stored
	   in multiple buffers. In libdwarf however, we only use libelf
	   as a consumer (there is a dwarf producer API, but it doesn't
	   use libelf). Because of this, this single call to elf_getdata
	   will retrieve the entire section in a single contiguous
	   buffer. */
	data = elf_getdata(scn, NULL);
	if (data == NULL) {
          *error = DW_DLE_MDE;
          return DW_DLV_ERROR;
	}
	*section_data = data->d_buf;
#endif /* !defined(__SGI_FAST_LIBELF) */
    }
    return DW_DLV_OK;
}


/* dwarf_elf_access method table. */
static const struct Dwarf_Obj_Access_Methods_s dwarf_elf_object_access_methods = 
{
    dwarf_elf_object_access_get_section_info,
    dwarf_elf_object_access_get_byte_order,
    dwarf_elf_object_access_get_length_size,
    dwarf_elf_object_access_get_pointer_size,
    dwarf_elf_object_access_get_section_count,
    dwarf_elf_object_access_load_section
};


/*
    Interface for the ELF object file implementation.
 */
int 
dwarf_elf_object_access_init(dwarf_elf_handle elf, 
    int libdwarf_owns_elf,
    Dwarf_Obj_Access_Interface** ret_obj)
{
    int err = 0;
    int res = 0;
    dwarf_elf_object_access_internals_t *internals = 0;
    Dwarf_Obj_Access_Interface *intfc = 0;

    internals = malloc(sizeof(dwarf_elf_object_access_internals_t));
    if(!internals) {
        /* Impossible case, we hope. Give up. */
        return DW_DLV_ERROR;
    }
    memset(internals,0,sizeof(internals));
    res = dwarf_elf_object_access_internals_init(internals, elf, &err);
    if(res != DW_DLV_OK){
        free(internals);
        return DW_DLV_ERROR;
    }
    internals->libdwarf_owns_elf = libdwarf_owns_elf;
    
    intfc = malloc(sizeof(Dwarf_Obj_Access_Interface));
    if(!intfc) {
        /* Impossible case, we hope. Give up. */
        free(internals);
        return DW_DLV_ERROR;
    }
    /* Initialize the interface struct */
    intfc->object = internals;
    intfc->methods = &dwarf_elf_object_access_methods;

    *ret_obj = intfc;
    return DW_DLV_OK;
}


/*
    Clean up the Dwarf_Obj_Access_Interface returned by elf_access_init.
 */
void 
dwarf_elf_object_access_finish(Dwarf_Obj_Access_Interface* obj)
{
    if(!obj) {
        return;
    }
    if(obj->object) {
        dwarf_elf_object_access_internals_t *internals = 
            (dwarf_elf_object_access_internals_t *)obj->object;
        if(internals->libdwarf_owns_elf){
#ifdef __SGI_FAST_LIBELF
            elf_sgi_free(internals->elf);
#else
            elf_end(internals->elf);
#endif
        }
    }
    free(obj->object);
    free(obj);
}

/*
    This function returns the Elf * pointer
    associated with a Dwarf_Debug.

    This function only makes sense if ELF is implied.
 */
int
dwarf_get_elf(Dwarf_Debug dbg, dwarf_elf_handle * elf,
              Dwarf_Error * error)
{   
    struct Dwarf_Obj_Access_Interface_s * obj = 0;
    if (dbg == NULL) {
        _dwarf_error(NULL, error, DW_DLE_DBG_NULL);
        return (DW_DLV_ERROR);
    }

    obj = dbg->de_obj_file; 
    if(obj) {
       dwarf_elf_object_access_internals_t *internals =
           (dwarf_elf_object_access_internals_t*)obj->object;
       if(internals->elf == NULL) {
           _dwarf_error(dbg, error, DW_DLE_FNO);
           return (DW_DLV_ERROR);
       }
       *elf = internals->elf;
       return DW_DLV_OK;
       
    }
    _dwarf_error(dbg, error, DW_DLE_FNO);
    return DW_DLV_ERROR;
}
    

