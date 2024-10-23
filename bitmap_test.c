#include <stdio.h>
#include <stdlib.h>
#include "cmdtlv.h"
#include <netinet/in.h>
#include "libcli.h"
#include "bitmap.h"

static bitmap_t bm;

#define CONFIG_BITMAP_INIT  1
#define CONFIG_BITMAP_SETBIT 2
#define CONFIG_BITMAP_UNSETBIT 3
#define CONFIG_UINT32_GEN_ONES 4
#define CONFIG_UINT32_BITS_COPY   5

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

    int index;
    int bm_size;
    uint8_t count;
    uint32_t num;    
    uint8_t st_offset;
    uint8_t end_offset;
    tlv_struct_t *tlvptr;

    int cmdcode = EXTRACT_CMD_CODE(tlv_buf);

    TLV_LOOP_BEGIN(tlv_buf, tlvptr)
    {
        if (parser_match_leaf_id(tlvptr->leaf_id, "bm-size"))
            bm_size = atoi(tlvptr->value);
        if (parser_match_leaf_id(tlvptr->leaf_id, "index"))
            index = atoi(tlvptr->value);
        else if (parser_match_leaf_id(tlvptr->leaf_id, "st-offset"))    
            st_offset = atoi(tlvptr->value);
        else if (parser_match_leaf_id(tlvptr->leaf_id, "end-offset"))    
            end_offset = atoi(tlvptr->value); 
        else if (parser_match_leaf_id(tlvptr->leaf_id, "uint32-num"))    
            num = atoi(tlvptr->value);             
        else if (parser_match_leaf_id(tlvptr->leaf_id, "count"))    
            count = atoi(tlvptr->value);                          
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

            case CONFIG_UINT32_GEN_ONES:
            {
                assert (st_offset >= 0 && st_offset <= 31);
                assert (end_offset >= 0 && end_offset <= 31);
                assert (st_offset <= end_offset);
                uint32_t m = bits_generate_ones(st_offset, end_offset);
                m = htonl (m);
                memcpy (bm.bits, &m, 4);
            }
            break;

            case CONFIG_UINT32_BITS_COPY:
            {
                assert (st_offset >= 0 && st_offset <= 31);
                assert (end_offset >= 0 && end_offset <= 31);
                num = htonl(num);
                uint32_t dst = 0;
                uint32_bits_copy (&num, &dst, st_offset, end_offset, count);
                bitmap_init(&bm, 32);
                memcpy (bm.bits, &dst, 4);
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
            /* generate-ones <start-offset> <end-offset>*/
            static param_t gen_ones;
            init_param(&gen_ones, CMD, "generate-ones", 0, 0, INVALID, 0, "gen-ones command");
            libcli_register_param(&bitmap, &gen_ones);
            {
                static param_t start_offset;
                init_param(&start_offset, LEAF, 0, 0, 0, INT, "st-offset", "start-offset[0-31]");
                libcli_register_param(&gen_ones, &start_offset);
                {
                    static param_t end_offset;
                    init_param(&end_offset, LEAF, 0, bitmap_config_handler, 0, INT, "end-offset", "end-offset[0-31]");
                    libcli_register_param(&start_offset, &end_offset);
                    set_param_cmd_code(&end_offset, CONFIG_UINT32_GEN_ONES);
                }
            }
        }

        {
            /* copy <MV> <start-offset> <end-offset> <count>*/
            static param_t copy;
            init_param(&copy, CMD, "copy", 0, 0, INVALID, 0, "copy command");
            libcli_register_param(&bitmap, &copy);
            {
                static param_t mv;
                init_param(&mv, LEAF, 0, 0, 0, INT, "uint32-num", "uint32_t number");
                libcli_register_param(&copy, &mv);
                {
                    static param_t start_offset;
                    init_param(&start_offset, LEAF, 0, 0, 0, INT, "st-offset", "start-offset[0-31]");
                    libcli_register_param(&mv, &start_offset);
                    {
                        static param_t end_offset;
                        init_param(&end_offset, LEAF, 0, 0, 0, INT, "end-offset", "end-offset[0-31]");
                        libcli_register_param(&start_offset, &end_offset);
                        {
                            static param_t count;
                            init_param(&count, LEAF, 0, bitmap_config_handler, 0, INT, "count", "count[0-32]");
                            libcli_register_param(&end_offset, &count);
                            set_param_cmd_code(&count, CONFIG_UINT32_BITS_COPY);
                        }
                    }
                }
            }
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