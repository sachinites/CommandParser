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

    param_t *show   = libcli_get_show_hook();
    param_t *debug  = libcli_get_debug_hook();
    param_t *config = libcli_get_config_hook();

    static param_t cmsh;
    init_param(&cmsh, CMD, "cmsh", 0, 0, INVALID, 0, "cmsh hidden commands");
    libcli_register_param(0, &cmsh);

    /*Level 1*/
    static param_t ip;
    init_param(&ip, CMD, "ip", 0, 0, INVALID, 0, "Internet Protocol(IP)");
    libcli_register_param(show, &ip);

    static param_t ipv6;
    init_param(&ipv6, CMD, "ipv6", 0, 0, INVALID, 0, "Internet Protocol(IPV6)");
    libcli_register_param(show, &ipv6);

    static param_t debug_show;
    init_param(&debug_show, CMD, "show", 0, 0, INVALID, 0, "debug show commands");
    libcli_register_param(debug, &debug_show);
     
    static param_t config_router;
    init_param(&config_router, CMD, "router", 0, 0, INVALID, 0, "Configuration Router");
    libcli_register_param(config, &config_router);

    /*Level 2*/
    static param_t config_router_name;
    init_param(&config_router_name, CMD, "name", 0, 0, INVALID, 0, "configure router name");
    libcli_register_param(&config_router, &config_router_name);

    static param_t igmp;
    init_param(&igmp, CMD, "igmp", 0, 0, INVALID, 0, "IGMP Protocol");
    libcli_register_param(&ip, &igmp);

    static param_t debug_show_cmdtree;
    init_param(&debug_show_cmdtree, CMD, "cmdtree", show_cmd_tree, 0, INVALID, 0, "Display command tree");
    libcli_register_param(&debug_show, &debug_show_cmdtree);

    /*Level 3*/
    static param_t config_router_name_name;
    init_param(&config_router_name_name, LEAF, 0, config_router_name_handler, 0,  
                                              STRING, "cons-name", "Name of Console");
    libcli_register_param(&config_router_name, &config_router_name_name);
    
    static param_t show_ip_igmp_groups;
    init_param(&show_ip_igmp_groups, CMD, "groups", show_ip_igmp_groups_handler, 0, INVALID, 0, "Ipv4 Group Address");
    libcli_register_param(&igmp, &show_ip_igmp_groups);

    static param_t show_ip_igmp_groups_vlan;
    init_param(&show_ip_igmp_groups_vlan, CMD, "vlan", 0, 0, INVALID, 0, "vlan");
    libcli_register_param(&show_ip_igmp_groups, &show_ip_igmp_groups_vlan);

    static param_t show_ip_igmp_groups_vlan_vlan;
    init_param(&show_ip_igmp_groups_vlan_vlan, LEAF, 0, show_ip_igmp_groups_handler, user_vlan_validation_callback,
        INT, "vlan-id", "Vlan id(1-4095)");
    libcli_register_param(&show_ip_igmp_groups_vlan, &show_ip_igmp_groups_vlan_vlan);

    /*Level 5*/
    static param_t  show_ip_igmp_groups_group_ip;
    init_param(&show_ip_igmp_groups_group_ip, LEAF, 0, show_ip_igmp_groups_handler, 0, IPV4, 
                "group-ip" ,"Multicast Group Address");
    libcli_register_param(&show_ip_igmp_groups, &show_ip_igmp_groups_group_ip);

    /*level 6*/
    static param_t show_ip_igmp_groups_group_ip_vlan;
    init_param(&show_ip_igmp_groups_group_ip_vlan, CMD, "vlan", 0, 0, INVALID, 0, "Vlan");
    libcli_register_param(&show_ip_igmp_groups_group_ip, &show_ip_igmp_groups_group_ip_vlan);

    /*level 7*/
    static param_t show_ip_igmp_groups_group_ip_vlan_vlan_id;
    init_param(&show_ip_igmp_groups_group_ip_vlan_vlan_id, LEAF, 0, show_ip_igmp_groups_handler, 
                                    user_vlan_validation_callback, INT, "vlan-id" ,"Vlan id(1-4095)");
    libcli_register_param(&show_ip_igmp_groups_group_ip_vlan, &show_ip_igmp_groups_group_ip_vlan_vlan_id);

    start_shell();
    return 0;
}
