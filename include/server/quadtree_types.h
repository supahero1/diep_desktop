#pragma once

#include <stdint.h>

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


typedef float QuadtreePosition;


typedef struct QuadtreeRectExtent
{
	QuadtreePosition min_x, max_x, min_y, max_y;
}
QuadtreeRectExtent;


typedef struct QuadtreeHalfExtent
{
	QuadtreePosition x, y, w, h;
}
QuadtreeHalfExtent;


static inline bool
QuadtreeIntersects(
	QuadtreeRectExtent a,
	QuadtreeRectExtent b
	)
{
	return
		a.max_x >= b.min_x &&
		a.max_y >= b.min_y &&
		b.max_x >= a.min_x &&
		b.max_y >= a.min_y;
}


static inline bool
QuadtreeIsInside(
	QuadtreeRectExtent a,
	QuadtreeRectExtent b
	)
{
	return
		a.min_x > b.min_x &&
		a.min_y > b.min_y &&
		b.max_x > a.max_x &&
		b.max_y > a.max_y;
}


static inline QuadtreeRectExtent
QuadtreeHalfToRectExtent(
	QuadtreeHalfExtent half_extent_t
	)
{
	return
	(QuadtreeRectExtent)
	{
		.min_x = half_extent_t.x - half_extent_t.w,
		.max_x = half_extent_t.x + half_extent_t.w,
		.min_y = half_extent_t.y - half_extent_t.h,
		.max_y = half_extent_t.y + half_extent_t.h
	};
}


static inline QuadtreeHalfExtent
QuadtreeRectToHalfExtent(
	QuadtreeRectExtent rect_extent_t
	)
{
	return
	(QuadtreeHalfExtent)
	{
		.x = (rect_extent_t.max_x + rect_extent_t.min_x) * 0.5f,
		.y = (rect_extent_t.max_y + rect_extent_t.min_y) * 0.5f,
		.w = (rect_extent_t.max_x - rect_extent_t.min_x) * 0.5f,
		.h = (rect_extent_t.max_y - rect_extent_t.min_y) * 0.5f
	};
}
