#ifndef READNETWORKDATA_H
#define READNETWORKDATA_H

#include "interfaces_list.h"

#include <stdio.h>
#include <stdint.h>

struct RawInterfaceData {
	char name[LENGTH];
	char ip[LENGTH];
	int mask;
	int rx;
	int tx;
};

struct Parameters;
struct Netinterface;
struct blob_attr;
struct ubus_context;

int read_network_interfaces_by_name(struct Netinterface **interfacelist, struct RawInterfaceData *netinterfacedata, struct ubus_context *ctx, uint32_t *id);
int read_network_interface_data(struct RawInterfaceData *interfacedata, struct ubus_context *ctx, uint32_t *id);
void read_ip_and_netmask_by_name(struct blob_attr *data, struct RawInterfaceData *interfacedata);
void parse_ipv4_data(struct blob_attr *data, struct RawInterfaceData *interfacedata);
void parse_address_and_mask(struct blob_attr *data, struct RawInterfaceData *interfacedata);
void convert_raw_interface_data(struct RawInterfaceData *raw, struct Netinterface **interfacelist);
#endif