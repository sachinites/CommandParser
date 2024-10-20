#include <stdio.h>
#include <stdlib.h>
#include "cmdtlv.h"
#include "libcli.h"
#include "bitmap.h"

static bitmap_t bm;

#define CONFIG_BITMAP_INIT  1
#define CONFIG_BITMAP_SETBIT 2
#define CONFIG_BITMAP_UNSETBIT 3

#define SHOW_BITMAP 1

static int
bitmap_show_handler (param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable){

    int cmdcode = EXTRACT_CMD_CODE(tlv_buf);

    switch (cmdcode) {

        case SHOW_BITMAP:
            {
                char *output = bitmap_print(&bm);
                cprintf ("%s", output);
            }
            break;

        default: ;
    }

    return 0;
}


static int
bitmap_config_handler (param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable){

    int bm_size;
    int index;
    tlv_struct_t *tlvptr;

    int cmdcode = EXTRACT_CMD_CODE(tlv_buf);

    TLV_LOOP_BEGIN(tlv_buf, tlvptr)
    {
        if (parser_match_leaf_id(tlvptr->leaf_id, "bm-size"))
            bm_size = atoi(tlvptr->value);
        else if (parser_match_leaf_id(tlvptr->leaf_id, "index"))    
            index = atoi(tlvptr->value);
    }
    TLV_LOOP_END;

    switch (cmdcode) {

        case CONFIG_BITMAP_INIT:
            {
                if ((bm_size %32) != 0) {
                    printf("Error : Bitmap size should be multiple of 32\n");
                    exit(0);
                }
                bitmap_init(&bm, bm_size);
            }
            break;

            case CONFIG_BITMAP_SETBIT:
            {
                bitmap_set_bit_at(&bm, index);
            }
            break;

            case CONFIG_BITMAP_UNSETBIT:
            {
                bitmap_unset_bit_at(&bm, index);
            }
            break;

            default: ;
    }

    return 0;
}

extern void
ut_parser_init ( ) ;

int 
main (int argc, char **argv) {

    init_libcli();
    ut_parser_init ( ) ;

    param_t *show   = libcli_get_show_hook();
    param_t *config = libcli_get_config_hook();

    /* Implement bit map command */
    
    /* config bitmap <size>*/
    {
        static param_t bitmap;
        init_param(&bitmap, CMD, "bitmap", 0, 0, INVALID, 0, "bitmap commands");
        libcli_register_param(config, &bitmap);
        {
            static param_t bitmap_size;
            init_param(&bitmap_size, LEAF, 0, bitmap_config_handler, 0, INT, "bm-size", "bitmap size");
            libcli_register_param(&bitmap, &bitmap_size);
            set_param_cmd_code(&bitmap_size, CONFIG_BITMAP_INIT);
        }
        {
            /* setbit <index>*/
            static param_t setbit;
            init_param(&setbit, CMD, "setbit", 0, 0, INVALID, 0, "set bit at index");
            libcli_register_param(&bitmap, &setbit);
            {
                static param_t index;
                init_param(&index, LEAF, 0, bitmap_config_handler, 0, INT, "index", "index position in bitmap");
                libcli_register_param(&setbit, &index);
                set_param_cmd_code(&index, CONFIG_BITMAP_SETBIT);
            }
        }

        {
            /* unsetbit <index>*/
            static param_t unsetbit;
            init_param(&unsetbit, CMD, "unsetbit", 0, 0, INVALID, 0, "unset bit at index");
            libcli_register_param(&bitmap, &unsetbit);
            {
                static param_t index;
                init_param(&index, LEAF, 0, bitmap_config_handler, 0, INT, "index", "index position in bitmap");
                libcli_register_param(&unsetbit, &index);
                set_param_cmd_code(&index, CONFIG_BITMAP_UNSETBIT);
            }
        }


    }

    /* Show commands */
    {
        /* show bitmap */
        {
            static param_t bitmap;
            init_param(&bitmap, CMD, "bitmap", bitmap_show_handler, 0, INVALID, 0, "show bitmap commands");
            libcli_register_param(show, &bitmap);
            set_param_cmd_code(&bitmap, SHOW_BITMAP);
        }
    }

    support_cmd_negation(config);
    start_shell();
    return 0;
}