#ifndef OUROS_LIST_H
#define OUROS_LIST_H

#include <ouros/type.h>

#define nullptr 0
typedef unsigned int uint;
struct list_node_struct {
	struct list_node_struct *prev;
	struct list_node_struct *next;
};
typedef struct list_node_struct list_node;

struct list_struct {
	list_node *head;
	uint size;
};
typedef struct list_struct list;

uint get_size(list *list);
list_node *get_head(list *list);
list_node *get_prev(list_node *node);
list_node *get_next(list_node *node);

list_node *find_list_node(list *list, uint index);
int insert_list_node(list *list, list_node *node, uint index);
int append_list_node(list *list, list_node *node);

list_node *delete_list_node(list *list, uint index);
list_node *pop_list_node(list *list);

void traversal_list(list *list, void (*f)(list_node *));

#endif // OUROS_LIST_H
