/*
 * =====================================================================================
 *
 *       Filename:  cmdtlv2.h
 *
 *    Description:  TLV implementation on top of serialized library
 *
 *        Version:  1.0
 *        Created:  Friday 04 August 2017 03:59:45  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *
 * =====================================================================================
 */


#ifndef __CMDTLV__H
#define __CMDTLV__H

#include "cmd_hier.h"
#include "serialize.h"

#pragma pack (push,1)
typedef struct tlv_struct{
    leaf_type_t leaf_type;
    char leaf_id[LEAF_ID_SIZE];
    char value[LEAF_VALUE_HOLDER_SIZE];
} tlv_struct_t;
#pragma pack(pop)

#define TLV_LOOP(ser_buff, tlvptr, i)                                                   \
    assert(ser_buff);                                                                   \
    tlvptr = (tlv_struct_t *)(ser_buff->b);                                             \
    i = 0;                                                                              \
    for(; i < (get_serialize_buffer_size(ser_buff)/sizeof(tlv_struct_t)); i++, tlvptr++)

#define tlv_copy_leaf_id(tlvptr, dst)                          \
    strncpy(dst, tlvptr->leaf_id, strlen(tlvptr->leaf_id));    \
    dst[strlen(tlvptr->leaf_id)] = '\0';


#define tlv_copy_leaf_value(tlvptr, dst)                         \
    strncpy(dst, tlvptr->value, strlen(tlvptr->value));          \
    dst[strlen(tlvptr->value)] = '\0';

#define collect_tlv(ser_buff, tlvptr)           \
    serialize_string(ser_buff, (char *)tlvptr, sizeof(tlv_struct_t))

#define prepare_tlv_from_leaf(leaf, tlvptr)    \
    tlvptr->leaf_type = leaf->leaf_type;       \
    strncpy(tlvptr->leaf_id, leaf->leaf_id, MIN(LEAF_ID_SIZE, strlen(leaf->leaf_id)));

#define put_value_in_tlv(tlvptr, _val)         \
    strncpy(tlvptr->value, _val, MIN(LEAF_VALUE_HOLDER_SIZE, strlen(_val)));


#endif /* __CMDTLV__H */
