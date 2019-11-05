#ifndef OUROS_LIST_H
#define OUROS_LIST_H

#include <ouros/type.h>

struct list_node_struct {
	struct list_node_struct *prev;
	struct list_node_struct *next;
};
typedef struct list_node_struct list_node;

struct list_struct {
	list_node *head;
	int size;
};
typedef struct list_struct list;

void init_list(list *list);
void init_list_node(list_node *node);

int get_size(list *list);
bool is_empty(list *list);

list_node *get_head(list *list);
list_node *get_prev(list_node *node);
list_node *get_next(list_node *node);

int get_pos_idx(list *list, int index);
int list_valid_idx(list *list, int index);
list_node *get_list_node(list *list, int index);
int find_list_node(list *list, list_node *node);

int insert_list_node(list *list, list_node *node, int index);
int append_list_node(list *list, list_node *node);

list_node *delete_list_node(list *list, list_node *node);
list_node *delete_list_node_by_idx(list *list, int index);
list_node *pop_list_node(list *list);

void traversal_list(list *list, void (*f)(list_node *));

#endif // OUROS_LIST_H
