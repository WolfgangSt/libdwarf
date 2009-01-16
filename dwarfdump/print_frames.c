/* 
  Copyright (C) 2006 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan



$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/print_frames.c,v 1.3 2006/04/18 04:29:39 davea Exp $ */
#include "globals.h"

#include "print_frames.h"
#include "dwconf.h"


static void
print_one_frame_reg_col( Dwarf_Unsigned rule_id,
                        Dwarf_Small value_type,
		Dwarf_Unsigned reg_used,
                        struct dwconf_s *config_data,
		        Dwarf_Signed offset_relevant,
                        Dwarf_Signed offset, 
			Dwarf_Ptr block_ptr);

/*
    Gather the fde print logic here so the control logic
    determining what FDE to print is clearer.
*/
int
print_one_fde(Dwarf_Debug dbg, Dwarf_Fde fde,
	      Dwarf_Unsigned fde_index,
	      Dwarf_Cie * cie_data,
	      Dwarf_Signed cie_element_count,
	      Dwarf_Half address_size, int is_eh,
		struct dwconf_s * config_data)
{
    Dwarf_Addr j = 0;
    Dwarf_Addr low_pc = 0;
    Dwarf_Unsigned func_length = 0;
    Dwarf_Ptr fde_bytes = NULL;
    Dwarf_Unsigned fde_bytes_length = 0;
    Dwarf_Off cie_offset = 0;
    Dwarf_Signed cie_index = 0;
    Dwarf_Off fde_offset = 0;
    Dwarf_Signed eh_table_offset = 0;
    int fres = 0;
    int offres = 0;
    string temps = 0;
    Dwarf_Error err = 0;
    int printed_intro_addr = 0;

    fres = dwarf_get_fde_range(fde,
			       &low_pc, &func_length,
			       &fde_bytes,
			       &fde_bytes_length,
			       &cie_offset, &cie_index,
			       &fde_offset, &err);
    if (fres == DW_DLV_ERROR) {
	print_error(dbg, "dwarf_get_fde_range", fres, err);
    }
    if (fres == DW_DLV_NO_ENTRY) {
	return DW_DLV_NO_ENTRY;
    }
    if (cu_name_flag &&
	fde_offset_for_cu_low != DW_DLV_BADOFFSET &&
	(fde_offset < fde_offset_for_cu_low ||
	 fde_offset > fde_offset_for_cu_high)) {
	return DW_DLV_NO_ENTRY;
    }
    /* eh_table_offset is IRIX ONLY. */
    fres = dwarf_get_fde_exception_info(fde, &eh_table_offset, &err);
    if (fres == DW_DLV_ERROR) {
	print_error(dbg, "dwarf_get_fde_exception_info", fres, err);
    }
    temps = get_fde_proc_name(dbg, low_pc);
    printf
	("<%3lld><%#llx:%#llx><%s><fde offset 0x%llx length: 0x%llx>",
	 cie_index, low_pc, (low_pc + func_length),
	 temps ? temps : "", fde_offset, fde_bytes_length);


    if (!is_eh) {
	/* IRIX uses eh_table_offset. */
	if (eh_table_offset == DW_DLX_NO_EH_OFFSET) {
	    printf("<eh offset %s>\n", "none");
	} else if (eh_table_offset == DW_DLX_EH_OFFSET_UNAVAILABLE) {
	    printf("<eh offset %s>\n", "unknown");
	} else {
	    printf("<eh offset 0x%llx>\n", eh_table_offset);
	}
    } else {
	int ares = 0;
	Dwarf_Small *data = 0;
	Dwarf_Unsigned len = 0;

	ares = dwarf_get_fde_augmentation_data(fde, &data, &len, &err);
	if (ares == DW_DLV_NO_ENTRY) {
	    /* do nothing. */
	} else if (ares == DW_DLV_OK) {
	    int k2;

	    printf("<eh aug data len 0x%llx bytes 0x", (long long) len);
	    for (k2 = 0; k2 < len; ++k2) {
		printf("%02x ", (unsigned char) data[k2]);
	    }
	    printf(">");
	}			/* else DW_DLV_ERROR, do nothing */
    }
    /* call dwarf_get_fde_info_for_reg() to get whole matrix */

