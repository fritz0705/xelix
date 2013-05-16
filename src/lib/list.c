/* list.c: Generic list structure
 * Copyright © 2011 Fritz Grimpen
 *
 * This file is part of Xelix.
 *
 * Xelix is kfree software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Xelix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Xelix. If not, see <http://www.gnu.org/licenses/>.
 */

#include "list.h"
#include <memory/kmalloc.h>

struct list
{
	list_node_t stub;
};

struct list_node
{
	list_node_t next;
	list_node_t prev;
	list_node_t lstub;
	bool stub:1;
	void *data;
};

list_t list_create()
{
	list_t l = kmalloc(sizeof *l);
	if (!l)
		return NULL;

	l->stub = kmalloc(sizeof *l->stub);

	if (!l->stub)
	{
		kfree(l);
		return NULL;
	}

	*l->stub = (struct list_node){
		.next = l->stub,
		.prev = l->stub,
		.lstub = l->stub,
		.stub = true,
		.data = l
	};

	return l;
}

list_node_t list_head(list_t l)
{
	return l->stub;
}

list_node_t list_tail(list_t l)
{
	return l->stub;
}

list_node_t list_next(list_node_t l)
{
	return l->next;
}

list_node_t list_prev(list_node_t l)
{
	return l->prev;
}

list_node_t list_insert_after(list_node_t n, void *data)
{
	/* Allocate new list node */
	list_node_t new_n = kmalloc(sizeof *new_n);
	/* Fill new list node with parameters */
	*new_n = (struct list_node){
		.next = n->next,
		.prev = n,
		.lstub = n->lstub,
		.stub = false,
		.data = data
	};

	n->next->prev = new_n;
	n->next = new_n;

	return new_n;
}

list_node_t list_insert_before(list_node_t n, void *data)
{
	/* Allocate new list node */
	list_node_t new_n = kmalloc(sizeof *new_n);
	/* Fill new list node with parameters */
	*new_n = (struct list_node){
		.next = n,
		.prev = n->prev,
		.lstub = n->lstub,
		.stub = false,
		.data = data
	};

	n->prev->next = new_n;
	n->prev = new_n;

	return new_n;
}

list_node_t list_replace(list_node_t n, void *data)
{
	n->data = data;
	return n;
}

void *list_data(list_node_t n)
{
	return n->data;
}

bool list_remove(list_node_t n)
{
	if (n->stub)
		return false;

	n->next->prev = n->prev;
	n->prev->next = n->next;

	kfree(n);

	return true;
}

bool list_is_stub(list_node_t n)
{
	return n->stub;
}

void list_destroy(list_t l)
{
	(void)l;
}

list_t list_get_list(list_node_t n)
{
	return n->lstub->data;
}

list_node_t list_append(list_t l, void *data)
{
	return list_insert_before(l->stub, data);
}

list_node_t list_prepend(list_t l, void *data)
{
	return list_insert_after(l->stub, data);
}
