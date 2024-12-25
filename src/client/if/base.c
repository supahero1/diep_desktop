#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/client/if/base.h>

#include <string.h>
#include <inttypes.h>
#include <byteswap.h>

#include <utf8proc.h>

#define SCROLLBAR_SIZE 8.0f
#define SLIDER_SIZE 16.0f
#define SCROLL_STRENGTH 64.0f


Static UIElement* ColorPickerContainer;
Static UIElement* ColorPickerElement;
Static UIElement* ColorPickerBackground;
Static IHandle ColorPickerHexText;

typedef struct IColorPickerLine
{
	UISlider* Slider;
	IHandle Value;
}
IColorPickerLine;

Static IColorPickerLine ColorPickerBrightness;
Static IColorPickerLine ColorPickerOpacity;
Static IColorPickerLine ColorPickerRed;
Static IColorPickerLine ColorPickerGreen;
Static IColorPickerLine ColorPickerBlue;

Static UIElement* CurrentColorContainer;

Static UIElement* CurrentDropdown;

Static const char NoNewlineCharFilter[256] =
{
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , ' ',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	' ', '!', '"', '#', '$', '%', '&','\'', '(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[','\\', ']', '^', '_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~',  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
};

Static const char IntegerCharFilter[256] =
{
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , '+',  0 , '-',  0 ,  0 ,
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
};

Static const char HexColorCharFilter[256] =
{
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 , '#',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 , 'A', 'B', 'C', 'D', 'E', 'F',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 , 'a', 'b', 'c', 'd', 'e', 'f',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
};

Static const char* TextCharFilters[] =
{
	[I_TEXT_TYPE_MULTILINE_TEXT] = NULL,
	[I_TEXT_TYPE_SINGLELINE_TEXT] = NoNewlineCharFilter,
	[I_TEXT_TYPE_INTEGER] = IntegerCharFilter,
	[I_TEXT_TYPE_HEX_COLOR] = HexColorCharFilter,
};


void
IUpdateElement(
	IHandle* Element
	)
{
	UIUpdateElement(Element->Source);
}


void
IActivateElement(
	IHandle* Element
	)
{
	UIActivateElement(Element->Source);
}


void
IDeactivateElement(
	IHandle* Element
	)
{
	UIDeactivateElement(Element->Source);
}


void
IAddElementFirst(
	IHandle* Element,
	IHandle* Parent
	)
{
	UIAddElementFirst(Element->Source, Parent->Destination);
}


void
IAddElementLast(
	IHandle* Element,
	IHandle* Parent
	)
{
	UIAddElementLast(Element->Source, Parent->Destination);
}


void
IAddElementBefore(
	IHandle* Element,
	IHandle* Before
	)
{
	UIAddElementBefore(Element->Source, Before->Source);
}


void
IAddElementAfter(
	IHandle* Element,
	IHandle* After
	)
{
	UIAddElementAfter(Element->Source, After->Source);
}


void
IUnlinkElement(
	IHandle* Element
	)
{
	UIUnlinkElement(Element->Source);
}


void
IRemoveElement(
	IHandle* Element
	)
{
	UIRemoveElement(Element->Source);
}





typedef struct IPrivateContainerData
{
	UICallback Callback;
	void* Data;
}
IPrivateContainerData;


Static void
IContainerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertNotNull(Element->Data);
	IPrivateContainerData* Data = Element->Data;

	AssertTrue(Element->Scrollable);

	AssertEQ(Element->Type, UI_TYPE_CONTAINER);
	UIContainer* Parent = &Element->Container;
	AssertNotNull(Parent->Head);

	UIElement* Scrollable = Parent->Head;
	AssertEQ(Scrollable->Type, UI_TYPE_CONTAINER);
	UIContainer* Container = &Scrollable->Container;

	UIElement* Bar = Parent->Tail;
	AssertEQ(Scrollable->Next, Bar);


	switch(Event)
	{

	case UI_EVENT_FREE:
	{
		Data->Callback(Element, UI_EVENT_FREE);
		free(Data);

		break;
	}

	case UI_EVENT_SCROLL:
	{
		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			Container->GoalOffsetY += ScrollOffset * SCROLL_STRENGTH;
		}
		else
		{
			Container->GoalOffsetX += ScrollOffset * SCROLL_STRENGTH;
		}

		Fallthrough();
	}

	default:
	{
		IPrivateContainerData* Data = Element->Data;
		Data->Callback(Element, Event);
		break;
	}

	}
}


Static void
IScrollableContainerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_CONTAINER);

	UIElement* Parent = Element->Parent;
	AssertNotNull(Parent);
	AssertEQ(Parent->Type, UI_TYPE_CONTAINER);

	AssertNotNull(Parent->Data);
	IPrivateContainerData* Data = Parent->Data;

	UIElement* Scrollbar = Element->Next;
	AssertEQ(Scrollbar->Type, UI_TYPE_SCROLLBAR);


	switch(Event)
	{

	case UI_EVENT_MOUSE_IN:
	case UI_EVENT_MOUSE_OUT:
		/* Handled by the parent, don't want duplicates */
		break;

	case UI_EVENT_RESIZE:
	{
		UIScrollbarUpdate(Scrollbar);

		Fallthrough();
	}

	case UI_EVENT_FREE: break;

	default:
	{
		Data->Callback(Parent, Event);
		break;
	}

	}
}


