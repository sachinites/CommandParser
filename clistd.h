/*
 * =====================================================================================
 *
 *       Filename:  clistd.h
 *
 *    Description:  All Validation functions for leaves and standard default callbacks are defined in this file
 *
 *        Version:  1.0
 *        Created:  Sunday 06 August 2017 06:00:22  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 * =====================================================================================
 */

#ifndef __CLISTD__ 
#define __CLISTD__

#include "libcliid.h"

typedef struct leaf leaf_t;
typedef struct _param_t_ param_t;

typedef struct serialized_buffer ser_buff_t;

typedef enum{
    VALIDATION_FAILED = -1,
    VALIDATION_SUCCESS
} CLI_VAL_RC;

/*Default validation handlers for Data types*/

CLI_VAL_RC
int_validation_handler(leaf_t *leaf, char *value_passed);

CLI_VAL_RC
string_validation_handler(leaf_t *leaf, char *value_passed);

CLI_VAL_RC
ipv4_validation_handler(leaf_t *leaf, char *value_passed);

CLI_VAL_RC
ipv6_validation_handler(leaf_t *leaf, char *value_passed);

CLI_VAL_RC
float_validation_handler(leaf_t *leaf, char *value_passed);

/* Default command handlers */
int
config_console_name_handler(param_t *param, ser_buff_t *b, op_mode enable_or_disable);

int
repeat_last_command(param_t *param, ser_buff_t *b, op_mode enable_or_disable);

int
mode_enter_callback(param_t *param, ser_buff_t *b, op_mode enable_or_disable);

#endif /* __CLISTD__ */