#ifndef PE1_PSYQ_STDARG_H
#define PE1_PSYQ_STDARG_H

/* PSY-Q STDARG.H: the caller spills a0-a3 to its argument area, so the
 * variadic words follow the last named parameter in memory. */
typedef char *va_list;

#define __va_rounded_size(TYPE) \
    (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(AP, LASTARG) \
    (AP = ((char *)&(LASTARG) + __va_rounded_size(LASTARG)))

#define va_end(AP) (AP = (char *)0)

#define va_arg(AP, TYPE) \
    (*(TYPE *)(((AP) += __va_rounded_size(TYPE)) - __va_rounded_size(TYPE)))

#endif