IHandle
ICreateContainer(
	const IElement* Element,
	const IContainer* Container
	)
{
	IHandle Handle;

	if(!Container->Scrollable)
	{
		UIElement* C = UIGetElement();
		*C =
		(UIElement)
		{
			.X = Element->X,
			.Y = Element->Y,
			.W = Element->W,
			.H = Element->H,

			.MarginTop = Element->MarginTop,
			.MarginLeft = Element->MarginLeft,
			.MarginRight = Element->MarginRight,
			.MarginBottom = Element->MarginBottom,

			.BorderTop = Element->BorderTop,
			.BorderLeft = Element->BorderLeft,
			.BorderRight = Element->BorderRight,
			.BorderBottom = Element->BorderBottom,

			.Opacity = Element->Opacity,
			.BorderColor = Element->BorderColor,

			.AlignX = Element->AlignX,
			.AlignY = Element->AlignY,
			.Position = Element->Position,
			.RelativeAlignX = Element->RelativeAlignX,
			.RelativeAlignY = Element->RelativeAlignY,

			.AutoW = Container->AutoW,
			.AutoH = Container->AutoH,
			.Clickable = Container->Clickable,
			.ClickPassthrough = Container->ClickPassthrough,
			.Selectable = Container->Selectable,
			.ScrollPassthrough = Container->ScrollPassthrough,
			.InteractiveBorder = Element->InteractiveBorder,

			.TextFocus = Container->TextFocus,
			.Relative = Element->Relative ? Element->Relative->Source : NULL,

			.Callback = Element->Callback,
			.Data = Element->Data,

			.Type = UI_TYPE_CONTAINER,
			.Container =
			(UIContainer)
			{
				.Axis = Container->Axis,

				.WhiteColor = Container->WhiteColor,
				.BlackColor = Container->BlackColor,
				.Texture = Container->Texture
			}
		};

		UIInitialize(C);

		Handle.Source = Handle.Destination = C;
	}
	else
	{
		AssertFalse((Container->AutoW || Container->AutoH));

		IPrivateContainerData* Data = malloc(sizeof(IPrivateContainerData));
		AssertNotNull(Data);

		Data->Callback = Element->Callback ? Element->Callback : UIEmptyCallback;
		Data->Data = Element->Data;

		UIElement* P = UIGetElement();
		*P =
		(UIElement)
		{
			.X = Element->X,
			.Y = Element->Y,
			.W = Element->W,
			.H = Element->H,

			.MarginTop = Element->MarginTop,
			.MarginLeft = Element->MarginLeft,
			.MarginRight = Element->MarginRight,
			.MarginBottom = Element->MarginBottom,

			.BorderTop = Element->BorderTop,
			.BorderLeft = Element->BorderLeft,
			.BorderRight = Element->BorderRight,
			.BorderBottom = Element->BorderBottom,

			.Opacity = Element->Opacity,
			.BorderColor = Element->BorderColor,

			.AlignX = Element->AlignX,
			.AlignY = Element->AlignY,
			.Position = Element->Position,
			.RelativeAlignX = Element->RelativeAlignX,
			.RelativeAlignY = Element->RelativeAlignY,

			.AutoW = Container->AutoW,
			.AutoH = Container->AutoH,
			.Clickable = Container->Clickable,
			.ClickPassthrough = Container->ClickPassthrough,
			.Selectable = Container->Selectable,
			.Scrollable = true,
			.ScrollPassthrough = Container->ScrollPassthrough,
			.InteractiveBorder = Element->InteractiveBorder,

			.Relative = Element->Relative ? Element->Relative->Source : NULL,

			.Callback = IContainerCallback,
			.Data = Data,

			.Type = UI_TYPE_CONTAINER,
			.Container =
			(UIContainer)
			{
				.Axis = !Container->Axis,

				.WhiteColor = Container->WhiteColor,
				.BlackColor = Container->BlackColor,
				.Texture = Container->Texture
			}
		};

		UIInitialize(P);

		float W;
		float H;
		bool AutoW;
		bool AutoH;
		float ScrollbarW;
		float ScrollbarH;

		if(Container->Axis == UI_AXIS_HORIZONTAL)
		{
			/* No point in making it scrollable if content never overflows */
			AssertFalse(Container->AutoW);

			W = 0.0f;
			AutoW = true;

			if(Container->AutoH)
			{
				H = 0.0f;
				AutoH = true;
			}
			else
			{
				H = Element->H - SCROLLBAR_SIZE;
				AutoH = false;
			}

			ScrollbarW = Element->W;
			ScrollbarH = SCROLLBAR_SIZE;
		}
		else
		{
			AssertFalse(Container->AutoH);

			if(Container->AutoW)
			{
				W = 0.0f;
				AutoW = true;
			}
			else
			{
				W = Element->W - SCROLLBAR_SIZE;
				AutoW = false;
			}

			H = 0.0f;
			AutoH = true;

			ScrollbarW = SCROLLBAR_SIZE;
			ScrollbarH = Element->H;
		}

		UIElement* C = UIGetElement();
		*C =
		(UIElement)
		{
			.W = W,
			.H = H,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP,

			.AutoW = AutoW,
			.AutoH = AutoH,
			.Scrollable = true,
			.ScrollPassthrough = true,

			.Callback = IScrollableContainerCallback,

			.Type = UI_TYPE_CONTAINER,
			.Container =
			(UIContainer)
			{
				.Axis = Container->Axis
			}
		};

		UIInitialize(C);

		UIElement* S = UIGetElement();
		*S =
		(UIElement)
		{
			.W = ScrollbarW,
			.H = ScrollbarH,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP,

			.Clickable = true,
			.ScrollPassthrough = true,

			.Callback = UIScrollbarCallback,

			.Type = UI_TYPE_SCROLLBAR,
			.Scrollbar =
			(UIScrollbar)
			{
				.Axis = Container->Axis,

				.Color = Container->ScrollbarColor,
				.AltColor = Container->ScrollbarAltColor
			}
		};

		UIInitialize(S);

		UIAddElementLast(C, P);
		UIAddElementLast(S, P);

		Handle.Source = P;
		Handle.Destination = C;
	}

	return Handle;
}





typedef struct IPrivateTextData
{
	UICallback Callback;
	void* Data;

	ITextType Type;
	ITextData TextData;

	UIElement* Placeholder;
}
IPrivateTextData;


Static const uint32_t*
SkipCodepoint(
	const uint32_t* Str,
	const uint32_t Char
	)
{
	const uint32_t* S = Str;

	while(*S && *S == Char)
	{
		++S;
	}

	if(S != Str)
	{
		--S;
	}

	return S;
}


Static void
ITextWrite(
	UIElement* Element,
	char* Str,
	uint32_t Length,
	bool Changed
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	uint32_t* Codepoints = malloc(Length * sizeof(uint32_t));
	AssertNotNull(Codepoints);

	uint32_t Count = utf8proc_decompose((const uint8_t*) Str,
		Length, (int32_t*) Codepoints, Length, UTF8PROC_NULLTERM);
	AssertLE(Count, Length);

	Codepoints = realloc(Codepoints, Count * sizeof(uint32_t));
	AssertNotNull(Codepoints);

	free(Text->Codepoints);
	Text->Codepoints = Codepoints;
	Text->Length = Count;

	UIUpdateElement(Element);

	IPrivateTextData* Data = Element->Data;
	AssertNotNull(Data);

	if(Changed)
	{
		Data->Callback(Element, UI_EVENT_CHANGE);
	}
}


typedef char*
(*ValueToTextFunc)(
	char* Str,
	uint32_t* Length,
	const ITextData* TextData
	);

typedef void
(*TextSubmitFunc)(
	UIElement* Element
	);


Static char*
IntegerToText(
	char* Str,
	uint32_t* Length,
	const ITextData* TextData
	)
{
	if(!Str)
	{
		Str = malloc(32);
		AssertNotNull(Str);
	}

	int64_t Value = TextData->Integer.Value;

	int Len = snprintf(Str, 32, "%" PRIi64, Value);
	AssertGT(Len, 0);

	if(Length)
	{
		*Length = Len;
	}

	return Str;
}


Static void
ITextOnIntegerSubmit(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	AssertNotNull(Element->Data);
	IPrivateTextData* Data = Element->Data;
	AssertEQ(Data->Type, I_TEXT_TYPE_INTEGER);
	ITextData* TextData = &Data->TextData;
	ITextInteger* Integer = &TextData->Integer;

	const uint32_t* S = SkipCodepoint(Text->Codepoints, ' ');
	S = SkipCodepoint(S, '-');
	S = SkipCodepoint(S, '+');

	int64_t Value = MIN(MAX(strtoll(S, NULL, 10), Integer->Min), Integer->Max);
	bool Changed = Integer->Value != Value;
	Integer->Value = Value;

	char Str[32];
	uint32_t Length;
	IntegerToText(Str, &Length, TextData);

	ITextWrite(Element, Str, Length, Changed);
}


