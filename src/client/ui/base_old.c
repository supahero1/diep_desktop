#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/threads.h>
#include <DiepDesktop/client/ui/base.h>
#include <DiepDesktop/client/tex/font.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_clipboard.h>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#define DRAW_DEPTH_JIFFIE (1.0f / (1U << 22U))
#define DRAW_DEPTH_LEAP (1.0f / (1U << 20U))
#define COLOR_PICKER_HANDLE_SIZE 0.0625f


Static UIElement Elements[GAME_CONST_MAX_UI_ELEMENTS] =
	{ { .Callback = UIEmptyCallback, .Type = UI_TYPE_CONTAINER } };
Static UIElement* FreeElement;
Static uint32_t ElementsCount = 1;

UIElement* UIWindow = Elements;
float MouseX;
float MouseY;
float ScrollOffset;
bool SameElement;

Static VkVertexInstanceInput DrawInput[GAME_CONST_MAX_TEXTURES];
Static uint32_t DrawIndex;
Static float DeltaTime;
Static float Depth;

Static UIElement* ElementUnderMouse;
Static UIElement* ClickableUnderMouse;
Static UIElement* ScrollableUnderMouse;

Static UIElement* SelectedElement;
Static UIElement* SelectedTextElement;

Static UIElement* LastScrollable;

Static UITextOffset SelectionHead;
Static UITextOffset SelectionTail;
Static UITextOffset SelectionStart;
Static UITextOffset SelectionEnd;
Static UITextOffset LastSelection;

Static float DeltaW;
Static float DeltaH;

Static uint64_t LastDrawAt;
Static uint64_t LastCursorSetAt;

Static bool KeyState[kUI_KEY] = {0};
Static bool ButtonState[kUI_BUTTON] = {0};

Static FT_Library FreeTypeLibrary;
Static FT_Face FreeTypeFace;
Static hb_font_t* HarfBuzzFont;
Static hb_feature_t HarfBuzzFeatures[] =
{
	{ HB_TAG('l', 'i', 'g', 'a'), 0, 0, -1 }, /* Ligatures suck */
};


Static uint32_t
DefaultFilter(
	uint32_t Codepoint
	)
{
	switch(Codepoint)
	{

	case   '\t': return ' ';
	case   '\n': case     32: case     33: case     34: case     35: case     36: case     37: case     38:
	case     39: case     40: case     41: case     42: case     43: case     44: case     45: case     46:
	case     47: case     48: case     49: case     50: case     51: case     52: case     53: case     54:
	case     55: case     56: case     57: case     58: case     59: case     60: case     61: case     62:
	case     63: case     64: case     65: case     66: case     67: case     68: case     69: case     70:
	case     71: case     72: case     73: case     74: case     75: case     76: case     77: case     78:
	case     79: case     80: case     81: case     82: case     83: case     84: case     85: case     86:
	case     87: case     88: case     89: case     90: case     91: case     92: case     93: case     94:
	case     95: case     96: case     97: case     98: case     99: case    100: case    101: case    102:
	case    103: case    104: case    105: case    106: case    107: case    108: case    109: case    110:
	case    111: case    112: case    113: case    114: case    115: case    116: case    117: case    118:
	case    119: case    120: case    121: case    122: case    123: case    124: case    125: case    126:
	case    160: case    161: case    162: case    163: case    164: case    165: case    166: case    167:
	case    168: case    169: case    170: case    171: case    172: case    173: case    174: case    175:
	case    176: case    177: case    178: case    179: case    180: case    181: case    182: case    183:
	case    184: case    185: case    186: case    187: case    188: case    189: case    190: case    191:
	case    192: case    193: case    194: case    195: case    196: case    197: case    198: case    199:
	case    200: case    201: case    202: case    203: case    204: case    205: case    206: case    207:
	case    208: case    209: case    210: case    211: case    212: case    213: case    214: case    215:
	case    216: case    217: case    218: case    219: case    220: case    221: case    222: case    223:
	case    224: case    225: case    226: case    227: case    228: case    229: case    230: case    231:
	case    232: case    233: case    234: case    235: case    236: case    237: case    238: case    239:
	case    240: case    241: case    242: case    243: case    244: case    245: case    246: case    247:
	case    248: case    249: case    250: case    251: case    252: case    253: case    254: case    255:
	case    256: case    257: case    258: case    259: case    260: case    261: case    262: case    263:
	case    264: case    265: case    266: case    267: case    268: case    269: case    270: case    271:
	case    272: case    273: case    274: case    275: case    276: case    277: case    278: case    279:
	case    280: case    281: case    282: case    283: case    284: case    285: case    286: case    287:
	case    288: case    289: case    290: case    291: case    292: case    293: case    294: case    295:
	case    296: case    297: case    298: case    299: case    300: case    301: case    302: case    303:
	case    304: case    305: case    306: case    307: case    308: case    309: case    310: case    311:
	case    312: case    313: case    314: case    315: case    316: case    317: case    318: case    319:
	case    320: case    321: case    322: case    323: case    324: case    325: case    326: case    327:
	case    328: case    329: case    330: case    331: case    332: case    333: case    334: case    335:
	case    336: case    337: case    338: case    339: case    340: case    341: case    342: case    343:
	case    344: case    345: case    346: case    347: case    348: case    349: case    350: case    351:
	case    352: case    353: case    354: case    355: case    356: case    357: case    358: case    359:
	case    360: case    361: case    362: case    363: case    364: case    365: case    366: case    367:
	case    368: case    369: case    370: case    371: case    372: case    373: case    374: case    375:
	case    376: case    377: case    378: case    379: case    380: case    381: case    382: case    383:
	case    384: case    385: case    386: case    387: case    388: case    389: case    390: case    391:
	case    392: case    393: case    394: case    395: case    396: case    397: case    398: case    399:
	case    400: case    401: case    402: case    403: case    404: case    405: case    406: case    407:
	case    408: case    409: case    410: case    411: case    412: case    413: case    414: case    415:
	case    416: case    417: case    418: case    419: case    420: case    421: case    422: case    423:
	case    424: case    425: case    426: case    427: case    428: case    429: case    430: case    431:
	case    432: case    433: case    434: case    435: case    436: case    437: case    438: case    439:
	case    440: case    441: case    442: case    443: case    444: case    445: case    446: case    447:
	case    448: case    449: case    450: case    451: case    452: case    453: case    454: case    455:
	case    456: case    457: case    458: case    459: case    460: case    461: case    462: case    463:
	case    464: case    465: case    466: case    467: case    468: case    469: case    470: case    471:
	case    472: case    473: case    474: case    475: case    476: case    477: case    478: case    479:
	case    480: case    481: case    482: case    483: case    484: case    485: case    486: case    487:
	case    488: case    489: case    490: case    491: case    492: case    493: case    494: case    495:
	case    496: case    497: case    498: case    499: case    500: case    501: case    502: case    503:
	case    504: case    505: case    506: case    507: case    508: case    509: case    510: case    511:
	case    512: case    513: case    514: case    515: case    516: case    517: case    518: case    519:
	case    520: case    521: case    522: case    523: case    524: case    525: case    526: case    527:
	case    528: case    529: case    530: case    531: case    532: case    533: case    534: case    535:
	case    536: case    537: case    538: case    539: case    540: case    541: case    542: case    543:
	case    544: case    545: case    546: case    547: case    548: case    549: case    550: case    551:
	case    552: case    553: case    554: case    555: case    556: case    557: case    558: case    559:
	case    560: case    561: case    562: case    563: case    564: case    565: case    566: case    567:
	case    568: case    569: case    570: case    571: case    572: case    573: case    574: case    575:
	case    576: case    577: case    578: case    579: case    580: case    581: case    582: case    583:
	case    584: case    585: case    586: case    587: case    588: case    589: case    590: case    591:
	case    616: case    658: case    700: case    710: case    711: case    713: case    728: case    729:
	case    730: case    731: case    732: case    733: case    768: case    769: case    770: case    771:
	case    772: case    774: case    775: case    776: case    778: case    779: case    780: case    783:
	case    785: case    786: case    803: case    806: case    807: case    808: case    809: case    816:
	case    817: case    821: case    900: case    901: case    902: case    904: case    905: case    906:
	case    908: case    910: case    911: case    912: case    913: case    914: case    915: case    916:
	case    917: case    918: case    919: case    920: case    921: case    922: case    923: case    924:
	case    925: case    926: case    927: case    928: case    929: case    931: case    932: case    933:
	case    934: case    935: case    936: case    937: case    938: case    939: case    940: case    941:
	case    942: case    943: case    944: case    945: case    946: case    947: case    948: case    949:
	case    950: case    951: case    952: case    953: case    954: case    955: case    956: case    957:
	case    958: case    959: case    960: case    961: case    962: case    963: case    964: case    965:
	case    966: case    967: case    968: case    969: case    970: case    971: case    972: case    973:
	case    974: case   1024: case   1025: case   1026: case   1027: case   1028: case   1029: case   1030:
	case   1031: case   1032: case   1033: case   1034: case   1035: case   1036: case   1037: case   1038:
	case   1039: case   1040: case   1041: case   1042: case   1043: case   1044: case   1045: case   1046:
	case   1047: case   1048: case   1049: case   1050: case   1051: case   1052: case   1053: case   1054:
	case   1055: case   1056: case   1057: case   1058: case   1059: case   1060: case   1061: case   1062:
	case   1063: case   1064: case   1065: case   1066: case   1067: case   1068: case   1069: case   1070:
	case   1071: case   1072: case   1073: case   1074: case   1075: case   1076: case   1077: case   1078:
	case   1079: case   1080: case   1081: case   1082: case   1083: case   1084: case   1085: case   1086:
	case   1087: case   1088: case   1089: case   1090: case   1091: case   1092: case   1093: case   1094:
	case   1095: case   1096: case   1097: case   1098: case   1099: case   1100: case   1101: case   1102:
	case   1103: case   1104: case   1105: case   1106: case   1107: case   1108: case   1109: case   1110:
	case   1111: case   1112: case   1113: case   1114: case   1115: case   1116: case   1117: case   1118:
	case   1119: case   1122: case   1123: case   1138: case   1139: case   1140: case   1141: case   1162:
	case   1163: case   1164: case   1165: case   1166: case   1167: case   1168: case   1169: case   1170:
	case   1171: case   1172: case   1173: case   1174: case   1175: case   1176: case   1177: case   1178:
	case   1179: case   1180: case   1181: case   1182: case   1183: case   1184: case   1185: case   1186:
	case   1187: case   1188: case   1189: case   1190: case   1191: case   1192: case   1193: case   1194:
	case   1195: case   1196: case   1197: case   1198: case   1199: case   1200: case   1201: case   1202:
	case   1203: case   1204: case   1205: case   1206: case   1207: case   1208: case   1209: case   1210:
	case   1211: case   1212: case   1213: case   1214: case   1215: case   1216: case   1217: case   1218:
	case   1219: case   1220: case   1221: case   1222: case   1223: case   1224: case   1225: case   1226:
	case   1227: case   1228: case   1229: case   1230: case   1231: case   1232: case   1233: case   1234:
	case   1235: case   1236: case   1237: case   1238: case   1239: case   1240: case   1241: case   1242:
	case   1243: case   1244: case   1245: case   1246: case   1247: case   1248: case   1249: case   1250:
	case   1251: case   1252: case   1253: case   1254: case   1255: case   1256: case   1257: case   1258:
	case   1259: case   1260: case   1261: case   1262: case   1263: case   1264: case   1265: case   1266:
	case   1267: case   1268: case   1269: case   1270: case   1271: case   1272: case   1273: case   7724:
	case   7725: case   7788: case   7789: case   7808: case   7809: case   7810: case   7811: case   7812:
	case   7813: case   7838: case   7882: case   7883: case   7922: case   7923: case   7936: case   7937:
	case   7938: case   7939: case   7940: case   7941: case   7942: case   7943: case   7944: case   7945:
	case   7946: case   7947: case   7948: case   7949: case   7950: case   7951: case   7952: case   7953:
	case   7954: case   7955: case   7956: case   7957: case   7960: case   7961: case   7962: case   7963:
	case   7964: case   7965: case   7968: case   7969: case   7970: case   7971: case   7972: case   7973:
	case   7974: case   7975: case   7976: case   7977: case   7978: case   7979: case   7980: case   7981:
	case   7982: case   7983: case   7984: case   7985: case   7986: case   7987: case   7988: case   7989:
	case   7990: case   7991: case   7992: case   7993: case   7994: case   7995: case   7996: case   7997:
	case   7998: case   7999: case   8000: case   8001: case   8002: case   8003: case   8004: case   8005:
	case   8008: case   8009: case   8010: case   8011: case   8012: case   8013: case   8016: case   8017:
	case   8018: case   8019: case   8020: case   8021: case   8022: case   8023: case   8025: case   8027:
	case   8029: case   8031: case   8032: case   8033: case   8034: case   8035: case   8036: case   8037:
	case   8038: case   8039: case   8040: case   8041: case   8042: case   8043: case   8044: case   8045:
	case   8046: case   8047: case   8048: case   8049: case   8050: case   8051: case   8052: case   8053:
	case   8054: case   8055: case   8056: case   8057: case   8058: case   8059: case   8060: case   8061:
	case   8064: case   8065: case   8066: case   8067: case   8068: case   8069: case   8070: case   8071:
	case   8072: case   8073: case   8074: case   8075: case   8076: case   8077: case   8078: case   8079:
	case   8080: case   8081: case   8082: case   8083: case   8084: case   8085: case   8086: case   8087:
	case   8088: case   8089: case   8090: case   8091: case   8092: case   8093: case   8094: case   8095:
	case   8096: case   8097: case   8098: case   8099: case   8100: case   8101: case   8102: case   8103:
	case   8104: case   8105: case   8106: case   8107: case   8108: case   8109: case   8110: case   8111:
	case   8112: case   8113: case   8114: case   8115: case   8116: case   8118: case   8119: case   8120:
	case   8121: case   8122: case   8123: case   8124: case   8125: case   8126: case   8127: case   8128:
	case   8129: case   8130: case   8131: case   8132: case   8134: case   8135: case   8136: case   8137:
	case   8138: case   8139: case   8140: case   8141: case   8142: case   8143: case   8144: case   8145:
	case   8146: case   8147: case   8150: case   8151: case   8152: case   8153: case   8154: case   8155:
	case   8157: case   8158: case   8159: case   8160: case   8161: case   8162: case   8163: case   8164:
	case   8165: case   8166: case   8167: case   8168: case   8169: case   8170: case   8171: case   8172:
	case   8173: case   8174: case   8175: case   8178: case   8179: case   8180: case   8182: case   8183:
	case   8184: case   8185: case   8186: case   8187: case   8188: case   8189: case   8190: case   8201:
	case   8211: case   8212: case   8213: case   8216: case   8217: case   8218: case   8220: case   8221:
	case   8222: case   8224: case   8225: case   8226: case   8230: case   8240: case   8242: case   8243:
	case   8249: case   8250: case   8260: case   8304: case   8308: case   8309: case   8310: case   8311:
	case   8312: case   8313: case   8320: case   8321: case   8322: case   8323: case   8324: case   8325:
	case   8326: case   8327: case   8328: case   8329: case   8364: case   8366: case   8372: case   8376:
	case   8377: case   8381: case   8467: case   8470: case   8471: case   8480: case   8482: case   8486:
	case   8494: case   8528: case   8529: case   8531: case   8532: case   8533: case   8534: case   8535:
	case   8536: case   8537: case   8538: case   8539: case   8540: case   8541: case   8542: case   8706:
	case   8710: case   8719: case   8721: case   8722: case   8725: case   8729: case   8730: case   8734:
	case   8747: case   8776: case   8800: case   8804: case   8805: case   9674: case   9676: case  10003:
	case  10060: case  64257: case  64258: return Codepoint;

	default: return 0;

	}
}