    for (j = low_pc; j < low_pc + func_length; j++) {
        Dwarf_Half k;

	if( config_data->cf_interface_number == 3) {
 	    Dwarf_Signed reg = 0;
            Dwarf_Signed offset_relevant = 0;
            Dwarf_Small value_type = 0;
            Dwarf_Signed offset_or_block_len = 0;
            Dwarf_Signed offset = 0;
            Dwarf_Ptr block_ptr = 0;
            Dwarf_Addr row_pc = 0;

            int fires = dwarf_get_fde_info_for_cfa_reg3(fde,
                            j,
                            &value_type,
                            &offset_relevant,
                            &reg,
                            &offset_or_block_len,
                            &block_ptr,
                            &row_pc,
                            &err);
		offset = offset_or_block_len;
		if (fires == DW_DLV_ERROR) {
                  print_error(dbg,
                            "dwarf_get_fde_info_for_reg", fires, err);
                }
                if (fires == DW_DLV_NO_ENTRY) {
                    continue;
                }
 		if (row_pc != j) {
                /* duplicate row */
                   continue;
            }
            if (!printed_intro_addr ) {
                printf("    %08llx:\t", j);
                printed_intro_addr = 1;
            }
            print_one_frame_reg_col(
                config_data->cf_cfa_reg,
                value_type, 
		reg,
		config_data,
		offset_relevant,
                offset,
                block_ptr);
	}
	for (k = 0; k < config_data->cf_table_entry_count ; k++) {
            Dwarf_Signed reg = 0;
	    Dwarf_Signed offset_relevant = 0;
	    int fires = 0;
	    Dwarf_Small value_type = 0;
	    Dwarf_Ptr block_ptr = 0;
	    Dwarf_Signed offset_or_block_len = 0;
            Dwarf_Signed offset = 0;
            Dwarf_Addr row_pc = 0;
	    
	    if( config_data->cf_interface_number == 3) {
	
                fires = dwarf_get_fde_info_for_reg3(fde,
                            k,
                            j,
                            &value_type,
                            &offset_relevant,
                            &reg,
                            &offset_or_block_len,
                            &block_ptr,
                            &row_pc,
                            &err);
		offset = offset_or_block_len;
	    } else { /* ASSERT: config_data->cf_interface_number == 2 */


		  value_type = DW_EXPR_OFFSET ;
	          fires = dwarf_get_fde_info_for_reg(fde, 
			k, 
			j,
			&offset_relevant,
			&reg,
			&offset, &row_pc, &err);
	    }
	    if (fires == DW_DLV_ERROR) {
		print_error(dbg,
			    "dwarf_get_fde_info_for_reg", fires, err);
	    }
	    if (fires == DW_DLV_NO_ENTRY) {
		continue;
	    }
	    if (row_pc != j) {
		/* duplicate row */
		break;
	    }
	    if (!printed_intro_addr ) {
		printf("    %08llx:\t", j);
		printed_intro_addr = 1;
	    }
            print_one_frame_reg_col(
                k,
		value_type, 
		reg,
		config_data,
		offset_relevant,
                offset,
                block_ptr);

	}
        if( printed_intro_addr) {
		printf("\n");
		printed_intro_addr = 0;
	}
    }
    if (verbose > 1) {
	Dwarf_Off fde_off;
	Dwarf_Off cie_off;

	/* get the fde instructions and print them in raw form, just
	   like cie instructions */
	Dwarf_Ptr instrs;
	Dwarf_Unsigned ilen;
	int res;

	res = dwarf_get_fde_instr_bytes(fde, &instrs, &ilen, &err);
	offres =
	    _dwarf_fde_section_offset(dbg, fde, &fde_off, &cie_off,
				      &err);
	if (offres == DW_DLV_OK) {
	    printf("\tfde sec. offset %llu 0x%llx"
		   " cie offset for fde: %llu 0x%llx\n",
		   (unsigned long long) fde_off,
		   (unsigned long long) fde_off,
		   (unsigned long long) cie_off,
		   (unsigned long long) cie_off);

	}


	if (res == DW_DLV_OK) {
	    int cires = 0;
	    Dwarf_Unsigned cie_length = 0;
	    Dwarf_Small version = 0;
	    string augmenter;
	    Dwarf_Unsigned code_alignment_factor = 0;
	    Dwarf_Signed data_alignment_factor = 0;
	    Dwarf_Half return_address_register_rule = 0;
	    Dwarf_Ptr initial_instructions = 0;
	    Dwarf_Unsigned initial_instructions_length = 0;

	    if (cie_index >= cie_element_count) {
		printf("Bad cie index %lld with fde index %lld! "
		       "(table entry max %lld)\n",
		       (long long) cie_index, (long long) fde_index,
		       (long long) cie_element_count);
		exit(1);
	    }

	    cires = dwarf_get_cie_info(cie_data[cie_index],
				       &cie_length,
				       &version,
				       &augmenter,
				       &code_alignment_factor,
				       &data_alignment_factor,
				       &return_address_register_rule,
				       &initial_instructions,
				       &initial_instructions_length,
				       &err);
	    if (cires == DW_DLV_ERROR) {
		printf
		    ("Bad cie index %lld with fde index %lld!\n",
		     (long long) cie_index, (long long) fde_index);
		print_error(dbg, "dwarf_get_cie_info", cires, err);
	    }
	    if (cires == DW_DLV_NO_ENTRY) {
		;		/* ? */
	    } else {

		print_frame_inst_bytes(dbg, instrs, 
					(Dwarf_Signed)ilen,
				       data_alignment_factor,
				       (int)code_alignment_factor,
				       address_size,
				config_data);
	    }
	} else if (res == DW_DLV_NO_ENTRY) {
	    printf
		("Impossible: no instr bytes for fde index %d?\n",
		 (int) fde_index);
	} else {
	    /* DW_DLV_ERROR */
	    printf
		("Error: on gettinginstr bytes for fde index %d?\n",
		 (int) fde_index);
	    print_error(dbg, "dwarf_get_fde_instr_bytes", res, err);
	}

    }
    return DW_DLV_OK;
}