Static char*
HexColorToText(
	char* Str,
	uint32_t* Length,
	const ITextData* TextData
	)
{
	if(!Str)
	{
		Str = malloc(16);
		AssertNotNull(Str);
	}

	ARGB Color = TextData->HexColor.Color;

	int Len = snprintf(Str, 16, "#%02X%02X%02X%02X", Color.R, Color.G, Color.B, Color.A);
	AssertGT(Len, 0);
	*Length = Len;

	return Str;
}


Static void
ITextOnHexColorSubmit(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	AssertNotNull(Element->Data);
	IPrivateTextData* Data = Element->Data;
	AssertEQ(Data->Type, I_TEXT_TYPE_HEX_COLOR);
	ITextData* TextData = &Data->TextData;
	ITextHexColor* HexColor = &TextData->HexColor;

	const char* S = SkipCodepoint(Text->Str, ' ');
	S = SkipCodepoint(S, '#');
	if(*S == '#')
	{
		++S;
	}


	uint32_t Value = strtoul(S, NULL, 16);
	ARGB Color;


	switch(strlen(S))
	{

	case 1:
	{
		Color =
		(ARGB)
		{
			.B = Value * 0x11,
			.G = Value * 0x11,
			.R = Value * 0x11,
			.A = 0xFF
		};

		break;
	}

	case 3:
	{
		Color =
		(ARGB)
		{
			.B = (Value & 0xF) * 0x11,
			.G = ((Value >> 4) & 0xF) * 0x11,
			.R = ((Value >> 8) & 0xF) * 0x11,
			.A = 0xFF
		};

		break;
	}

	case 4:
	{
		Color =
		(ARGB)
		{
			.B = ((Value >> 4) & 0xF) * 0x11,
			.G = ((Value >> 8) & 0xF) * 0x11,
			.R = ((Value >> 12) & 0xF) * 0x11,
			.A = (Value & 0xF) * 0x11
		};

		break;
	}

	case 6:
	{
		Color =
		(ARGB)
		{
			.B = Value & 0xFF,
			.G = (Value >> 8) & 0xFF,
			.R = (Value >> 16) & 0xFF,
			.A = 0xFF
		};

		break;
	}

	case 8:
	{
		Color =
		(ARGB)
		{
			.B = (Value >> 8) & 0xFF,
			.G = (Value >> 16) & 0xFF,
			.R = (Value >> 24) & 0xFF,
			.A = Value & 0xFF
		};

		break;
	}

	default:
	{
		Color = HexColor->Color;
		break;
	}

	}


	bool Changed = HexColor->Color.ARGB != Color.ARGB;
	HexColor->Color = Color;

	char Str[16];
	uint32_t Length;
	HexColorToText(Str, &Length, TextData);

	ITextWrite(Element, Str, Length, Changed);
}


Static ValueToTextFunc ValueToText[] =
{
	[I_TEXT_TYPE_MULTILINE_TEXT] = NULL,
	[I_TEXT_TYPE_SINGLELINE_TEXT] = NULL,
	[I_TEXT_TYPE_INTEGER] = IntegerToText,
	[I_TEXT_TYPE_HEX_COLOR] = HexColorToText,
};


Static TextSubmitFunc TextSubmit[] =
{
	[I_TEXT_TYPE_MULTILINE_TEXT] = NULL,
	[I_TEXT_TYPE_SINGLELINE_TEXT] = NULL,
	[I_TEXT_TYPE_INTEGER] = ITextOnIntegerSubmit,
	[I_TEXT_TYPE_HEX_COLOR] = ITextOnHexColorSubmit,
};


Static void
ITextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	AssertNotNull(Element->Data);
	IPrivateTextData* Data = Element->Data;

	UITextCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_CHANGE:
	{
		if(!Data->Placeholder)
		{
			break;
		}

		if(Text->Str[0] == 0)
		{
			if(!UIIsLinked(Data->Placeholder))
			{
				UIAddElementBefore(Data->Placeholder, Element);
			}
		}
		else
		{
			if(UIIsLinked(Data->Placeholder))
			{
				UIUnlinkElement(Data->Placeholder);
			}
		}

		break;
	}

	case UI_EVENT_SUBMIT:
	{
		TextSubmitFunc OnSubmit = TextSubmit[Data->Type];
		if(OnSubmit)
		{
			OnSubmit(Element);
		}

		Data->Callback(Element, UI_EVENT_SUBMIT);
		break;
	}

	case UI_EVENT_FREE:
	{
		free(Text->Str);

		Data->Callback(Element, UI_EVENT_FREE);
		free(Data);

		break;
	}

	default:
	{
		Data->Callback(Element, Event);
		break;
	}

	}
}


Static void
ITextPlaceholderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	UITextCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	{
		AssertNotNull(Element->Next);
		UIElement* ActualText = Element->Next;
		UIFocusText(ActualText);
		break;
	}

	case UI_EVENT_FREE:
	{
		free(Text->Str);
		break;
	}

	default: break;

	}
}


Static void
ITextContainerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_CONTAINER);
	UIContainer* Container = &Element->Container;

	if(Event != UI_EVENT_MOUSE_DOWN)
	{
		return;
	}

	AssertNotNull(Container->Tail);
	UIElement* ActualText = Container->Tail;
	UIFocusText(ActualText);
}


