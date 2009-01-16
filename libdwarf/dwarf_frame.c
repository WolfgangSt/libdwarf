/*

  Copyright (C) 2000,2002,2004,2005,2006 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU Lesser General Public 
  License along with this program; if not, write the Free Software 
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, 
  USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/



#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include <stdlib.h>
#include "dwarf_frame.h"
#include "dwarf_arange.h"	/* using Arange as a way to build a
				   list */


/* 
    This function is the heart of the debug_frame stuff.  Don't even
    think of reading this without reading both the Libdwarf and 
    consumer API carefully first.  This function basically executes     
    frame instructions contained in a Cie or an Fde, but does in a      
    number of different ways depending on the information sought.       
    Start_instr_ptr points to the first byte of the frame instruction    
    stream, and final_instr_ptr to the to the first byte after the       
    last.                                                                
                                                                        
    The offsets returned in the frame instructions are factored.  That   
    is they need to be multiplied by either the code_alignment_factor    
    or the data_alignment_factor, as appropriate to obtain the actual      
    offset.  This makes it possible to expand an instruction stream      
    without the corresponding Cie.  However, when an Fde frame instr     
    sequence is being expanded there must be a valid Cie with a pointer  
    to an initial table row.                                             
                                                                         

    If successful, returns DW_DLV_OK
		And sets returned_count thru the pointer
		 if make_instr is true.
		If make_instr is false returned_count 
		 should NOT be used by the caller (returned_count
		 is set to 0 thru the pointer by this routine...)
    If unsuccessful, returns DW_DLV_ERROR
		and sets returned_error to the error code

    It does not do a whole lot of input validation being a private 
    function.  Please make sure inputs are valid.
                                                                        
    (1) If make_instr is true, it makes a list of pointers to              
    Dwarf_Frame_Op structures containing the frame instructions          
    executed.  A pointer to this list is returned in ret_frame_instr.    
    Make_instr is true only when a list of frame instructions is to be   
    returned.  In this case since we are not interested in the contents  
    of the table, the input Cie can be NULL.  This is the only case
    where the inpute Cie can be NULL.

    (2) If search_pc is true, frame instructions are executed till       
    either a location is reached that is greater than the search_pc_val
    provided, or all instructions are executed.  At this point the       
    last row of the table generated is returned in a structure.          
    A pointer to this structure is supplied in table.                    
                                                                    
    (3) This function is also used to create the initial table row       
    defined by a Cie.  In this case, the Dwarf_Cie pointer cie, is       
    NULL.  For an FDE, however, cie points to the associated Cie.        

    make_instr - make list of frame instr? 0/1
    ret_frame_instr -  Ptr to list of ptrs to frame instrs
    search_pc  - Search for a pc value?  0/1
     search_pc_val -  Search for this pc value
    initial_loc - Initial code location value.
    start_instr_ptr -   Ptr to start of frame instrs.
    final_instr_ptr -   Ptr just past frame instrs. 
    table       -     Ptr to struct with last row. 
    cie     -   Ptr to Cie used by the Fde.

*/
int
_dwarf_exec_frame_instr(Dwarf_Bool make_instr,
			Dwarf_Frame_Op ** ret_frame_instr,
			Dwarf_Bool search_pc,
			Dwarf_Addr search_pc_val,
			Dwarf_Addr initial_loc,
			Dwarf_Small * start_instr_ptr,
			Dwarf_Small * final_instr_ptr,
			Dwarf_Frame table,
			Dwarf_Cie cie,
			Dwarf_Debug dbg,
			Dwarf_Sword * returned_count,
			int *returned_error)
{
    /* Sweeps the frame instructions. */
    Dwarf_Small *instr_ptr;

    /* Obvious from the documents. */
    Dwarf_Small instr, opcode;
    Dwarf_Small reg_no, reg_noA, reg_noB;
    Dwarf_Unsigned factored_N_value;
    Dwarf_Addr current_loc = initial_loc;	/* code location/
						   pc-value
						   corresponding to the 
						   frame instructions.
						   Starts at zero when
						   the caller has no
						   value to pass in. */

    Dwarf_Unsigned adv_loc;	/* must be min de_pointer_size bytes
				   and must be at least sizeof
				   Dwarf_ufixed */

    struct Dwarf_Reg_Rule_s reg[DW_FRAME_LAST_REG_NUM];


    /* This is used to end executing frame instructions.  */
    /* Becomes true when search_pc is true and current_loc */
    /* is greater than search_pc_val.  */
    Dwarf_Bool search_over = false;

    /* Used by the DW_FRAME_advance_loc instr */
    /* to hold the increment in pc value.  */
    Dwarf_Addr adv_pc;

    /* Contains the length in bytes of */
    /* an leb128 encoded number.  */
    Dwarf_Word leb128_length;

    /* Counts the number of frame instructions executed.  */
    Dwarf_Word instr_count = 0;

    /* 
       These contain the current fields of the current frame
       instruction. */
    Dwarf_Small fp_base_op = 0;
    Dwarf_Small fp_extended_op;
    Dwarf_Half fp_register;
    Dwarf_Unsigned fp_offset;
    Dwarf_Off fp_instr_offset;

    /* 
       Stack_table points to the row (Dwarf_Frame ie) being pushed or
       popped by a remember or restore instruction. Top_stack points to 
       the top of the stack of rows. */
    Dwarf_Frame stack_table;
    Dwarf_Frame top_stack = NULL;

    /* 
       These are used only when make_instr is true. Curr_instr is a
       pointer to the current frame instruction executed.
       Curr_instr_ptr, head_instr_list, and curr_instr_list are used to 
       form a chain of Dwarf_Frame_Op structs. Dealloc_instr_ptr is
       used to deallocate the structs used to form the chain.
       Head_instr_block points to a contiguous list of pointers to the
       Dwarf_Frame_Op structs executed. */
    Dwarf_Frame_Op *curr_instr;
    Dwarf_Chain curr_instr_item, dealloc_instr_item;
    Dwarf_Chain head_instr_chain = NULL;
    Dwarf_Chain tail_instr_chain = NULL;
    Dwarf_Frame_Op *head_instr_block;

    /* 
       These are the alignment_factors taken from the Cie provided.
       When no input Cie is provided they are set to 1, because only
       factored offsets are required. */
    Dwarf_Sword code_alignment_factor = 1;
    Dwarf_Sword data_alignment_factor = 1;

    /* 
       This flag indicates when an actual alignment factor is needed.
       So if a frame instruction that computes an offset using an
       alignment factor is encountered when this flag is set, an error
       is returned because the Cie did not have a valid augmentation. */
    Dwarf_Bool need_augmentation = false;

    Dwarf_Word i;

    /* Initialize first row from associated Cie. Using temp regs
       explicity */
    struct Dwarf_Reg_Rule_s *t1reg;
    struct Dwarf_Reg_Rule_s *t1end;
    struct Dwarf_Reg_Rule_s *t2reg;


    t1reg = reg;
    t1end = t1reg + DW_FRAME_LAST_REG_NUM;
    if (cie != NULL && cie->ci_initial_table != NULL) {
	t2reg = cie->ci_initial_table->fr_reg;
	for (; t1reg < t1end; t1reg++, t2reg++) {
	    *t1reg = *t2reg;
	}
    } else {			/* initialize with same_value */
	for (; t1reg < t1end; t1reg++) {
	    t1reg->ru_is_off = 0;
	    t1reg->ru_register = DW_FRAME_SAME_VAL;
	    t1reg->ru_offset = 0;
	}
    }

    /* 
       The idea here is that the code_alignment_factor and
       data_alignment_factor which are needed for certain instructions
       are valid only when the Cie has a proper augmentation string. So 
       if the augmentation is not right, only Frame instruction can be
       read. */
    if (cie != NULL && cie->ci_augmentation != NULL) {
	code_alignment_factor = cie->ci_code_alignment_factor;
	data_alignment_factor = cie->ci_data_alignment_factor;
    } else
	need_augmentation = !make_instr;

    instr_ptr = start_instr_ptr;
    while ((instr_ptr < final_instr_ptr) && (!search_over)) {


	fp_instr_offset = instr_ptr - start_instr_ptr;
	instr = *(Dwarf_Small *) instr_ptr;
	instr_ptr += sizeof(Dwarf_Small);

	fp_base_op = (instr & 0xc0) >> 6;
	if ((instr & 0xc0) == 0x00) {
	    opcode = instr;	/* is really extended op */
	    fp_extended_op = (instr & (~(0xc0))) & 0xff;
	} else {
	    opcode = instr & 0xc0;	/* is base op */
	    fp_extended_op = 0;
	}

	fp_register = 0;
	fp_offset = 0;
	switch (opcode) {

	case DW_CFA_advance_loc:{
		/* base op */
		fp_offset = adv_pc = instr & DW_FRAME_INSTR_OFFSET_MASK;

		if (need_augmentation) {

		    *returned_error = (DW_DLE_DF_NO_CIE_AUGMENTATION);
		    return DW_DLV_ERROR;
		}
		adv_pc = adv_pc * code_alignment_factor;

		search_over = search_pc &&
		    (current_loc + adv_pc > search_pc_val);
		/* If gone past pc needed, retain old pc.  */
		if (!search_over)
		    current_loc = current_loc + adv_pc;
		break;
	    }

	case DW_CFA_offset:{	/* base op */
		reg_no = (instr & DW_FRAME_INSTR_OFFSET_MASK);
		if (reg_no > DW_FRAME_LAST_REG_NUM) {
		    *returned_error = DW_DLE_DF_REG_NUM_TOO_HIGH;
		    return DW_DLV_ERROR;
		}

		factored_N_value =
		    _dwarf_decode_u_leb128(instr_ptr, &leb128_length);
		instr_ptr = instr_ptr + leb128_length;

		fp_register = reg_no;
		fp_offset = factored_N_value;

		if (need_augmentation) {
		    *returned_error = (DW_DLE_DF_NO_CIE_AUGMENTATION);
		    return DW_DLV_ERROR;
		}

		reg[reg_no].ru_is_off = 1;
		reg[reg_no].ru_register = DW_FRAME_CFA_COL;
		reg[reg_no].ru_offset = factored_N_value *
		    data_alignment_factor;

		break;
	    }

	case DW_CFA_restore:{	/* base op */
		reg_no = (instr & DW_FRAME_INSTR_OFFSET_MASK);
		if (reg_no > DW_FRAME_LAST_REG_NUM) {
		    *returned_error = (DW_DLE_DF_REG_NUM_TOO_HIGH);
		    return DW_DLV_ERROR;
		}

		fp_register = reg_no;

		if (cie != NULL && cie->ci_initial_table != NULL)
		    reg[reg_no] = cie->ci_initial_table->fr_reg[reg_no];
		else if (!make_instr) {
		    *returned_error = (DW_DLE_DF_MAKE_INSTR_NO_INIT);
		    return DW_DLV_ERROR;
		}

		break;
	    }
	case DW_CFA_set_loc:{
		Dwarf_Addr new_loc = 0;

		READ_UNALIGNED(dbg, new_loc, Dwarf_Addr,
			       instr_ptr, dbg->de_pointer_size);
		instr_ptr += dbg->de_pointer_size;
		if (new_loc != 0 && current_loc != 0) {
		    /* Pre-relocation or before current_loc is set the
		       test comparing new_loc and current_loc makes no
		       sense. Testing for non-zero (above) is a way
		       (fallible) to check that current_loc, new_loc are 
		       already relocated.  */
		    if (new_loc <= current_loc) {
			/* Within a frame, address must increase.
			   Seemingly it has not. Seems to be an error. */

			*returned_error =
			    (DW_DLE_DF_NEW_LOC_LESS_OLD_LOC);
			return DW_DLV_ERROR;
		    }
		}

		search_over = search_pc && (new_loc > search_pc_val);

		/* If gone past pc needed, retain old pc.  */
		if (!search_over)
		    current_loc = new_loc;
		fp_offset = new_loc;
		break;
	    }

	case DW_CFA_advance_loc1:{
		fp_offset = adv_loc = *(Dwarf_Small *) instr_ptr;
		instr_ptr += sizeof(Dwarf_Small);

		if (need_augmentation) {
		    *returned_error = (DW_DLE_DF_NO_CIE_AUGMENTATION);
		    return DW_DLV_ERROR;
		}
		adv_loc *= code_alignment_factor;

		search_over = search_pc &&
		    (current_loc + adv_loc > search_pc_val);

		/* If gone past pc needed, retain old pc.  */
		if (!search_over)
		    current_loc = current_loc + adv_loc;
		break;
	    }

	case DW_CFA_advance_loc2:{
		READ_UNALIGNED(dbg, adv_loc, Dwarf_Unsigned,
			       instr_ptr, sizeof(Dwarf_Half));
		instr_ptr += sizeof(Dwarf_Half);
		fp_offset = adv_loc;

		if (need_augmentation) {
		    *returned_error = (DW_DLE_DF_NO_CIE_AUGMENTATION);
		    return DW_DLV_ERROR;
		}
		adv_loc *= code_alignment_factor;

		search_over = search_pc &&
		    (current_loc + adv_loc > search_pc_val);

		/* If gone past pc needed, retain old pc.  */
		if (!search_over)
		    current_loc = current_loc + adv_loc;
		break;
	    }

	case DW_CFA_advance_loc4:{
		READ_UNALIGNED(dbg, adv_loc, Dwarf_Unsigned,
			       instr_ptr, sizeof(Dwarf_ufixed));
		instr_ptr += sizeof(Dwarf_ufixed);
		fp_offset = adv_loc;

		if (need_augmentation) {
		    *returned_error = (DW_DLE_DF_NO_CIE_AUGMENTATION);
		    return DW_DLV_ERROR;
		}
		adv_loc *= code_alignment_factor;

		search_over = search_pc &&
		    (current_loc + adv_loc > search_pc_val);

		/* If gone past pc needed, retain old pc.  */
		if (!search_over)
		    current_loc = current_loc + adv_loc;
		break;
	    }

	case DW_CFA_offset_extended:{
		Dwarf_Unsigned lreg;

		DECODE_LEB128_UWORD(instr_ptr, lreg)
		    reg_no = (Dwarf_Small) lreg;
		if (reg_no > DW_FRAME_LAST_REG_NUM) {
		    *returned_error = (DW_DLE_DF_REG_NUM_TOO_HIGH);
		    return DW_DLV_ERROR;
		}
		factored_N_value =
		    _dwarf_decode_u_leb128(instr_ptr, &leb128_length);
		instr_ptr += leb128_length;

		if (need_augmentation) {
		    *returned_error = (DW_DLE_DF_NO_CIE_AUGMENTATION);
		    return DW_DLV_ERROR;
		}
		reg[reg_no].ru_is_off = 1;
		reg[reg_no].ru_register = DW_FRAME_CFA_COL;
		reg[reg_no].ru_offset = factored_N_value *
		    data_alignment_factor;

		fp_register = reg_no;
		fp_offset = factored_N_value;
		break;
	    }

	case DW_CFA_restore_extended:{
		Dwarf_Unsigned lreg;

		DECODE_LEB128_UWORD(instr_ptr, lreg)
		    reg_no = (Dwarf_Small) lreg;

		if (reg_no > DW_FRAME_LAST_REG_NUM) {
		    *returned_error = (DW_DLE_DF_REG_NUM_TOO_HIGH);
		    return DW_DLV_ERROR;
		}

		if (cie != NULL && cie->ci_initial_table != NULL) {
		    reg[reg_no] = cie->ci_initial_table->fr_reg[reg_no];
		} else {
		    if (!make_instr) {
			*returned_error =
			    (DW_DLE_DF_MAKE_INSTR_NO_INIT);
			return DW_DLV_ERROR;
		    }
		}

		fp_register = reg_no;
		break;
	    }

	case DW_CFA_undefined:{
		Dwarf_Unsigned lreg;

		DECODE_LEB128_UWORD(instr_ptr, lreg)
		    reg_no = (Dwarf_Small) lreg;
		if (reg_no > DW_FRAME_LAST_REG_NUM) {
		    *returned_error = (DW_DLE_DF_REG_NUM_TOO_HIGH);
		    return DW_DLV_ERROR;
		}

		reg[reg_no].ru_is_off = 0;
		reg[reg_no].ru_register = DW_FRAME_UNDEFINED_VAL;
		reg[reg_no].ru_offset = 0;

		fp_register = reg_no;
		break;
	    }

	case DW_CFA_same_value:{
		Dwarf_Unsigned lreg;

		DECODE_LEB128_UWORD(instr_ptr, lreg)
		    reg_no = (Dwarf_Small) lreg;
		if (reg_no > DW_FRAME_LAST_REG_NUM) {
		    *returned_error = (DW_DLE_DF_REG_NUM_TOO_HIGH);
		    return DW_DLV_ERROR;
		}

		reg[reg_no].ru_is_off = 0;
		reg[reg_no].ru_register = DW_FRAME_SAME_VAL;
		reg[reg_no].ru_offset = 0;
		fp_register = reg_no;
		break;
	    }

	case DW_CFA_register:{
		Dwarf_Unsigned lreg;

		DECODE_LEB128_UWORD(instr_ptr, lreg)
		    reg_noA = (Dwarf_Small) lreg;

		if (reg_noA > DW_FRAME_LAST_REG_NUM) {
		    *returned_error = (DW_DLE_DF_REG_NUM_TOO_HIGH);
		    return DW_DLV_ERROR;
		}

		DECODE_LEB128_UWORD(instr_ptr, lreg)
		    reg_noB = (Dwarf_Small) lreg;

		if (reg_noB > DW_FRAME_LAST_REG_NUM) {
		    *returned_error = (DW_DLE_DF_REG_NUM_TOO_HIGH);
		    return DW_DLV_ERROR;
		}


		reg[reg_noA].ru_is_off = 0;
		reg[reg_noA].ru_register = reg_noB;

		reg[reg_noA].ru_offset = 0;

		fp_register = reg_noA;
		fp_offset = reg_noB;
		break;
	    }

	case DW_CFA_remember_state:{
		stack_table = (Dwarf_Frame)
		    _dwarf_get_alloc(dbg, DW_DLA_FRAME, 1);
		if (stack_table == NULL) {
		    *returned_error = (DW_DLE_DF_ALLOC_FAIL);
		    return DW_DLV_ERROR;
		}

		for (i = 0; i < DW_FRAME_LAST_REG_NUM; i++)
		    stack_table->fr_reg[i] = reg[i];

		if (top_stack != NULL)
		    stack_table->fr_next = top_stack;
		top_stack = stack_table;

		break;
	    }

	case DW_CFA_restore_state:{
		if (top_stack == NULL) {
		    *returned_error = (DW_DLE_DF_POP_EMPTY_STACK);
		    return DW_DLV_ERROR;
		}
		stack_table = top_stack;
		top_stack = stack_table->fr_next;

		for (i = 0; i < DW_FRAME_LAST_REG_NUM; i++)
		    reg[i] = stack_table->fr_reg[i];

		dwarf_dealloc(dbg, stack_table, DW_DLA_FRAME);
		break;
	    }

	case DW_CFA_def_cfa:{
		Dwarf_Unsigned lreg;

		DECODE_LEB128_UWORD(instr_ptr, lreg)
		    reg_no = (Dwarf_Small) lreg;

		if (reg_no > DW_FRAME_LAST_REG_NUM) {
		    *returned_error = (DW_DLE_DF_REG_NUM_TOO_HIGH);
		    return (DW_DLV_ERROR);
		}

		factored_N_value =
		    _dwarf_decode_u_leb128(instr_ptr, &leb128_length);
		instr_ptr += leb128_length;

		if (need_augmentation) {
		    *returned_error = (DW_DLE_DF_NO_CIE_AUGMENTATION);
		    return DW_DLV_ERROR;
		}
		reg[DW_FRAME_CFA_COL].ru_is_off = 1;
		reg[DW_FRAME_CFA_COL].ru_register = reg_no;
		reg[DW_FRAME_CFA_COL].ru_offset = factored_N_value;

		fp_register = reg_no;
		fp_offset = factored_N_value;
		break;
	    }

	case DW_CFA_def_cfa_register:{
		Dwarf_Unsigned lreg;

		DECODE_LEB128_UWORD(instr_ptr, lreg)
		    reg_no = (Dwarf_Small) lreg;

		if (reg_no > DW_FRAME_LAST_REG_NUM) {
		    *returned_error = (DW_DLE_DF_REG_NUM_TOO_HIGH);
		    return DW_DLV_ERROR;
		}

		reg[DW_FRAME_CFA_COL].ru_register = reg_no;
		/* Do NOT set ru_offset or ru_is_off here. See dwarf2/3 
		   spec.  */
		fp_register = reg_no;
		break;
	    }

	case DW_CFA_def_cfa_offset:{
		factored_N_value =
		    _dwarf_decode_u_leb128(instr_ptr, &leb128_length);
		instr_ptr += leb128_length;

		if (need_augmentation) {
		    *returned_error = (DW_DLE_DF_NO_CIE_AUGMENTATION);
		    return DW_DLV_ERROR;
		}
		/* Do set ru_is_off here, as here factored_N_value
		   counts.  */
		reg[DW_FRAME_CFA_COL].ru_is_off = 1;
		reg[DW_FRAME_CFA_COL].ru_offset = factored_N_value;

		fp_offset = factored_N_value;
		break;
	    }

	case DW_CFA_nop:{
		break;
	    }

#ifdef DW_CFA_GNU_window_save
	case DW_CFA_GNU_window_save:{
		/* no information: this just tells unwinder to restore
		   the window registers from the previous frame's
		   window save area */
		break;
	    }
#endif
#ifdef  DW_CFA_GNU_args_size
	    /* single uleb128 is the current arg area size in bytes. No 
	       register exists yet to save this in */
	case DW_CFA_GNU_args_size:{
		Dwarf_Unsigned lreg;

		DECODE_LEB128_UWORD(instr_ptr, lreg)
		    reg_no = (Dwarf_Small) lreg;

		break;
	    }
#endif
	}

	if (make_instr) {
	    instr_count++;

	    curr_instr = (Dwarf_Frame_Op *)
		_dwarf_get_alloc(dbg, DW_DLA_FRAME_OP, 1);
	    if (curr_instr == NULL) {
		*returned_error = (DW_DLE_DF_ALLOC_FAIL);
		return DW_DLV_ERROR;
	    }

	    curr_instr->fp_base_op = fp_base_op;
	    curr_instr->fp_extended_op = fp_extended_op;
	    curr_instr->fp_register = fp_register;
	    curr_instr->fp_offset = fp_offset;
	    curr_instr->fp_instr_offset = fp_instr_offset;

	    curr_instr_item = (Dwarf_Chain)
		_dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
	    if (curr_instr_item == NULL) {
		*returned_error = (DW_DLE_DF_ALLOC_FAIL);
		return DW_DLV_ERROR;
	    }

	    curr_instr_item->ch_item = curr_instr;
	    if (head_instr_chain == NULL)
		head_instr_chain = tail_instr_chain = curr_instr_item;
	    else {
		tail_instr_chain->ch_next = curr_instr_item;
		tail_instr_chain = curr_instr_item;
	    }
	}
    }

    /* 
       If frame instruction decoding was right we would stop exactly at 
       final_instr_ptr. */
    if (instr_ptr > final_instr_ptr) {
	*returned_error = (DW_DLE_DF_FRAME_DECODING_ERROR);
	return DW_DLV_ERROR;
    }

    /* Create the last row generated.  */
    if (table != NULL) {
	t1reg = reg;
	t1end = t1reg + DW_FRAME_LAST_REG_NUM;
	table->fr_loc = current_loc;
	t2reg = table->fr_reg;
	for (; t1reg < t1end; t1reg++, t2reg++) {
	    *t2reg = *t1reg;
	}
    }

    /* Dealloc anything remaining on stack. */
    for (; top_stack != NULL;) {
	stack_table = top_stack;
	top_stack = top_stack->fr_next;
	dwarf_dealloc(dbg, stack_table, DW_DLA_FRAME);
    }

    if (make_instr) {
	/* Allocate list of pointers to Dwarf_Frame_Op's.  */
	head_instr_block = (Dwarf_Frame_Op *)
	    _dwarf_get_alloc(dbg, DW_DLA_FRAME_BLOCK, instr_count);
	if (head_instr_block == NULL) {
	    *returned_error = DW_DLE_DF_ALLOC_FAIL;
	    return DW_DLV_ERROR;
	}

	/* 
	   Store pointers to Dwarf_Frame_Op's in this list and
	   deallocate the structs that chain the Dwarf_Frame_Op's. */
	curr_instr_item = head_instr_chain;
	for (i = 0; i < instr_count; i++) {
	    *(head_instr_block + i) =
		*(Dwarf_Frame_Op *) curr_instr_item->ch_item;
	    dealloc_instr_item = curr_instr_item;
	    curr_instr_item = curr_instr_item->ch_next;
	    dwarf_dealloc(dbg, dealloc_instr_item->ch_item,
			  DW_DLA_FRAME_OP);
	    dwarf_dealloc(dbg, dealloc_instr_item, DW_DLA_CHAIN);
	}
	*ret_frame_instr = head_instr_block;

	*returned_count = (Dwarf_Sword) instr_count;
    } else {
	*returned_count = 0;
    }
    return DW_DLV_OK;
}

