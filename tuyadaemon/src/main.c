#define _XOPEN_SOURCE 700

#include "interfaces_list.h"
#include "message_handler.h"
#include "argument_parsing.h"
#include "cloud_connection.h"
// #include "read_data.h"

#include "tuyalink_core.h"
#include "tuya_cacert.h"
#include "tuya_error_code.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>
#include <unistd.h>
#include <argp.h>
#include <signal.h>

tuya_mqtt_context_t client_instance;
volatile sig_atomic_t running = 1;

// Sigaction signal handler
static void sighandler(int signo){
    switch (signo) {
    case SIGTERM:
        syslog(LOG_INFO,"Terminating program via SIGTERM."); 
        break;
    case SIGINT:
        syslog(LOG_INFO,"Terminating program via SIGINT."); 
        break;
    case SIGQUIT:
        syslog(LOG_INFO,"Terminating program via SIGQUIT."); 
        break;
    }
    running = 0;
}

int main(int argc, char **argv)
{
    // Signal handler setup
    struct sigaction sig;
    sig.sa_handler = sighandler;
    sigemptyset (&sig.sa_mask);
    sig.sa_flags = 0;
    sigaction(SIGTERM, &sig, NULL);
    sigaction(SIGINT, &sig, NULL);
    sigaction(SIGQUIT, &sig, NULL);

    // Variable definitions
    struct Parameters parameters; // Parameters to send to cloud
    parameters.freeram = 0;
    parameters.totalram = 0;
    parameters.uptime = 0;
    parameters.cpu = 0;
    parameters.interfacelist = NULL;

    int period = 60; // Data sending period in seconds
    time_t prev_time; // Reference time for period calculation
    struct arguments arguments; // Arguments for argp
    int ret; // Return code

    // Syslog setup
    setlogmask(LOG_UPTO (LOG_DEBUG));
    openlog("tuyadaemon", LOG_CONS | LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_LOCAL0);
    
    // Argp setup
	argument_parsing(argc, argv, &arguments);

    // Tuya connection setup
	ret = OPRT_OK; 

    tuya_mqtt_context_t* client = &client_instance;

    ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t) {
        .host = "m1.tuyacn.com",
        .port = 8883,
        .cacert = tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = arguments.deviceid,
        .device_secret = arguments.devicesecret,
        .keepalive = 100,
        .timeout_ms = 2000,
        .on_connected = on_connected,
        .on_disconnect = on_disconnect,
        .on_messages = on_messages
    });
    if (ret != OPRT_OK) {
        syslog(LOG_ERR,"Tuya MQTT initialization error.");
        goto cleanup;
    }

    ret = tuya_mqtt_connect(client);
    if(ret != OPRT_OK) {
        syslog(LOG_ERR,"Tuya MQTT connection error.");
        goto cleanup;
    }
    prev_time = time(NULL);

    // Main loop
    while(running){
        // Loop to receive packets, and handles client keepalive 
        if (tuya_mqtt_loop(client) != 0) {
            syslog(LOG_ERR, "Tuya MQTT loop error.");
        }
        
        // Periodically sending data to cloud
        if ((time(NULL) - prev_time) >= period) { 
            if (send_to_cloud(client, arguments.deviceid, &parameters) == SUCCESS) {
                syslog(LOG_INFO, "Sent all parameters to cloud successfully.");
            } else {
                syslog(LOG_ERR, "Failure sending parameters to cloud.");
            }

            prev_time = time(NULL);
        }
    }
cleanup:
    tuya_mqtt_disconnect(client);
    tuya_mqtt_deinit(client);
    closelog();
    return ret;
}