IHandle
ICreateText(
	const IElement* Element,
	const IContainer* Container,
	const IText* Text
	)
{
	IPrivateTextData* Data = malloc(sizeof(IPrivateTextData));
	AssertNotNull(Data);

	Data->Callback = Element->Callback ? Element->Callback : UIEmptyCallback;
	Data->Data = Element->Data;

	Data->Type = Text->Type;
	Data->TextData = Text->Data;

	AssertLT(Text->Type, kI_TEXT_TYPE);
	const char* CharFilter = TextCharFilters[Text->Type];

	char* Str;
	uint32_t Length;
	if(!Text->Str)
	{
		ValueToTextFunc ToText = ValueToText[Text->Type];
		if(ToText)
		{
			Str = ToText(NULL, &Length, &Text->Data);
		}
		else
		{
			Str = malloc(1);
			AssertNotNull(Str);

			Str[0] = 0;
			Length = 0;
		}
	}
	else
	{
		if(!Text->Length)
		{
			Length = strlen(Text->Str);
		}
		else
		{
			Length = Text->Length;
		}

		Str = malloc(Length + 1);
		AssertNotNull(Str);

		memcpy(Str, Text->Str, Length + 1);
	}

	UIElement* T = UIGetElement();
	*T =
	(UIElement)
	{
		.Opacity = Element->Opacity,

		.AlignX = Element->AlignX,
		.AlignY = Element->AlignY,
		.Position = Element->Position,
		.RelativeAlignX = Element->RelativeAlignX,
		.RelativeAlignY = Element->RelativeAlignY,

		.AutoH = true,
		.Clickable = true,
		.Selectable = true,
		.ScrollPassthrough = true,

		.Callback = ITextCallback,
		.Data = Data,

		.Type = UI_TYPE_TEXT,
		.Text =
		(UIText)
		{
			.Str = Str,
			.Length = Text->Length,
			.MaxLength = Text->MaxLength,
			.MaxWidth = Text->MaxWidth,

			.FontSize = Text->FontSize,
			.AlignX = Text->AlignX,

			.Selectable = Text->Selectable,
			.Editable = Text->Editable,

			.Stroke = Text->Stroke,
			.InverseStroke = Text->InverseStroke,
			.Fill = Text->Fill,
			.InverseFill = Text->InverseFill,
			.Background = Text->Background,

			.CharFilter = CharFilter
		}
	};

	UIInitialize(T);

	if(Text->Placeholder)
	{
		UIElement* P = UIGetElement();
		*P =
		(UIElement)
		{
			.Opacity = AmulA(Element->Opacity, 0xB0),

			.AlignX = Text->AlignX,
			.AlignY = UI_ALIGN_TOP,
			.Position = UI_POSITION_INLINE,

			.AutoH = true,
			.Clickable = true,
			.Selectable = true,
			.ScrollPassthrough = true,

			.TextFocus = T,

			.Callback = ITextPlaceholderCallback,

			.Type = UI_TYPE_TEXT,
			.Text =
			(UIText)
			{
				.Str = strdup(Text->Placeholder),
				.MaxLength = Text->MaxLength,
				.MaxWidth = Text->MaxWidth,

				.FontSize = Text->FontSize,
				.AlignX = Text->AlignX,

				.Stroke = Text->Stroke,
				.InverseStroke = Text->InverseStroke,
				.Fill = Text->Fill,
				.InverseFill = Text->InverseFill,
				.Background = Text->Background
			}
		};

		UIInitialize(P);
		UIActivateElement(P);

		Data->Placeholder = P;
	}
	else
	{
		Data->Placeholder = NULL;
	}

	IHandle C = ICreateContainer(
		&((IElement)
		{
			.X = Element->X,
			.Y = Element->Y,
			.W = Element->W,
			.H = Element->H,

			.MarginTop = Element->MarginTop,
			.MarginLeft = Element->MarginLeft,
			.MarginRight = Element->MarginRight,
			.MarginBottom = Element->MarginBottom,

			.BorderTop = Element->BorderTop,
			.BorderLeft = Element->BorderLeft,
			.BorderRight = Element->BorderRight,
			.BorderBottom = Element->BorderBottom,

			.Opacity = Element->Opacity,
			.BorderColor = Element->BorderColor,

			.AlignX = Element->AlignX,
			.AlignY = Element->AlignY,
			.Position = Element->Position,
			.RelativeAlignX = Element->RelativeAlignX,
			.RelativeAlignY = Element->RelativeAlignY,

			.InteractiveBorder = Element->InteractiveBorder,

			.Relative = Element->Relative,

			.Callback = Container ? ITextContainerCallback : NULL
		}
		), Container ?
		&((IContainer)
		{
			.AutoW = Container->AutoW,
			.AutoH = Container->AutoH,
			.Clickable = true,
			.Selectable = true,
			.ScrollPassthrough = true,

			.WhiteColor = Container->WhiteColor,
			.BlackColor = Container->BlackColor,
			.Texture = Container->Texture,

			.TextFocus = T
		}) :
		&((IContainer)
		{
			.AutoW = true,
			.AutoH = true,
			.ClickPassthrough = true,
			.ScrollPassthrough = true,

			.TextFocus = T
		})
	);

	UIAddElementLast(T, C.Destination);

	return (IHandle) { .Source = C.Source, .Destination = NULL };
}





typedef struct IPrivateCheckboxData
{
	UICallback Callback;
	void* Data;
}
IPrivateCheckboxData;


Static void
ICheckboxCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_CHECKBOX);
	UICheckbox* Checkbox = &Element->Checkbox;

	(void) Checkbox;

	AssertNotNull(Element->Data);
	IPrivateCheckboxData* Data = Element->Data;

	UICheckboxCallback(Element, Event);

	Data->Callback(Element, Event);
}


IHandle
ICreateCheckbox(
	const IElement* Element,
	const ICheckbox* Checkbox
	)
{
	IPrivateCheckboxData* Data = malloc(sizeof(IPrivateCheckboxData));
	AssertNotNull(Data);

	Data->Callback = Element->Callback ? Element->Callback : UIEmptyCallback;
	Data->Data = Element->Data;

	UIElement* T = UIGetElement();
	*T =
	(UIElement)
	{
		.X = Element->X,
		.Y = Element->Y,
		.W = Element->W,
		.H = Element->H,

		.MarginTop = Element->MarginTop,
		.MarginLeft = Element->MarginLeft,
		.MarginRight = Element->MarginRight,
		.MarginBottom = Element->MarginBottom,

		.BorderTop = Element->BorderTop,
		.BorderLeft = Element->BorderLeft,
		.BorderRight = Element->BorderRight,
		.BorderBottom = Element->BorderBottom,

		.Opacity = Element->Opacity,
		.BorderColor = Element->BorderColor,

		.AlignX = Element->AlignX,
		.AlignY = Element->AlignY,
		.Position = Element->Position,
		.RelativeAlignX = Element->RelativeAlignX,
		.RelativeAlignY = Element->RelativeAlignY,

		.Clickable = true,
		.InteractiveBorder = Element->InteractiveBorder,

		.Relative = Element->Relative ? Element->Relative->Source : NULL,

		.Callback = ICheckboxCallback,
		.Data = Data,

		.Type = UI_TYPE_CHECKBOX,
		.Checkbox =
		(UICheckbox)
		{
			.Checked = Checkbox->Checked,

			.CheckYes = Checkbox->CheckYes,
			.CheckNo = Checkbox->CheckNo,
			.Background = Checkbox->Background
		}
	};

	UIInitialize(T);

	return (IHandle) { .Source = T, .Destination = NULL };
}


Static UIElement*
ITextGetTextElement(
	IHandle Text
	)
{
	UIElement* T;

	if(Text.Source->Type == UI_TYPE_CONTAINER)
	{
		T = Text.Source->Container.Head;
	}
	else
	{
		T = Text.Source;
	}
	AssertEQ(T->Type, UI_TYPE_TEXT);

	return T;
}


int64_t
ITextGetInteger(
	IHandle Text
	)
{
	UIElement* T = ITextGetTextElement(Text);

	IPrivateTextData* Data = T->Data;
	AssertNotNull(Data);

	ITextData* TextData = &Data->TextData;
	AssertEQ(Data->Type, I_TEXT_TYPE_INTEGER);
	ITextInteger* Integer = &TextData->Integer;

	return Integer->Value;
}


void
ITextSetIntegerExplicit(
	IHandle Text,
	int64_t Value,
	bool Changed
	)
{
	UIElement* T = ITextGetTextElement(Text);

	IPrivateTextData* Data = T->Data;
	AssertNotNull(Data);

	ITextData* TextData = &Data->TextData;
	AssertEQ(Data->Type, I_TEXT_TYPE_INTEGER);
	ITextInteger* Integer = &TextData->Integer;

	Integer->Value = Value;

	char Str[32];
	uint32_t Length;
	IntegerToText(Str, &Length, TextData);

	ITextWrite(T, Str, Length, Changed);
}


void
ITextSetInteger(
	IHandle Text,
	int64_t Value
	)
{
	ITextSetIntegerExplicit(Text, Value, true);
}


