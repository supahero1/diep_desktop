#pragma once

#include <stdint.h>

#define SHIFT(Bit) (1 << (Bit))

#define GET_BITS(Value) __builtin_choose_expr((Value) == 1, 0, 32 - __builtin_clz((Value) - 1))

#define BITS_COUNT(Name)	\
Name##__COUNT,				\
Name##__BITS = GET_BITS( Name##__COUNT )

#define BITS_COUNT_EXP(Name)	\
Name##__COUNT,					\
Name##__BITS = GET_BITS(SHIFT( Name##__COUNT ))

#define TO_BYTES(Bits) (((Bits) + 7) >> 3)

#define ARRAYLEN(A) (sizeof(A)/sizeof((A)[0]))

#define MIN(a, b)               \
({                              \
    __typeof__ (a) _a = (a);    \
    __typeof__ (b) _b = (b);    \
    _a > _b ? _b : _a;          \
})

#define MAX(a, b)               \
({                              \
    __typeof__ (a) _a = (a);    \
    __typeof__ (b) _b = (b);    \
    _a > _b ? _a : _b;          \
})

#define CLAMP(a, min, max) MIN(MAX((a), (min)), (max))
#define CLAMP_SYM(a, min_max) CLAMP((a), -(min_max), (min_max))

#define UINT_TO_FLOAT(a)	\
({							\
	union					\
	{						\
		float F32;			\
		uint32_t U32;		\
	}						\
	X =						\
	{						\
		.U32 = a			\
	};						\
							\
	X.F32;					\
})

#define FLOAT_TO_UINT(a)	\
({							\
	union					\
	{						\
		float F32;			\
		uint32_t U32;		\
	}						\
	X =						\
	{						\
		.F32 = a			\
	};						\
							\
	X.U32;					\
})

#define ROUNDF(Num)	\
({					\
	roundf(Num);	\
})


typedef union Pair
{
	struct
	{
		float X;
		float Y;
	};

	struct
	{
		float W;
		float H;
	};
}
Pair;


typedef union IPair
{
	struct
	{
		int X;
		int Y;
	};

	struct
	{
		int W;
		int H;
	};
}
IPair;


typedef union HalfExtent
{
	struct
	{
		union
		{
			Pair Pos;

			struct
			{
				float X;
				float Y;
			};
		};

		union
		{
			Pair Size;

			struct
			{
				float W;
				float H;
			};
		};
	};

	struct
	{
		float Top;
		float Left;
		float Right;
		float Bottom;
	};
}
HalfExtent;


typedef struct RectExtent
{
	union
	{
		Pair Min;

		struct
		{
			float MinX;
			float MinY;
		};
	};

	union
	{
		Pair Max;

		struct
		{
			float MaxX;
			float MaxY;
		};
	};
}
RectExtent;


typedef enum GameConst
{
	GAME_CONST_DEFAULT_WINDOW_WIDTH = 1280,
	GAME_CONST_DEFAULT_WINDOW_HEIGHT = 720,
	GAME_CONST_MAX_MOVEMENT_SPEED = 16,
	GAME_CONST_MAX_PLAYERS = 256,
	GAME_CONST_MAX_ENTITIES__BITS = GET_BITS(GAME_CONST_MAX_PLAYERS * 100),
	GAME_CONST_MAX_ENTITIES = 1 << GAME_CONST_MAX_ENTITIES__BITS,
	GAME_CONST_PORT = 2468,
	GAME_CONST_CLIENT_PACKET_SIZE = 256,
	GAME_CONST_SERVER_PACKET_SIZE = 65536,
	GAME_CONST_CLIENT_RECV_SIZE = GAME_CONST_SERVER_PACKET_SIZE << 3,
	GAME_CONST_SERVER_RECV_SIZE = GAME_CONST_CLIENT_PACKET_SIZE << 3,
	GAME_CONST_SERVER_PACKET_SIZE__BITS = GET_BITS(GAME_CONST_SERVER_PACKET_SIZE),
	GAME_CONST_TILE_SIZE = 128,
	GAME_CONST_HALF_ARENA_SIZE = GAME_CONST_TILE_SIZE * 32 + GAME_CONST_TILE_SIZE / 2,
	GAME_CONST_BORDER_PADDING = GAME_CONST_TILE_SIZE * 2,
	GAME_CONST_MIN_QUADTREE_NODE_SIZE = 4,
	GAME_CONST_QUADTREE_QUERY_PADDING = GAME_CONST_TILE_SIZE * 2,
	GAME_CONST_HALF_ARENA_CLEAR_ZONE = GAME_CONST_TILE_SIZE * 50,
	GAME_CONST_POSITION_INTEGER_BITS = GET_BITS(GAME_CONST_HALF_ARENA_SIZE + GAME_CONST_HALF_ARENA_CLEAR_ZONE),
	GAME_CONST_BUFFERED_STATES = 3,
	GAME_CONST_TICK_RATE_MS = 30,
	GAME_CONST_MAX_PLAYER_BARRELS = 8,
	GAME_CONST_MAX_PLAYER_BULLETS = 90,
	GAME_CONST_MAX_PLAYER_NAME_LENGTH = 16,
	GAME_CONST_MAX_PLAYER_SCORE_LENGTH = 6,
	GAME_CONST_MAX_SHAPES = GAME_CONST_MAX_PLAYERS << 4,
	GAME_CONST_MAX_UI_ELEMENTS =
		+ 1024 /* buffer for any containers and stuff */
		+ GAME_CONST_MAX_PLAYERS * 128
		+ GAME_CONST_MAX_SHAPES,
	GAME_CONST_MAX_TEXTURES =
		+ GAME_CONST_MAX_PLAYERS * (
			+ GAME_CONST_MAX_PLAYER_BARRELS
			+ GAME_CONST_MAX_PLAYER_BULLETS
			+ GAME_CONST_MAX_PLAYER_NAME_LENGTH * 2
			+ GAME_CONST_MAX_PLAYER_SCORE_LENGTH * 2
			+ 1 /* tank body */
			+ 6 /* hp bar */
			)
		+ GAME_CONST_MAX_SHAPES
		+ 10 /* minimap */
}
GameConst;


