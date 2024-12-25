#include <DiepDesktop/shared/base.h>


const float ShapeRadius[] =
{
	[SHAPE_SQUARE] = 50.0,
	[SHAPE_TRIANGLE] = 55.0,
	[SHAPE_PENTAGON] = 70.0
};


const float ShapeHitbox[] =
{
	[SHAPE_SQUARE] = 35.0,
	[SHAPE_TRIANGLE] = 25.0,
	[SHAPE_PENTAGON] = 55.0
};


typedef enum ShapeMaxHPValues
{
	SQUARE_MAX_HP = 10,
	TRIANGLE_MAX_HP = 20,
	PENTAGON_MAX_HP = 100
}
ShapeMaxHPValues;


const uint32_t ShapeMaxHP[] =
{
	[SHAPE_SQUARE] = SQUARE_MAX_HP,
	[SHAPE_TRIANGLE] = TRIANGLE_MAX_HP,
	[SHAPE_PENTAGON] = PENTAGON_MAX_HP
};


#define SHAPE_HP_BITS(ShapeName) [ SHAPE_##ShapeName ] = GET_BITS( ShapeName##_MAX_HP )

const uint32_t ShapeHPBits[] =
{
	SHAPE_HP_BITS(SQUARE),
	SHAPE_HP_BITS(TRIANGLE),
	SHAPE_HP_BITS(PENTAGON)
};

#undef SHAPE_HP_BITS


const uint32_t TypeToSubtypeBits[] =
{
	[ENTITY_TYPE_TANK] = TANK__BITS,
	[ENTITY_TYPE_SHAPE] = SHAPE__BITS
};