void
UIInit(
	void
	)
{
	int Status = FT_Init_FreeType(&FreeTypeLibrary);
	AssertEQ(Status, 0);

	Status = FT_New_Face(FreeTypeLibrary, "Ubuntu.ttf", 0, &FreeTypeFace);
	AssertEQ(Status, 0);

	FT_Set_Pixel_Sizes(FreeTypeFace, 0, FONT_AVG_SIZE);

	HarfBuzzFont = hb_ft_font_create(FreeTypeFace, NULL);
}


void
UIFree(
	void
	)
{
	UIElement* Element = Elements + 1;
	UIElement* ElementEnd = Elements + ElementsCount;

	while(Element != ElementEnd)
	{
		if(Element->Callback)
		{
			UIRetElement(Element);
		}

		++Element;
	}

	hb_font_destroy(HarfBuzzFont);

	FT_Done_Face(FreeTypeFace);
	FT_Done_FreeType(FreeTypeLibrary);
}


Static void
UITextFreeLines(
	UIText* Text
	)
{
	for(uint32_t i = 0; i < Text->LineCount; ++i)
	{
		free(Text->Lines[i]);
	}

	free(Text->Lines);
}


Static void
UITextFreeMod(
	UITextMod* Mod
	)
{
	free((uint32_t*) Mod->OldCodepoints);
	free((uint32_t*) Mod->NewCodepoints);
}


Static void
UITextFreeMods(
	UIText* Text
	)
{
	UITextMod* Mod = Text->Mods;
	UITextMod* ModEnd = Text->Mods + Text->ModCount;

	while(Mod != ModEnd)
	{
		UITextFreeMod(Mod++);
	}

	free(Text->Mods);
}


void
UITextOnFree(
	UIElement* Element
	)
{
	UIText* Text = &Element->Text;

	UITextFreeLines(Text);
	UITextFreeMods(Text);
}


typedef struct UITextParseState
{
	uint32_t LinesUsed;
	uint32_t LinesSize;
	UITextLine** Lines;

	float WidthLimit;
	float MaxLineWidth;

	float Scale;
	float Padding;
}
UITextParseState;


Static uint32_t
UITextParse(
	UITextParseState* State,
	const uint32_t* Codepoints,
	uint32_t Length
	)
{
	hb_buffer_t* HarfBuzzBuffer = hb_buffer_create();
	AssertEQ(hb_buffer_allocation_successful(HarfBuzzBuffer), 1);

	hb_buffer_set_direction(HarfBuzzBuffer, HB_DIRECTION_LTR);
	hb_buffer_set_script(HarfBuzzBuffer, HB_SCRIPT_LATIN);
	hb_buffer_set_language(HarfBuzzBuffer, hb_language_from_string("en", 2));
	hb_buffer_set_flags(HarfBuzzBuffer, HB_BUFFER_FLAG_REMOVE_DEFAULT_IGNORABLES);
	hb_buffer_set_cluster_level(HarfBuzzBuffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

	hb_buffer_add_codepoints(HarfBuzzBuffer, Codepoints, Length, 0, Length);

	hb_shape(HarfBuzzFont, HarfBuzzBuffer, HarfBuzzFeatures, ARRAYLEN(HarfBuzzFeatures));

	uint32_t HarfBuzzGlyphCount = hb_buffer_get_length(HarfBuzzBuffer);
	hb_glyph_info_t* HarfBuzzGlyphInfo = hb_buffer_get_glyph_infos(HarfBuzzBuffer, NULL);
	hb_glyph_position_t* HarfBuzzGlyphPos = hb_buffer_get_glyph_positions(HarfBuzzBuffer, NULL);

	typedef struct ValidGlyph
	{
		uint32_t GlyphIndex;
		uint32_t Flags;
		int32_t Advance;
		int32_t OffsetX;
		int32_t OffsetY;
	}
	ValidGlyph;

	ValidGlyph* ValidGlyphs = malloc(sizeof(ValidGlyph) * HarfBuzzGlyphCount);
	uint32_t ValidGlyphCount = 0;

	for(uint32_t i = 0; i < HarfBuzzGlyphCount; ++i)
	{
		hb_glyph_info_t* Info = HarfBuzzGlyphInfo + i;
		hb_glyph_position_t* Pos = HarfBuzzGlyphPos + i;
		hb_glyph_flags_t Flags = hb_glyph_info_get_glyph_flags(Info);

		uint32_t GlyphCode = Info->codepoint;
		AssertLT(GlyphCode, ARRAYLEN(GlyphMap));

		uint32_t GlyphIndex = GlyphMap[GlyphCode];
		AssertNEQ(GlyphIndex, 0);

		AssertEQ(Info->cluster, i);

		ValidGlyphs[ValidGlyphCount++] =
		(ValidGlyph)
		{
			.GlyphIndex = GlyphIndex,
			.Flags = Flags,
			.Advance = Pos->x_advance,
			.OffsetX = Pos->x_offset,
			.OffsetY = Pos->y_offset
		};
	}

	hb_buffer_destroy(HarfBuzzBuffer);


	float Width = State->Padding;

	const ValidGlyph* LastSpace = NULL;
	float WidthAtLastSpace;

	const ValidGlyph* Glyph = ValidGlyphs;
	const ValidGlyph* GlyphEnd = ValidGlyphs + ValidGlyphCount;

	uint32_t CharIndex = 0;

printf("within UITextParse(), validGlyphCount: %u\n", ValidGlyphCount);
	if(!ValidGlyphCount)
	{
		goto goto_form_line;
	}

	while(Glyph != GlyphEnd)
	{
		uint32_t Codepoint = Codepoints[CharIndex];
printf("parsing codepoint %u\n", Codepoint);
		if(Codepoint == ' ')
		{
			printf("glyph is a space\n");
			LastSpace = Glyph;
			WidthAtLastSpace = Width;
		}

		float Stride = Glyph->Advance / 64.0f * State->Scale;
printf("width: %f, stride: %f\n", Width, Stride);
		Width += Stride;

		if(Width > State->WidthLimit && CharIndex)
		{
			if(LastSpace)
			{
				Glyph = LastSpace;
				Width = WidthAtLastSpace;
			}
			else
			{
				Width -= Stride;
			}

			printf("breaking line at char idx %u line len %lu\n", CharIndex, Glyph - ValidGlyphs);

			if(!(Glyph->Flags & HB_GLYPH_FLAG_UNSAFE_TO_BREAK))
			{
				puts("safe to break");

				goto_form_line:;

				uint32_t Length = Glyph - ValidGlyphs;
				UITextLine* Line = malloc(sizeof(UITextLine) + Length * sizeof(UITextGlyph));
				AssertNotNull(Line);

				Line->Width = Width;

				Line->Separator = 0;
				Line->Length = Length;

				for(uint32_t i = 0; i < Length; ++i)
				{
					const ValidGlyph* CharGlyph = ValidGlyphs + i;
					const GlyphInfo* CharInfo = GlyphInfos + CharGlyph->GlyphIndex;

					Line->Glyphs[i] =
					(UITextGlyph)
					{
						.Top = (CharInfo->Top + CharGlyph->OffsetX / 64.0f) * State->Scale,
						.Left = (CharInfo->Left + CharGlyph->OffsetY / 64.0f) * State->Scale,
						.Stride = CharGlyph->Advance / 64.0f * State->Scale,
						.Size = TEXSIZE(CharInfo->Texture) * State->Scale,
						.Texture = CharInfo->Texture
					};
				}

				if(State->LinesUsed == State->LinesSize)
				{
					State->LinesSize <<= 1;
					State->Lines = realloc(State->Lines, State->LinesSize * sizeof(UITextLine*));
					AssertNotNull(State->Lines);
				}

				State->Lines[State->LinesUsed++] = Line;

				if(Width > State->MaxLineWidth)
				{
					State->MaxLineWidth = Width;
				}
puts("nice return");
				return CharIndex;
			}
			else
			{
				puts("unsafe to break :( not so cool return");
				return UITextParse(State, Codepoints, CharIndex);
			}

			AssertUnreachable();
		}

		++Glyph;
		++CharIndex;
	}

	goto goto_form_line;
}


Static void
UITextConfigure(
	UIElement* Element
	)
{
	UIText* Text = &Element->Text;

	/* Can safely change the codepoints from within this callback */
	Element->Callback(Element, UI_EVENT_CHANGE);

	UITextFreeLines(Text);

	AssertLE(Text->Length, Text->MaxLength);

	float Size = Text->FontSize;
	float Scale = Size / FONT_SIZE;
	float Padding = STROKE_THICKNESS * Scale * 2.0f;

	UITextParseState State =
	{
		.LinesUsed = 0,
		.LinesSize = 1,
		.Lines = malloc(sizeof(UITextLine*)),

		.WidthLimit = Text->MaxWidth ? Text->MaxWidth : __builtin_huge_valf32(),
		.MaxLineWidth = 0.0f,

		.Scale = Scale,
		.Padding = Padding
	};
	AssertNotNull(State.Lines);


	uint32_t LineBegin = 0;
printf("and so the parsing begins, text contains %u codepoints\n", Text->Length);
	for(uint32_t i = 0; i < Text->Length; ++i)
	{
		if(Text->Codepoints[i] != '\n')
		{
			continue;
		}

		goto_form_line:;

		printf("breakpoint at char idx %u\n", i);

		uint32_t Idx;
		uint32_t Length;

		do
		{
			printf("\nparsing line at idx %u\n", LineBegin);
			Length = i - LineBegin;
			Idx = UITextParse(&State, &Text->Codepoints[LineBegin], Length);
			printf("after parsing line, got idx %u, we want to reach %u tho\n", Idx, Length);
			LineBegin += Idx;
		}
		while(Idx != Length);

		State.Lines[State.LinesUsed - 1]->Separator = 1;

		++LineBegin;
	}

	if(LineBegin <= Text->Length)
	{
		puts("\nend thing goto thing");
		goto goto_form_line;
	}

	State.Lines[State.LinesUsed - 1]->Separator = 0;

#ifndef NDEBUG
	uint32_t CheckLength = 0;

	for(uint32_t i = 0; i < State.LinesUsed; ++i)
	{
		CheckLength += State.Lines[i]->Length + State.Lines[i]->Separator;
	}

	AssertEQ(CheckLength, Text->Length);
#endif

	Text->LineCount = State.LinesUsed;
	Text->Lines = State.Lines;

	Element->W = State.MaxLineWidth;
	Element->H = Size * Text->LineCount;

	float HeightOffset = -(Element->H - Size) * 0.5f;

	for(uint32_t i = 0; i < State.LinesUsed; ++i)
	{
		UITextLine* Line = State.Lines[i];


		switch(Text->AlignX)
		{

		case UI_ALIGN_LEFT:
		{
			Line->X = -Element->W * 0.5f;
			break;
		}

		case UI_ALIGN_CENTER:
		{
			Line->X = Line->Width * -0.5f;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Line->X = Element->W - Line->Width;
			break;
		}

		}


		Line->Y = HeightOffset + FONT_ASCENDER * Scale;
		Line->Width += Padding;
		HeightOffset += Size;
	}
}


void
UIInitialize(
	UIElement* Element
	)
{
	if(!Element->Callback)
	{
		Element->Callback = UIEmptyCallback;
	}

	Element->FrozenCallback = Element->Callback;
	Element->Callback = UIEmptyCallback;


	switch(Element->Type)
	{

	case UI_TYPE_TEXT:
	{
		UIText* Text = &Element->Text;

		if(!Text->Filter)
		{
			Text->Filter = DefaultFilter;
		}

		Text->AllowNewline = !!Text->Filter('\n');

		if(!Text->MaxLength)
		{
			Text->MaxLength = 1000;
		}

		break;
	}

	default: break;

	}


	UIUpdateSize(Element);
}


Static bool
UIMouseOver(
	UIElement* Element
	)
{
	switch(Element->Type)
	{

	case UI_TYPE_CONTAINER:
	case UI_TYPE_CHECKBOX:
	case UI_TYPE_SCROLLBAR:
	case UI_TYPE_TEXTURE:
	{
		return true;
	}

	case UI_TYPE_TEXT:
	{
		UIText* Text = &Element->Text;

		float Size = Text->FontSize;
		float Scale = Size / FONT_SIZE;
		float HalfPadding = STROKE_THICKNESS * Scale;

		float Top = Element->EndY - Element->H * 0.5f;
		if(MouseY < Top)
		{
			return false;
		}

		uint32_t LineIdx = (MouseY - Top) / Text->FontSize;
		if(LineIdx >= Text->LineCount)
		{
			return false;
		}

		UITextLine* Line = Text->Lines[LineIdx];
		float X = MouseX - (Element->EndX + Line->X) + HalfPadding;
		if(X < 0.0f || X > Line->Width)
		{
			return false;
		}

		return true;
	}

	case UI_TYPE_SLIDER:
	{
		return UIMouseOverSlider(
			Element->Slider.Axis,
			Element->EndX, Element->EndY,
			Element->W, Element->H
		);
	}

// TODO one function for sliders and borders and scrollbars that accepts width height and radius and mouse pos
	case UI_TYPE_COLOR_PICKER:
	{
		AssertEQ(Element->W, Element->H);

		float DiffX = Element->EndX - MouseX;
		float DiffY = Element->EndY - MouseY;
		float R = Element->W * 0.5f;

		return DiffX * DiffX + DiffY * DiffY < R * R;
	}

	}

	AssertUnreachable();
}


Static void
UIDrawElement(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable,
	UIElement* Parent
	)
{
	AssertEQ(Parent->Type, UI_TYPE_CONTAINER);
	UIContainer* Container = &Parent->Container;

	AssertNotNull(Element->Callback);
	Element->Callback(Element, UI_EVENT_BEFORE_UPDATE);


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
			Element->EndX = Parent->EndX - Parent->W * 0.5f
				+ Element->W * 0.5f + Element->EndMarginLeft + OffsetMinX;
			OffsetMinX += Element->TotalW;
			break;
		}

		case UI_ALIGN_CENTER:
		{
			Element->EndX = Parent->EndX;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Element->EndX = Parent->EndX + Parent->W * 0.5f
				-(Element->W * 0.5f + Element->EndMarginRight + OffsetMaxX);
			OffsetMaxX += Element->TotalW;
			break;
		}

		}


		switch(Element->AlignY)
		{

		case UI_ALIGN_TOP:
		{
			Element->EndY = Parent->EndY - Parent->H * 0.5f
				+ Element->H * 0.5f + Element->EndMarginTop + OffsetMinY;
			OffsetMinY += Element->TotalH;
			break;
		}

		case UI_ALIGN_MIDDLE:
		{
			Element->EndY = Parent->EndY;
			break;
		}

		case UI_ALIGN_BOTTOM:
		{
			Element->EndY = Parent->EndY + Parent->H * 0.5f
				-(Element->H * 0.5f + Element->EndMarginBottom + OffsetMaxY);
			OffsetMaxY += Element->TotalH;
			break;
		}

		}


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

		break;
	}

	case UI_POSITION_INLINE:
	{
		switch(Element->AlignX)
		{

		case UI_ALIGN_LEFT:
		{
			Element->EndX = Parent->EndX - Parent->W * 0.5f
				+ Element->W * 0.5f + Element->EndMarginLeft;
			break;
		}

		case UI_ALIGN_CENTER:
		{
			Element->EndX = Parent->EndX;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Element->EndX = Parent->EndX + Parent->W * 0.5f
				-(Element->W * 0.5f + Element->EndMarginRight);
			break;
		}

		}


		switch(Element->AlignY)
		{

		case UI_ALIGN_TOP:
		{
			Element->EndY = Parent->EndY - Parent->H * 0.5f
				+ Element->H * 0.5f + Element->EndMarginTop;
			break;
		}

		case UI_ALIGN_MIDDLE:
		{
			Element->EndY = Parent->EndY;
			break;
		}

		case UI_ALIGN_BOTTOM:
		{
			Element->EndY = Parent->EndY + Parent->H * 0.5f
				-(Element->H * 0.5f + Element->EndMarginBottom);
			break;
		}

		}


		break;
	}

	case UI_POSITION_ABSOLUTE:
	{
		Element->EndX = Parent->EndX + Element->X;
		Element->EndY = Parent->EndY + Element->Y;
		break;
	}

	case UI_POSITION_RELATIVE:
	{
		AssertNotNull(Element->Relative);
		UIElement* Relative = Element->Relative;


		Element->EndX = Relative->EndX;

		switch(Element->AlignX)
		{

		case UI_ALIGN_LEFT:
		{
			Element->EndX -= Element->W * 0.5f + Element->EndMarginRight;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Element->EndX += Element->W * 0.5f + Element->EndMarginLeft;
			break;
		}

		default: break;

		}

		switch(Element->RelativeAlignX)
		{

		case UI_ALIGN_LEFT:
		{
			Element->EndX -= Relative->W * 0.5f;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Element->EndX += Relative->W * 0.5f;
			break;
		}

		default: break;

		}


		Element->EndY = Relative->EndY;

		switch(Element->AlignY)
		{

		case UI_ALIGN_TOP:
		{
			Element->EndY -= Element->H * 0.5f + Element->EndMarginBottom;
			break;
		}

		case UI_ALIGN_BOTTOM:
		{
			Element->EndY += Element->H * 0.5f + Element->EndMarginTop;
			break;
		}

		default: break;

		}

		switch(Element->RelativeAlignY)
		{

		case UI_ALIGN_TOP:
		{
			Element->EndY -= Relative->H * 0.5f;
			break;
		}

		case UI_ALIGN_BOTTOM:
		{
			Element->EndY += Relative->H * 0.5f;
			break;
		}

		default: break;

		}


		break;
	}

	default: AssertUnreachable();

	}


	UIPreClipCalcFunctions[Element->Type](Element, Scrollable);


	Element->Callback(Element, UI_EVENT_DURING_UPDATE);


	UIClipExplicit(
		Element->EndX - Element->W * 0.5f - Element->BorderLeft,
		Element->EndX + Element->W * 0.5f + Element->BorderRight,
		Element->EndY - Element->H * 0.5f - Element->BorderTop,
		Element->EndY + Element->H * 0.5f + Element->BorderBottom,
		Border
		);

	UIAfterClip(Border);


	UIClipExplicit(
		Element->EndX - Element->W * 0.5f,
		Element->EndX + Element->W * 0.5f,
		Element->EndY - Element->H * 0.5f,
		Element->EndY + Element->H * 0.5f,
		Content
		);

	UIAfterClip(Content);


	UIPostClipCalcFunctions[Element->Type](Element,
		ContentClipEndX, ContentClipEndY,
		ContentClipEndW, ContentClipEndH,
		Opacity, Scrollable
		);


	if((Element->InteractiveBorder && BorderPass) || ContentPass)
	{
		float ClipMinX;
		float ClipMaxX;
		float ClipMinY;
		float ClipMaxY;

		if(Element->InteractiveBorder)
		{
			ClipMinX = BorderClipMinX;
			ClipMaxX = BorderClipMaxX;
			ClipMinY = BorderClipMinY;
			ClipMaxY = BorderClipMaxY;
		}
		else
		{
			ClipMinX = ContentClipMinX;
			ClipMaxX = ContentClipMaxX;
			ClipMinY = ContentClipMinY;
			ClipMaxY = ContentClipMaxY;
		}

		if(
			ClipMinX <= MouseX &&
			ClipMaxX >= MouseX &&
			ClipMinY <= MouseY &&
			ClipMaxY >= MouseY
			)
		{
			bool OldHovered = Element->Hovered;
			Element->Hovered = UIMouseOver(Element);

			if(OldHovered != Element->Hovered)
			{
				if(Element->Hovered)
				{
					Element->Callback(Element, UI_EVENT_MOUSE_IN);
				}
				else
				{
					Element->Callback(Element, UI_EVENT_MOUSE_OUT);
				}
			}

			if(Element->Hovered)
			{
				ElementUnderMouse = Element;

				if(!Element->ScrollPassthrough)
				{
					ScrollableUnderMouse = Element->Scrollable ? Element : NULL;
				}

				if(!Element->ClickPassthrough)
				{
					ClickableUnderMouse = Element->Clickable ? Element : NULL;
				}
			}
		}
		else
		{
			if(Element->Hovered)
			{
				Element->Callback(Element, UI_EVENT_MOUSE_OUT);
				Element->Hovered = false;
			}
		}
	}

	if(BorderPass)
	{
		UIDrawBorder(Element,
			BorderClipEndX, BorderClipEndY,
			BorderClipEndW, BorderClipEndH,
			Opacity
			);
	}

	if(ContentPass)
	{
		UIDrawFunctions[Element->Type](Element,
			ContentClipEndX, ContentClipEndY,
			ContentClipEndW, ContentClipEndH,
			Opacity, Scrollable
			);
	}


	Element->Callback(Element, UI_EVENT_AFTER_UPDATE);
}


