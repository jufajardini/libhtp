/***************************************************************************
 * Copyright (c) 2009-2010, Open Information Security Foundation
 * Copyright (c) 2009-2012, Qualys, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of the Qualys, Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

/**
 * @file
 * @author Ivan Ristic <ivanr@webkreator.com>
 */

#ifndef _DSLIB_H
#define	_DSLIB_H

typedef struct list_t list_t;
typedef struct list_array_t list_array_t;
typedef struct list_array_iterator_t list_array_iterator_t;
typedef struct list_linked_element_t list_linked_element_t;
typedef struct list_linked_t list_linked_t;

typedef struct htp_table_t htp_table_t;

#include "htp.h"
#include "bstr.h"

#ifdef __cplusplus
extern "C" {
#endif

// IMPORTANT This library is used internally by the parser and you should
//           not rely on it in your code. The implementation may change at
//           any time.

// Lists

#define list_push(L, E) (L)->push(L, E)
#define list_pop(L) (L)->pop(L)
#define list_empty(L) (L)->empty(L)
#define list_get(L, N) (L)->get((list_t *)L, N)
#define list_replace(L, N, E) (L)->replace((list_t *)L, N, E)
#define list_add(L, N) (L)->push(L, N)
#define list_size(L) (L)->size(L)
#define list_iterator_reset(L) (L)->iterator_reset(L)
#define list_iterator_next(L) (L)->iterator_next(L)
#define list_destroy(L) (*(L))->destroy(L)
#define list_shift(L) (L)->shift(L)

#define LIST_COMMON \
    int (*push)(list_t *, void *); \
    void *(*pop)(list_t *); \
    int (*empty)(const list_t *); \
    void *(*get)(const list_t *, size_t index); \
    int (*replace)(list_t *, size_t index, void *); \
    size_t (*size)(const list_t *); \
    void (*iterator_reset)(list_t *); \
    void *(*iterator_next)(list_t *); \
    void (*destroy)(list_t **); \
    void *(*shift)(list_t *)

struct list_t {
    LIST_COMMON;
};

struct list_linked_element_t {
    void *data;
    list_linked_element_t *next;
};

struct list_linked_t {
    LIST_COMMON;

    list_linked_element_t *first;
    list_linked_element_t *last;
};

struct list_array_t {
    LIST_COMMON;

    size_t first;
    size_t last;
    size_t max_size;
    size_t current_size;
    void **elements;

    size_t iterator_index;
};

struct list_array_iterator_t {
    list_array_t *l;
    size_t index;
};

list_t *list_linked_create(void);
void list_linked_destroy(list_linked_t **_l);

list_t *list_array_create(size_t size);
int list_array_push(list_array_t *l, void *e);
void list_array_int_iterator_reset(list_array_t *l);
void *list_array_int_iterator_next(list_array_t *l);
void list_array_destroy(list_array_t **_l);

/**
 * Initialize iterator for the given list. After this, repeatedly
 * invoking list_array_iterator_next() will walk the entire list.
 * 
 * @param[in] l
 * @param[in] it
 */
void list_array_iterator_init(list_array_t *l, list_array_iterator_t *it);

/**
 * Move the iterator to the next element in the list.
 * 
 * @param[in] it
 * @return Pointer to the next element, or NULL if no more elements are available.
 */
void *list_array_iterator_next(list_array_iterator_t *it);

// Table

struct htp_table_t {
    list_t *list;
};

/**
 * Add a new element to the table. The key will be copied, and the copy
 * managed by the table. The point of the element will be stored, but the
 * element itself will not be managed by the table.
 *
 * @param[in] table
 * @param[in] key
 * @param[in] element
 * @return HTP_OK on success, HTP_ERROR on failure.
 */
htp_status_t htp_table_add(htp_table_t *table, const bstr *key, const void *element);

/**
 * Add a new element to the table. The key provided will be adopted and managed
 * by the table. You should not keep a copy of the pointer to the key unless you're
 * certain that the table will live longer thant the copy. The table will make a
 * copy of the element pointer, but will not manage it.
 *
 * @param[in] table
 * @param[in] key
 * @param[in] element
 * @return
 */
htp_status_t htp_table_addn(htp_table_t *table, const bstr *key, const void *element);

/**
 * Remove all elements from the table. This function will free the keys,
 * but will do nothing about the elements in the table. If the elements need
 * freeing, you need to free them before invoking this function.
 *
 * @param[in] table
 */
void htp_table_clear(htp_table_t *table);

/**
 * Create a new table structure.
 *
 * @param[in] size
 * @return Newly created table instance, or NULL on failure.
 */
htp_table_t *htp_table_create(size_t size);

/**
 * Destroy a table. This function first frees the keys and then destroys the
 * table itself, but does nothing with the elements. If the elements need
 * freeing, you need to free them before invoking this function.
 *
 * @param[in]   table
 */
void htp_table_destroy(htp_table_t **_table);

/**
 * Retrieve the first element that matches the given bstr key.
 *
 * @param[in] table
 * @param[in] key
 * @return Matched element, or NULL if no elements match the key.
 */
void *htp_table_get(const htp_table_t *table, const bstr *key);

/**
 * Retrieve the first element that matches the given NUL-terminated key.
 *
 * @param[in] table
 * @param[in] ckey
 * @return Matched element, or NULL if no elements match the key.
 */
void *htp_table_get_c(const htp_table_t *table, const char *ckey);

/**
 * Advance iterator to the next table element.
 *
 * @param[in] table
 * @param[out] data A pointer to the pointer that will be set to the value of the next element.
 * @return Pointer to the key of the next element if there is one, NULL otherwise.
 */
bstr *htp_table_iterator_next(htp_table_t *table, void **data);

/**
 * Reset the table iterator.
 *
 * @param[in] table
 */
void htp_table_iterator_reset(htp_table_t *table);

/**
 * Return the size of the table.
 *
 * @param[in] table
 * @return table size
 */
size_t htp_table_size(const htp_table_t *table);

#ifdef __cplusplus
}
#endif

#endif	/* _DSLIB_H */

