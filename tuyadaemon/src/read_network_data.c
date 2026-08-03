#include "read_network_data.h"
#include "message_handler.h"
#include "interfaces_list.h"

#include <libubox/blobmsg_json.h>
#include <libubus.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <uci.h>



enum {
	NETWORK_DATA,
	__NETWORK_MAX,
};

enum {
	INTERFACE_DATA,
	__INTERFACE_DATA_MAX,
};

enum {
	RX_DATA,
	TX_DATA,
	__NETDATA_MAX,
};

static const struct blobmsg_policy network_statistics_policy[__NETWORK_MAX] = {
	[NETWORK_DATA]	= { .name = "statistics", .type = BLOBMSG_TYPE_TABLE },
};

static const struct blobmsg_policy network_data_policy[__NETDATA_MAX] = {
	[RX_DATA]	= { .name = "rx_bytes", .type = BLOBMSG_TYPE_INT64 },
	[TX_DATA]	= { .name = "tx_bytes", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy network_interface_policy[__INTERFACE_DATA_MAX] = {
	[INTERFACE_DATA]	= { .name = "interface", .type = BLOBMSG_TYPE_ARRAY },
};

static void netintstatus_cb(struct ubus_request *req, int type, struct blob_attr *msg);
static void netintdump_cb(struct ubus_request *req, int type, struct blob_attr *msg);

// For each network interface name gotten from UCI, reads interface data from UBUS
int read_network_interfaces_by_name(struct Netinterface **interfacelist, struct RawInterfaceData *interfacedata, struct ubus_context *ctx, uint32_t *id)
{
	int status = 0;
    struct uci_context *cursor;
    struct uci_ptr ptr;
    struct uci_package *config;
    struct uci_section *section;
    struct uci_option *option;

    cursor = uci_alloc_context();
    status = uci_load(cursor, "network", &config);
    if (status != UCI_OK) {
		syslog(LOG_ERR, "Error loading UCI");
        goto clean_up;
    }
	struct uci_element *i, *j;    // Iteration variables
	uci_foreach_element(&config->sections, i)
    {
        struct uci_section *section = uci_to_section(i);
        char *section_type = section->type;
        char *section_name = section->e.name;
		
		if (strcmp(section_type, "interface") != 0) {
			continue;
		}

		if ((strcmp(section_name, "wan6") == 0) || (strcmp(section_name, "loopback") == 0))  {
			continue;
		}

		uci_foreach_element(&section->options, j)
		{
			struct uci_option *option = uci_to_option(j);
			char *option_name = option->e.name;
			
			if (option->type != UCI_TYPE_STRING) {
				continue;
			}
			if (strcmp(option_name, "device") == 0) {
				strncpy(interfacedata->name, option->v.string, LENGTH);
				if (read_network_interface_data(interfacedata, ctx, id) != SUCCESS) {
					syslog(LOG_ERR, "Error reading data for interface \"%s\".", interfacedata->name);
				}
				convert_raw_interface_data(interfacedata, interfacelist);
			}
		}
    }

clean_up:
	uci_free_context(cursor);
    return status;
}

// Callback function for reading an interface's transmitted and received bytes from status
static void netintstatus_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct RawInterfaceData *receivedData = (struct RawInterfaceData *)req->priv;
	struct blob_attr *tb[__NETWORK_MAX];
	struct blob_attr *rx_tx[__NETDATA_MAX];
	
	// get statistics
	blobmsg_parse(network_statistics_policy, __NETWORK_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[NETWORK_DATA]) {
		syslog(LOG_ERR, "No network statistics data received.");
		return;
	}
	// get rx & tx
	blobmsg_parse(network_data_policy, __NETDATA_MAX, rx_tx, blobmsg_data(tb[NETWORK_DATA]),
		      blobmsg_data_len(tb[NETWORK_DATA]));

	receivedData->rx = blobmsg_get_u64(rx_tx[RX_DATA]);
	receivedData->tx = blobmsg_get_u64(rx_tx[TX_DATA]);
}

// Callback function for reading an interface's IP and netmask from dump
static void netintdump_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct RawInterfaceData *receivedData = (struct RawInterfaceData *)req->priv;
	struct blob_attr *tb[__INTERFACE_DATA_MAX];
	struct blob_attr *cur;
	size_t rem;

	blobmsg_parse(network_interface_policy, __INTERFACE_DATA_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[NETWORK_DATA]) {
		syslog(LOG_ERR, "No network interface data received.");
		return;
	}

	blobmsg_for_each_attr(cur, tb[NETWORK_DATA], rem) {
		strcpy(receivedData->ip, "\0");
		receivedData->mask = 0;
		read_ip_and_netmask_by_name(cur, receivedData);
		// Stops iterating when IP and mask have been found
		if ((strcmp(receivedData->ip,"\0") != 0) && (receivedData->mask != 0)) {
			break;
		}
	}
}