/* Print a cie.  Gather the print logic here so the
   control logic deciding what to print
   is clearer.
*/
int
print_one_cie(Dwarf_Debug dbg, Dwarf_Cie cie,
	      Dwarf_Unsigned cie_index, Dwarf_Half address_size,
		struct dwconf_s * config_data)
{

    int cires = 0;
    Dwarf_Unsigned cie_length = 0;
    Dwarf_Small version = 0;
    string augmenter = "";
    Dwarf_Unsigned code_alignment_factor = 0;
    Dwarf_Signed data_alignment_factor = 0;
    Dwarf_Half return_address_register_rule = 0;
    Dwarf_Ptr initial_instructions = 0;
    Dwarf_Unsigned initial_instructions_length = 0;
    Dwarf_Off cie_off = 0;
    Dwarf_Error err = 0;

    cires = dwarf_get_cie_info(cie,
			       &cie_length,
			       &version,
			       &augmenter,
			       &code_alignment_factor,
			       &data_alignment_factor,
			       &return_address_register_rule,
			       &initial_instructions,
			       &initial_instructions_length, &err);
    if (cires == DW_DLV_ERROR) {
	print_error(dbg, "dwarf_get_cie_info", cires, err);
    }
    if (cires == DW_DLV_NO_ENTRY) {
	;			/* ? */
	printf("Impossible DW_DLV_NO_ENTRY on cie %d\n",
	       (int) cie_index);
	return DW_DLV_NO_ENTRY;
    }
    {
	printf("<%3lld>\tversion\t\t\t\t%d\n", cie_index, version);
	cires = _dwarf_cie_section_offset(dbg, cie, &cie_off, &err);
	if (cires == DW_DLV_OK) {
	    printf("\tcie sec. offset %llu 0x%llx\n",
		   (unsigned long long) cie_off,
		   (unsigned long long) cie_off);

	}

	printf("\taugmentation\t\t\t%s\n", augmenter);
	printf("\tcode_alignment_factor\t\t%llu\n",
	       (unsigned long long)code_alignment_factor);
	printf("\tdata_alignment_factor\t\t%lld\n",
	       (long long)data_alignment_factor);
	printf("\treturn_address_register\t\t%d\n",
	       (int)return_address_register_rule);
	{
	    int ares = 0;
	    Dwarf_Small *data = 0;
	    Dwarf_Unsigned len = 0;

	    ares =
		dwarf_get_cie_augmentation_data(cie, &data, &len, &err);
	    if (ares == DW_DLV_NO_ENTRY) {
		/* do nothing. */
	    } else if (ares == DW_DLV_OK && len > 0) {
		int k2;

		printf
		    ("\teh aug data len 0x%llx bytes 0x",
		     (long long) len);
		for (k2 = 0; data && k2 < len; ++k2) {
		    printf("%02x ", (unsigned char) data[k2]);
		}
		printf("\n");
	    }			/* else DW_DLV_ERROR or no data, do
				   nothing */
	}

	printf
	    ("\tbytes of initial instructions:\t%lld\n",
	     (long long) initial_instructions_length);
	printf("\tcie length :\t\t\t%lld\n", (long long) cie_length);
	print_frame_inst_bytes(dbg,
			       initial_instructions,
			       (Dwarf_Signed)initial_instructions_length,
			       data_alignment_factor,
			       (int)code_alignment_factor, 
				address_size,
		config_data);
    }
    return DW_DLV_OK;
}