Static void
UIDrawChildren(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIContainer* Container = &Element->Container;
	UIElement* Child = Container->Head;

	Container->OffsetMin = 0.0f;
	Container->OffsetMax = 0.0f;

	if(Element->Scrollable)
	{
		Scrollable = Element;
	}

	while(Child)
	{
		uint32_t ChildOpacity = AmulA(Opacity, Child->Opacity);
		if(ChildOpacity)
		{
			Depth += DRAW_DEPTH_LEAP;

			UIDrawElement(Child, ClipX, ClipY, ClipW, ClipH, ChildOpacity, Scrollable, Element);
		}

		Child = Child->Next;
	}
}


Static void
UIDrawContainer(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIContainer* Container = &Element->Container;

	/* Note that there's no sense in adding WhiteDepthDelta or such,
	 * because Depth is incremented on every element call, so two
	 * distinct containers will never have a chance for their colors
	 * to depth blend. It only makes sense for a lot of textures in
	 * the bounds of a single element draw call, like UIText.     */

	UIClipTexture(Element->EndX, Element->EndY, Element->W, Element->H,
		.WhiteColor = RGBmulA(Container->WhiteColor, Opacity),
		.WhiteDepth = Depth,
		.BlackColor = RGBmulA(Container->BlackColor, Opacity),
		.BlackDepth = Depth,
		.Texture = Container->Texture
		);

	UIDrawChildren(Element, ClipX, ClipY, ClipW, ClipH, Opacity, Scrollable);
}


Static void
UIDrawText(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIText* Text = &Element->Text;

	bool SelectionPossible = Element == SelectedTextElement;
	bool TextCursorPossible = SelectionPossible && Text->Editable &&
		(((LastDrawAt - LastCursorSetAt) / 500000000) & 1) == 0;
	uint32_t Count = 0;

	ARGB Stroke			= RGBmulA(Text->Stroke			, Opacity);
	ARGB InverseStroke	= RGBmulA(Text->InverseStroke	, Opacity);
	ARGB Fill			= RGBmulA(Text->Fill			, Opacity);
	ARGB InverseFill	= RGBmulA(Text->InverseFill		, Opacity);
	ARGB Background		= RGBmulA(Text->Background		, Opacity);

	float X;
	float Y;

	for(uint32_t LineIdx = 0; LineIdx < Text->LineCount; ++LineIdx)
	{
		const UITextLine* Line = Text->Lines[LineIdx];
		X = Element->EndX + Line->X;
		Y = Element->EndY + Line->Y;

		UIClip(X + Line->Width * 0.5f, Y - Text->FontSize * 0.3333f, Line->Width, Text->FontSize);

		if(Pass)
		{
			for(uint32_t i = 0; i < Line->Length; ++i)
			{
				const UITextGlyph* Data = &Line->Glyphs[i];

				float Top = Data->Top;
				float Left = Data->Left;
				float Stride = Data->Stride;
				float Size = Data->Size;

				float TexX = X + Size / 2 + Left;
				float TexY = Y + Size / 2 - Top;

				ARGB ActualFill;
				ARGB ActualStroke;

				bool Selected = SelectionPossible && Count >= SelectionStart.Idx && Count < SelectionEnd.Idx;
				bool TextCursor = TextCursorPossible && Count == SelectionStart.Idx && Count == SelectionEnd.Idx;
				if(Selected)
				{
					ActualFill = InverseFill;
					ActualStroke = InverseStroke;
				}
				else
				{
					ActualFill = Fill;
					ActualStroke = Stroke;
				}


				Depth += DRAW_DEPTH_JIFFIE;

				UIClipTexture(TexX, TexY, Size, Size,
					.WhiteColor = ActualFill,
					.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
					.BlackColor = ActualStroke,
					.BlackDepth = Depth,
					.Texture = Data->Texture
					);

				Depth -= DRAW_DEPTH_JIFFIE;


				if(Selected)
				{
					UIClipTexture(X + Stride * 0.5f, Y - Text->FontSize * 0.3333f, Stride, Text->FontSize,
						.WhiteColor = Background,
						.WhiteDepth = Depth - DRAW_DEPTH_JIFFIE,
						.Texture = TEXTURE_RECT
						);
				}
				else if(TextCursor)
				{
					UIClipTexture(X, Y - Text->FontSize * 0.4f, Text->FontSize * 0.15, Text->FontSize * 0.9f,
						.WhiteColor = Fill,
						.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE * 2,
						.BlackColor = Stroke,
						.BlackDepth = Depth + DRAW_DEPTH_JIFFIE * 2,
						.Texture = TEXTURE_TEXT_CURSOR
						);
				}

				X += Stride;
				++Count;

				continue;
			}

			bool TextCursor = TextCursorPossible && Count == SelectionStart.Idx && Count == SelectionEnd.Idx;
			if(TextCursor)
			{
				UIClipTexture(X, Y - Text->FontSize * 0.4f, Text->FontSize * 0.15, Text->FontSize * 0.9f,
					.WhiteColor = Fill,
					.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE * 2,
					.BlackColor = Stroke,
					.BlackDepth = Depth + DRAW_DEPTH_JIFFIE * 2,
					.Texture = TEXTURE_TEXT_CURSOR
					);
			}
		}
		else
		{
			Count = Line->Idx + Line->Length;
		}

		Count += Line->Separator;
	}
}


Static void
UIDrawCheckbox(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	AssertEQ(Element->W, Element->H);

	UICheckbox* Checkbox = &Element->Checkbox;

	UIClipTexture(Element->EndX, Element->EndY, Element->W, Element->H,
		.WhiteColor = RGBmulA(Checkbox->Background, Opacity),
		.WhiteDepth = Depth - DRAW_DEPTH_JIFFIE,
		.Texture = TEXTURE_RECT
		);

	UIClipTexture(Element->EndX, Element->EndY, Element->W, Element->H,
		.WhiteColor = RGBmulA(Checkbox->CheckYes, Opacity),
		.WhiteDepth = Depth,
		.Texture = Checkbox->Checked ? TEXTURE_CHECK_YES : TEXTURE_CHECK_NO
		);
}


