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

#pragma once

#include <DiepDesktop/shared/extent.h>

/* Above or equal, if can, 1 node splits into 4 */
#define QUADTREE_SPLIT_THRESHOLD 8

/* Below or equal, if can, 4 nodes merge into 1 */
#define QUADTREE_MERGE_THRESHOLD 7

/* You might want to increase this if you get a lot of collisions per tick */
#define QUADTREE_HASH_TABLE_FACTOR 1

#define QUADTREE_DEDUPE_COLLISIONS 1

/* Do not modify unless you know what you are doing. Use Quadtree.MinSize. */
#define QUADTREE_MAX_DEPTH 20

/* Do not modify */
#define QUADTREE_DFS_LENGTH (QUADTREE_MAX_DEPTH * 3 + 1)


typedef struct quadtree_node
{
	int32_t count;

	union
	{
		uint32_t next;

		struct
		{
			uint32_t head;
			uint32_t position_flags;
		};

		uint32_t heads[4];
	};
}
quadtree_node_t;


typedef struct quadtree_node_entity
{
	uint32_t next;
	uint32_t entity;
}
quadtree_node_entity_t;