/*  Depending on version, either read the return address register
    as a ubyte or as an leb number.
    The form of this value changed for DWARF3.
*/
Dwarf_Unsigned
_dwarf_get_return_address_reg(Dwarf_Small * frame_ptr,
			      int version, unsigned long *size)
{
    Dwarf_Unsigned uvalue = 0;
    Dwarf_Word leb128_length = 0;

    if (version == 1) {
	*size = 1;
	uvalue = *(unsigned char *) frame_ptr;
	return uvalue;
    }
    uvalue = _dwarf_decode_u_leb128(frame_ptr, &leb128_length);
    *size = leb128_length;
    return uvalue;
}


/* Trivial consumer function. 
*/
int
dwarf_get_cie_of_fde(Dwarf_Fde fde,
		     Dwarf_Cie * cie_returned, Dwarf_Error * error)
{
    if (fde == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_NULL);
	return (DW_DLV_ERROR);
    }

    *cie_returned = fde->fd_cie;
    return DW_DLV_OK;

}

/*
  For g++ .eh_frame fde and cie.
  the cie id is different as the
  definition of the cie_id in an fde
	is the distance back from the address of the
	value to the cie.
  Or 0 if this is a true cie.
  Non standard dwarf, designed this way to be
  convenient at run time for an allocated 
  (mapped into memory as part of the running image) section.
*/
int
dwarf_get_fde_list_eh(Dwarf_Debug dbg,
		      Dwarf_Cie ** cie_data,
		      Dwarf_Signed * cie_element_count,
		      Dwarf_Fde ** fde_data,
		      Dwarf_Signed * fde_element_count,
		      Dwarf_Error * error)
{
    int res;

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_frame_eh_gnu_index,
			    &dbg->de_debug_frame_eh_gnu, error);

    if (res != DW_DLV_OK) {
	return res;
    }

    res =
	_dwarf_get_fde_list_internal(dbg,
				     cie_data,
				     cie_element_count,
				     fde_data,
				     fde_element_count,
				     dbg->de_debug_frame_eh_gnu,
				     dbg->de_debug_frame_size_eh_gnu,
				     dbg->de_debug_frame_eh_gnu_index,
				     /* cie_id_value */ 0,
				     /* use_gnu_cie_calc= */ 1,
				     error);
    return res;
}



