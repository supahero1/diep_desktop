#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>
#include <DiepDesktop/client/ui/container.h>


Static void
UIContainerBubbleDown(
	UIElement* Element,
	UIBubbleCallback Callback,
	void* Data
	)
{
	UIContainer* Container = Element->TypeData;
	UIElement* Child = Container->Head;

	while(Child)
	{
		/* Child might be free'd during the bubble */
		UIElement* Next = Child->Next;
		Child->VirtualTable->BubbleDown(Child, Callback, Data);
		Child = Next;
	}

	Callback(Element, Data);
}


Static void
UIContainerPropagateSize(
	UIElement* Element,
	UIElement* Child,
	Pair Delta
	)
{
	AssertTrue(UIElementIsContainer(Element));
	UIContainer* Container = Element->TypeData;
	AssertNotNull(Container->Head);
	AssertNotNull(Container->Tail);

	bool Pass = false;

	Pair OldSize = Element->Extent.Size;

	if(Container->AutoW)
	{
		if(Container->Axis == UI_AXIS_HORIZONTAL)
		{
			if(!Child->Inline)
			{
				Container->ContentW += Delta.W;
			}
			else /* Inline */
			{
				if(Delta.W >= 0.0f)
				{
					Container->MaxInlineW = MAX(Container->MaxInlineW, Child->EndExtent.W);
				}
				else
				{
					float TotalW = 0.0f;
					UIElement* Child = Container->Head;

					while(Child)
					{
						if(Child->Inline)
						{
							TotalW = MAX(TotalW, Child->EndExtent.W);
						}
						Child = Child->Next;
					}

					Container->MaxInlineW = TotalW;
				}
			}
		}
		else /* UI_AXIS_VERTICAL */
		{
			if(!Child->Inline)
			{
				if(Delta.W >= 0.0f)
				{
					Container->ContentW = MAX(Container->ContentW, Child->EndExtent.W);
				}
				else
				{
					float TotalW = 0.0f;
					UIElement* Child = Container->Head;

					while(Child)
					{
						if(!Child->Inline)
						{
							TotalW = MAX(TotalW, Child->EndExtent.W);
						}
						Child = Child->Next;
					}

					Container->ContentW = TotalW;
				}
			}
			else /* Inline */
			{
				if(Delta.W >= 0.0f)
				{
					Container->MaxInlineW = MAX(Container->MaxInlineW, Child->EndExtent.W);
				}
				else
				{
					float TotalW = 0.0f;
					UIElement* Child = Container->Head;

					while(Child)
					{
						if(Child->Inline)
						{
							TotalW = MAX(TotalW, Child->EndExtent.W);
						}
						Child = Child->Next;
					}

					Container->MaxInlineW = TotalW;
				}
			}
		}

		Element->Extent.W = MAX(Container->ContentW, Container->MaxInlineW);

		Delta.W = Element->Extent.W - OldSize.W;
		if(Delta.W)
		{
			Pass = true;
			UIUpdateWidth(Element);
		}
	}
	else
	{
		Delta.W = 0.0f;
	}


	if(Container->AutoH)
	{
		/* Analogy to x above */

		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			if(!Child->Inline)
			{
				Container->ContentH += Delta.H;
			}
			else /* Inline */
			{
				if(Delta.H >= 0.0f)
				{
					Container->MaxInlineH = MAX(Container->MaxInlineH, Child->EndExtent.H);
				}
				else
				{
					float TotalH = 0.0f;
					UIElement* Child = Container->Head;

					while(Child)
					{
						if(Child->Inline)
						{
							TotalH = MAX(TotalH, Child->EndExtent.H);
						}
						Child = Child->Next;
					}

					Container->MaxInlineH = TotalH;
				}
			}
		}
		else /* UI_AXIS_HORIZONTAL */
		{
			if(!Child->Inline)
			{
				if(Delta.H >= 0.0f)
				{
					Container->ContentH = MAX(Container->ContentH, Child->EndExtent.H);
				}
				else
				{
					float TotalH = 0.0f;
					UIElement* Child = Container->Head;

					while(Child)
					{
						if(!Child->Inline)
						{
							TotalH = MAX(TotalH, Child->EndExtent.H);
						}
						Child = Child->Next;
					}

					Container->ContentH = TotalH;
				}
			}
			else /* Inline */
			{
				if(Delta.H >= 0.0f)
				{
					Container->MaxInlineH = MAX(Container->MaxInlineH, Child->EndExtent.H);
				}
				else
				{
					float TotalH = 0.0f;
					UIElement* Child = Container->Head;

					while(Child)
					{
						if(Child->Inline)
						{
							TotalH = MAX(TotalH, Child->EndExtent.H);
						}
						Child = Child->Next;
					}

					Container->MaxInlineH = TotalH;
				}
			}
		}

		Element->Extent.H = MAX(Container->ContentH, Container->MaxInlineH);

		Delta.H = Element->Extent.H - OldSize.H;
		if(Delta.H)
		{
			Pass = true;
			UIUpdateHeight(Element);
		}
	}
	else
	{
		Delta.H = 0.0f;
	}


	if(!Pass)
	{
		return;
	}

	UIResizeData EventData =
	(UIResizeData)
	{
		.OldSize = OldSize,
		.NewSize = Element->Extent.Size
	};

	EventNotify(&Element->ResizeTarget, &EventData);


	if(Element->Parent)
	{
		Element->VirtualTable->PropagateSize(
			Element->Parent,
			Element,
			Delta
			);
	}
}


