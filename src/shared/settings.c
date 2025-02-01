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

#include <DiepDesktop/shared/file.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/shared/settings.h>
#include <DiepDesktop/shared/alloc_ext.h>
#include <DiepDesktop/shared/bit_buffer.h>

#include <zstd.h>


void
settings_init(
	settings_t* settings,
	const char* path,
	time_timers_t* timers
	)
{
	sync_mtx_init(&settings->mtx);
	hash_table_init(&settings->table, 256);
	settings->dirty = false;

	settings->path = path;

	settings->timers = timers;
	time_timer_init(&settings->save_timer);

	event_target_init(&settings->save_target);
	event_target_init(&settings->load_target);

	settings_load(settings);
}


void
settings_free(
	settings_t* settings
	)
{
	if(time_timers_cancel_timeout(settings->timers, &settings->save_timer))
	{
		settings_save(settings);
	}

	event_target_free(&settings->load_target);
	event_target_free(&settings->save_target);

	time_timer_free(&settings->save_timer);

	hash_table_free(&settings->table);
	sync_mtx_free(&settings->mtx);
}


private void
settings_for_each_sum_fn(
	const char* name,
	uint32_t len,
	setting_t* setting,
	uint64_t* sum
	)
{
	*sum += bit_buffer_len_str(len);
	*sum += bit_buffer_len_bits_var(setting->type, 3);


	switch(setting->type)
	{

	case SETTING_TYPE_I64:
	{
		*sum += bit_buffer_len_signed_bits_var(setting->value.i64.value, 7);
		break;
	}

	case SETTING_TYPE_F32:
	{
		*sum += bit_buffer_len_bytes(sizeof(float));
		break;
	}

	case SETTING_TYPE_BOOLEAN:
	{
		*sum += bit_buffer_len_bits(1);
		break;
	}

	case SETTING_TYPE_STR:
	{
		*sum += bit_buffer_len_str(setting->value.str.len);
		break;
	}

	case SETTING_TYPE_COLOR:
	{
		*sum += bit_buffer_len_bytes(sizeof(color_argb_t));
		break;
	}

	default: assert_unreachable();

	}
}


private void
settings_for_each_set_fn(
	const char* name,
	uint32_t len,
	setting_t* setting,
	bit_buffer_t* buffer
	)
{
	bit_buffer_set_str(buffer, (const uint8_t*) name, len);
	bit_buffer_set_bits_var(buffer, setting->type, 3);


	switch(setting->type)
	{

	case SETTING_TYPE_I64:
	{
		bit_buffer_set_signed_bits_var(buffer, setting->value.i64.value, 7);
		break;
	}

	case SETTING_TYPE_F32:
	{
		bit_buffer_set_bytes(buffer, &setting->value.f32.value, sizeof(float));
		break;
	}

	case SETTING_TYPE_BOOLEAN:
	{
		bit_buffer_set_bits(buffer, setting->value.boolean.value, 1);
		break;
	}

	case SETTING_TYPE_STR:
	{
		bit_buffer_set_str(buffer, setting->value.str.str, setting->value.str.len);
		break;
	}

	case SETTING_TYPE_COLOR:
	{
		bit_buffer_set_bytes(buffer, &setting->value.color.argb, sizeof(color_argb_t));
		break;
	}

	default: assert_unreachable();

	}
}


void
settings_save(
	settings_t* settings
	)
{
	sync_mtx_lock(&settings->mtx);

	if(!settings->dirty)
	{
		sync_mtx_unlock(&settings->mtx);
		return;
	}

	uint64_t sum = 0;
	hash_table_for_each(&settings->table, (hash_table_for_each_fn_t) settings_for_each_sum_fn, &sum);
	sum = MACRO_TO_BYTES(sum) + 60;

	bit_buffer_t buffer;
	bit_buffer_set(&buffer, alloc_malloc(sum), sum);
	assert_not_null(buffer.data);

	uint32_t magic = 0x015FF510;
	bit_buffer_set_bits(&buffer, magic, 60);

	hash_table_for_each(&settings->table, (hash_table_for_each_fn_t) settings_for_each_set_fn, &buffer);

	sync_mtx_unlock(&settings->mtx);

	uint64_t compressed_size = ZSTD_compressBound(buffer.len);
	uint8_t* compressed = alloc_malloc(compressed_size);
	assert_not_null(compressed);

	uint64_t actual_compressed_size = ZSTD_compress(
		compressed, compressed_size, buffer.data, buffer.len, 10);
	int error = ZSTD_isError(actual_compressed_size);
	assert_false(error);

	alloc_free(sum, buffer.data);

	file_t file;
	file.data = compressed;
	file.len = actual_compressed_size;

	bool status = file_write(settings->path, file);
	alloc_free(compressed_size, compressed);

	settings_save_event_data_t save_data =
	{
		.settings = settings,
		.success = status
	};
	event_target_fire(&settings->save_target, &save_data);
}


private void
settings_save_fn(
	settings_t* settings
	)
{
	thread_data_t data =
	{
		.fn = (thread_fn_t) settings_save,
		.data = settings
	};
	thread_init(NULL, data);
}


