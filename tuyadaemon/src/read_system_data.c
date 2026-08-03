#include "read_network_data.h"
#include "read_system_data.h"
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
	TOTAL_MEMORY,
	FREE_MEMORY,
	__MEMORY_MAX,
};

enum {
	MEMORY_DATA,
	CPU_LOAD_DATA,
	UPTIME_DATA,
	__INFO_MAX,
};

static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY]	  = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
	[FREE_MEMORY]	  = { .name = "free", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA]	= { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
	[CPU_LOAD_DATA]	= { .name = "load", .type = BLOBMSG_TYPE_ARRAY },
	[UPTIME_DATA]	= { .name = "uptime", .type = BLOBMSG_TYPE_INT32 },
};

static void systeminfo_cb(struct ubus_request *req, int type, struct blob_attr *msg);

// Gets total RAM, free RAM, CPU load, uptime and network interface parameters from UBUS
int read_ubus_data(struct Parameters *parameters) {
    struct ubus_context *ctx;
	uint32_t id;
	int status = SUCCESS;

	struct RawSystemInfoData systeminfodata = { 0 };
	struct RawInterfaceData interfacedata = { 0 };

	ctx = ubus_connect(NULL);
	if (!ctx) {
		syslog(LOG_ERR,  "Failed to connect to ubus\n");
		return FAILURE;
	}

	if (read_system_info_data(parameters, &systeminfodata, ctx, &id) != SUCCESS) {
		syslog(LOG_ERR, "Failed to read CPU, RAM and uptime data");
		status = FAILURE;
	}
	if (read_network_interfaces_by_name(&(parameters->interfacelist), &interfacedata, ctx, &id) != SUCCESS) {
		syslog(LOG_ERR, "Failed to read network interface data");
		status = FAILURE;
	}

	ubus_free(ctx);

	return status;
}

// Callback function for reading CPU, RAM and uptime data from system info
static void systeminfo_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct RawSystemInfoData *receivedData = (struct RawSystemInfoData *)req->priv;
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *memory[__MEMORY_MAX];

	blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[MEMORY_DATA]) {
		syslog(LOG_ERR, "No memory data received\n");
		return;
	}
	if (!tb[CPU_LOAD_DATA]) {
		syslog(LOG_ERR, "No cpu data received\n");
		return;
	}
	if (!tb[UPTIME_DATA]) {
		syslog(LOG_ERR, "No uptime data received\n");
		return;
	}

	// Uptime
	receivedData->uptime = blobmsg_get_u32(tb[UPTIME_DATA]);

	// CPU
	parse_cpu_load_data(tb, receivedData);

	// RAM
	blobmsg_parse(memory_policy, __MEMORY_MAX, memory, blobmsg_data(tb[MEMORY_DATA]),
		      blobmsg_data_len(tb[MEMORY_DATA]));
	receivedData->total = blobmsg_get_u64(memory[TOTAL_MEMORY]);
	receivedData->free = blobmsg_get_u64(memory[FREE_MEMORY]);
}

// Calls system info
int read_system_info_data(struct Parameters *parameters, struct RawSystemInfoData *systeminfodata, struct ubus_context *ctx, uint32_t *id)
{
	double mb = 1024*1024;
	double seconds_in_hour = 3600;
	double load_constant =  65536.0;
	// "The values in load are the load averages over 1, 5, and 15 minutes."
	// "to get to the familiar values reported by uptime divide these numbers by 65536.0" 
	// https://openwrt.org/docs/guide-developer/ubus/system
	int cores = (int)sysconf(_SC_NPROCESSORS_ONLN); // Gets number of online CPU cores

	if (ubus_lookup_id(ctx, "system", id)) {
		syslog(LOG_ERR, "UBUS cannot find ID of object \"system\"\n");
		return FAILURE;
	}

	if (ubus_invoke(ctx, *id, "info", NULL, systeminfo_cb, systeminfodata, 3000)) {
		syslog(LOG_ERR, "Cannot request system info from procd\n");
		return FAILURE;
	} else {
		parameters->totalram = (double)systeminfodata->total / mb; // Converts to megabytes
    	parameters->freeram = (double)systeminfodata->free / mb;
		parameters->cpu = (double)systeminfodata->load[0] / load_constant / cores * 100.0; // Converts load avg to percentage
		parameters->uptime = (double)systeminfodata->uptime / seconds_in_hour; // Converts to hours
	}

	return SUCCESS;
}

// Converts CPU load data into integer format
void parse_cpu_load_data(struct blob_attr *tb[], struct RawSystemInfoData *receivedData)
{
	int i = 0;
	struct blob_attr *cur;
	size_t rem;
	blobmsg_for_each_attr(cur, tb[CPU_LOAD_DATA], rem) {
		if (blobmsg_type(cur) != BLOBMSG_TYPE_INT32) {
			continue;
		}
		receivedData->load[i++] = blobmsg_get_u32(cur);
	}
}