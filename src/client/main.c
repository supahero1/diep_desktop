// #include <DiepDesktop/socket.h>

#include <DiepDesktop/shared/base.h>
#include <DiepDesktop/shared/rand.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/client/window.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/shared/bit_buffer.h>

#include <DiepDesktop/client/ui/base.h>
#include <DiepDesktop/client/tex/base.h>
#include <DiepDesktop/client/ui/checkbox.h>
#include <DiepDesktop/client/ui/container.h>

#include <DiepDesktop/client/base64.h>



#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


#define INTERPOLATED(Value) Value[2]

typedef struct GameEntity
{
	EntityType Type;
	uint32_t Subtype;

	INTERPOLATED(float X);
	INTERPOLATED(float Y);
	union
	{
		INTERPOLATED(float W);
		INTERPOLATED(float R);
	};
	INTERPOLATED(float H);

	INTERPOLATED(float Rotation);

	uint32_t MaxHP;
	uint32_t HPBits;
	INTERPOLATED(uint32_t HP);
	INTERPOLATED(float OpacityHP);

	float RotationDir;

	INTERPOLATED(float Damageness);
}
GameEntity;

typedef struct GameState
{
	uint64_t Duration;

	GameEntity* Entities;
	uint16_t EntityCount;

	float INTERPOLATED(FoV);
	float INTERPOLATED(CameraX);
	float INTERPOLATED(CameraY);
}
GameState;

#undef INTERPOLATED


Static GameState States[GAME_CONST_BUFFERED_STATES];
Static Mutex StateMutex;
Static uintptr_t CurrentStateIdx;
Static uint64_t CompletedStates = -1;
Static uint64_t CompletedTime;


Static float
LerpF(
	float Old,
	float New,
	float By
	)
{
	return Old + (New - Old) * By;
}


Static float
ShortestAngleDifference(
	float A,
	float B
	)
{
	float Diff = B - A;

	if(fabsf(Diff) > M_PI)
	{
		if(Diff > 0)
		{
			Diff -= M_PI * 2.0;
		}
		else
		{
			Diff += M_PI * 2.0;
		}
	}

	return Diff;
}


#define LERP(Value) LerpF(Value[0], Value[1], Scale)

#define OLD 0
#define NEW 1

#define NEW_VALUE(To, Value) NewState->To[NEW] = (Value)
#define NEW_ENTITY_VALUE(To, Value) NewEntity->To[NEW] = (Value)
#define COPY_OVER(To) NewState->To[OLD] = OldState->To[NEW]
#define SHIFT_OVER(To) NewEntity->To[OLD] = NewEntity->To[NEW]

Static void
OnPacket(
	BitBuffer* Buffer,
	ServerOpCode OpCode
	)
{
	switch(OpCode)
	{

	case SERVER_OPCODE_UPDATE:
	{
		MutexLock(&StateMutex);

		++CompletedStates;

		GameState* OldState = States + CurrentStateIdx;
		CurrentStateIdx = (CurrentStateIdx + 1) % ARRAYLEN(States);
		GameState* NewState = States + CurrentStateIdx;

		if(!CompletedStates)
		{
			CompletedTime = WindowGetTime();
		}
		else
		{
			CompletedTime += NewState->Duration;
		}

		OldState->Duration = BitBufferGetBits(Buffer, FIELD_SIZE_TICK_DURATION) * 10000;
		NewState->Duration = OldState->Duration;

		COPY_OVER(FoV);
		NEW_VALUE(FoV, BitBufferGetFixedPoint(Buffer, FIXED_POINT(FOV)));

		COPY_OVER(CameraX);
		NEW_VALUE(CameraX, BitBufferGetSignedFixedPoint(Buffer, FIXED_POINT(POS)));

		COPY_OVER(CameraY);
		NEW_VALUE(CameraY, BitBufferGetSignedFixedPoint(Buffer, FIXED_POINT(POS)));

		uint16_t Count = BitBufferGetBits(Buffer, GAME_CONST_MAX_ENTITIES__BITS);

		GameEntity* NewEntities = malloc(sizeof(*NewEntities) * Count);
		AssertNotNull(NewEntities);

		GameEntity* OldEntity = OldState->Entities;
		GameEntity* NewEntity = NewEntities;

		for(uint16_t i = 0; i < Count; ++i)
		{
			uint8_t OldSet = BitBufferGetBits(Buffer, 1);
			uint8_t NewSet = BitBufferGetBits(Buffer, 1);

			if(!OldSet)
			{
				/* Creation */

				NewEntity->Type = BitBufferGetBits(Buffer, ENTITY_TYPE__BITS);
				NewEntity->Subtype = BitBufferGetBits(Buffer, TypeToSubtypeBits[NewEntity->Type]);

				NEW_ENTITY_VALUE(X, BitBufferGetSignedFixedPoint(Buffer, FIXED_POINT(SCREEN_POS)));
				SHIFT_OVER(X);

				NEW_ENTITY_VALUE(Y, BitBufferGetSignedFixedPoint(Buffer, FIXED_POINT(SCREEN_POS)));
				SHIFT_OVER(Y);


				switch(NewEntity->Type)
				{

				case ENTITY_TYPE_TANK:
				{
					NEW_ENTITY_VALUE(R, BitBufferGetFixedPoint(Buffer, FIXED_POINT(RADIUS)));
					SHIFT_OVER(R);

					NewEntity->MaxHP = 1000;
					NewEntity->HPBits = 10;

					break;
				}

				case ENTITY_TYPE_SHAPE:
				{
					NEW_ENTITY_VALUE(R, ShapeRadius[NewEntity->Subtype]);
					SHIFT_OVER(R);

					NEW_ENTITY_VALUE(Rotation, RandAngle());
					SHIFT_OVER(Rotation);

					NewEntity->RotationDir = RandBit() ? 1 : -1;

					NewEntity->MaxHP = ShapeMaxHP[NewEntity->Subtype];
					NewEntity->HPBits = ShapeHPBits[NewEntity->Subtype];

					break;
				}

				default: __builtin_unreachable();

				}


				switch(NewEntity->Type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					uintptr_t WroteHP = BitBufferGetBits(Buffer, 1);

					if(BitBufferGetBits(Buffer, 1))
					{
						NEW_ENTITY_VALUE(Damageness, 0.1666f);
					}
					else
					{
						NEW_ENTITY_VALUE(Damageness, 0.0f);
					}

					SHIFT_OVER(Damageness);

					if(WroteHP)
					{
						NEW_ENTITY_VALUE(HP, BitBufferGetBits(Buffer, NewEntity->HPBits));
						NEW_ENTITY_VALUE(OpacityHP, 1.0f);
					}
					else
					{
						NEW_ENTITY_VALUE(HP, NewEntity->MaxHP);
						NEW_ENTITY_VALUE(OpacityHP, 0.0f);
					}

					SHIFT_OVER(HP);
					SHIFT_OVER(OpacityHP);

					break;
				}

				default: __builtin_unreachable();

				}


				++NewEntity;
			}
			else if(NewSet)
			{
				/* Update */

				*NewEntity = *OldEntity;

				SHIFT_OVER(X);
				SHIFT_OVER(Y);
				SHIFT_OVER(W);
				SHIFT_OVER(H);
				SHIFT_OVER(Rotation);
				SHIFT_OVER(HP);
				SHIFT_OVER(OpacityHP);
				SHIFT_OVER(Damageness);


				switch(NewEntity->Type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					NEW_ENTITY_VALUE(X, BitBufferGetSignedFixedPoint(Buffer, FIXED_POINT(SCREEN_POS)));
					NEW_ENTITY_VALUE(Y, BitBufferGetSignedFixedPoint(Buffer, FIXED_POINT(SCREEN_POS)));

					break;
				}

				default: __builtin_unreachable();

				}


				switch(NewEntity->Type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					uintptr_t WroteHP = BitBufferGetBits(Buffer, 1);

					if(BitBufferGetBits(Buffer, 1))
					{
						NEW_ENTITY_VALUE(Damageness, MIN(0.5f, NewEntity->Damageness[OLD] + 0.1666f));
					}
					else
					{
						NEW_ENTITY_VALUE(Damageness, MAX(0.0f, NewEntity->Damageness[OLD] - 0.1666f));
					}

					if(WroteHP)
					{
						NEW_ENTITY_VALUE(HP, BitBufferGetBits(Buffer, NewEntity->HPBits));
					}

					break;
				}

				default: __builtin_unreachable();

				}


				switch(NewEntity->Type)
				{

				case ENTITY_TYPE_TANK:
				{
					break;
				}

				case ENTITY_TYPE_SHAPE:
				{
					float Rotation = 0.001f * NewEntity->RotationDir * (250.0f / NewEntity->R[NEW]);
					NEW_ENTITY_VALUE(Rotation, NewEntity->Rotation[OLD] + Rotation);

					break;
				}

				default: __builtin_unreachable();

				}


				if(NewEntity->HP[NEW] != NewEntity->MaxHP)
				{
					NEW_ENTITY_VALUE(OpacityHP, MIN(1.0f, NewEntity->OpacityHP[OLD] + 0.15f));
				}
				else
				{
					NEW_ENTITY_VALUE(OpacityHP, MAX(0.0f, NewEntity->OpacityHP[OLD] - 0.15f));
				}

				++OldEntity;
				++NewEntity;
			}
			else
			{
				/* Removal */

				++OldEntity;
			}
		}

		AssertEQ(BitBufferGetConsumed(Buffer), Buffer->Length);

		free(NewState->Entities);
		NewState->Entities = NewEntities;
		NewState->EntityCount = NewEntity - NewEntities;

		MutexUnlock(&StateMutex);

		break;
	}

	default: __builtin_unreachable();

	}
}