/*
  For standard dwarf .debug_frame
  cie_id is -1  in a cie, and
  is the section offset in the .debug_frame section
  of the cie otherwise.  Standard dwarf
*/
int
dwarf_get_fde_list(Dwarf_Debug dbg,
		   Dwarf_Cie ** cie_data,
		   Dwarf_Signed * cie_element_count,
		   Dwarf_Fde ** fde_data,
		   Dwarf_Signed * fde_element_count,
		   Dwarf_Error * error)
{
    int res;

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_frame_index,
			    &dbg->de_debug_frame, error);

    if (res != DW_DLV_OK) {
	return res;
    }

    res =
	_dwarf_get_fde_list_internal(dbg, cie_data,
				     cie_element_count,
				     fde_data,
				     fde_element_count,
				     dbg->de_debug_frame,
				     dbg->de_debug_frame_index,
				     dbg->de_debug_frame_size,
				     DW_CIE_ID,
				     /* use_gnu_cie_calc= */ 0,
				     error);

    return res;
}


/*
   Only works on dwarf sections, not eh_frame
   Givenn a Dwarf_Die, see if it has a
   DW_AT_MIPS_fde attribute and if so use that
   to get an fde offset.
   Then create a Dwarf_Fde to return thru the ret_fde pointer.
   Also creates a cie (pointed at from the Dwarf_Fde).
*/
int
dwarf_get_fde_for_die(Dwarf_Debug dbg,
		      Dwarf_Die die,
		      Dwarf_Fde * ret_fde, Dwarf_Error * error)
{
    Dwarf_Attribute attr;
    Dwarf_Unsigned fde_offset = 0;
    Dwarf_Signed signdval = 0;
    Dwarf_Fde new_fde = 0;
    unsigned char *fde_ptr = 0;
    unsigned char *cie_ptr = 0;
    Dwarf_Unsigned cie_id = 0;

    /* Fields for the current Cie being read. */
    int res;
    int resattr;
    int sdatares;

    struct cie_fde_prefix_s prefix;
    struct cie_fde_prefix_s prefix_c;

    if (die == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DIE_NULL);
	return (DW_DLV_ERROR);
    }

    resattr = dwarf_attr(die, DW_AT_MIPS_fde, &attr, error);
    if (resattr != DW_DLV_OK) {
	return resattr;
    }

    /* why is this formsdata? FIX */
    sdatares = dwarf_formsdata(attr, &signdval, error);
    if (sdatares != DW_DLV_OK) {
	return sdatares;
    }

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_frame_index,
			    &dbg->de_debug_frame, error);
    if (res != DW_DLV_OK) {
	return res;
    }

    fde_offset = signdval;
    fde_ptr = (dbg->de_debug_frame + fde_offset);


    /* First read in the 'common prefix' to figure out what * we are to 
       do with this entry. */
    memset(&prefix_c, 0, sizeof(prefix_c));
    memset(&prefix, 0, sizeof(prefix));
    res = dwarf_read_cie_fde_prefix(dbg, fde_ptr,
				    /* section_ptr= */ 0,
				    /* section_index= */ 0,
				    /* section_length= */ 0,
				    &prefix, error);
    if (res == DW_DLV_ERROR) {
	return res;
    }
    if (res == DW_DLV_NO_ENTRY)
	return res;
    fde_ptr = prefix.cf_addr_after_prefix;
    cie_id = prefix.cf_cie_id;
    res = dwarf_create_fde_from_after_start(dbg, &prefix, fde_ptr,
					    /* use_gnu_cie_calc= */ 0,
					    /* Dwarf_Cie = */ 0,
					    &new_fde, error);

    if (res == DW_DLV_ERROR) {
	return res;
    } else if (res == DW_DLV_NO_ENTRY) {
	return res;
    }
    /* DW_DLV_OK */

    /* now read the cie corresponding to the fde */
    cie_ptr = new_fde->fd_section_ptr + cie_id;
    res = dwarf_read_cie_fde_prefix(dbg, cie_ptr,	/* section_ptr= 
							 */ 0,
				    /* section_index= */ 0,
				    /* section_length= */ 0,
				    &prefix_c, error);
    if (res == DW_DLV_ERROR) {
	return res;
    }
    if (res == DW_DLV_NO_ENTRY)
	return res;

    cie_ptr = prefix_c.cf_addr_after_prefix;
    cie_id = prefix_c.cf_cie_id;


    if (cie_id == DW_CIE_ID) {
	int res2 = 0;
	Dwarf_Cie new_cie = 0;

	res2 = dwarf_create_cie_from_after_start(dbg,
						 &prefix_c, cie_ptr,
						 /* cie_count= */ 0,
						 /* use_gnu_cie_calc= */
						 0, &new_cie, error);
	if (res2 == DW_DLV_ERROR) {
	    dwarf_dealloc(dbg, new_fde, DW_DLA_FDE);
	    return res;
	} else if (res2 == DW_DLV_NO_ENTRY) {
	    dwarf_dealloc(dbg, new_fde, DW_DLA_FDE);
	    return res;
	}


	new_fde->fd_cie = new_cie;
    } else {
	_dwarf_error(dbg, error, DW_DLE_NO_CIE_FOR_FDE);
	return (DW_DLV_ERROR);
    }

    *ret_fde = new_fde;
    return DW_DLV_OK;
}

