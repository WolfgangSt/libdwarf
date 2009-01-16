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

    dwarf_line.h

    Defines the opaque structures used only by dwarf_line.c
    and pro_line.c

    $Revision: 1.14 $ $Date: 1999/03/05 22:00:08 $
*/

#define DW_EXTENDED_OPCODE	0

/*
    This is used as the starting value for an algorithm
    to get the minimum difference between 2 values.
    UINT_MAX is used as our approximation to infinity.
*/
#define MAX_LINE_DIFF       UINT_MAX


/*
    This structure is used to build a list of all the
    files that are used in the current compilation unit.
    All of the fields execpt fi_next have meanings that
    are obvious from section 6.2.4 of the Libdwarf Doc.
*/
struct Dwarf_File_Entry_s {
        /* Points to string naming the file. */
    Dwarf_Small        		*fi_file_name;

        /* 
            Index into the list of directories of 
            the directory in which this file exits.
        */
    Dwarf_Sword          	fi_dir_index;

        /* Time of last modification of the file. */
    Dwarf_Unsigned          	fi_time_last_mod;

        /* Length in bytes of the file. */
    Dwarf_Unsigned          	fi_file_length;

        /* Pointer for chaining file entries. */
    Dwarf_File_Entry        	fi_next;
};


typedef struct Dwarf_Line_Context_s 	*Dwarf_Line_Context;

/* 
    This structure provides the context in which the fields of 
    a Dwarf_Line structure are interpreted.  They come from the 
    statement program prologue.  **Updated by dwarf_srclines in 
    dwarf_line.c.
*/
struct Dwarf_Line_Context_s {
	/*
	    Points to a chain of entries providing info
	    about source files for the current set of
	    Dwarf_Line structures.
	*/
    Dwarf_File_Entry    lc_file_entries;
	/*  
	    Count of number of source files for this set of
	    Dwarf_Line structures. 
	*/
    Dwarf_Sword      	lc_file_entry_count;
	/* 
	    Points to the portion of .debug_line section
	    that contains a list of strings naming the
	    included directories.
	*/
    Dwarf_Small         *lc_include_directories;

	/* Count of the number of included directories. */
    Dwarf_Sword      	lc_include_directories_count;

	/* Count of the number of lines for this cu. */
    Dwarf_Sword		lc_line_count;

	/* Points to name of compilation directory. */
    Dwarf_Small		*lc_compilation_directory;

    Dwarf_Debug		lc_dbg;
};


/*
    This structure defines a row of the line table.
    All of the fields except li_dbg have the exact 
    same meaning that is defined in Section 6.2.2 
    of the Libdwarf Document.
*/
struct Dwarf_Line_s {
   Dwarf_Addr              li_address;         /* pc value of machine instr */
   union addr_or_line_s {
     struct li_inner_s {
      Dwarf_Sword             li_file;          /* int identifying src file */
      Dwarf_Sword             li_line;          /* source file line number. */
      Dwarf_Half              li_column;        /* source file column number */
      Dwarf_Small             li_is_stmt;       /* indicate start of stmt */
      Dwarf_Small             li_basic_block;   /* indicate start basic block */
      Dwarf_Small             li_end_sequence;  /* first post sequence instr */
     }li_l_data;
     Dwarf_Off                li_offset;        /* for rqs */
   } li_addr_line;
   Dwarf_Line_Context      li_context;         /* assoc Dwarf_Line_Context_s */
};


int
_dwarf_line_address_offsets(Dwarf_Debug dbg,
                Dwarf_Die die,
                Dwarf_Addr **addrs,
                Dwarf_Off  **offs,
                Dwarf_Unsigned *returncount,
                Dwarf_Error *err);

