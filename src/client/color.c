#include <DiepDesktop/shared/base.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/client/color.h>

#include <math.h>


HSV
RGBtoHSV(
	ARGB Color
	)
{
	float R = Color.R / 255.0f;
	float G = Color.G / 255.0f;
	float B = Color.B / 255.0f;

	float Max = MAX(MAX(R, G), B);
	float Min = MIN(MIN(R, G), B);
	float Delta = Max - Min;

	HSV Out;

	if(Delta == 0.0f)
	{
		Out.H = 0.0f;
	}
	else if(Max == R)
	{
		Out.H = (G - B) / Delta;

		if(Out.H < 0.0f)
		{
			Out.H += 6.0f;
		}
	}
	else if(Max == G)
	{
		Out.H = (B - R) / Delta + 2;
	}
	else
	{
		Out.H = (R - G) / Delta + 4;
	}

	Out.H /= 6.0f;
	AssertLE(Out.H, 1.0f);

	if(Max == 0.0f)
	{
		Out.S = 0.0f;
	}
	else
	{
		Out.S = Delta / Max;
	}

	AssertLE(Out.S, 1.0f);

	Out.V = Max;

	return Out;
}


ARGB
HSVtoRGB(
	HSV Color
	)
{
	float	V			= Color.V * 255.0f;
	float	S			= Color.S * V;
	uint32_t Section	= Color.H * 6.0f;
	float	Remainder	= Color.H * 6.0f - Section;
	uint8_t	Zero		= nearbyintf(V - S                     );
	uint8_t	Dropping	= nearbyintf(V - S *         Remainder );
	uint8_t	Rising		= nearbyintf(V - S * (1.0f - Remainder));

	switch(Section)
	{

	case 0: return (ARGB){ .B = Zero	, .G = Rising	, .R = V		, .A = 0xFF };
	case 1: return (ARGB){ .B = Zero	, .G = V		, .R = Dropping	, .A = 0xFF };
	case 2: return (ARGB){ .B = Rising	, .G = V		, .R = Zero		, .A = 0xFF };
	case 3: return (ARGB){ .B = V		, .G = Dropping	, .R = Zero		, .A = 0xFF };
	case 4: return (ARGB){ .B = V		, .G = Zero		, .R = Rising	, .A = 0xFF };
	case 5: return (ARGB){ .B = Dropping, .G = Zero		, .R = V		, .A = 0xFF };
	case 6: return (ARGB){ .B = Zero	, .G = Zero		, .R = V		, .A = 0xFF };

	default: AssertUnreachable();

	}
}


XOY
HSVtoXOY(
	HSV Color
	)
{
	float Distance = Color.S * 1.0f * 0.5f;
	float Angle = Color.H * 2.0f * M_PI;

	XOY Out;
	Out.X = -Distance * cosf(Angle);
	Out.Y = -Distance * sinf(Angle);

	return Out;
}


HSV
XOYtoHSV(
	XOY Position
	)
{
	float H = (atan2f(Position.Y, Position.X) + M_PI) / (M_PI * 2.0f);
	AssertLE(H, 1.0f);

	float S = sqrtf(Position.X * Position.X + Position.Y * Position.Y) * 2.0f;
	S = MIN(S, 1.0f);
	AssertLE(S, 1.0f);

	return (HSV){ .H = H, .S = S, .V = 1.0f };
}


uint8_t
AmulA(
	uint8_t Alpha,
	uint8_t Other
	)
{
	uint32_t One = Alpha;
	uint32_t Two = Other;

	return (One * Two + 255) >> 8;
}


ARGB
RGBmulA(
	ARGB Color,
	uint8_t Alpha
	)
{
	Color.A = AmulA(Alpha, Color.A);
	return Color;
}
