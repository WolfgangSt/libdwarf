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

        pro_reloc_symbolic.c
        $Revision: 1.3 $    $Date: 1999/09/30 02:38:34 $
        $Source: /isms/cmplrs.src/osprey1.0/libdwarf/RCS/pro_reloc_symbolic.c,v $

        This files contains routines for handling relocation
        information when we are creating symbolic relocs.
        That is, Elf-independent relocations are being 
	created here.
*/

#include "config.h"
#include "libdwarfdefs.h"
#include <stdio.h>
#include <string.h>
/*#include <elfaccess.h> */
#include "pro_incl.h"
#include "pro_section.h"
#include "pro_reloc.h" 
#include "pro_reloc_symbolic.h"

/*
	Return DW_DLV_ERROR on malloc error.
	Return DW_DLV_OK otherwise
*/

int
_dwarf_pro_reloc_name_symbolic(
        Dwarf_P_Debug dbg,
	int     base_sec_index,
        Dwarf_Unsigned offset, /* r_offset of reloc */
        Dwarf_Unsigned symidx,
	enum Dwarf_Rel_Type type, int reltarget_length)
{ 
	/* get a slot, fill in the slot entry */
        void *relrec_to_fill;
        int res;
	struct Dwarf_Relocation_Data_s *slotp;

        res = _dwarf_pro_reloc_get_a_slot(dbg,base_sec_index,
                        &relrec_to_fill);
        if(res != DW_DLV_OK)
                return res;

	slotp = (struct Dwarf_Relocation_Data_s *)relrec_to_fill;
	slotp->drd_type         = type;
	slotp->drd_length       = reltarget_length;
	slotp->drd_offset       = offset;
	slotp->drd_symbol_index = symidx;
	return DW_DLV_OK;

}



/*
	Return DW_DLV_ERROR on malloc error.
	Return DW_DLV_OK otherwise
*/
int
_dwarf_pro_reloc_length_symbolic(
        Dwarf_P_Debug dbg,
	int     base_sec_index,
        Dwarf_Unsigned offset, /* r_offset of reloc */
        Dwarf_Unsigned start_symidx,
	Dwarf_Unsigned end_symidx,
	enum Dwarf_Rel_Type type,
	int reltarget_length)
{ 
	/* get a slot, fill in the slot entry */
        void *relrec_to_fill;
        int res;
        struct Dwarf_Relocation_Data_s *slotp1;
        struct Dwarf_Relocation_Data_s *slotp2;



        res = _dwarf_pro_reloc_get_a_slot(dbg,base_sec_index,
                        &relrec_to_fill);
        if(res != DW_DLV_OK)
                return res;
	slotp1 = (struct Dwarf_Relocation_Data_s *)relrec_to_fill;
        res = _dwarf_pro_reloc_get_a_slot(dbg,base_sec_index,
                        &relrec_to_fill);
        if(res != DW_DLV_OK)
                return res;
	slotp2 = (struct Dwarf_Relocation_Data_s *)relrec_to_fill;

	/* ASSERT: type == dwarf_drt_first_of_length_type_pair */
	slotp1->drd_type         = type;
	slotp1->drd_length       = reltarget_length;
	slotp1->drd_offset       = offset;
	slotp1->drd_symbol_index = start_symidx;

	slotp2->drd_type         = dwarf_drt_second_of_length_pair;
	slotp2->drd_length       = reltarget_length;
	slotp2->drd_offset       = offset;
	slotp2->drd_symbol_index = end_symidx;

	return DW_DLV_OK;
}