Static void
UIDrawSlider(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UISlider* Slider = &Element->Slider;

	float Percentage = Slider->Value / (Slider->Sections - 1);

	float BgWidth;
	float BgHeight;
	float BgMinX;
	float BgMaxX;
	float BgMinY;
	float BgMaxY;

	float Diameter;

	float SliderX;
	float SliderY;

	float SW;
	float SH;
	float OX;
	float OY;

	if(Slider->Axis == UI_AXIS_HORIZONTAL)
	{
		BgWidth = Element->W - Element->H;
		BgHeight = Element->H;

		Diameter = BgHeight;

		float Half = BgWidth * 0.5f + Diameter * 0.25f;
		BgMinX = Element->EndX - Half;
		BgMaxX = Element->EndX + Half;
		BgMinY = Element->EndY;
		BgMaxY = Element->EndY;

		SliderX = Element->EndX + BgWidth * (Percentage - 0.5f);
		SliderY = Element->EndY;

		SW = 0.5f;
		SH = 1.0f;
		OX = 0.25f;
		OY = 0.5f;
	}
	else
	{
		BgWidth = Element->W;
		BgHeight = Element->H - Element->W;

		Diameter = BgWidth;

		float Half = BgHeight * 0.5f + Diameter * 0.25f;
		BgMinX = Element->EndX;
		BgMaxX = Element->EndX;
		BgMinY = Element->EndY - Half;
		BgMaxY = Element->EndY + Half;

		SliderX = Element->EndX;
		SliderY = Element->EndY + BgHeight * (Percentage - 0.5f);

		SW = 1.0f;
		SH = 0.5f;
		OX = 0.5f;
		OY = 0.25f;
	}

	ARGB Color = RGBmulA(Slider->Color, Opacity);
	ARGB BgColor = RGBmulA(Slider->BgColor, Opacity);


	switch(Slider->Type)
	{

	case UI_SLIDER_TYPE_NORMAL:
	{
		UIClipTexture(Element->EndX, Element->EndY, BgWidth, BgHeight,
			.WhiteColor = BgColor,
			.WhiteDepth = Depth,
			.Texture = TEXTURE_RECT
			);

		UIClipTextureExplicit(BgMinX, BgMinY, Diameter, Diameter, SW, SH, OX, OY,
			.WhiteColor = BgColor,
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTextureExplicit(BgMaxX, BgMaxY, Diameter, Diameter, SW, SH, 1.0f - OX, 1.0f - OY,
			.WhiteColor = BgColor,
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter, Diameter,
			.WhiteColor = Color,
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		break;
	}

	case UI_SLIDER_TYPE_BRIGHTNESS:
	{
		AssertEQ(Slider->Axis, UI_AXIS_HORIZONTAL);

		UIClipTexture(Element->EndX, Element->EndY, BgWidth, BgHeight,
			.WhiteColor = Color,
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CS_B
			);

		UIClipTextureExplicit(BgMinX, BgMinY, Diameter, Diameter, SW, SH, OX, OY,
			.WhiteColor = (ARGB){ .R = 0x00, .G = 0x00, .B = 0x00, .A = Opacity },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTextureExplicit(BgMaxX, BgMaxY, Diameter, Diameter, SW, SH, 1.0f - OX, 1.0f - OY,
			.WhiteColor = Color,
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter - 3.0f, Diameter - 3.0f,
			.WhiteColor = (ARGB){ .R = 0xFF, .G = 0xFF, .B = 0xFF, .A = Opacity },
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter, Diameter,
			.WhiteColor = (ARGB){ .R = 0x00, .G = 0x00, .B = 0x00, .A = Opacity },
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		break;
	}

	case UI_SLIDER_TYPE_OPACITY:
	{
		AssertEQ(Slider->Axis, UI_AXIS_HORIZONTAL);

		UIClipTexture(Element->EndX, Element->EndY, BgWidth, BgHeight,
			.WhiteColor = (ARGB){ .R = 0xFF, .G = 0xFF, .B = 0xFF, .A = Opacity },
			.WhiteDepth = Depth - DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CS_T
			);

		if(Opacity != 0xFF)
		{
			UIClipTextureExplicit(BgMaxX, BgMaxY, Diameter, Diameter, SW, SH, 1.0f - OX, 1.0f - OY,
				.WhiteColor = (ARGB){ .R = 0xFF, .G = 0xFF, .B = 0xFF, .A = Opacity },
				.WhiteDepth = Depth - DRAW_DEPTH_JIFFIE,
				.Texture = TEXTURE_CIRCLE_T
				);
		}

		UIClipTexture(Element->EndX, Element->EndY, BgWidth, BgHeight,
			.WhiteColor = Color,
			.WhiteDepth = Depth,
			.Texture = TEXTURE_T_MASK
			);

		UIClipTextureExplicit(BgMinX, BgMinY, Diameter, Diameter, SW, SH, OX, OY,
			.WhiteColor = (ARGB){ .R = 0xFF, .G = 0xFF, .B = 0xFF, .A = Opacity },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE_T
			);

		UIClipTextureExplicit(BgMaxX, BgMaxY, Diameter, Diameter, SW, SH, 1.0f - OX, 1.0f - OY,
			.WhiteColor = Color,
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter - 3.0f, Diameter - 3.0f,
			.WhiteColor = (ARGB){ .R = 0xFF, .G = 0xFF, .B = 0xFF, .A = Opacity },
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter, Diameter,
			.WhiteColor = (ARGB){ .R = 0x00, .G = 0x00, .B = 0x00, .A = Opacity },
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		break;
	}

	case UI_SLIDER_TYPE_RED:
	{
		AssertEQ(Slider->Axis, UI_AXIS_HORIZONTAL);

		UIClipTexture(Element->EndX, Element->EndY, BgWidth, BgHeight,
			.WhiteColor = (ARGB){ Color.ARGB | 0x00FF0000 },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CS_RED
			);

		UIClipTextureExplicit(BgMinX, BgMinY, Diameter, Diameter, SW, SH, OX, OY,
			.WhiteColor = (ARGB){ Color.ARGB & 0xFF00FFFF },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTextureExplicit(BgMaxX, BgMaxY, Diameter, Diameter, SW, SH, 1.0f - OX, 1.0f - OY,
			.WhiteColor = (ARGB){ Color.ARGB | 0x00FF0000 },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter - 3.0f, Diameter - 3.0f,
			.WhiteColor = (ARGB){ .R = 0xFF, .G = 0xFF, .B = 0xFF, .A = Opacity },
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter, Diameter,
			.WhiteColor = (ARGB){ .R = 0x00, .G = 0x00, .B = 0x00, .A = Opacity },
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		break;
	}

	case UI_SLIDER_TYPE_GREEN:
	{
		AssertEQ(Slider->Axis, UI_AXIS_HORIZONTAL);

		UIClipTexture(Element->EndX, Element->EndY, BgWidth, BgHeight,
			.WhiteColor = (ARGB){ Color.ARGB | 0x0000FF00 },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CS_GREEN
			);

		UIClipTextureExplicit(BgMinX, BgMinY, Diameter, Diameter, SW, SH, OX, OY,
			.WhiteColor = (ARGB){ Color.ARGB & 0xFFFF00FF },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTextureExplicit(BgMaxX, BgMaxY, Diameter, Diameter, SW, SH, 1.0f - OX, 1.0f - OY,
			.WhiteColor = (ARGB){ Color.ARGB | 0x0000FF00 },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter - 3.0f, Diameter - 3.0f,
			.WhiteColor = (ARGB){ .R = 0xFF, .G = 0xFF, .B = 0xFF, .A = Opacity },
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter, Diameter,
			.WhiteColor = (ARGB){ .R = 0x00, .G = 0x00, .B = 0x00, .A = Opacity },
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		break;
	}

	case UI_SLIDER_TYPE_BLUE:
	{
		AssertEQ(Slider->Axis, UI_AXIS_HORIZONTAL);

		UIClipTexture(Element->EndX, Element->EndY, BgWidth, BgHeight,
			.WhiteColor = (ARGB){ Color.ARGB | 0x000000FF },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CS_BLUE
			);

		UIClipTextureExplicit(BgMinX, BgMinY, Diameter, Diameter, SW, SH, OX, OY,
			.WhiteColor = (ARGB){ Color.ARGB & 0xFFFFFF00 },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTextureExplicit(BgMaxX, BgMaxY, Diameter, Diameter, SW, SH, 1.0f - OX, 1.0f - OY,
			.WhiteColor = (ARGB){ Color.ARGB | 0x000000FF },
			.WhiteDepth = Depth,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter - 3.0f, Diameter - 3.0f,
			.WhiteColor = (ARGB){ .R = 0xFF, .G = 0xFF, .B = 0xFF, .A = Opacity },
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		UIClipTexture(SliderX, SliderY, Diameter, Diameter,
			.WhiteColor = (ARGB){ .R = 0x00, .G = 0x00, .B = 0x00, .A = Opacity },
			.WhiteDepth = Depth + DRAW_DEPTH_JIFFIE,
			.Texture = TEXTURE_CIRCLE
			);

		break;
	}

	}
}


Static void
UIDrawScrollbar(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIScrollbar* Scrollbar = &Element->Scrollbar;

	UIElement* ScrollableElement = Element->Prev;
	AssertEQ(ScrollableElement->Type, UI_TYPE_CONTAINER);

	float Width;
	float Height;
	float Diameter;

	float SliderWidth;
	float SliderHeight;

	float SliderMinX;
	float SliderMaxX;
	float SliderX;

	float SliderMinY;
	float SliderMaxY;
	float SliderY;

	float SW;
	float SH;
	float OX;
	float OY;

	if(Scrollbar->Axis == UI_AXIS_HORIZONTAL)
	{
		float Total = 1.0f / ScrollableElement->W;
		float ViewMin = MAX(0.0f, Scrollbar->ViewMin * Total);
		float ViewMax = MIN(1.0f, Scrollbar->ViewMax * Total);

		float BgWidth = Element->W - Element->H;
		float BgHeight = Element->H;

		Width = (ViewMax - ViewMin) * BgWidth;
		Height = BgHeight;
		Diameter = BgHeight;

		SliderWidth = Width + BgHeight;
		SliderHeight = Height;

		SliderMinX = Element->EndX + BgWidth * (ViewMin - 0.5f);
		SliderMaxX = Element->EndX + BgWidth * (ViewMax - 0.5f);
		SliderX = (SliderMinX + SliderMaxX) * 0.5f;

		SliderMinY = Element->EndY;
		SliderMaxY = Element->EndY;
		SliderY = Element->EndY;

		SW = 0.5f;
		SH = 1.0f;
		OX = 0.25f;
		OY = 0.5f;
	}
	else
	{
		float Total = 1.0f / ScrollableElement->H;
		float ViewMin = MAX(0.0f, Scrollbar->ViewMin * Total);
		float ViewMax = MIN(1.0f, Scrollbar->ViewMax * Total);

		float BgWidth = Element->W;
		float BgHeight = Element->H - Element->W;

		Width = BgWidth;
		Height = (ViewMax - ViewMin) * BgHeight;
		Diameter = BgWidth;

		SliderWidth = Width;
		SliderHeight = Height + BgWidth;

		SliderMinX = Element->EndX;
		SliderMaxX = Element->EndX;
		SliderX = Element->EndX;

		SliderMinY = Element->EndY + BgHeight * (ViewMin - 0.5f);
		SliderMaxY = Element->EndY + BgHeight * (ViewMax - 0.5f);
		SliderY = (SliderMinY + SliderMaxY) * 0.5f;

		SW = 1.0f;
		SH = 0.5f;
		OX = 0.5f;
		OY = 0.25f;
	}


	if(Element->Hovered)
	{
		Scrollbar->Hovered = UIMouseOverSlider(
			Scrollbar->Axis,
			SliderX,
			SliderY,
			SliderWidth,
			SliderHeight
		);
	}
	else
	{
		Scrollbar->Hovered = false;
	}


	ARGB Color;

	if(Scrollbar->Held || Scrollbar->Hovered)
	{
		Color = RGBmulA(Scrollbar->AltColor, Opacity);
	}
	else
	{
		Color = RGBmulA(Scrollbar->Color, Opacity);
	}


	UIClipTexture(SliderX, SliderY, Width, Height,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_RECT
		);

	UIClipTextureExplicit(SliderMinX, SliderMinY, Diameter, Diameter, SW, SH, OX, OY,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_CIRCLE
		);

	UIClipTextureExplicit(SliderMaxX, SliderMaxY, Diameter, Diameter, SW, SH, 1.0f - OX, 1.0f - OY,
		.WhiteColor = Color,
		.WhiteDepth = Depth,
		.Texture = TEXTURE_CIRCLE
		);
}


Static void
UIDrawColorPicker(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	AssertEQ(Element->W, Element->H);

	UIColorPicker* ColorPicker = &Element->ColorPicker;

	float TextureW = Element->W * (1.0f - COLOR_PICKER_HANDLE_SIZE);
	float TextureH = Element->H * (1.0f - COLOR_PICKER_HANDLE_SIZE);
	float HandleW = Element->W * COLOR_PICKER_HANDLE_SIZE;
	float HandleH = Element->H * COLOR_PICKER_HANDLE_SIZE;

	float X = Element->EndX + TextureW * ColorPicker->Position.X;
	float Y = Element->EndY + TextureH * ColorPicker->Position.Y;

	UIClipTexture(Element->EndX, Element->EndY, TextureW, TextureH,
		.WhiteColor = (ARGB){ .R = 0xFF, .G = 0xFF, .B = 0xFF, .A = Opacity },
		.WhiteDepth = Depth - DRAW_DEPTH_JIFFIE,
		.Texture = TEXTURE_CS_HS
		);

	ARGB Color = ColorPicker->ColorRGB;
	UIClipTexture(X, Y, HandleW - 3.0f, HandleH - 3.0f,
		.WhiteColor = (ARGB){ .R = Color.R, .G = Color.G, .B = Color.B, .A = Opacity },
		.WhiteDepth = Depth,
		.Texture = TEXTURE_CIRCLE
		);

	UIClipTexture(X, Y, HandleW, HandleH,
		.WhiteColor = (ARGB){ .R = 0x00, .G = 0x00, .B = 0x00, .A = Opacity },
		.WhiteDepth = Depth,
		.Texture = TEXTURE_CIRCLE
		);
}


Static void
UIDrawTexture(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UITexture* Texture = &Element->Texture;

	UIClipTextureExplicit(Element->EndX, Element->EndY, Element->W, Element->H,
						  Texture->SW, Texture->SH, Texture->OX, Texture->OY,
		.WhiteColor = RGBmulA(Texture->WhiteColor, Opacity),
		.WhiteDepth = Depth,
		.BlackColor = RGBmulA(Texture->BlackColor, Opacity),
		.BlackDepth = Depth,
		.Texture = Texture->Texture,
		.Rotation = Texture->Rotation
		);
}


Static void
UIPreClipCalcContainer(
	UIElement* Element,
	UIElement* Scrollable
	)
{
	UIContainer* Container = &Element->Container;

	if(Element->Scrollable)
	{
		UIElement* Parent = Element->Parent;

		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			float ClampedGoalOffsetY = MIN(MAX(Container->GoalOffsetY, Parent->H - Element->H), 0);
			Container->GoalOffsetY = glm_lerpc(Container->GoalOffsetY, ClampedGoalOffsetY, 0.5f * DeltaTime);
			Container->OffsetY     = glm_lerpc(Container->OffsetY, Container->GoalOffsetY, 0.2f * DeltaTime);
		}
		else
		{
			float ClampedGoalOffsetX = MIN(MAX(Container->GoalOffsetX, Parent->W - Element->W), 0);
			Container->GoalOffsetX = glm_lerpc(Container->GoalOffsetX, ClampedGoalOffsetX, 0.5f * DeltaTime);
			Container->OffsetX     = glm_lerpc(Container->OffsetX, Container->GoalOffsetX, 0.2f * DeltaTime);
		}

		Element->EndX += Container->OffsetX;
		Element->EndY += Container->OffsetY;
	}
}


Static void
UIPreClipCalcText(
	UIElement* Element,
	UIElement* Scrollable
	)
{
	UIText* Text = &Element->Text;

	(void) Text;
}


Static void
UIPreClipCalcCheckbox(
	UIElement* Element,
	UIElement* Scrollable
	)
{
	UICheckbox* Checkbox = &Element->Checkbox;

	(void) Checkbox;
}


Static void
UIPreClipCalcSlider(
	UIElement* Element,
	UIElement* Scrollable
	)
{
	UISlider* Slider = &Element->Slider;

	(void) Slider;
}


Static void
UIPreClipCalcScrollbar(
	UIElement* Element,
	UIElement* Scrollable
	)
{
	UIScrollbar* Scrollbar = &Element->Scrollbar;

	(void) Scrollbar;

	UIScrollbarUpdate(Element);
}


Static void
UIPreClipCalcColorPicker(
	UIElement* Element,
	UIElement* Scrollable
	)
{
	UIColorPicker* ColorPicker = &Element->ColorPicker;

	(void) ColorPicker;
}


Static void
UIPreClipCalcTexture(
	UIElement* Element,
	UIElement* Scrollable
	)
{
	UITexture* Texture = &Element->Texture;

	(void) Texture;
}


Static void
UIPostClipCalcContainer(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIContainer* Container = &Element->Container;

	(void) Container;
}


Static void
UIPostClipCalcText(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIText* Text = &Element->Text;

	if(Scrollable && SelectedTextElement == Element)
	{
		UIContainer* Container = &Scrollable->Container;
		bool Moved = false;

		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			float StartY = Element->EndY - Element->H * 0.5f;
			float Y = StartY + SelectionTail.LineIdx * Text->FontSize;
			float D = Y - (ClipY - ClipH * 0.5f);
			if(D < 0)
			{
				Container->GoalOffsetY -= D * 0.33f;
				Moved = true;
			}

			Y = StartY + (SelectionTail.LineIdx + 1) * Text->FontSize;
			D = Y - (ClipY + ClipH * 0.5f);
			if(D > 0)
			{
				Container->GoalOffsetY -= D * 0.33f;
				Moved = true;
			}
		}
		else
		{
			float Scale = Text->FontSize / FONT_SIZE;
			float X = Element->EndX + Text->Lines[SelectionTail.LineIdx]->X + SelectionTail.Stride * Scale;

			float D = X - (ClipX - ClipW * 0.5f);
			if(D < 0)
			{
				Container->GoalOffsetX -= D * 0.33f;
				Moved = true;
			}

			D = X - (ClipX + ClipW * 0.5f);
			if(D > 0)
			{
				Container->GoalOffsetX -= D * 0.33f;
				Moved = true;
			}
		}

		if(Moved && SelectedElement == Element)
		{
			/* Bit of a workaround, can't call UITextOnMouseMove because it's not defined here yet */
			Element->Callback(Element, UI_EVENT_MOUSE_MOVE);
		}
	}
}


Static void
UIPostClipCalcCheckbox(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UICheckbox* Checkbox = &Element->Checkbox;

	(void) Checkbox;
}


Static void
UIPostClipCalcSlider(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UISlider* Slider = &Element->Slider;

	(void) Slider;
}


Static void
UIPostClipCalcScrollbar(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIScrollbar* Scrollbar = &Element->Scrollbar;

	(void) Scrollbar;
}


Static void
UIPostClipCalcColorPicker(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIColorPicker* ColorPicker = &Element->ColorPicker;

	(void) ColorPicker;
}


Static void
UIPostClipCalcTexture(
	UIElement* Element,
	float ClipX,
	float ClipY,
	float ClipW,
	float ClipH,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UITexture* Texture = &Element->Texture;

	(void) Texture;
}


Static void
UIDraw(
	void
	)
{
	ElementUnderMouse = Elements;
	ScrollableUnderMouse = NULL;

	DrawIndex = 0;
	Depth = DRAW_DEPTH_LEAP;

	UIDrawChildren(Elements, Elements->X, Elements->Y, Elements->W, Elements->H, 0xFF, NULL);

	AssertNotNull(ElementUnderMouse);

	if(ElementUnderMouse->Selectable)
	{
		WindowSetCursor(WINDOW_CURSOR_TYPING);
	}
	else if(ElementUnderMouse->Clickable)
	{
		WindowSetCursor(WINDOW_CURSOR_POINTING);
	}
	else
	{
		WindowSetCursor(WINDOW_CURSOR_DEFAULT);
	}
}





Static UITextOffset
UITextCalculateOffset(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	float Scale = Text->FontSize / FONT_SIZE;
	float HalfPadding = STROKE_THICKNESS * Scale;

	float Top = Element->EndY - Element->H * 0.5f;
	int32_t LineIdx = (MAX(MouseY, Top) - Top) / Text->FontSize;
	LineIdx = MIN(MAX(LineIdx, 0), Text->LineCount - 1);

	UITextLine* Line = Text->Lines[LineIdx];
	uint32_t Idx = Line->Idx;
	float X = MouseX - (Element->EndX + Line->X) + HalfPadding;
	X = MIN(MAX(X, 0.0f), Element->W - HalfPadding * 2.0f);

	uint32_t Total = X / Scale;
	uint32_t Original = Total;
	uint32_t CharIdx = 0;

	// for(uint32_t i = 0; i < Line->Length; ++i)
	// {
	// 	unsigned char Index = *(Char++) - ' ';
	// 	const FontDrawData* Data = FontData + Index;

	// 	if(Total > Data->Stride)
	// 	{
	// 		Total -= Data->Stride;
	// 		++CharIdx;
	// 	}
	// 	else
	// 	{
	// 		if(Total * 2 >= Data->Stride)
	// 		{
	// 			Total -= Data->Stride;
	// 			++CharIdx;
	// 		}

	// 		break;
	// 	}
	// }

	return
	(UITextOffset)
	{
		.LineIdx = LineIdx,
		.Offset = CharIdx,
		.Idx = Idx + CharIdx,
		.Stride = Original - Total
	};
}


Static void
UITextRecalculateOffset(
	UIElement* Element,
	UITextOffset* Selection
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	uint32_t Offset = MIN(MAX(Selection->Idx, 0), Text->Length);
	uint32_t LineIdx = 0;
	UITextLine* Line;

	while(1)
	{
		Line = Text->Lines[LineIdx];

		if(Offset >= Line->Length + Line->Separator && LineIdx != Text->LineCount - 1)
		{
			Offset -= Line->Length + Line->Separator;
			++LineIdx;
		}
		else
		{
			break;
		}
	}

	uint32_t Stride = 0;
	uint32_t CurIdx = Offset;
	// const char* Char = Line->Str;

	// while(CurIdx--)
	// {
	// 	AssertNEQ(*Char, 0);
	// 	unsigned char Index = *(Char++) - ' ';
	// 	const FontDrawData* Data = FontData + Index;

	// 	Stride += Data->Stride;
	// }

	Selection->LineIdx = LineIdx;
	Selection->Offset = Offset;
	Selection->Stride = Stride;
}


Static void
UITextAddMod(
	uint32_t Start,
	uint32_t End,
	const char* Str,
	uint32_t Length
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Editable);


	UITextMod* Mod = Text->Mods + Text->CurrentMod;
	UITextMod* ModEnd = Text->Mods + Text->ModCount;

	while(Mod != ModEnd)
	{
		UITextFreeMod(Mod++);
	}


	/*uint32_t OldLength = End - Start;
	char* OldStr = malloc(OldLength + 1);
	AssertNotNull(OldStr);

	memcpy(OldStr, Text->Str + Start, OldLength);
	OldStr[OldLength] = 0;


	char* NewStr = malloc(Length + 1);
	AssertNotNull(NewStr);

	memcpy(NewStr, Str, Length);
	NewStr[Length] = 0;


	Text->Mods = realloc(Text->Mods, sizeof(*Text->Mods) * (Text->CurrentMod + 1));
	AssertNotNull(Text->Mods);

	Text->Mods[Text->CurrentMod++] =
	(UITextMod)
	{
		.Start = Start,

		.OldStr = OldStr,
		.OldLength = OldLength,

		.NewStr = NewStr,
		.NewLength = Length
	};

	Text->ModCount = Text->CurrentMod;*/
}


Static void
UITextCopy(
	void
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Selectable);

	/*char* EndChar = Text->Str + SelectionEnd.Idx;
	const char Char = *EndChar;
	*EndChar = 0;

	int Status = SDL_SetClipboardText(Text->Str + SelectionStart.Idx);
	AssertEQ(Status, 0);

	*EndChar = Char;*/
}


Static void
UITextSelectAll(
	void
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Selectable);

	if(SelectionStart.Idx != 0)
	{
		SelectionStart.Idx = 0;
		UITextRecalculateOffset(SelectedTextElement, &SelectionStart);
		SelectionHead = SelectionStart;
	}

	if(SelectionEnd.Idx != Text->Length)
	{
		SelectionEnd.Idx = Text->Length;
		UITextRecalculateOffset(SelectedTextElement, &SelectionEnd);
		SelectionTail = SelectionEnd;
	}

	if(SelectionStart.Idx == SelectionEnd.Idx)
	{
		LastCursorSetAt = GetTime();
		LastSelection = SelectionEnd;
	}
}


Static void
UITextDelete(
	uint32_t Offset
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Editable);

	/*if(SelectionStart.Idx == SelectionEnd.Idx)
	{
		uint32_t Idx;

		if(Offset == -1)
		{
			if(SelectionStart.Idx == 0)
			{
				return;
			}

			Idx = SelectionStart.Idx - 1;

			--SelectionStart.Idx;
		}
		else
		{
			if(SelectionEnd.Idx == Text->Length)
			{
				return;
			}

			Idx = SelectionStart.Idx;
		}

		UITextAddMod(
			Idx,
			Idx + 1,
			NULL,
			0
			);

		memmove(
			Text->Str + Idx,
			Text->Str + Idx + 1,
			Text->Length - Idx
			);

		--Text->Length;
	}
	else
	{
		uint32_t Length = SelectionEnd.Idx - SelectionStart.Idx;

		UITextAddMod(
			SelectionStart.Idx,
			SelectionEnd.Idx,
			NULL,
			0
			);

		AssertEQ(Text->Str[Text->Length], 0);

		memmove(
			Text->Str + SelectionStart.Idx,
			Text->Str + SelectionEnd.Idx,
			Text->Length - SelectionEnd.Idx + 1
			);

		Text->Length -= Length;
	}

	AssertEQ(Text->Str[Text->Length], 0);

	UIUpdateElement(SelectedTextElement);
	UITextRecalculateOffset(SelectedTextElement, &SelectionStart);
	SelectionHead = SelectionTail = SelectionEnd = LastSelection = SelectionStart;*/
}


Static void
UITextExecMod(
	uint32_t Start,
	uint32_t End,
	const char* Str,
	uint32_t Length
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Editable);

	/*uint32_t CurLength = End - Start;
	int32_t DiffLength = Length - CurLength;

	if(DiffLength < 0)
	{
		memmove(
			Text->Str + End + DiffLength,
			Text->Str + End,
			Text->Length - End + 1
			);
	}

	Text->Str = realloc(Text->Str, Text->Length + DiffLength + 1);
	AssertNotNull(Text->Str);

	if(DiffLength >= 0)
	{
		memmove(
			Text->Str + End + DiffLength,
			Text->Str + End,
			Text->Length - End + 1
			);
	}

	memcpy(
		Text->Str + Start,
		Str,
		Length
		);

	Text->Length += DiffLength;
	AssertEQ(Text->Str[Text->Length], 0);

	UIUpdateElement(SelectedTextElement);

	uint32_t EndIdx = Start + Length;
	if(SelectionEnd.Idx != EndIdx)
	{
		SelectionEnd.Idx = EndIdx;
		UITextRecalculateOffset(SelectedTextElement, &SelectionEnd);
	}

	SelectionHead = SelectionTail = SelectionStart = LastSelection = SelectionEnd;
	LastCursorSetAt = GetTime();*/
}


Static void
UITextExecPrevMod(
	void
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Editable);

	if(!Text->CurrentMod)
	{
		return;
	}

	/*UITextMod* Mod = Text->Mods + --Text->CurrentMod;
	UITextExecMod(
		Mod->Start,
		Mod->Start + Mod->NewLength,
		Mod->OldStr,
		Mod->OldLength
		);*/
}


Static void
UITextExecNextMod(
	void
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Editable);

	if(Text->CurrentMod == Text->ModCount)
	{
		return;
	}

	/*UITextMod* Mod = Text->Mods + Text->CurrentMod++;
	UITextExecMod(
		Mod->Start,
		Mod->Start + Mod->OldLength,
		Mod->NewStr,
		Mod->NewLength
		);*/
}


Static void
UITextPasteExplicit(
	const char* OriginalStr,
	uint32_t Length
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Editable);

	/*if(!Length)
	{
		return;
	}

	if(Length > 1024 * 1024)
	{
		Length = 1024 * 1024;
	}

	char Str[Length];

	const char* OriginalChar = OriginalStr;
	const char* OriginalEnd = OriginalStr + Length;
	char* Char = Str;

	while(OriginalChar < OriginalEnd)
	{
		char Replacement = Text->CharFilter[(uint8_t) *(OriginalChar++)];
		if(Replacement)
		{
			*(Char++) = Replacement;
		}
	}

	Length = Char - Str;

	uint32_t AddedLength = Text->Length + Length - (SelectionEnd.Idx - SelectionStart.Idx);
	int32_t DiffLength = AddedLength - Text->MaxLength;
	if(DiffLength > 0)
	{
		Length -= DiffLength;

		if(!Length)
		{
			return;
		}
	}

	if(SelectionStart.Idx != SelectionEnd.Idx)
	{
		UITextDelete(-1);
	}
	AssertEQ(SelectionStart.Idx, SelectionEnd.Idx);

	void* New = realloc(Text->Str, Text->Length + Length + 1);
	if(!New)
	{
		return;
	}

	Text->Str = New;

	UITextAddMod(
		SelectionStart.Idx,
		SelectionStart.Idx,
		Str,
		Length
		);

	memmove(
		Text->Str + SelectionStart.Idx + Length,
		Text->Str + SelectionStart.Idx,
		Text->Length - SelectionStart.Idx + 1
		);

	memcpy(Text->Str + SelectionStart.Idx, Str, Length);

	Text->Length += Length;
	SelectionStart.Idx += Length;

	UIUpdateElement(SelectedTextElement);
	UITextRecalculateOffset(SelectedTextElement, &SelectionStart);
	SelectionHead = SelectionTail = SelectionEnd = LastSelection = SelectionStart;*/
}


Static void
UITextPaste(
	void
	)
{
	if(!SDL_HasClipboardText())
	{
		return;
	}

	char* Str = SDL_GetClipboardText();
	uint32_t Length = strlen(Str);

	if(!Str)
	{
		return;
	}

	UITextPasteExplicit(Str, Length);

	SDL_free(Str);
}


Static void
UITextMoveSelectionHorizontallyWholeWord(
	uint32_t Direction
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Selectable);

	/*if(Direction == -1)
	{
		uint32_t Idx = SelectionTail.Idx;

		if(Idx == 0)
		{
			return;
		}

		--Idx;

		if(Idx != 0 && (Text->Str[Idx] == ' ' || Text->Str[Idx] == '\n'))
		{
			--Idx;
		}

		while(Idx && Text->Str[Idx] != ' ' && Text->Str[Idx] != '\n') --Idx;

		SelectionTail.Idx = Idx + 1;
	}
	else
	{
		const char* Char = Text->Str + SelectionTail.Idx;

		if(!*Char)
		{
			return;
		}

		++Char;

		while(*Char && *Char != ' ' && *Char != '\n') ++Char;

		SelectionTail.Idx = Char - Text->Str;
	}

	UITextRecalculateOffset(SelectedTextElement, &SelectionTail);*/
}


