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

#include <shared/hash.h>
#include <shared/debug.h>
#include <shared/options.h>
#include <shared/alloc_ext.h>

#include <string.h>
#include <stdlib.h>


options_t global_options = NULL;


struct options
{
	hash_table_t table;
};


private void
options_key_free_fn(
	str_t key
	)
{
	str_clear(key);
}


private void
options_value_free_fn(
	str_t value
	)
{
	str_free(value);
}


options_t
options_init(
	int argc,
	const char* const* argv
	)
{
	options_t options = alloc_malloc(options, 1);
	assert_not_null(options);

	options->table = hash_table_init(16,
		(void*) options_key_free_fn, (void*) options_value_free_fn);

	const char* const* arg = argv + 1;
	const char* const* arg_end = argv + argc;

	while(arg < arg_end)
	{
		const char* key = *(arg++);
		const char* value = strchrnul(key, '=');

		if(strncmp(key, "--", 2))
		{
			continue;
		}

		key += 2;
		uint32_t len = value - key;
		key = cstr_init_len(key, len);

		str_t value_str = NULL;
		if(*value == '=')
		{
			value_str = str_init_copy_cstr(++value);
		}

		hash_table_set(options->table, key, value_str);
	}

	return options;
}


void
options_free(
	options_t options
	)
{
	assert_not_null(options);

	hash_table_free(options->table);

	alloc_free(options, 1);
}


void
options_set(
	options_t options,
	const char* key,
	str_t value
	)
{
	assert_not_null(options);
	assert_not_null(key);

	key = cstr_init(key);
	hash_table_set(options->table, key, value);
}


void
options_set_default(
	options_t options,
	const char* key,
	str_t value
	)
{
	assert_not_null(options);
	assert_not_null(key);

	key = cstr_init(key);
	hash_table_add(options->table, key, value);
}


const str_t
options_get(
	options_t options,
	const char* key
	)
{
	assert_not_null(options);
	assert_not_null(key);

	return hash_table_get(options->table, key);
}


int64_t
options_get_i64(
	options_t options,
	const char* key,
	int64_t min_value,
	int64_t max_value,
	int64_t default_value
	)
{
	assert_not_null(options);
	assert_not_null(key);

	const str_t value = options_get(options, key);
	if(value == NULL || str_is_empty(value))
	{
		return default_value;
	}

	int64_t result = strtoll(value->str, NULL, 10);
	if(result < min_value || result > max_value)
	{
		result = default_value;
	}

	return result;
}


float
options_get_f32(
	options_t options,
	const char* key,
	float min_value,
	float max_value,
	float default_value
	)
{
	assert_not_null(options);
	assert_not_null(key);

	const str_t value = options_get(options, key);
	if(value == NULL || str_is_empty(value))
	{
		return default_value;
	}

	float result = strtof(value->str, NULL);
	if(result < min_value || result > max_value)
	{
		result = default_value;
	}

	return result;
}


bool
options_get_boolean(
	options_t options,
	const char* key,
	bool default_value
	)
{
	assert_not_null(options);
	assert_not_null(key);

	const str_t value = options_get(options, key);
	if(value == NULL || str_is_empty(value))
	{
		return default_value;
	}

	if(
		str_case_cmp_len(value, "true", 4) ||
		str_case_cmp_len(value, "yes", 3) ||
		str_case_cmp_len(value, "1", 1)
		)
	{
		return true;
	}

	return false;
}


const str_t
options_get_str(
	options_t options,
	const char* key,
	const char* default_value
	)
{
	assert_not_null(options);
	assert_not_null(key);
	assert_not_null(default_value);

	const str_t value = options_get(options, key);
	if(value == NULL || str_is_empty(value))
	{
		return str_init_copy_cstr(default_value);
	}

	return value;
}


bool
options_exists(
	options_t options,
	const char* key
	)
{
	assert_not_null(options);
	assert_not_null(key);

	return hash_table_has(options->table, key);
}
