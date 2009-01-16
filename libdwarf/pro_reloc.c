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

        pro_reloc.c
        $Revision: 1.2 $    $Date: 1999/06/22 16:33:40 $
        $Source: /isms/cmplrs.src/osprey1.0/libdwarf/RCS/pro_reloc.c,v $

        This files contains routines for handling relocation
        information.
	(for transforming to disk form or to an output array)
*/

#include "config.h"
#include "libdwarfdefs.h"
#include <stdio.h>
#include <string.h>
/*#include <elfaccess.h> */
#include "pro_incl.h"


/*Do initial alloc of newslots slots.
  Fails only if malloc fails.

  Supposed to be called before any relocs allocated.
  Ignored if after any allocated.

  Purely an optimization. Used in few places. Never necessary
  to call this.

  returns DW_DLV_OK or  DW_DLV_ERROR
*/
int
_dwarf_pro_pre_alloc_n_reloc_slots(
	Dwarf_P_Debug  dbg,
	int            rel_sec_index,
	Dwarf_Unsigned newslots)
{
	unsigned long len;
	struct Dwarf_P_Relocation_Block_s * data;
	Dwarf_P_Per_Reloc_Sect  prel =
		&dbg->de_reloc_sect[rel_sec_index];
	unsigned long slots_in_blk = (unsigned long)newslots;
	unsigned long rel_rec_size = dbg->de_relocation_record_size;

	if(prel->pr_first_block)
		return DW_DLV_OK; /* do nothing */

	len = sizeof (struct Dwarf_P_Relocation_Block_s) +
		slots_in_blk *rel_rec_size;
		
	
	data = (struct Dwarf_P_Relocation_Block_s *)
		_dwarf_p_get_alloc(dbg, len);
	if(!data) {
		return DW_DLV_ERROR;
	}
	data->rb_slots_in_block = slots_in_blk; /* could use
		default here, as fallback in case our
	        origininal estimate wrong. 
		When we call this we presumably know what we
		are doing, so keep this count for now */
	data->rb_next_slot_to_use = 0;
	data->rb_where_to_add_next =
		((char *)data) + sizeof(struct Dwarf_P_Relocation_Block_s);
	data->rb_data = data->rb_where_to_add_next;
	
	prel->pr_first_block = data;
	prel->pr_last_block = data;

	
	return DW_DLV_OK;
}


/*Do alloc of slots.
  Fails only if malloc fails.

  Only allocator used.

  returns DW_DLV_OK or  DW_DLV_ERROR
*/
int
_dwarf_pro_alloc_reloc_slots(
	Dwarf_P_Debug  dbg,
	int            rel_sec_index)
{
	unsigned long len;
	struct Dwarf_P_Relocation_Block_s * data;
	Dwarf_P_Per_Reloc_Sect  prel =
		&dbg->de_reloc_sect[rel_sec_index];
	unsigned long slots_in_blk = prel->pr_slots_per_block_to_alloc;
	unsigned long rel_rec_size = dbg->de_relocation_record_size;

	len = sizeof (struct Dwarf_P_Relocation_Block_s) +
		slots_in_blk *rel_rec_size;
	
	data = (struct Dwarf_P_Relocation_Block_s *)
		_dwarf_p_get_alloc(dbg, len);
	if(!data) {
		return DW_DLV_ERROR;
	}

	if(prel->pr_first_block) {
		prel->pr_last_block->rb_next = data;
		prel->pr_last_block = data;
		prel->pr_block_count += 1;
		
	} else {
		
		prel->pr_first_block = data;
		prel->pr_last_block = data;
		prel->pr_block_count = 1;
	}

	data->rb_slots_in_block = slots_in_blk;
	data->rb_next_slot_to_use = 0;
	data->rb_where_to_add_next =
		((char *)data) + sizeof(struct Dwarf_P_Relocation_Block_s);
	data->rb_data = data->rb_where_to_add_next;
	
	return DW_DLV_OK;
	
}

