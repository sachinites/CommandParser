/*
 * =====================================================================================
 *
 *       Filename:  cmd_hier.c
 *
 *    Description:  This file defines the structure for maintaining cmd hierarchy
 *
 *        Version:  1.0
 *        Created:  Thursday 03 August 2017 02:12:46  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 * =====================================================================================
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cmdtlv.h"
#include "libcli.h"
#include "clistd.h"

#define TLV_MAX_BUFFER_SIZE 1048

param_t root;
leaf_type_handler leaf_handler_array[LEAF_MAX];
ser_buff_t *tlv_buff;

/*Default zero level commands hooks. */
param_t show;
param_t debug;
param_t config;

/* Function to be used to get access to above hooks*/

param_t *
libcli_get_show_hook(void){
    return &show;
}

param_t *
libcli_get_debug_hook(void){
    return &debug;
}

param_t *
libcli_get_config_hook(void){
    return &config;
}

extern char *
get_last_command();

extern void
parse_input_cmd(char *input, unsigned int len);

char*
get_str_leaf_type(leaf_type_t leaf_type){

    switch(leaf_type){
        case INT:
            return "INT";
        case STRING:
            return "STRING";
        case IPV4:
            return "IPV4";
        case FLOAT:
            return "FLOAT";
        case IPV6:
            return "IPV6";
        case LEAF_MAX:
            return "LEAF_MAX";
        default:
            return "Unknown";
    }
    return NULL;
}

void 
init_libcli(){

    init_param(&root, CMD, "ROOT", 0, 0, INVALID, 0, "ROOT");

    /*Leaf datatypes standard Validation callbacks registration*/
    leaf_handler_array[INT]     = int_validation_handler;
    leaf_handler_array[STRING]  = string_validation_handler;
    leaf_handler_array[IPV4]    = ipv4_validation_handler;
    leaf_handler_array[IPV6]    = ipv6_validation_handler;
    leaf_handler_array[FLOAT]   = float_validation_handler;

    /*Intialised serialized buffer to collect leaf values in TLV format*/
    init_serialized_buffer_of_defined_size(&tlv_buff, TLV_MAX_BUFFER_SIZE);

    set_console_name("router");

    /*Registering Zero level default command hooks*/
    /*Show hook*/
    init_param(&show, CMD, "show", 0, 0, INVALID, 0, "show");
    libcli_register_param(&root, &show);

    /*debug hook*/
    init_param(&debug, CMD, "debug", 0, 0, INVALID, 0, "debug");
    libcli_register_param(&root, &debug);

    /*configure hook*/
    init_param(&config, CMD, "config", 0, 0, INVALID, 0, "config");
    libcli_register_param(&root, &config);

    /*configure repeat*/
    static param_t repeat;
    init_param(&repeat, CMD, "repeat", repeat_last_command, 0, INVALID, 0, "repeat");
    libcli_register_param(&root, &repeat);
    
    /* 'no' hook*/
    static param_t no;
    init_param(&no, CMD, "no", 0, 0, INVALID, 0, "command negation");
    libcli_register_param(&root, &no);
    
    /* 'no config' hook*/
    libcli_register_param(&no, &config); 
    
    /*config console name <new name>*/
    static param_t config_console;
    init_param(&config_console, CMD, "console", 0, 0, INVALID, 0, "console");
    libcli_register_param(&config, &config_console);

    
    static param_t config_console_name;
    init_param(&config_console_name, CMD, "name", 0, 0, INVALID, 0, "name");
    libcli_register_param(&config_console, &config_console_name);

    static param_t config_console_name_name;
    init_param(&config_console_name_name, LEAF, 0, config_console_name_handler, 0, STRING, "cons-name", "Name of Console"); 
    libcli_register_param(&config_console_name, &config_console_name_name);
}

void
init_param(param_t *param,                               /* pointer to static param_t variable*/
        param_type_t param_type,                         /* CMD|LEAF*/
        char *cmd_name,                                  /* <command name> | NULL*/
        cmd_callback callback,                           /* Callback field*/
        user_validation_callback user_validation_cb_fn,  /* NULL | <callback ptr>*/
        leaf_type_t leaf_type,                           /* INVALID | leaf type*/
        char *leaf_id,                                   /* NULL, <STRING>*/
        char *help){                                     /* Help String*/

    int i = 0;
    if(param_type == CMD){
        GET_PARAM_CMD(param) = calloc(1, sizeof(cmd_t));
        param->param_type = CMD;
        strncpy(GET_CMD_NAME(param), cmd_name, MIN(CMD_NAME_SIZE, strlen(cmd_name)));
        GET_CMD_NAME(param)[CMD_NAME_SIZE -1] = '\0';
    }
    else{
        GET_PARAM_LEAF(param) = calloc(1, sizeof(leaf_t));
        param->param_type = LEAF;
        GET_PARAM_LEAF(param)->leaf_type = leaf_type;
        param->cmd_type.leaf->user_validation_cb_fn = user_validation_cb_fn;
        strncpy(GET_PARAM_HELP_STRING(param), help, MIN(PARAM_HELP_STRING_SIZE, strlen(help)));
        GET_PARAM_HELP_STRING(param)[PARAM_HELP_STRING_SIZE -1] = '\0';
        strncpy(GET_LEAF_ID(param), leaf_id, MIN(LEAF_ID_SIZE, strlen(leaf_id)));
        GET_LEAF_ID(param)[LEAF_ID_SIZE -1] = '\0';
    }

    param->parent = NULL;
    param->callback = callback;
    strncpy(GET_PARAM_HELP_STRING(param), help, MIN(PARAM_HELP_STRING_SIZE, strlen(help)));

    for(; i < MAX_OPTION_SIZE; i++){
        param->options[i] = NULL;
    }
}

void
libcli_register_param(param_t *parent, param_t *child){
    int i = 0;
    if(!parent)
        parent = &root;
        
    for(; i < MAX_OPTION_SIZE; i++){
        if(parent->options[i])
            continue;

        parent->options[i] = child;
        child->parent = parent;
        return;
    }

    printf("%s() : Error : No space for new command\n", __FUNCTION__);
    assert(0);
}

static void
_dump_one_cmd(param_t *param, unsigned short tabs){

    int i = 0;
    cmd_t *cmd = NULL;

    PRINT_TABS(tabs);

    if(IS_PARAM_CMD(param))
        printf("-->%s(%d)", GET_PARAM_CMD(param)->cmd_name, tabs);
    else
        printf("-->%s(%d)", GET_LEAF_TYPE_STR(param), tabs);

    if(IS_PARAM_CMD(param)){
        cmd = GET_PARAM_CMD(param);
        /*Skip dumping the 'no' branch of the cmd tree*/
        if(strncmp(cmd->cmd_name, "no",2) == 0)
            return;
    }

    for(; i < MAX_OPTION_SIZE; i++){
        if(param->options[i]){
            printf("\n");
            _dump_one_cmd(param->options[i], ++tabs);
            --tabs;
        }
        else
            break;
    }
}

void
dump_cmd_tree(){
    _dump_one_cmd(&root, 0);
    printf("\n");
}

extern 
void command_parser(void);

void
start_shell(void){
    command_parser();
}

extern char console_name[TERMINAL_NAME_SIZE];

void
set_console_name(const char *cons_name){
    strncpy(console_name, cons_name, TERMINAL_NAME_SIZE);
    console_name[TERMINAL_NAME_SIZE -1] = '\0';    
}
