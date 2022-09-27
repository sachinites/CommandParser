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
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "string_util.h"
#include <errno.h>
#include "cmdtlv.h"
#include "cliconst.h"
#include "css.h"
#include "libcli.h"

extern param_t root;
extern leaf_type_handler leaf_handler_array[LEAF_MAX];
extern ser_buff_t *tlv_buff;
char console_name[TERMINAL_NAME_SIZE];
extern void
run_test_case(char *file_name, uint16_t tc_no);

static bool cmd_recording_enabled = true;
void parse_file(char *file_name) ;

static param_t*
array_of_possibilities[POSSIBILITY_ARRAY_SIZE];

void
place_console(char new_line){
    if(new_line)
        printf("\n");
    printf("%s $ ", console_name);
}

static char cons_input_buffer[CONS_INPUT_BUFFER_SIZE];
static char last_command_input_buffer[CONS_INPUT_BUFFER_SIZE];

static tlv_struct_t command_code_tlv;

char *
get_last_command(){
    return last_command_input_buffer;
}

param_t*
find_matching_param(param_t **options, const char *cmd_name){
    
    int i = 0, leaf_index = -1,
        j = 0,
        choice = -1;

    bool ex_match = false;

    memset(array_of_possibilities, 0, POSSIBILITY_ARRAY_SIZE * sizeof(param_t *));

    for(; options[i] && i <= CHILDREN_END_INDEX; i++){

        if(IS_PARAM_LEAF(options[i])){
            leaf_index = i;
            continue;
        }

        if (is_cmd_string_match(options[i], cmd_name, &ex_match) == 0) {

            if (ex_match) {
                array_of_possibilities[ 0 ] = options[i];
                j = 1;
                break;
            }
            array_of_possibilities[ j++ ] = options[i];
            assert (j < POSSIBILITY_ARRAY_SIZE);
            continue;
        }
    }

    if(leaf_index >= 0 && j == 0)
        return options[leaf_index];

    if( j == 0)
        return NULL;

    if(j == 1)
        return array_of_possibilities[0];

    /* More than one param matched*/
    printf("%d possibilities :\n", j);
    for(i = 0; i < j; i++)
        printf("%-2d. %s\n", i, GET_CMD_NAME(array_of_possibilities[i]));

    printf("Choice [0-%d] : ? ", j-1);
    scanf("%d", &choice);

    /*  Read \n and discard */
    if((fgets((char *)cons_input_buffer, sizeof(cons_input_buffer)-1, stdin) == NULL)){
        printf("error in reading from stdin\n");
    }

    if(choice < 0 || choice > (j-1)){
        printf("\nInvalid Choice");
        return NULL;
    }

    return array_of_possibilities[choice];   
}


static tlv_struct_t tlv;