/*
   Reset whatever fields of Dwarf_P_Per_Reloc_Sect_s
   we must to allow adding a fresh new single
   block easily (block consolidation use only).

*/
static void
_dwarf_reset_reloc_sect_info(struct Dwarf_P_Per_Reloc_Sect_s *pblk,
		unsigned long ct)
{


        /* do not zero pr_sect_num_of_reloc_sect */

        pblk->pr_reloc_total_count = 0;
        pblk->pr_first_block = 0;
        pblk->pr_last_block = 0;
        pblk->pr_block_count = 0;
	pblk->pr_slots_per_block_to_alloc = ct;
}
/*
        Ensure each stream is a single buffer and
        add that single buffer to the set of stream buffers.

	By creating a new buffer and copying if necessary.
	(If > 1 block, reduce to 1 block)

        Free the input set of buffers if we consolidate.

	We pass back *new_sec_count as zero because we
        are not creating normal sections for a .o, but
	symbolic relocations, separately counted.

        Return -1 on error (malloc failure)


        Return DW_DLV_OK on success. Any other return indicates 
	malloc failed.
*/
int
_dwarf_symbolic_relocs_to_disk(
        Dwarf_P_Debug dbg,
        Dwarf_Signed *new_sec_count)
{
	/*unsigned long total_size =0; */
	Dwarf_Small *data;
        int   sec_index;
	int   res;
	unsigned long i;
	Dwarf_Error error;

	Dwarf_Signed sec_count = 0;

	Dwarf_P_Per_Reloc_Sect p_reloc = &dbg->de_reloc_sect[0];

	for(i = 0; i < NUM_DEBUG_SECTIONS; ++i, ++p_reloc) {

	    unsigned long ct = p_reloc->pr_reloc_total_count;
	    struct Dwarf_P_Relocation_Block_s *p_blk;
	    struct Dwarf_P_Relocation_Block_s *p_blk_last;
	    /* int len */
	    int err;

	    
	    if(ct == 0) {
		continue;
	    }

	    /*len = dbg->de_relocation_record_size; */
	    ++sec_count;

	    /*total_size = ct *len; */
	    sec_index = p_reloc->pr_sect_num_of_reloc_sect;
	    if(sec_index == 0) {
		/* call de_func or de_func_b, getting
			   section number of reloc sec */
		int rel_section_index;
		int int_name;
	        Dwarf_Unsigned name_idx;

		/*
		This is a bit of a fake, as we do not
		really have true elf sections at all.
		Just the data such might contain.
		But this lets the caller eventually
		link things together: without this
		call we would not know what
		rel data goes with what section
		when we are asked for the
		real arrays.
		*/

		if(dbg->de_func_b) {
 	 	  rel_section_index = dbg->de_func_b(
                	_dwarf_rel_section_names[i],
        	 	dbg->de_relocation_record_size,
			/*type*/SHT_REL, 
			/*flags*/0, 
			/*link to symtab, which
			  we cannot know*/SHN_UNDEF,
                	/*sec rels apply to */
				dbg->de_elf_sects[i],
        		&name_idx, &err);
		} else {
 	 	  rel_section_index = dbg->de_func(
                	_dwarf_rel_section_names[i],
        	 	dbg->de_relocation_record_size,
			/* type */SHT_REL, 
			/*flags*/0, 
			/* link to symtab, which
			  we cannot know */ SHN_UNDEF,
			/* sec rels apply to, in elf, sh_info */
                	    dbg->de_elf_sects[i],
        		&int_name, &err);
			name_idx = int_name;
		}
    		if (rel_section_index == -1) {
		   {_dwarf_p_error(dbg, &error, DW_DLE_ELF_SECT_ERR); 
				return(DW_DLV_ERROR);}
    		}
    		p_reloc->pr_sect_num_of_reloc_sect = rel_section_index;
		sec_index = rel_section_index;
	    }

	    p_blk = p_reloc->pr_first_block;
	    
	    if(p_reloc->pr_block_count > 1) {
	       struct Dwarf_P_Relocation_Block_s *new_blk;

		/* HACK , not normal interfaces */
	       _dwarf_reset_reloc_sect_info(p_reloc,ct);
	       res = _dwarf_pro_pre_alloc_n_reloc_slots( dbg,
			(int)i,
			ct);
		/* creating new block, 
		ASSERT p_reloc->pr_block_count == 1 */
	       if(res != DW_DLV_OK) {
			return res;
	       }
		/* ASSERT: 1 and only on block, and big enough for
		all data */
	       new_blk = p_reloc->pr_first_block; 
	    
	       data = (Dwarf_Small *)new_blk->rb_data;

	       /* following loop executes at least once.
		   Effects the consolidation to a single
		   block
		   And frees the input block.
	       */
	       while(  p_blk ){

		   unsigned long len =
			p_blk->rb_where_to_add_next -
			p_blk->rb_data;

		   memcpy(data, p_blk->rb_data,len);
	  	   data +=  len;

		   p_blk_last = p_blk;
		   p_blk      = p_blk->rb_next;
   
		   _dwarf_p_dealloc(dbg,(Dwarf_Small *)p_blk_last);
	       }
	       /* ASSERT: sum of len copied == total_size */
	       new_blk->rb_next_slot_to_use = ct;
	       new_blk->rb_where_to_add_next = 
			(char *)data;
	       p_reloc->pr_reloc_total_count = ct;
       
	       /* have now created a single block, but no
		  change in slots used (pr_reloc_total_count) 
	       */
	  }
	}

	*new_sec_count = 0;
	return DW_DLV_OK;
}

