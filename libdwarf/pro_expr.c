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

#include "config.h"
#include "libdwarfdefs.h"
#include <stdio.h>
#include <string.h>
#include "pro_incl.h"
#include "pro_expr.h"

/*
    This function creates a new expression 
    struct that can be used to build up a
    location expression.
*/
Dwarf_P_Expr
dwarf_new_expr (
    Dwarf_P_Debug	dbg,
    Dwarf_Error		*error
)
{
    Dwarf_P_Expr	ret_expr;

    if (dbg == NULL) {
	_dwarf_p_error(NULL, error, DW_DLE_DBG_NULL);
	return(NULL);
    }

    ret_expr = (Dwarf_P_Expr)
	_dwarf_p_get_alloc(dbg, sizeof(struct Dwarf_P_Expr_s));
    if (ret_expr == NULL) {
	_dwarf_p_error(dbg, error, DW_DLE_ALLOC_FAIL);
	return(NULL);
    }

    ret_expr->ex_dbg = dbg;

    return(ret_expr);
}


Dwarf_Unsigned
dwarf_add_expr_gen (
    Dwarf_P_Expr	expr,
    Dwarf_Small		opcode,
    Dwarf_Unsigned	val1,
    Dwarf_Unsigned	val2,
    Dwarf_Error		*error
)
{
    char encode_buffer[2*ENCODE_SPACE_NEEDED]; /* 2* since used
		to concatenate 2 leb's below */
    char encode_buffer2[ENCODE_SPACE_NEEDED];
    int  res;
    Dwarf_P_Debug dbg = expr->ex_dbg;
	/* 
	    Give the buffer where the operands are first
	    going to be assembled the largest alignment.
	*/
    Dwarf_Unsigned	operand_buffer[10];

	/* 
	    Size of the byte stream buffer that needs 
	    to be memcpy-ed.
	*/
    int			operand_size;

	/* 
	    Points to the byte stream for the first operand,
	    and finally to the buffer that is memcp-ed into
	    the Dwarf_P_Expr_s struct.
	*/
    Dwarf_Small		*operand;

	/* Size of the byte stream for second operand. */
    int			operand2_size;

	/* Points to next byte to be written in Dwarf_P_Expr_s struct. */
    Dwarf_Small		*next_byte_ptr;

	/* Offset past the last byte written into Dwarf_P_Expr_s. */
    int			next_byte_offset;

    /* ***** BEGIN CODE ***** */

    if (expr == NULL) {
	_dwarf_p_error(NULL, error, DW_DLE_EXPR_NULL);
	return(DW_DLV_NOCOUNT);
    }

    if (expr->ex_dbg == NULL) {
	_dwarf_p_error(NULL, error, DW_DLE_DBG_NULL);
	return(DW_DLV_NOCOUNT);
    }

    operand = NULL;
    operand_size = 0;

    switch (opcode) {
        case DW_OP_reg0 :
	case DW_OP_reg1 :
	case DW_OP_reg2 :
	case DW_OP_reg3 :
	case DW_OP_reg4 :
	case DW_OP_reg5 :
	case DW_OP_reg6 :
	case DW_OP_reg7 :
	case DW_OP_reg8 :
	case DW_OP_reg9 :
	case DW_OP_reg10 :
	case DW_OP_reg11 :
	case DW_OP_reg12 :
	case DW_OP_reg13 :
	case DW_OP_reg14 :
	case DW_OP_reg15 :
	case DW_OP_reg16 :
	case DW_OP_reg17 :
	case DW_OP_reg18 :
	case DW_OP_reg19 :
	case DW_OP_reg20 :
	case DW_OP_reg21 :
	case DW_OP_reg22 :
	case DW_OP_reg23 :
	case DW_OP_reg24 :
	case DW_OP_reg25 :
	case DW_OP_reg26 :
	case DW_OP_reg27 :
	case DW_OP_reg28 :
	case DW_OP_reg29 :
	case DW_OP_reg30 :
	case DW_OP_reg31 :
	    break;

	case DW_OP_breg0 :
	case DW_OP_breg1 :
	case DW_OP_breg2 :
	case DW_OP_breg3 :
	case DW_OP_breg4 :
	case DW_OP_breg5 :
	case DW_OP_breg6 :
	case DW_OP_breg7 :
	case DW_OP_breg8 :
	case DW_OP_breg9 :
	case DW_OP_breg10 :
	case DW_OP_breg11 :
	case DW_OP_breg12 :
	case DW_OP_breg13 :
	case DW_OP_breg14 :
	case DW_OP_breg15 :
	case DW_OP_breg16 :
	case DW_OP_breg17 :
	case DW_OP_breg18 :
	case DW_OP_breg19 :
	case DW_OP_breg20 :
	case DW_OP_breg21 :
	case DW_OP_breg22 :
	case DW_OP_breg23 :
	case DW_OP_breg24 :
	case DW_OP_breg25 :
	case DW_OP_breg26 :
	case DW_OP_breg27 :
	case DW_OP_breg28 :
	case DW_OP_breg29 :
	case DW_OP_breg30 :
	case DW_OP_breg31 :
	    res = _dwarf_pro_encode_signed_leb128_nm(val1, 
			&operand_size,encode_buffer,sizeof(encode_buffer));
	    if(res != DW_DLV_OK) {
	        _dwarf_p_error(expr->ex_dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	        return(DW_DLV_NOCOUNT);
	    }
	    operand = (Dwarf_Small *)encode_buffer;
	    break;

        case DW_OP_regx :
	    res = _dwarf_pro_encode_leb128_nm(val1, &operand_size,
			encode_buffer, sizeof(encode_buffer));
	    if(res != DW_DLV_OK) {
	        _dwarf_p_error(expr->ex_dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	        return(DW_DLV_NOCOUNT);
	    }
	    operand = (Dwarf_Small *)encode_buffer;
	    break;

	case DW_OP_lit0 :
	case DW_OP_lit1 :
	case DW_OP_lit2 :
	case DW_OP_lit3 :
	case DW_OP_lit4 :
	case DW_OP_lit5 :
	case DW_OP_lit6 :
	case DW_OP_lit7 :
	case DW_OP_lit8 :
	case DW_OP_lit9 :
	case DW_OP_lit10 :
	case DW_OP_lit11 :
	case DW_OP_lit12 :
	case DW_OP_lit13 :
	case DW_OP_lit14 :
	case DW_OP_lit15 :
	case DW_OP_lit16 :
	case DW_OP_lit17 :
	case DW_OP_lit18 :
	case DW_OP_lit19 :
	case DW_OP_lit20 :
	case DW_OP_lit21 :
	case DW_OP_lit22 :
	case DW_OP_lit23 :
	case DW_OP_lit24 :
	case DW_OP_lit25 :
	case DW_OP_lit26 :
	case DW_OP_lit27 :
	case DW_OP_lit28 :
	case DW_OP_lit29 :
	case DW_OP_lit30 :
	case DW_OP_lit31 :
	    break;
        
	case DW_OP_addr :
	    _dwarf_p_error(expr->ex_dbg, error, DW_DLE_BAD_EXPR_OPCODE);
	    return(DW_DLV_NOCOUNT);

	case DW_OP_const1u :
	case DW_OP_const1s :
            operand = (Dwarf_Small *)&operand_buffer[0];
	    WRITE_UNALIGNED(dbg,operand,&val1,sizeof(val1),1);
	    operand_size = 1;
	    break;

	case DW_OP_const2u :
	case DW_OP_const2s :
	    operand = (Dwarf_Small *)&operand_buffer[0];
	    WRITE_UNALIGNED(dbg,operand,&val1,sizeof(val1),2);
	    operand_size = 2;
	    break;

	case DW_OP_const4u :
	case DW_OP_const4s :
	    operand = (Dwarf_Small *)&operand_buffer[0];
	    WRITE_UNALIGNED(dbg,operand,&val1,sizeof(val1),4);
	    operand_size = 4;
	    break;

	case DW_OP_const8u :
	case DW_OP_const8s :
	    operand = (Dwarf_Small *)&operand_buffer[0];
	    WRITE_UNALIGNED(dbg,operand,&val1,sizeof(val1),8);
	    operand_size = 8;
	    break;

        case DW_OP_constu :
	    res = _dwarf_pro_encode_leb128_nm(val1, 
			&operand_size,
			encode_buffer, sizeof(encode_buffer));
	    if(res != DW_DLV_OK) {
	        _dwarf_p_error(expr->ex_dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	        return(DW_DLV_NOCOUNT);
	    }
	    operand = (Dwarf_Small *)encode_buffer;
	    break;

	case DW_OP_consts :
	    res = _dwarf_pro_encode_signed_leb128_nm(val1, 
			&operand_size,
			encode_buffer, sizeof(encode_buffer));
	    if(res != DW_DLV_OK) {
	        _dwarf_p_error(expr->ex_dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	        return(DW_DLV_NOCOUNT);
	    }
	    operand = (Dwarf_Small *)encode_buffer;
	    break;

	case DW_OP_fbreg :
	    res = _dwarf_pro_encode_signed_leb128_nm(val1, 
			&operand_size,
			encode_buffer, sizeof(encode_buffer));
	    if(res != DW_DLV_OK) {
	        _dwarf_p_error(expr->ex_dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	        return(DW_DLV_NOCOUNT);
	    }
	    operand = (Dwarf_Small *)encode_buffer;
	    break;

	case DW_OP_bregx :
	    res = _dwarf_pro_encode_leb128_nm(val1, &operand_size,
			encode_buffer, sizeof(encode_buffer));
	    if(res != DW_DLV_OK) {
	        _dwarf_p_error(expr->ex_dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	        return(DW_DLV_NOCOUNT);
	    }
	    operand = (Dwarf_Small *)encode_buffer;
		/* put this one directly into 'operand' at tail of
		   prev value
	 	*/
	    res =  _dwarf_pro_encode_signed_leb128_nm(val2, &operand2_size,
			((char *)operand)+operand_size, sizeof(encode_buffer2));
	    if(res != DW_DLV_OK) {
	        _dwarf_p_error(expr->ex_dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	        return(DW_DLV_NOCOUNT);
	    }
	    operand_size += operand2_size;

	case DW_OP_dup :
	case DW_OP_drop :
	    break;

	case DW_OP_pick :
	    operand = (Dwarf_Small *)&operand_buffer[0];
	    WRITE_UNALIGNED(dbg,operand,(const void *)val1,
			sizeof(val1),1);
	    operand_size = 1;
	    break;

	case DW_OP_over :
	case DW_OP_swap :
	case DW_OP_rot :
	case DW_OP_deref :
	case DW_OP_xderef :
            break;

	case DW_OP_deref_size :
	case DW_OP_xderef_size :
	    operand = (Dwarf_Small *)&operand_buffer[0];
	    WRITE_UNALIGNED(dbg,operand,(const void *)val1,
			sizeof(val1),1);
	    operand_size = 1;
	    break;

        case DW_OP_abs :
	case DW_OP_and :
	case DW_OP_div :
	case DW_OP_minus :
	case DW_OP_mod :
	case DW_OP_mul :
	case DW_OP_neg :
	case DW_OP_not :
	case DW_OP_or :
	case DW_OP_plus :
	    break;

	case DW_OP_plus_uconst :
	    res = _dwarf_pro_encode_leb128_nm(val1, &operand_size,
			encode_buffer, sizeof(encode_buffer));
	    if(res != DW_DLV_OK) {
	        _dwarf_p_error(expr->ex_dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	        return(DW_DLV_NOCOUNT);
	    }
	    operand = (Dwarf_Small *)encode_buffer;
	    break;

	case DW_OP_shl :
	case DW_OP_shr :
	case DW_OP_shra :
	case DW_OP_xor :
	    break;

	case DW_OP_le :
	case DW_OP_ge :
	case DW_OP_eq :
	case DW_OP_lt :
	case DW_OP_gt :
	case DW_OP_ne :
	    break;

	case DW_OP_skip :
	case DW_OP_bra :
            /* FIX: unhandled! OP_bra, OP_skip! */
	    _dwarf_p_error(expr->ex_dbg, error, DW_DLE_BAD_EXPR_OPCODE);
	    return(DW_DLV_NOCOUNT);

	case DW_OP_piece :
	    res = _dwarf_pro_encode_leb128_nm(val1, &operand_size,
			encode_buffer, sizeof(encode_buffer));
	    if(res != DW_DLV_OK) {
	        _dwarf_p_error(expr->ex_dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	        return(DW_DLV_NOCOUNT);
	    }
	    operand = (Dwarf_Small *)encode_buffer;
	    break;

	case DW_OP_nop :
	    break;

	default :
	    _dwarf_p_error(expr->ex_dbg, error, DW_DLE_BAD_EXPR_OPCODE);
	    return(DW_DLV_NOCOUNT);
    }

    next_byte_offset = expr->ex_next_byte_offset + operand_size + 1;

    if (next_byte_offset > MAXIMUM_LOC_EXPR_LENGTH) {
	_dwarf_p_error(expr->ex_dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	return(DW_DLV_NOCOUNT);
    }

    next_byte_ptr = &(expr->ex_byte_stream[0]) + expr->ex_next_byte_offset;

    *next_byte_ptr = opcode;
    next_byte_ptr++;
    memcpy(next_byte_ptr, operand, operand_size);

    expr->ex_next_byte_offset = next_byte_offset;
    return(next_byte_offset);
}

Dwarf_Unsigned
dwarf_add_expr_addr_b (
    Dwarf_P_Expr	expr,
    Dwarf_Unsigned	addr,
    Dwarf_Unsigned	sym_index,
    Dwarf_Error		*error
)
{
    Dwarf_P_Debug		dbg;
    Dwarf_Small		*next_byte_ptr;
    Dwarf_Unsigned	next_byte_offset;
    int upointer_size;

    if (expr == NULL) {
	_dwarf_p_error(NULL, error, DW_DLE_EXPR_NULL);
	return(DW_DLV_NOCOUNT);
    }

    dbg = expr->ex_dbg;
    if (dbg == NULL) {
	_dwarf_p_error(NULL, error, DW_DLE_DBG_NULL);
	return(DW_DLV_NOCOUNT);
    }

    upointer_size = dbg->de_pointer_size;
    next_byte_offset = expr->ex_next_byte_offset + upointer_size + 1;
    if (next_byte_offset > MAXIMUM_LOC_EXPR_LENGTH) {
	_dwarf_p_error(dbg, error, DW_DLE_EXPR_LENGTH_BAD);
	return(DW_DLV_NOCOUNT);
    }

    next_byte_ptr = &(expr->ex_byte_stream[0]) + expr->ex_next_byte_offset;

    *next_byte_ptr = DW_OP_addr;
    next_byte_ptr++;
    WRITE_UNALIGNED (dbg,next_byte_ptr,(const void *)&addr,
		sizeof(addr),upointer_size);

    if (expr->ex_reloc_offset != 0) {
	_dwarf_p_error(dbg, error, DW_DLE_MULTIPLE_RELOC_IN_EXPR);
	return(DW_DLV_NOCOUNT);
    }

    expr->ex_reloc_sym_index = sym_index;
    expr->ex_reloc_offset = expr->ex_next_byte_offset + 1;

    expr->ex_next_byte_offset = next_byte_offset;
    return(next_byte_offset);
}

Dwarf_Unsigned
dwarf_add_expr_addr (
    Dwarf_P_Expr	expr,
    Dwarf_Unsigned	addr,
    Dwarf_Signed	sym_index,
    Dwarf_Error		*error
)
{
  return 
    dwarf_add_expr_addr_b(expr, addr,(Dwarf_Unsigned) sym_index, error);
}


Dwarf_Unsigned
dwarf_expr_current_offset (
    Dwarf_P_Expr	expr,
    Dwarf_Error		*error
)
{
    if (expr == NULL) {
	_dwarf_p_error(NULL, error, DW_DLE_EXPR_NULL);
	return(DW_DLV_NOCOUNT);
    }

    if (expr->ex_dbg == NULL) {
	_dwarf_p_error(NULL, error, DW_DLE_DBG_NULL);
	return(DW_DLV_NOCOUNT);
    }

    return(expr->ex_next_byte_offset);
}


Dwarf_Addr
dwarf_expr_into_block (
    Dwarf_P_Expr	expr,
    Dwarf_Unsigned	*length,
    Dwarf_Error		*error
)
{
    if (expr == NULL) {
	_dwarf_p_error(NULL, error, DW_DLE_EXPR_NULL);
	return(DW_DLV_BADADDR);
    }

    if (expr->ex_dbg == NULL) {
	_dwarf_p_error(NULL, error, DW_DLE_DBG_NULL);
	return(DW_DLV_BADADDR);
    }

    if (length != NULL) *length = expr->ex_next_byte_offset;
    return((Dwarf_Addr)&(expr->ex_byte_stream[0]));
}
