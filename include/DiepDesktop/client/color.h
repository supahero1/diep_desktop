#pragma once

#include <stdint.h>


typedef union HSV
{
	float HSV[3];

	struct
	{
		float H;
		float S;
		float V;
	};
}
HSV;


typedef union ARGB
{
	uint32_t ARGB;

	struct
	{
		uint8_t B;
		uint8_t G;
		uint8_t R;
		uint8_t A;
	};
}
ARGB;


typedef union XOY
{
	float XOY[2];

	struct
	{
		float X;
		float Y;
	};
}
XOY;


extern HSV
RGBtoHSV(
	ARGB Color
	);


extern ARGB
HSVtoRGB(
	HSV Color
	);


extern XOY
HSVtoXOY(
	HSV Color
	);


extern HSV
XOYtoHSV(
	XOY Position
	);


extern uint8_t
AmulA(
	uint8_t Alpha,
	uint8_t Other
	);


extern ARGB
RGBmulA(
	ARGB Color,
	uint8_t Alpha
	);
