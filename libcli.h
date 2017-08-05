/*
 * =====================================================================================
 *
 *       Filename:  libcli.h
 *
 *    Description:  User interface Header file.
 *
 *        Version:  1.0
 *        Created:  Saturday 05 August 2017 11:23:15  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 * =====================================================================================
 */

#ifndef __LIBCLI__
#define __LIBCLI__

#include "libcliid.h"
#include "cmd_hier.h"


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

void
dump_cmd_tree();

void
start_shell(void);

#endif