ARGB
ITextGetHexColor(
	IHandle Text
	)
{
	UIElement* T = ITextGetTextElement(Text);

	IPrivateTextData* Data = T->Data;
	AssertNotNull(Data);

	ITextData* TextData = &Data->TextData;
	AssertEQ(Data->Type, I_TEXT_TYPE_HEX_COLOR);
	ITextHexColor* HexColor = &TextData->HexColor;

	return HexColor->Color;
}


void
ITextSetHexColorExplicit(
	IHandle Text,
	ARGB Color,
	bool Changed
	)
{
	UIElement* T = ITextGetTextElement(Text);

	IPrivateTextData* Data = T->Data;
	AssertNotNull(Data);

	ITextData* TextData = &Data->TextData;
	AssertEQ(Data->Type, I_TEXT_TYPE_HEX_COLOR);
	ITextHexColor* HexColor = &TextData->HexColor;

	HexColor->Color = Color;

	char Str[16];
	uint32_t Length;
	HexColorToText(Str, &Length, TextData);

	ITextWrite(T, Str, Length, Changed);
}


void
ITextSetHexColor(
	IHandle Value,
	ARGB Color
	)
{
	ITextSetHexColorExplicit(Value, Color, true);
}





typedef struct IPrivateSliderData
{
	UICallback Callback;
	void* Data;
}
IPrivateSliderData;


Static void
ISliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_SLIDER);
	// UISlider* Slider = &Element->Slider;

	AssertNotNull(Element->Data);
	IPrivateSliderData* Data = Element->Data;

	UISliderCallback(Element, Event);

	Data->Callback(Element, Event);
}


IHandle
ICreateSlider(
	const IElement* Element,
	const ISlider* Slider
	)
{
	AssertGE(Slider->Value, 0);
	AssertLT(Slider->Value, Slider->Sections);

	IPrivateSliderData* Data = malloc(sizeof(IPrivateSliderData));
	AssertNotNull(Data);

	Data->Callback = Element->Callback ? Element->Callback : UIEmptyCallback;
	Data->Data = Element->Data;

	UIElement* T = UIGetElement();
	*T =
	(UIElement)
	{
		.X = Element->X,
		.Y = Element->Y,
		.W = Element->W,
		.H = Element->H,

		.MarginTop = Element->MarginTop,
		.MarginLeft = Element->MarginLeft,
		.MarginRight = Element->MarginRight,
		.MarginBottom = Element->MarginBottom,

		.BorderTop = Element->BorderTop,
		.BorderLeft = Element->BorderLeft,
		.BorderRight = Element->BorderRight,
		.BorderBottom = Element->BorderBottom,

		.Opacity = Element->Opacity,
		.BorderColor = Element->BorderColor,

		.AlignX = Element->AlignX,
		.AlignY = Element->AlignY,
		.Position = Element->Position,
		.RelativeAlignX = Element->RelativeAlignX,
		.RelativeAlignY = Element->RelativeAlignY,

		.Clickable = true,
		.InteractiveBorder = Element->InteractiveBorder,

		.Relative = Element->Relative ? Element->Relative->Source : NULL,

		.Callback = ISliderCallback,
		.Data = Data,

		.Type = UI_TYPE_SLIDER,
		.Slider =
		(UISlider)
		{
			.Axis = Slider->Axis,
			.Sections = Slider->Sections,
			.Value = Slider->Value,

			.Color = Slider->Color,
			.BgColor = Slider->BgColor,

			.Type = UI_SLIDER_TYPE_NORMAL
		}
	};

	UIInitialize(T);

	return (IHandle) { .Source = T, .Destination = NULL };
}





Static void
IColorPickerUpdate(
	ARGB Color
	)
{
	ARGB OpaqueColor = Color;
	OpaqueColor.A = 255;

	ColorPickerElement->ColorPicker.ColorRGB = OpaqueColor;
	float Value = UIColorPickerUpdate(ColorPickerElement);
	uint32_t Brightness = nearbyintf(Value * 255.0f);

	ColorPickerBrightness.Slider->Color = ColorPickerElement->ColorPicker.ColorRGB;
	ColorPickerOpacity.Slider->Color = OpaqueColor;
	ColorPickerRed.Slider->Color = OpaqueColor;
	ColorPickerGreen.Slider->Color = OpaqueColor;
	ColorPickerBlue.Slider->Color = OpaqueColor;

	CurrentColorContainer->Container.WhiteColor = Color;
	ColorPickerBackground->Container.WhiteColor = Color;

	ColorPickerBrightness.Slider->Value = Brightness;
	ColorPickerOpacity.Slider->Value = Color.A;
	ColorPickerRed.Slider->Value = Color.R;
	ColorPickerGreen.Slider->Value = Color.G;
	ColorPickerBlue.Slider->Value = Color.B;

	ITextSetHexColorExplicit(ColorPickerHexText, Color, false);

	ITextSetIntegerExplicit(ColorPickerBrightness.Value, Brightness, false);
	ITextSetIntegerExplicit(ColorPickerOpacity.Value, Color.A, false);
	ITextSetIntegerExplicit(ColorPickerRed.Value, Color.R, false);
	ITextSetIntegerExplicit(ColorPickerGreen.Value, Color.G, false);
	ITextSetIntegerExplicit(ColorPickerBlue.Value, Color.B, false);

	CurrentColorContainer->Callback(CurrentColorContainer, UI_EVENT_CHANGE);
}


Static void
IHexTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ARGB Color = ITextGetHexColor(ColorPickerHexText);
	IColorPickerUpdate(Color);
}


Static void
IColorPickerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UIColorPickerCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		HSV ColorHSV = RGBtoHSV(CurrentColorContainer->Container.WhiteColor);
		HSV PickedColor = ColorPickerElement->ColorPicker.ColorHSV;
		ColorHSV.H = PickedColor.H;
		ColorHSV.S = PickedColor.S;
		ARGB ColorRGB = HSVtoRGB(ColorHSV);
		ColorRGB.A = CurrentColorContainer->Container.WhiteColor.A;
		IColorPickerUpdate(ColorRGB);
		break;
	}

	default: break;

	}
}


Static void
IBrightnessSliderRecalculate(
	void
	)
{
	HSV ColorHSV = ColorPickerElement->ColorPicker.ColorHSV;
	ColorHSV.V = ColorPickerBrightness.Slider->Value / 255.0f;
	ARGB ColorRGB = HSVtoRGB(ColorHSV);
	ColorPickerElement->ColorPicker.ColorRGB = ColorRGB;
	ColorRGB.A = ColorPickerOpacity.Slider->Value;
	IColorPickerUpdate(ColorRGB);
}


Static void
IBrightnessSliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UISliderCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		IBrightnessSliderRecalculate();
		break;
	}

	default: break;

	}
}


Static void
IBrightnessTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ColorPickerBrightness.Slider->Value = ITextGetInteger(ColorPickerBrightness.Value);
	IBrightnessSliderRecalculate();
}


Static void
IOpacitySliderRecalculate(
	void
	)
{
	ARGB ColorRGB = CurrentColorContainer->Container.WhiteColor;
	ColorRGB.A = ColorPickerOpacity.Slider->Value;
	IColorPickerUpdate(ColorRGB);
}


Static void
IOpacitySliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UISliderCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		IOpacitySliderRecalculate();
		break;
	}

	default: break;

	}
}


