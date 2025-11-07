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

#include <tests/base.h>
#include <shared/debug.h>
#include <shared/extent.h>
#include <shared/alloc_ext.h>


typedef struct test_entity_data
{
	rect_extent_t rect_extent;
}
test_entity_data_t;


#define quadtree_entity_data test_entity_data_t
#include <shared/quadtree.h>


void assert_used
test_normal_pass__quadtree_init_free(
	void
	)
{
	quadtree_t qt;
	quadtree_init(&qt);
	quadtree_free(&qt);
}


void assert_used
test_normal_fail__quadtree_init_null(
	void
	)
{
	quadtree_init(NULL);
}


void assert_used
test_normal_fail__quadtree_free_null(
	void
	)
{
	quadtree_free(NULL);
}


void assert_used
test_normal_fail__quadtree_insert_null_qt(
	void
	)
{
	quadtree_insert(NULL, TEST_PTR);
}


void assert_used
test_normal_fail__quadtree_insert_null_data(
	void
	)
{
	quadtree_t qt;
	quadtree_init(&qt);
	quadtree_insert(&qt, NULL);
}


void assert_used
test_normal_fail__quadtree_remove_null(
	void
	)
{
	quadtree_remove(NULL, 1);
}


void assert_used
test_normal_fail__quadtree_remove_invalid_entity_idx_1(
	void
	)
{
	quadtree_t qt;
	quadtree_init(&qt);
	quadtree_remove(NULL, 0);
}


void assert_used
test_normal_fail__quadtree_remove_invalid_entity_idx_2(
	void
	)
{
	quadtree_t qt;
	quadtree_init(&qt);
	quadtree_remove(NULL, 1);
}


void assert_used
test_normal_fail__quadtree_normalize_null(
	void
	)
{
	quadtree_normalize(NULL);
}


void assert_used
test_normal_fail__quadtree_update_null_qt(
	void
	)
{
	quadtree_update(NULL, TEST_PTR);
}


void assert_used
test_normal_fail__quadtree_update_null_update_fn(
	void
	)
{
	quadtree_t qt;
	quadtree_init(&qt);
	quadtree_update(&qt, NULL);
}


void assert_used
test_normal_fail__quadtree_query_null_qt(
	void
	)
{
	quadtree_query(NULL, (rect_extent_t){0}, TEST_PTR);
}


void assert_used
test_normal_fail__quadtree_query_null_query_fn(
	void
	)
{
	quadtree_t qt;
	quadtree_init(&qt);
	quadtree_query(&qt, (rect_extent_t){0}, NULL);
}


void assert_used
test_normal_fail__quadtree_query_nodes_null_qt(
	void
	)
{
	quadtree_query_nodes(NULL, (rect_extent_t){0}, TEST_PTR);
}


void assert_used
test_normal_fail__quadtree_query_nodes_null_node_query_fn(
	void
	)
{
	quadtree_t qt;
	quadtree_init(&qt);
	quadtree_query_nodes(&qt, (rect_extent_t){0}, NULL);
}


void assert_used
test_normal_fail__quadtree_collide_null_qt(
	void
	)
{
	quadtree_collide(NULL, TEST_PTR);
}


void assert_used
test_normal_fail__quadtree_collide_null_collide_fn(
	void
	)
{
	quadtree_t qt;
	quadtree_init(&qt);
	quadtree_collide(&qt, NULL);
}
