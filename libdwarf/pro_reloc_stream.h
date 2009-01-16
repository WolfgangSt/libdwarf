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

        pro_reloc_stream.h
        $Revision: 1.1 $ $Date: 1999/04/20 16:43:51 $
        $Source: /isms/cmplrs.src/osprey1.0/libdwarf/RCS/pro_reloc_stream.h,v $

        This files contains routines for handling relocation
        information when we are creating stream relocs.
        That is, Elf-independent relocations are being 
	created here.
*/


int _dwarf_pro_reloc_name_stream64(
        Dwarf_P_Debug dbg,
	int     base_sec_index,
        Dwarf_Unsigned offset, /* r_offset of reloc */
        Dwarf_Unsigned symidx,
	enum Dwarf_Rel_Type,
	int reltarget_length);
int _dwarf_pro_reloc_name_stream32(
        Dwarf_P_Debug dbg,
	int     base_sec_index,
        Dwarf_Unsigned offset, /* r_offset of reloc */
        Dwarf_Unsigned symidx,
	enum Dwarf_Rel_Type,
	int reltarget_length);
int _dwarf_pro_reloc_length_stream(
        Dwarf_P_Debug dbg,
	int     base_sec_index,
        Dwarf_Unsigned offset, /* r_offset of reloc */
        Dwarf_Unsigned start_symidx,
	Dwarf_Unsigned end_symidx,
	enum Dwarf_Rel_Type,
	int reltarget_length);

int _dwarf_stream_relocs_to_disk(
        Dwarf_P_Debug dbg,
        Dwarf_Signed *new_sec_count);
