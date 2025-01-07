/*
 *   Copyright 2024-2025 Franciszek Balcerak
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <DiepDesktop/shared/hash.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>

#include <string.h>


private uint32_t
hash_table_hash(
	const char* key,
	uint32_t* len
	)
{
	const char* key_start = key;
	uint32_t hash = 0x811c9dc5;

	while(*key)
	{
		hash ^= *(key++);
		hash *= 0x01000193;
	}

	*len = key - key_start;
	return hash;
}


void
hash_table_init(
	hash_table_t* table,
	uint32_t bucket_count
	)
{
	table->bucket_count = bucket_count;
	assert_neq(table->bucket_count, 0);

	table->entries_used = 1;
	table->entries_size = 0;
	table->free_entry = 0;

	table->buckets = alloc_calloc(sizeof(uint32_t) * table->bucket_count);
	assert_not_null(table->buckets);

	table->entries = NULL;
}


void
hash_table_free(
	hash_table_t* table
	)
{
	alloc_free(sizeof(uint32_t) * table->bucket_count, table->buckets);
	alloc_free(sizeof(hash_table_entry_t) * table->entries_size, table->entries);
}


void
hash_table_clear(
	hash_table_t* table
	)
{
	(void) memset(table->buckets, 0, sizeof(uint32_t) * table->bucket_count);

	alloc_free(sizeof(hash_table_entry_t) * table->entries_size, table->entries);

	table->entries = NULL;
	table->entries_used = 1;
	table->entries_size = 0;
	table->free_entry = 0;
}


void
hash_table_for_each(
	hash_table_t* table,
	hash_table_for_each_fn_t fn,
	void* data
	)
{
	uint32_t* bucket = table->buckets;
	uint32_t* bucket_end = bucket + table->bucket_count;

	for(; bucket < bucket_end; ++bucket)
	{
		uint32_t entry_idx = *bucket;
		for(hash_table_entry_t* entry = &table->entries[entry_idx]; entry_idx; entry = &table->entries[entry_idx])
		{
			fn(entry->key, entry->len, entry->value, data);
			entry_idx = entry->next;
		}
	}
}


private uint32_t
hash_table_get_entry(
	hash_table_t* table
	)
{
	if(table->free_entry)
	{
		uint32_t entry = table->free_entry;
		table->free_entry = table->entries[entry].next;
		return entry;
	}

	if(table->entries_used >= table->entries_size)
	{
		uint32_t new_size = (table->entries_used << 1) | 1;
		table->entries = alloc_remalloc(
			sizeof(hash_table_entry_t) * table->entries_size,
			table->entries,
			sizeof(hash_table_entry_t) * new_size
			);
		assert_not_null(table->entries);

		table->entries_size = new_size;
	}

	return table->entries_used++;
}


private void
hash_table_ret_entry(
	hash_table_t* table,
	uint32_t entry
	)
{
	table->entries[entry].next = table->free_entry;
	table->free_entry = entry;
}


bool
hash_table_has(
	hash_table_t* table,
	const char* key
	)
{
	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	uint32_t entry_idx = table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[entry_idx]; entry_idx; entry = &table->entries[entry_idx])
	{
		if(len == entry->len && strcasecmp(key, entry->key) == 0)
		{
			return true;
		}

		entry_idx = entry->next;
	}

	return false;
}


bool
hash_table_add(
	hash_table_t* table,
	const char* key,
	void* value
	)
{
	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	uint32_t* next = &table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[*next]; *next; entry = &table->entries[*next])
	{
		if(len == entry->len && strcasecmp(key, entry->key) == 0)
		{
			return false;
		}

		next = &entry->next;
	}

	uint32_t entry = hash_table_get_entry(table);
	table->entries[entry] =
	(hash_table_entry_t)
	{
		.key = key,
		.value = value,
		.next = 0
	};

	*next = entry;
	return true;
}


bool
hash_table_set(
	hash_table_t* table,
	const char* key,
	void* value
	)
{
	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	uint32_t* next = &table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[*next]; *next; entry = &table->entries[*next])
	{
		if(len == entry->len && strcasecmp(key, entry->key) == 0)
		{
			entry->value = value;
			return true;
		}

		next = &entry->next;
	}

	uint32_t entry = hash_table_get_entry(table);
	table->entries[entry] =
	(hash_table_entry_t)
	{
		.key = key,
		.value = value,
		.next = 0
	};

	*next = entry;
	return false;
}


bool
hash_table_modify(
	hash_table_t* table,
	const char* key,
	void* value
	)
{
	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	uint32_t* next = &table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[*next]; *next; entry = &table->entries[*next])
	{
		if(len == entry->len && strcasecmp(key, entry->key) == 0)
		{
			entry->value = value;
			return true;
		}

		next = &entry->next;
	}

	return false;
}


void*
hash_table_get(
	hash_table_t* table,
	const char* key
	)
{
	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	uint32_t entry_idx = table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[entry_idx]; entry_idx; entry = &table->entries[entry_idx])
	{
		if(len == entry->len && strcasecmp(key, entry->key) == 0)
		{
			return entry->value;
		}

		entry_idx = entry->next;
	}

	return NULL;
}


bool
hash_table_del(
	hash_table_t* table,
	const char* key
	)
{
	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	uint32_t* next = &table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[*next]; *next; entry = &table->entries[*next])
	{
		if(len == entry->len && strcasecmp(key, entry->key) == 0)
		{
			uint32_t next_idx = *next;
			*next = table->entries[next_idx].next;
			hash_table_ret_entry(table, next_idx);
			return true;
		}

		next = &table->entries[*next].next;
	}

	return false;
}