Static void
UITextMoveSelectionHorizontally(
	uint32_t Direction
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Selectable);

	if(Direction == -1)
	{
		if(SelectionTail.Idx == 0)
		{
			return;
		}
	}
	else
	{
		if(SelectionTail.Idx == Text->Length)
		{
			return;
		}
	}

	SelectionTail.Idx += Direction;
	UITextRecalculateOffset(SelectedTextElement, &SelectionTail);
}


Static void
UITextMoveHorizontally(
	uint32_t Direction,
	int Select,
	int WholeWord
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Selectable);

	if(!Select && SelectionStart.Idx != SelectionEnd.Idx)
	{
		if(Direction == -1)
		{
			SelectionHead = SelectionTail = SelectionEnd = SelectionStart;
		}
		else
		{
			SelectionHead = SelectionTail = SelectionStart = SelectionEnd;
		}

		goto goto_set_cursor;
	}

	if(!WholeWord)
	{
		UITextMoveSelectionHorizontally(Direction);
	}
	else
	{
		UITextMoveSelectionHorizontallyWholeWord(Direction);
	}

	if(!Select)
	{
		SelectionHead = SelectionStart = SelectionEnd = SelectionTail;
	}
	else
	{
		if(SelectionHead.Idx <= SelectionTail.Idx)
		{
			SelectionStart = SelectionHead;
			SelectionEnd = SelectionTail;
		}
		else
		{
			SelectionStart = SelectionTail;
			SelectionEnd = SelectionHead;
		}
	}

	if(SelectionStart.Idx == SelectionEnd.Idx)
	{
		goto_set_cursor:
		LastCursorSetAt = GetTime();
	}

	LastSelection = SelectionTail;
}


