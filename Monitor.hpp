#pragma once

#include "stdinc.h"
#include "Object.hpp"
#include "Types.hpp"

typedef BYTE TMonitorDefaultTo;
#define mdNearest		0x0
#define mdNull			0x1
#define mdPrimary		0x2

class cVCL_API CMonitor : public CObject{
private:
	HMONITOR Handle;
	INT MonitorNum;
public:
	CMonitor();
	virtual ~CMonitor();
	DEFINE_ACCESSOR(HMONITOR, Handle)
	DEFINE_ACCESSOR(INT, MonitorNum)
	INT GetLeft();
	INT GetHeight();
	INT GetTop();
	INT GetWidth();
	TRect GetBoundsRect();
	TRect GetWorkareaRect();
	BOOL GetPrimary();

	REF_DYN_CLASS(CMonitor)
};
DECLARE_DYN_CLASS(CMonitor, CObject)
