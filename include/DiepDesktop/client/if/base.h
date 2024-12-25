#pragma once

#include <DiepDesktop/client/ui/base.h>
#include <DiepDesktop/client/tex/base.h>


typedef struct IHandle IHandle;

typedef struct IElement
{
	float X;
	float Y;
	float W;
	float H;

	float MarginTop;
	float MarginLeft;
	float MarginRight;
	float MarginBottom;

	float BorderTop;
	float BorderLeft;
	float BorderRight;
	float BorderBottom;

	uint32_t Opacity;
	ARGB BorderColor;

	UIAlign AlignX;
	UIAlign AlignY;
	UIPosition Position;
	UIAlign RelativeAlignX;
	UIAlign RelativeAlignY;

	bool InteractiveBorder;

	IHandle* Relative;

	UICallback Callback;
	void* Data;
}
IElement;

struct IHandle
{
	UIElement* Source;
	UIElement* Destination;
};


#define IWindow ((IHandle){ .Source = UIWindow, .Destination = UIWindow })


extern void
IUpdateElement(
	IHandle* Element
	);


extern void
IActivateElement(
	IHandle* Element
	);


extern void
IDeactivateElement(
	IHandle* Element
	);


extern void
IAddElementFirst(
	IHandle* Element,
	IHandle* Parent
	);


extern void
IAddElementLast(
	IHandle* Element,
	IHandle* Parent
	);


extern void
IAddElementBefore(
	IHandle* Element,
	IHandle* Before
	);


extern void
IAddElementAfter(
	IHandle* Element,
	IHandle* After
	);


extern void
IUnlinkElement(
	IHandle* Element
	);


extern void
IRemoveElement(
	IHandle* Element
	);



typedef struct IContainer
{
	UIAxis Axis;

	bool AutoW;
	bool AutoH;
	bool Clickable;
	bool ClickPassthrough;
	bool Selectable;
	bool Scrollable;
	bool ScrollPassthrough;

	ARGB WhiteColor;
	ARGB BlackColor;
	TexInfo Texture;
	ARGB ScrollbarColor;
	ARGB ScrollbarAltColor;

	UIElement* TextFocus;
}
IContainer;

extern IHandle
ICreateContainer(
	const IElement* Element,
	const IContainer* Container
	);


typedef enum ITextType
{
	I_TEXT_TYPE_MULTILINE_TEXT,
	I_TEXT_TYPE_SINGLELINE_TEXT,
	I_TEXT_TYPE_INTEGER,
	I_TEXT_TYPE_HEX_COLOR,
	kI_TEXT_TYPE
}
ITextType;

typedef struct ITextInteger
{
	int64_t Min;
	int64_t Max;
	int64_t Value;
}
ITextInteger;

typedef struct ITextHexColor
{
	ARGB Color;
}
ITextHexColor;

typedef union ITextData
{
	ITextInteger Integer;
	ITextHexColor HexColor;
}
ITextData;

typedef struct IText
{
	const char* Str;
	const char* Placeholder;
	uint32_t Length;
	uint32_t MaxLength;
	float MaxWidth;

	float FontSize;
	UIAlign AlignX;

	bool Selectable;
	bool Editable;

	ARGB Stroke;
	ARGB InverseStroke;
	ARGB Fill;
	ARGB InverseFill;
	ARGB Background;

	ITextType Type;
	ITextData Data;
}
IText;

extern IHandle
ICreateText(
	const IElement* Element,
	const IContainer* Container,
	const IText* Text
	);


extern int64_t
ITextGetInteger(
	IHandle Textbox
	);


extern void
ITextSetIntegerExplicit(
	IHandle Textbox,
	int64_t Value,
	bool Changed
	);


extern void
ITextSetInteger(
	IHandle Textbox,
	int64_t Value
	);


extern ARGB
ITextGetHexColor(
	IHandle Textbox
	);


extern void
ITextSetHexColorExplicit(
	IHandle Textbox,
	ARGB Color,
	bool Changed
	);


extern void
ITextSetHexColor(
	IHandle Textbox,
	ARGB Color
	);



typedef struct ICheckbox
{
	uint32_t Checked;

	ARGB CheckYes;
	ARGB CheckNo;
	ARGB Background;
}
ICheckbox;

extern IHandle
ICreateCheckbox(
	const IElement* Element,
	const ICheckbox* Checkbox
	);


typedef struct ISlider
{
	UIAxis Axis;
	float Sections;
	float Value;

	ARGB Color;
	ARGB BgColor;
}
ISlider;

extern IHandle
ICreateSlider(
	const IElement* Element,
	const ISlider* Slider
	);


typedef struct IColorPicker
{
	ARGB Color;
}
IColorPicker;

extern IHandle
ICreateColorPicker(
	const IElement* Element,
	const IColorPicker* ColorPicker
	);


typedef struct IDropdownOption
{
	const char* Text;
	uintptr_t Data;
}
IDropdownOption;

typedef struct IDropdown
{
	ARGB BgColor;
	ARGB AltBgColor;

	uint32_t FontSize;
	ARGB Stroke;
	ARGB Fill;

	IDropdownOption* Options;
	uint32_t Count;
	uint32_t Chosen;
}
IDropdown;

extern IHandle
ICreateDropdown(
	const IElement* Element,
	const IDropdown* Dropdown
	);


typedef struct ITexture
{
	bool UseExplicit;
	float SW;
	float SH;
	float OX;
	float OY;

	ARGB WhiteColor;
	ARGB BlackColor;
	TexInfo Texture;
	float Rotation;
}
ITexture;

extern IHandle
ICreateTexture(
	const IElement* Element,
	const ITexture* Texture
	);

