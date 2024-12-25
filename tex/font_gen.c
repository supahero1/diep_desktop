#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftstroke.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <DiepDesktop/shared/base.h>
#include <DiepDesktop/shared/file.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>

#define AVG_FONT_SIZE 400
#define STROKE_THICKNESS 31
#define TEXT_FILE_SIZE (UINT32_C(1) << 24)


typedef struct GlyphInfo
{
	int16_t Top;
	int16_t Left;
	uint32_t Codepoint;
}
GlyphInfo;


static uint32_t
po2_ge(uint32_t value)
{
	return 1 << (32 - __builtin_clz(value - 1));
}


int
main(
	void
	)
{
	int WriteImg = getenv("WRITE_IMG") != NULL;
	int FillBlank = getenv("FILL_BLANK") != NULL;

	FT_Library FreeType;
	int Status = FT_Init_FreeType(&FreeType);
	AssertEQ(Status, 0);

	FT_Face Face;
	Status = FT_New_Face(FreeType, "tex/Ubuntu.ttf", 0, &Face);
	AssertEQ(Status, 0);

	FT_Set_Pixel_Sizes(Face, 0, AVG_FONT_SIZE);

	uint32_t MaxWidth = 0;
	uint32_t MaxHeight = 0;
	uint32_t MaxIndex = 0;
	uint32_t MaxCodepoint = 0;

	uint32_t TableSize = UINT32_C(1) << 24;

	GlyphInfo* GlyphData = AllocMalloc(sizeof(GlyphInfo) * TableSize);
	AssertNotNull(GlyphData);

	GlyphInfo* CurrentGlyphData = GlyphData + 1;

	uint32_t* GlyphMap = AllocCalloc(sizeof(uint32_t) * TableSize);
	AssertNotNull(GlyphMap);

	uint32_t* CodepointMap = AllocCalloc(sizeof(uint32_t) * TableSize);
	AssertNotNull(CodepointMap);

	CodepointMap['\t'] = ' ';
	CodepointMap['\n'] = '\n';

	uint32_t GlyphIndex;
	uint32_t Codepoint = FT_Get_First_Char(Face, &GlyphIndex);

	while(GlyphIndex)
	{
		printf("Codepoint %u GlyphIndex %u\n", Codepoint, GlyphIndex);

		if(Codepoint < ' ')
		{
			goto goto_skip;
		}

		CodepointMap[Codepoint] = Codepoint;
		if(Codepoint > MaxCodepoint)
		{
			MaxCodepoint = Codepoint;
		}

		Status = FT_Load_Char(Face, Codepoint, FT_LOAD_TARGET_MONO);
		AssertEQ(Status, 0);

		FT_Stroker Stroker;
		Status = FT_Stroker_New(FreeType, &Stroker);
		AssertEQ(Status, 0);

		FT_Stroker_Set(Stroker, STROKE_THICKNESS * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_MITER_VARIABLE, 0);

		FT_Glyph Glyph;
		Status = FT_Get_Glyph(Face->glyph, &Glyph);
		AssertEQ(Status, 0);

		Status = FT_Glyph_StrokeBorder(&Glyph, Stroker, 0, 1);
		AssertEQ(Status, 0);

		Status = FT_Glyph_To_Bitmap(&Glyph, FT_RENDER_MODE_MONO, 0, 1);
		AssertEQ(Status, 0);

		FT_BitmapGlyph BitmapGlyph = (FT_BitmapGlyph) Glyph;

		uint32_t BgWidth = BitmapGlyph->bitmap.width;
		if(BgWidth > MaxWidth)
		{
			printf("New MaxWidth %u (bg) for Codepoint %u\n", BgWidth, Codepoint);
			MaxWidth = BgWidth;
		}

		uint32_t BgHeight = BitmapGlyph->bitmap.rows;
		if(BgHeight > MaxHeight)
		{
			printf("New MaxHeight %u (bg) for Codepoint %u\n", BgHeight, Codepoint);
			MaxHeight = BgHeight;
		}

		uint32_t BgTop = BitmapGlyph->top;
		uint32_t BgLeft = BitmapGlyph->left;
		uint32_t BgPitch = BitmapGlyph->bitmap.pitch << 3;

		uint32_t BgSize = po2_ge(MAX(BgWidth, BgHeight));
		if(BgSize <= 4)
		{
			BgSize = 4;
		}

		GlyphMap[GlyphIndex] = CurrentGlyphData - GlyphData;
		if(GlyphIndex > MaxIndex)
		{
			MaxIndex = GlyphIndex;
		}

		uint32_t ImageSize = BgSize * BgSize * 4;
		uint8_t* BgImage = AllocCalloc(ImageSize);
		AssertNotNull(BgImage);

		for(uint32_t Y = 0; Y < BgHeight; ++Y)
		{
			for(uint32_t X = 0; X < BgWidth; ++X)
			{
				uint32_t OldIdx = Y * BgPitch + X;
				uint32_t NewIdx = Y * BgSize + X;
				uint32_t Byte = OldIdx >> 3;
				uint32_t Bit = 7 - (OldIdx & 7);

				uint32_t Pixel = (BitmapGlyph->bitmap.buffer[Byte] >> Bit) & 1;
				if(Pixel)
				{
					BgImage[4 * NewIdx + 3] = 255;
				}
			}
		}

		if(FillBlank)
		{
			uint32_t MaxY = BgHeight;
			uint32_t MaxX = BgWidth;

			for(uint32_t Y = 0; Y < MaxY; ++Y)
			{
				for(uint32_t X = 0; X < MaxX; ++X)
				{
					uint8_t* Pixel = BgImage + 4 * (Y * BgSize + X) + 3;

					if(*Pixel)
					{
						continue;
					}

					typedef struct ImgPos
					{
						uint16_t Y;
						uint16_t X;
					}
					ImgPos;

					ImgPos Stack[MaxX * MaxY];
					ImgPos* Head = Stack;

					uint8_t Visited[MaxX * MaxY];
					memset(Visited, 0, MaxX * MaxY);

					*(Head++) = (ImgPos){ Y, X };
					Visited[Y * MaxX + X] = 1;

					int Enclosed = 1;

					do
					{
						--Head;

						uint16_t CurY = Head->Y;
						uint16_t CurX = Head->X;

						if(CurX)
						{
							uint8_t* Visit = Visited + CurY * MaxX + (CurX - 1);

							if(!*Visit && !BgImage[4 * (CurY * BgSize + (CurX - 1)) + 3])
							{
								*Visit = 1;
								*(Head++) = (ImgPos){ CurY, CurX - 1 };
							}
						}
						else
						{
							Enclosed = 0;
							break;
						}

						if(CurX != MaxX - 1)
						{
							uint8_t* Visit = Visited + CurY * MaxX + (CurX + 1);

							if(!*Visit && !BgImage[4 * (CurY * BgSize + (CurX + 1)) + 3])
							{
								*Visit = 1;
								*(Head++) = (ImgPos){ CurY, CurX + 1 };
							}
						}
						else
						{
							Enclosed = 0;
							break;
						}

						if(CurY)
						{
							uint8_t* Visit = Visited + (CurY - 1) * MaxX + CurX;

							if(!*Visit && !BgImage[4 * ((CurY - 1) * BgSize + CurX) + 3])
							{
								*Visit = 1;
								*(Head++) = (ImgPos){ CurY - 1, CurX };
							}
						}
						else
						{
							Enclosed = 0;
							break;
						}

						if(CurY != MaxY - 1)
						{
							uint8_t* Visit = Visited + (CurY + 1) * MaxX + CurX;

							if(!*Visit && !BgImage[4 * ((CurY + 1) * BgSize + CurX) + 3])
							{
								*Visit = 1;
								*(Head++) = (ImgPos){ CurY + 1, CurX };
							}
						}
						else
						{
							Enclosed = 0;
							break;
						}
					}
					while(Head != Stack);

					if(Enclosed)
					{
						*Pixel = 255;
					}
				}
			}
		}

		FT_Stroker_Done(Stroker);
		FT_Done_Glyph(Glyph);

		Status = FT_Get_Glyph(Face->glyph, &Glyph);
		AssertEQ(Status, 0);

		Status = FT_Glyph_To_Bitmap(&Glyph, FT_RENDER_MODE_MONO, 0, 1);
		AssertEQ(Status, 0);

		BitmapGlyph = (FT_BitmapGlyph) Glyph;

		uint32_t Width = BitmapGlyph->bitmap.width;
		if(Width > MaxWidth)
		{
			printf("New MaxWidth %u for Codepoint %u\n", Width, Codepoint);
			MaxWidth = Width;
		}

		uint32_t Height = BitmapGlyph->bitmap.rows;
		if(Height > MaxHeight)
		{
			printf("New MaxHeight %u for Codepoint %u\n", Height, Codepoint);
			MaxHeight = Height;
		}

		uint32_t Left = BitmapGlyph->left;
		uint32_t Top = BitmapGlyph->top;
		uint32_t OffX = Left - BgLeft;
		uint32_t OffY = BgTop - Top;
		uint32_t Pitch = BitmapGlyph->bitmap.pitch << 3;

		for(uint32_t Y = 0; Y < Height; ++Y)
		{
			for(uint32_t X = 0; X < Width; ++X)
			{
				uint32_t OldIdx = Y * Pitch + X;
				uint32_t NewBgIdx = (Y + OffY) * BgSize + (X + OffX);
				uint32_t Byte = OldIdx >> 3;
				uint32_t Bit = 7 - (OldIdx & 7);

				uint32_t Pixel = (BitmapGlyph->bitmap.buffer[Byte] >> Bit) & 1;
				if(Pixel)
				{
					BgImage[4 * NewBgIdx + 0] = 255;
					BgImage[4 * NewBgIdx + 1] = 255;
					BgImage[4 * NewBgIdx + 2] = 255;
					BgImage[4 * NewBgIdx + 3] = 255;
				}
			}
		}

		CurrentGlyphData->Top = Face->glyph->metrics.horiBearingY / 64.0f;
		CurrentGlyphData->Left = Face->glyph->metrics.horiBearingX / 64.0f;
		CurrentGlyphData->Codepoint = Codepoint;
		++CurrentGlyphData;

		if(WriteImg)
		{
			char Filename[32];
			sprintf(Filename, "tex/font/%u.png", Codepoint);

			stbi_write_png(Filename, BgSize, BgSize, 4, BgImage, BgSize * 4);
		}

		AllocFree(ImageSize, BgImage);


		goto_skip:

		Codepoint = FT_Get_Next_Char(Face, Codepoint, &GlyphIndex);
	}

	printf("MaxWidth %d MaxHeight %d MaxIndex %u\n", MaxWidth, MaxHeight, MaxIndex);
	++MaxIndex;

	FT_Done_Face(Face);
	FT_Done_FreeType(FreeType);


	char* GlyphDataText = AllocMalloc(TEXT_FILE_SIZE);
	AssertNotNull(GlyphDataText);

	char* Text = GlyphDataText;

	uint32_t Fonts = CurrentGlyphData - GlyphData;
	int FontSize = (int)(MaxHeight * 1.1f);

	Text += sprintf(Text, "#pragma once\n\n");
	Text += sprintf(Text, "#include <stdint.h>\n\n");
	Text += sprintf(Text, "#include <DiepDesktop/client/tex/base.h>\n\n");
	Text += sprintf(Text, "#define FONT_AVG_SIZE %d\n", AVG_FONT_SIZE);
	Text += sprintf(Text, "#define FONT_SIZE %d\n", FontSize);
	Text += sprintf(Text, "#define FONT_ASCENDER %.04ff\n", FontSize * 0.3f - STROKE_THICKNESS * 1.6666f);
	Text += sprintf(Text, "#define FONT_STROKE_THICKNESS %d\n\n\n", STROKE_THICKNESS);
	Text += sprintf(Text, "typedef struct GlyphInfo\n{\n");
	Text += sprintf(Text, "\tfloat Top;\n");
	Text += sprintf(Text, "\tfloat Left;\n");
	Text += sprintf(Text, "\tTexInfo Texture;\n");
	Text += sprintf(Text, "}\nGlyphInfo;\n\n\n");
	Text += sprintf(Text, "extern const GlyphInfo GlyphInfos[%d];\n\n\n", Fonts);
	Text += sprintf(Text, "extern const uint32_t GlyphMap[%d];\n", MaxIndex);

	FileWrite("include/DiepDesktop/client/tex/font.h", (FileFile) {
		.Buffer = (void*) GlyphDataText,
		.Length = Text - GlyphDataText
	});


	Text = GlyphDataText;

	Text += sprintf(Text, "#include <DiepDesktop/client/tex/font.h>\n\n\n");
	Text += sprintf(Text, "const GlyphInfo GlyphInfos[%d] =\n{\n", Fonts);
	Text += sprintf(Text, "/*   0*/{0},\n");

	GlyphInfo* Data = GlyphData + 1;
	uint32_t i = 1;

	while(Data != CurrentGlyphData)
	{
		Text += sprintf(Text,
			"/*%4u*/{ %.01ff, %.01ff, TEXTURE_%u },\n",
			i, (float) Data->Top, (float) Data->Left, Data->Codepoint);

		++Data;
		++i;
	}

	AllocFree(sizeof(GlyphInfo) * TableSize, GlyphData);

	Text += sprintf(Text, "};\n\n\n");
	Text += sprintf(Text, "const uint32_t GlyphMap[%d] =\n{\n", MaxIndex);

	uint32_t eights = MaxIndex >> 3;
	for(i = 0; i < eights << 3; i += 8)
	{
		Text += sprintf(Text, "/*%4u*/%6u, %6u, %6u, %6u, %6u, %6u, %6u, %6u,\n",
			i, GlyphMap[i], GlyphMap[i + 1], GlyphMap[i + 2], GlyphMap[i + 3],
			GlyphMap[i + 4], GlyphMap[i + 5], GlyphMap[i + 6], GlyphMap[i + 7]);
	}

	for(; i < MaxIndex; ++i)
	{
		Text += sprintf(Text, "/*%4u*/%6u,\n", i, GlyphMap[i]);
	}

	AllocFree(sizeof(uint32_t) * TableSize, GlyphMap);

	Text += sprintf(Text, "};\n");

	FileWrite("src/client/tex/font.c", (FileFile) {
		.Buffer = (void*) GlyphDataText,
		.Length = Text - GlyphDataText
	});

	AllocFree(TEXT_FILE_SIZE, GlyphDataText);


	printf("\tswitch(Codepoint)\n\t{\n\n\tcase   '\\t': return ' ';\n\tcase   '\\n': ");

	uint32_t Printed = 1;
	uint32_t Dedupe = 1;
	for(i = 32; i <= MaxCodepoint; ++i)
	{
		if(!(Printed & 7) && !Dedupe)
		{
			printf("\n\t");
			Dedupe = 1;
		}

		if(CodepointMap[i])
		{
			printf("case %6u: ", i);
			++Printed;
			Dedupe = 0;
		}
	}

	printf("return Codepoint;\n\n\tdefault: return 0;\n\n\t}\n");

	AllocFree(sizeof(uint32_t) * TableSize, CodepointMap);


	return 0;
}
