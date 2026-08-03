#include "interfaces_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Deletes linked list
void delete_list(struct Netinterface **list)
{
    struct Netinterface *to_delete = *list;
    while (*list != NULL) {
        *list = (*(list))->next;
        free(to_delete);
        to_delete = *list;
    }
}

// Creates a linked list node from given struct members
struct Netinterface* create_node(char *name, char *address, char *netmask, char *received, char *transmitted)
{
    struct Netinterface *list = NULL;
    list = (struct Netinterface*) malloc(sizeof(struct Netinterface));

    if (list == NULL) {
        return NULL;
    }

	strcpy(list->name, name);
	strcpy(list->ip, address);
	strcpy(list->netmask, netmask);
    strcpy(list->received, received);
    strcpy(list->transmitted, transmitted);
    
    list->next = NULL;
    return list;
}

// Adds a node to the linked list
void add_to_list(struct Netinterface **list, struct Netinterface *node)
{
    struct Netinterface *temp = *list;

    if (temp == NULL) {
        *list = node;
        return; 
    }

    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = node;
}