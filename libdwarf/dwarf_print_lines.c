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

	dwarf_print_lines.c
	$Revision: 1.6 $  $Date: 1999/07/21 21:29:44 $

	Used by dwarfdump to print line internals.
	Hard to see how to get this right at the dwarfdump
	level, so doing it internal to libdwarf.


*/

#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include <time.h>
#include "dwarf_line.h"

static void print_line_header(void)
{
printf(
"                                                         s b e\n"
"                                                         t l s\n"
"                                                         m c e\n"
" section    op                                       col t k q\n"
" offset     code               address     file line umn ? ? ?\n"
);
}

static void print_line_detail(char *prefix,
	int opcode,
	unsigned long long address,
	unsigned long file,
	unsigned long line,
	unsigned long column,
	int is_stmt,
	int basic_block,
	int end_sequence)
{
               printf(
                  "%-15s %2d 0x%08llx "
                  "%2lu   %4lu %2lu   %1d %1d %1d\n",
		  prefix,
                  (int)opcode,
                  (long long)address,
                  (int)file,
                  (int)line,
                  (int)column,
                  (int)is_stmt,
                  (int)basic_block,
                  (int)end_sequence);

}


/*
	return DW_DLV_OK if ok. else DW_DLV_NO_ENTRY or DW_DLV_ERROR
*/
int
_dwarf_internal_printlines (
    Dwarf_Die       die,
    Dwarf_Error     *error
)
{
        /* 
            This pointer is used to scan the portion of  
            the .debug_line section for the current cu.  
        */
    Dwarf_Small     *line_ptr;      
    Dwarf_Small     *orig_line_ptr;      

        /* 
            This points to the last byte of the         
            .debug_line portion for the current cu. 
        */
    Dwarf_Small     *line_ptr_end;  

        /* 
            This points to the end of the statement program prologue
            for the current cu, and serves to check that the prologue   
            was correctly decoded.                                      
        */
    Dwarf_Small     *check_line_ptr;

	/* 
	    Pointer to a DW_AT_stmt_list attribute 
	    in case it exists in the die.
	*/
    Dwarf_Attribute	stmt_list_attr;

	/* Pointer to DW_AT_comp_dir attribute in die. */
    Dwarf_Attribute	comp_dir_attr;

	/* Pointer to name of compilation directory. */
    Dwarf_Small		*comp_dir = NULL;

	/* 
	    Offset into .debug_line specified by a 
	    DW_AT_stmt_list attribute.
	*/
    Dwarf_Unsigned	line_offset;

        /* These are the fields of the statement program header. */
    Dwarf_Unsigned          total_length;
    Dwarf_Half              version;
    Dwarf_Unsigned          prologue_length;
    Dwarf_Small             minimum_instruction_length;
    Dwarf_Small             default_is_stmt;
    Dwarf_Sbyte             line_base;
    Dwarf_Small             line_range;
    Dwarf_Small             opcode_base;

        /* 
            The full UCHAR_MAX number of standard opcode
            lengths is used for the opcode_length table because
            that is the only totally safe limit for static 
            allocation to avoid malloc-ing the exact size needed.
        */
    Dwarf_Small             opcode_length[UCHAR_MAX];
    
        /* These are the state machine state variables. */
    Dwarf_Addr              address;
    Dwarf_Word              file;
    Dwarf_Word              line;
    Dwarf_Word              column;
    Dwarf_Bool              is_stmt;
    Dwarf_Bool              basic_block;
    Dwarf_Bool              end_sequence;

    Dwarf_Sword	            	i, file_entry_count, include_directories_count;

        /* 
            This is the current opcode read     
            from the statement program.     
        */
    Dwarf_Small         	opcode;

        /* 
	    Pointer to a Dwarf_Line_Context_s structure that
	    contains the context such as file names and include
	    directories for the set of lines being generated.
        */
    Dwarf_Line_Context		line_context;


        /* 
            These variables are used to decode leb128 numbers.      
            Leb128_num holds the decoded number, and leb128_length  
            is its length in bytes.                                 
        */
    Dwarf_Word	            leb128_num;
    Dwarf_Word              leb128_length;
    Dwarf_Sword		    advance_line;

        /* 
            This is the operand of the latest fixed_advance_pc  
            extended opcode.                                        
        */
    Dwarf_Half              fixed_advance_pc;

        /* This is the length of an extended opcode instr.  */
    Dwarf_Word          instr_length;
    Dwarf_Small         ext_opcode;
    int                 local_length_size;
    int                 local_extension_size;


	/* The Dwarf_Debug this die belongs to. */
    Dwarf_Debug		dbg;
    int resattr;
    int lres;

    /* ***** BEGIN CODE ***** */

    if (error != NULL) *error = NULL;

    CHECK_DIE(die, DW_DLV_ERROR)
    dbg = die->di_cu_context->cc_dbg;
    if (dbg->de_debug_line == NULL) {
        return(DW_DLV_NO_ENTRY);
    }

    resattr = dwarf_attr(die, DW_AT_stmt_list,&stmt_list_attr,error);
    if(resattr != DW_DLV_OK) {
	return resattr;
    }

    

    lres = dwarf_formudata(stmt_list_attr,&line_offset, error);
    if(lres != DW_DLV_OK) {
	return lres;
    }
	
    if (line_offset >= dbg->de_debug_line_size) {
        _dwarf_error(dbg, error, DW_DLE_LINE_OFFSET_BAD);
        return(DW_DLV_ERROR);
    }
    orig_line_ptr =  dbg->de_debug_line;
    line_ptr = dbg->de_debug_line + line_offset;
    dwarf_dealloc(dbg, stmt_list_attr, DW_DLA_ATTR);

	/* 
	    If die has DW_AT_comp_dir attribute, get the
	    string that names the compilation directory.
	*/
    resattr = dwarf_attr(die, DW_AT_comp_dir,&comp_dir_attr, error);
    if(resattr == DW_DLV_ERROR) {
        return resattr;
    }
    if (resattr == DW_DLV_OK) {
	int cres;
	char *cdir;
	cres = dwarf_formstring(comp_dir_attr,&cdir, error);
	if(cres == DW_DLV_ERROR){
		return cres;
	}else if(cres == DW_DLV_OK) {
		comp_dir = (Dwarf_Small *)cdir;
	}
    }
    if(resattr == DW_DLV_OK) {
            dwarf_dealloc(dbg, comp_dir_attr, DW_DLA_ATTR);
    }

        /* 
            Following is a straightforward decoding of the  
            statement program prologue information.         
        */

    /* READ_AREA_LENGTH updates line_ptr for consumed bytes*/
    READ_AREA_LENGTH(dbg,total_length,Dwarf_Unsigned,
            line_ptr,local_length_size,local_extension_size);



    line_ptr_end = line_ptr + total_length;
    if (line_ptr_end > dbg->de_debug_line + 
        dbg->de_debug_line_size) {
        _dwarf_error(dbg,error,DW_DLE_DEBUG_LINE_LENGTH_BAD); 
        return(DW_DLV_ERROR);
    }

    printf("total line info length %ld bytes, "
		"line offset 0x%llx %lld\n",
		(long)total_length,
		(long long)line_offset,
		(long long)line_offset);
    printf("compilation_directory %s\n",
		comp_dir?((char *)comp_dir):"");
    READ_UNALIGNED(dbg,version, Dwarf_Half,
		line_ptr, sizeof(Dwarf_Half));
    line_ptr += sizeof(Dwarf_Half);
    if (version != CURRENT_VERSION_STAMP) {
        _dwarf_error(dbg,error,DW_DLE_VERSION_STAMP_ERROR);
        return(DW_DLV_ERROR);
    }

    READ_UNALIGNED(dbg,prologue_length, Dwarf_Unsigned,
		line_ptr, local_length_size);
    line_ptr += local_length_size;
    check_line_ptr = line_ptr;

    minimum_instruction_length = *(Dwarf_Small *)line_ptr;
    line_ptr = line_ptr + sizeof(Dwarf_Small);

    default_is_stmt = *(Dwarf_Small *)line_ptr;
    line_ptr = line_ptr + sizeof(Dwarf_Small);

    line_base = *(Dwarf_Sbyte *)line_ptr;
    line_ptr = line_ptr + sizeof(Dwarf_Sbyte);

    line_range = *(Dwarf_Small *)line_ptr;
    line_ptr = line_ptr + sizeof(Dwarf_Small);

    opcode_base = *(Dwarf_Small *)line_ptr;
    line_ptr = line_ptr + sizeof(Dwarf_Small);
    printf("  min instruction length %d\n",(int)minimum_instruction_length); 
    printf("  default is stmt        %d\n",(int)default_is_stmt); 
    printf("  line base              %d\n",(int)line_base); 
    printf("  line_range             %d\n",(int)line_range); 

    for (i = 1; i < opcode_base; i++) {
        opcode_length[i] = *(Dwarf_Small *)line_ptr;
	printf("  opcode[%d] length %d\n",(int)i,(int)opcode_length[i]);
        line_ptr = line_ptr + sizeof(Dwarf_Small);
    }

    include_directories_count = 0;
    while ((*(char *)line_ptr) != '\0') {
	printf("  include dir[%d] %s\n",
		(int)include_directories_count,
		line_ptr);
        line_ptr = line_ptr + strlen((char *)line_ptr) + 1;
        include_directories_count++;
    }
    line_ptr++;

    file_entry_count = 0;
    while (*(char *)line_ptr != '\0') {

	Dwarf_Unsigned tlm2;
	Dwarf_Unsigned di;
	Dwarf_Unsigned fl;

	printf("  file[%d]  %s\n",file_entry_count,
		(char *)line_ptr);

        line_ptr = line_ptr + strlen((char *)line_ptr) + 1;

        di = 
            _dwarf_decode_u_leb128(line_ptr,&leb128_length);
        line_ptr = line_ptr + leb128_length;

        tlm2 = 
            _dwarf_decode_u_leb128(line_ptr,&leb128_length);
        line_ptr = line_ptr + leb128_length;

        fl = 
            _dwarf_decode_u_leb128(line_ptr,&leb128_length);
        line_ptr = line_ptr + leb128_length;

	printf("    dir index %d\n",
			(int)di);
	{
	time_t tt = (time_t)tlm2;
	printf("    last time 0x%x %s", /* ctime supplies newline */
		(unsigned)tlm2,ctime(&tt));
	}
	printf("    file length %ld 0x%lx\n",
			(long)fl,(unsigned long)fl);


        file_entry_count++;
    }
    line_ptr++;

    if (line_ptr != check_line_ptr + prologue_length) {
        _dwarf_error(dbg,error,DW_DLE_LINE_PROLOG_LENGTH_BAD);
        return(DW_DLV_ERROR);
    }

        /* Set up context structure for this set of lines. */
    line_context = (Dwarf_Line_Context)
	_dwarf_get_alloc(dbg, DW_DLA_LINE_CONTEXT, 1);
    if (line_context == NULL) {
	_dwarf_error(dbg,error,DW_DLE_ALLOC_FAIL);
	return(DW_DLV_ERROR);
    }

    printf("  statement prog offset in section: %lld 0x%llx\n",
		(long long)(line_ptr   - orig_line_ptr),
		(long long)(line_ptr   - orig_line_ptr));
		
        /* Initialize the state machine.    */
    address = 0;
    file = 1;
    line = 1;
    column = 0;
    is_stmt = default_is_stmt;
    basic_block = false;
    end_sequence = false;

    print_line_header();
        /* Start of statement program.  */
    while (line_ptr < line_ptr_end) {
        
	printf(" [0x%06llx] ",
		(long long)(line_ptr - orig_line_ptr));
        opcode = *(Dwarf_Small *)line_ptr;
        line_ptr++;
        switch (opcode) {
            
                /* 
                    These are special opcodes between opcode_base
                    and UCHAR_MAX.                            
                */
            default : {
		char special[50];
		unsigned origop = opcode;

                opcode = opcode - opcode_base;
                address = address + minimum_instruction_length * 
		    (opcode / line_range);
                line = line + line_base + opcode % line_range;

		sprintf(special,"Specialop %3u",origop);
		print_line_detail(special,
			opcode,address,(int)file,line,column,
			is_stmt,basic_block,end_sequence);

                basic_block = false;
                break;
            }

            case DW_LNS_copy : {
                if (opcode_length[DW_LNS_copy] != 0) {
                    _dwarf_error(dbg,error,DW_DLE_LINE_NUM_OPERANDS_BAD);
                    return(DW_DLV_ERROR);
                }

		print_line_detail("DW_LNS_copy",
			opcode,address,file,line,column,
			is_stmt,basic_block,end_sequence);

                basic_block = false;
                break;
            }

            case DW_LNS_advance_pc : {
		Dwarf_Unsigned utmp2;
                if (opcode_length[DW_LNS_advance_pc] != 1) {
                    _dwarf_error(dbg,error,DW_DLE_LINE_NUM_OPERANDS_BAD);
                    return(DW_DLV_ERROR);
                }

		DECODE_LEB128_UWORD(line_ptr, utmp2)
		printf("DW_LNS_advance_pc val %lld 0x%llx\n",
			(long long)(Dwarf_Word)utmp2,
			(long long)(Dwarf_Word)utmp2);
		leb128_num = (Dwarf_Word)utmp2;
                address = address + minimum_instruction_length * leb128_num;
                break;
            }

            case DW_LNS_advance_line : {
		Dwarf_Signed stmp;
                if (opcode_length[DW_LNS_advance_line] != 1) {
                    _dwarf_error(dbg,error,DW_DLE_LINE_NUM_OPERANDS_BAD);
                    return(DW_DLV_ERROR);
                }

		DECODE_LEB128_SWORD(line_ptr, stmp)
		advance_line = (Dwarf_Sword)stmp;
		printf("DW_LNS_advance_line val %lld 0x%llx\n",
			(long long)advance_line,
			(long long)advance_line);
                line = line + advance_line;
                break;
            }

            case DW_LNS_set_file : {
		Dwarf_Unsigned utmp2;
                if (opcode_length[DW_LNS_set_file] != 1) {
                    _dwarf_error(dbg,error,DW_DLE_LINE_NUM_OPERANDS_BAD);
                    return(DW_DLV_ERROR);
                }

		DECODE_LEB128_UWORD(line_ptr, utmp2)
		file = (Dwarf_Word)utmp2;
		printf("DW_LNS_set_file  %dl\n",file);
                break;
            }

            case DW_LNS_set_column : {
		Dwarf_Unsigned utmp2;
                if (opcode_length[DW_LNS_set_column] != 1) {
                    _dwarf_error(dbg,error,DW_DLE_LINE_NUM_OPERANDS_BAD);
                    return(DW_DLV_ERROR);
                }

		DECODE_LEB128_UWORD(line_ptr, utmp2)
		column = (Dwarf_Word)utmp2;
		printf("DW_LNS_set_column val %lld 0x%llx\n",
			(long long)column,
			(long long)column);
                break;
            }

            case DW_LNS_negate_stmt : {
                if (opcode_length[DW_LNS_negate_stmt] != 0) {
                    _dwarf_error(dbg,error,DW_DLE_LINE_NUM_OPERANDS_BAD);
                    return(DW_DLV_ERROR);
                }

                is_stmt = !is_stmt;
		printf("DW_LNS_negate_stmt\n");
                break;
            }

            case DW_LNS_set_basic_block : {
                if (opcode_length[DW_LNS_set_basic_block] != 0) {
                    _dwarf_error(dbg,error,DW_DLE_LINE_NUM_OPERANDS_BAD);
                    return(DW_DLV_ERROR);
                }

		printf("DW_LNS_set_basic_block\n");
                basic_block = true;
                break;
            }

            case DW_LNS_const_add_pc : {
                opcode = UCHAR_MAX - opcode_base;
                address = address + minimum_instruction_length *
		    (opcode / line_range);

		printf("DW_LNS_const_add_pc new address 0x%llx\n",
			(long long)address);
                break;
            }

            case DW_LNS_fixed_advance_pc : {
                if (opcode_length[DW_LNS_fixed_advance_pc] != 1) {
                    _dwarf_error(dbg,error,DW_DLE_LINE_NUM_OPERANDS_BAD);
                    return(DW_DLV_ERROR);
                }

		READ_UNALIGNED(dbg,fixed_advance_pc, Dwarf_Half,
		   	line_ptr, sizeof(Dwarf_Half));
                line_ptr += sizeof(Dwarf_Half);
                address = address + fixed_advance_pc;
		printf("DW_LNS_fixed_advance_pc val %lld 0x%llx"
			" new address 0x%llx\n",
			(long long)fixed_advance_pc,
			(long long)fixed_advance_pc,
			(long long)address);
                break;
            }

            case DW_EXTENDED_OPCODE : {
		Dwarf_Unsigned utmp3;
		DECODE_LEB128_UWORD(line_ptr, utmp3)
		instr_length = (Dwarf_Word)utmp3;
                ext_opcode = *(Dwarf_Small *)line_ptr;
                line_ptr++;
                switch (ext_opcode) {
                    
                    case DW_LNE_end_sequence : {
                        end_sequence = true;

		print_line_detail("DW_LNE_end_sequence extended",
			opcode,address,file,line,column,
			is_stmt,basic_block,end_sequence);

                        address = 0;
                        file = 1;
                        line = 1;
                        column = 0;
                        is_stmt = default_is_stmt;
                        basic_block = false;
                        end_sequence = false;

                        break;
                    }

                    case DW_LNE_set_address : {
                        if (instr_length - 1 == dbg->de_pointer_size) {
			    READ_UNALIGNED(dbg,address, Dwarf_Addr,
				line_ptr,
				dbg->de_pointer_size);

                            line_ptr += dbg->de_pointer_size;
			    printf("DW_LNE_set_address address 0x%llx\n",
					(long long)address);
                        }
                        else {
                            _dwarf_error(dbg,error,
                                DW_DLE_LINE_SET_ADDR_ERROR);
                            return(DW_DLV_ERROR);
                        }

                        break;
                    }

                    case DW_LNE_define_file : {


			Dwarf_Small *fn;
		        Dwarf_Signed di;
		        Dwarf_Signed tlm;
			Dwarf_Unsigned fl;
                        fn = (Dwarf_Small *)line_ptr;
                        line_ptr = line_ptr + strlen((char *)line_ptr) + 1;

                        di = 
                            _dwarf_decode_u_leb128(line_ptr,&leb128_length);
                        line_ptr = line_ptr + leb128_length;

                        tlm = 
                            _dwarf_decode_u_leb128(line_ptr,&leb128_length);
                        line_ptr = line_ptr + leb128_length;

                        fl = 
                            _dwarf_decode_u_leb128(line_ptr,&leb128_length);
                        line_ptr = line_ptr + leb128_length;


		        printf("DW_LNE_define_file %s \n",
				fn);
		        printf("    dir index %d\n",
                             (int)di);
			{
			time_t tt3 = (time_t)tlm;
				 /* ctime supplies newline */
        		printf("    last time 0x%x %s",
                             (unsigned)tlm,ctime(&tt3));
			}
                        printf("    file length %ld 0x%lx\n",
                            (long)fl,(unsigned long)fl);

                        break;
                    }
                                
                    default : {
                        _dwarf_error(dbg,error,DW_DLE_LINE_EXT_OPCODE_BAD);
                        return(DW_DLV_ERROR);
                    }
                }

            }
        }
    }

    return(DW_DLV_OK);
}

/*
	Caller passes in compilation unit DIE.
*/
int
_dwarf_print_lines(
    Dwarf_Die       die,
    Dwarf_Error     *error
)
{
	int res;
	res = _dwarf_internal_printlines(die, error);
	if(res != DW_DLV_OK) {
		return res;
	}
	return res;
}

