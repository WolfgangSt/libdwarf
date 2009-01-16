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


    dwarf_alloc.h
    $Revision: 1.18 $    $Date: 1999/03/05 21:59:59 $

*/

Dwarf_Ptr _dwarf_get_alloc (Dwarf_Debug, Dwarf_Small, Dwarf_Unsigned);
Dwarf_Debug _dwarf_get_debug (void);
Dwarf_Debug _dwarf_setup_debug (Dwarf_Debug);
int _dwarf_free_all_of_one_debug (Dwarf_Debug);

typedef struct Dwarf_Alloc_Area_s	*Dwarf_Alloc_Area;
typedef struct Dwarf_Free_List_s	*Dwarf_Free_List;

#define ALLOC_AREA_INDEX_TABLE_MAX 42
#define ALLOC_AREA_REAL_TABLE_MAX 31

/* 
    This struct is used to chain all the deallocated
    structs on the free list of each chain.  The structs
    are chained internally, by using the memory they
    contain.
*/
struct Dwarf_Free_List_s {
    Dwarf_Free_List		fl_next;
};


/*
    This struct is used to manage all the chunks malloc'ed
    for a particular alloc_type.  Many of the fields are
    initialized by dwarf_init().
*/
struct Dwarf_Alloc_Hdr_s {

	/* Count of actual number of structs user app 
		holds pointers to currently. */
    Dwarf_Sword			ah_struct_user_holds;

	/* 
	    Size of each struct that will be allocated for
	    this alloc_type.  Initialized by dwarf_init().
	*/
    Dwarf_Half			ah_bytes_one_struct;

	/* 
	    Number of structs of this alloc_type that will
	    be contained in each chunk that is malloc'ed.
	    Initialized by dwarf_init().
	*/
    Dwarf_Word			ah_structs_per_chunk;

	/* 
	    Number of bytes malloc'ed per chunk, that is
	    ah_bytes_one_struct * ah_alloc_num.
	*/
    Dwarf_Word			ah_bytes_malloc_per_chunk;

	/* Count of chunks currently allocated for type. */
    Dwarf_Sword			ah_chunks_allocated;

	/* 
	    Points to a chain of Dwarf_Alloc_Area_s structs
	    that represent all the chunks currently allocated
	    for the alloc_type.
	*/
    Dwarf_Alloc_Area		ah_alloc_area_head;

	/* Last Alloc Area that was allocated from. */
    Dwarf_Alloc_Area		ah_last_alloc_area;
};


/*
    This struct is used to manage each chunk that is
    malloc'ed for a particular alloc_type.  For each
    allocation type, the allocation header points to
    a list of all the chunks malloc'ed for that type.
*/
struct Dwarf_Alloc_Area_s {

	/* Points to the free list of structs in the chunk. */
    Dwarf_Ptr		aa_free_list;

	/* 
	    Count of the number of free structs in the chunk.
	    This includes both those on the free list, and in
	    the blob.
	*/
    Dwarf_Sword		aa_free_structs_in_chunk;

#ifdef notdef
	/* Count of the number of structs in this chunk.
	   Needed since we now allocate differing amounts
	   in different chunks.
	*/
    Dwarf_Word          aa_structs_in_this_chunk;
#endif

	/* 
	    Points to the first byte of the blob from which
	    struct will be allocated.  A struct is put on the
	    free_list only when it dwarf_deallocated.  Initial
	    allocations are from the blob.
	*/
    Dwarf_Small		*aa_blob_start;

	/* Points just past the last byte of the blob. */
    Dwarf_Small		*aa_blob_end;

	/* Points to alloc_hdr this alloc_area is linked to. */
    Dwarf_Alloc_Hdr	aa_alloc_hdr;

	/* 
	    Used for chaining Dwarf_Alloc_Area_s atructs. 
	    Alloc areas are doubly linked to enable deletion
	    from the list in constant time.
	*/
    Dwarf_Alloc_Area	aa_next;
    Dwarf_Alloc_Area	aa_prev;
};
