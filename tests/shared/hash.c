/*
 *   Copyright 2025 Franciszek Balcerak
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

#include <string.h>


void assert_used
test_should_pass__hash_table_init_free(
	void
	)
{
	hash_table_t table;
	hash_table_init(&table, 256);
	hash_table_free(&table);
}


void assert_used
test_should_fail__hash_table_init_null(
	void
	)
{
	hash_table_init(NULL, 1);
}


void assert_used
test_should_fail__hash_table_init_zero_buckets(
	void
	)
{
	hash_table_t table;
	hash_table_init(&table, 0);
}


void assert_used
test_should_fail__hash_table_free_null(
	void
	)
{
	hash_table_free(NULL);
}


typedef struct for_each_data
{
	const char* key;
	uint32_t len;
	void* value;
}
for_each_data_t;


static void
hash_table_for_each_fn(
	const char* key,
	uint32_t len,
	void* value,
	void* data
	)
{
	for_each_data_t** ptr = data;
	for_each_data_t* cur = *ptr;

	cur->key = key;
	cur->len = len;
	cur->value = value;

	*ptr = cur + 1;
}


void assert_used
test_should_pass__hash_table_functions(
	void
	)
{
	hash_table_t table;
	hash_table_init(&table, 8);

	assert_false(hash_table_has(&table, "foo"));
	assert_false(hash_table_has(&table, "bar"));
	assert_false(hash_table_modify(&table, "foo", (void*) 1));
	assert_false(hash_table_del(&table, "foo"));

	hash_table_clear(&table);

	assert_true(hash_table_add(&table, "foo", (void*) 1));
	assert_true(hash_table_has(&table, "foo"));
	assert_eq(hash_table_get(&table, "foo"), (void*) 1);

	assert_false(hash_table_set(&table, "bar", (void*) 2));
	assert_false(hash_table_add(&table, "bar", (void*) 3));

	assert_true(hash_table_has(&table, "foo"));
	assert_eq(hash_table_get(&table, "foo"), (void*) 1);

	assert_true(hash_table_has(&table, "bar"));
	assert_eq(hash_table_get(&table, "bar"), (void*) 2);

	assert_true(hash_table_modify(&table, "foo", (void*) 3));
	assert_eq(hash_table_get(&table, "foo"), (void*) 3);

	assert_true(hash_table_set(&table, "foo", (void*) 1));
	assert_eq(hash_table_get(&table, "foo"), (void*) 1);

	for_each_data_t data[4];
	for_each_data_t* data_end = data;
	hash_table_for_each(&table, hash_table_for_each_fn, &data_end);
	assert_eq(data_end, data + 2);

	for(for_each_data_t* cur = data; cur != data_end; ++cur)
	{
		if(strcmp(cur->key, "foo") == 0)
		{
			assert_eq(cur->len, 3);
			assert_eq(cur->value, (void*) 1);
		}
		else if(strcmp(cur->key, "bar") == 0)
		{
			assert_eq(cur->len, 3);
			assert_eq(cur->value, (void*) 2);
		}
		else
		{
			assert_unreachable();
		}
	}

	assert_true(hash_table_del(&table, "foo"));
	assert_false(hash_table_has(&table, "foo"));
	assert_false(hash_table_del(&table, "foo"));
	assert_true(hash_table_has(&table, "bar"));
	assert_eq(hash_table_get(&table, "bar"), (void*) 2);

	data_end = data;
	hash_table_for_each(&table, hash_table_for_each_fn, &data_end);
	assert_eq(data_end, data + 1);

	for(for_each_data_t* cur = data; cur != data_end; ++cur)
	{
		assert_false(strcmp(cur->key, "bar"));
		assert_eq(cur->len, 3);
		assert_eq(cur->value, (void*) 2);
	}

	hash_table_clear(&table);

	assert_false(hash_table_has(&table, "foo"));
	assert_false(hash_table_has(&table, "bar"));

	data_end = data;
	hash_table_for_each(&table, hash_table_for_each_fn, &data_end);
	assert_eq(data_end, data + 0);

	hash_table_free(&table);
}


void assert_used
test_should_fail__hash_table_has_null_table(
	void
	)
{
	hash_table_has(NULL, "foo");
}


void assert_used
test_should_fail__hash_table_has_null_key(
	void
	)
{
	hash_table_t table;
	hash_table_init(&table, 1);
	hash_table_has(&table, NULL);
}


void assert_used
test_should_fail__hash_table_has_null(
	void
	)
{
	hash_table_has(NULL, NULL);
}


void assert_used
test_should_fail__hash_table_add_null_table(
	void
	)
{
	hash_table_add(NULL, "foo", (void*) 1);
}


void assert_used
test_should_fail__hash_table_add_null_key(
	void
	)
{
	hash_table_t table;
	hash_table_init(&table, 1);
	hash_table_add(&table, NULL, (void*) 1);
}


void assert_used
test_should_fail__hash_table_add_null(
	void
	)
{
	hash_table_add(NULL, NULL, (void*) 1);
}


void assert_used
test_should_fail__hash_table_get_null_table(
	void
	)
{
	hash_table_get(NULL, "foo");
}


void assert_used
test_should_fail__hash_table_get_null_key(
	void
	)
{
	hash_table_t table;
	hash_table_init(&table, 1);
	hash_table_get(&table, NULL);
}


void assert_used
test_should_fail__hash_table_get_null(
	void
	)
{
	hash_table_get(NULL, NULL);
}


void assert_used
test_should_fail__hash_table_del_null_table(
	void
	)
{
	hash_table_del(NULL, "foo");
}


void assert_used
test_should_fail__hash_table_del_null_key(
	void
	)
{
	hash_table_t table;
	hash_table_init(&table, 1);
	hash_table_del(&table, NULL);
}


void assert_used
test_should_fail__hash_table_del_null(
	void
	)
{
	hash_table_del(NULL, NULL);
}


void assert_used
test_should_fail__hash_table_for_each_null_table(
	void
	)
{
	hash_table_for_each(NULL, hash_table_for_each_fn, NULL);
}


void assert_used
test_should_fail__hash_table_for_each_null_fn(
	void
	)
{
	hash_table_t table;
	hash_table_init(&table, 1);
	hash_table_for_each(&table, NULL, NULL);
}


void assert_used
test_should_fail__hash_table_for_each_null(
	void
	)
{
	hash_table_for_each(NULL, NULL, NULL);
}


void assert_used
test_should_fail__hash_table_clear_null_table(
	void
	)
{
	hash_table_clear(NULL);
}


void assert_used
test_should_fail__hash_table_set_null_table(
	void
	)
{
	hash_table_set(NULL, "foo", (void*) 1);
}


void assert_used
test_should_fail__hash_table_set_null_key(
	void
	)
{
	hash_table_t table;
	hash_table_init(&table, 1);
	hash_table_set(&table, NULL, (void*) 1);
}


void assert_used
test_should_fail__hash_table_set_null(
	void
	)
{
	hash_table_set(NULL, NULL, (void*) 1);
}


void assert_used
test_should_fail__hash_table_modify_null_table(
	void
	)
{
	hash_table_modify(NULL, "foo", (void*) 1);
}


void assert_used
test_should_fail__hash_table_modify_null_key(
	void
	)
{
	hash_table_t table;
	hash_table_init(&table, 1);
	hash_table_modify(&table, NULL, (void*) 1);
}


void assert_used
test_should_fail__hash_table_modify_null(
	void
	)
{
	hash_table_modify(NULL, NULL, (void*) 1);
}
