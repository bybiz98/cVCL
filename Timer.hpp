#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"

class cVCL_API CTimer : public CComponent{
private:
	UINT Interval;
	HWND  WindowHandle;
	BOOL Enabled;
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Timer);
private:
	void UpdateTimer();
	LRESULT WndProc(UINT Msg, WPARAM wParam, LPARAM lParam);
protected:
	virtual void Timer();
public:
	CTimer(CComponent* AOwner = NULL);
	virtual ~CTimer();
	
	void SetEnabled(BOOL Value);
	void SetInterval(UINT Value);
	DEFINE_GETTER(BOOL, Enabled)
	DEFINE_GETTER(UINT, Interval)

	REF_DYN_CLASS(CTimer)
};
DECLARE_DYN_CLASS(CTimer, CComponent)