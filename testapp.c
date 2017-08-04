/*
 * =====================================================================================
 *
 *       Filename:  testapp.c
 *
 *    Description:  Test Application to demonstrate the library usage
 *
 *        Version:  1.0
 *        Created:  Friday 04 August 2017 01:26:06  IST
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

int
show_ip_igmp_groups_handler(ser_buff_t *tlv_buf){
    
    printf("%s() is called ....", __FUNCTION__);
    return 0;
}

int
show_cmd_tree(ser_buff_t *tlv_buf){
        
    dump_cmd_tree();
    return 0;
}

int
user_vlan_validation_callback(char *vlan_id){

    printf("%s() is called ....\n", __FUNCTION__);
    int vlan_no = atoi(vlan_id);

    if(vlan_no > 0 && vlan_no < 4096)
        return 0;

    return -1;
}



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

    static cmd_t debug_show = {"show", 0, NULL_OPTIONS};
    static_register_command_after_command(&debug, &debug_show);
     
    /*Level 2*/
    static cmd_t igmp = {"igmp", 0, NULL_OPTIONS};
    static_register_command_after_command(&ip, &igmp);

    cmd_t *show_ip_pim = dynamic_register_command_after_command(&ip, "pim", 0);

    static cmd_t debug_show_cmdtree = {"cmdtree", show_cmd_tree, NULL_OPTIONS};
    static_register_command_after_command(&debug_show, &debug_show_cmdtree);

    /*Level 3*/
    static cmd_t groups = {"groups", show_ip_igmp_groups_handler, NULL_OPTIONS};
    static_register_command_after_command(&igmp, &groups);

    dynamic_register_command_after_command(&igmp, "statistics", 0);
    cmd_t *show_ip_pim_groups = dynamic_register_command_after_command(show_ip_pim, "groups", 0);
    cmd_t *show_ip_pim_mcache = dynamic_register_command_after_command(show_ip_pim, "mcache", 0);

    /*Level 4*/
    static cmd_t show_ip_igmp_groups_vlan = {"vlan", show_ip_igmp_groups_handler, NULL_OPTIONS};
    static_register_command_after_command(&groups, &show_ip_igmp_groups_vlan);

    static leaf_t group_ip = {IPV4, "0.0.0.0", show_ip_igmp_groups_handler, 0, NULL_OPTIONS};
    static_register_leaf_after_command(&groups, &group_ip);

    /*Level 5*/
    static leaf_t vlan_id = {INT, "10", show_ip_igmp_groups_handler, 
                            user_vlan_validation_callback, NULL_OPTIONS};

    static_register_leaf_after_command(&show_ip_igmp_groups_vlan, &vlan_id);

    /*level 6*/
    static cmd_t show_ip_igmp_groups_group_ip_vlan = {"vlan", show_ip_igmp_groups_handler, NULL_OPTIONS};
    static_register_command_after_leaf(&group_ip, &show_ip_igmp_groups_group_ip_vlan);

    /*level 7*/
    static leaf_t show_ip_igmp_groups_group_ip_vlan_vlan_id = {INT, "10", show_ip_igmp_groups_handler, 
                                    user_vlan_validation_callback, NULL_OPTIONS};

    static_register_leaf_after_command(&show_ip_igmp_groups_group_ip_vlan, &show_ip_igmp_groups_group_ip_vlan_vlan_id);

    start_shell();
    return 0;
}
