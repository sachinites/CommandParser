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
#include <stdio.h>
#include "cmdtlv.h"
#include "libcli.h"
#include <stdlib.h>

int
show_ip_igmp_groups_handler(ser_buff_t *tlv_buf, op_mode enable_or_disable){

    tlv_struct_t *tlv = NULL;   
    int i = 0;
   
    dump_tlv_serialized_buffer(tlv_buf);
    
#if 0
    TLV_LOOP(tlv_buf, tlv, i){
        if(strncmp(tlv->leaf_id, "group-ip", strlen("group-ip")) == 0){
            printf("Group Ip Recvd in application = %s\n", tlv->value);   
        }
        else if(strncmp(tlv->leaf_id, "vlan-id", strlen("vlan-id")) == 0){
            printf("vlan recieved in application = %s\n", tlv->value);
        }
    }
#endif
    return 0;
}

int
config_router_name_handler(ser_buff_t *tlv_buf, op_mode enable_or_disable){
    set_console_name("router2");
    return 0;
}


int
show_cmd_tree(ser_buff_t *tlv_buf, op_mode enable_or_disable){
        
    dump_cmd_tree();
    return 0;
}

int
user_vlan_validation_callback(char *vlan_id){

    int vlan_no = atoi(vlan_id);

    if(vlan_no > 0 && vlan_no < 4096)
        return 0;

    printf("Invalid vlan. Pls follow Help\n");
    return -1;
}

int
main(int argc, char **argv){
    
    init_libcli();
    /*Level 0*/

    cmd_t *show   = libcli_get_show_hook();
    cmd_t *debug  = libcli_get_debug_hook();
    cmd_t *config = libcli_get_config_hook();

    static cmd_t cmsh = {"cmsh", 0, "cmsh hidden commands", NULL_OPTIONS};
    static_register_command_after_command(0, &cmsh);

    /*Level 1*/
    static cmd_t ip = {"ip", 0, "Internet Protocol(IP)", NULL_OPTIONS};
    static_register_command_after_command(show, &ip);

    static cmd_t ipv6 = {"ipv6", 0, "Internet Protocol(IPV6)" ,NULL_OPTIONS};
    static_register_command_after_command(show, &ipv6);

    static cmd_t debug_show = {"show", 0, "debug show commands", NULL_OPTIONS};
    static_register_command_after_command(debug, &debug_show);
     
    static cmd_t config_router = {"router", 0, "Configuration Router",NULL_OPTIONS};
    static_register_command_after_command(config, &config_router);

    /*Level 2*/

    static cmd_t config_router_name = {"name", 0, "configure router name" , NULL_OPTIONS};
    static_register_command_after_command(&config_router, &config_router_name);

    static cmd_t igmp = {"igmp", 0, "IGMP Protocol", NULL_OPTIONS};
    static_register_command_after_command(&ip, &igmp);

    cmd_t *show_ip_pim = dynamic_register_command_after_command(&ip, "pim", 0);

    static cmd_t debug_show_cmdtree = {"cmdtree", show_cmd_tree, "Display command tree", NULL_OPTIONS};
    static_register_command_after_command(&debug_show, &debug_show_cmdtree);

    /*Level 3*/

    static leaf_t config_router_name_name = {STRING, "router", config_router_name_handler, 
                                              0, "console-id", "Name of Console", NULL_OPTIONS};

    static_register_leaf_after_command(&config_router_name, &config_router_name_name);

    static cmd_t groups = {"groups", show_ip_igmp_groups_handler, "Ipv4 Group Address", NULL_OPTIONS};
    static_register_command_after_command(&igmp, &groups);

    dynamic_register_command_after_command(&igmp, "statistics", 0);
    cmd_t *show_ip_pim_groups = dynamic_register_command_after_command(show_ip_pim, "groups", 0);
    cmd_t *show_ip_pim_mcache = dynamic_register_command_after_command(show_ip_pim, "mcache", 0);

    /*Level 4*/

    static leaf_t show_ip_pim_mcache_src = {IPV4, "0.0.0.0", 0, 0, "source-ip", 
                                             "Ipv4 address of Source", NULL_OPTIONS};
    static_register_leaf_after_command(show_ip_pim_mcache, &show_ip_pim_mcache_src);

#if 0
    static leaf_t show_ip_pim_mcache_grp = {IPV4, "0.0.0.0", 0, 0, NULL_OPTIONS};
    static_register_leaf_after_command(show_ip_pim_mcache, &show_ip_pim_mcache_grp);
#endif

    static cmd_t show_ip_igmp_groups_vlan = {"vlan", show_ip_igmp_groups_handler, "Vlan", NULL_OPTIONS};
    static_register_command_after_command(&groups, &show_ip_igmp_groups_vlan);

    static leaf_t group_ip = {IPV4, "0.0.0.0", show_ip_igmp_groups_handler, 0, 
                            "group-ip", "Multicast Group Address", NULL_OPTIONS};
    static_register_leaf_after_command(&groups, &group_ip);

    /*Level 5*/

    static leaf_t show_ip_pim_mcache_src_grp = {IPV4, "0.0.0.0", 0, 0, "group-ip", "Multicast Group Address", NULL_OPTIONS};
    static_register_leaf_after_leaf(&show_ip_pim_mcache_src, &show_ip_pim_mcache_src_grp);

    static leaf_t vlan_id = {INT, "10", show_ip_igmp_groups_handler, 
                            user_vlan_validation_callback, "vlan-id", "Vlan id(1-4095)", NULL_OPTIONS};

    static_register_leaf_after_command(&show_ip_igmp_groups_vlan, &vlan_id);

    /*level 6*/
    static cmd_t show_ip_igmp_groups_group_ip_vlan = {"vlan", show_ip_igmp_groups_handler, "Vlan", NULL_OPTIONS};
    static_register_command_after_leaf(&group_ip, &show_ip_igmp_groups_group_ip_vlan);

    /*level 7*/
    static leaf_t show_ip_igmp_groups_group_ip_vlan_vlan_id = {INT, "10", show_ip_igmp_groups_handler, 
                                    user_vlan_validation_callback, "vlan-id", "Vlan id(1-4095)", NULL_OPTIONS};

    static_register_leaf_after_command(&show_ip_igmp_groups_group_ip_vlan, &show_ip_igmp_groups_group_ip_vlan_vlan_id);

    start_shell();
    return 0;
}
