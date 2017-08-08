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
#include "string_util.h"
#include "css.h"
#include <signal.h>

param_t root;
leaf_type_handler leaf_handler_array[LEAF_MAX];
ser_buff_t *tlv_buff;
param_t *cmd_tree_cursor = NULL;

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

/* Cursor functions*/
void
reset_cmd_tree_cursor(){
    cmd_tree_cursor = &root;
    reset_serialize_buffer(tlv_buff);
}

void
set_cmd_tree_cursor(param_t *param){
    assert(param);
    cmd_tree_cursor = param;
}

param_t *
get_cmd_tree_cursor(){
    return cmd_tree_cursor;
}

int
is_user_in_cmd_mode(){
        return (get_cmd_tree_cursor() != &root);
}

extern char *
get_last_command();

extern char console_name[TERMINAL_NAME_SIZE];

extern  ser_buff_t *file_read_buffer;

extern void
parse_input_cmd(char *input, unsigned int len);

extern void
place_console(char new_line);

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


static void
ctrlC_signal_handler(int sig){
    printf("Ctrl-C pressed\n");
    printf("Bye Bye\n");
    system("clear");
    exit(0);
}

void 
init_libcli(){

    init_param(&root, CMD, "ROOT", 0, 0, INVALID, 0, "ROOT");
    
    /*Intialised serialized buffer to collect leaf values in TLV format*/
    init_serialized_buffer_of_defined_size(&tlv_buff, TLV_MAX_BUFFER_SIZE);
    init_serialized_buffer_of_defined_size(&file_read_buffer, TLV_MAX_BUFFER_SIZE);

    reset_cmd_tree_cursor();

    /*Leaf datatypes standard Validation callbacks registration*/
    leaf_handler_array[INT]     = int_validation_handler;
    leaf_handler_array[STRING]  = string_validation_handler;
    leaf_handler_array[IPV4]    = ipv4_validation_handler;
    leaf_handler_array[IPV6]    = ipv6_validation_handler;
    leaf_handler_array[FLOAT]   = float_validation_handler;


    set_device_name("root@juniper");

    /*Registering Zero level default command hooks*/
    /*Show hook*/
    init_param(&show, CMD, "show", 0, 0, INVALID, 0, "show");
    libcli_register_param(&root, &show);

    /*show history*/
    static param_t history;
    init_param(&history, CMD, "history", show_history_callback, 0, INVALID, INVALID, "Command history");
    libcli_register_param(&show, &history);
    
    static param_t no_of_commands;
    init_param(&no_of_commands, LEAF, "N", show_history_callback, 0, INT, "N", "No Of Commands to fetch");  
    libcli_register_param(&history, &no_of_commands);

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

    /* Install clear command "cls"*/
    static param_t cls;
    init_param(&cls, CMD, "cls", clear_screen_handler, 0, INVALID, 0, "clear screen");
    libcli_register_param(0, &cls);

    /* Resgister CTRL-C signal handler*/
    signal(SIGINT, ctrlC_signal_handler);
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
set_device_name(const char *cons_name){
    
    char** tokens = NULL;
    size_t token_cnt = 0;    

    assert(cons_name);

    if(strlen(console_name))
        tokens = str_split(console_name, '>', &token_cnt);
    
    sprintf(console_name, "%s>", cons_name);
    
    if(token_cnt > 1){
        strcat(console_name, " ");
        strcat(console_name, tokens[1]);
    }

    if(tokens)
        free_tokens(tokens);
}

int
is_param_mode_capable(param_t *param){

    int i = 0;
    assert(param);

    if(param == &root)
        return -1;

    if(param->options[0] == NULL)
        return -1;
          
    for(; i < MAX_OPTION_SIZE; i++){

        if(IS_PARAM_LEAF(param->options[i]))
            continue;

        if(strncmp(GET_CMD_NAME(param->options[i]), "*", 1) == 0)
            return 0;

    }
    return -1;
}

int
insert_moding_capability(param_t *param){

    assert(param);
    if(param == &root)
        return -1;
    /*Do not insert mode capability in mode param itself,
     * avoid chicken and egg problem here. Also, negate commands
     * are also should not be mode capable*/

    if(is_mode_exception_cmd(param) == 0)
        return -1;

    /*Bail if param is already mode capable*/
    if(is_param_mode_capable(param) == 0)
        return -1;

    param_t * mode_param = calloc(1, sizeof(param_t));
    init_param(mode_param, CMD, "*", mode_enter_callback , 0, INVALID, 0, "ENTER MODE");
    param->options[0] = mode_param;
    mode_param->parent = param;
    return 0;
}

void
libcli_register_param(param_t *parent, param_t *child){
    int i = 0;
    if(!parent)
        parent = &root;
        
    for(; i < MAX_OPTION_SIZE; i++){
        if(parent->options[i])
            continue;

        if(insert_moding_capability(parent) == 0){
            i++;
            assert(i != MAX_OPTION_SIZE);
        }
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

    if(IS_PARAM_CMD(param)){
        /*Skip dumping the 'no' branch of the cmd tree*/
        if(strncmp(GET_CMD_NAME(param), "no",2) == 0)
            return;
    }

    PRINT_TABS(tabs);

    if(IS_PARAM_CMD(param)){
        /*if(strncmp(GET_CMD_NAME(param), "*", 1) == 0)
            return;*/
        printf("-->%s(%d)", GET_PARAM_CMD(param)->cmd_name, tabs);
    }
    else
        printf("-->%s(%d)", GET_LEAF_TYPE_STR(param), tabs);

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
}

extern 
void command_parser(void);

extern 
void  enhanced_command_parser(void);

void
start_shell(void){
    //command_parser();
    enhanced_command_parser();
}


void
set_console_name(const char *cons_name){
    sprintf(console_name, "%s>", cons_name);
    console_name[TERMINAL_NAME_SIZE -1] = '\0';    
}

/* Command Mode implementation */

param_t*
get_current_branch_hook(param_t *current_param){
    assert(current_param);
    assert(current_param != &root);
    while(current_param->parent != &root){
        current_param = current_param->parent;
    }
    return current_param;;
}


/*-----------------------------------------------------------------------------
 *  This fn resets the current cmd tree cursor to root and flush the leaf value 
 *  present in the branch of tree from root to curr_cmd_tree_cursor
 *-----------------------------------------------------------------------------*/
void
goto_top_of_cmd_tree(param_t *curr_cmd_tree_cursor){

    char** tokens = NULL;
    size_t token_cnt = 0;

    assert(curr_cmd_tree_cursor);
    
    if(curr_cmd_tree_cursor == &root){
        printf(ANSI_COLOR_BLUE "Info : At Roof top Already\n" ANSI_COLOR_RESET);
        return;
    }

    do{
        if(IS_PARAM_CMD(curr_cmd_tree_cursor)){
            curr_cmd_tree_cursor = curr_cmd_tree_cursor->parent;
            continue;
        }
        memset(GET_LEAF_VALUE_PTR(curr_cmd_tree_cursor), 0, LEAF_VALUE_HOLDER_SIZE);
        curr_cmd_tree_cursor = curr_cmd_tree_cursor->parent;
    } while(curr_cmd_tree_cursor != &root);
    
    reset_cmd_tree_cursor();
    tokens = str_split(console_name, '>', &token_cnt);
    sprintf(console_name, "%s>", tokens[0]);
}

void
go_one_level_up_cmd_tree(param_t *curr_cmd_tree_cursor){

    char** tokens = NULL;
    size_t token_cnt = 0;

    assert(curr_cmd_tree_cursor);

    if(curr_cmd_tree_cursor == &root){
        printf(ANSI_COLOR_BLUE "Info : At Roof top Already\n" ANSI_COLOR_RESET);
        return;
    }

    if(IS_PARAM_LEAF(curr_cmd_tree_cursor)){
        memset(GET_LEAF_VALUE_PTR(curr_cmd_tree_cursor), 0, LEAF_VALUE_HOLDER_SIZE);
        serialize_buffer_skip(tlv_buff, -1 * sizeof(tlv_struct_t));/*Rewind*/
        mark_checkpoint_serialize_buffer(tlv_buff);
    }

     set_cmd_tree_cursor(curr_cmd_tree_cursor->parent);

     if(get_cmd_tree_cursor() == &root){
        tokens = str_split(console_name, '>', &token_cnt);
        sprintf(console_name, "%s>", tokens[0]);
        reset_serialize_buffer(tlv_buff);
        return;
     }
    
     build_mode_console_name(get_cmd_tree_cursor());
}


/*-----------------------------------------------------------------------------
 *  Build new console name when entered into MODE from root to dst_parm(incl)
 *-----------------------------------------------------------------------------*/
void
build_mode_console_name(param_t *dst_param){

    assert(dst_param);
    assert(dst_param != &root);/*This fn should not be called for root*/

    int i = MAX_CMD_TREE_DEPTH -1;
    size_t token_cnt = 0;
    
    char** tokens = NULL;
    char *append_string = NULL;

    static char cmd_names[MAX_CMD_TREE_DEPTH][LEAF_VALUE_HOLDER_SIZE];
    char *admin_set_console_name = NULL;

    tokens = str_split(console_name, '>', &token_cnt);
    admin_set_console_name = tokens[0];
    sprintf(console_name, "%s> ", admin_set_console_name);
    free_tokens(tokens);
    
    do{
        assert(i != -1); 
        if(IS_PARAM_CMD(dst_param))
            append_string = GET_CMD_NAME(dst_param);
        else
            append_string = GET_LEAF_VALUE_PTR(dst_param);

        strncpy(cmd_names[i], append_string, strlen(append_string));
        i--;
        dst_param = dst_param->parent;
    }while(dst_param != &root);

    for(i = i+1; i < MAX_CMD_TREE_DEPTH -1; i++){
        strcat(console_name, cmd_names[i]);
        strcat(console_name, "-");
    }

    strcat(console_name, cmd_names[i]);
    memset(cmd_names, 0, MAX_CMD_TREE_DEPTH * LEAF_VALUE_HOLDER_SIZE);
}

/*Source and Destination command MUST be in the same branch AND
 *  * Source must be at higher level as compared to Destination*/
void
build_cmd_tree_leaves_data(ser_buff_t *tlv_buff,/*Output serialize buffer*/
        param_t *src_param, /*Source command*/
        param_t *dst_param){/*Destination command*/

    assert(tlv_buff);
    assert(src_param);
    assert(dst_param);

    tlv_struct_t tlv, *tlv_temp = NULL;
    unsigned int tlv_units = 0, i = 0, j = 0;

    memset(&tlv, 0, sizeof(tlv_struct_t));
    reset_serialize_buffer(tlv_buff);

    while(dst_param != src_param){
        if(IS_PARAM_CMD(dst_param)){
            dst_param = dst_param->parent;
            continue;
        }

        prepare_tlv_from_leaf(GET_PARAM_LEAF(dst_param), (&tlv));
        put_value_in_tlv((&tlv), GET_LEAF_VALUE_PTR(dst_param)); 
        collect_tlv(tlv_buff, &tlv);
        memset(&tlv, 0, sizeof(tlv_struct_t));

        dst_param = dst_param->parent;
    }

    if(IS_PARAM_LEAF(dst_param)){
        prepare_tlv_from_leaf(GET_PARAM_LEAF(dst_param), (&tlv));
        put_value_in_tlv((&tlv), GET_LEAF_VALUE_PTR(dst_param)); 
        collect_tlv(tlv_buff, &tlv);
    }

    /*Now reverse the TLV buffer*/
    if(get_serialize_buffer_size(tlv_buff) < (sizeof(tlv_struct_t) << 1)){
        return;
    }

    tlv_units = get_serialize_buffer_size(tlv_buff)/sizeof(tlv_struct_t);
    tlv_temp = (tlv_struct_t *)(tlv_buff->b);
    j = tlv_units -1;

    for(; i < (tlv_units >> 1); i++, j--){
        swap_tlv_units(tlv_temp+i, tlv_temp +j);
    }
}

