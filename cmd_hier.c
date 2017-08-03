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

#include "cmd_hier.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "serialize.h"


#define TLV_MAX_BUFFER_SIZE 1048

param_t root;
leaf_type_handler leaf_handler_array[LEAF_MAX];
ser_buff_t *tlv_buff;

static param_t*
get_param_from_cmd(cmd_t *cmd){
    param_t *param = calloc(1, sizeof(param_t));
    param->param_type = CMD;
    param->param.cmd = cmd;
    return param;
}

static param_t*
get_param_from_leaf(leaf_t *leaf){
    param_t *param = calloc(1, sizeof(param_t));
    param->param_type = LEAF;
    param->param.leaf = leaf;
    return param;
}

static void*
int_handler(void *arg){
    return NULL;
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
    root.param.cmd = calloc(1, sizeof(cmd_t));
    strncpy(root.param.cmd->cmd_name, "ROOT", CMD_NAME_SIZE-1);
    root.param.cmd->cmd_name[CMD_NAME_SIZE-1] = '\0';
    leaf_handler_array[INT] = int_handler;
    init_serialized_buffer_of_defined_size(&tlv_buff, TLV_MAX_BUFFER_SIZE);
}

void
static_register_command_after_command(cmd_t *parent, cmd_t *child){
    
    int i = 0;
    if(!parent)
        parent = root.param.cmd;

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



cmd_t*
dynamic_register_command_after_command(cmd_t *parent, 
                         const char *cmd_name, 
                         cmd_callback callback){

    int i = 0;
    cmd_t *child = NULL;

    if(!parent)
        parent = root.param.cmd;

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
    if(IS_PARAM_CMD(param)){
        printf("-->%s", GET_PARAM_CMD(param)->cmd_name);
    }
    else{
        printf("-->%s", GET_LEAF_TYPE_STR(param));
    }
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

int
main(int argc, char **argv){
    init_libcli();

    /*Level 0*/
    static cmd_t show = {"show", 0, NULL_OPTIONS};
    static_register_command_after_command(0, &show);
    
    static cmd_t debug = {"debug", 0, NULL_OPTIONS};
    static_register_command_after_command(0, &debug);


    static cmd_t config = {"config", 0, NULL_OPTIONS};
    static_register_command_after_command(0, &config);


    /*Level 1*/
    static cmd_t ip = {"ip", 0, NULL_OPTIONS};
    static_register_command_after_command(&show, &ip);


    static cmd_t ipv6 = {"ipv6", 0, NULL_OPTIONS};
    static_register_command_after_command(&show, &ipv6);

    /*Level 2*/
    static cmd_t igmp = {"igmp", 0, NULL_OPTIONS};
    static_register_command_after_command(&ip, &igmp);

    cmd_t *show_ip_pim = dynamic_register_command_after_command(&ip, "pim", 0);

    /*Level 3*/
    static cmd_t groups = {"groups", 0, NULL_OPTIONS};
    static_register_command_after_command(&igmp, &groups);

    dynamic_register_command_after_command(&igmp, "statistics", 0);
    cmd_t *show_ip_pim_groups = dynamic_register_command_after_command(show_ip_pim, "groups", 0);
    cmd_t *show_ip_pim_mcache = dynamic_register_command_after_command(show_ip_pim, "mcache", 0);

    /*Level 4*/
    static cmd_t show_ip_igmp_groups_vlan = {"vlan", 0, NULL_OPTIONS};
    static_register_command_after_command(&groups, &show_ip_igmp_groups_vlan);

    /*Level 5*/
    static leaf_t vlan_id = {INT, "10", 0, NULL_OPTIONS};
    static_register_leaf_after_command(&show_ip_igmp_groups_vlan, &vlan_id);
     
    //dump_cmd_tree(); 
    command_parser();

    return 0;
}
