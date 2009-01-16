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

	pro_incl.h 
	$Revision: 1.8 $    $Date: 1999/06/22 16:33:38 $    

	main include file. Its included in all producer related 
	source files.

*/

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

/* The target address is given: the place in the source integer
   is to be determined.
*/
#ifdef WORDS_BIGENDIAN
#define WRITE_UNALIGNED(dbg,dest,source, srclength,len_out) \
    { \
      dbg->de_copy_word(dest, \
                        ((char *)source) +srclength-len_out,  \
			len_out) ; \
    }


#else /* LITTLE ENDIAN */

#define WRITE_UNALIGNED(dbg,dest,source, srclength,len_out) \
    { \
      dbg->de_copy_word( (dest) , \
                        ((char *)source) ,  \
			len_out) ; \
    }
#endif



#include "libdwarf.h"

#include "dwarf.h"
#include "pro_opaque.h"
#include "pro_error.h"
#include "pro_util.h"
#include "pro_encode_nm.h"
#include "pro_alloc.h"
