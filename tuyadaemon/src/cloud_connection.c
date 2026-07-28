#include "cloud_connection.h"
#include "tuyalink_core.h"
#include "message_handler.h"

#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

void on_connected(tuya_mqtt_context_t* context, void* user_data)
{
    syslog(LOG_INFO, "Connected to Tuya.");
}

void on_disconnect(tuya_mqtt_context_t* context, void* user_data)
{
    syslog(LOG_INFO, "Disconnected from Tuya.");
}

int action_validate(char *data, char *action) 
{
    int valid_action_number = 1;
    char *valid_actions[1] = {"upload"};

    // Gets action code
    char *startstring = "\"actionCode\":\"";
    char *endstring = "\"}";

    char *result = strstr(data, startstring);
    int position = result - data + strlen(startstring); // Beginning of action code
    char *endresult = strstr(data + position, endstring);
    int endposition = endresult - data; // End of action code

    strncpy(action, data + position, endposition - position);

    // Validates action code
    for(int i = 0; i < valid_action_number; i++) {
        if(strcmp(action, valid_actions[i]) == 0) {
            return SUCCESS;
        }
    }
    return FAILURE;
}

void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
    char action[50]="";
    
    syslog(LOG_INFO, "On message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
    switch (msg->type) {
        // "Implement an action (not data set). This action should accept one parameter which could be a text."
        case THING_TYPE_ACTION_EXECUTE:
            syslog(LOG_INFO, "Get action:%s\r\n", msg->data_string); 

            if (action_validate(msg->data_string, action) != 0) {
                syslog(LOG_WARNING, "Received invalid action %s.", action); 
                break;
            }

            if (strcmp(action, "upload") == 0) {
                if (action_log(msg->data_string) != 0) {
                    syslog(LOG_ERR, "Error writing to /tmp/tuya_action.log. %s", strerror(errno)); 
                } 
            }
            
            break;
        default:
            break;
    }
}

