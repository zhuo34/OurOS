#include "list.h"

void init_list(list *list)
{
	if (list) {
		list->head = nullptr;
		list->size = 0;
	}
}

void init_list_node(list_node *node)
{
	if (node) {
		node->next = node->prev = nullptr;
	}
}

int get_size(list *list)
{
	if (!list)
		return 0;
	return list->size;
}

bool is_empty(list *list)
{
	return get_size(list) == 0;
}

list_node *get_head(list *list)
{
	if (!list)
		return nullptr;
	return list->head;
}

list_node *get_prev(list_node *node)
{
	if (!node)
		return nullptr;
	return node->prev;
}

list_node *get_next(list_node *node)
{
	if (!node)
		return nullptr;
	return node->next;
}

int get_pos_idx(list *list, int index)
{
	return index < 0 ? (list->size + index) : index;
}

int list_valid_idx(list *list, int index)
{
	if (!list)
		return 0;
	if (index >= 0 && index < list->size)
		return 1;
	else if (index < 0 && -index <= list->size)
		return 1;
	else
		return 0;
}

list_node *get_list_node(list *list, int index)
{
	list_node *ret = nullptr;
	if (!list_valid_idx(list, index)) {
		return ret;
	}
	int pos_index = get_pos_idx(list, index);
	int go_back = 0;
	list_node *p = list->head;
	if (pos_index > list->size / 2)
		go_back = 1;
	if (go_back) {
		pos_index = list->size - pos_index;
		while (pos_index--) {
			p = p->prev;
		}
	} else {
		while (pos_index--) {
			p = p->next;
		}
	}
	ret = p;
	return ret;
}

int find_list_node(list *list, list_node *node)
{
	if (!list) return -1;
	list_node *p = list->head;
	int idx = 0;
	int size = list->size;
	while (size--) {
		if (p == node)
			return list->size - size - 1;
		p = p->next;
	}
	return -1;
}

int insert_list_node(list *list, list_node *node, int index)
{
	if (!list || !node)
		return 0;
	int pos_idx = get_pos_idx(list, index);
	if (pos_idx > list->size || pos_idx < 0)
		return 0;
	if (pos_idx == list->size)
		return append_list_node(list, node);
	if (index == 0) {
		int ret = append_list_node(list, node);
		if (ret) list->head = list->head->prev;
		return ret;
	}
	list_node *p = get_list_node(list, index);
	list_node *head = list->head;
	list->head = p;
	int ret = insert_list_node(list, node, 0);
	list->head = head;
	return ret;
}

int append_list_node(list *list, list_node *node)
{
	int ret = 0;
	if (!list || !node) {
		ret = 0;
	} else if (list->size == 0) {
		node->next = node;
		node->prev = node;
		list->head = node;
		ret = 1;
	} else {
		node->next = list->head;
		node->prev = list->head->prev;
		node->prev->next = node;
		list->head->prev = node;
		ret = 1;
	}
	list->size += ret;
	return ret;
}

list_node *delete_list_node(list *list, list_node *node)
{
	list_node *ret = nullptr;
	if (!find_list_node(list, node))
		return ret;
	list_node *head = list->head;
	list->head = node->next;
	ret = pop_list_node(list);
	list->head = head;
	return ret;
}

list_node *delete_list_node_by_idx(list *list, int index)
{
	if (!list || !list_valid_idx(list, index))
		return nullptr;
	int pos_idx = get_pos_idx(list, index);
	if (pos_idx == 0 || list->size == 1) {
		list->head = list->head->next;
		return pop_list_node(list);
	}

	list_node *head = list->head;
	list_node *p = get_list_node(list, pos_idx);
	list->head = p->next;
	list_node *ret = pop_list_node(list);
	list->head = head;
	return ret;
}

list_node *pop_list_node(list *list)
{
	if (!list || list->size == 0)
		return nullptr;
	list_node *ret = nullptr;
	if (list->size == 1) {
		ret = list->head;
		list->head = nullptr;
	} else {
		ret = list->head->prev;
		ret->prev->next = ret->next;
		ret->next->prev = ret->prev;
	}

	list->size -= ret != nullptr;
	return ret;
}

void traversal_list(list *list, void (*f)(list_node *))
{
	if (!list)
		return;
	list_node *p = list->head;
	int size = list->size;
	while (size--) {
		f(p);
		p = p->next;
	}
}
