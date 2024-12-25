#pragma once

#include <DiepDesktop/client/ui/base.h>
#include <DiepDesktop/client/tex/base.h>


typedef struct UICheckbox
{
	bool Checked;

	ARGB CheckYes;
	ARGB CheckYesBackground;

	ARGB CheckNo;
	ARGB CheckNoBackground;
}
UICheckbox;


typedef struct UICheckboxInfo
{
	bool Checked;

	ARGB CheckYes;
	ARGB CheckYesBackground;

	ARGB CheckNo;
	ARGB CheckNoBackground;
}
UICheckboxInfo;


extern bool
UIElementIsCheckbox(
	UIElement* Element
	);


extern UIElement*
UIAllocCheckbox(
	UIElementInfo ElementInfo,
	UICheckboxInfo Info
	);


extern bool
UICheckboxGetChecked(
	UIElement* Element
	);


extern void
UICheckboxSetChecked(
	UIElement* Element,
	bool Checked
	);
