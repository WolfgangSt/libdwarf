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

	pro_util.h 
	$Revision: 1.15 $    $Date: 1999/06/22 16:33:42 $    
	$Source: /isms/cmplrs.src/osprey1.0/libdwarf/RCS/pro_util.h,v $

	Utility routines 

*/


#define IS_64BIT(dbg) 	((dbg)->de_flags & DW_DLC_SIZE_64 ? 1 : 0)
#define ISA_IA64(dbg) 	((dbg)->de_flags & DW_DLC_ISA_IA64 ? 1 : 0)

/* definition of sizes of types, given target machine */
#define sizeof_sbyte(dbg) 	sizeof(Dwarf_Sbyte)
#define sizeof_ubyte(dbg)	sizeof(Dwarf_Ubyte)
#define sizeof_uhalf(dbg)	sizeof(Dwarf_Half)
/* certain sizes not defined here, but set in dbg record.
   See pro_init.c
*/

/* Computes amount of padding necessary to align n to a k-boundary. */
/* Important: Assumes n, k both GREATER than zero. */
#define PADDING(n, k) ( (k)-1 - ((n)-1)%(k) )

/* The following defines are only important for users of the
** producer part of libdwarf, and such should have these
** defined correctly (as necessary) 
** by the #include <elf.h> done in pro_incl.h
** before the #include "pro_util.h".
** For others producer macros do not matter so 0 is a usable value, and
** zero values let compilation succeed on more non-MIPS architectures.
** A better approach would be welcome.
*/
/* R_MIPS* are #define so #ifndef works */
/* R_IA_64* are not necessarily #define (might be enum) so #ifndef
   is useless, we use the configure script generating 
   HAVE_R_IA_64_DIR32LSB.
*/
#ifndef R_MIPS_64
#define R_MIPS_64 0
#endif
#ifndef R_MIPS_32
#define R_MIPS_32 0
#endif
#ifndef R_MIPS_SCN_DISP
#define R_MIPS_SCN_DISP 0
#endif

#ifndef HAVE_R_IA_64_DIR32LSB
#define R_IA_64_DIR32LSB 0
#define R_IA_64_DIR64LSB 0
#define R_IA_64_SEGREL64LSB 0
#define R_IA_64_SEGREL32LSB 0
#endif

#ifdef HAVE_SYS_IA64_ELF_H
#define Get_REL64_isa(dbg)         (ISA_IA64(dbg) ? \
				R_IA_64_DIR64LSB : R_MIPS_64)
#define Get_REL32_isa(dbg)         (ISA_IA64(dbg) ? \
				R_IA_64_DIR32LSB : R_MIPS_32)


/* ia64 uses 32bit dwarf offsets for sections */
#define Get_REL_SEGREL_isa(dbg)    (ISA_IA64(dbg) ? \
				R_IA_64_SEGREL32LSB : R_MIPS_SCN_DISP)
#else

#if !defined(linux) && !defined(__BEOS__)
#define Get_REL64_isa(dbg)         (R_MIPS_64)
#define Get_REL32_isa(dbg)         (R_MIPS_32)
#define Get_REL_SEGREL_isa(dbg)    (R_MIPS_SCN_DISP)
#else
#define Get_REL64_isa(dbg)	(R_IA_64_DIR64LSB)
#define Get_REL32_isa(dbg)	(R_IA_64_DIR32LSB)
#define Get_REL_SEGREL_isa(dbg)	(R_IA_64_SEGREL64LSB)
#endif

#endif