/* A dwarf consumer operation, see the consumer library documentation.
*/
int
dwarf_get_fde_range(Dwarf_Fde fde,
		    Dwarf_Addr * low_pc,
		    Dwarf_Unsigned * func_length,
		    Dwarf_Ptr * fde_bytes,
		    Dwarf_Unsigned * fde_byte_length,
		    Dwarf_Off * cie_offset,
		    Dwarf_Signed * cie_index,
		    Dwarf_Off * fde_offset, Dwarf_Error * error)
{
    Dwarf_Debug dbg;

    if (fde == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_NULL);
	return (DW_DLV_ERROR);
    }

    dbg = fde->fd_dbg;
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_DBG_NULL);
	return (DW_DLV_ERROR);
    }


    /* We have always already done the section load here, so no need to 
       load the section. We did the section load in order to create the 
       Dwarf_Fde pointer passed in here. */


    if (low_pc != NULL)
	*low_pc = fde->fd_initial_location;
    if (func_length != NULL)
	*func_length = fde->fd_address_range;
    if (fde_bytes != NULL)
	*fde_bytes = fde->fd_fde_start;
    if (fde_byte_length != NULL)
	*fde_byte_length = fde->fd_length;
    if (cie_offset != NULL)
	*cie_offset = fde->fd_cie_offset;
    if (cie_index != NULL)
	*cie_index = fde->fd_cie_index;
    if (fde_offset != NULL)
	*fde_offset = fde->fd_fde_start - fde->fd_section_ptr;

    return DW_DLV_OK;
}

