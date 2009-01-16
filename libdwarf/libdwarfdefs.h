/* libdwarfdefs.h
*/

#ifndef LIBDWARFDEFS_H
#define LIBDWARFDEFS_H

/* We want __uint32_t and __uint64_t and __int32_t __int64_t
   properly defined but not duplicated, since duplicate typedefs
   are not legal C.
*/
/*
 HAVE___UINT32_T
 HAVE___UINT64_T will be set by configure if
 our 4 types are predefined in compiler
*/


#if (!defined(HAVE___UINT32_T)) && defined(HAVE_SGIDEFS_H)
#include <sgidefs.h> /* sgidefs.h defines them */
#define HAVE___UINT32_T 1
#define HAVE___UINT64_T 1
#endif



#if (!defined(HAVE___UINT32_T)) &&   \
	defined(HAVE_SYS_TYPES_H) &&   \
	defined(HAVE___UINT32_T_IN_SYS_TYPES_H)
#  include <sys/types.h>
/* we assume __[u]int32_t and __[u]int64_t defined 
   since __uint32_t defined in the sys/types.h in use */
#define HAVE___UINT32_T 1
#define HAVE___UINT64_T 1
#endif

#ifndef HAVE___UINT32_T
typedef int __int32_t;
typedef unsigned  __uint32_t;
#define HAVE___UINT32_T 1
#endif
#ifndef HAVE___UINT64_T
typedef long long __int64_t;
typedef unsigned long long  __uint64_t;
#define HAVE___UINT64_T 1
#endif

#endif
