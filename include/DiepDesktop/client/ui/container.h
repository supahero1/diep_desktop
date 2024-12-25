#pragma once

#include <DiepDesktop/client/ui/base.h>
#include <DiepDesktop/client/tex/base.h>


typedef struct UIContainer
{
	UIAxis Axis;

	bool AutoW;
	bool AutoH;

	float ContentW;
	float ContentH;

	float MaxInlineW;
	float MaxInlineH;

	float GoalOffsetX;
	float GoalOffsetY;

	float OffsetX;
	float OffsetY;

	float OffsetMin;
	float OffsetMax;

	ARGB WhiteColor;
	ARGB BlackColor;
	TexInfo Texture;

	UIElement* Head;
	UIElement* Tail;
}
UIContainer;


typedef struct UIContainerInfo
{
	UIAxis Axis;

	bool AutoW;
	bool AutoH;

	ARGB WhiteColor;
	ARGB BlackColor;
	TexInfo Texture;
}
UIContainerInfo;


extern bool
UIElementIsContainer(
	UIElement* Element
	);


extern UIElement*
UIAllocContainer(
	UIElementInfo ElementInfo,
	UIContainerInfo Info
	);


extern void
UIInsertFirst(
	UIElement* Element,
	UIElement* Parent
	);


extern void
UIInsertLast(
	UIElement* Element,
	UIElement* Parent
	);


extern void
UIInsertBefore(
	UIElement* Element,
	UIElement* Before
	);


extern void
UIInsertAfter(
	UIElement* Element,
	UIElement* After
	);


extern bool
UIIsLinked(
	UIElement* Element
	);


extern void
UIUnlink(
	UIElement* Element
	);
