#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/client/window.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/client/ui/base.h>
#include <DiepDesktop/shared/alloc_ext.h>

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_clipboard.h>


float Depth;
float DeltaTime;
uint64_t LastDrawAt;

Static UIElement* RootElement;

Pair UIMouse;
UIElement* UIElementUnderMouse;
UIElement* UIClickableUnderMouse;
UIElement* UIScrollableUnderMouse;
UIElement* UISelectedElement;
UIElement* UISelectedSelectableElement;
bool UISameElement;


Static void
UIRootElementOnWindowResize(
	UIElement* Element,
	WindowResizeData* Data
	)
{
	Pair HalfSize =
	(Pair)
	{
		.W = Data->NewSize.W / 2.0f,
		.H = Data->NewSize.H / 2.0f
	};

	if(
		HalfSize.W != Element->Extent.W ||
		HalfSize.H != Element->Extent.H
		)
	{
		Element->Extent.Size = HalfSize;
		UIResizeElement(Element);
	}
}


void
UISetRootElement(
	UIElement* Element
	)
{
	if(RootElement)
	{
		EventUnlisten(&WindowResizeTarget,
			(void*) UIRootElementOnWindowResize, RootElement);
	}

	RootElement = Element;

	if(RootElement)
	{
		EventListen(&WindowResizeTarget,
			(void*) UIRootElementOnWindowResize, RootElement);


		Pair WindowSize = WindowGetSize();

		Element->Extent.Size =
		(Pair)
		{
			.W = WindowSize.W / 2.0f,
			.H = WindowSize.H / 2.0f
		};

		UIResizeElement(Element);
	}
}


UIElement*
UIGetRootElement(
	void
	)
{
	return RootElement;
}


UIElement*
UIAllocElement(
	UIElementInfo Info
	)
{
	UIElement* Element = AllocMalloc(sizeof(UIElement));
	AssertNotNull(Element);

	*Element =
	(UIElement)
	{
		.Extent = Info.Extent,
		.Margin = Info.Margin,

		.BorderRadius = Info.BorderRadius,
		.BorderColor = Info.BorderColor,
		.Opacity = Info.Opacity,

		.AlignX = Info.AlignX,
		.AlignY = Info.AlignY,
		.Position = Info.Position,

		.Relative = Info.Relative,
		.RelativeAlignX = Info.RelativeAlignX,
		.RelativeAlignY = Info.RelativeAlignY,

		.Clickable = Info.Clickable,
		.ClickPassthrough = Info.ClickPassthrough,
		.Selectable = Info.Selectable,
		.Scrollable = Info.Scrollable,
		.ScrollPassthrough = Info.ScrollPassthrough,
		.InteractiveBorder = Info.InteractiveBorder,
		.Inline = Info.Inline,
		.Block = Info.Block,
		.ClipToBorder = Info.ClipToBorder
	};

	UIUpdateWidth(Element);
	UIUpdateHeight(Element);

	return Element;
}


Static void
UIFreeElementCallback(
	UIElement* Element,
	void* Data
	)
{
	EventNotify(&Element->FreeTarget, &((UIFreeData){0}));

	EventFree(&Element->MouseDownTarget);
	EventFree(&Element->MouseUpTarget);
	EventFree(&Element->MouseMoveTarget);
	EventFree(&Element->MouseInTarget);
	EventFree(&Element->MouseOutTarget);
	EventFree(&Element->MouseScrollTarget);

	EventFree(&Element->ResizeTarget);
	EventFree(&Element->ChangeTarget);
	EventFree(&Element->SubmitTarget);
	EventFree(&Element->FreeTarget);

	EventFree(&Element->TextSelectAllTarget);
	EventFree(&Element->TextMoveTarget);
	EventFree(&Element->TextCopyTarget);
	EventFree(&Element->TextPasteTarget);
	EventFree(&Element->TextEscapeTarget);
	EventFree(&Element->TextEnterTarget);
	EventFree(&Element->TextDeleteTarget);
	EventFree(&Element->TextUndoTarget);
	EventFree(&Element->TextRedoTarget);

	AllocFree(sizeof(UIElement), Element);
}


void
UIFreeElement(
	UIElement* Element
	)
{
	Element->VirtualTable->BubbleDown(Element, UIFreeElementCallback, NULL);
}


extern void
UIInheritPosition(
	UIElement* Element,
	RectExtent Clip
	);


