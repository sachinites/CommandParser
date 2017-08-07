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
#include "cliconst.h"
#include "css.h"

extern param_t root;
extern leaf_type_handler leaf_handler_array[LEAF_MAX];
extern ser_buff_t *tlv_buff;
char console_name[TERMINAL_NAME_SIZE];

void
place_console(char new_line){
    if(new_line)
        printf("\n");
    printf("%s $ ", console_name);
}

static char cons_input_buffer[CONS_INPUT_BUFFER_SIZE];
static char last_command_input_buffer[CONS_INPUT_BUFFER_SIZE];


typedef enum{
    COMPLETE,
    ERROR,
    INVALID_LEAF,
    USER_INVALID_LEAF,
    CMD_NOT_FOUND,
    QUESTION_MARK,
    INCOMPLETE_COMMAND,
    MULTIPLE_MATCHING_COMMANDS,
    UNKNOWN
} CMD_PARSE_STATUS;

char *
get_last_command(){
    return last_command_input_buffer;
}



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

/*-----------------------------------------------------------------------------
 *  Return 0 on Success, -1 on failure
 *  if Success, param is the pointer to the leaf of the cmd tree
 *  if Failure, param is the pointer to the mismatch patch of the cmd tree
 *-----------------------------------------------------------------------------*/

static tlv_struct_t tlv;

static CMD_PARSE_STATUS
build_tlv_buffer(char **tokens, 
                 size_t token_cnt, 
                 op_mode enable_or_disable){ 

    int i = 0; 
    param_t *param = get_cmd_tree_cursor();
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

                /*If it is a leaf, collect the leaf value and continue to parse. Below function performs
                 * basic standard sanity checks on the leaf value input by the user */ 
                if(INVOKE_LEAF_LIB_VALIDATION_CALLBACK(param, *(tokens +i)) == VALIDATION_SUCCESS){

                    /*Standard librray checks have passed, now call user validation callback function*/
                    if(INVOKE_LEAF_USER_VALIDATION_CALLBACK(param, *(tokens +i)) == VALIDATION_SUCCESS){
                        /*Now collect this leaf information into TLV*/
                        prepare_tlv_from_leaf(GET_PARAM_LEAF(param), (&tlv));
                        put_value_in_tlv((&tlv), *(tokens +i));
                        strncpy(GET_LEAF_VALUE_PTR(param), *(tokens +i), MIN(strlen(*(tokens +i)), LEAF_VALUE_HOLDER_SIZE));
                        GET_LEAF_VALUE_PTR(param)[strlen(*(tokens +i))] = '\0';
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

    if(status == COMPLETE){
        if(!IS_APPLICATION_CALLBACK_HANDLER_REGISTERED(param))
            status = INCOMPLETE_COMMAND;
    }

    switch(status){
        case MULTIPLE_MATCHING_COMMANDS:
            break;
        case QUESTION_MARK:
            {
                i = 0;
                
                if(IS_APPLICATION_CALLBACK_HANDLER_REGISTERED(parent))
                    printf("<Enter>\n");

                    for(; i < MAX_OPTION_SIZE; i++){
                        if(parent->options[i]){
                            if(IS_PARAM_CMD(parent->options[i])){
                                printf(ANSI_COLOR_MAGENTA "nxt cmd  -> %-31s   |   %s\n" ANSI_COLOR_RESET, GET_CMD_NAME(parent->options[i]), GET_PARAM_HELP_STRING(parent->options[i]));
                                continue;
                            }
                            printf(ANSI_COLOR_CYAN "nxt leaf -> %-32s  |   %s\n" ANSI_COLOR_RESET, GET_LEAF_TYPE_STR(parent->options[i]), GET_PARAM_HELP_STRING(parent->options[i]));
                            continue;
                        }
                        break;
                    }
            } 
            break;
        case CMD_NOT_FOUND:
            printf(ANSI_COLOR_RED "Error : Following Token not registered : %s\n" ANSI_COLOR_RESET, *(tokens +i));
            break;
        case INVALID_LEAF:
            printf(ANSI_COLOR_RED "Error : Following leaf value could not be validated : %s, Expected Data type = %s\n" ANSI_COLOR_RESET, *(tokens +i), GET_LEAF_TYPE_STR(param));
            break;
        case COMPLETE:
            printf(ANSI_COLOR_GREEN "Parse Success.\n" ANSI_COLOR_RESET);
            INVOKE_APPLICATION_CALLBACK_HANDLER(param, tlv_buff, enable_or_disable);
            break;
        case USER_INVALID_LEAF:
            printf(ANSI_COLOR_YELLOW "Error : User validation has failed\n" ANSI_COLOR_RESET);
            break;
        case INCOMPLETE_COMMAND:
            printf(ANSI_COLOR_YELLOW "Error : Incomplete Command\n" ANSI_COLOR_RESET);
            break;
        default:
            printf(ANSI_COLOR_RED "FATAL : Unknown case fall\n" ANSI_COLOR_RESET);
    }
    return status;;
}

void
parse_input_cmd(char *input, unsigned int len){
    
        char** tokens = NULL;
        size_t token_cnt = 0;
        CMD_PARSE_STATUS status = COMPLETE;
         
        tokens = str_split(input, ' ', &token_cnt);
        if(!tokens)
            return;

        /*Walk the cmd tree now and build the TLV buffer of leavf values, if any*/

        /*Honour the negate command triggered from TOP level or with in condif branch*/
        if((strncmp(tokens[0], "no", 2) == 0) && token_cnt > 1){ 
            if((get_current_branch_hook(get_cmd_tree_cursor()) == libcli_get_config_hook())
                        || (get_cmd_tree_cursor() == &root))
                status = build_tlv_buffer(&tokens[1], token_cnt -1, CONFIG_DISABLE);
             else
                 printf(ANSI_COLOR_YELLOW "Info : Command Negation is supported only for config commands\n" ANSI_COLOR_RESET);
        }

        else if((strncmp(tokens[0], "end", strlen("end")) == 0) && (token_cnt == 1))
            goto_top_of_cmd_tree(get_cmd_tree_cursor());

        else if((strncmp(tokens[0], "exit", strlen("exit")) == 0) && (token_cnt == 1))
            go_one_level_up_cmd_tree(get_cmd_tree_cursor());

        /* Honour the config command from TOP or from within config branch*/
        else if((strncmp(tokens[0], "config", MIN(strlen("config"), strlen(tokens[0]))) == 0) ||
                ((get_cmd_tree_cursor() != &root) && 
                (get_current_branch_hook(get_cmd_tree_cursor()) == libcli_get_config_hook())))
            
                 status = build_tlv_buffer(tokens, token_cnt, CONFIG_ENABLE);
        else 
            status = build_tlv_buffer(tokens, token_cnt, OPERATIONAL); 

        free_tokens(tokens);

        if(is_user_in_cmd_mode())
            restore_checkpoint_serialize_buffer(tlv_buff);
        else
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

        if(strncmp(cons_input_buffer, "repeat", strlen(cons_input_buffer)) == 0){
            memset(cons_input_buffer, 0, CONS_INPUT_BUFFER_SIZE);
            place_console(1);
            continue;
        }
        memset(last_command_input_buffer, 0, CONS_INPUT_BUFFER_SIZE);

        memcpy(last_command_input_buffer, cons_input_buffer, strlen(cons_input_buffer));

        last_command_input_buffer[strlen(last_command_input_buffer)] = '\0';

        memset(cons_input_buffer, 0, CONS_INPUT_BUFFER_SIZE);

        place_console(1);
    }
}

void
enhanced_command_parser(void){
    

}