/* IRIX specific function.   The exception tables
   have C++ destructor information and are
   at present undocumented.  */
int
dwarf_get_fde_exception_info(Dwarf_Fde fde,
			     Dwarf_Signed *
			     offset_into_exception_tables,
			     Dwarf_Error * error)
{
    Dwarf_Debug dbg;

    dbg = fde->fd_dbg;
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_DBG_NULL);
	return (DW_DLV_ERROR);
    }
    *offset_into_exception_tables =
	fde->fd_offset_into_exception_tables;
    return DW_DLV_OK;
}


/* A consumer code function.
   Given a CIE pointer, return the normal CIE data thru
   pointers.
   Special augmentation data is not returned here.
*/
int
dwarf_get_cie_info(Dwarf_Cie cie,
		   Dwarf_Unsigned * bytes_in_cie,
		   Dwarf_Small * ptr_to_version,
		   char **augmenter,
		   Dwarf_Unsigned * code_alignment_factor,
		   Dwarf_Signed * data_alignment_factor,
		   Dwarf_Half * return_address_register,
		   Dwarf_Ptr * initial_instructions,
		   Dwarf_Unsigned * initial_instructions_length,
		   Dwarf_Error * error)
{
    Dwarf_Debug dbg;

    if (cie == NULL) {
	_dwarf_error(NULL, error, DW_DLE_CIE_NULL);
	return (DW_DLV_ERROR);
    }

    dbg = cie->ci_dbg;
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_CIE_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    if (ptr_to_version != NULL)
	*ptr_to_version = cie->ci_cie_version_number;
    if (augmenter != NULL)
	*augmenter = cie->ci_augmentation;
    if (code_alignment_factor != NULL)
	*code_alignment_factor = cie->ci_code_alignment_factor;
    if (data_alignment_factor != NULL)
	*data_alignment_factor = cie->ci_data_alignment_factor;
    if (return_address_register != NULL)
	*return_address_register = cie->ci_return_address_register;
    if (initial_instructions != NULL)
	*initial_instructions = cie->ci_cie_instr_start;
    if (initial_instructions_length != NULL) {
	*initial_instructions_length = cie->ci_length +
	    cie->ci_length_size +
	    cie->ci_extension_size -
	    (cie->ci_cie_instr_start - cie->ci_cie_start);

    }
    *bytes_in_cie = (cie->ci_length);
    return (DW_DLV_OK);
}

