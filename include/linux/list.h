#ifndef LINUX_LIST_H
#define LINUX_LIST_H

#include <ouros/type.h>
#include <ouros/utils.h>

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->prev = list;
	list->next = list;
}

static inline void __list_add(struct list_head *new_node, struct list_head *prev, struct list_head *next)
{
	next->prev = new_node;
	new_node->next = next;
	new_node->prev = prev;
	prev->next = new_node;
}

static inline void list_add(struct list_head *new_node, struct list_head *head)
{
	__list_add(new_node, head, head->next);
}

static inline void list_add_tail(struct list_head *new_node, struct list_head *head)
{
	__list_add(new_node, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

static inline void list_del_init(struct list_head *entry) {
	list_del(entry);
    INIT_LIST_HEAD(entry);
}

static inline void list_replace(struct list_head *old, struct list_head *new_node)
{
	new_node->next = old->next;
	new_node->next->prev = new_node;
	new_node->prev = old->prev;
	new_node->prev->next = new_node;
}

static inline void list_replace_init(struct list_head *old, struct list_head *new_node)
{
	list_replace(old, new_node);
	INIT_LIST_HEAD(old);
}

static inline void list_swap(struct list_head *a, struct list_head *b)
{
	// if b before a, may error
	struct list_head temp;
	temp.prev = b->prev;
	temp.next = b->next;

	list_del(a);
	list_del(b);
	list_add(b, a->prev);
	list_add_tail(a, temp.next);
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
	list_del(list);
	list_add(list, head);
}

static inline void list_move_tail(struct list_head *list, struct list_head *head)
{
	list_del(list);
	list_add_tail(list, head);
}

static inline int list_is_first(const struct list_head *list, const struct list_head *head)
{
	return list->prev == head;
}

static inline int list_is_last(const struct list_head *list, const struct list_head *head)
{
	return list->next == head;
}

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_next_entry(pos, member))

static inline bool list_contain_node(const struct list_head *node, const struct list_head *head)
{
	bool ret = false;
	struct list_head *p;
	list_for_each(p, head) {
		if (p == node) {
			ret = true;
			break;
		}
	}
	return ret;
}

static inline void list_sort(struct list_head *head, 
							 int (*cmp)(const struct list_head *, const struct list_head *))
{
	struct list_head *start;
	struct list_head *end;
	for (end = head; end != head->next; end = end->prev) {
		for(start = head->next; start->next != end; start = start->next) {
			if(cmp(start, start->next) > 0) {
				list_swap(start, start->next);
				start = start->prev;
			}
		}
	}
}

#endif // LINUX_LIST_H