Static void
UITextMoveSelectionVertically(
	uint32_t Direction
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Selectable);

	if(Direction == -1)
	{
		if(SelectionTail.LineIdx == 0)
		{
			SelectionTail.Idx = SelectionTail.Offset = SelectionTail.Stride = 0;
			LastSelection = SelectionTail;

			return;
		}
	}
	else
	{
		if(SelectionTail.LineIdx == Text->LineCount - 1)
		{
			UITextLine* Line = Text->Lines[SelectionTail.LineIdx];
			SelectionTail.Idx -= SelectionTail.Offset;
			SelectionTail.Idx += Line->Length;

			UITextRecalculateOffset(SelectedTextElement, &SelectionTail);
			LastSelection = SelectionTail;

			return;
		}
	}

	float Scale = Text->FontSize / FONT_SIZE;

	UITextLine* OriginLine = Text->Lines[LastSelection.LineIdx];
	UITextLine* LastLine = Text->Lines[SelectionTail.LineIdx];
	UITextLine* NewLine = Text->Lines[SelectionTail.LineIdx + Direction];
	int32_t Target =
		+ (uint32_t)(OriginLine->X / Scale)
		- (uint32_t)(NewLine->X / Scale)
		+ LastSelection.Stride;
	int32_t Total = 0;
	uint32_t Offset = 0;

	// const char* Char = NewLine->Str;
	// while(*Char)
	// {
	// 	unsigned char Index = *(Char++) - ' ';
	// 	const FontDrawData* Data = FontData + Index;

	// 	int32_t Old = Total;
	// 	Total += Data->Stride;
	// 	++Offset;

	// 	if(Total >= Target)
	// 	{
	// 		if(abs(Target - Old) < abs(Total - Target))
	// 		{
	// 			Total = Old;
	// 			--Offset;
	// 		}

	// 		break;
	// 	}
	// }

	if(Direction == -1)
	{
		SelectionTail.Idx = SelectionTail.Idx - SelectionTail.Offset - (NewLine->Length + NewLine->Separator - Offset);
	}
	else
	{
		SelectionTail.Idx = SelectionTail.Idx + (LastLine->Length + LastLine->Separator - SelectionTail.Offset) + Offset;
	}

	SelectionTail.LineIdx += Direction;
	SelectionTail.Offset = Offset;

	UITextRecalculateOffset(SelectedTextElement, &SelectionTail);
}


Static void
UITextMoveVertically(
	uint32_t Direction,
	int Select
	)
{
	UIText* Text = &SelectedTextElement->Text;
	AssertTrue(Text->Selectable);

	if(!Select && SelectionStart.Idx != SelectionEnd.Idx)
	{
		if(Direction == -1)
		{
			SelectionHead = SelectionTail = SelectionEnd = SelectionStart;
		}
		else
		{
			SelectionHead = SelectionTail = SelectionStart = SelectionEnd;
		}

		LastSelection = SelectionHead;
		goto goto_set_cursor;
	}

	UITextMoveSelectionVertically(Direction);

	if(!Select)
	{
		SelectionHead = SelectionStart = SelectionEnd = SelectionTail;
	}
	else
	{
		if(SelectionHead.Idx <= SelectionTail.Idx)
		{
			SelectionStart = SelectionHead;
			SelectionEnd = SelectionTail;
		}
		else
		{
			SelectionStart = SelectionTail;
			SelectionEnd = SelectionHead;
		}
	}

	if(SelectionStart.Idx == SelectionEnd.Idx)
	{
		goto_set_cursor:
		LastCursorSetAt = GetTime();
	}
}


Static void
UITextSubmit(
	void
	)
{
	SelectedTextElement->Callback(SelectedTextElement, UI_EVENT_SUBMIT);
	SelectedTextElement = NULL;
	WindowStopTyping();
}


void
UIFocusText(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);

	if(SelectedTextElement == Element)
	{
		return;
	}

	if(SelectedTextElement)
	{
		UITextSubmit();
	}

	UITextCallback(Element, UI_EVENT_MOUSE_DOWN);
}


void
UIScrollbarUpdate(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_SCROLLBAR);
	UIScrollbar* Scrollbar = &Element->Scrollbar;

	UIElement* ScrollableElement = Element->Prev;
	AssertEQ(ScrollableElement->Type, UI_TYPE_CONTAINER);
	UIContainer* Container = &ScrollableElement->Container;

	UIElement* Parent = Element->Parent;

	float Offset;
	float View;

	if(Container->Axis == UI_AXIS_HORIZONTAL)
	{
		Offset = -Container->OffsetX;
		View = Parent->W;
	}
	else
	{
		Offset = -Container->OffsetY;
		View = Parent->H;
	}

	Scrollbar->ViewMin = Offset;
	Scrollbar->ViewMax = Offset + View;
}


float
UIColorPickerUpdate(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_COLOR_PICKER);
	UIColorPicker* ColorPicker = &Element->ColorPicker;

	ColorPicker->ColorHSV = RGBtoHSV(ColorPicker->ColorRGB);
	float Value = ColorPicker->ColorHSV.V;
	ColorPicker->ColorHSV.V = 1.0f;
	ColorPicker->ColorRGB = HSVtoRGB(ColorPicker->ColorHSV);
	ColorPicker->Position = HSVtoXOY(ColorPicker->ColorHSV);

	return Value;
}


Static void
UIColorPickerUpdatePos(
	UIColorPicker* ColorPicker
	)
{
	ColorPicker->ColorHSV = XOYtoHSV(ColorPicker->Position);
	ColorPicker->ColorRGB = HSVtoRGB(ColorPicker->ColorHSV);
}





Static void
UITextOnMouseDown(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	if(!Text->Selectable)
	{
		return;
	}

	SelectedTextElement = Element;
	SelectionHead = SelectionTail =
		SelectionStart = SelectionEnd = LastSelection = UITextCalculateOffset(Element);

	if(Text->Editable)
	{
		WindowStartTyping();
		LastCursorSetAt = GetTime();
	}
}


Static void
UITextOnMouseMove(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	if(!Text->Selectable)
	{
		return;
	}

	if(SelectedTextElement != Element)
	{
		return;
	}

	SelectionTail = LastSelection = UITextCalculateOffset(Element);

	if(SelectionHead.Idx <= SelectionTail.Idx)
	{
		SelectionStart = SelectionHead;
		SelectionEnd = SelectionTail;
	}
	else
	{
		SelectionStart = SelectionTail;
		SelectionEnd = SelectionHead;
	}
}


void
UITextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_TEXT);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	{
		UITextOnMouseDown(Element);
		break;
	}

	case UI_EVENT_MOUSE_MOVE:
	{
		UITextOnMouseMove(Element);
		break;
	}

	case UI_EVENT_FREE:
	{
		UITextOnFree(Element);
		break;
	}

	default: break;

	}
}



Static void
UICheckboxOnMouseUp(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_CHECKBOX);
	UICheckbox* Checkbox = &Element->Checkbox;

	if(!SameElement)
	{
		return;
	}

	Checkbox->Checked ^= 1;

	Element->Callback(Element, UI_EVENT_CHANGE);
}


void
UICheckboxCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_CHECKBOX);


	switch(Event)
	{

	case UI_EVENT_MOUSE_UP:
	{
		UICheckboxOnMouseUp(Element);
		break;
	}

	default: break;

	}
}



Static void
UISliderOnMouseDown(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_SLIDER);
	UISlider* Slider = &Element->Slider;

	float Min;
	float Max;
	float Mouse;

	if(Slider->Axis == UI_AXIS_HORIZONTAL)
	{
		float HalfW = (Element->W - Element->H) * 0.5f;
		Min = Element->EndX - HalfW;
		Max = Element->EndX + HalfW;
		Mouse = MouseX;
	}
	else
	{
		float HalfH = (Element->H - Element->W) * 0.5f;
		Min = Element->EndY - HalfH;
		Max = Element->EndY + HalfH;
		Mouse = MouseY;
	}

	float Percentage = (MIN(MAX(Mouse, Min), Max) - Min) / (Max - Min);
	Slider->Value = nearbyintf(Percentage * (Slider->Sections - 1));

	Element->Callback(Element, UI_EVENT_CHANGE);
}


Static void
UISliderOnMouseMove(
	UIElement* Element
	)
{
	UISliderOnMouseDown(Element);
}


void
UISliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_SLIDER);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	{
		UISliderOnMouseDown(Element);
		break;
	}

	case UI_EVENT_MOUSE_MOVE:
	{
		UISliderOnMouseMove(Element);
		break;
	}

	default: break;

	}
}



Static void
UIScrollbarOnMouseDown(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_SCROLLBAR);
	UIScrollbar* Scrollbar = &Element->Scrollbar;

	UIElement* ScrollableElement = Element->Prev;
	AssertEQ(ScrollableElement->Type, UI_TYPE_CONTAINER);
	UIContainer* Container = &ScrollableElement->Container;

	float Offset;

	if(Scrollbar->Axis == UI_AXIS_HORIZONTAL)
	{
		float Total = 1.0f / Element->W;
		float ViewMin = MAX(0.0f, Scrollbar->ViewMin * Total);
		float ViewMax = MIN(1.0f, Scrollbar->ViewMax * Total);

		float BgWidth = Element->W - Element->H;

		float SliderX = Element->EndX + BgWidth * ((ViewMax + ViewMin) * 0.5f - 0.5f);

		Offset = (MouseX < SliderX ? 1.0f : -1.0f) * (Scrollbar->ViewMax - Scrollbar->ViewMin);
	}
	else
	{
		float Total = 1.0f / Element->H;
		float ViewMin = MAX(0.0f, Scrollbar->ViewMin * Total);
		float ViewMax = MIN(1.0f, Scrollbar->ViewMax * Total);

		float BgHeight = Element->H - Element->W;

		float SliderY = Element->EndY + BgHeight * ((ViewMax + ViewMin) * 0.5f - 0.5f);

		Offset = (MouseY < SliderY ? 1.0f : -1.0f) * (Scrollbar->ViewMax - Scrollbar->ViewMin);
	}

	Scrollbar->Held = Scrollbar->Hovered;

	if(!Scrollbar->Held)
	{
		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			Container->GoalOffsetY += Offset;
		}
		else
		{
			Container->GoalOffsetX += Offset;
		}
	}
	else
	{
		Scrollbar->Offset = Scrollbar->ViewMin;

		if(Container->Axis == UI_AXIS_HORIZONTAL)
		{
			Scrollbar->Mouse = MouseX;
		}
		else
		{
			Scrollbar->Mouse = MouseY;
		}
	}
}


Static void
UIScrollbarOnMouseUp(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_SCROLLBAR);
	UIScrollbar* Scrollbar = &Element->Scrollbar;

	Scrollbar->Held = false;
}