/* Print the frame instructions in detail for a glob of instructions.
*/

 /*ARGSUSED*/ void
print_frame_inst_bytes(Dwarf_Debug dbg,
		       Dwarf_Ptr cie_init_inst, Dwarf_Signed len,
		       Dwarf_Signed data_alignment_factor,
		       int code_alignment_factor, Dwarf_Half addr_size,
			struct dwconf_s * config_data)
{
    unsigned char *instp = (unsigned char *) cie_init_inst;
    Dwarf_Unsigned uval;
    Dwarf_Unsigned uval2;
    unsigned int uleblen;
    unsigned int off = 0;
    unsigned int loff = 0;
    unsigned short u16;
    unsigned int u32;
    unsigned long long u64;

    for (; len > 0;) {
	unsigned char ibyte = *instp;
	int top = ibyte & 0xc0;
	int bottom = ibyte & 0x3f;
	int delta;
	int reg;

	switch (top) {
	case DW_CFA_advance_loc:
	    delta = ibyte & 0x3f;
	    printf("\t%2u DW_CFA_advance_loc %d", off,
		   (int) (delta * code_alignment_factor));
	    if (verbose) {
		printf("  (%d * %d)", (int) delta,
		       (int) code_alignment_factor);
	    }
	    printf("\n");
	    break;
	case DW_CFA_offset:
	    loff = off;
	    reg = ibyte & 0x3f;
	    uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
	    instp += uleblen;
	    len -= uleblen;
	    off += uleblen;
	    printf("\t%2u DW_CFA_offset ", loff);
	    printreg((Dwarf_Signed) reg,config_data);
	    printf(" %lld", (signed long long)
		   (((Dwarf_Signed) uval) * data_alignment_factor));
	    if (verbose) {
		printf("  (%llu * %d)", (unsigned long long) uval,
		       (int) data_alignment_factor);
	    }
	    printf("\n");
	    break;

	case DW_CFA_restore:
	    reg = ibyte & 0x3f;
	    printf("\t%2u DW_CFA_restore \n", off);
	    printreg((Dwarf_Signed) reg,config_data);
	    printf("\n");
	    break;

	default:
	    loff = off;
	    switch (bottom) {
	    case DW_CFA_set_loc:
		/* operand is address, so need address size */
		/* which will be 4 or 8. */
		switch (addr_size) {
		case 4:
		    {
			__uint32_t v32;

			memcpy(&v32, instp + 1, addr_size);
			uval = v32;
		    }
		    break;
		case 8:
		    {
			__uint64_t v64;

			memcpy(&v64, instp + 1, addr_size);
			uval = v64;
		    }
		    break;
		default:
		    printf
			("Error: Unexpected address size %d in DW_CFA_set_loc!\n",
			 addr_size);
		    uval = 0;
		}

		instp += addr_size;
		len -= (Dwarf_Signed) addr_size;
		off += addr_size;
		printf("\t%2u DW_CFA_set_loc %llu\n",
		       loff, (unsigned long long) uval);
		break;
	    case DW_CFA_advance_loc1:
		delta = (unsigned char) *(instp + 1);
		uval2 = delta;
		instp += 1;
		len -= 1;
		off += 1;
		printf("\t%2u DW_CFA_advance_loc1 %llu\n",
		       loff, (unsigned long long) uval2);
		break;
	    case DW_CFA_advance_loc2:
		memcpy(&u16, instp + 1, 2);
		uval2 = u16;
		instp += 2;
		len -= 2;
		off += 2;
		printf("\t%2u DW_CFA_advance_loc2 %llu\n",
		       loff, (unsigned long long) uval2);
		break;
	    case DW_CFA_advance_loc4:
		memcpy(&u32, instp + 1, 4);
		uval2 = u32;
		instp += 4;
		len -= 4;
		off += 4;
		printf("\t%2u DW_CFA_advance_loc4 %llu\n",
		       loff, (unsigned long long) uval2);
		break;
	    case DW_CFA_MIPS_advance_loc8:
		memcpy(&u64, instp + 1, 8);
		uval2 = u64;
		instp += 8;
		len -= 8;
		off += 8;
		printf("\t%2u DW_CFA_MIPS_advance_loc8 %llu\n",
		       loff, (unsigned long long) uval2);
		break;
	    case DW_CFA_offset_extended:
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		uval2 =
		    local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		printf("\t%2u DW_CFA_offset_extended ", loff);
		printreg((Dwarf_Signed) uval,config_data);
		printf(" %lld", (signed long long)
		       (((Dwarf_Signed) uval2) *
			data_alignment_factor));
		if (verbose) {
		    printf("  (%llu * %d)", (unsigned long long) uval2,
			   (int) data_alignment_factor);
		}
		printf("\n");
		break;

	    case DW_CFA_restore_extended:
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		printf("\t%2u DW_CFA_restore_extended ", loff);
		printreg((Dwarf_Signed) uval,config_data);
		printf("\n");
		break;
	    case DW_CFA_undefined:
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		printf("\t%2u DW_CFA_undefined ", loff);
		printreg((Dwarf_Signed) uval,config_data);
		printf("\n");
		break;
	    case DW_CFA_same_value:
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		printf("\t%2u DW_CFA_same_value ", loff);
		printreg((Dwarf_Signed) uval,config_data);
		printf("\n");
		break;
	    case DW_CFA_register:
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		uval2 =
		    local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		printf("\t%2u DW_CFA_register ", loff);
		printreg((Dwarf_Signed) uval,config_data);
		printf(" = ");
		printreg((Dwarf_Signed) uval2,config_data);
		printf("\n");
		break;
	    case DW_CFA_remember_state:
		printf("\t%2u DW_CFA_remember_state\n", loff);
		break;
	    case DW_CFA_restore_state:
		printf("\t%2u DW_CFA_restore_state\n", loff);
		break;
	    case DW_CFA_def_cfa:
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		uval2 =
		    local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		printf("\t%2u DW_CFA_def_cfa ", loff);
		printreg((Dwarf_Signed) uval,config_data);
		printf(" %llu", (unsigned long long) uval2);
		printf("\n");
		break;
	    case DW_CFA_def_cfa_register:
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		printf("\t%2u DW_CFA_def_cfa_register ", loff);
		printreg((Dwarf_Signed) uval,config_data);
		printf("\n");
		break;
	    case DW_CFA_def_cfa_offset:
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		printf("\t%2u DW_CFA_def_cfa_offset %llu\n",
		       loff, (unsigned long long) uval);
		break;

	    case DW_CFA_nop:
		printf("\t%2u DW_CFA_nop\n", loff);
		break;

	    case DW_CFA_def_cfa_expression:	/* DWARF3 */
		{
		    Dwarf_Unsigned block_len =
			local_dwarf_decode_u_leb128(instp + 1,
						    &uleblen);
		    instp += uleblen;
		    len -= uleblen;
		    off += uleblen;
		    printf
			("\t%2u DW_CFA_def_cfa_expression expr block len %lld\n",
			 loff, (unsigned long long)
			 block_len);
		    dump_block("\t\t", (char *) instp, (Dwarf_Signed)block_len);
		    instp += block_len;
		    len -= block_len;
		    off += block_len;
		}
		break;
	    case DW_CFA_expression:	/* DWARF3 */
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		{
		    Dwarf_Unsigned block_len =
			local_dwarf_decode_u_leb128(instp + 1,
						    &uleblen);
		    instp += uleblen;
		    len -= uleblen;
		    off += uleblen;
		    printf
			("\t%2u DW_CFA_expression %llu expr block len %lld\n",
			 loff, (unsigned long long) uval,
			 (unsigned long long)
			 block_len);
		    dump_block("\t\t", (char *) instp, (Dwarf_Signed)block_len);
		    instp += block_len;
		    len -= block_len;
		    off += block_len;
		}

		break;
	    case DW_CFA_cfa_offset_extended_sf:	/* DWARF3 */
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		{
		    Dwarf_Signed sval2 =
			local_dwarf_decode_s_leb128(instp + 1,
						    &uleblen);
		    instp += uleblen;
		    len -= uleblen;
		    off += uleblen;
		    printf("\t%2u DW_CFA_offset_extended_sf ", loff);
		    printreg((Dwarf_Signed) uval,config_data);
		    printf(" %lld", (signed long long)
			   ((sval2) * data_alignment_factor));
		    if (verbose) {
			printf("  (%lld * %d)", (long long) sval2,
			       (int) data_alignment_factor);
		    }
		}
		printf("\n");
		break;
	    case DW_CFA_def_cfa_sf:	/* DWARF3 */
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		{
		    Dwarf_Signed sval2 =
			local_dwarf_decode_s_leb128(instp + 1,
						    &uleblen);
		    instp += uleblen;
		    len -= uleblen;
		    off += uleblen;
		    printf("\t%2u DW_CFA_def_cfa_sf ", loff);
		    printreg((Dwarf_Signed)uval,config_data);
		    printf(" %lld", (long long) sval2);
		}
		printf("\n");
		break;
	    case DW_CFA_def_cfa_offset_sf:	/* DWARF3 */
		{
		    Dwarf_Signed sval =
			local_dwarf_decode_s_leb128(instp + 1,
						    &uleblen);

		    instp += uleblen;
		    len -= uleblen;
		    off += uleblen;
		    printf("\t%2u DW_CFA_def_cfa_offset_sf %lld\n",
			   loff, (long long) sval);

		}
		break;
	    case DW_CFA_val_offset:	/* DWARF3 */
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		{
		    uval2 =
			local_dwarf_decode_s_leb128(instp + 1,
						    &uleblen);
		    instp += uleblen;
		    len -= uleblen;
		    off += uleblen;
		    printf("\t%2u DW_CFA_val_offset ", loff);
		    printreg((Dwarf_Signed) uval,config_data);
		    printf(" %lld", (unsigned long long)
			   (((Dwarf_Signed) uval2) *
			    data_alignment_factor));
		    if (verbose) {
			printf("  (%lld * %d)", (long long) uval2,
			       (int) data_alignment_factor);
		    }
		}
		printf("\n");

		break;
	    case DW_CFA_val_offset_sf:	/* DWARF3 */
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		{
		    Dwarf_Signed sval2 =
			local_dwarf_decode_s_leb128(instp + 1,
						    &uleblen);
		    instp += uleblen;
		    len -= uleblen;
		    off += uleblen;
		    printf("\t%2u DW_CFA_val_offset_sf ", loff);
		    printreg((Dwarf_Signed) uval,config_data);
		    printf(" %lld", (signed long long)
			   ((sval2) * data_alignment_factor));
		    if (verbose) {
			printf("  (%lld * %d)", (long long) sval2,
			       (int) data_alignment_factor);
		    }
		}
		printf("\n");

		break;
	    case DW_CFA_val_expression:	/* DWARF3 */
		uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
		instp += uleblen;
		len -= uleblen;
		off += uleblen;
		{
		    Dwarf_Unsigned block_len =
			local_dwarf_decode_u_leb128(instp + 1,
						    &uleblen);
		    instp += uleblen;
		    len -= uleblen;
		    off += uleblen;
		    printf
			("\t%2u DW_CFA_val_expression %llu expr block len %lld\n",
			 loff, (unsigned long long) uval,
			 (unsigned long long)
			 block_len);
		    dump_block("\t\t", (char *) instp, (Dwarf_Signed)block_len);
		    instp += block_len;
		    len -= block_len;
		    off += block_len;
		}


		break;


#ifdef DW_CFA_GNU_window_save
	    case DW_CFA_GNU_window_save:{
		    /* no information: this just tells unwinder to
		       restore the window registers from the previous
		       frame's window save area */
		    printf("\t%2u DW_CFA_GNU_window_save \n", loff);
		    break;
		}
#endif
#ifdef DW_CFA_GNU_negative_offset_extended
	    case DW_CFA_GNU_negative_offset_extended:{
		    printf
			("\t%2u DW_CFA_GNU_negative_offset_extended \n",
			 loff);
		}
#endif
#ifdef  DW_CFA_GNU_args_size
		/* single uleb128 is the current arg area size in
		   bytes. no register exists yet to save this in */
	    case DW_CFA_GNU_args_size:{
		    Dwarf_Unsigned lreg;

		    lreg =
			local_dwarf_decode_u_leb128(instp + 1,
						    &uleblen);
		    printf
			("\t%2u DW_CFA_GNU_args_size arg size: %llu\n",
			 loff, (unsigned long long) lreg);
		    instp += uleblen;
		    len -= uleblen;
		    off += uleblen;

		    break;
		}
#endif

	    default:
		printf("\t%u Unexpected op 0x%x: \n",
		       loff, (unsigned int) bottom);
		len = 0;
		break;
	    }
	}
	instp++;
	len--;
	off++;
    }
}