typedef enum KeyButton
{
	KEY_BUTTON_W,
	KEY_BUTTON_A,
	KEY_BUTTON_S,
	KEY_BUTTON_D,
	KEY_BUTTON_C,
	KEY_BUTTON_E,
	KEY_BUTTON_LMB,
	KEY_BUTTON_RMB,
	BITS_COUNT_EXP(KEY_BUTTON)
}
KeyButton;


typedef enum EntityFlags
{
	ENTITY_FLAG_X,
	ENTITY_FLAG_Y,
	ENTITY_FLAG_W,
	ENTITY_FLAG_R = ENTITY_FLAG_W,
	ENTITY_FLAG_H,
	ENTITY_FLAG_ANGLE,
	BITS_COUNT_EXP(ENTITY_FLAG)
}
EntityFlags;


typedef enum ClientOpCode
{
	CLIENT_OPCODE_INPUT,
	BITS_COUNT(CLIENT_OPCODE)
}
ClientOpCode;


typedef enum ServerOpCode
{
	SERVER_OPCODE_UPDATE,
	BITS_COUNT(SERVER_OPCODE)
}
ServerOpCode;


typedef enum EntityType
{
	ENTITY_TYPE_TANK,
	ENTITY_TYPE_SHAPE,
	BITS_COUNT(ENTITY_TYPE)
}
EntityType;


typedef enum Tank
{
	TANK_BASIC,
	BITS_COUNT(TANK)
}
Tank;


typedef enum Shape
{
	SHAPE_SQUARE,
	SHAPE_TRIANGLE,
	SHAPE_PENTAGON,
	BITS_COUNT(SHAPE)
}
Shape;

extern const float ShapeRadius[];
extern const float ShapeHitbox[];
extern const uint32_t ShapeMaxHP[];
extern const uint32_t ShapeHPBits[];

extern const uint32_t TypeToSubtypeBits[];

typedef enum FieldSize
{
	FIELD_SIZE_TICK_DURATION = 51,
	FIELD_SIZE_MOUSE_X = 11,
	FIELD_SIZE_MOUSE_Y = 11,
	FIELD_SIZE_FOV = 12
}
FieldSize;


#define FIXED_POINT(Name) FIXED_POINT_INTEGER_##Name, FIXED_POINT_FRACTION_##Name
#define DEF_FIXED_POINT(Name, Integer, Fraction)	\
FIXED_POINT_INTEGER_##Name = Integer,				\
FIXED_POINT_FRACTION_##Name = Fraction

typedef enum FixedPoint
{
	DEF_FIXED_POINT(FOV, 0, 10),
	DEF_FIXED_POINT(POS, GAME_CONST_POSITION_INTEGER_BITS, 4),
	DEF_FIXED_POINT(SCREEN_POS, 12, 4),
	DEF_FIXED_POINT(RADIUS, GET_BITS(512), 10)
}
FixedPoint;

#undef DEF_FIXED_POINT