// Reads network interface data from UBUS for a given name
int read_network_interface_data(struct RawInterfaceData *interfacedata, struct ubus_context *ctx, uint32_t *id)
{
	// ubus call network.device status '{"name": "name"}'
	// -> statistics -> tx bytes
	// -> statistics -> rx bytes
	char command[50];
	int status = SUCCESS;
	struct blob_buf b;
	blob_buf_init(&b, 0);

	if (ubus_lookup_id(ctx, "network.device", id)) {
		syslog(LOG_ERR, "UBUS cannot find ID of object \"network.device\".");
		status = FAILURE;
		goto clean_up;
	} 
	
	sprintf(command, "{\"name\": \"%s\"}", interfacedata->name);
	if (!(blobmsg_add_json_from_string(&b, command))) {
		syslog(LOG_ERR, "Failure adding json parameter %s.", command);
		status = FAILURE;
		goto clean_up;
	}
	
	if (ubus_invoke(ctx, *id, "status", b.head, netintstatus_cb, interfacedata, 3000)) {
		syslog(LOG_ERR, "Cannot request network device status from procd.");
		status = FAILURE;
		goto clean_up;
	} 

	// ubus call network.interface dump
	// -> interface -> device (compare)
	// -> interface -> ipv4-address -> address
	// -> interface -> ipv4-address -> mask

	if (ubus_lookup_id(ctx, "network.interface", id)) {
		syslog(LOG_ERR, "UBUS cannot find ID of object \"network.interface\".");
		status = FAILURE;
		goto clean_up;
	} 

	if (ubus_invoke(ctx, *id, "dump", NULL, netintdump_cb, interfacedata, 3000)) {
		syslog(LOG_ERR, "Cannot request network interface dump from procd.");
		status = FAILURE;
		goto clean_up;
	} 

clean_up:
	blob_buf_free(&b);
	return status;
}


// Reads netmask and IP for a given interface name
void read_ip_and_netmask_by_name(struct blob_attr *data, struct RawInterfaceData *interfacedata)
{
	struct blob_attr *cur;
	size_t rem;
	char device[LENGTH];
	char proto[LENGTH];
	char up;
	
	// Discards if the interface name doesn't match, if the interface is down, or if the protocol is dhcpv6
	blobmsg_for_each_attr(cur, data, rem) {
		if (strcmp(blobmsg_name(cur), "device") == 0) {
			strncpy(device, blobmsg_get_string(cur), LENGTH);
			if (strcmp(device, interfacedata->name) != 0) {
				break;
			}
		}
		if (strcmp(blobmsg_name(cur), "up") == 0) {
			up = blobmsg_get_u8(cur);	
			if (up == 0) {
				break;
			}
		}
		if (strcmp(blobmsg_name(cur), "proto") == 0) {
			strncpy(proto, blobmsg_get_string(cur), LENGTH);
			if (strcmp(proto, "dhcpv6") == 0) {
				break;
			}
		}

		if (strcmp(blobmsg_name(cur), "ipv4-address") == 0) {
			parse_ipv4_data(cur, interfacedata);
		}
	}
}

// Reads "ipv4-address" table
void parse_ipv4_data(struct blob_attr *data, struct RawInterfaceData *interfacedata)
{
	struct blob_attr *cur;
	size_t rem;
	blobmsg_for_each_attr(cur, data, rem) {
		parse_address_and_mask(cur, interfacedata);
	}
}

// Gets ipv4 address and netmask from "ipv4-address" table
void parse_address_and_mask(struct blob_attr *data, struct RawInterfaceData *interfacedata)
{
	struct blob_attr *cur;
	size_t rem;
	blobmsg_for_each_attr(cur, data, rem) {
		if (strcmp(blobmsg_name(cur), "address") == 0) {
			strncpy(interfacedata->ip, blobmsg_get_string(cur), LENGTH);
		}
		if (strcmp(blobmsg_name(cur), "mask") == 0) {
			interfacedata->mask = blobmsg_get_u32(cur);
		}
	}
}

// Converts raw interface data to needed formats & adds to linked interface list
void convert_raw_interface_data(struct RawInterfaceData *raw, struct Netinterface **interfacelist)
{
	char netmask[LENGTH];
	char received[LENGTH];
	char transmitted[LENGTH];
	struct Netinterface *node = NULL;

	// Converts CIDR prefix to subnet mask
	// Shifts 0xFFFFFFFF to the left by amount of free bits
	unsigned long mask = (0xFFFFFFFF << (32 - raw->mask)) & 0xFFFFFFFF; 
	// Splits into bytes
	snprintf(netmask, LENGTH, "%ld.%ld.%ld.%ld", mask >> 24, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask & 0xFF);
	snprintf(received, LENGTH, "%d", raw->rx);
	snprintf(transmitted, LENGTH, "%d", raw->tx);

	node = create_node(raw->name, raw->ip, netmask, received, transmitted);
	add_to_list(interfacelist, node);
}
