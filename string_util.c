/*
 * =====================================================================================
 *
 *       Filename:  string_util.c
 *
 *    Description:  String utilities
 *
 *        Version:  1.0
 *        Created:  Thursday 03 August 2017 05:35:37  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 * =====================================================================================
 */

#include "string_util.h"
#include <stdlib.h>
#include <assert.h>
#include "cliconst.h"


static char a_str[CONS_INPUT_BUFFER_SIZE];

char** str_split(char* _a_str, const char a_delim, size_t *token_cnt)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = NULL;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    memset(a_str, 0, CONS_INPUT_BUFFER_SIZE);
    memcpy(a_str, _a_str, strlen(_a_str));
    tmp = a_str;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
     *      *        knows where the list of returned strings ends. */
    count++;

    result = (char **)malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    *token_cnt = count -1;
    return result;
}

void
free_tokens(char **tokens){
    int i = 0;
    for ( ;*(tokens + i); i++)
    {
        free(*(tokens + i));
    }
    free(tokens);
}
