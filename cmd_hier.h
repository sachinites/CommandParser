/*
 * =====================================================================================
 *
 *       Filename:  cmd_hier.h
 *
 *    Description:  This file defines the structure for maintaining cmd hierarchy
 *
 *        Version:  1.0
 *        Created:  Thursday 03 August 2017 02:08:10  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 * =====================================================================================
 */

#ifndef __CMD_HIER__
#define __CMD_HIER__

#include <string.h>
#include <assert.h>

/* If you modify the below Macro, pls put 
 * as many zeroes in NULL_OPTIONS expansion as well*/
#define MAX_OPTION_SIZE 30
#define NULL_OPTIONS    {0,0,0,0,0,0,0,0,0,0,\
                         0,0,0,0,0,0,0,0,0,0,\
                         0,0,0,0,0,0,0,0,0,0,}



#define CMD_NAME_SIZE           32
#define LEAF_VALUE_HOLDER_SIZE  64
#define LEAF_ID_SIZE            32
#define LEAF_HELP_STRING_SIZE   64
#define CMD_HELP_STRING_SIZE    LEAF_HELP_STRING_SIZE


typedef enum{
    CONFIG_DISABLE,
    CONFIG_ENABLE
} op_mode;

typedef struct serialized_buffer ser_buff_t;
typedef int (*cmd_callback)(ser_buff_t *tlv_buf);
typedef int (*user_validation_callback)(char *leaf_value);


typedef enum{
    INT,
    STRING,
    IPV4,
    FLOAT,
    IPV6,
    LEAF_MAX
} leaf_type_t;


typedef struct _param_t_ param_t;

typedef struct cmd{
    char cmd_name[CMD_NAME_SIZE];
    cmd_callback callback;
    char help[CMD_HELP_STRING_SIZE];
    param_t *options[MAX_OPTION_SIZE];
} cmd_t;

typedef struct leaf{
    leaf_type_t leaf_type;
    char value_holder[LEAF_VALUE_HOLDER_SIZE];
    cmd_callback callback;
    user_validation_callback user_validation_cb_fn;
    char leaf_id[LEAF_ID_SIZE];/*Within a single command, it should be unique*/
    char help[LEAF_HELP_STRING_SIZE];
    param_t *options[MAX_OPTION_SIZE];
} leaf_t;

typedef int (*leaf_type_handler)(leaf_t *leaf, char *value_passed);

typedef enum{
    CMD,
    LEAF
} param_type_t;

typedef union _param_t{
    cmd_t *cmd;
    leaf_t *leaf;
} _param_t;

struct _param_t_{
    param_type_t param_type;
    _param_t cmd_type;
};

void
init_libcli();

void
set_console_name(const char *cons_name);

/*Command Registration*/
void 
static_register_command_after_command(cmd_t *parent, cmd_t *child);
void 
static_register_command_after_leaf(leaf_t *parent, cmd_t *child);
cmd_t *
dynamic_register_command_after_command(cmd_t *parent, const char *cmd_name, cmd_callback callback);
cmd_t *
dynamic_register_command_after_leaf(leaf_t *parent, const char *cmd_name, cmd_callback callback);

/*Leaf registration*/
void
static_register_leaf_after_command(cmd_t *parent, leaf_t *child);
void
static_register_leaf_after_leaf(leaf_t *parent, leaf_t *child);
leaf_t *
dynamic_register_leaf_after_command(cmd_t *parent, leaf_type_t leaf_type, 
                                    const char *def_leaf_value, cmd_callback callback);
leaf_t *
dynamic_register_leaf_after_leaf(leaf_t *parent, leaf_type_t leaf_type,
                                 const char *def_leaf_value, cmd_callback callback);

char*
get_str_leaf_type(leaf_type_t leaf_type);

void
dump_cmd_tree();

void
start_shell(void);


#define MIN(a,b)    (a < b ? a : b)

#define GET_PARAM_CMD(param)    (param->cmd_type.cmd)
#define GET_PARAM_LEAF(param)   (param->cmd_type.leaf)
#define IS_PARAM_CMD(param)     (param->param_type == CMD)
#define IS_PARAM_LEAF(param)    (param->param_type == LEAF)
#define GET_LEAF_TYPE_STR(param)    (get_str_leaf_type(GET_PARAM_LEAF(param)->leaf_type))
#define GET_LEAF_VALUE_PTR(param)   (GET_PARAM_LEAF(param)->value_holder)
#define GET_LEAF_TYPE(param)        (GET_PARAM_LEAF(param)->leaf_type)
#define GET_CMD_NAME(param)         (GET_PARAM_CMD(param)->cmd_name)
#define GET_LEAF_HELP_STRING(param) (GET_PARAM_LEAF(param)->help)
#define GET_CMD_HELP_STRING(param)  (GET_PARAM_CMD(param)->help)
#define GET_LEAF_ID(param)          (GET_PARAM_LEAF(param)->leaf_id)

#define IS_LEAF_USER_VALIDATION_CALLBACK_REGISTERED(param)  \
                    (param->cmd_type.leaf->user_validation_cb_fn)
#define _INVOKE_LEAF_USER_VALIDATION_CALLBACK(param, arg) \
                    (param->cmd_type.leaf->user_validation_cb_fn(arg))
#define INVOKE_LEAF_LIB_VALIDATION_CALLBACK(param, arg) \
                    (leaf_handler_array[GET_LEAF_TYPE(param)](GET_PARAM_LEAF(param), arg))

#define INVOKE_APPLICATION_CALLBACK_HANDLER(param, arg)                    \
    if(IS_PARAM_CMD(param) && param->cmd_type.cmd->callback)               \
        param->cmd_type.cmd->callback(arg);                                \
    else if(IS_PARAM_LEAF(param) && param->cmd_type.leaf->callback)        \
        param->cmd_type.leaf->callback(arg); 
                                      
static inline int
is_cmd_string_match(param_t *param, const char *str){
    return (strncmp(param->cmd_type.cmd->cmd_name, 
            str, 
            strlen(str)));        
}

static inline param_t **
get_child_array_ptr(param_t *param){
    if(IS_PARAM_CMD(param)){
        return &param->cmd_type.cmd->options[0];
    }
    else{
        return &param->cmd_type.leaf->options[0];
    }
}

static inline int
INVOKE_LEAF_USER_VALIDATION_CALLBACK(param_t *param, char *leaf_value) {

    assert(param);
    assert(leaf_value);

    /*If validation fn is not registered, then validation is assumed to be passed*/
    if(!IS_LEAF_USER_VALIDATION_CALLBACK_REGISTERED(param))
        return 0;

    return _INVOKE_LEAF_USER_VALIDATION_CALLBACK(param, leaf_value);
}

#define PRINT_TABS(n)     \
do{                       \
   unsigned short _i = 0; \
   for(; _i < n; _i++)    \
       printf("  ");      \
} while(0);


#endif