void
UIDrawElement(
	UIElement* Element,
	RectExtent Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIInheritPosition(Element, Clip);

	Element->VirtualTable->PreClip(Element, Scrollable);


	RectExtent Bounds =
	(RectExtent)
	{
		.MinX = Element->EndExtent.X - Element->Extent.W,
		.MinY = Element->EndExtent.Y - Element->Extent.H,
		.MaxX = Element->EndExtent.X + Element->Extent.W,
		.MaxY = Element->EndExtent.Y + Element->Extent.H
	};

	float Diameter = Element->BorderRadius * 2.0f;


	UIClipExplicit(
		Bounds.MinX - Diameter,
		Bounds.MaxX + Diameter,
		Bounds.MinY - Diameter,
		Bounds.MaxY + Diameter,
		Border
		);


	UIClipExplicit(
		Bounds.MinX,
		Bounds.MaxX,
		Bounds.MinY,
		Bounds.MaxY,
		Content
		);


	RectExtent DrawClip;
	bool DrawPass;

	if(Element->ClipToBorder)
	{
		DrawClip = BorderPostClip;
		DrawPass = BorderClipPass;
	}
	else
	{
		DrawClip = ContentPostClip;
		DrawPass = ContentClipPass;
	}


	Element->VirtualTable->PostClip(
		Element,
		DrawClip,
		Opacity,
		Scrollable
		);


	if((Element->InteractiveBorder && BorderClipPass) || ContentClipPass)
	{
		RectExtent MouseClip;

		if(Element->InteractiveBorder)
		{
			MouseClip = BorderPostClip;
		}
		else
		{
			MouseClip = ContentPostClip;
		}

		if(
			UIMouse.X >= MouseClip.MinX &&
			UIMouse.Y >= MouseClip.MinY &&
			UIMouse.X <= MouseClip.MaxX &&
			UIMouse.Y <= MouseClip.MaxY
			)
		{
			bool OldHovered = Element->Hovered;
			Element->Hovered = Element->VirtualTable->MouseOver(Element);

			if(OldHovered != Element->Hovered)
			{
				if(Element->Hovered)
				{
					EventNotify(&Element->MouseInTarget, &((UIMouseInData){0}));
				}
				else
				{
					EventNotify(&Element->MouseOutTarget, &((UIMouseOutData){0}));
				}
			}

			if(Element->Hovered)
			{
				UIElementUnderMouse = Element;

				if(Element->Clickable && !Element->ClickPassthrough)
				{
					UIClickableUnderMouse = Element;
				}

				if(Element->Scrollable && !Element->ScrollPassthrough)
				{
					UIScrollableUnderMouse = Element;
				}
			}
			else
			{
				if(Element->Hovered)
				{
					EventNotify(&Element->MouseOutTarget, &((UIMouseOutData){0}));
				}
			}
		}
	}

	if(BorderClipPass)
	{
		UIDrawBorder(
			Element,
			BorderPostClip,
			Opacity
			);
	}

	if(DrawPass)
	{
		Element->VirtualTable->DrawDetail(
			Element,
			DrawClip,
			Opacity,
			Scrollable
			);
	}

	Element->VirtualTable->DrawChildren(
		Element,
		DrawClip,
		Opacity,
		Scrollable
		);
}


Static void
UIDrawCallback(
	void* _,
	void* __
	)
{
	if(LastDrawAt == 0)
	{
		LastDrawAt = WindowGetTime();
	}
	else
	{
		uint64_t Now = WindowGetTime();
		uint64_t Delta = Now - LastDrawAt;
		LastDrawAt = Now;
		DeltaTime = (double) Delta / 16666666.6;
	}

	UIElement* Root = UIGetRootElement();
	if(!Root)
	{
		return;
	}

	Depth = DRAW_DEPTH_LEAP;

	Pair Mouse = WindowGetMousePosition();

	UIMouse =
	(Pair)
	{
		.X = CLAMP_SYM(Mouse.X, Root->Extent.W),
		.Y = CLAMP_SYM(Mouse.Y, Root->Extent.H)
	};

	UIElementUnderMouse = NULL;
	UIClickableUnderMouse = NULL;
	UIScrollableUnderMouse = NULL;

	UIDrawElement(
		Root,
		(RectExtent)
		{
			.Min = {{ -Root->Extent.W, -Root->Extent.H }},
			.Max = {{  Root->Extent.W,  Root->Extent.H }}
		},
		0xFF,
		NULL
		);

	AssertNotNull(UIElementUnderMouse);

	if(UIElementUnderMouse->Selectable)
	{
		WindowSetCursor(WINDOW_CURSOR_TYPING);
	}
	else if(UIElementUnderMouse->Clickable)
	{
		WindowSetCursor(WINDOW_CURSOR_POINTING);
	}
	else
	{
		WindowSetCursor(WINDOW_CURSOR_DEFAULT);
	}
}