#undef SHIFT_OVER
#undef COPY_OVER
#undef NEW_ENTITY_VALUE
#undef NEW_VALUE


void
SocketOnOpen(
	void
	)
{
	puts("socket open");
}


uint32_t
SocketOnData(
	uint8_t* Data,
	uint32_t Length
	)
{
	BitBuffer Buffer = {0};
	Buffer.Buffer = Data;
	Buffer.At = Buffer.Buffer;
	Buffer.Length = Length;

	if(BitBufferGetAvailableBits(&Buffer) < SERVER_OPCODE__BITS)
	{
		return 0;
	}

	ServerOpCode OpCode = BitBufferGetBits(&Buffer, SERVER_OPCODE__BITS);

	switch(OpCode)
	{

	case SERVER_OPCODE_UPDATE:
	{
		if(BitBufferGetAvailableBits(&Buffer) < GAME_CONST_SERVER_PACKET_SIZE__BITS)
		{
			return 0;
		}

		uintptr_t Size = BitBufferGetBits(&Buffer, GAME_CONST_SERVER_PACKET_SIZE__BITS);

		if(Size <= Length)
		{
			Buffer.Length = Size;

			OnPacket(&Buffer, OpCode);

			return Size;
		}

		return 0;
	}

	default: __builtin_unreachable();

	}
}


void
SocketOnClose(
	void
	)
{
	puts("socket close");
}

