#ifndef READSYSTEMDATA_H
#define READSYSTEMDATA_H

#include <stdio.h>
#include <stdint.h>

struct RawSystemInfoData {
	int total;
	int free;
	int load [3];
	int uptime;
};

struct Parameters;
struct blob_attr;
struct ubus_context;

int read_ubus_data(struct Parameters *parameters);
void parse_cpu_load_data(struct blob_attr *tb[], struct RawSystemInfoData *receivedData);
int read_system_info_data(struct Parameters *parameters, struct RawSystemInfoData *systeminfodata, struct ubus_context *ctx, uint32_t *id);

#endif