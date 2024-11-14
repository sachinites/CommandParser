#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "bitmap.h"

static uint8_t 
digit_array_divide (char *digit_array, 
                                uint16_t size, 
                                bool *has_non_zero_elements) {

    int i = 0;
    char c;
    uint8_t digit;
    uint8_t carry = 0;
    *has_non_zero_elements = false;

    while (digit_array[i] == '0')
    {
        i++;
        continue;
    }

    for (; i < size; i++)
    {
        c = digit_array[i];
        digit = atoi(&c);
        digit = 10 * carry + digit;
        digit_array[i] = '0' + (digit / 2);
        if (digit_array[i] != '0')
            *has_non_zero_elements = true;
        carry = digit % 2;
    }

    return carry;
}

bitmap_t *
GI_to_bitmap (char *gi, uint16_t n) {

    if (!gi || !n) return NULL;

    bitmap_t *bitmap = (bitmap_t *) calloc (1, sizeof(bitmap_t));
    if (!bitmap) return NULL;
    
    char *input = (char *) calloc (n, sizeof(char));
    strncpy (input, gi, n);
    bitmap_init(bitmap, 32);

    uint8_t carry;
    bool has_non_zero_elements;

    do {
        
        carry = digit_array_divide (input, n, &has_non_zero_elements);

        if (carry)
            bitmap_set_bit_at(bitmap, bitmap->next);
        else
            bitmap_unset_bit_at(bitmap, bitmap->next);

        bitmap->next++;

    } while (has_non_zero_elements);

    bitmap_reverse (bitmap, bitmap->next);
    bitmap_rshift (bitmap, bitmap->tsize - bitmap->next);
    free(input);
    return bitmap;
}

typedef struct list_node_ {

    uint8_t digit;
    struct list_node_ *next;
}  list_node_t;

static void 
digit_list_multiply (list_node_t *head, uint16_t *count) {

    list_node_t *node = head;
    list_node_t *prev = NULL;
    uint8_t carry = 0;
    uint8_t digit = 0;

    while (node) {

        digit = node->digit * 2 + carry;
        node->digit = digit % 10;
        carry = digit / 10;
        prev = node;
        node = node->next;
    }

    if (carry) {

        list_node_t *new_node = (list_node_t *) calloc (1, sizeof(list_node_t));
        new_node->digit = carry;
        prev->next = new_node;
        (*count)++;
    }
}

static void 
digit_list_increment (list_node_t *head, uint16_t *count) {

    list_node_t *node = head;
    list_node_t *prev = NULL;
    uint8_t carry = 1;
    uint8_t digit = 0;

    while (node) {

        digit = node->digit + carry;
        node->digit = digit % 10;
        carry = digit / 10;
        prev = node;
        node = node->next;
    }

    if (carry) {

        list_node_t *new_node = (list_node_t *) calloc (1, sizeof(list_node_t));
        new_node->digit = carry;
        prev->next = new_node;
        (*count)++;
    }
}

static char *
digit_list_to_char_array (list_node_t *head, uint16_t count) {

    char *output = (char *) calloc (count, sizeof(uint8_t));
    list_node_t *node = head;
    uint8_t i = 0;

    while (node) {

        output[i++] = '0' + node->digit;
        node = node->next;
    }

    return output;
}


char *
bitmap_to_GI (bitmap_t *bitmap, uint16_t *nout) {

    if (!bitmap || !nout) return NULL;

    list_node_t *head = (list_node_t *)calloc (1, sizeof (list_node_t));
    head->digit = 0;
    head->next = NULL;

    uint16_t count = 1;
    uint16_t index;
    bool bit;

    ITERATE_BITMAP_BEGIN(bitmap, 0, index, bit) {

        digit_list_multiply (head, &count);

        if (bit) {
            digit_list_increment (head, &count);
        }

    } ITERATE_BITMAP_END;

    char *output = digit_list_to_char_array (head, count);

    *nout = count;

    /* Free the linked list*/   
    while (head) {
        list_node_t *temp = head;
        head = head->next;
        free(temp);
    }

    return output;
}

static uint32_t
Add_two_uint32_bitmaps  (uint32_t *bm1, 
			 uint32_t *bm2,
			 uint32_t carry,
			 uint32_t *res_bm) {

    uint32_t res = 0;
    uint32_t bm1_rv = htonl (*bm1);
    uint32_t bm2_rv = htonl (*bm2);
    uint64_t result = bm1_rv + bm2_rv + carry;
    if (result > UINT32_MAX) res = 1;
    *res_bm = bm1_rv + bm2_rv + carry;
    *res_bm = htonl (*res_bm);
    return res;
}



char *
GI_add (char *gi1, uint16_t n1, 
              char *gi2, uint16_t n2,
              uint16_t *nout ) {

    char *GI_res_array;
    bitmap_t *bm1 = GI_to_bitmap(gi1, n1);
    bitmap_t *bm2 = GI_to_bitmap(gi2, n2);

    
    bitmap_t *smaller_bm = NULL;
    uint16_t size_max = bm1->tsize;

    if (bm1->tsize > bm2->tsize){
	    smaller_bm = bm2;
	    size_max = bm1->tsize ;
    }
    else if (bm1->tsize < bm2->tsize) {
	    smaller_bm = bm1;
	    size_max = bm2->tsize;
    }
   
    uint16_t bits_expanded = 0;

    if (smaller_bm) {

	smaller_bm->bits = (uint32_t *)realloc (smaller_bm->bits, size_max/32);
	bits_expanded = size_max - smaller_bm->tsize;
	smaller_bm->tsize = size_max;
    	bitmap_rshift (smaller_bm, bits_expanded);
    }
 
    int i;
    uint32_t carry = 0;
    uint16_t n_blocks = size_max/32;
    bitmap_t result;

    bitmap_init (&result, size_max);

    for (i = n_blocks - 1; i > -1; i--) {

	carry = Add_two_uint32_bitmaps(&bm1->bits[i], &bm2->bits[i], carry, &result.bits[i]);
    }
     
    GI_res_array = bitmap_to_GI (&result, nout);

    if (carry == 1) {
       char *temp = GI_res_array;
       GI_res_array = (char *)calloc (1, *nout + 1);
       GI_res_array[0] = '1';
       memcpy (GI_res_array + 1, temp, *nout);
       free(temp);
    }

    bitmap_free(bm1);
    bitmap_free(bm2);
    bitmap_free_internal (&result);    
    return GI_res_array;
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

#if 1
/* Main fn to test */
int
main (int argc, char **argv) {

#if 0
    char *gi1 = "123456789876543212345678987654321";
    bitmap_t *bm = GI_to_bitmap(gi1, strlen(gi1));
    printf ("%d %s\n", bm->next, bitmap_print(bm));
    bm->next = bm->tsize;
    uint16_t n = 0;
    char *gi2 = bitmap_to_GI(bm, &n);
    printf ("%d %s\n", n, gi2);
    free(gi2);
    bitmap_free(bm);
#endif
    char *gi1 = "123";
    char *gi2 = "123";
    uint16_t res_size;
    char *res = GI_add(gi1, strlen (gi1), gi2, strlen (gi2), &res_size);
    printf ("GI after Add = %s\n", res);
    return 0;
}
#endif