/* Return the register rules for all registers at a given pc. 
*/
static int
_dwarf_get_fde_info_for_a_pc_row(Dwarf_Fde fde,
				 Dwarf_Addr pc_requested,
				 Dwarf_Frame table, Dwarf_Error * error)
{
    Dwarf_Debug dbg;
    Dwarf_Cie cie;
    Dwarf_Sword i;
    int dw_err;
    Dwarf_Sword icount;
    int res;

    if (fde == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_NULL);
	return (DW_DLV_ERROR);
    }

    dbg = fde->fd_dbg;
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    if (pc_requested < fde->fd_initial_location ||
	pc_requested >=
	fde->fd_initial_location + fde->fd_address_range) {
	_dwarf_error(dbg, error, DW_DLE_PC_NOT_IN_FDE_RANGE);
	return (DW_DLV_ERROR);
    }

    cie = fde->fd_cie;
    if (cie->ci_initial_table == NULL) {
	cie->ci_initial_table = _dwarf_get_alloc(dbg, DW_DLA_FRAME, 1);
	if (cie->ci_initial_table == NULL) {
	    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
	    return (DW_DLV_ERROR);
	}
	for (i = 0; i < DW_FRAME_LAST_REG_NUM; i++) {
	    cie->ci_initial_table->fr_reg[i].ru_is_off = 0;
	    cie->ci_initial_table->fr_reg[i].ru_register =
		DW_FRAME_SAME_VAL;
	    cie->ci_initial_table->fr_reg[i].ru_offset = 0;
	}


	res = _dwarf_exec_frame_instr( /* make_instr= */ false,
				      /* ret_frame_instr= */ NULL,
				      /* search_pc */ false,
				      /* search_pc_val */ 0,
				      /* location */ 0,
				      cie->ci_cie_instr_start,
				      cie->ci_cie_instr_start +
				      (cie->ci_length +
				       cie->ci_length_size +
				       cie->ci_extension_size -
				       (cie->ci_cie_instr_start -
					cie->ci_cie_start)),
				      cie->ci_initial_table, cie, dbg,
				      &icount, &dw_err);
	if (res == DW_DLV_ERROR) {
	    _dwarf_error(dbg, error, dw_err);
	    return (res);
	} else if (res == DW_DLV_NO_ENTRY) {
	    return res;
	}
    }

    {
	Dwarf_Small *instr_end = fde->fd_fde_instr_start +
	    fde->fd_length +
	    fde->fd_length_size +
	    fde->fd_extension_size -
	    (fde->fd_fde_instr_start - fde->fd_fde_start);

	res = _dwarf_exec_frame_instr( /* make_instr= */ false,
				      /* ret_frame_instr= */ NULL,
				      /* search_pc */ true,
				      /* search_pc_val */ pc_requested,
				      fde->fd_initial_location,
				      fde->fd_fde_instr_start,
				      instr_end,
				      table, cie, dbg, &icount,
				      &dw_err);
    }
    if (res == DW_DLV_ERROR) {
	_dwarf_error(dbg, error, dw_err);
	return (res);
    } else if (res == DW_DLV_NO_ENTRY) {
	return res;
    }

    return DW_DLV_OK;
}

