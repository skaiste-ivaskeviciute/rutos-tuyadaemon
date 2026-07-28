#ifndef ARGUMENTPARSING_H
#define ARGUMENTPARSING_H

struct arguments
{
	char *productid;
	char *deviceid;
	char *devicesecret;
    char daemon;
};

void remove_whitespace(char *string);
int argument_parsing(int argc, char ** argv, struct arguments *arguments) ;

#endif