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
#define CONFIG_UINT32_COMPARE 6
#define CONFIG_BITMAP_COPY 7
#define CONFIG_BITMAP_LSHIFT 8
#define CONFIG_BITMAP_RSHIFT 9
#define CONFIG_BITMAP_COMPARE 10
#define CONFIG_BITMAP_GET_EFF_BIT 11
#define CONFIG_BITMAP_PREFIX_APPLY_MASK 12
#define CONFIG_BITMAP_REVERSE   13

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
    uint16_t count;
    uint32_t num;    
    uint8_t st_offset;
    uint8_t end_offset;
    tlv_struct_t *tlvptr;
    uint32_t uint32_num1;
    uint32_t uint32_num2;
    uint32_t uint32_num3;
    uint32_t uint32_num4;
    uint32_t uint32_num5;
    uint32_t uint32_num6;    

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
        else if (parser_match_leaf_id(tlvptr->leaf_id, "uint32-num1"))    
            uint32_num1 = atoi(tlvptr->value);        
        else if (parser_match_leaf_id(tlvptr->leaf_id, "uint32-num2"))    
            uint32_num2 = atoi(tlvptr->value);           
        else if (parser_match_leaf_id(tlvptr->leaf_id, "uint32-num3"))    
            uint32_num3 = atoi(tlvptr->value);                      
        else if (parser_match_leaf_id(tlvptr->leaf_id, "uint32-num4"))    
            uint32_num4 = atoi(tlvptr->value);        
        else if (parser_match_leaf_id(tlvptr->leaf_id, "uint32-num5"))    
            uint32_num5 = atoi(tlvptr->value);           
        else if (parser_match_leaf_id(tlvptr->leaf_id, "uint32-num6"))    
            uint32_num6 = atoi(tlvptr->value);                                   
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
                memcpy (bm.bits, &dst, 4);
            }
            break;


            case CONFIG_UINT32_COMPARE:
            {
                assert (count >= 0 && count <= 32);
                if (count == 0) {
                    break;
                }
                uint32_t mask = bits_generate_ones(0, count - 1);
                if ((uint32_num1 & mask) == (uint32_num2 & mask)) {
                    bitmap_set_bit_at(&bm, 0);
                }
            }
            break;

            case CONFIG_BITMAP_COPY:
            {
                bitmap_t src = {0};
                bitmap_init(&src, 96);
                uint32_t temp = htonl (uint32_num1);
                memcpy (src.bits, &temp, 4);
                temp = htonl (uint32_num2);
                memcpy (src.bits + 1, &temp, 4);
                temp = htonl (uint32_num3);
                memcpy (src.bits + 2, &temp, 4);
                bitmap_copy(&src, &bm, st_offset, count);
                bitmap_free_internal(&src);
            }
            break;

            case CONFIG_BITMAP_LSHIFT:
            {
                bitmap_lshift(&bm, count);
            }
            break;

            case CONFIG_BITMAP_RSHIFT:
            {
                bitmap_rshift(&bm, count);
            }
            break;

            case CONFIG_BITMAP_COMPARE:
            {
                bitmap_t prefix1 = {0};
                bitmap_t prefix2 = {0};
                bitmap_init(&prefix1, 96);
                bitmap_init(&prefix2, 96);
                uint32_t temp = htonl (uint32_num1);
                memcpy (prefix1.bits, &temp, 4);
                temp = htonl (uint32_num2);
                memcpy (prefix1.bits + 1, &temp, 4);
                temp = htonl (uint32_num3);
                memcpy (prefix1.bits + 2, &temp, 4);
                temp = htonl (uint32_num4);
                memcpy (prefix2.bits, &temp, 4);
                temp = htonl (uint32_num5);
                memcpy (prefix2.bits + 1, &temp, 4);
                temp = htonl (uint32_num6);
                memcpy (prefix2.bits + 2, &temp, 4);
                if (bitmap_compare(&prefix1, &prefix2, count)) {
                    bitmap_set_bit_at(&bm, 0);
                }
                bitmap_free_internal(&prefix1);
                bitmap_free_internal(&prefix2);
            }
            break;

            case CONFIG_BITMAP_GET_EFF_BIT:
            {
                bitmap_t wc = {0};
                bitmap_init(&wc, 64);
                uint32_t temp = htonl (uint32_num1);
                memcpy (bm.bits, &temp, 4);
                temp = htonl (uint32_num2);
                memcpy (bm.bits + 1, &temp, 4);
                temp = htonl (uint32_num3);
                memcpy (wc.bits, &temp, 4);
                temp = htonl (uint32_num4);
                memcpy (wc.bits + 1, &temp, 4);    
                bit_type_t bit = bitmap_effective_bit_at(&bm, &wc, count);
                bitmap_init (&bm, 32);
                switch (bit) {
                    case ZERO:
                        bitmap_set_bit_at(&bm, 0);
                        break;
                    case ONE:
                        bitmap_set_bit_at(&bm, 1);
                        break;
                    case DONT_CARE:
                        bitmap_set_bit_at(&bm, 0);
                        bitmap_set_bit_at(&bm, 1);
                        break;
                    default: ;
                }
                bitmap_free_internal(&wc);
            }
            break;

            case CONFIG_BITMAP_REVERSE:
            {
                bitmap_reverse (&bm, count);
            }
            break;
            
            case CONFIG_BITMAP_PREFIX_APPLY_MASK:
            {
                bitmap_t mask = {0};
                bitmap_init (&mask, 64);
                uint32_t temp = htonl (uint32_num1);
                memcpy (mask.bits, &temp, 4);
                temp = htonl (uint32_num2);
                memcpy (mask.bits + 1, &temp, 4);
                bitmap_prefix_apply_mask(&bm, &mask, count);
                bitmap_free_internal(&mask);
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
            /* mv-copy <MV> <start-offset> <end-offset> <count>*/
            static param_t mv_copy;
            init_param(&mv_copy, CMD, "mv-copy", 0, 0, INVALID, 0, "memory value copy command");
            libcli_register_param(&bitmap, &mv_copy);
            {
                static param_t mv;
                init_param(&mv, LEAF, 0, 0, 0, INT, "uint32-num", "uint32_t number");
                libcli_register_param(&mv_copy, &mv);
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

        {
            /* reverse <count>*/
            static param_t reverse;
            init_param(&reverse, CMD, "reverse", 0, 0, INVALID, 0, "Reverse bits");
            libcli_register_param(&bitmap, &reverse);
            {
                static param_t count;
                init_param(&count, LEAF, 0, bitmap_config_handler, 0, INT, "count", "Number of bits to be reversed");
                libcli_register_param(&reverse, &count);
                set_param_cmd_code(&count, CONFIG_BITMAP_REVERSE);
            }
        }


        {
            /* eff-bit  <uint32_t> <uint32_t> <uint32_t> <uint32_t> <count>*/
            static param_t eff_bit;
            init_param(&eff_bit, CMD, "eff-bit", 0, 0, INVALID, 0, "effective bit command");
            libcli_register_param(&bitmap, &eff_bit);
            {
                static param_t uint32_num1;
                init_param(&uint32_num1, LEAF, 0, 0, 0, INT, "uint32-num1", "uint32_t number");
                libcli_register_param(&eff_bit, &uint32_num1);
                {
                    static param_t uint32_num2;
                    init_param(&uint32_num2, LEAF, 0, 0, 0, INT, "uint32-num2", "uint32_t number");
                    libcli_register_param(&uint32_num1, &uint32_num2);
                    {
                        static param_t uint32_num3;
                        init_param(&uint32_num3, LEAF, 0, 0, 0, INT, "uint32-num3", "uint32_t number");
                        libcli_register_param(&uint32_num2, &uint32_num3);
                        {
                            static param_t uint32_num4;
                            init_param(&uint32_num4, LEAF, 0, 0, 0, INT, "uint32-num4", "uint32_t number");
                            libcli_register_param(&uint32_num3, &uint32_num4);
                            {
                                static param_t count;
                                init_param(&count, LEAF, 0, bitmap_config_handler, 0, INT, "count", "count[0-32]");
                                libcli_register_param(&uint32_num4, &count);
                                set_param_cmd_code(&count, CONFIG_BITMAP_GET_EFF_BIT);
                            }
                        }
                    }
                }
            }
        }


        {
            /* config bitmap compare uint32 <uint32_t> <uint32_t> <count>*/
            static param_t compare;
            init_param(&compare, CMD, "compare", 0, 0, INVALID, 0, "compare command");
            libcli_register_param(&bitmap, &compare);
            {
                static param_t uint32;
                init_param(&uint32, CMD, "uint32", 0, 0, INVALID, 0, "uint32 command");
                libcli_register_param(&compare, &uint32);
                {
                    static param_t uint32_num1;
                    init_param(&uint32_num1, LEAF, 0, 0, 0, INT, "uint32-num1", "uint32_t number");
                    libcli_register_param(&uint32, &uint32_num1);
                    {
                        static param_t uint32_num2;
                        init_param(&uint32_num2, LEAF, 0, 0, 0, INT, "uint32-num2", "uint32_t number");
                        libcli_register_param(&uint32_num1, &uint32_num2);
                        {
                            static param_t count;
                            init_param(&count, LEAF, 0, bitmap_config_handler, 0, INT, "count", "count[0-32]");
                            libcli_register_param(&uint32_num2, &count);
                            set_param_cmd_code(&count, CONFIG_UINT32_COMPARE);
                        }
                    }
                }
            }
        }

        {
            /* config bitmap bm-copy <uint32_t> <uint32_t> <uint32_t> <start_index> <count>*/
            static param_t bm_copy;
            init_param(&bm_copy, CMD, "bm-copy", 0, 0, INVALID, 0, "bitmap copy command");
            libcli_register_param(&bitmap, &bm_copy);
            {
                static param_t uint32_num1;
                init_param(&uint32_num1, LEAF, 0, 0, 0, INT, "uint32-num1", "uint32_t number");
                libcli_register_param(&bm_copy, &uint32_num1);
                {
                    static param_t uint32_num2;
                    init_param(&uint32_num2, LEAF, 0, 0, 0, INT, "uint32-num2", "uint32_t number");
                    libcli_register_param(&uint32_num1, &uint32_num2);
                    {
                        static param_t uint32_num3;
                        init_param(&uint32_num3, LEAF, 0, 0, 0, INT, "uint32-num3", "uint32_t number");
                        libcli_register_param(&uint32_num2, &uint32_num3);
                        {
                            static param_t start_offset;
                            init_param(&start_offset, LEAF, 0, 0, 0, INT, "st-offset", "start index");
                            libcli_register_param(&uint32_num3, &start_offset);
                            {
                                static param_t count;
                                init_param(&count, LEAF, 0, bitmap_config_handler, 0, INT, "count", "Number of bits to copy");
                                libcli_register_param(&start_offset, &count);
                                set_param_cmd_code(&count, CONFIG_BITMAP_COPY);
                            }
                        }
                    }
                }
            }
        }

        {
            /* config bitmap apply-mask <uint32_t> <uint32_t> <count>*/
            static param_t apply_mask;
            init_param(&apply_mask, CMD, "apply-mask", 0, 0, INVALID, 0, "bitmap apply-mask command");
            libcli_register_param(&bitmap, &apply_mask);
            {
                static param_t uint32_num1;
                init_param(&uint32_num1, LEAF, 0, 0, 0, INT, "uint32-num1", "uint32_t number");
                libcli_register_param(&apply_mask, &uint32_num1);
                {
                    static param_t uint32_num2;
                    init_param(&uint32_num2, LEAF, 0, 0, 0, INT, "uint32-num2", "uint32_t number");
                    libcli_register_param(&uint32_num1, &uint32_num2);
                    {
                        static param_t count;
                        init_param(&count, LEAF, 0, bitmap_config_handler, 0, INT, "count", "Number of bits to copy");
                        libcli_register_param(&uint32_num2, &count);
                        set_param_cmd_code(&count, CONFIG_BITMAP_PREFIX_APPLY_MASK);
                    }
                }
            }
        }



        {
            /* config bitmap bm-compare <uint32_t> <uint32_t> <uint32_t> > <uint32_t> <uint32_t> <uint32_t> <count>*/
            static param_t bm_compare;
            init_param(&bm_compare, CMD, "bm-compare", 0, 0, INVALID, 0, "bitmap compare command");
            libcli_register_param(&bitmap, &bm_compare);
            {
                static param_t uint32_num1;
                init_param(&uint32_num1, LEAF, 0, 0, 0, INT, "uint32-num1", "uint32_t number");
                libcli_register_param(&bm_compare, &uint32_num1);
                {
                    static param_t uint32_num2;
                    init_param(&uint32_num2, LEAF, 0, 0, 0, INT, "uint32-num2", "uint32_t number");
                    libcli_register_param(&uint32_num1, &uint32_num2);
                    {
                        static param_t uint32_num3;
                        init_param(&uint32_num3, LEAF, 0, 0, 0, INT, "uint32-num3", "uint32_t number");
                        libcli_register_param(&uint32_num2, &uint32_num3);
                        {
                            static param_t uint32_num4;
                            init_param(&uint32_num4, LEAF, 0, 0, 0, INT, "uint32-num4", "uint32_t number");
                            libcli_register_param(&uint32_num3, &uint32_num4);
                            {
                                static param_t uint32_num5;
                                init_param(&uint32_num5, LEAF, 0, 0, 0, INT, "uint32-num5", "uint32_t number");
                                libcli_register_param(&uint32_num4, &uint32_num5);
                                {
                                    static param_t uint32_num6;
                                    init_param(&uint32_num6, LEAF, 0, 0, 0, INT, "uint32-num6", "uint32_t number");
                                    libcli_register_param(&uint32_num5, &uint32_num6);
                                    {
                                        static param_t count;
                                        init_param(&count, LEAF, 0, bitmap_config_handler, 0, INT, "count", "Number of bits to copy");
                                        libcli_register_param(&uint32_num6, &count);
                                        set_param_cmd_code(&count, CONFIG_BITMAP_COMPARE);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        {
            /* config bitmap left-shift <count>*/
            static param_t lshift;
            init_param(&lshift, CMD, "left-shift", 0, 0, INVALID, 0, "left shift command");
            libcli_register_param(&bitmap, &lshift);
            {
                static param_t count;
                init_param(&count, LEAF, 0, bitmap_config_handler, 0, INT, "count", "Number of bits to shift");
                libcli_register_param(&lshift, &count);
                set_param_cmd_code(&count, CONFIG_BITMAP_LSHIFT);
            }
        }

        {
            /* config bitmap right-shift <count>*/
            static param_t rshift;
            init_param(&rshift, CMD, "right-shift", 0, 0, INVALID, 0, "right shift command");
            libcli_register_param(&bitmap, &rshift);
            {
                static param_t count;
                init_param(&count, LEAF, 0, bitmap_config_handler, 0, INT, "count", "Number of bits to shift");
                libcli_register_param(&rshift, &count);
                set_param_cmd_code(&count, CONFIG_BITMAP_RSHIFT);
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