Static void
UIScrollbarOnMouseMove(
	UIElement* Element
	)
{
	AssertEQ(Element->Type, UI_TYPE_SCROLLBAR);
	UIScrollbar* Scrollbar = &Element->Scrollbar;

	UIElement* ScrollableElement = Element->Prev;
	AssertEQ(ScrollableElement->Type, UI_TYPE_CONTAINER);
	UIContainer* Container = &ScrollableElement->Container;

	UIElement* Parent = Element->Parent;

	if(!Scrollbar->Held)
	{
		return;
	}

	if(Container->Axis == UI_AXIS_HORIZONTAL)
	{
		float Scale = ScrollableElement->W / Parent->W;
		float Delta = (Scrollbar->Mouse - MouseX) * Scale;
		float Offset = -Scrollbar->Offset + Delta;
		Container->GoalOffsetX = MIN(MAX(Offset, Parent->W - ScrollableElement->W), 0);
	}
	else
	{
		float Scale = ScrollableElement->H / Parent->H;
		float Delta = (Scrollbar->Mouse - MouseY) * Scale;
		float Offset = -Scrollbar->Offset + Delta;
		Container->GoalOffsetY = MIN(MAX(Offset, Parent->H - ScrollableElement->H), 0);
	}
}


void
UIScrollbarCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_SCROLLBAR);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	{
		UIScrollbarOnMouseDown(Element);
		break;
	}

	case UI_EVENT_MOUSE_UP:
	{
		UIScrollbarOnMouseUp(Element);
		break;
	}

	case UI_EVENT_MOUSE_MOVE:
	{
		UIScrollbarOnMouseMove(Element);
		break;
	}

	default: break;

	}
}



Static void
UIColorPickerOnMouseDown(
	UIElement* Element
	)
{
	UIColorPicker* ColorPicker = &Element->ColorPicker;

	AssertEQ(Element->W, Element->H);

	float DiffX = MouseX - Element->EndX;
	float DiffY = MouseY - Element->EndY;
	float R = Element->W * 0.5f;

	float X;
	float Y;

	if(DiffX * DiffX + DiffY * DiffY > R * R)
	{
		float Angle = atan2f(DiffY, DiffX);
		X = Element->EndX + cosf(Angle) * R;
		Y = Element->EndY + sinf(Angle) * R;
	}
	else
	{
		X = MouseX;
		Y = MouseY;
	}

	X = (X - Element->EndX) / Element->W;
	Y = (Y - Element->EndY) / Element->W;

	ColorPicker->Position.X = MIN(MAX(X, -0.5f), 0.5f);
	ColorPicker->Position.Y = MIN(MAX(Y, -0.5f), 0.5f);

	UIColorPickerUpdatePos(ColorPicker);

	Element->Callback(Element, UI_EVENT_CHANGE);
}


Static void
UIColorPickerOnMouseMove(
	UIElement* Element
	)
{
	UIColorPickerOnMouseDown(Element);
}


void
UIColorPickerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	AssertEQ(Element->Type, UI_TYPE_COLOR_PICKER);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	{
		UIColorPickerOnMouseDown(Element);
		break;
	}

	case UI_EVENT_MOUSE_MOVE:
	{
		UIColorPickerOnMouseMove(Element);
		break;
	}

	default: break;

	}
}





void
WindowOnResize(
	float Width,
	float Height
	)
{
	Elements->W = Width;
	Elements->H = Height;
}


void
WindowOnFocus(
	void
	)
{
	puts("window focused");
}


void
WindowOnBlur(
	void
	)
{
	puts("window blur");
}


Static bool
UIKeyDown(
	int Key,
	int Mods,
	int Repeat
	)
{
	if(SelectedTextElement != NULL)
	{
		UIText* Text = &SelectedTextElement->Text;

		if(Mods & SDL_KMOD_CTRL)
		{
			switch(Key)
			{

			case SDLK_A:
			{
				UITextSelectAll();
				break;
			}

			case SDLK_C:
			{
				UITextCopy();
				break;
			}

			case SDLK_V:
			{
				if(Text->Editable)
				{
					UITextPaste();
				}

				break;
			}

			case SDLK_Z:
			{
				if(Text->Editable)
				{
					if(Mods & SDL_KMOD_SHIFT)
					{
						UITextExecNextMod();
					}
					else
					{
						UITextExecPrevMod();
					}
				}

				break;
			}

			case SDLK_LEFT:
			{
				UITextMoveHorizontally(-1, Mods & SDL_KMOD_SHIFT, 1);
				break;
			}

			case SDLK_RIGHT:
			{
				UITextMoveHorizontally(1, Mods & SDL_KMOD_SHIFT, 1);
				break;
			}

			default: break;

			}
		}

		switch(Key)
		{
			case SDLK_ESCAPE:
			{
				UITextSubmit();
				break;
			}

			default: break;
		}

		if(Text->Editable)
		{
			switch(Key)
			{

			case SDLK_RETURN:
			{
				if(Text->AllowNewline)
				{
					char Str[2] = { '\n', 0 };
					UITextPasteExplicit(Str, 1);
				}
				else
				{
					UITextSubmit();
				}

				break;
			}

			case SDLK_BACKSPACE:
			{
				UITextDelete(-1);
				break;
			}

			case SDLK_DELETE:
			{
				UITextDelete(0);
				break;
			}

			case SDLK_LEFT:
			{
				UITextMoveHorizontally(-1, Mods & SDL_KMOD_SHIFT, 0);
				break;
			}

			case SDLK_RIGHT:
			{
				UITextMoveHorizontally(1, Mods & SDL_KMOD_SHIFT, 0);
				break;
			}

			case SDLK_UP:
			{
				UITextMoveVertically(-1, Mods & SDL_KMOD_SHIFT);
				break;
			}

			case SDLK_DOWN:
			{
				UITextMoveVertically(1, Mods & SDL_KMOD_SHIFT);
				break;
			}

			default: break;

			}
		}
		else
		{
			switch(Key)
			{

			case SDLK_RETURN:
			{
				UITextSubmit();
				break;
			}

			case SDLK_LEFT:
			{
				UITextMoveHorizontally(-1, Mods & SDL_KMOD_SHIFT, 0);
				break;
			}

			case SDLK_RIGHT:
			{
				UITextMoveHorizontally(1, Mods & SDL_KMOD_SHIFT, 0);
				break;
			}

			case SDLK_UP:
			{
				UITextMoveVertically(-1, Mods & SDL_KMOD_SHIFT);
				break;
			}

			case SDLK_DOWN:
			{
				UITextMoveVertically(1, Mods & SDL_KMOD_SHIFT);
				break;
			}

			default: break;

			}
		}

		return true;
	}

	if(Repeat)
	{
		return true;
	}

	return false;
}


