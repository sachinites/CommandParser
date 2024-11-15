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
    struct list_node_ *prev, *next;
}  list_node_t;

typedef struct list_ {

    list_node_t *head;
    list_node_t *tail;

} list_t;

static void 
print_list (list_t *list) {

    list_node_t *it = list->head;

    while (it) {
        printf ("%d ", it->digit);
        it = it->next;
    }
}

#define is_list_empty(list_ptr) \  
    (list_ptr->head == list_ptr->tail && list_ptr->head == NULL)

static void 
digit_list_multiply (list_t *list, uint16_t *count) {

    uint8_t carry = 0;
    list_node_t *prev = NULL; 

    if ( is_list_empty(list) ) return;

    list_node_t *iter = list->tail;
    
    while (iter) {
        iter->digit = (iter->digit * 2) + carry;
        carry = 0;
        if (iter->digit >= 10) {
            carry = iter->digit / 10;
            iter->digit = iter->digit % 10;
        }
        iter = iter->prev;
    }

    if (carry) {
        list_node_t *carry_node = (list_node_t *)calloc (1, sizeof (list_node_t));
        carry_node->digit = carry;
        carry_node->next = list->head;
        list->head->prev = carry_node;
        list->head = carry_node;
        (*count)++;
    }
}


static void 
digit_list_increment (list_t *list, uint16_t *count) {

    uint8_t carry = 1;
    list_node_t *prev = NULL; 

    if ( is_list_empty(list) ) return;

    list_node_t *iter = list->tail;
    
    while (iter) {
        iter->digit = iter->digit + carry;
        carry = 0;
        if (iter->digit == 10) {
            iter->digit = 0;
            carry = 1;
        }
        iter = iter->prev;
    }

    if (carry) {
        list_node_t *carry_node = (list_node_t *)calloc (1, sizeof (list_node_t));
        carry_node->digit = carry;
        carry_node->next = list->head;
        list->head->prev = carry_node;
        list->head = carry_node;
        (*count)++;
    }
}

static char *
digit_list_to_char_array (list_node_t *head, uint16_t count) {

    char *output = (char *) calloc (count + 1, sizeof(uint8_t));
    list_node_t *node = head;
    uint8_t i = 0;

    while (node) {

        output[i++] = '0' + node->digit;
        node = node->next;
    }

    output[count] = '\0';
    return output;
}


char *
bitmap_to_GI (bitmap_t *bitmap, uint16_t *nout) {

    list_t list = {0, 0};

    if (!bitmap || !nout) return NULL;

    list.head = (list_node_t *)calloc (1, sizeof (list_node_t));
    list.head->digit = 0;
    list.head->next = NULL;
    list.head->prev = NULL;
    list.tail = list.head;

    uint16_t count = 1;
    uint16_t index;
    bool bit;

    ITERATE_BITMAP_BEGIN(bitmap, 0, index, bit) {

        digit_list_multiply (&list, &count);

        if (bit) {
            digit_list_increment (&list, &count);
        }

    } ITERATE_BITMAP_END;

    char *output = digit_list_to_char_array (list.head, count);

    *nout = count;

    /* Free the linked list*/   
    while (list.head) {
        list_node_t *temp = list.head;
        list.head = list.head->next;
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
    // watch below step , if you dont typecast, it will go wrong !
    uint64_t result = (uint64_t)bm1_rv + (uint64_t)bm2_rv + (uint64_t)carry;
    if (result > (uint64_t)UINT32_MAX) res = 1;
    *res_bm = (uint32_t) result;
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

        smaller_bm->bits = (uint32_t *)realloc (smaller_bm->bits, size_max/8);
        bits_expanded = size_max - smaller_bm->tsize;
        smaller_bm->tsize = size_max;
    	bitmap_rshift (smaller_bm, bits_expanded);
    }
 
    int i;
    uint32_t carry = 0;
    uint16_t n_blocks = size_max/32;
    bitmap_t result;
    result.bits = NULL;
    bitmap_init (&result, size_max);

#if 0
    printf ("Adding below two Bitmaps : \n");
    printf (" %s\n", bitmap_print(bm1));
    printf ("+%s\n", bitmap_print(bm2));
#endif 

    for (i = n_blocks - 1; i > -1; i--) {
	    carry = Add_two_uint32_bitmaps(&bm1->bits[i], &bm2->bits[i], carry, &result.bits[i]);    
        //printf ("Addition of block %d produce carry %d\n", i, carry);
    }

    //printf (" %s\n", bitmap_print(&result));

    GI_res_array = bitmap_to_GI (&result, nout);

    if (carry == 1) {
       char *temp = GI_res_array;
       GI_res_array = (char *)calloc ((*nout) + 1, sizeof(uint8_t));
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


/* Main fn to test */
#if 1
int
main (int argc, char **argv) {

#if 0
    char *gi1 = "123456789123456789123456789";
    bitmap_t *bm = GI_to_bitmap(gi1, strlen(gi1));
    printf ("%d %s\n", bm->next, bitmap_print(bm));
    bm->next = bm->tsize;
    uint16_t n = 0;
    char *gi2 = bitmap_to_GI(bm, &n);
    printf ("%d %s\n", n, gi2);
    free(gi2);
    bitmap_free(bm);
#elif 1
    char *gi1 = "999999999999999999999999999999999999999999999999999";
    char *gi2 =   "99999999999999999999999999999999999999999999999999";
    uint16_t res_size;
    char *res = GI_add(gi1, strlen (gi1), gi2, strlen (gi2), &res_size);
    printf ("GI after Add = %s\n", res);
    free(res);
#else

#endif
    return 0;
}

#endif