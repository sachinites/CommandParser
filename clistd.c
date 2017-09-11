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
#include "libcli.h"
#include "css.h"
#include <signal.h>
#include "clicbext.h"
#include "string_util.h"

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

int
boolean_validation_handler(leaf_t *leaf, char *value_passed){

    if((strncmp(value_passed, "TRUE", strlen("TRUE")) == 0) || 
            (strncmp(value_passed, "FALSE", strlen("FALSE")) ==0))
        return VALIDATION_SUCCESS;

    return VALIDATION_FAILED;
}


/* Default command handlers */
/*config console name <cons name>*/
extern char console_name[TERMINAL_NAME_SIZE];

 int
config_console_name_handler(param_t *param, ser_buff_t *b, op_mode enable_or_disable){

    tlv_struct_t *tlv = NULL;
    int i = 0;

    TLV_LOOP(b, tlv, i){
        if(enable_or_disable == CONFIG_ENABLE)
            set_device_name(tlv->value);
        else{
            if(strncmp(tlv->value, console_name, strlen(tlv->value)) == 0)
                set_device_name(DEFAULT_DEVICE_NAME);
            else
                printf("Error : Incorrect device name\n");
        }
    }
    return 0;
}

/*repeat*/
 int
repeat_last_command(param_t *param, ser_buff_t *b, op_mode enable_or_disable){
    static char new_line_consume[2];
    char *last_cmd = get_last_command();
    printf("prev : %s", last_cmd);
    scanf("%c", new_line_consume);;
    parse_input_cmd(last_cmd, strlen(last_cmd));
    return 0;
}


 int
mode_enter_callback(param_t *param, ser_buff_t *b, op_mode enable_or_disable){
 
    if(param == libcli_get_root()){
        printf(ANSI_COLOR_YELLOW "Info : Mode not supported at root level\n" ANSI_COLOR_RESET);
        return 0;   
    }
    set_cmd_tree_cursor(param);
    build_mode_console_name(param);
    
    if(IS_APPLICATION_CALLBACK_HANDLER_REGISTERED(param))
        INVOKE_APPLICATION_CALLBACK_HANDLER(param, b, enable_or_disable);

    return 0;
}
 
 
 int
display_sub_options_callback(param_t *param, ser_buff_t *b, op_mode enable_or_disable){
    
    int i = 0;
    tlv_struct_t dummy;

    if(IS_APPLICATION_CALLBACK_HANDLER_REGISTERED(param))
        printf("<Enter>\n");

    for(i = CHILDREN_START_INDEX; i <= CHILDREN_END_INDEX; i++){
        if(param->options[i]){

            if(IS_PARAM_HIDDEN(param->options[i]))
                continue;

            if(IS_PARAM_CMD(param->options[i]) || IS_PARAM_NO_CMD(param->options[i])){
                printf(ANSI_COLOR_MAGENTA "nxt cmd  -> %-31s   |   %s\n" ANSI_COLOR_RESET, GET_CMD_NAME(param->options[i]), GET_PARAM_HELP_STRING(param->options[i]));
                continue;
            }
            printf(ANSI_COLOR_CYAN "nxt leaf -> %-32s  |   %s\n" ANSI_COLOR_RESET, GET_LEAF_TYPE_STR(param->options[i]), GET_PARAM_HELP_STRING(param->options[i]));
            continue;
        }
        break;
    }

    /*Means param is a leaf*/
    if(param->disp_callback){

        /*Add a dummy TLV to compensate for the cmd code TLV*/
        memset(&dummy, 0, sizeof(tlv_struct_t));
        collect_tlv(b, &dummy);
        printf(ANSI_COLOR_YELLOW "possible values :\n");
        param->disp_callback(param, b);
        printf(ANSI_COLOR_RESET);
    }
    return 0;
}
/* show history calback*/

ser_buff_t *file_read_buffer = NULL;

 int
show_history_callback(param_t *param, ser_buff_t *b, op_mode enable_or_disable){
    
    FILE *f = fopen("LIBCLI_HIST_FILE", "r");
    if(!f){
        printf("Error : History file could not be read\n");
        return 0;
    }
#if 0
    unsigned int i = 0;
    tlv_struct_t *tlv = NULL;

    tlv = (tlv_struct_t *)(b->b);
#endif
   return 0; 
}

