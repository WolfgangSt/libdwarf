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

    dwarf_util.c
    $Revision: 1.4 $   $Date: 1999/07/21 21:29:42 $

    Dwarf utility functions.
*/

#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>


/*
    decode ULEB
*/
Dwarf_Unsigned
_dwarf_decode_u_leb128 (
    Dwarf_Small     	*leb128,
    Dwarf_Word      	*leb128_length
)
{
    Dwarf_Small     	byte;
    Dwarf_Word		word_number;
    Dwarf_Unsigned  	number;
    Dwarf_Sword  	shift;
    Dwarf_Sword		byte_length;

    if ((*leb128 & 0x80) == 0) {
	if (leb128_length != NULL) *leb128_length = 1;
	return(*leb128);
    }
    else if ((*(leb128 + 1) & 0x80) == 0) {
	if (leb128_length != NULL) *leb128_length = 2;

	word_number = *leb128 & 0x7f;
	word_number |= (*(leb128 + 1) & 0x7f) << 7;
	return(word_number);
    }
    else if ((*(leb128 + 2) & 0x80) == 0) {
	if (leb128_length != NULL) *leb128_length = 3;

	word_number = *leb128 & 0x7f;
	word_number |= (*(leb128 + 1) & 0x7f) << 7;
	word_number |= (*(leb128 + 2) & 0x7f) << 14;
	return(word_number);
    }
    else if ((*(leb128 + 3) & 0x80) == 0) {
	if (leb128_length != NULL) *leb128_length = 4;

	word_number = *leb128 & 0x7f;
	word_number |= (*(leb128 + 1) & 0x7f) << 7;
	word_number |= (*(leb128 + 2) & 0x7f) << 14;
	word_number |= (*(leb128 + 3) & 0x7f) << 21;
	return(word_number);
    }

    number = 0;
    shift = 0;
    byte_length = 1;
    byte = *(leb128);
    for (;;) {
	number |= (byte & 0x7f) << shift;
	shift += 7;

	if ((byte & 0x80) == 0) {
	    if (leb128_length != NULL) *leb128_length = byte_length;
	    return(number);
	}

	byte_length++;
	byte = *(++leb128);
    }
}


/*
    decode SLEB
*/
Dwarf_Signed
_dwarf_decode_s_leb128 (
    Dwarf_Small     	*leb128,
    Dwarf_Word          *leb128_length
)
{
    Dwarf_Small     	byte = *leb128;
    Dwarf_Sword		word_number = 0;
    Dwarf_Signed    	number;
    Dwarf_Bool	    	sign = 0;
    Dwarf_Bool		ndone = true;
    Dwarf_Sword  	shift = 0;
    Dwarf_Sword		byte_length = 0;

    while (byte_length++ < 4) {
	sign = byte & 0x40;
	word_number |= (byte & 0x7f) << shift;
	shift += 7;

	if ((byte & 0x80) == 0) {
	    ndone = false;
	    break;
	}
	byte = *(++leb128);
    }

    number = word_number;
    while (ndone) {
	sign = byte & 0x40;
        number |= (byte & 0x7f) << shift;
        shift += 7;

	if ((byte & 0x80) == 0) {
	    break;
	}

	    /* 
		Increment after byte has been placed in
	        number on account of the increment already
	        done when the first loop terminates.  That
	        is the fourth byte is picked up and byte_length
	        updated in the first loop.  So increment not
	        needed in this loop if break is taken.
	    */
	byte_length++;
        byte = *(++leb128);
    }

    if ((shift < sizeof(Dwarf_Signed)*8) && sign) 
	number |= - (1 << shift);

    if (leb128_length != NULL) *leb128_length = byte_length;
    return(number);
}


