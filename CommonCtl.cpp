#include "stdinc.h"
#include "CommonCtl.hpp"
#include "WinControl.hpp"
#include "WinUtils.hpp"

void SetComCtlStyle(CWinControl* Ctl, INT Value, BOOL UseStyle){
	if(Ctl->HandleAllocated()){
		LONG_PTR Style = GetWindowLongPtr(Ctl->GetHandle(), GWL_STYLE);
		if(!UseStyle)
			Style &= ~Value;
		else 
			Style |= Value;
		SetWindowLongPtr(Ctl->GetHandle(), GWL_STYLE, Style);
	}
}

BOOL InitCommonControl(INT CC){
	INITCOMMONCONTROLSEX ICC;
	ZeroMemory(&ICC, sizeof(ICC));
	ICC.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ICC.dwICC = CC;
	BOOL Result = InitCommonControlsEx((const INITCOMMONCONTROLSEX*)&ICC);
	if(!Result)
		InitCommonControls();
	return Result;
}

INT IndexToOverlayMask(INT Index){
	return Index << 8;
}

LONG_PTR IndexToStateImageMask(LONG_PTR I){
	return I << 12;
}

INT ComCtlVersion = 0;
INT GetComCtlVersion(){
	if(ComCtlVersion == 0)
		ComCtlVersion = GetFileVersion(TEXT("comctl32.dll"));
	return ComCtlVersion;
}
