/*
 * =====================================================================================
 *
 *       Filename:  parser.c
 *
 *    Description:  Command parser
 *
 *        Version:  1.0
 *        Created:  Thursday 03 August 2017 04:06:50  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "string_util.h"
#include "cmdtlv.h"


extern param_t root;
extern leaf_type_handler leaf_handler_array[LEAF_MAX];
extern ser_buff_t *tlv_buff;
char console_name[32];


static void
place_console(char new_line){
    if(new_line)
        printf("\n");
    printf("%s> ", console_name);
}

static char cons_input_buffer[2048];

typedef enum{
    COMPLETE,
    ERROR,
    INVALID_LEAF,
    USER_INVALID_LEAF,
    CMD_NOT_FOUND,
    QUESTION_MARK,
    UNKNOWN
} CMD_PARSE_STATUS;

#if 1
static param_t*
find_matching_param(param_t **options, const char *cmd_name){

    int i = 0, leaf_index = -1;
    for(; options[i] && i < MAX_OPTION_SIZE; i++){
        if(IS_PARAM_LEAF(options[i])){
            leaf_index = i;
            continue;
        }

        if(is_cmd_string_match(options[i], cmd_name) == 0)
            return options[i];
    }

    if(leaf_index >= 0)
        return options[leaf_index];

     return NULL;
}
#endif

/*-----------------------------------------------------------------------------
 *  Return 0 on Success, -1 on failure
 *  if Success, param is the pointer to the leaf of the cmd tree
 *  if Failure, param is the pointer to the mismatch patch of the cmd tree
 *-----------------------------------------------------------------------------*/

static tlv_struct_t tlv;

static int
build_tlv_buffer(char **tokens, size_t token_cnt, param_t **out_param, op_mode enable_or_disable){ 
    
    int i = 0;
    param_t *param = &root;
    param_t *parent = NULL;
    CMD_PARSE_STATUS status = COMPLETE;
    memset(&tlv, 0, sizeof(tlv_struct_t));
         
    for(; i < token_cnt; i++){
        parent = param;
        if(strncmp(*(tokens +i), "?", 1) == 0){
            status = QUESTION_MARK;
            break;
        }

        param = find_matching_param(get_child_array_ptr(param), *(tokens +i));
        if(param){
            if(IS_PARAM_LEAF(param)){
                /*printf("token[%d] = %s Not found in cmd tree, leaf = %s\n", 
                        i, *(tokens +i), GET_LEAF_TYPE_STR(param));*/

                /*If it is a leaf, collect the leaf value and continue to parse. Below function performs
                 * basic standard sanity checks on the leaf value input by the user */ 

                if(INVOKE_LEAF_LIB_VALIDATION_CALLBACK(param, *(tokens +i)) ==0){

                    /*Standard librray checks have passed, now call user validation callback function*/
                    if(INVOKE_LEAF_USER_VALIDATION_CALLBACK(param, *(tokens +i)) == 0){
                        /*Now collect this leaf information into TLV*/
                        prepare_tlv_from_leaf(GET_PARAM_LEAF(param), (&tlv));
                        put_value_in_tlv((&tlv), *(tokens +i));
                        collect_tlv(tlv_buff, &tlv);
                        memset(&tlv, 0, sizeof(tlv_struct_t));
                        continue;
                    }
                    else{
                        status = USER_INVALID_LEAF;
                    }
                }
                else{
                    /*If leaf is not a valid value, terminate the command parsing immediately*/
                    status = INVALID_LEAF;
                }
                break;
            }
            else{
                /*printf("token[%d] = %s found in cmd tree\n", i, *(tokens +i));*/
                continue;
            }
        }

        status = CMD_NOT_FOUND;
        break;
    }

    switch(status){
        case QUESTION_MARK:
            /*print all childs of parent*/
            {
                i = 0;
                cmd_t *cmd = NULL;
                leaf_t *leaf = NULL;

                if(IS_PARAM_CMD(parent))
                    cmd = GET_PARAM_CMD(parent);
                else
                    leaf = GET_PARAM_LEAF(parent);
                
                //printf("\n");            
                if(cmd){
                    for(; i < MAX_OPTION_SIZE; i++){
                        if(cmd->options[i]){
                            if(IS_PARAM_CMD(cmd->options[i])){
                                printf("nxt cmd  -> %-31s   |   %s\n", GET_CMD_NAME(cmd->options[i]), GET_CMD_HELP_STRING(cmd->options[i]));
                                continue;
                            }
                            printf("nxt leaf -> %-32s  |   %s\n", GET_LEAF_TYPE_STR(cmd->options[i]), GET_LEAF_HELP_STRING(cmd->options[i]));
                            continue;
                        }
                        break;
                    }
                }
                else{
                    for(; i < MAX_OPTION_SIZE; i++){
                        if(leaf->options[i]){
                            if(IS_PARAM_CMD(leaf->options[i])){
                                printf("nxt cmd  -> %-31s   |   %s\n", GET_CMD_NAME(leaf->options[i]), GET_CMD_HELP_STRING(leaf->options[i]));
                                continue;
                            }
                            printf("nxt leaf -> %-32s  |   %s\n", GET_LEAF_TYPE_STR(leaf->options[i]), GET_LEAF_HELP_STRING(leaf->options[i]));
                            continue;
                        }
                        break;
                    }

                }
            } 
            return -1;
        case CMD_NOT_FOUND:
            printf("Error : Following Token not registered : %s\n", *(tokens +i));
            return -1;
        case INVALID_LEAF:
            printf("Error : Following leaf value could not be validated : %s, Expected Data type = %s\n", 
                            *(tokens +i), GET_LEAF_TYPE_STR(param));
            return -1;
        case COMPLETE:
            printf("Parse Success.\n");
            INVOKE_APPLICATION_CALLBACK_HANDLER(param, tlv_buff, enable_or_disable);
            break;
            case USER_INVALID_LEAF:
                printf("Error : User validation has failed\n");
                return -1;
        default:
            printf("FATAL : Unknown case fall\n");
    }
    return 0;
}