int
clear_screen_handler(param_t *param, ser_buff_t *b, op_mode enable_or_disable){
    
    system("clear");
    return 0;   
}

int
exit_cmd_handler(param_t *param, ser_buff_t *b, op_mode enable_or_disable){
   
    go_one_level_up_cmd_tree(get_cmd_tree_cursor());
    return 0;
}

int
end_cmd_handler(param_t *param, ser_buff_t *b, op_mode enable_or_disable){

    goto_top_of_cmd_tree(get_cmd_tree_cursor());    
    return 0;
}

int
config_mode_enter_handler(param_t *param, ser_buff_t *b, op_mode enable_or_disable){

    set_cmd_tree_cursor(param);
    build_mode_console_name(param);
    mark_checkpoint_serialize_buffer(b);
    return 0;
}

int
negate_callback(param_t *param, ser_buff_t *b, op_mode enable_or_disable){
    printf("Command Negation - Type the cmd following to Negate\n");
    return 0;
}

int
supportsave_handler(param_t *param, ser_buff_t *b, op_mode enable_or_disable){

    switch(enable_or_disable){
        case CONFIG_ENABLE:
            signal(SIGTERM, terminate_signal_handler);/*When process is killed*/
            signal(SIGSEGV, terminate_signal_handler);/*When process access illegal memory*/
            signal(SIGABRT, terminate_signal_handler);/*when process abort itself*/
            break;
        case CONFIG_DISABLE:
            signal(SIGTERM, SIG_DFL);/*When process is killed*/
            signal(SIGSEGV, SIG_DFL);/*When process access illegal memory*/
            signal(SIGABRT, SIG_DFL);/*when process abort itself*/
            break;
        default:
            assert(0);
    }
    return 0;
}

int
show_help_handler(param_t *param, ser_buff_t *b, op_mode enable_or_disable){

    printf("Welcome to Help Wizard\n");
    printf("========================\n");
    printf("1. Use %s Character after the command to enter command mode\n", MODE_CHARACTER);
    printf("2. Use %s Character after the command to see possible follow up suboptions\n", SUBOPTIONS_CHARACTER);
    printf("3. Use 'do' from within the config branch to directly trigger operational commands\n");
    printf("4. Built-in commands:\n");
    printf("    a. cls - clear screen\n");
    printf("    b. end - jump to top of cmd tree\n");
    printf("    c. exit - jump one level up of command tree\n");
    printf("    d. config [no] console name <console name> - set/unset new console name\n");
    printf("    e. config [no] supportsave enable - enable/disable supportsave facility\n");
    printf("    f. debug show cmdtree - Show entire command tree\n");
    printf("    g. show history - show history of commands triggered\n");
    printf("    h. repeat - repeat the last command\n");
    printf(ANSI_COLOR_YELLOW "                      Author : Abhishek Sagar, Juniper Networks\n" ANSI_COLOR_RESET);
    return 0;
}

static void
dump_all_commands(param_t *root, unsigned int index){

        if(!root)
            return;

        if(IS_PARAM_NO_CMD(root))
            return;

        if(IS_PARAM_CMD(root)){
            untokenize(index);
            tokenize(GET_CMD_NAME(root), strlen(GET_CMD_NAME(root)), index);
        }
        else if(IS_PARAM_LEAF(root)){
            untokenize(index);
            tokenize(GET_LEAF_ID(root), strlen(GET_LEAF_ID(root)), index);
        }

        unsigned int i = CHILDREN_START_INDEX;

        for( ; i <= CHILDREN_END_INDEX; i++)
            dump_all_commands(root->options[i], index+1);
        
        if(IS_APPLICATION_CALLBACK_HANDLER_REGISTERED(root)){
            print_tokens(index + 1);
            printf("\n");
        }
}

int
show_resgistered_cmd_handler(param_t *param, ser_buff_t *b, op_mode enable_or_disable){
    
    /*Implement DFS and dump all complete commands*/
    re_init_tokens(MAX_CMD_TREE_DEPTH);

    param_t *root = libcli_get_root();
    
    unsigned int index = 0;
    dump_all_commands(root, index);
    return 0;
}

int
show_cmd_tree(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable){

        dump_cmd_tree();
        return 0;
}

