#include <stdio.h>
#include <string.h>
#include "bitmap.h"
#include "giantInt.h"

int
main (int argc, char **argv) {

    char *gi1 = "123456789123456789123456789";
    bitmap_t *bm = GI_to_bitmap(gi1, strlen(gi1));
    printf ("%s\n", bitmap_print(bm));
    bitmap_free(bm);
    return 0;
}