static CMD_PARSE_STATUS
build_tlv_buffer(char **tokens, 
                 size_t token_cnt){ 

    int i = 0; 
    param_t *parent = NULL;
    param_t *param = get_cmd_tree_cursor();
    CMD_PARSE_STATUS status = COMPLETE;
    op_mode enable_or_disable = MODE_UNKNOWN; 


    memset(&tlv, 0, sizeof(tlv_struct_t));

    for(; i < token_cnt; i++){
        
        parent = param;    
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
                if(IS_PARAM_NO_CMD(param)){
                    enable_or_disable = CONFIG_DISABLE;
                }
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

        case CMD_NOT_FOUND:
            printf(ANSI_COLOR_RED "Error : Following Token not registered : %s\n"
                        ANSI_COLOR_RESET, *(tokens +i));
            break;

        case INVALID_LEAF:
            printf(ANSI_COLOR_RED "Error : Following leaf value could not be validated"
                    ": %s, Expected Data type = %s\n" 
                    ANSI_COLOR_RESET, *(tokens +i), GET_LEAF_TYPE_STR(param));
            break;

        case COMPLETE:
            printf(ANSI_COLOR_GREEN "Parse Success.\n" ANSI_COLOR_RESET);
            if(param == libcli_get_show_brief_extension_param()){
                if(!IS_APPLICATION_CALLBACK_HANDLER_REGISTERED(parent)){
                    status = INCOMPLETE_COMMAND;
                    printf(ANSI_COLOR_YELLOW "Error : Incomplete Command\n" ANSI_COLOR_RESET);
                    break;
                }
                enable_or_disable = OPERATIONAL;
                /*Add the show extension param TLV to tlv buffer, this is really not an
                 * application callback*/
                INVOKE_APPLICATION_CALLBACK_HANDLER(param, tlv_buff, enable_or_disable);
                memset(command_code_tlv.value, 0, LEAF_VALUE_HOLDER_SIZE);
                sprintf(command_code_tlv.value, "%d", parent->CMDCODE);
                collect_tlv(tlv_buff, &command_code_tlv); 
                /*Now invoke the pplication handler*/
                INVOKE_APPLICATION_CALLBACK_HANDLER(parent, tlv_buff, enable_or_disable);
            }

            else if(param == libcli_get_suboptions_param()){
                memset(command_code_tlv.value, 0, LEAF_VALUE_HOLDER_SIZE);
                sprintf(command_code_tlv.value, "%d", parent->CMDCODE);
                mark_checkpoint_serialize_buffer(tlv_buff);
                display_sub_options_callback(parent, tlv_buff, MODE_UNKNOWN);
                set_cmd_tree_cursor(parent);
		if (parent == libcli_get_root()) break;
                build_mode_console_name(parent);
                break;
            }

            else if(param == libcli_get_mode_param()){
                
                memset(command_code_tlv.value, 0, LEAF_VALUE_HOLDER_SIZE);
                sprintf(command_code_tlv.value, "%d", parent->CMDCODE);
                /*Let us checkpoint the ser buffer before adding the commandcode, 
                 * because we would not want cmd code in subsequent comds in mode*/
                mark_checkpoint_serialize_buffer(tlv_buff);
                collect_tlv(tlv_buff, &command_code_tlv);
                mode_enter_callback(parent, tlv_buff, 
                    enable_or_disable == CONFIG_DISABLE ? CONFIG_DISABLE : CONFIG_ENABLE);
            }

            else if(param == libcli_get_cmd_expansion_param())
                display_cmd_expansion_callback(parent, tlv_buff, MODE_UNKNOWN);

            else{
                param_t *curr_hook = get_current_branch_hook(param);

                if(curr_hook == libcli_get_config_hook() &&
                        enable_or_disable != CONFIG_DISABLE)
                    enable_or_disable = CONFIG_ENABLE;

                else if(curr_hook != libcli_get_config_hook())
                    enable_or_disable = OPERATIONAL;

                if(curr_hook != libcli_get_repeat_hook() &&
                    param != libcli_get_config_hook()){
                    /*Add command code here*/
                    memset(command_code_tlv.value, 0, LEAF_VALUE_HOLDER_SIZE);
                    sprintf(command_code_tlv.value, "%d", param->CMDCODE);
                    collect_tlv(tlv_buff, &command_code_tlv); 
                }
                INVOKE_APPLICATION_CALLBACK_HANDLER(param, tlv_buff, enable_or_disable);
            }
            break;

        case USER_INVALID_LEAF:
            printf(ANSI_COLOR_YELLOW "Error : User validation has failed : Invalid value for Leaf : %s\n", GET_LEAF_ID(param));
            printf(ANSI_COLOR_RESET);
            break;

        case INCOMPLETE_COMMAND:
            printf(ANSI_COLOR_YELLOW "Error : Incomplete Command\n" ANSI_COLOR_RESET);
            break;

        default:
            printf(ANSI_COLOR_RED "FATAL : Unknown case fall\n" ANSI_COLOR_RESET);
    }
    return status;;
}

static void
parser_process_repeat_cmd(char *next_token) {

    INVOKE_APPLICATION_CALLBACK_HANDLER(
            libcli_get_repeat_hook() , 0, OPERATIONAL);
}

