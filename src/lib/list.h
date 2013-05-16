#pragma once

/* Copyright © 2011 Fritz Grimpen
 *
 * This file is part of Xelix.
 *
 * Xelix is free software: you can redistribute it and/or modify
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

#include <lib/generic.h>

/**
 * This type implements a circular doubly linked list with pre-allocated stub
 * nodes, which mark the beginning of the list and backreferences from nodes
 * to the stub and from stub to the list. This structure is optimized for
 * operations with O(1) and should not be used as Array.
 */
typedef struct list *list_t;
typedef struct list_node *list_node_t;

/**
 * Create empty list with stub nodes
 */
list_t list_create();

/**
 * Return stub node
 */
list_node_t list_head(list_t l);
/**
 * Return stub node
 */
list_node_t list_tail(list_t l);

/**
 * Return node which follows given node
 */
list_node_t list_next(list_node_t l);
/**
 * Return node which precedes given node
 */
list_node_t list_prev(list_node_t l);

/**
 * Insert given data after a given node and return new allocated node
 */
list_node_t list_insert_after(list_node_t n, void *data);
/**
 * Insert given data before a given node and return new allocated node
 */
list_node_t list_insert_before(list_node_t n, void *data);

/**
 * Replace data of given node with new data.
 */
list_node_t list_replace(list_node_t n, void *data);

/**
 * Append given data to list and return new allocated node
 */
list_node_t list_append(list_t l, void *data);

/**
 * Prepend given data to list and return new allocated node
 */
list_node_t list_prepend(list_t l, void *data);

/**
 * Return pointer to list for given list node
 */
list_t list_get_list(list_node_t n);

/**
 * Return data associated with given node
 */
void *list_data(list_node_t n);

/**
 * Remove node from list. If n is a stub node, then nothing will happen and this
 * function returns false, otherwise true.
 */
bool list_remove(list_node_t n);

/**
 * Return true if the given node is a stub, otherwise false.
 */
bool list_is_stub(list_node_t n);

/**
 * Destroy given list and all of its nodes
 */
void list_destroy(list_t l);

#define LIST_FOR_EACH(l, n) \
	for (list_node_t n = list_prev(list_head(l));\
			list_is_stub(n); n = list_next(n))

#define LIST_FOR_EACH_R(l, n) \
	for (list_node_t n = list_prev(list_head(l));\
			list_is_stub(n); n = list_prev(n))