/* A consumer call for efficiently getting the register info
   for all registers in one call.
*/
int
dwarf_get_fde_info_for_all_regs(Dwarf_Fde fde,
				Dwarf_Addr pc_requested,
				Dwarf_Regtable * reg_table,
				Dwarf_Addr * row_pc,
				Dwarf_Error * error)
{

    struct Dwarf_Frame_s fde_table;
    Dwarf_Sword i;
    int res;

    /* _dwarf_get_fde_info_for_a_pc_row will perform more sanity checks 
     */
    res = _dwarf_get_fde_info_for_a_pc_row(fde, pc_requested,
					   &fde_table, error);
    if (res != DW_DLV_OK) {
	return res;
    }

    for (i = 0; i < DW_REG_TABLE_SIZE; i++) {
	reg_table->rules[i].dw_offset_relevant =
	    fde_table.fr_reg[i].ru_is_off;
	reg_table->rules[i].dw_regnum = fde_table.fr_reg[i].ru_register;
	reg_table->rules[i].dw_offset = fde_table.fr_reg[i].ru_offset;
    }

    if (row_pc != NULL)
	*row_pc = fde_table.fr_loc;

    return DW_DLV_OK;
}


/* Gets the register info for a single register at a given PC value
   for the FDE specified.
*/
int
dwarf_get_fde_info_for_reg(Dwarf_Fde fde,
			   Dwarf_Half table_column,
			   Dwarf_Addr pc_requested,
			   Dwarf_Signed * offset_relevant,
			   Dwarf_Signed * register_num,
			   Dwarf_Signed * offset,
			   Dwarf_Addr * row_pc, Dwarf_Error * error)
{
    struct Dwarf_Frame_s fde_table;
    int res;


    if (table_column > DW_FRAME_LAST_REG_NUM) {
	_dwarf_error(NULL, error, DW_DLE_FRAME_TABLE_COL_BAD);
	return (DW_DLV_ERROR);
    }

    /* _dwarf_get_fde_info_for_a_pc_row will perform more sanity checks 
     */
    res =
	_dwarf_get_fde_info_for_a_pc_row(fde, pc_requested, &fde_table,
					 error);
    if (res != DW_DLV_OK) {
	return res;
    }

    if (register_num != NULL)
	*register_num = fde_table.fr_reg[table_column].ru_register;
    if (offset != NULL)
	*offset = fde_table.fr_reg[table_column].ru_offset;
    if (row_pc != NULL)
	*row_pc = fde_table.fr_loc;

    *offset_relevant = (fde_table.fr_reg[table_column].ru_is_off);
    return DW_DLV_OK;
}

/*
	Return pointer to the instructions in the dwarf
	fde.
*/
int
dwarf_get_fde_instr_bytes(Dwarf_Fde inFde, Dwarf_Ptr * outinstraddr,
			  Dwarf_Unsigned * outaddrlen,
			  Dwarf_Error * error)
{
    Dwarf_Unsigned len = 0;
    unsigned char *instrs = 0;
    Dwarf_Debug dbg;

    if (inFde == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_NULL);
	return (DW_DLV_ERROR);
    }

    dbg = inFde->fd_dbg;
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    instrs = inFde->fd_fde_instr_start;

    len = (inFde->fd_fde_start + inFde->fd_length +
	   inFde->fd_length_size + inFde->fd_extension_size)
	- instrs;

    *outinstraddr = instrs;
    *outaddrlen = len;
    return DW_DLV_OK;
}

/* Allows getting an fde from its table via an index.  
   With more error checking than simply indexing oneself.
*/
int
dwarf_get_fde_n(Dwarf_Fde * fde_data,
		Dwarf_Unsigned fde_index,
		Dwarf_Fde * returned_fde, Dwarf_Error * error)
{
    Dwarf_Debug dbg;

    if (fde_data == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_PTR_NULL);
	return (DW_DLV_ERROR);
    }

    if (*fde_data == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_NULL);
	return (DW_DLV_ERROR);
    }

    dbg = (*fde_data)->fd_dbg;
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    if (fde_index >= dbg->de_fde_count) {
	return (DW_DLV_NO_ENTRY);
    }
    *returned_fde = (*(fde_data + fde_index));
    return DW_DLV_OK;
}


/* 
    Lopc and hipc are extensions to the interface to 
    return the range of addresses that are described
    by the returned fde.
*/
int
dwarf_get_fde_at_pc(Dwarf_Fde * fde_data,
		    Dwarf_Addr pc_of_interest,
		    Dwarf_Fde * returned_fde,
		    Dwarf_Addr * lopc,
		    Dwarf_Addr * hipc, Dwarf_Error * error)
{
    Dwarf_Debug dbg;
    Dwarf_Fde fde = NULL;

    if (fde_data == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_PTR_NULL);
	return (DW_DLV_ERROR);
    }

    if (*fde_data == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_NULL);
	return (DW_DLV_ERROR);
    }

    dbg = (*fde_data)->fd_dbg;
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_DBG_NULL);
	return (DW_DLV_ERROR);
    }
    {
	/* The fde's are sorted by their addresses. Binary search to
	   find correct fde. */
	int low = 0;
	int high = dbg->de_fde_count - 1;
	int middle = 0;
	Dwarf_Fde cur_fde;

	while (low <= high) {
	    middle = (low + high) / 2;
	    cur_fde = fde_data[middle];
	    if (pc_of_interest < cur_fde->fd_initial_location) {
		high = middle - 1;
	    } else if (pc_of_interest >=
		       (cur_fde->fd_initial_location +
			cur_fde->fd_address_range)) {
		low = middle + 1;
	    } else {
		fde = fde_data[middle];
		break;
	    }
	}
    }

    if (fde) {
	if (lopc != NULL)
	    *lopc = fde->fd_initial_location;
	if (hipc != NULL)
	    *hipc = fde->fd_initial_location +
		fde->fd_address_range - 1;
	*returned_fde = fde;
	return (DW_DLV_OK);
    }

    return (DW_DLV_NO_ENTRY);
}


