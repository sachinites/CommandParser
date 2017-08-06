/*
 * =====================================================================================
 *
 *       Filename:  clistd.h
 *
 *    Description:  All Validation functions for leaves and standard default callbacks are defined in this file
 *
 *        Version:  1.0
 *        Created:  Sunday 06 August 2017 05:56:03  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 * =====================================================================================
 */

#include "cmd_hier.h"
#include "clistd.h"
#include "cmdtlv.h"

extern void
set_console_name(const char *cons_name);

extern void
parse_input_cmd(char *input, unsigned int len);

extern char *
get_last_command();

/*Default validation handlers for Data types*/

 int
int_validation_handler(leaf_t *leaf, char *value_passed){
        /*printf("%s is called for leaf type = %s, leaf value = %s\n", __FUNCTION__,
         *                             get_str_leaf_type(leaf->leaf_type), value_passed);*/
            return VALIDATION_SUCCESS;
}


 int
string_validation_handler(leaf_t *leaf, char *value_passed){
        /*printf("%s is called for leaf type = %s, leaf value = %s\n", __FUNCTION__,
         *                             get_str_leaf_type(leaf->leaf_type), value_passed);*/
            return VALIDATION_SUCCESS;
}


 int
ipv4_validation_handler(leaf_t *leaf, char *value_passed){
        /*printf("%s is called for leaf type = %s, leaf value = %s\n", __FUNCTION__,
         *                             get_str_leaf_type(leaf->leaf_type), value_passed);*/
            return VALIDATION_SUCCESS;
}


 int
ipv6_validation_handler(leaf_t *leaf, char *value_passed){
        /*printf("%s is called for leaf type = %s, leaf value = %s\n", __FUNCTION__,
         *                             get_str_leaf_type(leaf->leaf_type), value_passed);*/
            return VALIDATION_SUCCESS;
}


 int
float_validation_handler(leaf_t *leaf, char *value_passed){
        /*printf("%s is called for leaf type = %s, leaf value = %s\n", __FUNCTION__,
         *                             get_str_leaf_type(leaf->leaf_type), value_passed);*/
            return VALIDATION_SUCCESS;
}

/* Default command handlers */
/*config console name <cons name>*/

 int
config_console_name_handler(ser_buff_t *b, op_mode enable_or_disable){

    tlv_struct_t *tlv = NULL;
    int i = 0;

    TLV_LOOP(b, tlv, i){
        if(enable_or_disable == CONFIG_ENABLE)
            set_console_name(tlv->value);
        else
            set_console_name("router");
    }
    return 0;
}



/*repeat*/
 int
repeat_last_command(ser_buff_t *b, op_mode enable_or_disable){
    static char new_line_consume[2];
    char *last_cmd = get_last_command();
    printf("prev : %s", last_cmd);
    scanf("%c", new_line_consume);;
    parse_input_cmd(last_cmd, strlen(last_cmd));
    return 0;
}



