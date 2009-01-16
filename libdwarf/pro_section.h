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

	pro_section.h 
	$Revision: 1.15 $    $Date: 1999/04/20 16:43:47 $    
	$Source: /isms/cmplrs.src/osprey1.0/libdwarf/RCS/pro_section.h,v $

        For number/macro-name correspondence, see pro_opaque.h
*/



/* relocation section names */
extern char * _dwarf_rel_section_names[];
/* section names */
extern char * _dwarf_sectnames[];

/* struct to hold relocation entries. Its mantained as a linked
   list of relocation structs, and will then be written at as a 
   whole into the relocation section. Whether its 32 bit or
   64 bit will be obtained from Dwarf_Debug pointer.
*/





/*
	struct stores a chunk of data pertaining to a section 
*/
struct Dwarf_P_Section_Data_s {
    int 		  ds_elf_sect_no; /* elf section number */
    char 		 *ds_data;        /* data contained in section */
    unsigned long	  ds_nbytes;      /* bytes of data 
					used so far*/
    unsigned long	  ds_orig_alloc;  /* bytes allocated 
					originally */
    Dwarf_P_Section_Data  ds_next;        /* next on the list */
};

/* Used to allow a dummy initial struct (which we
   drop before it gets used
   This must not match any legitimate 'section' number.
*/
#define MAGIC_SECT_NO -3

/* Size of chunk of data allocated in one alloc 
   Not clear if this is the best size.
   Used to be just 4096 for user data, the section data struct
   was a separate malloc.
*/
#define 	CHUNK_SIZE (4096 - sizeof (struct Dwarf_P_Section_Data_s))

/*
	chunk alloc routine - 
	if chunk->ds_data is nil, it will alloc CHUNK_SIZE bytes, 
	and return pointer to the beginning. If chunk is not nil, 
	it will see if there's enoungh space for nbytes in current 
	chunk, if not, add new chunk to linked list, and return 
	a char * pointer to it. Return null if unsuccessful.
*/
Dwarf_Small * _dwarf_pro_buffer(Dwarf_P_Debug dbg,int sectno, 
		unsigned long nbytes );

#define GET_CHUNK(dbg,sectno,ptr,nbytes,error) \
	{ \
	    (ptr) = _dwarf_pro_buffer((dbg),(sectno),(nbytes)); \
	    if ((ptr) == NULL) { \
		DWARF_P_DBG_ERROR(dbg,DW_DLE_CHUNK_ALLOC,-1); \
	    } \
	}



int
_dwarf_transform_arange_to_disk (
    Dwarf_P_Debug       dbg,
    Dwarf_Error         *error
);

/* These are for creating ELF section type codes.
*/
#if defined(linux) || defined(__BEOS__) || !defined(SHT_MIPS_DWARF)
/* Intel's SoftSdv accepts only this */
#define SECTION_TYPE            SHT_PROGBITS 
#else
#define SECTION_TYPE            SHT_MIPS_DWARF
#endif

