/*
 * DECT Information Elements
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup ie Information Elements
 * @{
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <asm/byteorder.h>

#include <dect/ie.h>
#include <libdect.h>

#if 0
#define refcnt_debug(fmt, ...)	dect_debug(DECT_DEBUG_UNKNOWN, fmt, ## __VA_ARGS__)
#else
#define refcnt_debug(fmt, ...)
#endif

/*
 * Information Elements
 */

struct dect_ie_common *dect_ie_alloc(const struct dect_handle *dh, size_t size)
{
	struct dect_ie_common *ie;

	ie = dect_zalloc(dh, size);
	if (ie == NULL)
		return NULL;
	ie->refcnt = 1;
	return ie;
}
EXPORT_SYMBOL(dect_ie_alloc);

void dect_ie_destroy(const struct dect_handle *dh, struct dect_ie_common *ie)
{
	dect_free(dh, ie);
}
EXPORT_SYMBOL(dect_ie_destroy);

struct dect_ie_common *__dect_ie_clone(const struct dect_handle *dh,
				       const struct dect_ie_common *ie,
				       size_t size)
{
	struct dect_ie_common *clone;

	clone = dect_ie_alloc(dh, size);
	if (clone == NULL)
		return NULL;
	memcpy(clone + 1, ie + 1, size - sizeof(*ie));
	return clone;
}
EXPORT_SYMBOL(__dect_ie_clone);

struct dect_ie_common *__dect_ie_hold(struct dect_ie_common *ie)
{
	if (ie == NULL)
		return NULL;
	refcnt_debug("IE %p: hold refcnt=%u\n", ie, ie->refcnt);
	dect_assert(ie->refcnt != 0);
	ie->refcnt++;
	return ie;
}
EXPORT_SYMBOL(__dect_ie_hold);

void __dect_ie_put(const struct dect_handle *dh, struct dect_ie_common *ie)
{
	if (ie == NULL)
		return;
	refcnt_debug("IE %p: release refcnt=%u\n", ie, ie->refcnt);
	dect_assert(ie->refcnt != 0);
	if (--ie->refcnt == 0)
		dect_ie_destroy(dh, ie);
}
EXPORT_SYMBOL(__dect_ie_put);

/*
 * Information Element lists
 */

static struct dect_ie_common ie_list_marker;

void dect_ie_list_init(struct dect_ie_list *ie)
{
	ie->common.next = &ie_list_marker;
	ie->list = NULL;
}
EXPORT_SYMBOL(dect_ie_list_init);

void __dect_ie_list_add(struct dect_ie_common *ie, struct dect_ie_list *iel)
{
	struct dect_ie_common **pprev;

	pprev = &iel->list;
	while (*pprev != NULL)
		pprev = &(*pprev)->next;

	ie->next = NULL;
	*pprev = ie;
}
EXPORT_SYMBOL(__dect_ie_list_add);

struct dect_ie_list *dect_ie_list_hold(struct dect_ie_list *iel)
{
	struct dect_ie_common *ie;

	refcnt_debug("IEL %p: hold\n", iel);
	dect_foreach_ie(ie, iel)
		__dect_ie_hold(ie);
	return iel;
}
EXPORT_SYMBOL(dect_ie_list_hold);

void dect_ie_list_put(const struct dect_handle *dh, struct dect_ie_list *iel)
{
	struct dect_ie_common *ie;

	refcnt_debug("IEL %p: release\n", iel);
	dect_foreach_ie(ie, iel)
		__dect_ie_put(dh, ie);
}
EXPORT_SYMBOL(dect_ie_list_put);

/*
 * Information Element collections
 */

void *dect_ie_collection_alloc(const struct dect_handle *dh, unsigned int size)
{
	struct dect_ie_collection *iec;

	iec = dect_zalloc(dh, size);
	if (iec == NULL)
		return NULL;
	iec->refcnt = 1;
	iec->size   = size;
	return iec;
}
EXPORT_SYMBOL(dect_ie_collection_alloc);

static void dect_ie_collection_free(const struct dect_handle *dh,
				    struct dect_ie_collection *iec)
{
	struct dect_ie_common *ie;
	unsigned int size;
	void **ptr;

	size = iec->size - sizeof(*iec);
	ptr = (void **)&iec->ie;

	while (size > 0) {
		if (*ptr == &ie_list_marker) {
			dect_ie_list_put(dh, *ptr);
			size -= sizeof(struct dect_ie_list);
			ptr = ((void *)ptr) + sizeof(struct dect_ie_list);
		} else {
			ie = *ptr;
			if (ie != NULL)
				__dect_ie_put(dh, ie);
			size -= sizeof(struct dect_ie_common *);
			ptr++;
		}
	}

	dect_free(dh, iec);
}
EXPORT_SYMBOL(dect_ie_collection_free);

void __dect_ie_collection_put(const struct dect_handle *dh, struct dect_ie_collection *iec)
{
	dect_assert(iec->refcnt != 0);
	if (--iec->refcnt > 0)
		return;
	dect_ie_collection_free(dh, iec);
}
EXPORT_SYMBOL(__dect_ie_collection_put);

struct dect_ie_collection *__dect_ie_collection_hold(struct dect_ie_collection *iec)
{
	dect_assert(iec->refcnt != 0);
	iec->refcnt++;
	return iec;
}
EXPORT_SYMBOL(__dect_ie_collection_hold);

/** @} */
