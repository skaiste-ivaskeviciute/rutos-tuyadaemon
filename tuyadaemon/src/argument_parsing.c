#include "argument_parsing.h"
#include "make_daemon.h"

#include <argp.h>
#include <syslog.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

const char *argp_program_version = "Tuya Daemon 1.0";
static char args_doc[] = "";
static char doc[] = "Tuya Daemon";

static struct argp_option options[] = {
	{"productid", 'p', "VALUE", 0, "Specify product id"},
	{"deviceid", 'd', "VALUE", 0, "Specify device id"},
	{"devicesecret", 's', "VALUE", 0, "Specify device secret"},
    {"daemon", 'D', NULL, 0, "Run as daemon"}, // Optional
	{0},
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = (struct arguments*)state->input;

    switch (key) {
        case 'p':
            arguments->productid = arg;
            break;
        case 'd':
            arguments->deviceid = arg;
            break;
        case 's':
            arguments->devicesecret = arg;
            break;
        case 'D':
            arguments->daemon = 1;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

void remove_whitespace(char *string)
{
    char* d = string;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*string++ = *d++);

    string[strlen(string) - 1] = '\0';
}

int argument_parsing(int argc, char **argv, struct arguments *arguments) 
{
    // Default values
    arguments->productid = NULL;
    arguments->deviceid = NULL;
    arguments->devicesecret = NULL;
    arguments->daemon = 0;

    argp_parse(&argp, argc, argv, 0, 0, &(*arguments));

    if ((arguments->productid == NULL) || (arguments->deviceid == NULL) || (arguments->devicesecret == NULL)) {
        syslog(LOG_ERR, "Missing arguments (productID, deviceID, deviceSecret). Exiting program.");
        exit(1);
    } 

    remove_whitespace(arguments->productid);
    remove_whitespace(arguments->deviceid);
    remove_whitespace(arguments->devicesecret);

    if (arguments->productid[0] == '\0'|| arguments->deviceid[0] == '\0' || arguments->devicesecret[0] == '\0') {
        syslog(LOG_ERR, "Empty arguments (productID, deviceID, deviceSecret). Exiting program.");
        exit(1);
    }

    // Daemon setup
    if (arguments->daemon == 1) {
        make_daemon();
        syslog(LOG_INFO, "Running tuyadaemon program as daemon.");
    } else {
        syslog(LOG_INFO, "Running tuyadaemon program in foreground.");
    }
}