/* Expands a single frame instruction block
   into a n array of Dwarf_Frame_Op-s.
*/
int
dwarf_expand_frame_instructions(Dwarf_Debug dbg,
				Dwarf_Ptr instruction,
				Dwarf_Unsigned i_length,
				Dwarf_Frame_Op ** returned_op_list,
				Dwarf_Signed * returned_op_count,
				Dwarf_Error * error)
{
    Dwarf_Sword instr_count;
    int res;
    int dw_err;

    if (dbg == 0) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    if (returned_op_list == 0 || returned_op_count == 0) {
	_dwarf_error(dbg, error, DW_DLE_RET_OP_LIST_NULL);
	return (DW_DLV_ERROR);
    }

    /* The cast to Dwarf_Ptr may get a compiler warning, but it is safe 
       as it is just an i_length offset from 'instruction' itself. A
       caller has made a big mistake if the result is not a valid
       pointer. */
    res = _dwarf_exec_frame_instr( /* make_instr= */ true,
				  returned_op_list,
				  /* search_pc */ false,
				  /* search_pc_val */ 0,
				  /* location */ 0,
				  instruction,
				  (Dwarf_Ptr) ((Dwarf_Unsigned)
					       instruction + i_length),
				  /* Dwarf_Frame */ NULL,
				  /* cie_ptr */ NULL,
				  dbg, &instr_count, &dw_err);
    if (res != DW_DLV_OK) {
	if (res == DW_DLV_ERROR) {
	    _dwarf_error(dbg, error, dw_err);
	}
	return (res);
    }

    *returned_op_count = instr_count;
    return DW_DLV_OK;
}


/* Used by dwarfdump -v to print offsets, for debugging
   dwarf info
*/
/* ARGSUSED 4 */
int
_dwarf_fde_section_offset(Dwarf_Debug dbg, Dwarf_Fde in_fde,
			  Dwarf_Off * fde_off, Dwarf_Off * cie_off,
			  Dwarf_Error * err)
{
    int res;
    char *start;
    char *loc;

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_frame_index,
			    &dbg->de_debug_frame, err);
    if (res != DW_DLV_OK) {
	return res;
    }

    start = (char *) dbg->de_debug_frame;
    loc = (char *) in_fde->fd_fde_start;

    *fde_off = (loc - start);
    *cie_off = in_fde->fd_cie_offset;
    return DW_DLV_OK;
}

/* Used by dwarfdump -v to print offsets, for debugging
   dwarf info
*/
/* ARGSUSED 4 */
int
_dwarf_cie_section_offset(Dwarf_Debug dbg, Dwarf_Cie in_cie,
			  Dwarf_Off * cie_off, Dwarf_Error * err)
{
    int res;
    char *start;
    char *loc;

    res =
	_dwarf_load_section(dbg,
			    dbg->de_debug_frame_index,
			    &dbg->de_debug_frame, err);
    if (res != DW_DLV_OK) {
	return res;
    }

    start = (char *) dbg->de_debug_frame;
    loc = (char *) in_cie->ci_cie_start;

    *cie_off = (loc - start);
    return DW_DLV_OK;
}

/* Returns  a pointer to target-specific augmentation data thru augdata
   and returns the length of the data thru augdata_len.

   It's up to the consumer code to know how to interpret the bytes
   of target-specific data (endian issues apply too, these
   are just raw bytes pointed to).
   See  Linux Standard Base Core Specification version 3.0 for
   the details on .eh_frame info.

   Returns DW_DLV_ERROR if fde is NULL or some other serious
   error.
   Returns DW_DLV_NO_ENTRY if there is no target-specific
   augmentation data. 

   The bytes pointed to are in the Dwarf_Cie, and as long as that
   is valid the bytes are there. No 'dealloc' call is needed
   for the bytes.
*/
int
dwarf_get_cie_augmentation_data(Dwarf_Cie cie,
				Dwarf_Ptr * augdata,
				Dwarf_Unsigned * augdata_len,
				Dwarf_Error * error)
{
    if (cie == NULL) {
	_dwarf_error(NULL, error, DW_DLE_CIE_NULL);
	return (DW_DLV_ERROR);
    }
    if (cie->ci_gnu_eh_augmentation_len == 0) {
	return DW_DLV_NO_ENTRY;
    }
    *augdata = cie->ci_gnu_eh_augmentation_bytes;
    *augdata_len = cie->ci_gnu_eh_augmentation_len;
    return DW_DLV_OK;
}


/* Returns  a pointer to target-specific augmentation data thru augdata
   and returns the length of the data thru augdata_len.

   It's up to the consumer code to know how to interpret the bytes
   of target-specific data (endian issues apply too, these
   are just raw bytes pointed to).
   See  Linux Standard Base Core Specification version 3.0 for
   the details on .eh_frame info.

   Returns DW_DLV_ERROR if fde is NULL or some other serious
   error.
   Returns DW_DLV_NO_ENTRY if there is no target-specific
   augmentation data. 

   The bytes pointed to are in the Dwarf_Fde, and as long as that
   is valid the bytes are there. No 'dealloc' call is needed
   for the bytes.
*/
int
dwarf_get_fde_augmentation_data(Dwarf_Fde fde,
				Dwarf_Ptr * augdata,
				Dwarf_Unsigned * augdata_len,
				Dwarf_Error * error)
{
    Dwarf_Cie cie = 0;

    if (fde == NULL) {
	_dwarf_error(NULL, error, DW_DLE_FDE_NULL);
	return (DW_DLV_ERROR);
    }
    cie = fde->fd_cie;
    if (cie == NULL) {
	_dwarf_error(NULL, error, DW_DLE_CIE_NULL);
	return (DW_DLV_ERROR);
    }
    if (cie->ci_gnu_eh_augmentation_len == 0) {
	return DW_DLV_NO_ENTRY;
    }
    *augdata = fde->fd_gnu_eh_augmentation_bytes;
    *augdata_len = fde->fd_gnu_eh_augmentation_len;
    return DW_DLV_OK;
}