Static void
UIMouseDownCallback(
	void* _,
	WindowMouseDownData* Data
	)
{
	if(Data->Button != MOUSE_BUTTON_LEFT)
	{
		return;
	}

	UISelectedElement = UIClickableUnderMouse;

	if(UISelectedElement)
	{
		UISelectedElement->Held = true;

		UIMouseDownData EventData =
		(UIMouseDownData)
		{
			.Button = Data->Button,
			.Position = Data->Position,
			.Clicks = Data->Clicks
		};

		EventNotify(&UISelectedElement->MouseDownTarget, &EventData);
	}
}


Static void
UIMouseUpCallback(
	void* _,
	WindowMouseUpData* Data
	)
{
	if(Data->Button != MOUSE_BUTTON_LEFT)
	{
		return;
	}

	if(UISelectedElement)
	{
		UISameElement = UIClickableUnderMouse == UISelectedElement;

		UIMouseUpData EventData =
		(UIMouseUpData)
		{
			.Button = Data->Button,
			.Position = Data->Position
		};

		EventNotify(&UISelectedElement->MouseUpTarget, &EventData);

		UISelectedElement = NULL;
	}
}


void // TODO convert to constructor
UIInit(
	void
	)
{
	EventListen(&WindowDrawTarget, (void*) UIDrawCallback, NULL);
	EventListen(&WindowMouseDownTarget, (void*) UIMouseDownCallback, NULL);
	EventListen(&WindowMouseUpTarget, (void*) UIMouseUpCallback, NULL);
}


void
UIFree(
	void
	)
{
	EventUnlisten(&WindowDrawTarget, UIDrawCallback, NULL);

	UIElement* Root = UIGetRootElement();
	if(Root)
	{
		UISetRootElement(NULL);
		UIFreeElement(Root);
	}
}


void
UIUpdateWidth( // TODO apply modifiers
	UIElement* Element
	)
{
	Element->DrawMargin.Left = Element->Margin.Left + Element->BorderRadius;
	Element->DrawMargin.Right = Element->Margin.Right + Element->BorderRadius;

	Element->EndExtent.W =
		Element->DrawMargin.Left + Element->Extent.W + Element->DrawMargin.Right;
}


Static float
UIEndWidthToWidth(
	UIElement* Element,
	float OldEndWidth
	)
{
	return OldEndWidth - Element->DrawMargin.Left - Element->DrawMargin.Right;
}


void
UIUpdateHeight(
	UIElement* Element
	)
{
	Element->DrawMargin.Top = Element->Margin.Top + Element->BorderRadius;
	Element->DrawMargin.Bottom = Element->Margin.Bottom + Element->BorderRadius;

	Element->EndExtent.H =
		Element->DrawMargin.Top + Element->Extent.H + Element->DrawMargin.Bottom;
}


Static float
UIEndHeightToHeight(
	UIElement* Element,
	float OldEndHeight
	)
{
	return OldEndHeight - Element->DrawMargin.Top - Element->DrawMargin.Bottom;
}


void
UIResizeElement(
	UIElement* Element
	)
{
	Pair OldSize = Element->EndExtent.Size;

	UIUpdateWidth(Element);
	UIUpdateHeight(Element);

	if(
		Element->EndExtent.W == OldSize.W &&
		Element->EndExtent.H == OldSize.H
		)
	{
		return;
	}

	UIResizeData EventData =
	(UIResizeData)
	{
		.OldSize =
		(Pair)
		{
			.W = UIEndWidthToWidth(Element, OldSize.W),
			.H = UIEndHeightToHeight(Element, OldSize.H)
		},
		.NewSize = Element->Extent.Size
	};

	EventNotify(&Element->ResizeTarget, &EventData);

	if(Element->Parent)
	{
		Element->VirtualTable->PropagateSize(
			Element->Parent,
			Element,
			(Pair)
			{
				.W = Element->EndExtent.W - OldSize.W,
				.H = Element->EndExtent.H - OldSize.H
			}
			);
	}
}