Static UIKey
UIMapKey(
	int Key
	)
{
	switch(Key)
	{

	case SDLK_UNKNOWN: return UI_KEY_UNKNOWN;
	case SDLK_RETURN: return UI_KEY_RETURN;
	case SDLK_ESCAPE: return UI_KEY_ESCAPE;
	case SDLK_BACKSPACE: return UI_KEY_BACKSPACE;
	case SDLK_TAB: return UI_KEY_TAB;
	case SDLK_SPACE: return UI_KEY_SPACE;
	case SDLK_EXCLAIM: return UI_KEY_EXCLAIM;
	case SDLK_DBLAPOSTROPHE: return UI_KEY_DBLAPOSTROPHE;
	case SDLK_HASH: return UI_KEY_HASH;
	case SDLK_DOLLAR: return UI_KEY_DOLLAR;
	case SDLK_PERCENT: return UI_KEY_PERCENT;
	case SDLK_AMPERSAND: return UI_KEY_AMPERSAND;
	case SDLK_APOSTROPHE: return UI_KEY_APOSTROPHE;
	case SDLK_LEFTPAREN: return UI_KEY_LEFTPAREN;
	case SDLK_RIGHTPAREN: return UI_KEY_RIGHTPAREN;
	case SDLK_ASTERISK: return UI_KEY_ASTERISK;
	case SDLK_PLUS: return UI_KEY_PLUS;
	case SDLK_COMMA: return UI_KEY_COMMA;
	case SDLK_MINUS: return UI_KEY_MINUS;
	case SDLK_PERIOD: return UI_KEY_PERIOD;
	case SDLK_SLASH: return UI_KEY_SLASH;
	case SDLK_0: return UI_KEY_0;
	case SDLK_1: return UI_KEY_1;
	case SDLK_2: return UI_KEY_2;
	case SDLK_3: return UI_KEY_3;
	case SDLK_4: return UI_KEY_4;
	case SDLK_5: return UI_KEY_5;
	case SDLK_6: return UI_KEY_6;
	case SDLK_7: return UI_KEY_7;
	case SDLK_8: return UI_KEY_8;
	case SDLK_9: return UI_KEY_9;
	case SDLK_COLON: return UI_KEY_COLON;
	case SDLK_SEMICOLON: return UI_KEY_SEMICOLON;
	case SDLK_LESS: return UI_KEY_LESS;
	case SDLK_EQUALS: return UI_KEY_EQUALS;
	case SDLK_GREATER: return UI_KEY_GREATER;
	case SDLK_QUESTION: return UI_KEY_QUESTION;
	case SDLK_AT: return UI_KEY_AT;
	case SDLK_LEFTBRACKET: return UI_KEY_LEFTBRACKET;
	case SDLK_BACKSLASH: return UI_KEY_BACKSLASH;
	case SDLK_RIGHTBRACKET: return UI_KEY_RIGHTBRACKET;
	case SDLK_CARET: return UI_KEY_CARET;
	case SDLK_UNDERSCORE: return UI_KEY_UNDERSCORE;
	case SDLK_GRAVE: return UI_KEY_GRAVE;
	case SDLK_A: return UI_KEY_A;
	case SDLK_B: return UI_KEY_B;
	case SDLK_C: return UI_KEY_C;
	case SDLK_D: return UI_KEY_D;
	case SDLK_E: return UI_KEY_E;
	case SDLK_F: return UI_KEY_F;
	case SDLK_G: return UI_KEY_G;
	case SDLK_H: return UI_KEY_H;
	case SDLK_I: return UI_KEY_I;
	case SDLK_J: return UI_KEY_J;
	case SDLK_K: return UI_KEY_K;
	case SDLK_L: return UI_KEY_L;
	case SDLK_M: return UI_KEY_M;
	case SDLK_N: return UI_KEY_N;
	case SDLK_O: return UI_KEY_O;
	case SDLK_P: return UI_KEY_P;
	case SDLK_Q: return UI_KEY_Q;
	case SDLK_R: return UI_KEY_R;
	case SDLK_S: return UI_KEY_S;
	case SDLK_T: return UI_KEY_T;
	case SDLK_U: return UI_KEY_U;
	case SDLK_V: return UI_KEY_V;
	case SDLK_W: return UI_KEY_W;
	case SDLK_X: return UI_KEY_X;
	case SDLK_Y: return UI_KEY_Y;
	case SDLK_Z: return UI_KEY_Z;
	case SDLK_LEFTBRACE: return UI_KEY_LEFTBRACE;
	case SDLK_PIPE: return UI_KEY_PIPE;
	case SDLK_RIGHTBRACE: return UI_KEY_RIGHTBRACE;
	case SDLK_TILDE: return UI_KEY_TILDE;
	case SDLK_DELETE: return UI_KEY_DELETE;
	case SDLK_PLUSMINUS: return UI_KEY_PLUSMINUS;
	case SDLK_CAPSLOCK: return UI_KEY_CAPSLOCK;
	case SDLK_F1: return UI_KEY_F1;
	case SDLK_F2: return UI_KEY_F2;
	case SDLK_F3: return UI_KEY_F3;
	case SDLK_F4: return UI_KEY_F4;
	case SDLK_F5: return UI_KEY_F5;
	case SDLK_F6: return UI_KEY_F6;
	case SDLK_F7: return UI_KEY_F7;
	case SDLK_F8: return UI_KEY_F8;
	case SDLK_F9: return UI_KEY_F9;
	case SDLK_F10: return UI_KEY_F10;
	case SDLK_F11: return UI_KEY_F11;
	case SDLK_F12: return UI_KEY_F12;
	case SDLK_PRINTSCREEN: return UI_KEY_PRINTSCREEN;
	case SDLK_SCROLLLOCK: return UI_KEY_SCROLLLOCK;
	case SDLK_PAUSE: return UI_KEY_PAUSE;
	case SDLK_INSERT: return UI_KEY_INSERT;
	case SDLK_HOME: return UI_KEY_HOME;
	case SDLK_PAGEUP: return UI_KEY_PAGEUP;
	case SDLK_END: return UI_KEY_END;
	case SDLK_PAGEDOWN: return UI_KEY_PAGEDOWN;
	case SDLK_RIGHT: return UI_KEY_RIGHT;
	case SDLK_LEFT: return UI_KEY_LEFT;
	case SDLK_DOWN: return UI_KEY_DOWN;
	case SDLK_UP: return UI_KEY_UP;
	case SDLK_NUMLOCKCLEAR: return UI_KEY_NUMLOCKCLEAR;
	case SDLK_KP_DIVIDE: return UI_KEY_KP_DIVIDE;
	case SDLK_KP_MULTIPLY: return UI_KEY_KP_MULTIPLY;
	case SDLK_KP_MINUS: return UI_KEY_KP_MINUS;
	case SDLK_KP_PLUS: return UI_KEY_KP_PLUS;
	case SDLK_KP_ENTER: return UI_KEY_KP_ENTER;
	case SDLK_KP_1: return UI_KEY_KP_1;
	case SDLK_KP_2: return UI_KEY_KP_2;
	case SDLK_KP_3: return UI_KEY_KP_3;
	case SDLK_KP_4: return UI_KEY_KP_4;
	case SDLK_KP_5: return UI_KEY_KP_5;
	case SDLK_KP_6: return UI_KEY_KP_6;
	case SDLK_KP_7: return UI_KEY_KP_7;
	case SDLK_KP_8: return UI_KEY_KP_8;
	case SDLK_KP_9: return UI_KEY_KP_9;
	case SDLK_KP_0: return UI_KEY_KP_0;
	case SDLK_KP_PERIOD: return UI_KEY_KP_PERIOD;
	case SDLK_APPLICATION: return UI_KEY_APPLICATION;
	case SDLK_POWER: return UI_KEY_POWER;
	case SDLK_KP_EQUALS: return UI_KEY_KP_EQUALS;
	case SDLK_F13: return UI_KEY_F13;
	case SDLK_F14: return UI_KEY_F14;
	case SDLK_F15: return UI_KEY_F15;
	case SDLK_F16: return UI_KEY_F16;
	case SDLK_F17: return UI_KEY_F17;
	case SDLK_F18: return UI_KEY_F18;
	case SDLK_F19: return UI_KEY_F19;
	case SDLK_F20: return UI_KEY_F20;
	case SDLK_F21: return UI_KEY_F21;
	case SDLK_F22: return UI_KEY_F22;
	case SDLK_F23: return UI_KEY_F23;
	case SDLK_F24: return UI_KEY_F24;
	case SDLK_EXECUTE: return UI_KEY_EXECUTE;
	case SDLK_HELP: return UI_KEY_HELP;
	case SDLK_MENU: return UI_KEY_MENU;
	case SDLK_SELECT: return UI_KEY_SELECT;
	case SDLK_STOP: return UI_KEY_STOP;
	case SDLK_AGAIN: return UI_KEY_AGAIN;
	case SDLK_UNDO: return UI_KEY_UNDO;
	case SDLK_CUT: return UI_KEY_CUT;
	case SDLK_COPY: return UI_KEY_COPY;
	case SDLK_PASTE: return UI_KEY_PASTE;
	case SDLK_FIND: return UI_KEY_FIND;
	case SDLK_MUTE: return UI_KEY_MUTE;
	case SDLK_VOLUMEUP: return UI_KEY_VOLUMEUP;
	case SDLK_VOLUMEDOWN: return UI_KEY_VOLUMEDOWN;
	case SDLK_KP_COMMA: return UI_KEY_KP_COMMA;
	case SDLK_KP_EQUALSAS400: return UI_KEY_KP_EQUALSAS400;
	case SDLK_ALTERASE: return UI_KEY_ALTERASE;
	case SDLK_SYSREQ: return UI_KEY_SYSREQ;
	case SDLK_CANCEL: return UI_KEY_CANCEL;
	case SDLK_CLEAR: return UI_KEY_CLEAR;
	case SDLK_PRIOR: return UI_KEY_PRIOR;
	case SDLK_RETURN2: return UI_KEY_RETURN2;
	case SDLK_SEPARATOR: return UI_KEY_SEPARATOR;
	case SDLK_OUT: return UI_KEY_OUT;
	case SDLK_OPER: return UI_KEY_OPER;
	case SDLK_CLEARAGAIN: return UI_KEY_CLEARAGAIN;
	case SDLK_CRSEL: return UI_KEY_CRSEL;
	case SDLK_EXSEL: return UI_KEY_EXSEL;
	case SDLK_KP_00: return UI_KEY_KP_00;
	case SDLK_KP_000: return UI_KEY_KP_000;
	case SDLK_THOUSANDSSEPARATOR: return UI_KEY_THOUSANDSSEPARATOR;
	case SDLK_DECIMALSEPARATOR: return UI_KEY_DECIMALSEPARATOR;
	case SDLK_CURRENCYUNIT: return UI_KEY_CURRENCYUNIT;
	case SDLK_CURRENCYSUBUNIT: return UI_KEY_CURRENCYSUBUNIT;
	case SDLK_KP_LEFTPAREN: return UI_KEY_KP_LEFTPAREN;
	case SDLK_KP_RIGHTPAREN: return UI_KEY_KP_RIGHTPAREN;
	case SDLK_KP_LEFTBRACE: return UI_KEY_KP_LEFTBRACE;
	case SDLK_KP_RIGHTBRACE: return UI_KEY_KP_RIGHTBRACE;
	case SDLK_KP_TAB: return UI_KEY_KP_TAB;
	case SDLK_KP_BACKSPACE: return UI_KEY_KP_BACKSPACE;
	case SDLK_KP_A: return UI_KEY_KP_A;
	case SDLK_KP_B: return UI_KEY_KP_B;
	case SDLK_KP_C: return UI_KEY_KP_C;
	case SDLK_KP_D: return UI_KEY_KP_D;
	case SDLK_KP_E: return UI_KEY_KP_E;
	case SDLK_KP_F: return UI_KEY_KP_F;
	case SDLK_KP_XOR: return UI_KEY_KP_XOR;
	case SDLK_KP_POWER: return UI_KEY_KP_POWER;
	case SDLK_KP_PERCENT: return UI_KEY_KP_PERCENT;
	case SDLK_KP_LESS: return UI_KEY_KP_LESS;
	case SDLK_KP_GREATER: return UI_KEY_KP_GREATER;
	case SDLK_KP_AMPERSAND: return UI_KEY_KP_AMPERSAND;
	case SDLK_KP_DBLAMPERSAND: return UI_KEY_KP_DBLAMPERSAND;
	case SDLK_KP_VERTICALBAR: return UI_KEY_KP_VERTICALBAR;
	case SDLK_KP_DBLVERTICALBAR: return UI_KEY_KP_DBLVERTICALBAR;
	case SDLK_KP_COLON: return UI_KEY_KP_COLON;
	case SDLK_KP_HASH: return UI_KEY_KP_HASH;
	case SDLK_KP_SPACE: return UI_KEY_KP_SPACE;
	case SDLK_KP_AT: return UI_KEY_KP_AT;
	case SDLK_KP_EXCLAM: return UI_KEY_KP_EXCLAM;
	case SDLK_KP_MEMSTORE: return UI_KEY_KP_MEMSTORE;
	case SDLK_KP_MEMRECALL: return UI_KEY_KP_MEMRECALL;
	case SDLK_KP_MEMCLEAR: return UI_KEY_KP_MEMCLEAR;
	case SDLK_KP_MEMADD: return UI_KEY_KP_MEMADD;
	case SDLK_KP_MEMSUBTRACT: return UI_KEY_KP_MEMSUBTRACT;
	case SDLK_KP_MEMMULTIPLY: return UI_KEY_KP_MEMMULTIPLY;
	case SDLK_KP_MEMDIVIDE: return UI_KEY_KP_MEMDIVIDE;
	case SDLK_KP_PLUSMINUS: return UI_KEY_KP_PLUSMINUS;
	case SDLK_KP_CLEAR: return UI_KEY_KP_CLEAR;
	case SDLK_KP_CLEARENTRY: return UI_KEY_KP_CLEARENTRY;
	case SDLK_KP_BINARY: return UI_KEY_KP_BINARY;
	case SDLK_KP_OCTAL: return UI_KEY_KP_OCTAL;
	case SDLK_KP_DECIMAL: return UI_KEY_KP_DECIMAL;
	case SDLK_KP_HEXADECIMAL: return UI_KEY_KP_HEXADECIMAL;
	case SDLK_LCTRL: return UI_KEY_LCTRL;
	case SDLK_LSHIFT: return UI_KEY_LSHIFT;
	case SDLK_LALT: return UI_KEY_LALT;
	case SDLK_LGUI: return UI_KEY_LGUI;
	case SDLK_RCTRL: return UI_KEY_RCTRL;
	case SDLK_RSHIFT: return UI_KEY_RSHIFT;
	case SDLK_RALT: return UI_KEY_RALT;
	case SDLK_RGUI: return UI_KEY_RGUI;
	case SDLK_MODE: return UI_KEY_MODE;
	case SDLK_SLEEP: return UI_KEY_SLEEP;
	case SDLK_WAKE: return UI_KEY_WAKE;
	case SDLK_CHANNEL_INCREMENT: return UI_KEY_CHANNEL_INCREMENT;
	case SDLK_CHANNEL_DECREMENT: return UI_KEY_CHANNEL_DECREMENT;
	case SDLK_MEDIA_PLAY: return UI_KEY_MEDIA_PLAY;
	case SDLK_MEDIA_PAUSE: return UI_KEY_MEDIA_PAUSE;
	case SDLK_MEDIA_RECORD: return UI_KEY_MEDIA_RECORD;
	case SDLK_MEDIA_FAST_FORWARD: return UI_KEY_MEDIA_FAST_FORWARD;
	case SDLK_MEDIA_REWIND: return UI_KEY_MEDIA_REWIND;
	case SDLK_MEDIA_NEXT_TRACK: return UI_KEY_MEDIA_NEXT_TRACK;
	case SDLK_MEDIA_PREVIOUS_TRACK: return UI_KEY_MEDIA_PREVIOUS_TRACK;
	case SDLK_MEDIA_STOP: return UI_KEY_MEDIA_STOP;
	case SDLK_MEDIA_EJECT: return UI_KEY_MEDIA_EJECT;
	case SDLK_MEDIA_PLAY_PAUSE: return UI_KEY_MEDIA_PLAY_PAUSE;
	case SDLK_MEDIA_SELECT: return UI_KEY_MEDIA_SELECT;
	case SDLK_AC_NEW: return UI_KEY_AC_NEW;
	case SDLK_AC_OPEN: return UI_KEY_AC_OPEN;
	case SDLK_AC_CLOSE: return UI_KEY_AC_CLOSE;
	case SDLK_AC_EXIT: return UI_KEY_AC_EXIT;
	case SDLK_AC_SAVE: return UI_KEY_AC_SAVE;
	case SDLK_AC_PRINT: return UI_KEY_AC_PRINT;
	case SDLK_AC_PROPERTIES: return UI_KEY_AC_PROPERTIES;
	case SDLK_AC_SEARCH: return UI_KEY_AC_SEARCH;
	case SDLK_AC_HOME: return UI_KEY_AC_HOME;
	case SDLK_AC_BACK: return UI_KEY_AC_BACK;
	case SDLK_AC_FORWARD: return UI_KEY_AC_FORWARD;
	case SDLK_AC_STOP: return UI_KEY_AC_STOP;
	case SDLK_AC_REFRESH: return UI_KEY_AC_REFRESH;
	case SDLK_AC_BOOKMARKS: return UI_KEY_AC_BOOKMARKS;
	case SDLK_SOFTLEFT: return UI_KEY_SOFTLEFT;
	case SDLK_SOFTRIGHT: return UI_KEY_SOFTRIGHT;
	case SDLK_CALL: return UI_KEY_CALL;
	case SDLK_ENDCALL: return UI_KEY_ENDCALL;
	default: return UI_KEY_UNKNOWN;

	}
}


void
WindowOnKeyDown(
	int Key,
	int Mods,
	int Repeat
	)
{
	if(!UIKeyDown(Key, Mods, Repeat))
	{
		UIKey MappedKey = UIMapKey(Key);
		if(MappedKey != UI_KEY_UNKNOWN && !KeyState[MappedKey])
		{
			KeyState[MappedKey] = true;
			UIOnKeyDown(MappedKey);
		}
	}
}


Static void
UIKeyUp(
	int Key,
	int Mods
	)
{
	(void) 0;
}


void
WindowOnKeyUp(
	int Key,
	int Mods
	)
{
	UIKey MappedKey = UIMapKey(Key);
	if(MappedKey != UI_KEY_UNKNOWN && KeyState[MappedKey])
	{
		KeyState[MappedKey] = false;
		UIOnKeyUp(MappedKey);
	}
	else
	{
		UIKeyUp(Key, Mods);
	}
}


void
WindowOnText(
	const char* Text
	)
{
	AssertNotNull(SelectedTextElement);

	int Length = strlen(Text);
	if(!Length)
	{
		return;
	}

	UITextPasteExplicit(Text, Length);
}


Static UIButton
UIMapButton(
	int Button
	)
{
	switch(Button)
	{

	case SDL_BUTTON_LEFT: return UI_BUTTON_LEFT;
	case SDL_BUTTON_MIDDLE: return UI_BUTTON_MIDDLE;
	case SDL_BUTTON_RIGHT: return UI_BUTTON_RIGHT;
	case SDL_BUTTON_X1: return UI_BUTTON_X1;
	case SDL_BUTTON_X2: return UI_BUTTON_X2;
	default: return UI_BUTTON_UNKNOWN;

	}
}


Static bool
UIMouseDown(
	int Button
	)
{
	if(Button != SDL_BUTTON_LEFT)
	{
		return false;
	}

	SelectedElement = ClickableUnderMouse;

	if(
		SelectedTextElement &&
		ElementUnderMouse != SelectedTextElement &&
		!(SelectedElement && SelectedElement->TextFocus == SelectedTextElement)
		)
	{
		UITextSubmit();
	}

	if(!SelectedElement)
	{
		return false;
	}

	SelectedElement->Held = true;
	SelectedElement->Callback(SelectedElement, UI_EVENT_MOUSE_DOWN); // move this above and add a func to preventDefault to not submit text

	return true;
}


void
WindowOnMouseDown(
	int Button
	)
{
	if(!UIMouseDown(Button)) {
		UIButton MappedButton = UIMapButton(Button);
		if(MappedButton != UI_BUTTON_UNKNOWN && !ButtonState[MappedButton])
		{
			ButtonState[MappedButton] = true;
			UIOnMouseDown(MappedButton);
		}
	}
}


Static void
UIMouseUp(
	int Button
	)
{
	if(Button != SDL_BUTTON_LEFT)
	{
		return;
	}

	if(!SelectedElement)
	{
		return;
	}

	SameElement = ClickableUnderMouse == SelectedElement;
	SelectedElement->Callback(SelectedElement, UI_EVENT_MOUSE_UP);
	SelectedElement = NULL;
}


void
WindowOnMouseUp(
	int Button
	)
{
	UIButton MappedButton = UIMapButton(Button);
	if(MappedButton != UI_BUTTON_UNKNOWN && ButtonState[MappedButton])
	{
		ButtonState[MappedButton] = false;
		UIOnMouseUp(MappedButton);
	}
	else
	{
		UIMouseUp(Button);
	}
}


Static bool
UIMouseMove(
	float X,
	float Y
	)
{
	MouseX = X;
	MouseY = Y;

	if(SelectedElement)
	{
		SelectedElement->Callback(SelectedElement, UI_EVENT_MOUSE_MOVE);
		return true;
	}

	return false;
}


void
WindowOnMouseMove(
	float X,
	float Y
	)
{
	if(!UIMouseMove(X, Y)) {
		UIOnMouseMove(X, Y);
	}
}


Static bool
UIMouseScroll(
	float OffsetY
	)
{
	ScrollOffset = OffsetY;

	if(ScrollableUnderMouse)
	{
		ScrollableUnderMouse->Callback(ScrollableUnderMouse, UI_EVENT_SCROLL);
		return true;
	}

	return false;
}


void
WindowOnMouseScroll(
	float OffsetY
	)
{
	if(!UIMouseScroll(OffsetY))
	{
		UIOnMouseScroll(OffsetY);
	}
}


DrawData
VulkanGetDrawData(
	void
	)
{
	if(LastDrawAt == 0)
	{
		LastDrawAt = GetTime();
	}
	else
	{
		uint64_t Now = GetTime();
		uint64_t Delta = Now - LastDrawAt;
		LastDrawAt = Now;
		DeltaTime = (double) Delta / 16666666.6f;
	}

	UIDraw();

	return
	(DrawData)
	{
		.Input = DrawInput,
		.Count = DrawIndex
	};
}
