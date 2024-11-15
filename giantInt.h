#ifndef __GIANT_INT__
#define  __GIANT_INT__

#include <stdint.h>

typedef struct bitmap_ bitmap_t;

bitmap_t *
GI_to_bitmap (char *gi1, uint16_t n);

char *
bitmap_to_GI (bitmap_t *bitmap, uint16_t *nout);

char *
GI_add (char *gi1, uint16_t n1, 
              char *gi2, uint16_t n2,
              uint16_t *nout );

char *
GI_mul (char *gi1, uint16_t n1, 
              char *gi2, uint16_t n2,
              uint16_t *nout );

#endif 