/*
Static VkVertexInstanceInput* DrawInput;
Static uint32_t DrawCount;


Static void
DrawBar(
	float X,
	float Y,
	float W,
	float Scale,
	float Opacity,
	DrawDepth Depth
	)
{
	float BlackBarHalfHeight = W * 0.125f;
	float BlackBarHalfWidth = W - BlackBarHalfHeight;
	float BlackBarHeight = BlackBarHalfHeight * 2.0f;
	float BlackBarLeft = X - BlackBarHalfWidth;
	float BlackBarRight = X + BlackBarHalfWidth;
	float BlackBarMiddle = (BlackBarLeft + BlackBarRight) * 0.5f;

	float Y_Pos = Y + BlackBarHalfHeight;

	float HPBarScale = 0.75f;
	float HPBarPadding = BlackBarHalfHeight * HPBarScale;
	float HPBarHalfHeight = BlackBarHalfHeight - HPBarPadding;
	float HPBarHalfWidth = BlackBarHalfWidth - HPBarPadding * (1.0f - HPBarScale);
	float HPBarHeight = HPBarHalfHeight * 2.0f;
	float HPBarLeft = X - HPBarHalfWidth;
	float HPBarRight = HPBarLeft + HPBarHalfWidth * 2.0f * Scale;
	float HPBarMiddle = (HPBarLeft + HPBarRight) * 0.5f;

	float R = MIN(1.000f, (1.000f - Scale) * 2.0f);
	float G = MIN(1.000f, Scale * 2.0f);
	float B = 0.000f;


	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { HPBarMiddle, Y_Pos, DRAW_DEPTH(Depth + 4) },
		.Color = { R, G, B, Opacity },
		.Dimensions = { HPBarRight - HPBarLeft, HPBarHeight },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { HPBarLeft, Y_Pos, DRAW_DEPTH(Depth + 3) },
		.Color = { R, G, B, Opacity },
		.Dimensions = { HPBarHeight, HPBarHeight },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { HPBarRight, Y_Pos, DRAW_DEPTH(Depth + 3) },
		.Color = { R, G, B, Opacity },
		.Dimensions = { HPBarHeight, HPBarHeight },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};


	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { BlackBarMiddle, Y_Pos, DRAW_DEPTH(Depth + 2) },
		.Color = { 0.000f, 0.000f, 0.000f, Opacity },
		.Dimensions = { BlackBarRight - BlackBarLeft, BlackBarHeight },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { BlackBarLeft, Y_Pos, DRAW_DEPTH(Depth + 1) },
		.Color = { 0.000f, 0.000f, 0.000f, Opacity },
		.Dimensions = { BlackBarHeight, BlackBarHeight },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { BlackBarRight, Y_Pos, DRAW_DEPTH(Depth + 1) },
		.Color = { 0.000f, 0.000f, 0.000f, Opacity },
		.Dimensions = { BlackBarHeight, BlackBarHeight },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};
}


Static float
TextWidth(
	const char* Text,
	uint32_t FontSize
	)
{
	const char* Char = Text;
	uint32_t Width = 0;

	while(*Char)
	{
		Width += FontData[*(Char++) - ' '].Stride;
	}

	float Scale = (float) FontSize / FONT_SIZE;
	return Width * Scale / 64.0f;
}


typedef enum TextAlign
{
	TEXT_ALIGN_LEFT,
	TEXT_ALIGN_CENTER,
	TEXT_ALIGN_RIGHT
}
TextAlign;

typedef enum TextBaseline
{
	TEXT_BASELINE_BOTTOM,
	TEXT_BASELINE_MIDDLE,
	TEXT_BASELINE_TOP
}
TextBaseline;

Static void
DrawAText(
	const char* Text,
	float X,
	float Y,
	float Opacity,
	uint32_t FontSize,
	TextAlign Align,
	TextBaseline Baseline,
	DrawDepth Depth
	)
{
	X -= TextWidth(Text, FontSize) * Align * 0.5f;
	Y += FontSize * Baseline * 0.3333f;

	float Scale = (float) FontSize / FONT_SIZE;


	const char* Char = Text;
	float CurrentDepth = DRAW_DEPTH(Depth - 1);
	float CurrentX = X;

	while(*Char)
	{
		const FontDrawData* Data = FontData + (*(Char++) - ' ');

		float Top = Data->Top  * Scale;
		float Left = Data->Left * Scale;
		float Stride = Data->Stride * Scale / 64.0f;
		float Size = Data->Size * Scale;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.Position = { CurrentX + Size / 2 + Left, Y + Size / 2 - Top, CurrentDepth },
			.Color = { 1.000f, 1.000f, 1.000f, Opacity },
			.Dimensions = { Size, Size },
			.Rotation = 0.0f,
			.TexScale = { 1.0f, 1.0f },
			.TexOffset = { 0.5f, 0.5f },
			.TexRes = Data->BgTexRes,
			.TexIndex = Data->BgTexIndex
		};

		CurrentX += Stride;
	}


	Char = Text;
	CurrentDepth = DRAW_DEPTH(Depth);
	CurrentX = X;

	while(*Char)
	{
		const FontDrawData* Data = FontData + (*(Char++) - ' ');

		float Top = Data->Top  * Scale;
		float Left = Data->Left * Scale;
		float Stride = Data->Stride * Scale / 64.0f;
		float Size = Data->Size * Scale;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.Position = { CurrentX + Size / 2 + Left, Y + Size / 2 - Top, CurrentDepth },
			.Color = { 1.000f, 1.000f, 1.000f, Opacity },
			.Dimensions = { Size, Size },
			.Rotation = 0.0f,
			.TexScale = { 1.0f, 1.0f },
			.TexOffset = { 0.5f, 0.5f },
			.TexRes = Data->TexRes,
			.TexIndex = Data->TexIndex
		};

		CurrentX += Stride;
	}
}


DrawData
VulkanGetDrawData___(
	void
	)
{
	DrawCount = 0;

	if(CompletedStates == -1 || CompletedStates < ARRAYLEN(States))
	{
		return
		(DrawData)
		{
			.Input = DrawInput,
			.Count = DrawCount
		};
	}

	MutexLock(&StateMutex);

	uintptr_t OldestStateIdx = (CurrentStateIdx + 1) % ARRAYLEN(States);
	GameState* OldestState = States + OldestStateIdx;

	uint64_t TotalTime = 0;

	GameState* State = States;
	GameState* StateEnd = State + ARRAYLEN(States);

	do
	{
		TotalTime += State->Duration;
	}
	while(++State != StateEnd);

	static uint64_t DrawAt = 0;
	static uint64_t LastDrawAt = 0;

	uint64_t Now = GetTime();

	if(!LastDrawAt)
	{
		LastDrawAt = Now;
	}

	DrawAt += Now - LastDrawAt;
	LastDrawAt = Now;

	DrawAt = MAX(MIN(DrawAt, CompletedTime + TotalTime), CompletedTime);
	uint64_t StepAt = DrawAt - CompletedTime;

	uintptr_t StateIdx = OldestStateIdx;
	State = OldestState;

	while(StepAt > State->Duration)
	{
		StepAt -= State->Duration;
		StateIdx = (StateIdx + 1) % ARRAYLEN(States);
		State = States + StateIdx;
	}

	float Scale = (float)((double) StepAt / (double) State->Duration);

	float FoV = LERP(State->FoV);
	float CameraX = LERP(State->CameraX);
	float CameraY = LERP(State->CameraY);

	float TileSize = GAME_CONST_TILE_SIZE;
	float ScaledTileSize = TileSize * FoV;
	float XMod = fmodf(CameraX, TileSize) / TileSize;
	float YMod = fmodf(CameraY, TileSize) / TileSize;

	float ArenaMinX = MAX(-960.0f, (-GAME_CONST_HALF_ARENA_SIZE - CameraX) * FoV);
	float ArenaMinY = MAX(-540.0f, (-GAME_CONST_HALF_ARENA_SIZE - CameraY) * FoV);
	float ArenaMaxX = MIN(960.0f, (GAME_CONST_HALF_ARENA_SIZE - CameraX) * FoV);
	float ArenaMaxY = MIN(540.0f, (GAME_CONST_HALF_ARENA_SIZE - CameraY) * FoV);

	if(ArenaMinX != ArenaMaxX && ArenaMinY != ArenaMaxY)
	{
		float ArenaX = (ArenaMinX + ArenaMaxX) * 0.5f;
		float ArenaY = (ArenaMinY + ArenaMaxY) * 0.5f;
		float ArenaWidth = ArenaMaxX - ArenaMinX;
		float ArenaHeight = ArenaMaxY - ArenaMinY;
		float ScaledXMod = XMod + fmodf(ArenaX, ScaledTileSize) / ScaledTileSize;
		float ScaledYMod = YMod + fmodf(ArenaY, ScaledTileSize) / ScaledTileSize;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.Position = { ArenaX, ArenaY, DRAW_DEPTH(DRAW_DEPTH_BACKGROUND) },
			.Color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.Dimensions = { ArenaWidth, ArenaHeight },
			.Rotation = 0,
			.TexScale = { ArenaWidth / ScaledTileSize, ArenaHeight / ScaledTileSize },
			.TexOffset = { ScaledXMod, ScaledYMod },
			.TexRes = TEXTURE_RESOLUTION_BG_LIGHT,
			.TexIndex = TEXTURE_BG_LIGHT
		};
	}

	float LeftBorderMinX = -960.0f;
	float LeftBorderMinY = -540.0f;
	float LeftBorderMaxX = ArenaMinX;
	float LeftBorderMaxY = 540.0f;

	if(LeftBorderMinX != LeftBorderMaxX && LeftBorderMinY != LeftBorderMaxY)
	{
		float BorderX = (LeftBorderMinX + LeftBorderMaxX) * 0.5f;
		float BorderY = (LeftBorderMinY + LeftBorderMaxY) * 0.5f;
		float BorderWidth = LeftBorderMaxX - LeftBorderMinX;
		float BorderHeight = LeftBorderMaxY - LeftBorderMinY;
		float ScaledXMod = XMod + fmodf(BorderX, ScaledTileSize) / ScaledTileSize;
		float ScaledYMod = YMod + fmodf(BorderY, ScaledTileSize) / ScaledTileSize;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.Position = { BorderX, BorderY, DRAW_DEPTH(DRAW_DEPTH_BACKGROUND) },
			.Color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.Dimensions = { BorderWidth, BorderHeight },
			.Rotation = 0,
			.TexScale = { BorderWidth / ScaledTileSize, BorderHeight / ScaledTileSize },
			.TexOffset = { ScaledXMod, ScaledYMod },
			.TexRes = TEXTURE_RESOLUTION_BG_DARK,
			.TexIndex = TEXTURE_BG_DARK
		};
	}

	float RightBorderMinX = ArenaMaxX;
	float RightBorderMinY = -540.0f;
	float RightBorderMaxX = 960.0f;
	float RightBorderMaxY = 540.0f;

	if(RightBorderMinX != RightBorderMaxX && RightBorderMinY != RightBorderMaxY)
	{
		float BorderX = (RightBorderMinX + RightBorderMaxX) * 0.5f;
		float BorderY = (RightBorderMinY + RightBorderMaxY) * 0.5f;
		float BorderWidth = RightBorderMaxX - RightBorderMinX;
		float BorderHeight = RightBorderMaxY - RightBorderMinY;
		float ScaledXMod = XMod + fmodf(BorderX, ScaledTileSize) / ScaledTileSize;
		float ScaledYMod = YMod + fmodf(BorderY, ScaledTileSize) / ScaledTileSize;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.Position = { BorderX, BorderY, DRAW_DEPTH(DRAW_DEPTH_BACKGROUND) },
			.Color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.Dimensions = { BorderWidth, BorderHeight },
			.Rotation = 0,
			.TexScale = { BorderWidth / ScaledTileSize, BorderHeight / ScaledTileSize },
			.TexOffset = { ScaledXMod, ScaledYMod },
			.TexRes = TEXTURE_RESOLUTION_BG_DARK,
			.TexIndex = TEXTURE_BG_DARK
		};
	}

	float BottomBorderMinX = -960.0f;
	float BottomBorderMinY = ArenaMaxY;
	float BottomBorderMaxX = 960.0f;
	float BottomBorderMaxY = 540.0f;

	if(BottomBorderMinX != BottomBorderMaxX && BottomBorderMinY != BottomBorderMaxY)
	{
		float BorderX = (BottomBorderMinX + BottomBorderMaxX) * 0.5f;
		float BorderY = (BottomBorderMinY + BottomBorderMaxY) * 0.5f;
		float BorderWidth = BottomBorderMaxX - BottomBorderMinX;
		float BorderHeight = BottomBorderMaxY - BottomBorderMinY;
		float ScaledXMod = XMod + fmodf(BorderX, ScaledTileSize) / ScaledTileSize;
		float ScaledYMod = YMod + fmodf(BorderY, ScaledTileSize) / ScaledTileSize;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.Position = { BorderX, BorderY, DRAW_DEPTH(DRAW_DEPTH_BACKGROUND) },
			.Color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.Dimensions = { BorderWidth, BorderHeight },
			.Rotation = 0,
			.TexScale = { BorderWidth / ScaledTileSize, BorderHeight / ScaledTileSize },
			.TexOffset = { ScaledXMod, ScaledYMod },
			.TexRes = TEXTURE_RESOLUTION_BG_DARK,
			.TexIndex = TEXTURE_BG_DARK
		};
	}

	float TopBorderMinX = -960.0f;
	float TopBorderMinY = -540.0f;
	float TopBorderMaxX = 960.0f;
	float TopBorderMaxY = ArenaMinY;

	if(TopBorderMinX != TopBorderMaxX && TopBorderMinY != TopBorderMaxY)
	{
		float BorderX = (TopBorderMinX + TopBorderMaxX) * 0.5f;
		float BorderY = (TopBorderMinY + TopBorderMaxY) * 0.5f;
		float BorderWidth = TopBorderMaxX - TopBorderMinX;
		float BorderHeight = TopBorderMaxY - TopBorderMinY;
		float ScaledXMod = XMod + fmodf(BorderX, ScaledTileSize) / ScaledTileSize;
		float ScaledYMod = YMod + fmodf(BorderY, ScaledTileSize) / ScaledTileSize;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.Position = { BorderX, BorderY, DRAW_DEPTH(DRAW_DEPTH_BACKGROUND) },
			.Color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.Dimensions = { BorderWidth, BorderHeight },
			.Rotation = 0,
			.TexScale = { BorderWidth / ScaledTileSize, BorderHeight / ScaledTileSize },
			.TexOffset = { ScaledXMod, ScaledYMod },
			.TexRes = TEXTURE_RESOLUTION_BG_DARK,
			.TexIndex = TEXTURE_BG_DARK
		};
	}

	GameEntity* Entity = State->Entities;
	GameEntity* EntityEnd = Entity + State->EntityCount;

	typedef struct EntityDrawData
	{
		float X;
		float Y;
		float W;
		float H;

		DrawDepth Depth;

		float Rotation;

		float HPScale;
		float OpacityHP;

		float Damageness;

		uint32_t TexRes;
		uint32_t TexIndex;

		float R;
		float G;
		float B;
		float A;
	}
	EntityDrawData;

	EntityDrawData EntityData[State->EntityCount];
	EntityDrawData* Data = EntityData;

	while(Entity != EntityEnd)
	{
		Data->X = LERP(Entity->X);
		Data->Y = LERP(Entity->Y);
		Data->W = LERP(Entity->W);

		switch(Entity->Type)
		{

		case ENTITY_TYPE_TANK:
		case ENTITY_TYPE_SHAPE:
		{
			Data->H = Data->W;

			break;
		}

		default: __builtin_unreachable();

		}


		Data->W *= FoV;
		Data->H *= FoV;

		Data->Rotation =
			Entity->Rotation[OLD] + ShortestAngleDifference(Entity->Rotation[OLD], Entity->Rotation[NEW]) * Scale;

		Data->HPScale = LERP(Entity->HP) / Entity->MaxHP;
		Data->OpacityHP = LERP(Entity->OpacityHP);

		Data->Damageness = LERP(Entity->Damageness);

		Data->A = 1.000f;


		switch(Entity->Type)
		{

		case ENTITY_TYPE_TANK:
		{
			Data->Depth = DRAW_DEPTH_TANK;

			Data->R = 0.000f;
			Data->G = 0.450f;
			Data->B = 0.750f;

			Data->TexRes = TEXTURE_RESOLUTION_TANK;
			Data->TexIndex = TEXTURE_TANK;

			DrawAText("23.5k",
				Data->X,
				Data->Y - Data->H - 4,
				1.0f,
				16,
				TEXT_ALIGN_CENTER,
				TEXT_BASELINE_BOTTOM,
				DRAW_DEPTH_TEST1);

			DrawAText("Shadam",
				Data->X,
				Data->Y - Data->H - 20,
				1.0f,
				32,
				TEXT_ALIGN_CENTER,
				TEXT_BASELINE_BOTTOM,
				DRAW_DEPTH_TEST1);

			break;
		}

		case ENTITY_TYPE_SHAPE:
		{
			Data->Depth = DRAW_DEPTH_SHAPE;

			switch(Entity->Subtype)
			{

			case SHAPE_SQUARE:
			{
				Data->TexRes = TEXTURE_RESOLUTION_SQUARE;
				Data->TexIndex = TEXTURE_SQUARE;

				Data->R = 1.000f;
				Data->G = 0.800f;
				Data->B = 0.200f;

				break;
			}

			case SHAPE_TRIANGLE:
			{
				Data->TexRes = TEXTURE_RESOLUTION_TRIANGLE;
				Data->TexIndex = TEXTURE_TRIANGLE;

				Data->R = 1.000f;
				Data->G = 0.250f;
				Data->B = 0.250f;

				break;
			}

			case SHAPE_PENTAGON:
			{
				Data->TexRes = TEXTURE_RESOLUTION_PENTAGON;
				Data->TexIndex = TEXTURE_PENTAGON;

				Data->R = 0.150f;
				Data->G = 0.200f;
				Data->B = 1.000f;

				break;
			}

			default: __builtin_unreachable();

			}

			break;
		}

		default: __builtin_unreachable();

		}


		Data->R = LerpF(Data->R, 1.000f, Data->Damageness);
		Data->B = LerpF(Data->B, 0.000f, Data->Damageness);
		Data->G = LerpF(Data->G, 0.000f, Data->Damageness);


		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.Position = { Data->X, Data->Y, DRAW_DEPTH(Data->Depth) },
			.Color = { Data->R, Data->G, Data->B, Data->A },
			.Dimensions = { Data->W * 2.0f, Data->H * 2.0f },
			.Rotation = Data->Rotation,
			.TexScale = { 1.0f, 1.0f },
			.TexOffset = { 0.5f, 0.5f },
			.TexRes = Data->TexRes,
			.TexIndex = Data->TexIndex
		};


		++Entity;
		++Data;
	}

	Data = EntityData;
	EntityDrawData* DataEnd = EntityData + ARRAYLEN(EntityData);

	while(Data != DataEnd)
	{
		if(Data->OpacityHP != 0.0f)
		{
			AssertEQ(Data->W, Data->H);
			DrawBar(Data->X, Data->Y + Data->H + 1.0f, Data->W, Data->HPScale, Data->OpacityHP, Data->Depth);
		}

		++Data;
	}


	float MinimapHalfSize = 100.0f;
	float MinimapSize = MinimapHalfSize * 2.0f;
	float MinimapPadding = 20.0f;
	float MinimapX = 960.0f - MinimapHalfSize - MinimapPadding;
	float MinimapY = 540.0f - MinimapHalfSize - MinimapPadding;
	float MinimapBorder = 6.0f;
	float MinimapColor = 0.200f;


	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { MinimapX, MinimapY, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_RECT) },
		.Color = { 1.000f, 1.000f, 1.000f, 0.1f },
		.Dimensions = { MinimapSize, MinimapSize },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};


	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { MinimapX - MinimapHalfSize, MinimapY, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_BORDER) },
		.Color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.Dimensions = { MinimapBorder, MinimapSize },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { MinimapX + MinimapHalfSize, MinimapY, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_BORDER) },
		.Color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.Dimensions = { MinimapBorder, MinimapSize },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { MinimapX, MinimapY - MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_BORDER) },
		.Color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.Dimensions = { MinimapSize, MinimapBorder },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { MinimapX, MinimapY + MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_BORDER) },
		.Color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.Dimensions = { MinimapSize, MinimapBorder },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};


	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { MinimapX - MinimapHalfSize, MinimapY - MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_CORNER) },
		.Color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.Dimensions = { MinimapBorder, MinimapBorder },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { MinimapX + MinimapHalfSize, MinimapY - MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_CORNER) },
		.Color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.Dimensions = { MinimapBorder, MinimapBorder },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { MinimapX - MinimapHalfSize, MinimapY + MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_CORNER) },
		.Color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.Dimensions = { MinimapBorder, MinimapBorder },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { MinimapX + MinimapHalfSize, MinimapY + MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_CORNER) },
		.Color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.Dimensions = { MinimapBorder, MinimapBorder },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};


	float MinimapCameraX = (CameraX / (GAME_CONST_HALF_ARENA_SIZE + (GAME_CONST_BORDER_PADDING >> 1))) * MinimapHalfSize;
	float MinimapCameraY = (CameraY / (GAME_CONST_HALF_ARENA_SIZE + (GAME_CONST_BORDER_PADDING >> 1))) * MinimapHalfSize;

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.Position = { MinimapX + MinimapCameraX, MinimapY + MinimapCameraY, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_INDICATOR) },
		.Color = { 0.100f, 0.100f, 0.100f, 1.0f },
		.Dimensions = { 6.0f, 6.0f },
		.Rotation = 0.0f,
		.TexScale = { 1.0f, 1.0f },
		.TexOffset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};


	MutexUnlock(&StateMutex);

	return
	(DrawData)
	{
		.Input = DrawInput,
		.Count = DrawCount
	};
}

#undef LERP
*/

