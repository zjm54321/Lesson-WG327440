#ifdef CLANGD
// define for clangd

#define __data
#define __near
#define __xdata
#define __far
#define __idata
#define __pdata
#define __code
#define __interrupt(x)
#define __using(x)
#define __at(x)

#include <stdint.h>
typedef uint8_t __sfr;
typedef uint16_t __sfr16;
typedef uint32_t __sfr32;

// try find bool define
#if __STDC_VERSION__ >= 202311L
#define bit_t bool
#endif
#if !defined(bit_t) && __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#define bit_t _Bool
#endif
#if !defined(bit_t)
#define bit_t int
#endif

typedef bit_t __sbit;
typedef bit_t __bit;

#endif