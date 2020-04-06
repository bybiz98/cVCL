#pragma once

#include <CommCtrl.h>
#include "WinControl.hpp"

void SetComCtlStyle(CWinControl* Ctl, INT Value, BOOL UseStyle);

BOOL InitCommonControl(INT CC);

INT IndexToOverlayMask(INT Index);

LONG_PTR IndexToStateImageMask(LONG_PTR I);

INT GetComCtlVersion();

typedef PFNTVCOMPARE TTVCompare;
typedef TV_ITEM TTVItem;
typedef TVSORTCB TTVSortCB;
typedef TVINSERTSTRUCT TTVInsertStruct;

#define TVIS_FOCUSED 0x0001

#define ComCtlVersionIE3		0x00040046
#define ComCtlVersionIE4		0x00040047
#define ComCtlVersionIE401		0x00040048
#define ComCtlVersionIE5		0x00050050
#define ComCtlVersionIE501		0x00050051
#define ComCtlVersionIE6		0x00060000
