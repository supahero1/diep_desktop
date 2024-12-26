#pragma once

#include <DiepDesktop/client/color.h>
#include <DiepDesktop/shared/event.h>


typedef enum Setting : uint32_t
{
	kSETTING
}
Setting;

typedef enum SettingType : uint8_t
{
	SETTING_TYPE_INTEGER,
	SETTING_TYPE_FLOAT,
	SETTING_TYPE_BOOLEAN,
	SETTING_TYPE_STRING,
	SETTING_TYPE_ENUM,
	SETTING_TYPE_COLOR,
}
SettingType;

typedef union SettingValue
{
	int64_t Integer;
	float Float;
	bool Boolean;
	uint8_t* String;
	int64_t Enum;
	ARGB Color;
}
SettingValue; // merge with constraint below

typedef union SettingConstraint
{
	struct
	{
		int64_t Min;
		int64_t Max;
	}
	Integer;

	struct
	{
		float Min;
		float Max;
	}
	Float;

	struct
	{
		uint64_t MaxLength;
	}
	String;

	struct
	{
		int64_t Min;
		int64_t Max;
	}
	Enum;
}
SettingConstraint;

typedef struct SettingSpecification
{
	SettingType Type;
	SettingValue Value;
	SettingConstraint Constraint;
	EventTarget* ChangeTarget;
}
SettingSpecification;

typedef struct Settings
{
	// HashTable Specifications;
	// HashTable Values;
}
Settings;


extern void
SettingsInit(
	Settings* Settings
	);


extern void
SettingsLoad(
	const char* File,
	Settings* Settings
	);


extern void
SettingsSave(
	const char* File,
	Settings* Settings
	);


extern void
SettingsFree(
	Settings* Settings
	);


extern SettingValue
SettingsGetValue(
	Settings* Settings,
	const char* Name
	);


extern void
SettingsSetValue(
	Settings* Settings,
	const char* Name,
	SettingValue Value
	);
