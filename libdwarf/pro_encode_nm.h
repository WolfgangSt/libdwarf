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


	pro_encode_nm.h
	$Revision: 1.2 $
	$Date: 1999/03/05 22:00:19 $
	$Source: /isms/cmplrs.src/osprey1.0/libdwarf/RCS/pro_encode_nm.h,v $

*/

/*  Bytes needed to encode a number.
    Not a tight bound, just a reasonable bound.
*/ 
#define ENCODE_SPACE_NEEDED   (2*sizeof(Dwarf_Unsigned))


int _dwarf_pro_encode_leb128_nm(Dwarf_Unsigned val, int *nbytes,
       char *space, int splen);

int _dwarf_pro_encode_signed_leb128_nm(Dwarf_Signed value, int *nbytes,
    char *space, int splen);