Static void
UIContainerPreClip(
	UIElement* Element,
	UIElement* Scrollable
	)
{
	UIContainer* Container = Element->TypeData;

	if(Element->Scrollable)
	{
		UIElement* Parent = Element->Parent;

		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			float ClampedGoalOffsetY = CLAMP(Container->GoalOffsetY, Parent->Extent.H - Element->Extent.H, 0);
			Container->GoalOffsetY = glm_lerpc(Container->GoalOffsetY, ClampedGoalOffsetY, 0.5f * DeltaTime);
			Container->OffsetY     = glm_lerpc(Container->OffsetY, Container->GoalOffsetY, 0.2f * DeltaTime);
		}
		else
		{
			float ClampedGoalOffsetX = CLAMP(Container->GoalOffsetX, Parent->Extent.W - Element->Extent.W, 0);
			Container->GoalOffsetX = glm_lerpc(Container->GoalOffsetX, ClampedGoalOffsetX, 0.5f * DeltaTime);
			Container->OffsetX     = glm_lerpc(Container->OffsetX, Container->GoalOffsetX, 0.2f * DeltaTime);
		}

		Element->EndExtent.X += Container->OffsetX;
		Element->EndExtent.Y += Container->OffsetY;
	}
}


Static void
UIContainerPostClip(
	UIElement* Element,
	RectExtent Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{

}


Static bool
UIContainerMouseOver(
	UIElement* Element
	)
{
	return UIMouseOverElement(Element);
}


Static void
UIContainerDrawDetail(
	UIElement* Element,
	RectExtent Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIContainer* Container = Element->TypeData;

	UIClipTexture(Element->EndExtent.X, Element->EndExtent.Y,
		Element->Extent.W, Element->Extent.H,
		.WhiteColor = RGBmulA(Container->WhiteColor, Opacity),
		.WhiteDepth = Depth,
		.BlackColor = RGBmulA(Container->BlackColor, Opacity),
		.BlackDepth = Depth,
		.Texture = Container->Texture
		);
}


void
UIInheritPosition(
	UIElement* Element,
	RectExtent Clip
	)
{
	if(!Element->Parent)
	{
		Element->EndExtent.Pos = Element->Extent.Pos;
		return;
	}

	UIElement* Parent = Element->Parent;
	AssertTrue(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;


	switch(Element->Position)
	{

	case UI_POSITION_INHERIT:
	{
		float OffsetMinX;
		float OffsetMaxX;
		float OffsetMinY;
		float OffsetMaxY;

		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			OffsetMinX = 0.0f;
			OffsetMaxX = 0.0f;
			OffsetMinY = Container->OffsetMin;
			OffsetMaxY = Container->OffsetMax;
		}
		else
		{
			OffsetMinX = Container->OffsetMin;
			OffsetMaxX = Container->OffsetMax;
			OffsetMinY = 0.0f;
			OffsetMaxY = 0.0f;
		}


		switch(Element->AlignX)
		{

		case UI_ALIGN_LEFT:
		{
			Element->EndExtent.X = Parent->EndExtent.X - Parent->Extent.W
				+ Element->Extent.W + Element->DrawMargin.Left * 2.0f + OffsetMinX;
			OffsetMinX += Element->EndExtent.W * 2.0f;
			break;
		}

		case UI_ALIGN_CENTER:
		{
			Element->EndExtent.X = Parent->EndExtent.X;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Element->EndExtent.X = Parent->EndExtent.X + Parent->Extent.W
				-(Element->Extent.W + Element->DrawMargin.Right * 2.0f + OffsetMaxX);
			OffsetMaxX += Element->EndExtent.W * 2.0f;
			break;
		}

		}


		switch(Element->AlignY)
		{

		case UI_ALIGN_TOP:
		{
			Element->EndExtent.Y = Parent->EndExtent.Y - Parent->Extent.H
				+ Element->Extent.H + Element->DrawMargin.Top * 2.0f + OffsetMinY;
			OffsetMinY += Element->EndExtent.H * 2.0f;
			break;
		}

		case UI_ALIGN_MIDDLE:
		{
			Element->EndExtent.Y = Parent->EndExtent.Y;
			break;
		}

		case UI_ALIGN_BOTTOM:
		{
			Element->EndExtent.Y = Parent->EndExtent.Y + Parent->Extent.H
				-(Element->Extent.H + Element->DrawMargin.Bottom * 2.0f + OffsetMaxY);
			OffsetMaxY += Element->EndExtent.H * 2.0f;
			break;
		}

		}


		if(!Element->Inline)
		{
			if(Container->Axis == UI_AXIS_VERTICAL)
			{
				Container->OffsetMin = OffsetMinY;
				Container->OffsetMax = OffsetMaxY;
			}
			else
			{
				Container->OffsetMin = OffsetMinX;
				Container->OffsetMax = OffsetMaxX;
			}
		}

		break;
	}

	case UI_POSITION_ABSOLUTE:
	{
		Element->EndExtent.X = Parent->EndExtent.X + Element->Extent.X;
		Element->EndExtent.Y = Parent->EndExtent.Y + Element->Extent.Y;
		break;
	}

	case UI_POSITION_RELATIVE:
	{
		AssertNotNull(Element->Relative);
		UIElement* Relative = Element->Relative;


		Element->EndExtent.X = Relative->EndExtent.X;

		switch(Element->AlignX)
		{

		case UI_ALIGN_LEFT:
		{
			Element->EndExtent.X -= Element->Extent.W + Element->DrawMargin.Right * 2.0f;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Element->EndExtent.X += Element->Extent.W + Element->DrawMargin.Left * 2.0f;
			break;
		}

		default: break;

		}

		switch(Element->RelativeAlignX)
		{

		case UI_ALIGN_LEFT:
		{
			Element->EndExtent.X -= Relative->Extent.W;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Element->EndExtent.X += Relative->Extent.W;
			break;
		}

		default: break;

		}


		Element->EndExtent.Y = Relative->EndExtent.Y;

		switch(Element->AlignY)
		{

		case UI_ALIGN_TOP:
		{
			Element->EndExtent.Y -= Element->Extent.H + Element->DrawMargin.Bottom * 2.0f;
			break;
		}

		case UI_ALIGN_BOTTOM:
		{
			Element->EndExtent.Y += Element->Extent.H + Element->DrawMargin.Top * 2.0f;
			break;
		}

		default: break;

		}

		switch(Element->RelativeAlignY)
		{

		case UI_ALIGN_TOP:
		{
			Element->EndExtent.Y -= Relative->Extent.H;
			break;
		}

		case UI_ALIGN_BOTTOM:
		{
			Element->EndExtent.Y += Relative->Extent.H;
			break;
		}

		default: break;

		}


		break;
	}

	default: AssertUnreachable();

	}
}


Static void
UIContainerDrawChildren(
	UIElement* Element,
	RectExtent Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIContainer* Container = Element->TypeData;

	Container->OffsetMin = 0.0f;
	Container->OffsetMax = 0.0f;

	if(Element->Scrollable)
	{
		Scrollable = Element;
	}

	UIElement* Child = Container->Head;
	while(Child)
	{
		uint8_t ChildOpacity = AmulA(Opacity, Child->Opacity);
		if(ChildOpacity)
		{
			Depth += DRAW_DEPTH_LEAP;

			UIDrawElement(
				Child,
				Clip,
				ChildOpacity,
				Scrollable
				);
		}

		Child = Child->Next;
	}
}


Static UIVirtualTable ContainerVirtualTable =
{
	.BubbleDown = UIContainerBubbleDown,
	.PropagateSize = UIContainerPropagateSize,
	.PreClip = UIContainerPreClip,
	.PostClip = UIContainerPostClip,
	.MouseOver = UIContainerMouseOver,
	.DrawDetail = UIContainerDrawDetail,
	.DrawChildren = UIContainerDrawChildren,
};


bool
UIElementIsContainer(
	UIElement* Element
	)
{
	return Element->VirtualTable == &ContainerVirtualTable;
}


Static UIContainer*
UIAllocContainerData(
	void
	)
{
	void* Container = AllocMalloc(sizeof(UIContainer));
	AssertNotNull(Container);

	return Container;
}


Static void
UIFreeContainerData(
	UIContainer* Container,
	void* _
	)
{
	AssertNotNull(Container);
	AllocFree(sizeof(UIContainer), Container);
}


UIElement*
UIAllocContainer(
	UIElementInfo ElementInfo,
	UIContainerInfo Info
	)
{
	UIElement* Element = UIAllocElement(ElementInfo);
	AssertNotNull(Element);

	UIContainer* Container = UIAllocContainerData();
	*Container =
	(UIContainer)
	{
		.Axis = Info.Axis,

		.AutoW = Info.AutoW,
		.AutoH = Info.AutoH,

		.WhiteColor = Info.WhiteColor,
		.BlackColor = Info.BlackColor,
		.Texture = Info.Texture
	};

	Element->VirtualTable = &ContainerVirtualTable;
	Element->TypeData = Container;

	EventListen(&Element->FreeTarget, (void*) UIFreeContainerData, Container);

	return Element;
}


Static void
UIPropagateInsertion(
	UIElement* Element,
	UIElement* Parent
	)
{
	Parent->VirtualTable->PropagateSize(
		Parent,
		Element,
		Element->EndExtent.Size
		);
}


void
UIInsertFirst(
	UIElement* Element,
	UIElement* Parent
	)
{
	AssertTrue(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;

	Element->Parent = Parent;
	Element->Prev = NULL;
	Element->Next = Container->Head;

	if(Container->Head)
	{
		Container->Head->Prev = Element;
	}
	else
	{
		Container->Tail = Element;
	}

	Container->Head = Element;

	UIPropagateInsertion(Element, Parent);
}


void
UIInsertLast(
	UIElement* Element,
	UIElement* Parent
	)
{
	AssertTrue(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;

	Element->Parent = Parent;
	Element->Prev = Container->Tail;
	Element->Next = NULL;

	if(Container->Tail)
	{
		Container->Tail->Next = Element;
	}
	else
	{
		Container->Head = Element;
	}

	Container->Tail = Element;

	UIPropagateInsertion(Element, Parent);
}


void
UIInsertBefore(
	UIElement* Element,
	UIElement* Before
	)
{
	UIElement* Parent = Before->Parent;
	AssertTrue(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;

	Element->Parent = Parent;
	Element->Prev = Before->Prev;
	Element->Next = Before;

	if(Before->Prev)
	{
		Before->Prev->Next = Element;
	}
	else
	{
		Container->Head = Element;
	}

	Before->Prev = Element;

	UIPropagateInsertion(Element, Element->Parent);
}


void
UIInsertAfter(
	UIElement* Element,
	UIElement* After
	)
{
	UIElement* Parent = After->Parent;
	AssertTrue(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;

	Element->Parent = Parent;
	Element->Prev = After;
	Element->Next = After->Next;

	if(After->Next)
	{
		After->Next->Prev = Element;
	}
	else
	{
		Container->Tail = Element;
	}

	After->Next = Element;

	UIPropagateInsertion(Element, Element->Parent);
}


bool
UIIsLinked(
	UIElement* Element
	)
{
	return !!Element->Parent;
}


void
UIUnlink(
	UIElement* Element
	)
{
	if(Element == UIGetRootElement())
	{
		UISetRootElement(NULL);
	}

	UIElement* Parent = Element->Parent;
	AssertTrue(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;

	if(Element->Prev)
	{
		Element->Prev->Next = Element->Next;
	}
	else
	{
		Container->Head = Element->Next;
	}

	if(Element->Next)
	{
		Element->Next->Prev = Element->Prev;
	}
	else
	{
		Container->Tail = Element->Prev;
	}

	Element->Parent = NULL;
	Element->Prev = NULL;
	Element->Next = NULL;

	Parent->VirtualTable->PropagateSize(
		Parent,
		Element,
		(Pair)
		{
			.W = -Element->EndExtent.W,
			.H = -Element->EndExtent.H
		}
		);
}
