#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include "bitmap.h"

bitmap_t * __attribute__((weak)) 
GI_to_bitmap (char *gi, uint16_t n) {

    return NULL;
}

char * __attribute__((weak)) 
bitmap_to_GI (bitmap_t *bitmap, uint16_t *nout) {

    return NULL;
}


char * __attribute__((weak)) 
GI_add (char *gi1, uint16_t n1, 
              char *gi2, uint16_t n2,
              uint16_t *nout ) {


    return NULL;
}

char * __attribute__((weak)) 
GI_mul (char *gi1, uint16_t n1, 
              char *gi2, uint16_t n2,
              uint16_t *nout ) {


    return NULL;
}
