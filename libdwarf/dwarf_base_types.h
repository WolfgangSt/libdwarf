
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

    dwarf_opaque.h

    This file defines all the opaque structures for which pointers
    are provided in libdwarf.h.  In addition, it defines private
    structures that are used internally by libdwarf.

    $Revision: 1.4 $ $Date: 1999/06/22 16:33:24 $
*/
#include "libdwarfdefs.h"

#define true                    1
#define false                   0

/* to identify a cie */
#define DW_CIE_ID 		~(0x0)
#define DW_CIE_VERSION		1
#define ABBREV_HASH_TABLE_SIZE	10


/* 
    These are allocation type codes for structs that
    are internal to the Libdwarf Consumer library.
*/
#define DW_DLA_ABBREV_LIST	DW_DLA_ADDR + 1
#define DW_DLA_CHAIN		DW_DLA_ADDR + 2
#define DW_DLA_CU_CONTEXT	DW_DLA_ADDR + 3
#define DW_DLA_FRAME		DW_DLA_ADDR + 4
#define DW_DLA_GLOBAL_CONTEXT	DW_DLA_ADDR + 5
#define DW_DLA_FILE_ENTRY	DW_DLA_ADDR + 6
#define DW_DLA_LINE_CONTEXT	DW_DLA_ADDR + 7
#define DW_DLA_LOC_CHAIN	DW_DLA_ADDR + 8
#define DW_DLA_HASH_TABLE	DW_DLA_ADDR + 9
#define DW_DLA_FUNC_CONTEXT	DW_DLA_ADDR + 10
#define DW_DLA_TYPENAME_CONTEXT	DW_DLA_ADDR + 11
#define DW_DLA_VAR_CONTEXT	DW_DLA_ADDR + 12
#define DW_DLA_WEAK_CONTEXT	DW_DLA_ADDR + 13

/* Maximum number of allocation types for allocation routines. */
#define MAX_DW_DLA		DW_DLA_WEAK_CONTEXT

/*Dwarf_Word  is unsigned word usable for index, count in memory */
/*Dwarf_Sword is   signed word usable for index, count in memory */
/* The are 32 or 64 bits depending if 64 bit longs or not, which
** fits the  ILP32 and LP64 models
** These work equally well with ILP64.
*/

typedef unsigned long               Dwarf_Word;
typedef signed long		    Dwarf_Sword;

typedef signed char                 Dwarf_Sbyte;
typedef unsigned char               Dwarf_Ubyte;
typedef signed short		    Dwarf_Shalf;
typedef Dwarf_Small 		    *Dwarf_Byte_Ptr;

/* these 2 are fixed sizes which must not vary with the
** ILP32/LP64 model. Between these two, stay at 32 bit.
*/
typedef __uint32_t                  Dwarf_ufixed;
typedef __int32_t                   Dwarf_sfixed;

/*
        In various places the code mistakenly associates
        forms 8 bytes long with Dwarf_Signed or Dwarf_Unsigned
	This is not a very portable assumption.
        The following should be used instead for 64 bit integers.
*/
typedef __uint32_t                  Dwarf_ufixed64;
typedef __int32_t                   Dwarf_sfixed64;


typedef struct Dwarf_Abbrev_List_s	*Dwarf_Abbrev_List;
typedef struct Dwarf_File_Entry_s	*Dwarf_File_Entry;
typedef struct Dwarf_CU_Context_s	*Dwarf_CU_Context;
typedef struct Dwarf_Hash_Table_s	*Dwarf_Hash_Table;


typedef struct Dwarf_Alloc_Hdr_s	*Dwarf_Alloc_Hdr;
