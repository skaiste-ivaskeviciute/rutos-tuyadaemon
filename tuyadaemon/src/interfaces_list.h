#ifndef INTERFACESLIST_H
#define INTERFACESLIST_H
#define LENGTH 50
#define PARAMETER_NUMBER 5

struct Netinterface 
{
	char name[LENGTH];
	char ip[LENGTH];
	char netmask[LENGTH];
	char received[LENGTH];
	char transmitted[LENGTH];
	struct Netinterface *next;
};

void delete_list(struct Netinterface **list);
struct Netinterface* create_node(char *name, char *address, char *netmask, char *received, char *transmitted);
void add_to_list(struct Netinterface **list, struct Netinterface *node);

#endif