/*
void
UIOnKeyDown(
	UIKey Key
	)
{
	(void) Key;
}


void
UIOnKeyUp(
	UIKey Key
	)
{
	(void) Key;
}


void
UIOnMouseDown(
	UIMouseDownEventData* Data
	)
{
	(void) Data;
}


void
UIOnMouseUp(
	UIMouseUpEventData* Data
	)
{
	(void) Data;
}


void
UIOnMouseMove(
	UIMouseMoveEventData* Data
	)
{
	//printf("game mousemove %f %f\n", X, Y);
}


void
UIOnMouseScroll(
	UIMouseScrollEventData* Data
	)
{
	printf("game mousescroll %f\n", Data->OffsetY);
}
*/


void
WindowOnResize(
	float Width,
	float Height
	)
{
	printf("window resize %f %f\n", Width, Height);
}


void
WindowOnFocus(
	void
	)
{
	puts("window focused");
}


void
WindowOnBlur(
	void
	)
{
	puts("window blur");
}


void
WindowOnKeyDown(
	int Key,
	int Mods,
	int Repeat
	)
{
	printf("window keydown %d %d %d\n", Key, Mods, Repeat);
}


void
WindowOnKeyUp(
	int Key,
	int Mods
	)
{
	printf("window keyup %d %d\n", Key, Mods);
}


