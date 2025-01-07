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

#pragma once

#include <stdint.h>


typedef struct hash_table_entry
{
	const char* key;
	void* value;

	uint32_t len;
	uint32_t next;
}
hash_table_entry_t;


typedef struct hash_table
{
	uint32_t* buckets;
	hash_table_entry_t* entries;

	uint32_t bucket_count;
	uint32_t entries_used;
	uint32_t entries_size;
	uint32_t free_entry;
}
hash_table_t;


typedef void
(*hash_table_for_each_fn_t)(
	const char* key,
	uint32_t len,
	void* value,
	void* data
	);


extern void
hash_table_init(
	hash_table_t* table,
	uint32_t bucket_count
	);


extern void
hash_table_free(
	hash_table_t* table
	);


extern void
hash_table_clear(
	hash_table_t* table
	);


extern void
hash_table_for_each(
	hash_table_t* table,
	hash_table_for_each_fn_t fn,
	void* data
	);


/* false if not found */
extern bool
hash_table_has(
	hash_table_t* table,
	const char* key
	);


/* false if already exists */
extern bool
hash_table_add(
	hash_table_t* table,
	const char* key,
	void* value
	);


/* false if this is a new entry */
extern bool
hash_table_set(
	hash_table_t* table,
	const char* key,
	void* value
	);


/* false if not found */
extern bool
hash_table_modify(
	hash_table_t* table,
	const char* key,
	void* value
	);


extern void*
hash_table_get(
	hash_table_t* table,
	const char* key
	);


/* false if not found */
extern bool
hash_table_del(
	hash_table_t* table,
	const char* key
	);
