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

	pro_util.c
	$Revision: 1.3 $    $Date: 1999/06/22 16:33:35 $    
	$Source: /isms/cmplrs.src/osprey1.0/libdwarf/RCS/pro_encode_nm.c,v $

	utility functions 
*/

#include "config.h"
#include "libdwarfdefs.h"
#include <string.h>
#include "pro_incl.h"

#define MORE_BYTES      0x80
#define DATA_MASK       0x7f
#define DIGIT_WIDTH     7
#define SIGN_BIT        0x40


/*-------------------------------------------------------------
	Encode val as a leb128. This encodes it as an unsigned 
	number.
---------------------------------------------------------------*/
/* return DW_DLV_ERROR or DW_DLV_OK.
** space to write leb number is provided by caller, with caller
** passing length.
** number of bytes used returned thru nbytes arg
*/
int 
_dwarf_pro_encode_leb128_nm(Dwarf_Unsigned val, int *nbytes,
       char *space, int splen)
{
	char *a;
	char *end = space + splen;

	a = space;
  	do
    	{
		unsigned char uc;
		if(a >= end) {
			return DW_DLV_ERROR;
		}
    		uc = val & DATA_MASK;
    		val >>= DIGIT_WIDTH;
    		if (val != 0)
      		uc |= MORE_BYTES;
		*a = uc;
		a++;
    	} while (val);
	*nbytes = a - space;
	return DW_DLV_OK;
}

/* return DW_DLV_ERROR or DW_DLV_OK.
** space to write leb number is provided by caller, with caller
** passing length.
** number of bytes used returned thru nbytes arg
** encodes a signed number.
*/
int
_dwarf_pro_encode_signed_leb128_nm(Dwarf_Signed value, int *nbytes,
    char *space, int splen)
{
  char	*str;
  Dwarf_Signed sign = - (value < 0);
  int more = 1;
  char *end = space + splen;

  str = space;

  do {
      unsigned char byte = value & DATA_MASK;
      value >>= DIGIT_WIDTH;

      if(str >= end) {
	return DW_DLV_ERROR;
      }
      /*
       * Remaining chunks would just contain the sign bit, and this chunk
       * has already captured at least one sign bit.
       */
      if (value == sign && ((byte & SIGN_BIT) == (sign & SIGN_BIT)))
        more = 0;
      else
        byte |= MORE_BYTES;
      *str = byte;
      str++;
    } while (more);
  *nbytes = str - space;
  return DW_DLV_OK;
}