void
UIDrawBorder(
	UIElement* Element,
	RectExtent Clip,
	uint8_t Opacity
	)
{
	if(!Element->BorderRadius)
	{
		return;
	}

	RectExtent Border =
	(RectExtent)
	{
		.Min =
		(Pair)
		{
			.X = Element->EndExtent.X - Element->Extent.W - Element->BorderRadius,
			.Y = Element->EndExtent.Y - Element->Extent.H - Element->BorderRadius
		},
		.Max =
		(Pair)
		{
			.X = Element->EndExtent.X + Element->Extent.W + Element->BorderRadius,
			.Y = Element->EndExtent.Y + Element->Extent.H + Element->BorderRadius
		}
	};

	float Diameter = Element->BorderRadius * 2.0f;

	ARGB Color = RGBmulA(Element->BorderColor, Opacity);

	UIClipTexture(Element->EndExtent.X, Border.MinY,
		Element->Extent.W, Element->BorderRadius,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_RECT
		);

	UIClipTextureExplicit(Border.MaxX, Border.MinY, Diameter,
		Diameter, 0.5f, 0.5f, 0.75f, 0.25f,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_CIRCLE
		);

	UIClipTexture(Border.MaxX, Element->EndExtent.Y,
		Element->BorderRadius, Element->Extent.H,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_RECT
		);

	UIClipTextureExplicit(Border.MaxX, Border.MaxY, Diameter,
		Diameter, 0.5f, 0.5f, 0.75f, 0.75f,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_CIRCLE
		);

	UIClipTexture(Element->EndExtent.X, Border.MaxY,
		Element->Extent.W, Element->BorderRadius,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_RECT
		);

	UIClipTextureExplicit(Border.MinX, Border.MaxY, Diameter,
		Diameter, 0.5f, 0.5f, 0.25f, 0.75f,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_CIRCLE
		);

	UIClipTexture(Border.MinX, Element->EndExtent.Y,
		Element->BorderRadius, Element->Extent.H,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_RECT
		);

	UIClipTextureExplicit(Border.MinX, Border.MinY, Diameter,
		Diameter, 0.5f, 0.5f, 0.25f, 0.25f,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_CIRCLE
		);
}


bool
UIMouseOverElement(
	UIElement* Element
	)
{
	/* This function assumes that the mouse is over the bounding box of the
	 * element, as is the case during drawing (and this function is only used
	 * during drawing). Due to that, the first if statement here is optimized.
	 */

	{ /* Bound check */
		float Diameter = Element->BorderRadius * 2.0f;
		float MinX = Element->EndExtent.X - Element->Extent.W - Diameter;
		float MinY = Element->EndExtent.Y - Element->Extent.H - Diameter;
		float MaxX = Element->EndExtent.X + Element->Extent.W + Diameter;
		float MaxY = Element->EndExtent.Y + Element->Extent.H + Diameter;
		AssertFalse((UIMouse.X < MinX || UIMouse.Y < MinY || UIMouse.X > MaxX || UIMouse.Y > MaxY));
	}

	float MinX = Element->EndExtent.X - Element->Extent.W;
	float MinY = Element->EndExtent.Y - Element->Extent.H;
	float MaxX = Element->EndExtent.X + Element->Extent.W;
	float MaxY = Element->EndExtent.Y + Element->Extent.H;

	if(
		Element->BorderRadius == 0.0f ||
		(UIMouse.X >= MinX && UIMouse.X <= MaxX) ||
		(UIMouse.Y >= MinY && UIMouse.Y <= MaxY)
		)
	{
		return true;
	}

	/* At this point, only need to check corners. */

	float DiffX = UIMouse.X;
	if(UIMouse.X < MinX)
	{
		DiffX -= MinX;
	}
	else
	{
		DiffX -= MaxX;
	}

	float DiffY = UIMouse.Y;
	if(UIMouse.Y < MinY)
	{
		DiffY -= MinY;
	}
	else
	{
		DiffY -= MaxY;
	}

	return DiffX * DiffX + DiffY * DiffY <=
		Element->BorderRadius * Element->BorderRadius * 4.0f;
}
