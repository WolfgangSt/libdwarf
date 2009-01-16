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

	pro_error.h

	Macros for handling errors.   Made into macros since
	these are often used and may need to change.

	User must supply a trailing ;  as in normal statements
	Made as comma expresions so they can be added to an
	if or else or between them without causing any surprises.

	$Revision: 1.3 $     $Date: 1999/03/05 22:00:20 $
*/


/* Handle error passing in the name of the Dwarf_P_Debug
   User must supply {} around the macro.
   Putting the {} here leads to macro uses that don't look like C.
   The error argument to dwarf_error is hard coded here as 'error'
*/
#define DWARF_P_DBG_ERROR(dbg,errval,retval) \
     _dwarf_p_error(dbg,error,errval); return(retval);

struct Dwarf_Error_s {
    Dwarf_Sword         er_errval;
};

void _dwarf_p_error(Dwarf_P_Debug dbg, Dwarf_Error *error, Dwarf_Word  errval);
