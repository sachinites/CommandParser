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

#define TLV_MAX_BUFFER_SIZE 1048

param_t root;
leaf_type_handler leaf_handler_array[LEAF_MAX];
ser_buff_t *tlv_buff;

/*Default zero level commands hooks. Application must
 * extern below command hooks*/
cmd_t show;
cmd_t debug;
cmd_t config;



static param_t*
get_param_from_cmd(cmd_t *cmd){
    param_t *param = calloc(1, sizeof(param_t));
    param->param_type = CMD;
    param->cmd_type.cmd = cmd;
    return param;
}

static param_t*
get_param_from_leaf(leaf_t *leaf){
    param_t *param = calloc(1, sizeof(param_t));
    param->param_type = LEAF;
    param->cmd_type.leaf = leaf;
    return param;
}

/*Default validation handlers for Data types*/

static int
int_validation_handler(leaf_t *leaf, char *value_passed){
    /*printf("%s is called for leaf type = %s, leaf value = %s\n", __FUNCTION__, 
                            get_str_leaf_type(leaf->leaf_type), value_passed);*/
    return 0;
}


static int
string_validation_handler(leaf_t *leaf, char *value_passed){
    /*printf("%s is called for leaf type = %s, leaf value = %s\n", __FUNCTION__, 
                            get_str_leaf_type(leaf->leaf_type), value_passed);*/
    return 0;
}


static int
ipv4_validation_handler(leaf_t *leaf, char *value_passed){
    /*printf("%s is called for leaf type = %s, leaf value = %s\n", __FUNCTION__, 
                            get_str_leaf_type(leaf->leaf_type), value_passed);*/
    return 0;
}


static int
ipv6_validation_handler(leaf_t *leaf, char *value_passed){
    /*printf("%s is called for leaf type = %s, leaf value = %s\n", __FUNCTION__, 
                            get_str_leaf_type(leaf->leaf_type), value_passed);*/
    return 0;
}


static int
float_validation_handler(leaf_t *leaf, char *value_passed){
    /*printf("%s is called for leaf type = %s, leaf value = %s\n", __FUNCTION__, 
                            get_str_leaf_type(leaf->leaf_type), value_passed);*/
    return 0;
}


/*Default Command Handlers for Default Commands*/
static int
config_console_name_handler(ser_buff_t *b){
    
    tlv_struct_t *tlv = NULL;
    int i = 0;

    TLV_LOOP(b, tlv, i){
        set_console_name(tlv->value);
    }
    return 0;
}

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
        return "Unknown";
    }
}

void 
init_libcli(){

    memset(&root, 0, sizeof(param_t));
    root.param_type = CMD;
    root.cmd_type.cmd = calloc(1, sizeof(cmd_t));
    strncpy(root.cmd_type.cmd->cmd_name, "ROOT", CMD_NAME_SIZE-1);
    root.cmd_type.cmd->cmd_name[CMD_NAME_SIZE-1] = '\0';

    /*Leavf Validation callbacks registration*/
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
    memset(&show, 0, sizeof(cmd_t));
    strncpy(show.cmd_name, "show", strlen("show"));
    show.cmd_name[strlen("show")] = '\0';
    show.callback = NULL;
    strncpy(show.help, "show commands", strlen("show commands"));
    show.help[strlen(show.help)] = '\0';
    static_register_command_after_command(0, &show);

    /*debug hook*/
    memset(&debug, 0, sizeof(cmd_t));
    strncpy(debug.cmd_name, "debug", strlen("debug"));
    debug.cmd_name[strlen("debug")] = '\0';
    debug.callback = NULL;
    strncpy(debug.help, "debug commands", strlen("debug commands"));
    debug.help[strlen(debug.help)] = '\0';
    static_register_command_after_command(0, &debug);

    /*configure hook*/
    memset(&config, 0, sizeof(cmd_t));
    strncpy(config.cmd_name, "config", strlen("config"));
    config.cmd_name[strlen("config")] = '\0';
    config.callback = NULL;
    strncpy(config.help, "configuration commands", strlen("configuration commands"));
    config.help[strlen(config.help)] = '\0';
    static_register_command_after_command(0, &config);

    /*config console name <new name>*/
    static cmd_t config_console = {"console", 0, "console", NULL_OPTIONS};
    static_register_command_after_command(&config, &config_console);

    static cmd_t config_console_name = {"name", 0, "name", NULL_OPTIONS};
    static_register_command_after_command(&config_console, &config_console_name);

    static leaf_t config_console_name_name = {STRING, "Abhishek", config_console_name_handler,
                                              0, "console-name", "Name of Console", NULL_OPTIONS};

    static_register_leaf_after_command(&config_console_name, &config_console_name_name);
}