/*
	Reserve a slot. return DW_DLV_OK if succeeds.

	Return DW_DLV_ERROR if fails (malloc error).

	Use the relrec_to_fill to pass back a pointer to
	a slot space to use.
*/
int _dwarf_pro_reloc_get_a_slot(Dwarf_P_Debug dbg,
        int base_sec_index,
        void** relrec_to_fill)
{
        struct Dwarf_P_Relocation_Block_s * data;
        Dwarf_P_Per_Reloc_Sect  prel =
                &dbg->de_reloc_sect[base_sec_index];
        unsigned long rel_rec_size = dbg->de_relocation_record_size;

        char *ret_addr;

	data = prel->pr_last_block;
	if( (data == 0)  ||
	    (data->rb_next_slot_to_use >= 
			data->rb_slots_in_block)) {
	  int res;
	  res = _dwarf_pro_alloc_reloc_slots(dbg,base_sec_index);
          if(res != DW_DLV_OK) {
		return res;	
	  }
	}
	
	data = prel->pr_last_block;
	/* now we have an empty slot*/
	ret_addr = data->rb_where_to_add_next;

	data->rb_where_to_add_next += rel_rec_size;
	data->rb_next_slot_to_use += 1;

	prel->pr_reloc_total_count += 1;
	
	*relrec_to_fill = (void *)ret_addr;

	return DW_DLV_OK;

}

/*
   On success  returns count of
   .rel.* sections that are symbolic 
   thru count_of_relocation_sections.

   On success, returns DW_DLV_OK.

   If this is not a 'symbolic' run, returns
    DW_DLV_NO_ENTRY.

   No errors are possible.




*/

/*ARGSUSED*/
int  
dwarf_get_relocation_info_count(
        Dwarf_P_Debug    dbg,
        Dwarf_Unsigned * count_of_relocation_sections,
	int  *           drd_buffer_version,
        Dwarf_Error*     error)
{
	if(dbg->de_flags & DW_DLC_SYMBOLIC_RELOCATIONS) {
	   int i;
	   unsigned int count = 0;
	   for(i = 0; i < NUM_DEBUG_SECTIONS ; ++i) {
		if(dbg->de_reloc_sect[i].pr_reloc_total_count > 0) {
			++count;
		}
	   }
	  *count_of_relocation_sections = (Dwarf_Unsigned)count;
	  *drd_buffer_version      = DWARF_DRD_BUFFER_VERSION;
	  return DW_DLV_OK;
	}
	return DW_DLV_NO_ENTRY;
}

int dwarf_get_relocation_info(
        Dwarf_P_Debug           dbg,
        Dwarf_Signed          * elf_section_index,
        Dwarf_Signed          * elf_section_index_link,
        Dwarf_Unsigned        * relocation_buffer_count,
        Dwarf_Relocation_Data * reldata_buffer,
        Dwarf_Error*            error)
{
	int next = dbg->de_reloc_next_to_return;
        if(dbg->de_flags & DW_DLC_SYMBOLIC_RELOCATIONS) {
           int i;
           for(i = next; i < NUM_DEBUG_SECTIONS ; ++i) {
	        Dwarf_P_Per_Reloc_Sect  prel =
		     &dbg->de_reloc_sect[i];
                if(prel->pr_reloc_total_count > 0) {
		   dbg->de_reloc_next_to_return = i+1;


		   /* ASSERT: prel->.pr_block_count == 1 */

		   *elf_section_index       = 
			prel->pr_sect_num_of_reloc_sect;
		   *elf_section_index_link  = 
			dbg->de_elf_sects[i];
		   *relocation_buffer_count =  
			prel->pr_reloc_total_count;
		   *reldata_buffer          =  
			   (Dwarf_Relocation_Data)
			   (prel->pr_first_block->rb_data);
		   return DW_DLV_OK;
                }
           }
	  DWARF_P_DBG_ERROR(dbg, DW_DLE_REL_ALLOC, DW_DLV_ERROR);
        }
        return DW_DLV_NO_ENTRY;
}
