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



*/
/*
	If ag_end_symbol_index is zero, 
	ag_length must be known and non-zero.


	Deals with length being known costant or fr
	assembler output, not known.

*/

struct Dwarf_P_Arange_s {
    Dwarf_Addr		ag_begin_address; /* known address
				or for symbolic assem output,
				offset of symbol */
    Dwarf_Addr          ag_length; /* zero or address or
				offset */
    Dwarf_Unsigned	ag_symbol_index; 

    Dwarf_P_Arange	ag_next;

    Dwarf_Unsigned      ag_end_symbol_index; /*  zero
				or index/id of end symbol */
    Dwarf_Addr          ag_end_symbol_offset;/* known address
                                or for symbolic assem output,
                                offset of end symbol */

};