void
static_register_command_after_command(cmd_t *parent, cmd_t *child){

    int i = 0;
    if(!parent)
        parent = root.cmd_type.cmd;

    for(; i < MAX_OPTION_SIZE; i++){
        if(parent->options[i])
            continue;

        parent->options[i] = get_param_from_cmd(child);
        return;
    }
    printf("%s() : Error : No space for new command : %s\n", __FUNCTION__, child->cmd_name);
}


void
static_register_leaf_after_command(cmd_t *parent, leaf_t *child){

    int i = 0;
    assert(parent);

    for(; i < MAX_OPTION_SIZE; i++){
        if(parent->options[i])
            continue;

        parent->options[i] = get_param_from_leaf(child);
        return;
    }

    printf("%s() : Error : No space for new command : \n", __FUNCTION__);

}

void
static_register_command_after_leaf(leaf_t *parent, cmd_t *child){

    int i = 0;
    assert(parent);

    for(; i < MAX_OPTION_SIZE; i++){
        if(parent->options[i])
            continue;

        parent->options[i] = get_param_from_cmd(child);
        return;
    }

    printf("%s() : Error : No space for new command : \n", __FUNCTION__);
}

cmd_t*
dynamic_register_command_after_command(cmd_t *parent, 
                         const char *cmd_name, 
                         cmd_callback callback){

    int i = 0;
    cmd_t *child = NULL;

    if(!parent)
        parent = root.cmd_type.cmd;

    for(; i < MAX_OPTION_SIZE; i++){
        if(parent->options[i])
            continue;
        child = calloc(1, sizeof(cmd_t));
        strncpy(child->cmd_name, cmd_name, CMD_NAME_SIZE -1);
        child->cmd_name[CMD_NAME_SIZE -1] = '\0';
        child->callback = callback;
        parent->options[i] = get_param_from_cmd(child);
        return child;
    }

    printf("%s() : Error : No space for new command : %s\n", __FUNCTION__, cmd_name);
    return NULL;
}

void
static_register_leaf_after_leaf(leaf_t *parent, leaf_t *child){
   
   int i = 0;
   assert(parent);
   
   for(; i < MAX_OPTION_SIZE; i++){
       if(parent->options[i])
           continue;
           
       parent->options[i] = get_param_from_leaf(child);
       return; 
   }

   printf("%s() : Error : No space for new command : \n", __FUNCTION__);
}
    

leaf_t *
dynamic_register_leaf_after_command(cmd_t *parent, leaf_type_t leaf_type,
                                    const char *def_leaf_value, cmd_callback callback
                                    ){
    
    int i = 0;
    leaf_t *child = NULL;

    assert(parent);
    
    for(; i < MAX_OPTION_SIZE; i++){
        if(parent->options[i])
            continue;

        child = calloc(1, sizeof(leaf_t));
        child->leaf_type = leaf_type;
        strncpy(child->value_holder, def_leaf_value, 63);
        child->callback = callback;

        parent->options[i] = get_param_from_leaf(child);
        return child;
    }

    printf("%s() : Error : No space for new leaf\n", __FUNCTION__);
    return NULL;
}



leaf_t *
dynamic_register_leaf_after_leaf(leaf_t *parent, leaf_type_t leaf_type,
                                 const char *def_leaf_value, cmd_callback callback){

    return NULL;
}


static void
_dump_one_cmd(param_t *param, unsigned short tabs){

    int i = 0;
    cmd_t *cmd = NULL;
    leaf_t *leaf = NULL;

    PRINT_TABS(tabs);

    if(IS_PARAM_CMD(param))
        printf("-->%s", GET_PARAM_CMD(param)->cmd_name);
    else
        printf("-->%s", GET_LEAF_TYPE_STR(param));

    if(IS_PARAM_CMD(param)){
        cmd = GET_PARAM_CMD(param);
        for(; i < MAX_OPTION_SIZE; i++){
            if(cmd->options[i]){
                printf("\n");
                _dump_one_cmd(cmd->options[i], ++tabs);
                --tabs;
            }
            else
                break;
        }
    }
    else/*If the param is a leaf*/
    {
        leaf = GET_PARAM_LEAF(param);
        for(; i < MAX_OPTION_SIZE; i++){
            if(leaf->options[i]){
                printf("\n");
                _dump_one_cmd(leaf->options[i], ++tabs);
                --tabs;
            }
            else
                break;
        }
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

extern char console_name[32];

void
set_console_name(const char *cons_name){
    strncpy(console_name, cons_name, 31);
    console_name[31] = '\0';    
}
