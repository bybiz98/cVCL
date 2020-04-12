#include "stdinc.h"
#include "Monitor.hpp"
#include "SysInit.hpp"
#include "WinUtils.hpp"
#include "Monitor.hpp"

IMPL_DYN_CLASS(CMonitor)
CMonitor::CMonitor():Handle(0),MonitorNum(0){
}

CMonitor::~CMonitor(){
}

INT CMonitor::GetLeft(){
	return GetBoundsRect().left;
}

INT CMonitor::GetHeight(){
	TRect r = GetBoundsRect();
	return r.bottom - r.top;
}

INT CMonitor::GetTop(){
	return GetBoundsRect().top;
}

INT CMonitor::GetWidth(){
	TRect r = GetBoundsRect();
	return r.right - r.left;
}

TRect CMonitor::GetBoundsRect(){
	MONITORINFO MonInfo;
	MonInfo.cbSize = sizeof(MonInfo);
	GetMonitorInfo(Handle, &MonInfo);
	return *((PRect)&(MonInfo.rcMonitor));
}

TRect CMonitor::GetWorkareaRect(){
	MONITORINFO MonInfo;
	MonInfo.cbSize = sizeof(MonInfo);
	GetMonitorInfo(Handle, &MonInfo);
	return *((PRect)&(MonInfo.rcWork));
}

BOOL CMonitor::GetPrimary(){
	MONITORINFO MonInfo;
	MonInfo.cbSize = sizeof(MonInfo);
	GetMonitorInfo(Handle, &MonInfo);
	return (MonInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;
}