Static void
IOpacityTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ColorPickerOpacity.Slider->Value = ITextGetInteger(ColorPickerOpacity.Value);
	IOpacitySliderRecalculate();
}


Static void
IRedSliderRecalculate(
	void
	)
{
	ARGB ColorRGB = CurrentColorContainer->Container.WhiteColor;
	ColorRGB.R = ColorPickerRed.Slider->Value;
	IColorPickerUpdate(ColorRGB);
}


Static void
IRedSliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UISliderCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		IRedSliderRecalculate();
		break;
	}

	default: break;

	}
}


Static void
IRedTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ColorPickerRed.Slider->Value = ITextGetInteger(ColorPickerRed.Value);
	IRedSliderRecalculate();
}


Static void
IGreenSliderRecalculate(
	void
	)
{
	ARGB ColorRGB = CurrentColorContainer->Container.WhiteColor;
	ColorRGB.G = ColorPickerGreen.Slider->Value;
	IColorPickerUpdate(ColorRGB);
}


Static void
IGreenSliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UISliderCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		IGreenSliderRecalculate();
		break;
	}

	default: break;

	}
}


Static void
IGreenTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ColorPickerGreen.Slider->Value = ITextGetInteger(ColorPickerGreen.Value);
	IGreenSliderRecalculate();
}


Static void
IBlueSliderRecalculate(
	void
	)
{
	ARGB ColorRGB = CurrentColorContainer->Container.WhiteColor;
	ColorRGB.B = ColorPickerBlue.Slider->Value;
	IColorPickerUpdate(ColorRGB);
}


Static void
IBlueSliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UISliderCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		IBlueSliderRecalculate();
		break;
	}

	default: break;

	}
}


Static void
IBlueTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ColorPickerBlue.Slider->Value = ITextGetInteger(ColorPickerBlue.Value);
	IBlueSliderRecalculate();
}


Static UICallback ColorPickerSliderCallback[] =
{
	[UI_SLIDER_TYPE_BRIGHTNESS] = IBrightnessSliderCallback,
	[UI_SLIDER_TYPE_OPACITY] = IOpacitySliderCallback,
	[UI_SLIDER_TYPE_RED] = IRedSliderCallback,
	[UI_SLIDER_TYPE_GREEN] = IGreenSliderCallback,
	[UI_SLIDER_TYPE_BLUE] = IBlueSliderCallback
};


Static UICallback ColorPickerTextCallback[] =
{
	[UI_SLIDER_TYPE_BRIGHTNESS] = IBrightnessTextCallback,
	[UI_SLIDER_TYPE_OPACITY] = IOpacityTextCallback,
	[UI_SLIDER_TYPE_RED] = IRedTextCallback,
	[UI_SLIDER_TYPE_GREEN] = IGreenTextCallback,
	[UI_SLIDER_TYPE_BLUE] = IBlueTextCallback
};


Static IColorPickerLine
IColorPickerCreateLine(
	const char* Name,
	const char* ShortName,
	UISliderType SliderType,
	IHandle* ContentContainer
	)
{
	IHandle C = ICreateContainer(
		&((IElement)
		{
			.W = 410.0f,

			.MarginTop = 4.0f,
			.MarginLeft = 4.0f,
			.MarginRight = 4.0f,
			.MarginBottom = 4.0f,

			.BorderTop = 6.0f,
			.BorderLeft = 6.0f,
			.BorderRight = 6.0f,
			.BorderBottom = 6.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0x20FFFFFF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoH = true,

			.WhiteColor = (ARGB){ 0x20FFFFFF },
			.Texture = TEXTURE_RECT
		})
	);

	IHandle T = ICreateText(
		&((IElement)
		{
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		NULL,
		&((IText)
		{
			.Str = Name,
			.FontSize = 18,
			.AlignX = UI_ALIGN_CENTER,

			.Stroke = (ARGB){ 0xFF000000 },
			.Fill = (ARGB){ 0xFFFFFFFF }
		})
	);

	UIElement* S = UIGetElement();
	*S =
	(UIElement)
	{
		.W = 256.0f + SLIDER_SIZE * 2,
		.H = SLIDER_SIZE,

		.MarginRight = 4.0f,

		.Opacity = 0xFF,

		.AlignX = UI_ALIGN_RIGHT,
		.AlignY = UI_ALIGN_MIDDLE,

		.Callback = ColorPickerSliderCallback[SliderType],

		.Type = UI_TYPE_SLIDER,
		.Slider =
		(UISlider)
		{
			.Axis = UI_AXIS_HORIZONTAL,
			.Sections = 256,
			.Value = 0,

			.Type = SliderType
		}
	};

	UIInitialize(S);

	IHandle V = ICreateText(
		&((IElement)
		{
			.W = 30.0f,

			.BorderBottom = 2.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0xFFFFFFFF },

			.AlignX = UI_ALIGN_RIGHT,
			.AlignY = UI_ALIGN_MIDDLE,

			.Callback = ColorPickerTextCallback[SliderType]
		}),
		&((IContainer)
		{
			.AutoH = true
		}),
		&((IText)
		{
			.Placeholder = ShortName,
			.MaxLength = 3,

			.FontSize = 18,

			.Selectable = true,
			.Editable = true,

			.Stroke = (ARGB){ 0xFF000000 },
			.InverseStroke = (ARGB){ 0xFFFFFFFF },
			.Fill = (ARGB){ 0xFFFFFFFF },
			.InverseFill = (ARGB){ 0xFF000000 },
			.Background = (ARGB){ 0xA0000000 },

			.Type = I_TEXT_TYPE_INTEGER,
			.Data =
			(ITextData)
			{
				.Integer =
				(ITextInteger)
				{
					.Min = 0,
					.Max = 255
				}
			}
		})
	);

	IAddElementLast(&T, &C);
	IAddElementLast(&V, &C);
	UIAddElementLast(S, C.Destination);

	IAddElementLast(&C, ContentContainer);

	return (IColorPickerLine) { .Slider = &S->Slider, .Value = V };
}


Static void
IColorPickerCreate(
	void
	)
{
	IHandle ParentContainer = ICreateContainer(
		&((IElement)
		{
			.MarginLeft = 10.0f,

			.BorderTop = 6.0f,
			.BorderLeft = 6.0f,
			.BorderRight = 6.0f,
			.BorderBottom = 6.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0xFF000000 }
		}),
		&((IContainer)
		{
			.AutoW = true,
			.AutoH = true,

			.WhiteColor = (ARGB){ 0xFFFFFFFF },
			.Texture = TEXTURE_RECT128_T
		})
	);

	IHandle ColorContainer = ICreateContainer(
		&((IElement)
		{
			.Opacity = 0xFF
		}),
		&((IContainer)
		{
			.AutoW = true,
			.AutoH = true,

			.Texture = TEXTURE_RECT
		})
	);

	IHandle ContentContainer = ICreateContainer(
		&((IElement)
		{
			.BorderTop = 6.0f,
			.BorderLeft = 6.0f,
			.BorderRight = 6.0f,
			.BorderBottom = 6.0f,

			.MarginTop = 64.0f,
			.MarginLeft = 64.0f,
			.MarginRight = 64.0f,
			.MarginBottom = 64.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0xA0000000 },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_VERTICAL,

			.AutoW = true,
			.AutoH = true,

			.WhiteColor = (ARGB){ 0xA0000000 },
			.Texture = TEXTURE_RECT
		})
	);

	UIElement* ColorPicker = UIGetElement();
	*ColorPicker =
	(UIElement)
	{
		.W = 420.0f,
		.H = 420.0f,

		.Opacity = 0xFF,

		.AlignX = UI_ALIGN_CENTER,
		.AlignY = UI_ALIGN_TOP,

		.Callback = IColorPickerCallback,

		.Type = UI_TYPE_COLOR_PICKER
	};

	UIInitialize(ColorPicker);

	IHandle HexContainer = ICreateContainer(
		&((IElement)
		{
			// .W = 100.0f,

			.MarginTop = 4.0f,
			.MarginLeft = 4.0f,
			.MarginRight = 4.0f,
			.MarginBottom = 4.0f,

			.BorderTop = 6.0f,
			.BorderLeft = 6.0f,
			.BorderRight = 6.0f,
			.BorderBottom = 6.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0x20FFFFFF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_VERTICAL,

			.AutoW = true,
			.AutoH = true,

			.WhiteColor = (ARGB){ 0x20FFFFFF },
			.Texture = TEXTURE_RECT
		})
	);

	IHandle HexText = ICreateText(
		&((IElement)
		{
			.W = 108.0f,

			.BorderBottom = 2.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0xFFFFFFFF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_TOP,

			.Callback = IHexTextCallback
		}),
		&((IContainer)
		{
			.AutoH = true
		}),
		&((IText)
		{
			.Placeholder = "#RRGGBBAA",
			.MaxLength = 9,

			.FontSize = 18,

			.Selectable = true,
			.Editable = true,

			.Stroke = (ARGB){ 0xFF000000 },
			.InverseStroke = (ARGB){ 0xFFFFFFFF },
			.Fill = (ARGB){ 0xFFFFFFFF },
			.InverseFill = (ARGB){ 0xFF000000 },
			.Background = (ARGB){ 0xA0000000 },

			.Type = I_TEXT_TYPE_HEX_COLOR
		})
	);

	IAddElementLast(&ColorContainer, &ParentContainer);
	IAddElementLast(&ContentContainer, &ColorContainer);
	UIAddElementLast(ColorPicker, ContentContainer.Destination);
	IAddElementLast(&HexContainer, &ContentContainer);
	IAddElementLast(&HexText, &HexContainer);

	ColorPickerContainer	= ParentContainer.Source;
	ColorPickerBackground	= ColorContainer.Source;
	ColorPickerElement		= ColorPicker;
	ColorPickerHexText		= HexText;

	ColorPickerBrightness	= IColorPickerCreateLine("Brightness", "V", UI_SLIDER_TYPE_BRIGHTNESS, &ContentContainer);
	ColorPickerOpacity		= IColorPickerCreateLine("Opacity", "A", UI_SLIDER_TYPE_OPACITY, &ContentContainer);
	ColorPickerRed			= IColorPickerCreateLine("Red", "R", UI_SLIDER_TYPE_RED, &ContentContainer);
	ColorPickerGreen		= IColorPickerCreateLine("Green", "G", UI_SLIDER_TYPE_GREEN, &ContentContainer);
	ColorPickerBlue			= IColorPickerCreateLine("Blue", "B", UI_SLIDER_TYPE_BLUE, &ContentContainer);
}


