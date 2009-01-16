

/* see if __uint32_t is predefined in the compiler */
/* #undef HAVE___UINT32_T */

/* see if __uint64_t is predefined in the compiler */
/* #undef HAVE___UINT64_T */

/* Define 1 if sys/types.h defines __uint32_t */
#define HAVE___UINT32_T_IN_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

#if (!defined(HAVE___UINT32_T)) &&   \
        defined(HAVE_SYS_TYPES_H) &&   \
        defined(HAVE___UINT32_T_IN_SYS_TYPES_H)
#  include <sys/types.h>
/* we assume __[u]int32_t and __[u]int64_t defined
   since __uint32_t defined in the sys/types.h in use */
#define HAVE___UINT32_T 1
#define HAVE___UINT64_T 1
#endif


__uint32_t x;
