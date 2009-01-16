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

	pro_frame.h
	$Revision: 1.13 $    $Date: 1999/05/12 22:44:50 $    
	$Source : $

	pro_frame.h contains definitions used to create and store
	debug_frame information.
*/

/*
    Largest register value that can be coded into
    the opcode since there are only 6 bits in the
    register field.
*/
#define MAX_6_BIT_VALUE		0x3f

/*
	This struct holds debug_frame instructions
*/
typedef struct Dwarf_P_Frame_Pgm_s 	*Dwarf_P_Frame_Pgm;

struct Dwarf_P_Frame_Pgm_s {
    Dwarf_Ubyte		dfp_opcode;	/* opcode - includes reg # */
    char		*dfp_args;	/* operands */
    int			dfp_nbytes;	/* number of bytes in args */
#if 0
    Dwarf_Unsigned      dfp_sym_index;  /* 0 unless reloc needed */
#endif
    Dwarf_P_Frame_Pgm	dfp_next;
};


/*
	This struct has cie related information. Used to gather data 
	from user program, and later to transform to disk form
*/
struct Dwarf_P_Cie_s {
	Dwarf_Ubyte		cie_version;
	char 			*cie_aug;	/* augmentation */
	Dwarf_Ubyte		cie_code_align;	/* alignment of code */
	Dwarf_Sbyte		cie_data_align;	
	Dwarf_Ubyte		cie_ret_reg;	/* return register # */
	char			*cie_inst;	/* initial instruction */
	long			cie_inst_bytes;
						/* no of init_inst */
	Dwarf_P_Cie		cie_next;
};


/* producer fields */
struct Dwarf_P_Fde_s {
	Dwarf_Unsigned		fde_unused1;	

	    /* function/subr die for this fde */
	Dwarf_P_Die		fde_die;	

	    /* index to asso. cie */
	Dwarf_Word 		fde_cie;	

	    /* Address of first location of the code
	       this frame applies to 
	       If fde_end_symbol non-zero, this represents
	       the offset from the symbol indicated 
		by fde_r_symidx
	    */
	Dwarf_Addr		fde_initloc;	

	    /* Relocation symbol  for address of the code
		this frame applies to. */
	Dwarf_Unsigned		fde_r_symidx;	

	    /* Bytes of instr for this fde, if known 
	    */
	Dwarf_Unsigned		fde_addr_range;	

	    /* linked list of instructions we will put in
	       fde.
	    */
	Dwarf_P_Frame_Pgm	fde_inst;	

	    /* number of instructions in fde */ 
	long			fde_n_inst;	

	    /* number of bytes of inst in fde */
	long			fde_n_bytes;	

	    /* offset into exception table for this function. */
	Dwarf_Signed		fde_offset_into_exception_tables;

	    /* The symbol for the exception table elf section. */
	Dwarf_Unsigned		fde_exception_table_symbol;

	    /* pointer to last inst */
	Dwarf_P_Frame_Pgm	fde_last_inst;	

	Dwarf_P_Fde		fde_next;

	/* The symbol and offset of the  end symbol.
	   When fde_end_symbol is non-zero we
	   must represent the 
	*/
        Dwarf_Addr              fde_end_symbol_offset;
        Dwarf_Unsigned          fde_end_symbol;

	int                     fde_uwordb_size;
};
