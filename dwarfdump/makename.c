/* Copyright (c) 1982 Regents of the University of California */

/* 
   makename.c
   $Revision: 1.2 $ 
   $Date: 1999/03/05 21:59:52 $

   Name are the internal representation for identifiers.
  
   A hash table is used to map identifiers to names.

   Names are allocated in large chunks to avoid calls to malloc
   and to cluster names in memory so that tracing hash chains
   doesn't cause many a page fault.

   Strings space is allocated in large chunks too.

   The latest string added is on the front of the synonym list.

   The Name structure itself is not exported.
*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "makename.h"

struct Name {
	char        *n_identifier;
	struct Name *n_chain;
};

#define HASHTABLESIZE 16384
static struct Name * Nametable[HASHTABLESIZE];

#define POOLSIZE 8192

/* a bit under POOLSIZE bytes in a namepool */
#define CHUNKSIZE  ((POOLSIZE/sizeof(struct Name))- 1) 

struct Namepool {
	struct Name name[CHUNKSIZE];
	struct Namepool *prevpool;
};

static struct Namepool *Namepooltab;
static long Nleft = 0;
static char * new_string_space(unsigned len);


/* The address of a string region
*/
static char *curbuf_base;

/* A pointer to the next string space a string should use
*/
static char *curbuf_cur;
/* The amount of the original buffer still left.
*/
static unsigned buf_chars_left;

/* The amount of space per malloc for strings
*/
#define STRING_BUF_SIZE          (64*1024)

/* The length of a string which is the most we will put in
** the string tables allocated above: stuff this large
** goes in its own malloc space, not a string table.
** This controls the amount of wasted space  in a buffer.
** The assumption is that strings this large are rarely needed.
** In fact, we assume most strings are very small.
*/
#define SEPARATE_STRING_SIZE     (STRING_BUF_SIZE/32)



/*
   Given an identifier, convert it to a name.
   If it's not in the hash table, then put it there.
 
	Return pointer to a string.
	Guarantee the string only appears once in the string
	table(s).
*/
char * 
makename( char * s)
{
    unsigned int		len;
    unsigned int		c;
    unsigned int		h = 0;
    unsigned int		g;
    char 			*p;
    struct Name *		n;
    struct Namepool	*	newpool;
#define MASK 0xf0000000
    h = 0;
    /* p will actually point PAST the null terminator when the
    ** loop terminates.
    ** elf_hash  32-bit version: adequate for our needs.
	*/
    for (p = s; (c = *p++) != '\0'; ) {
	h = (h<<4) + c;
        if((g = h&MASK) != 0) { 
	   h ^= (g>>24);
	}
	/*
	   SYSV ABI says h &= ~g; here, but elf_hash code 
	   says  h &= ~MASK;  
	   The MASK form has the same effect as the original but
	   one may be faster than the other....
	*/
	h &= ~MASK; 
    }
#undef MASK

    /* end ELFHash, h is the value we want.
	*/

    h = h & (HASHTABLESIZE -1);

    /* len includes the null terminator, so len is the interesting length.
	*/
    len = p - s;

    n = Nametable[h];
    while (n != 0) {
      if (strlen(n->n_identifier) + 1 == len &&
	  memcmp(s, n->n_identifier, len) == 0)
		return n->n_identifier;
      n = n->n_chain;
    } /* while */

    /*
     * Now we know that name hasn't been found (otherwise we'd have jumped
     * down to match), so we allocate a name, store the identifier, and
     * enter it in the hash table.
     */

    if (Nleft <= 0) {
	newpool = (struct Namepool *)malloc(sizeof(struct Namepool));
        if(newpool == 0) {
		fprintf(stderr,"Out of memory malloc %d bytes\n",sizeof(struct Namepool));
		exit(1);
	}
	/* Leaving the Namepool filled with random malloc junk
	** aside from the prevpool pointer.
	** We don't need to clear this space.
	*/
	newpool->prevpool = Namepooltab;
	Namepooltab = newpool;
	Nleft = CHUNKSIZE;
    }
    /* By taking entries back to front, we preserve some paging locality:
    ** the first mention of the pool set prevpool which is the
    ** last entry (next to the Nleft-1 entry).
	*/
    --Nleft;
    n = &(Namepooltab->name[Nleft]);
    n->n_chain = Nametable[h];
    Nametable[h] = n;

    n->n_identifier = new_string_space(len);

    /* Finally, copy the string to it's final resting place.
	*/
    memcpy(n->n_identifier, s, len);
    return n->n_identifier;
}


/*
  Find a nice place to put the string itself.
  No memory clearing here. Just leave the random
  garbage returned by malloc.

*/
static char * new_string_space(unsigned len)
{
	char *newspace;
	if(len > buf_chars_left) {
	   if(len > SEPARATE_STRING_SIZE) {
	        /*really big strings we allocate separately
			*/
		newspace = (char*)malloc(len);
		if(newspace == 0) {
		  fprintf(stderr,"dbx: cannot allocate string space %u bytes\n",len);
		  exit(1);
		}
		return newspace;
	   }
	   curbuf_base = (char *)malloc(STRING_BUF_SIZE);
	   if(curbuf_base == 0) {
	      fprintf(stderr,"dbx: Cannot allocate string space %u bytes\n",
			STRING_BUF_SIZE);
		exit(1);
	   }
	   curbuf_cur = curbuf_base+ len;
	   buf_chars_left = STRING_BUF_SIZE - len;
	   return curbuf_base;
	   
        }
	newspace = curbuf_cur;
	curbuf_cur += len;
	buf_chars_left -= len;
	return newspace;
}
