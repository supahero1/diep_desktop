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

#include <DiepDesktop/shared/sync.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/event.h>
#include <DiepDesktop/shared/alloc_ext.h>


void
event_target_init(
	event_target_t* target
	)
{
	assert_not_null(target);

	target->head = NULL;
}


void
event_target_free(
	event_target_t* target
	)
{
	assert_not_null(target);

	if(target->head)
	{
		assert_unreachable();
	}
}


typedef struct event_once_data
{
	event_target_t* target;
	event_listener_data_t data;
}
event_once_data_t;


private void
event_once_fn(
	event_listener_t* listener,
	void* event_data
	)
{
	event_once_data_t once = *(event_once_data_t*)(listener + 1);
	event_target_del_once(once.target, listener);
	once.data.fn(once.data.data, event_data);
}


private event_listener_t*
event_target_add_common(
	event_target_t* target,
	event_listener_data_t data,
	bool once
	)
{
	assert_not_null(target);
	assert_not_null(data.fn);

	event_listener_t* listener;

	if(once)
	{
		event_once_data_t* once_data;

		listener = alloc_malloc(sizeof(*listener) + sizeof(*once_data));
		assert_not_null(listener);

		once_data = (void*)(listener + 1);

		*once_data =
		(event_once_data_t)
		{
			.target = target,
			.data = data
		};

		data =
		(event_listener_data_t)
		{
			.fn = (void*) event_once_fn,
			.data = listener
		};
	}
	else
	{
		listener = alloc_malloc(sizeof(*listener));
		assert_not_null(listener);
	}

	listener->prev = NULL;
	listener->next = target->head;
	listener->data = data;

	if(target->head)
	{
		target->head->prev = listener;
	}

	target->head = listener;

	return listener;
}


event_listener_t*
event_target_add(
	event_target_t* target,
	event_listener_data_t data
	)
{
	return event_target_add_common(target, data, false);
}


event_listener_t*
event_target_once(
	event_target_t* target,
	event_listener_data_t data
	)
{
	return event_target_add_common(target, data, true);
}


private void
event_target_del_common(
	event_target_t* target,
	event_listener_t* listener,
	alloc_t size
	)
{
	assert_not_null(target);
	assert_not_null(listener);

#ifndef NDEBUG
	event_listener_t* current = target->head;
	bool found = false;

	while(current)
	{
		if(current == listener)
		{
			found = true;
			break;
		}

		current = current->next;
	}

	assert_true(found);
#endif

	if(listener->prev)
	{
		listener->prev->next = listener->next;
	}
	else
	{
		target->head = listener->next;
	}

	if(listener->next)
	{
		listener->next->prev = listener->prev;
	}

	alloc_free(listener, size);
}


void
event_target_del(
	event_target_t* target,
	event_listener_t* listener
	)
{
	event_target_del_common(target, listener, sizeof(*listener));
}


void
event_target_del_once(
	event_target_t* target,
	event_listener_t* listener
	)
{
	event_target_del_common(target, listener,
		sizeof(*listener) + sizeof(event_once_data_t));
}


void
event_target_fire(
	event_target_t* target,
	void* event_data
	)
{
	assert_not_null(target);

	event_listener_t* listener = target->head;
	while(listener)
	{
		event_listener_t* next = listener->next;
		listener->data.fn(listener->data.data, event_data);
		listener = next;
	}
}


typedef struct event_wait_data
{
	sync_mtx_t mtx;
	void* event_data;
}
event_wait_data_t;


private void
event_target_wait_fn(
	event_wait_data_t* data,
	void* event_data
	)
{
	data->event_data = event_data;
	sync_mtx_unlock(&data->mtx);
}


void*
event_target_wait(
	event_target_t* target
	)
{
	event_wait_data_t wait_data;
	sync_mtx_init(&wait_data.mtx);
	sync_mtx_lock(&wait_data.mtx);

	event_listener_data_t data =
	{
		.fn = (void*) event_target_wait_fn,
		.data = &wait_data
	};
	event_listener_t* listener = event_target_add(target, data);

	sync_mtx_lock(&wait_data.mtx);

	event_target_del(target, listener);

	sync_mtx_unlock(&wait_data.mtx);
	sync_mtx_free(&wait_data.mtx);

	return wait_data.event_data;
}