/* Print our register names for the cases we have a name.
   Delegate to the configure code to actually do the print.
*/
void
printreg(Dwarf_Signed reg, struct dwconf_s *config_data)
{
    char *intfmt = "r%lld";
    print_reg_from_config_data(intfmt,reg,config_data);
}


/*
   Actually does the printing of a rule in the table.
   This may print something or may print nothing!
*/

static void
print_one_frame_reg_col( Dwarf_Unsigned rule_id,
			  Dwarf_Small value_type,
		Dwarf_Unsigned reg_used,
			  struct dwconf_s *config_data,
			 Dwarf_Signed offset_relevant,
			  Dwarf_Signed offset, 
		Dwarf_Ptr block_ptr)
{
    char * type_title = "";
    int print_type_title = 1;

    if (config_data->cf_interface_number == 2)
		print_type_title = 0;

	switch (value_type) {
	case DW_EXPR_OFFSET:
	    type_title = "off";
	    goto preg2;
	case DW_EXPR_VAL_OFFSET:
	    type_title = "valoff";
	  preg2:
	    if (reg_used == config_data->cf_initial_rule_value) {
		break;
	    }
	    if (print_type_title)
		printf("<%s ", type_title);
	    printreg((Dwarf_Signed) rule_id, config_data);
	    printf("=");
	    if (offset_relevant == 0) {
		printreg((Dwarf_Signed) reg_used, config_data);
		printf(" ");
	    } else {
		printf("%02lld", offset);
		printf("(");
		printreg((Dwarf_Signed) reg_used, config_data);
		printf(") ");
	    }
	    if (print_type_title)
		printf("%s","> ");
	    break;
	case DW_EXPR_EXPRESSION:
	    type_title = "expr";
	    goto pexp2;
	case DW_EXPR_VAL_EXPRESSION:
	    type_title = "valexpr";
	  pexp2:
	    if (print_type_title)
		printf("<%s ", type_title);
	    printreg((Dwarf_Signed) rule_id, config_data);
	    printf("=");
	    printf("expr-block-len=%lld", (long long) offset);
	    if (print_type_title)
		printf("%s","> ");
	    if (verbose) {
		char pref[40];

		strcpy(pref, "<");
	        strcat(pref, type_title);
	        strcat(pref, "bytes:");
		dump_block(pref, block_ptr, offset);
		printf("%s","> ");
	    }
	    break;
	default:
	    printf("Internal error in libdwarf, value type %d\n",
		   value_type);
		exit(1);
	}
    return;
}
