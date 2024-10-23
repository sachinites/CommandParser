/* You are not suppose to modify this file */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>
#include "bitmap.h"

void __attribute__((weak)) 
bitmap_init(bitmap_t *bitmap, uint16_t size) {
    
    assert(!(size % 32));
    if (bitmap->bits) free(bitmap->bits);
    bitmap->bits = (uint32_t *)calloc(0, (size/8) * sizeof(uint8_t));
    bitmap->tsize = size;
    bitmap->next = 0;
}

void __attribute__((weak))
bitmap_free_internal(bitmap_t *bitmap) {
   
}

void __attribute__((weak))
bitmap_free(bitmap_t *bitmap) {
   
}

void __attribute__((weak))
bitmap_reset(bitmap_t *bitmap) {

}

bool __attribute__((weak))
bitmap_at(bitmap_t *bitmap, uint16_t index) {

    return true;
}

/* Endianess independent */
void __attribute__((weak))
bitmap_set_bit_at(bitmap_t *bitmap, uint16_t index) {


}

/* Endianess independent */
void __attribute__((weak))
bitmap_unset_bit_at(bitmap_t *bitmap, uint16_t index) {


}

void __attribute__((weak))
bitmap_set(bitmap_t *bitmap, uint16_t start_offset, 
                            uint16_t end_offset, bool set) {
    
}

void __attribute__((weak))
bitmap_prefix_apply_mask(bitmap_t *prefix, bitmap_t *mask, uint16_t count) {


}

void __attribute__((weak))
bitmap_inverse(bitmap_t *bitmap, uint16_t count) {

}

void __attribute__((weak))
bitmap_slow_copy(bitmap_t *src, 
                      bitmap_t *dst,
                      uint16_t src_start_offset,
                      uint16_t dst_start_offset,
                      uint16_t count) {

}

void __attribute__((weak))
bitmap_fast_copy(bitmap_t *src, 
                              bitmap_t *dst,
                              uint16_t count) {


}

void __attribute__((weak))
bitmap_lshift(bitmap_t *bitmap, uint16_t count) {

}



void __attribute__((weak))
bitmap_rshift(bitmap_t *bitmap, uint16_t count) {


}

bool __attribute__((weak))
bitmap_slow_compare(bitmap_t *src, 
                      bitmap_t *dst,
                      uint16_t src_start_offset,
                      uint16_t dst_start_offset,
                      uint16_t count) {

    return true;
}

bool __attribute__((weak))
bitmap_fast_compare(bitmap_t *src, 
                                    bitmap_t *dst,
                                    uint16_t count) {

   return true;
}

bool __attribute__((weak))
bitmap_prefix_match(bitmap_t *input, 
                                    bitmap_t *prefix, 
                                    bitmap_t *mask,
                                    uint16_t prefix_len) {

    return true;
}

bit_type_t __attribute__((weak))
bitmap_effective_bit_at(bitmap_t *prefix, bitmap_t *mask, uint16_t pos) {

    return ZERO;
}

char * __attribute__((weak))
bitmap_print(bitmap_t *bitmap) {

   return NULL;
}

char * __attribute__((weak))
bitmap_prefix_print(bitmap_t *prefix, bitmap_t *mask, uint16_t count) {

    return NULL;
}

/* Functions on 32 bit integers, used by bitmaps as helper fns */

bool __attribute__((weak))
prefix32bit_match(uint32_t input, uint32_t prefix, 
                                uint32_t mask, uint8_t prefix_len) {

	return false;
}

void __attribute__((weak))
uint32_bits_copy(uint32_t *src, uint32_t *dst,
                             uint8_t src_start_pos,
                             uint8_t dst_start_pos, uint8_t count) {


}

void __attribute__((weak))
uint32_bits_copy_preserve(uint32_t *src, 
                                            uint32_t *dst, 
                                            uint8_t src_start_pos,
                                            uint8_t dst_start_pos,
                                            uint8_t count) {

}

bool __attribute__((weak))
uint32_bits_compare (uint32_t bits1, uint32_t bits2, uint8_t count) {

    return true;
}

uint32_t __attribute__((weak))
bits_generate_ones(uint8_t start_offset, uint8_t end_offset) {

    return 0;
}


