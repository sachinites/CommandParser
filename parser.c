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

#include "cmd_hier.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "serialize.h"
#include "string_util.h"

extern param_t root;
extern leaf_type_handler leaf_handler_array[LEAF_MAX];
extern ser_buff_t *tlv_buff;

static void
place_console(){
    printf("\nrouter> ");
}

static char cons_input_buffer[2048];

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

static int
build_tlv_buffer(char **tokens, size_t token_cnt, param_t **out_param){ 
#if 1
    param_t *param = &root;
    int i = 0;
    
    for(; i < token_cnt; i++){
        param = find_matching_param(get_child_array_ptr(param), *(tokens +i));
        if(param){
            if(IS_PARAM_LEAF(param)){
                printf("token[%d] = %s Not found in cmd tree, leaf = %s\n", i, *(tokens +i), GET_LEAF_TYPE_STR(param));
                continue;
            }
            else{
                printf("token[%d] = %s found in cmd tree\n", i, *(tokens +i));
                continue;
            }
        }
        break;
    }

    if(!param){
        printf("Token not registered : %s\n", *(tokens +i));
    }
#endif
    return 0;
}

static void
parse_input_cmd(char *input, unsigned int len){
    
        printf("input -- > %d : %s", len, cons_input_buffer);
        place_console();

        int i = 0, tok_no = 0;
        char** tokens;
        param_t *param = NULL;
        size_t token_cnt = 0;

        tokens = str_split(input, ' ', &token_cnt);
        if(!tokens)
            return;

        /*Walk the cmd tree now and build the TLV buffer of leavf values, if any*/

        build_tlv_buffer(tokens, token_cnt, &param);

        i = 0;
        printf("No of token = %d\n", token_cnt);
        for ( ;*(tokens + i); i++)
        {
            printf("token=[%d] = %s\n", i, *(tokens + i));
            free(*(tokens + i));
        }
        free(tokens);
}


void
command_parser(void){

    place_console();
    while(1){
        if((fgets((char *)cons_input_buffer, sizeof(cons_input_buffer)-1, stdin) == NULL)){
            printf("error in reading from stdin\n");
            exit(EXIT_SUCCESS);
        }
       
        cons_input_buffer[strlen(cons_input_buffer) - 1] = '\0'; 
        parse_input_cmd(cons_input_buffer, strlen(cons_input_buffer));
        memset(cons_input_buffer, 0, sizeof(cons_input_buffer));    
        place_console();
    }
}