Static void
IColorContainerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	if(Event != UI_EVENT_MOUSE_UP)
	{
		return;
	}

	if(!SameElement)
	{
		return;
	}

	if(UIIsLinked(ColorPickerContainer))
	{
		UIUnlinkElement(ColorPickerContainer);
	}

	if(CurrentColorContainer != Element)
	{
		CurrentColorContainer = Element;
		IColorPickerUpdate(Element->Container.WhiteColor);
		UIAddElementAfter(ColorPickerContainer, Element->Parent);
	}
	else
	{
		CurrentColorContainer = NULL;
	}
}


IHandle
ICreateColorPicker(
	const IElement* Element,
	const IColorPicker* ColorPicker
	)
{
	if(ColorPickerElement == NULL)
	{
		IColorPickerCreate();
	}

	IHandle ParentContainer = ICreateContainer(
		&((IElement)
		{
			.MarginTop = Element->MarginTop,
			.MarginLeft = Element->MarginLeft,
			.MarginRight = Element->MarginRight,
			.MarginBottom = Element->MarginBottom,

			.Opacity = Element->Opacity,

			.AlignX = Element->AlignX,
			.AlignY = Element->AlignY,
			.Position = Element->Position,
			.RelativeAlignX = Element->RelativeAlignX,
			.RelativeAlignY = Element->RelativeAlignY
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoW = true,
			.AutoH = true
		})
	);

	IHandle TransparencyContainer = ICreateContainer(
		&((IElement)
		{
			.BorderTop = 6.0f,
			.BorderLeft = 6.0f,
			.BorderRight = 6.0f,
			.BorderBottom = 6.0f,

			.Opacity = Element->Opacity,
			.BorderColor = (ARGB){ 0xFF000000 },

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		&((IContainer)
		{
			.AutoW = true,
			.AutoH = true,

			.WhiteColor = (ARGB){ 0xFFFFFFFF },
			.Texture = TEXTURE_RECT8_T
		})
	);

	IHandle ColorContainer = ICreateContainer(
		&((IElement)
		{
			.W = 32.0f,
			.H = 32.0f,

			.Opacity = 0xFF,

			.Callback = IColorContainerCallback
		}),
		&((IContainer)
		{
			.Clickable = true,

			.WhiteColor = ColorPicker->Color,
			.Texture = TEXTURE_RECT
		})
	);

	IAddElementLast(&TransparencyContainer, &ParentContainer);
	IAddElementLast(&ColorContainer, &TransparencyContainer);

	return (IHandle){ .Source = ParentContainer.Source, .Destination = NULL };
}


Static void
IDropdownOptionCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_CONTAINER);
	UIContainer* Container = &Element->Container;


	switch(Event)
	{

	case UI_EVENT_MOUSE_IN:
	{
		Container->WhiteColor = (ARGB){ 0xFF717171 };
		break;
	}

	case UI_EVENT_MOUSE_OUT:
	{
		Container->WhiteColor = (ARGB){ 0xFFAAAAAA };
		break;
	}

	case UI_EVENT_MOUSE_UP:
	{
		if(!SameElement)
		{
			break;
		}

		UIElement* Chosen = CurrentDropdown->Container.Head;
		AssertEQ(Chosen->Type, UI_TYPE_TEXT);

		UIText* OwnText = &Container->Head->Text;
		UIText* Text = &Chosen->Text;

		Text->Str = realloc(Text->Str, OwnText->Length + 1);
		AssertNotNull(Text->Str);

		memcpy(Text->Str, OwnText->Str, OwnText->Length + 1);
		Text->Length = OwnText->Length;

		UIUpdateElement(Chosen);

		break;
	}

	default: break;

	}
}