void
WindowOnText(
	const char* Text
	)
{
	printf("window text %s\n", Text);
}


void
WindowOnMouseDown(
	int Button
	)
{
	printf("window mousedown %d\n", Button);
}


void
lmfao(
	void* null,
	WindowMouseDownData* Data
	)
{
	printf("window mousedown %d %.02f %.02f %hhu\n", Data->Button, Data->Position.X, Data->Position.Y, Data->Clicks);
}

void
lol(
	void* null,
	WindowMouseMoveData* Data
	)
{
	// printf("window mousemove old %.02f %.02f new %.02f %.02f\n", Data->OldPosition.X, Data->OldPosition.Y, Data->NewPosition.X, Data->NewPosition.Y);
}

void
hehe(
	void* null,
	WindowResizeData* Data
	)
{
	printf("window resize old %.02f %.02f new %.02f %.02f\n", Data->OldSize.W, Data->OldSize.H, Data->NewSize.W, Data->NewSize.H);
}


void
hihi(
	void* null,
	UIResizeData* Data
	)
{
	printf("    ui resize old %.02f %.02f new %.02f %.02f\n", Data->OldSize.W, Data->OldSize.H, Data->NewSize.W, Data->NewSize.H);
}


void
WindowOnMouseUp(
	int Button
	)
{
	printf("window mouseup %d\n", Button);
}


