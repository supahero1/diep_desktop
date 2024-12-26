#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>
#include <DiepDesktop/client/ui/checkbox.h>
#include <DiepDesktop/client/ui/container.h>


Static void
UICheckboxBubbleDown(
	UIElement* Element,
	UIBubbleCallback Callback,
	void* Data
	)
{
}


Static void
UICheckboxPropagateSize(
	UIElement* Element,
	UIElement* Child,
	Pair Delta
	)
{
}


Static void
UICheckboxPreClip(
	UIElement* Element,
	UIElement* Scrollable
	)
{

}


Static void
UICheckboxPostClip(
	UIElement* Element,
	RectExtent Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UICheckbox* Checkbox = Element->TypeData;

	Element->BorderColor = Checkbox->Checked ? Checkbox->CheckYesBackground : Checkbox->CheckNoBackground;
}


Static bool
UICheckboxMouseOver(
	UIElement* Element
	)
{
	return UIMouseOverElement(Element);
}


Static void
UICheckboxDrawDetail(
	UIElement* Element,
	RectExtent Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	AssertEQ(Element->Extent.W, Element->Extent.H);

	UICheckbox* Checkbox = Element->TypeData;

	UIClipTexture(Element->EndExtent.X, Element->EndExtent.Y, Element->Extent.W, Element->Extent.H,
		.WhiteColor = RGBmulA(Checkbox->Checked ? Checkbox->CheckYesBackground : Checkbox->CheckNoBackground, Opacity),
		.WhiteDepth = Depth,
		.Texture = TEXTURE_RECT
		);

	float Size = (Element->Extent.W * 0.66666f + Element->BorderRadius * 1.33333f) * 1.1f;

	UIClipTexture(Element->EndExtent.X, Element->EndExtent.Y, Size, Size,
		.WhiteColor = RGBmulA(Checkbox->Checked ? Checkbox->CheckYes : Checkbox->CheckNo, Opacity),
		.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
		.Texture = Checkbox->Checked ? TEXTURE_CHECK_YES : TEXTURE_CHECK_NO
		);
}


Static void
UICheckboxDrawChildren(
	UIElement* Element,
	RectExtent Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
}


Static UIVirtualTable CheckboxVirtualTable =
{
	.BubbleDown = UICheckboxBubbleDown,
	.PropagateSize = UICheckboxPropagateSize,
	.PreClip = UICheckboxPreClip,
	.PostClip = UICheckboxPostClip,
	.MouseOver = UICheckboxMouseOver,
	.DrawDetail = UICheckboxDrawDetail,
	.DrawChildren = UICheckboxDrawChildren
};


bool
UIElementIsCheckbox(
	UIElement* Element
	)
{
	return Element->VirtualTable == &CheckboxVirtualTable;
}


Static UICheckbox*
UIAllocCheckboxData(
	void
	)
{
	void* Checkbox = AllocMalloc(sizeof(UICheckbox));
	AssertNotNull(Checkbox);

	return Checkbox;
}


Static void
UIFreeCheckboxData(
	UICheckbox* Checkbox,
	void* _
	)
{
	AssertNotNull(Checkbox);
	AllocFree(sizeof(UICheckbox), Checkbox);
}


Static void
UICheckboxMouseUpCallback(
	UICheckbox* Checkbox,
	UIMouseUpData* Data
	)
{
	if(!UISameElement)
	{
		return;
	}

	Checkbox->Checked = !Checkbox->Checked;
}


UIElement*
UIAllocCheckbox(
	UIElementInfo ElementInfo,
	UICheckboxInfo Info
	)
{
	ElementInfo.Clickable = true;
	ElementInfo.InteractiveBorder = true;
	ElementInfo.ClipToBorder = true;

	UIElement* Element = UIAllocElement(ElementInfo);
	AssertNotNull(Element);

	UICheckbox* Checkbox = UIAllocCheckboxData();
	*Checkbox =
	(UICheckbox)
	{
		.Checked = Info.Checked,

		.CheckYes = Info.CheckYes,
		.CheckYesBackground = Info.CheckYesBackground,

		.CheckNo = Info.CheckNo,
		.CheckNoBackground = Info.CheckNoBackground
	};

	Element->VirtualTable = &CheckboxVirtualTable;
	Element->TypeData = Checkbox;

	EventListen(&Element->MouseUpTarget, (void*) UICheckboxMouseUpCallback, Checkbox);
	EventListen(&Element->FreeTarget, (void*) UIFreeCheckboxData, Checkbox);

	return Element;
}


bool
UICheckboxGetChecked(
	UIElement* Element
	)
{
	AssertTrue(UIElementIsCheckbox(Element));
	UICheckbox* Checkbox = Element->TypeData;

	return Checkbox->Checked;
}


void
UICheckboxSetChecked(
	UIElement* Element,
	bool Checked
	)
{
	AssertTrue(UIElementIsCheckbox(Element));
	UICheckbox* Checkbox = Element->TypeData;

	Checkbox->Checked = Checked;
}
