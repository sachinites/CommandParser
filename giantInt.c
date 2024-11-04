#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
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

#if 0
/* Main fn to test */
int
main (int argc, char **argv) {

    char *gi1 = "123456789876543212345678987654321";
    bitmap_t *bm = GI_to_bitmap(gi1, strlen(gi1));
    printf ("%d %s\n", bm->next, bitmap_print(bm));
    bm->next = bm->tsize;
    uint16_t n = 0;
    char *gi2 = bitmap_to_GI(bm, &n);
    printf ("%d %s\n", n, gi2);
    free(gi2);
    bitmap_free(bm);
    return 0;
}
#endif