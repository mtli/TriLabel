#pragma once

#include <afxwin.h>

/******** Coded Settings *****************/

#define ST_MIN_LABEL_AREA		4
#define ST_RESIZEHANDLE_SIZE	8
#define ST_RESIZEHANDLE_COLOR	(RGB(0, 0, 0))

#define ST_SELBOX_COLOR			(RGB(20, 20, 20))
#define ST_MIN_BOX_SIZE			ST_RESIZEHANDLE_SIZE

#define ST_LABEL_COLORS \
	Gdiplus::Color(0, 255, 0), /* Green for Positive */ \
	Gdiplus::Color(255, 0, 0), /* Red for Negative */ \
	Gdiplus::Color(255, 255, 0), /* Yellow for Ambiguity */ \
	/* Inverting the color above */ \
	Gdiplus::Color(255, 0, 255), \
	Gdiplus::Color(0, 255, 255), \
	Gdiplus::Color(0, 0, 255)

#define ST_OUTPUT_MARKING		"/* TriLabel Format */"

#define ST_FDDB_FORMAT_EXT		""

/******** User Selectable Settings *******/
// TODO: provide interface
