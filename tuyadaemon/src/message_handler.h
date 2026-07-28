#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include "tuyalink_core.h"

#define SUCCESS 0
#define FAILURE 1

#define REPORT_SIZE 4096
#define MESSAGE_SIZE 128
#define INTERFACE_NUMBER 10

struct Netinterface;

struct Parameters
{
	double totalram;
	double freeram;
	double uptime;
	double cpu;
	struct Netinterface *interfacelist;
};

int action_log(char* str);
int format_network_interfaces(tuya_mqtt_context_t* context, const char* device_id, struct Netinterface **interfacelist, char *finalreport);
int send_to_cloud(tuya_mqtt_context_t* context, const char* device_id, struct Parameters *parameters);
#endif