void
settings_load(
	settings_t* settings
	)
{
	file_t file;
	bool status = file_read_cap(settings->path, &file, /* 1MiB */ 0x100000);
	if(!status)
	{
		goto goto_failure;
	}

	uint64_t decompressed_size = ZSTD_getFrameContentSize(file.data, file.len);
	if(decompressed_size > /* 1MiB */ 0x100000)
	{
		goto goto_failure_file;
	}

	uint8_t* decompressed = alloc_malloc(decompressed_size);
	if(!decompressed)
	{
		goto goto_failure_file;
	}

	uint64_t actual_decompressed_size = ZSTD_decompress(
		decompressed, decompressed_size, file.data, file.len);
	int error = ZSTD_isError(actual_decompressed_size);
	if(error)
	{
		goto goto_failure_compressed;
	}

	file_free(file);
	file.len = 0;
	file.data = NULL;

	bit_buffer_t buffer;
	bit_buffer_set(&buffer, decompressed, actual_decompressed_size);

	uint32_t magic = bit_buffer_get_bits(&buffer, 60);
	if(magic != 0x015FF510)
	{
		goto goto_failure_compressed;
	}

	uint8_t name[127];

	settings_load_event_data_t load_data =
	{
		.settings = settings,
		.success = false
	};

	hash_table_t new_table;
	hash_table_init(&new_table, 256);

	while(bit_buffer_available_bits(&buffer) >= 8 + 8 + 3 + 1)
	{
		uint64_t name_len = 127;
		bit_buffer_get_str_safe(&buffer, name, &name_len, &status);
		if(!status || !name_len)
		{
			goto goto_failure_compressed;
		}

		setting_t setting;
		setting.type = bit_buffer_get_bits_var_safe(&buffer, 3, &status);
		if(!status)
		{
			goto goto_failure_compressed;
		}


		switch(setting.type)
		{

		case SETTING_TYPE_I64:
		{
			setting.value.i64.value = bit_buffer_get_signed_bits_var_safe(&buffer, 7, &status);
			if(!status)
			{
				goto goto_failure_compressed;
			}

			break;
		}

		case SETTING_TYPE_F32:
		{
			bit_buffer_get_bytes_safe(&buffer, &setting.value.f32.value, sizeof(float), &status);
			if(!status)
			{
				goto goto_failure_compressed;
			}

			break;
		}

		case SETTING_TYPE_BOOLEAN:
		{
			setting.value.boolean.value = bit_buffer_get_bits_safe(&buffer, 1, &status);
			if(!status)
			{
				goto goto_failure_compressed;
			}

			break;
		}

		case SETTING_TYPE_STR:
		{
			setting.value.str.len = 127;
			setting.value.str.str = alloc_malloc(setting.value.str.len);
			assert_not_null(setting.value.str.str);

			bit_buffer_get_str_safe(&buffer, setting.value.str.str, &setting.value.str.len, &status);
			if(!status)
			{
				alloc_free(setting.value.str.len, setting.value.str.str);
				goto goto_failure_compressed;
			}

			break;
		}

		case SETTING_TYPE_COLOR:
		{
			bit_buffer_get_bytes_safe(&buffer, &setting.value.color.argb, sizeof(color_argb_t), &status);
			if(!status)
			{
				goto goto_failure_compressed;
			}

			break;
		}

		default: assert_unreachable();

		}


		hash_table_modify(&new_table, (const char*) name, &setting);
	}

	sync_mtx_lock(&settings->mtx);
		time_timers_cancel_timeout(settings->timers, &settings->save_timer);

		hash_table_free(&settings->table);
		settings->table = new_table;
		settings->dirty = false;
	sync_mtx_unlock(&settings->mtx);

	alloc_free(decompressed_size, decompressed);

	load_data.success = true;
	event_target_fire(&settings->load_target, &load_data);

	return;


	goto_failure_compressed:
	alloc_free(decompressed_size, decompressed);

	goto_failure_file:
	file_free(file);

	goto_failure:
	event_target_fire(&settings->load_target, &load_data);
}


setting_t*
settings_get(
	settings_t* settings,
	const char* name
	)
{
	sync_mtx_lock(&settings->mtx);
		setting_t* setting = hash_table_get(&settings->table, name);
	sync_mtx_unlock(&settings->mtx);

	return setting;
}


void
settings_add(
	settings_t* settings,
	const char* name,
	setting_t* setting
	)
{
	sync_mtx_lock(&settings->mtx);
		bool status = hash_table_add(&settings->table, name, setting);
		assert_true(status);
	sync_mtx_unlock(&settings->mtx);
}


void
settings_set(
	settings_t* settings,
	const char* name,
	setting_value_t value
	)
{
	sync_mtx_lock(&settings->mtx);
		setting_t* setting = hash_table_get(&settings->table, name);
		assert_not_null(setting);

		setting->value = value;

		time_timers_lock(settings->timers);
			uint64_t time = time_get_with_sec(5);
			if(!time_timers_set_timeout_u(settings->timers, &settings->save_timer, time))
			{
				time_timeout_t timeout =
				{
					.timer = &settings->save_timer,
					.data =
					{
						.fn = (time_fn_t) settings_save_fn,
						.data = settings
					},
					.time = time
				};
				time_timers_add_timeout_u(settings->timers, timeout);
			}
		time_timers_unlock(settings->timers);
	sync_mtx_unlock(&settings->mtx);
}


void
settings_del(
	settings_t* settings,
	const char* name
	)
{
	sync_mtx_lock(&settings->mtx);
		bool status = hash_table_del(&settings->table, name);
		assert_true(status);
	sync_mtx_unlock(&settings->mtx);
}
