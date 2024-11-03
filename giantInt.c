#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include "bitmap.h"

bitmap_t *
GI_to_bitmap (char *gi, uint16_t n) {

    if (!gi || !n) return NULL;

    bitmap_t *bitmap = (bitmap_t *) calloc (1, sizeof(bitmap_t));
    if (!bitmap) return NULL;
    
    /* To start with, let us start with 32 bit bitmaps. It increases 
        as we insert more bits into it. Check bitmap_set_bit_at( ) 
        and bitmap_unset_bit_at( )
    */
    bitmap_init(bitmap, 32);

    uint8_t carry = 0;
    bool go_again = true;
    uint8_t i = 0, digit;

    while (go_again)
    {
        go_again = false;
        carry = 0;

        /* Skip all leading zeros 0's */
        while (gi[i] == '0')
        {
            i++;
        }

        for (; i < n; i++)
        {
            digit = atoi(gi[i]);
            digit = 10 * carry + digit;
            gi[i] = itoa(digit / 2);
            if (gi[i] != '0')
                go_again = true;
            carry = digit % 2;
        }

        if (carry) 
            bitmap_set_bit_at(bitmap, bitmap->next);
        else
            bitmap_unset_bit_at(bitmap, bitmap->next);
            
        bitmap->next++;
    }

    bitmap_reverse (bitmap);
    bitmap_rshift (bitmap, bitmap->tsize - bitmap->next);
    return bitmap;
}

char *
bitmap_to_GI (bitmap_t *bitmap, uint16_t *nout) {

    return NULL;
}

char *
GI_add (char *gi1, uint16_t n1, 
              char *gi2, uint16_t n2,
              uint16_t *nout ) {


    return NULL;
}

char *
GI_sub (char *gi1, uint16_t n1, 
              char *gi2, uint16_t n2,
              uint16_t *nout ) {

    return NULL;
}

char *
GI_mul (char *gi1, uint16_t n1, 
              char *gi2, uint16_t n2,
              uint16_t *nout ) {


    return NULL;
}


/* Main fn to test */
int 
main (int argc, char **argv) {


    return 0;
}