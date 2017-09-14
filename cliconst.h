/*
 * =====================================================================================
 *
 *       Filename:  cliconst.h
 *
 *    Description:  Contstants used by libcli
 *
 *        Version:  1.0
 *        Created:  Saturday 05 August 2017 11:52:57  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 * =====================================================================================
 */

#ifndef __LIBCLICONSTANTS__
#define __LIBCLICONSTANTS__

#define CMD_NAME_SIZE           32
#define MAX_CMD_TREE_DEPTH      16
#define LEAF_VALUE_HOLDER_SIZE  64/* It should be atleast CMD_NAME_SIZE*/
#define LEAF_ID_SIZE            32
#define PARAM_HELP_STRING_SIZE  64
#define CONS_INPUT_BUFFER_SIZE  2048
#define TERMINAL_NAME_SIZE      CONS_INPUT_BUFFER_SIZE
#define TLV_MAX_BUFFER_SIZE     1024

#define DEFAULT_DEVICE_NAME     "root@juniper"
#define MODE_CHARACTER          "/"
#define SUBOPTIONS_CHARACTER    "?"
#define CMD_EXPANSION_CHARACTER "."
#define MAX_OPTION_SIZE         16

#define MODE_PARAM_INDEX        0
#define SUBOPTIONS_INDEX        1
#define CMD_EXPANSION_INDEX     2
#define CHILDREN_START_INDEX    3
#define CHILDREN_END_INDEX      (MAX_OPTION_SIZE -1)

#define NEGATE_CHARACTER        "no"



/* Default Command Codes*/
/*CMD codes need not be unique, but should be unique with in same command sharing 
 * the same command handler. For example, cmd code of SHOW_HISTORY and SHOW_HISTORY_N 
 * should be distinct because both commands have the same cmd handler */

#define SHOW_HELP                   1
#define SHOW_HISTORY                2
#define SHOW_HISTORY_N              3
#define SHOW_REGISTERED_COMMANDS    4
#define CONFIG_SUPPORTSAVE_ENABLE   5
#define CONFIG_CONSOLEN_NAME_NAME   6
#define DEBUG_SHOW_CMDTREE          7
#endif /* __LIBCLICONSTANTS__ */
