#include "stdinc.h"
#include "SysInit.hpp"
#include "Timer.hpp"
#include "ObjWndProc.hpp"

IMPL_DYN_CLASS(CTimer)
CTimer::CTimer(CComponent* AOwner):CComponent(AOwner),
	Enabled(TRUE),
	Interval(1000),
	INIT_EVENT(Timer){
	WindowHandle = AllocateHWnd(GetGlobal().GetHInstance(), this, (TObjectWndProc)&CTimer::WndProc);
}

CTimer::~CTimer(){
	Enabled = FALSE;
	UpdateTimer();
	DeallocateHWnd(WindowHandle);
}

void CTimer::UpdateTimer(){
	KillTimer(WindowHandle, 1);
	if(Interval != 0 && Enabled && OnTimer != NULL)
		if(SetTimer(WindowHandle, 1, Interval, NULL) == 0)
			throw "out of resource no timers.";//raise EOutOfResources.Create(SNoTimers);
}

void CTimer::SetEnabled(BOOL Value){
	if(Value != Enabled){
		Enabled = Value;
		UpdateTimer();
	}
}

void CTimer::SetInterval(UINT Value){
	if(Value != Interval){
		Interval = Value;
		UpdateTimer();
	}
}

LRESULT CTimer::WndProc(UINT Msg, WPARAM wParam, LPARAM lParam){
	if(Msg == WM_TIMER){
      __try{
        Timer();
	  }
	  __except(EXCEPTION_EXECUTE_HANDLER){
		  GetGlobal().CallHandleException(this);
	  }
	}
	else
		return DefWindowProc(WindowHandle, Msg, wParam, lParam);
	return 0;
}

void CTimer::Timer(){
	if(OnTimer != NULL)
		CALL_EVENT(Timer)(this);
}