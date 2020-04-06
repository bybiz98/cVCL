#pragma once

#include "stdinc.h"
#include "Object.hpp"

typedef TNotifyEvent THandleExceptionMethod;

#define MAX_CSDVERSION_LEN			128

class cVCL_API CGlobal : public CObject{
private:
	CObject* _ExeptionHandler;
	THandleExceptionMethod HandleException;
	HINSTANCE MainInstance;
	HINSTANCE HInstance;
	UINT RM_GetObjectInstance;
	UINT RM_TaskbarCreated;
	BOOL SysLocaleMiddleEast;
	BOOL SysLocaleFastEast;
	INT Win32Platform;
	INT Win32MajorVersion;
	INT Win32MinorVersion;
	INT Win32BuildNumber;
	TCHAR Win32CSDVersion[MAX_CSDVERSION_LEN];
	BOOL MouseWheelPresent;
	UINT RegMouseWheelMessage;
	BOOL NewStyleControls;
	HICON AppIcon;
	LPTSTR LineBreak;

	UINT InitMsg();
	INT InitPlatformId();
public:
	CGlobal();
	virtual ~CGlobal();

	void Initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPTSTR lpCmdLine, int nCmdShow);
	void Finalize();

	void CallHandleException(CObject* Sender);
	void SetExceptionHandler(CObject* Handler, THandleExceptionMethod Method);

	DEFINE_GETTER(HINSTANCE, MainInstance)
	DEFINE_GETTER(HINSTANCE, HInstance)
	DEFINE_GETTER(UINT, RM_GetObjectInstance)
	DEFINE_GETTER(UINT, RM_TaskbarCreated)
	DEFINE_GETTER(BOOL, SysLocaleMiddleEast)
	DEFINE_GETTER(BOOL, SysLocaleFastEast)
	DEFINE_GETTER(INT, Win32Platform)
	DEFINE_GETTER(INT, Win32MajorVersion)
	DEFINE_GETTER(INT, Win32MinorVersion)
	DEFINE_GETTER(INT, Win32BuildNumber)
	DEFINE_GETTER(LPTSTR, Win32CSDVersion)
	DEFINE_ACCESSOR(BOOL, MouseWheelPresent)
	DEFINE_GETTER(UINT, RegMouseWheelMessage)
	DEFINE_GETTER(BOOL, NewStyleControls)
	DEFINE_ACCESSOR(HICON, AppIcon)
	DEFINE_GETTER(LPTSTR, LineBreak)

	REF_DYN_CLASS(CGlobal)
};
DECLARE_DYN_CLASS(CGlobal, CObject)

cVCL_API CGlobal& GetGlobal();