Static IHandle
ICreateDropdownOption(
	const IElement* Element,
	const IDropdown* Dropdown,
	const IDropdownOption* Option
	)
{
	IHandle Container = ICreateContainer(
		&((IElement)
		{
			.W = Element->W,

			.BorderTop = 2.0f,

			.Opacity = 0xFF,
			.BorderColor = (ARGB){ 0xFF000000 },

			.Callback = IDropdownOptionCallback
		}),
		&((IContainer)
		{
			.AutoH = true,

			.WhiteColor = Dropdown->BgColor,
			.Texture = TEXTURE_RECT
		})
	);

	IHandle Text = ICreateText(
		&((IElement)
		{
			.W = Element->W - 4.0f,

			.MarginTop = 4.0f,
			.MarginLeft = 4.0f,
			.MarginBottom = 4.0f,

			.Opacity = 0xFF,

			.AlignY = UI_ALIGN_MIDDLE
		}),
		NULL,
		&((IText)
		{
			.Str = Option->Text,

			.FontSize = Dropdown->FontSize,
			.AlignX = UI_ALIGN_LEFT,

			.Stroke = Dropdown->Stroke,
			.Fill = Dropdown->Fill
		})
	);

	IAddElementLast(&Text, &Container);

	return (IHandle){ .Source = Container.Source, .Destination = NULL };
}


Static void
IDropdownCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_CONTAINER);
	UIContainer* Container = &Element->Container;


	switch(Event)
	{

	case UI_EVENT_MOUSE_IN:
	{
		Container->WhiteColor = (ARGB){ 0xFF717171 };
		break;
	}

	case UI_EVENT_MOUSE_OUT:
	{
		Container->WhiteColor = (ARGB){ 0xFFAAAAAA };
		break;
	}

	case UI_EVENT_MOUSE_UP:
	{
		if(!SameElement)
		{
			break;
		}

		UIElement* Dropdown = CurrentDropdown;
		if(CurrentDropdown)
		{
			UIUnlinkElement(CurrentDropdown->Data);
			CurrentDropdown = NULL;

			Dropdown->Container.Tail->Texture.Texture = TEXTURE_TRIANGLE_DOWN;
		}

		if(Dropdown != Element)
		{
			CurrentDropdown = Element;
			UIAddElementAfter(Element->Data, Element);

			CurrentDropdown->Container.Tail->Texture.Texture = TEXTURE_TRIANGLE_UP;
		}

		break;
	}

	default: break;

	}
}


IHandle
ICreateDropdown(
	const IElement* Element,
	const IDropdown* Dropdown
	)
{
	const IDropdownOption* Option = Dropdown->Options + Dropdown->Chosen;

	IHandle ParentContainer = ICreateContainer(
		&((IElement)
		{
			.MarginTop = Element->MarginTop,
			.MarginLeft = Element->MarginLeft,
			.MarginRight = Element->MarginRight,
			.MarginBottom = Element->MarginBottom,

			.Opacity = Element->Opacity,

			.AlignX = Element->AlignX,
			.AlignY = Element->AlignY,
			.Position = Element->Position,
			.RelativeAlignX = Element->RelativeAlignX,
			.RelativeAlignY = Element->RelativeAlignY,

			.Relative = Element->Relative
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_VERTICAL,

			.AutoW = true,
			.AutoH = true
		})
	);

	IHandle Container = ICreateContainer(
		&((IElement)
		{
			.W = Element->W,

			.Opacity = 0xFF,

			.Callback = IDropdownCallback
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoH = true,

			.WhiteColor = Dropdown->BgColor,
			.Texture = TEXTURE_RECT
		})
	);

	IHandle Text = ICreateText(
		&((IElement)
		{
			.W = Element->W - 32.0f,

			.MarginTop = 4.0f,
			.MarginLeft = 4.0f,
			.MarginBottom = 4.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		NULL,
		&((IText)
		{
			.Str = Option->Text,

			.FontSize = Dropdown->FontSize,
			.AlignX = UI_ALIGN_LEFT,

			.Stroke = Dropdown->Stroke,
			.Fill = Dropdown->Fill
		})
	);

	IHandle Texture = ICreateTexture(
		&((IElement)
		{
			.W = 16.0f,
			.H = 16.0f,

			.MarginRight = 6.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_RIGHT,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		&((ITexture)
		{
			.WhiteColor = (ARGB){ 0xFF000000 },
			.Texture = TEXTURE_TRIANGLE_DOWN
		})
	);

	IAddElementLast(&Text, &Container);
	IAddElementLast(&Texture, &Container);
	IAddElementLast(&Container, &ParentContainer);

	IHandle DummyContainer = ICreateContainer(
		&((IElement)
		{
			.Opacity = 0xFF
		}),
		&((IContainer)
		{
			.AutoW = true,
			.AutoH = true
		})
	);

	for(uint32_t i = 0; i < Dropdown->Count; ++i)
	{
		IHandle Handle = ICreateDropdownOption(Element, Dropdown, Dropdown->Options + i);
		IAddElementLast(&Handle, &DummyContainer);
	}

	Container.Source->Data = DummyContainer.Source;

	return (IHandle){ .Source = ParentContainer.Source, .Destination = NULL };
}


IHandle
ICreateTexture(
	const IElement* Element,
	const ITexture* Texture
	)
{
	UITexture Tex;

	if(Texture->UseExplicit)
	{
		Tex =
		(UITexture)
		{
			.SW = Texture->SW,
			.SH = Texture->SH,
			.OX = Texture->OX,
			.OY = Texture->OY,

			.WhiteColor = Texture->WhiteColor,
			.BlackColor = Texture->BlackColor,
			.Texture = Texture->Texture,
			.Rotation = Texture->Rotation
		};
	}
	else
	{
		Tex =
		(UITexture)
		{
			.SW = 1.0f,
			.SH = 1.0f,
			.OX = 0.5f,
			.OY = 0.5f,

			.WhiteColor = Texture->WhiteColor,
			.BlackColor = Texture->BlackColor,
			.Texture = Texture->Texture,
			.Rotation = Texture->Rotation
		};
	}

	UIElement* T = UIGetElement();
	*T =
	(UIElement)
	{
		.X = Element->X,
		.Y = Element->Y,
		.W = Element->W,
		.H = Element->H,

		.MarginTop = Element->MarginTop,
		.MarginLeft = Element->MarginLeft,
		.MarginRight = Element->MarginRight,
		.MarginBottom = Element->MarginBottom,

		.BorderTop = Element->BorderTop,
		.BorderLeft = Element->BorderLeft,
		.BorderRight = Element->BorderRight,
		.BorderBottom = Element->BorderBottom,

		.Opacity = Element->Opacity,
		.BorderColor = Element->BorderColor,

		.AlignX = Element->AlignX,
		.AlignY = Element->AlignY,
		.Position = Element->Position,
		.RelativeAlignX = Element->RelativeAlignX,
		.RelativeAlignY = Element->RelativeAlignY,

		.InteractiveBorder = Element->InteractiveBorder,

		.Relative = Element->Relative ? Element->Relative->Source : NULL,

		.Callback = Element->Callback,
		.Data = Element->Data,

		.Type = UI_TYPE_TEXTURE,
		.Texture = Tex
	};

	UIInitialize(T);

	return (IHandle){ .Source = T, .Destination = NULL };
}
