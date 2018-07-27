#ifndef _INCLUDES__H__
#define _INCLUDES__H__

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <strings.h> 
#include <inttypes.h>
#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>

#define countof(a) (sizeof(a)/sizeof(a[0]))

#define downcast(p, type, member)  ((p) ? ((type *)(((char *)(p)) - offsetof(type, member))) : NULL)

#include "list.h"
#include "slist.h"

#if !defined(ffsll)
static inline int
ffsll(long long int i)
{
    unsigned long long int x = i & -i;

    if (x <= 0xffffffff) {
        return ffs (i);
    }
    return 32 + ffs (i >> 32);
}
#endif

#endif /* _INCLUDES__H__ */
