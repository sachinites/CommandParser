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
#define TLV_MAX_BUFFER_SIZE     1048

#define DEFAULT_DEVICE_NAME     "root@juniper"
#define MODE_CHARACTER          "/"

#define LEAF_HISTORY_SIZE       15
#endif /* __LIBCLICONSTANTS__ */