void
WindowOnMouseMove(
	float X,
	float Y
	)
{
	printf("window mousemove %f %f\n", X, Y);
}


void
WindowOnMouseScroll(
	float OffsetY
	)
{
	printf("window mousescroll %f\n", OffsetY);
}


DrawData
VulkanGetDrawData(
	void
	)
{
	return (DrawData){0};
}


Static void
GameInit(
	void
	)
{
	MutexInit(&StateMutex);

	SeedRand(WindowGetTime());

	EventListen(&WindowMouseDownTarget, (void*) lmfao, NULL);
	EventListen(&WindowMouseMoveTarget, (void*) lol, NULL);
	EventListen(&WindowResizeTarget, (void*) hehe, NULL);
}


Static void
GameFree(
	void
	)
{
	MutexDestroy(&StateMutex);
}


Static void
CreateUI(
	void
	)
{
	UIInit();

	UIElement* Window = UIAllocContainer(
		(UIElementInfo)
		{
			.Opacity = 0xFF
		},
		(UIContainerInfo)
		{
			.Axis = UI_AXIS_VERTICAL
		}
		);

	UISetRootElement(Window);

	UIElement* MiddleC = UIAllocContainer(
		(UIElementInfo)
		{
			.BorderRadius = 10.0f,
			.BorderColor = (ARGB){ 0x80D0D0D0 },
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_MIDDLE,
		},
		(UIContainerInfo)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoW = true,
			.AutoH = true,

			.WhiteColor = (ARGB){ 0x80D0D0D0 },
			.Texture = TEXTURE_RECT
		}
		);

	UIInsertLast(MiddleC, Window);

	UIElement* FirstC = UIAllocContainer(
		(UIElementInfo)
		{
			.Margin =
			(HalfExtent)
			{
				.Top = 5.0f,
				.Left = 5.0f,
				.Right = 5.0f,
				.Bottom = 5.0f
			},

			.BorderRadius = 5.0f,
			.BorderColor = (ARGB){ 0xFFFF1080 },
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_MIDDLE
		},
		(UIContainerInfo)
		{
			.Axis = UI_AXIS_VERTICAL,

			.AutoW = true,
			.AutoH = true,

			.WhiteColor = (ARGB){ 0xFFFF1080 },
			.Texture = TEXTURE_RECT
		}
		);

	UIInsertLast(FirstC, MiddleC);

	UIElement* CheckboxC = UIAllocContainer(
		(UIElementInfo)
		{
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		},
		(UIContainerInfo)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoW = true,
			.AutoH = true
		}
		);

	UIInsertLast(CheckboxC, FirstC);

	UIElement* Checkbox1 = UIAllocCheckbox(
		(UIElementInfo)
		{
			.Extent =
			(HalfExtent)
			{
				.W = 10.0f,
				.H = 10.0f
			},
			.Margin =
			(HalfExtent)
			{
				.Top = 5.0f,
				.Left = 5.0f,
				.Right = 5.0f,
				.Bottom = 5.0f
			},

			.BorderRadius = 10.0f,
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		},
		(UICheckboxInfo)
		{
			.Checked = true,

			.CheckYes = (ARGB){ 0xFF00FF00 },
			.CheckYesBackground = (ARGB){ 0xFFFFFFFF },

			.CheckNo = (ARGB){ 0xFFFF0000 },
			.CheckNoBackground = (ARGB){ 0xFFFFFFFF }
		}
		);

	UIInsertLast(Checkbox1, CheckboxC);

	UIElement* Checkbox2 = UIAllocCheckbox(
		(UIElementInfo)
		{
			.Extent =
			(HalfExtent)
			{
				.W = 0.0f,
				.H = 0.0f
			},
			.Margin =
			(HalfExtent)
			{
				.Top = 5.0f,
				.Left = 5.0f,
				.Right = 5.0f,
				.Bottom = 5.0f
			},

			.BorderRadius = 15.0f,
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		},
		(UICheckboxInfo)
		{
			.Checked = true,

			.CheckYes = (ARGB){ 0xFF00FF00 },
			.CheckYesBackground = (ARGB){ 0xFFFFFFFF },

			.CheckNo = (ARGB){ 0xFFFF0000 },
			.CheckNoBackground = (ARGB){ 0xFFFFFFFF }
		}
		);

	UIInsertLast(Checkbox2, CheckboxC);

	/*uint32_t Codes[] = { 'a', 'b', 'c', '\n', 'd', 'e', 'f', '\n' };

	UIElement* T = UIGetElement();
	*T = (UIElement)
	{
		.Type = UI_TYPE_TEXT,
		.Text =
		(UIText)
		{
			.Codepoints = Codes,
			.Length = ARRAYLEN(Codes),
			.MaxWidth = 120.0f,
			.FontSize = 100.0f
		}
	};

	puts("initializing our amazing text thing");

	UIInitialize(T);

	printf("\nafter initialization, we have %u lines\n", T->Text.LineCount);

	for(uint32_t i = 0; i < T->Text.LineCount; ++i)
	{
		UITextLine* Line = T->Text.Lines[i];
		printf("line #%u: %u glyphs\n", i + 1, Line->Length);

		for(uint32_t j = 0; j < Line->Length; ++j)
		{
			UITextGlyph* Glyph = Line->Glyphs + j;
			printf("glyph #%u: { Top: %.02f, Left: %.02f, Stride: %.02f, Size: %.02f, Texture: { Index: %hu, Layer: %hu } }\n",
				j + 1, Glyph->Top, Glyph->Left, Glyph->Stride, Glyph->Size, Glyph->Texture.Index, Glyph->Texture.Layer);
		}

		puts("");
	}

	puts("");*/

	/*IHandle m1 = ICreateContainer(
		&((IElement)
		{
			.H = 300.0f,

			.BorderTop = 10.0f,
			.BorderLeft = 10.0f,
			.BorderRight = 10.0f,
			.BorderBottom = 10.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0x333333FF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_MIDDLE,

			.InteractiveBorder = true
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_VERTICAL,

			.AutoW = true,
			.Scrollable = true,

			.WhiteColor = (ARGB){ 0x333333FF },
			.Texture = TEXTURE_RECT,
			.ScrollbarColor = (ARGB){ 0xFFAAAAAA },
			.ScrollbarAltColor = (ARGB){ 0xFF717171 },
		})
	);

	IHandle t1 = ICreateText(
		&((IElement)
		{
			.W = 1900.0f,

			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IContainer)
		{
			.AutoH = true,

			.WhiteColor = (ARGB){ 0xFFDDDDDD },
			.Texture = TEXTURE_RECT
		}),
		&((IText)
		{
			.Str = "A",

			.FontSize = 128,
			.AlignX = UI_ALIGN_LEFT,

			.Selectable = true,
			.Editable = true,

			.Stroke = (ARGB){ 0xFF000000 },
			.InverseStroke = (ARGB){ 0xFFFFFFFF },
			.Fill = (ARGB){ 0xFFFFFFFF },
			.InverseFill = (ARGB){ 0xFF000000 },
			.Background = (ARGB){ 0xA0000000 },

			.Type = I_TEXT_TYPE_MULTILINE_TEXT,
			.Data =
			(ITextData)
			{
				.HexColor =
				(ITextHexColor)
				{
					{ .ARGB = 0x87654321 }
				}
			}
		})
	);

	IHandle t2 = ICreateText(
		&((IElement)
		{
			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		NULL,
		&((IText)
		{
			.Str = "yes hello",

			.FontSize = 50,
			.AlignX = UI_ALIGN_CENTER,

			.Selectable = true,
			.Editable = true,

			.Stroke = (ARGB){ 0xFF000000 },
			.InverseStroke = (ARGB){ 0xFFFFFFFF },
			.Fill = (ARGB){ 0xFFFFFF20 },
			.InverseFill = (ARGB){ 0xFF000020 },
			.Background = (ARGB){ 0xA0000000 }
		})
	);

	IHandle s1 = ICreateSlider(
		&((IElement)
		{
			.W = 200.0f,
			.H = 20.0f,

			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((ISlider)
		{
			.Axis = UI_AXIS_HORIZONTAL,
			.Sections = 9,
			.Value = 4,

			.Color = (ARGB){ 0xFF4C99E5 },
			.BgColor =(ARGB){ 0xFFFFFFFF }
		})
	);

	IHandle p1 = ICreateColorPicker(
		&((IElement)
		{
			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IColorPicker)
		{
			.Color = (ARGB){ 0x802288FF }
		})
	);

	IHandle p2 = ICreateColorPicker(
		&((IElement)
		{
			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IColorPicker)
		{
			.Color = (ARGB){ 0x80FF8822 }
		})
	);

	IHandle d1 = ICreateDropdown(
		&((IElement)
		{
			.W = 130.0f,

			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.BorderTop = 5.0f,
			.BorderLeft = 5.0f,
			.BorderRight = 5.0f,
			.BorderBottom = 5.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0xFF000000 },

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IDropdown)
		{
			.BgColor = (ARGB){ 0xFFAAAAAA },

			.FontSize = 24,
			.Stroke = (ARGB){ 0xFF000000 },
			.Fill = (ARGB){ 0xFFFFFFFF },

			.Options =
			(IDropdownOption[])
			{
				{
					.Text = "Carrot"
				},
				{
					.Text = "Cabbage"
				},
				{
					.Text = "Potato"
				},
				{
					.Text = "Onion"
				}
			},
			.Count = 4,
			.Chosen = 2
		})
	);

	IHandle d2 = ICreateDropdown(
		&((IElement)
		{
			.W = 240.0f,

			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.BorderTop = 5.0f,
			.BorderLeft = 5.0f,
			.BorderRight = 5.0f,
			.BorderBottom = 5.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0xFF000000 },

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IDropdown)
		{
			.BgColor = (ARGB){ 0xFFAAAAAA },

			.FontSize = 24,
			.Stroke = (ARGB){ 0xFF000000 },
			.Fill = (ARGB){ 0xFFFFFFFF },

			.Options =
			(IDropdownOption[])
			{
				{
					.Text = "These can get very long"
				},
				{
					.Text = "and honestly could also contain"
				},
				{
					.Text = "miscellaneous elements besides text"
				},
				{
					.Text = "but for now it is what it is"
				}
			},
			.Count = 4,
			.Chosen = 0
		})
	);

	IHandle m2 = ICreateContainer(
		&((IElement)
		{
			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.BorderTop = 5.0f,
			.BorderLeft = 5.0f,
			.BorderRight = 5.0f,
			.BorderBottom = 5.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0xFF4C00FF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_TOP,
			.Position = UI_POSITION_RELATIVE,
			.RelativeAlignX = UI_ALIGN_CENTER,
			.RelativeAlignY = UI_ALIGN_TOP,

			.Relative = &p1
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoW = true,
			.AutoH = true,

			.Color = (ARGB){ 0xFF804CB2 },
			.Texture = TEXTURE_RECT
		})
	);

	IHandle c1 = ICreateCheckbox(
		&((IElement)
		{
			.W = 32.0f,
			.H = 32.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_CENTER
		}),
		&((ICheckbox)
		{
			.Checked = 1,

			.CheckYes = (ARGB){ 0xFF00FF00 },
			.CheckNo = (ARGB){ 0xFFFF0000 },
			.Background = (ARGB){ 0xFFFFFFFF }
		})
	);

	IHandle t3 = ICreateText(
		&((IElement)
		{
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		NULL,
		&((IText)
		{
			.Str = "checkbox test",

			.FontSize = 24,
			.AlignX = UI_ALIGN_CENTER,

			.Stroke = (ARGB){ 0xFFFFFFFF },
			.Fill = (ARGB){ 0xFF000000 }
		})
	);

	IHandle b1 = ICreateText(
		&((IElement)
		{
			.W = 150.0f,

			.MarginTop = 10.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 10.0f,

			.BorderTop = 5.0f,
			.BorderLeft = 5.0f,
			.BorderRight = 5.0f,
			.BorderBottom = 5.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0xFF4C00FF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_TOP,
			.Position = UI_POSITION_RELATIVE,
			.RelativeAlignX = UI_ALIGN_CENTER,
			.RelativeAlignY = UI_ALIGN_TOP,

			.Relative = &d1
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoH = true,
			.Scrollable = true,

			.Color = (ARGB){ 0xFFFFFFFF },
			.Texture = TEXTURE_RECT,
			.ScrollbarColor = (ARGB){ 0xFFAAAAAA },
			.ScrollbarAltColor = (ARGB){ 0xFF717171 },
		}),
		&((IText)
		{
			.Str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}\\|;:'\",.<>/?`~",
			.Placeholder = "Your nickname",

			.FontSize = 32,
			.AlignX = UI_ALIGN_CENTER,

			.Selectable = true,
			.Editable = true,

			.Stroke = (ARGB){ 0xFF000000 },
			.InverseStroke = (ARGB){ 0xFFFFFFFF },
			.Fill = (ARGB){ 0xFFFFFFFF },
			.InverseFill = (ARGB){ 0xFF000000 },
			.Background = (ARGB){ 0xA0000000 },

			.Type = I_TEXT_TYPE_SINGLELINE_TEXT
		})
	);


	IAddElementLast(&m1, &IWindow);

	IAddElementLast(&t1, &m1);
	// IAddElementLast(&t2, &m1);

	// IAddElementLast(&s1, &m1);

	// IAddElementLast(&p1, &m1);
	// IAddElementLast(&p2, &m1);

	// IAddElementLast(&d1, &m1);
	// IAddElementLast(&d2, &m1);

	// IAddElementAfter(&m2, &d2);
	// IAddElementLast(&c1, &m2);
	// IAddElementLast(&t3, &m2);

	// IAddElementAfter(&b1, &m2);
*/
	//UIActivate();
}


int
main(
	void
	)
{
	CreateUI();

	WindowInit();

	//SocketInit();

	GameInit();

	WindowRun();

	//SocketFree();

	GameFree();

	WindowFree();

	// UIFree();

	return 0;
}
