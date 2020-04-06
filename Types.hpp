#pragma once

#include "stdinc.h"

#pragma pack (1)
typedef POINT TPoint;
typedef POINTS TSmallPoint;
typedef union {
	struct{
		LONG    left;
		LONG    top;
		LONG    right;
		LONG    bottom;
	};
	struct{
		POINT TopLeft;
		POINT BottomRight;
	};
} TRect, *PRect;

typedef struct{
	UINT Module;
	INT Identifier;
}*PResStringRec, TResStringRec;

typedef SHORT TCursor;
#define crDefault		0x0
#define crNone			(-1)
#define crArrow			(-2)
#define crCross			(-3)
#define crIBeam			(-4)
#define crSize			(-22)
#define crSizeNESW		(-6)
#define crSizeNS		(-7)
#define crSizeNWSE		(-8)
#define crSizeWE		(-9)
#define crUpArrow		(-10)
#define crHourGlass		(-11)
#define crDrag			(-12)
#define crNoDrop		(-13)
#define crHSplit		(-14)
#define crVSplit		(-15)
#define crMultiDrag		(-16)
#define crSQLWait		(-17)
#define crNo			(-18)
#define crAppStart		(-19)
#define crHelp			(-20)
#define crHandPoint		(-21)
#define crSizeAll		(-22)

typedef WORD TOwnerDrawState;
#define	odSelected		0x0001
#define	odGrayed		0x0002
#define	odDisabled		0x0004
#define	odChecked		0x0008
#define	odFocused		0x0010
#define	odDefault		0x0020
#define	odHotLight		0x0040
#define	odInactive		0x0080
#define	odNoAccel		0x0100
#define	odNoFocusRect	0x0200
#define	odReserved1		0x0400
#define	odReserved2		0x0800
#define	odComboBoxEdit	0x1000

#pragma pack ()