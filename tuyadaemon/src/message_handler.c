#include "message_handler.h"
#include "read_data.h"
#include "interfaces_list.h"
#include "tuyalink_core.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <errno.h>

// After receiving a parameter from an action, puts it into /tmp/tuya_action.log
int action_log(char* str)
{
    FILE *file;
    time_t now;
    struct tm *t;
    char buffer[256];
    char *startstring = "text\":\"";
    char *endstring = "\"},\"actionCode";
    char *result = strstr(str, startstring);
    char *endresult = strstr(str, endstring);
    int position = result - str + strlen(startstring); // Shifts forward to skip string text":"
    int endposition = endresult - str; // Ends before string "},"actionCode"

    file = fopen("/tmp/tuya_action.log", "a");
    if (file == NULL) {
        return FAILURE;
    }

    // Prints a timestamp to the log
    now = time(NULL);
    t = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);
    if (fprintf(file, "[%s] ", buffer) < 0) {
        return FAILURE;
    }
    
    // Prints the message to the log
    for (position; position < endposition; position++) {
        if (fputc(str[position], file) == EOF) {
            return FAILURE;
        }
    }
    fputc('\n', file);
    fclose(file);

    return SUCCESS;
}

// Reads all parameters & sends them to the cloud
int send_to_cloud(tuya_mqtt_context_t* context, const char* device_id, struct Parameters *parameters) 
{
    int status = SUCCESS;
    char *report = (char *) malloc(sizeof(char) * REPORT_SIZE);
    char *tmp = (char *) malloc(sizeof(char) * MESSAGE_SIZE);
    
    sprintf(report,"{");

    // UBUS information
    if (read_ubus_data(parameters) == 0) {
        sprintf(tmp, "\"totalram\":%lf,\"freeram\":%lf,\"cpu\":%lf,\"uptime\":%lf",
            parameters->totalram, parameters->freeram, parameters->cpu, parameters->uptime);
        strcat(report,tmp);
        strcat(report,",");
        format_network_interfaces(context,device_id,&(parameters->interfacelist), report);
        delete_list(&(parameters->interfacelist));
    } else {
        syslog(LOG_ERR, "Error reading UBUS data.");  
        status = FAILURE;
    }

    strcat(report,"}");

    tuyalink_thing_property_report(context, device_id, report);
    
    free(report);
    free(tmp);
    return status;
}

// Formats network interface data
int format_network_interfaces(tuya_mqtt_context_t* context, const char* device_id, struct Netinterface **interfacelist, char *destination)
{
    struct Netinterface *temp = *interfacelist;
    char *tmpchar = (char *) malloc(sizeof(char) * MESSAGE_SIZE);
    char *netreport = (char *) malloc(sizeof(char) * MESSAGE_SIZE * INTERFACE_NUMBER);

    sprintf(netreport,"\"interfaces\":[");
    while (temp != NULL) {
        //sprintf(tmpchar, "\"{\\\"name\\\":\\\"%s\\\",\\\"ip_addr\\\":\\\"%s\\\",\\\"netmask\\\":\\\"%s\\\",\\\"received\\\":\\\"%s\\\",\\\"transmitted\\\":\\\"%s\\\"}\"",
         //   temp->name, temp->ip, temp->netmask, temp->received, temp->transmitted);
        sprintf(tmpchar, "\"{\\\"name\\\":\\\"%s\\\",", temp->name);
        strcat(netreport,tmpchar);
        sprintf(tmpchar, "\\\"ip_addr\\\":\\\"%s\\\",", temp->ip);
        strcat(netreport,tmpchar);
        sprintf(tmpchar, "\\\"netmask\\\":\\\"%s\\\",", temp->netmask);
        strcat(netreport,tmpchar);
        sprintf(tmpchar, "\\\"received\\\":\\\"%s\\\",", temp->received);
        strcat(netreport,tmpchar);
        sprintf(tmpchar, "\\\"transmitted\\\":\\\"%s\\\"}\"", temp->transmitted);
        strcat(netreport,tmpchar);
        
        if (temp->next != NULL)
            strcat(netreport,",");
        else
            strcat(netreport,"]");
         temp = temp->next;
    }

    strcat(destination, netreport);

    free(tmpchar);
    free(netreport);
    return SUCCESS;
}