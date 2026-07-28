#ifndef CLOUDCONNECTION_H
#define CLOUDCONNECTION_H
#include "tuyalink_core.h"

void on_connected(tuya_mqtt_context_t* context, void* user_data);
void on_disconnect(tuya_mqtt_context_t* context, void* user_data);
int action_validate(char *data, char *action) ;
void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg);

#endif