static void
parse_input_cmd(char *input, unsigned int len){
    
        int i = 0, tok_no = 0;
        char** tokens = NULL;
        param_t *param = NULL;
        size_t token_cnt = 0;

        tokens = str_split(input, ' ', &token_cnt);
        if(!tokens)
            return;

        /*Walk the cmd tree now and build the TLV buffer of leavf values, if any*/
        if(strncmp(tokens[0], "config", MIN(strlen("config"), strlen(tokens[0]))) == 0)
            build_tlv_buffer(tokens, token_cnt, &param, CONFIG_ENABLE);
        else if((strncmp(tokens[0], "no", 2) == 0))   
            build_tlv_buffer(tokens, token_cnt, &param, CONFIG_DISABLE);
        else 
            build_tlv_buffer(tokens, token_cnt, &param, OPERATIONAL); 
        free_tokens(tokens);
        reset_serialize_buffer(tlv_buff);
}


void
command_parser(void){

    place_console(1);
    while(1){
        if((fgets((char *)cons_input_buffer, sizeof(cons_input_buffer)-1, stdin) == NULL)){
            printf("error in reading from stdin\n");
            exit(EXIT_SUCCESS);
        }
    
        /*IF only enter is hit*/ 
        if(strlen(cons_input_buffer) == 1){
            cons_input_buffer[0]= '\0';
            place_console(0);
            continue; 
        }
        cons_input_buffer[strlen(cons_input_buffer) - 1] = '\0'; 
        parse_input_cmd(cons_input_buffer, strlen(cons_input_buffer));
        memset(cons_input_buffer, 0, sizeof(cons_input_buffer));
        place_console(1);
    }
}

void
enhanced_command_parser(void){
    

}