CMD_PARSE_STATUS
parse_input_cmd(char *input, unsigned int len, bool *is_repeat_cmd){

    char** tokens = NULL;
    size_t token_cnt = 0;
    char file_name[128];
    CMD_PARSE_STATUS status = COMPLETE;
    
    tokens = tokenizer(input, ' ', &token_cnt);
    if(!token_cnt)
        return INCOMPLETE_COMMAND;

    if(token_cnt > 1 && 
            ((strncmp(tokens[0], DO, 2) == 0)) &&
            (get_cmd_tree_cursor() != libcli_get_root())) /*do commands are not allowed from root*/

    {
        if(IS_CURRENT_MODE_CONFIG()) /*Do commands are allowed only when user is operating in config mode*/
        {
            param_t *old_cursor_state = get_cmd_tree_cursor(), *new_cursor_state = NULL;
            set_cmd_tree_cursor(libcli_get_do_hook());/*It will also destroy the config mode serialize buffer*/

            /*There is a cross branch junp from config to Operational branch, hence reset the buffer*/
            reset_serialize_buffer(tlv_buff);
            status = build_tlv_buffer(&tokens[1], token_cnt-1);

            /*It might be possible that user may have switched the branch from within config branch*/
            new_cursor_state = get_cmd_tree_cursor();

            /*If new cursor is not same as do_hook, it means, 
             * user has entered into MODE in some operational branch*/

            if(new_cursor_state != libcli_get_do_hook()){
                if(IS_CURRENT_MODE_CONFIG()){
                    assert(0); /*Impossible case*/
                }
            }
            else{
                /*User is in the config mode only */
                set_cmd_tree_cursor(old_cursor_state);
                /*We need to rebuild the TLV buffer afresh*/
                build_cmd_tree_leaves_data(tlv_buff, libcli_get_root(), get_cmd_tree_cursor());
                mark_checkpoint_serialize_buffer(tlv_buff);
            }
        }
        else
            printf("Info : do is supported from within config mode only\n");
    }

    else if (strncmp (tokens[0], "repeat" , strlen(tokens[0])) == 0) {
        parser_process_repeat_cmd(token_cnt == 1 ? 0 : tokens[1]);
        if (is_repeat_cmd) *is_repeat_cmd = true;
    }   

    else if((strncmp(tokens[0], GOTO_ONE_LVL_UP_STRING, strlen(GOTO_ONE_LVL_UP_STRING)) == 0) && (token_cnt == 1))
        go_one_level_up_cmd_tree(get_cmd_tree_cursor());
    
    else if((strncmp(tokens[0], GOTO_TOP_STRING, strlen(GOTO_TOP_STRING)) == 0) && (token_cnt == 1))
        goto_top_of_cmd_tree(get_cmd_tree_cursor());

    
    else if((strncmp(tokens[0], CLEAR_SCR_STRING, strlen(CLEAR_SCR_STRING)) == 0) && (token_cnt == 1))
        clear_screen_handler(0, 0, MODE_UNKNOWN);

    else if (!strncmp(tokens[0], "run" , strlen("run"))  && 
	     !strncmp(tokens[1], "ut" , strlen("ut"))    &&
	     token_cnt == 4 ) {

        char *pend;
        long int tc_no = strtol(tokens[3], &pend, 10);
        strncpy(file_name, tokens[2], strlen(tokens[2]));
        file_name[strlen(tokens[2])] = '\0';
        run_test_case(file_name, tc_no);
    }
    
    else if ( !strncmp(tokens[0], "config" , strlen("config"))  &&
	      !strncmp(tokens[1], "load" , strlen("load"))      &&
	      token_cnt == 3 ) {

	    strncpy(file_name, tokens[2], strlen(tokens[2]));
	    file_name[strlen(tokens[2])] = '\0';
	    parse_file(file_name);
    }
    else 
        status = build_tlv_buffer(tokens, token_cnt); 

    re_init_tokens(MAX_CMD_TREE_DEPTH);

    if(is_user_in_cmd_mode())
        restore_checkpoint_serialize_buffer(tlv_buff);
    else
        reset_serialize_buffer(tlv_buff);

    return status;
}


void
command_parser(void){

    bool is_repeat_cmd;
    CMD_PARSE_STATUS status = UNKNOWN;

    printf("run - \'show help\' cmd to learn more");
    place_console(1);
    memset(&command_code_tlv, 0, sizeof(tlv_struct_t));

    command_code_tlv.leaf_type = INT;
    strncpy(command_code_tlv.leaf_id, "CMDCODE", LEAF_ID_SIZE);
    command_code_tlv.leaf_id[LEAF_ID_SIZE -1] = '\0';
    memset(cons_input_buffer, 0, CONS_INPUT_BUFFER_SIZE);

    while(1){

        is_repeat_cmd = false;

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
         
        status = parse_input_cmd(cons_input_buffer, strlen(cons_input_buffer), &is_repeat_cmd);

        if( is_repeat_cmd ) {
            memset(cons_input_buffer, 0, CONS_INPUT_BUFFER_SIZE);
            place_console(1);
            continue;
        }

        if(status == COMPLETE && cmd_recording_enabled) {
            record_command(CMD_HIST_RECORD_FILE,
						   cons_input_buffer,
						   strlen(cons_input_buffer));
		}

		cmd_recording_enabled = true;

        memset(last_command_input_buffer, 0, CONS_INPUT_BUFFER_SIZE);
        memcpy(last_command_input_buffer, cons_input_buffer, strlen(cons_input_buffer));
        last_command_input_buffer[strlen(last_command_input_buffer)] = '\0';

        memset(cons_input_buffer, 0, CONS_INPUT_BUFFER_SIZE);

        place_console(1);
    }
}

void
parse_file(char *file_name) {

	char line[256];
	char** tokens = NULL;
	size_t token_cnt = 0;

	FILE *fptr = fopen(file_name, "r");
	
	if (!fptr) {
	
		printf("Error : Could not open log file %s, errno = %d\n",
				file_name, errno);
		return;
	}

	memset(line, 0, sizeof(line));

	cmd_recording_enabled = false;
	reset_serialize_buffer(tlv_buff);

	while (fgets(line, sizeof(line) - 1, fptr)) {

		printf("Executing : %s", line);
	
		tokens = tokenizer(line, ' ', &token_cnt);		

		reset_serialize_buffer(tlv_buff);
		build_tlv_buffer(tokens, token_cnt);
		re_init_tokens(MAX_CMD_TREE_DEPTH);

		if (is_user_in_cmd_mode())
			restore_checkpoint_serialize_buffer(tlv_buff);
		else
			reset_serialize_buffer(tlv_buff);

		memset(line, 0, sizeof(line));
	}
	
	fclose(fptr);
	place_console(1);
}	

