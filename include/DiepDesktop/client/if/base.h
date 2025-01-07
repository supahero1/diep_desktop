#pragma once

#include <DiepDesktop/client/ui/base.h>
#include <DiepDesktop/client/tex/base.h>


typedef struct IHandle IHandle;

typedef struct IElement
{
	float x;
	float y;
	float w;
	float h;

	float MarginTop;
	float MarginLeft;
	float MarginRight;
	float MarginBottom;

	float BorderTop;
	float BorderLeft;
	float BorderRight;
	float BorderBottom;

	uint32_t Opacity;
	color_argb_t BorderColor;

	UIAlign AlignX;
	UIAlign AlignY;
	UIPosition pos;
	UIAlign RelativeAlignX;
	UIAlign RelativeAlignY;

	bool InteractiveBorder;

	IHandle* Relative;

	UICallback Callback;
	void* data;
}
IElement;

struct IHandle
{
	UIElement* source;
	UIElement* Destination;
};


#define IWindow ((IHandle){ .source = UIWindow, .Destination = UIWindow })


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

	color_argb_t white_color;
	color_argb_t black_color;
	TexInfo tex;
	color_argb_t ScrollbarColor;
	color_argb_t ScrollbarAltColor;

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
	I_TEXT_TYPE__COUNT
}
ITextType;

typedef struct ITextInteger
{
	int64_t min;
	int64_t max;
	int64_t value;
}
ITextInteger;

typedef struct ITextHexColor
{
	color_argb_t color;
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
	uint32_t len;
	uint32_t max_len;
	float MaxWidth;

	float FontSize;
	UIAlign AlignX;

	bool Selectable;
	bool Editable;

	color_argb_t Stroke;
	color_argb_t InverseStroke;
	color_argb_t Fill;
	color_argb_t InverseFill;
	color_argb_t Background;

	ITextType type;
	ITextData data;
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
	int64_t value,
	bool Changed
	);


extern void
ITextSetInteger(
	IHandle Textbox,
	int64_t value
	);


extern color_argb_t
ITextGetHexColor(
	IHandle Textbox
	);


extern void
ITextSetHexColorExplicit(
	IHandle Textbox,
	color_argb_t color,
	bool Changed
	);


extern void
ITextSetHexColor(
	IHandle Textbox,
	color_argb_t color
	);



typedef struct ICheckbox
{
	uint32_t Checked;

	color_argb_t CheckYes;
	color_argb_t CheckNo;
	color_argb_t Background;
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
	float value;

	color_argb_t color;
	color_argb_t BgColor;
}
ISlider;

extern IHandle
ICreateSlider(
	const IElement* Element,
	const ISlider* Slider
	);


typedef struct IColorPicker
{
	color_argb_t color;
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
	uintptr_t data;
}
IDropdownOption;

typedef struct IDropdown
{
	color_argb_t BgColor;
	color_argb_t AltBgColor;

	uint32_t FontSize;
	color_argb_t Stroke;
	color_argb_t Fill;

	IDropdownOption* Options;
	uint32_t count;
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

	color_argb_t white_color;
	color_argb_t black_color;
	TexInfo tex;
	float angle;
}
ITexture;

extern IHandle
ICreateTexture(
	const IElement* Element,
	const ITexture* tex
	);

