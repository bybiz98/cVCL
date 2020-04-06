
#include "stdinc.h"
#include "SysInit.hpp"

CGlobal* _Global = NULL;
CGlobal& GetGlobal(){
	if(_Global == NULL){
		_Global = new CGlobal();
	}
	return *_Global;
}

IMPL_DYN_CLASS(CGlobal)
CGlobal::CGlobal():MainInstance(0),
	HInstance(0),
	SysLocaleMiddleEast(FALSE),
	SysLocaleFastEast(FALSE),
	MouseWheelPresent(FALSE),
	RegMouseWheelMessage(0),
	NewStyleControls(TRUE),
	AppIcon(0),
	LineBreak(TEXT("\r\n")),
	_ExeptionHandler(NULL),
	HandleException(NULL),
	Win32Platform(0),
	Win32MajorVersion(0),
	Win32MinorVersion(0),
	Win32BuildNumber(0)
	{
	ZeroMemory(Win32CSDVersion, sizeof(Win32CSDVersion));
	RM_GetObjectInstance = InitMsg();
	RM_TaskbarCreated = RegisterWindowMessage(TEXT("TaskbarCreated"));
	InitPlatformId();
}

CGlobal::~CGlobal(){
}

void CGlobal::Initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPTSTR lpCmdLine, int nCmdShow){
	this->MainInstance = hInstance;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPTSTR)&GetGlobal, &(this->HInstance));
	if(this->HInstance == NULL){
		this->HInstance = hInstance;
	}
}
void CGlobal::Finalize(){

}

UINT CGlobal::InitMsg(){
	TCHAR Buf[255] = TEXT("ControlOfs%.8X%.8X");
	return RegisterWindowMessage(Buf);
}

INT CGlobal::InitPlatformId(){
	OSVERSIONINFO OSVersionInfo;
	OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVersionInfo);
	if(GetVersionEx(&OSVersionInfo)){
		Win32Platform = OSVersionInfo.dwPlatformId;
		Win32MajorVersion = OSVersionInfo.dwMajorVersion;
		Win32MinorVersion = OSVersionInfo.dwMinorVersion;
		if(Win32Platform == VER_PLATFORM_WIN32_WINDOWS)
			Win32BuildNumber = OSVersionInfo.dwBuildNumber & 0xFFFF;
		else
			Win32BuildNumber = OSVersionInfo.dwBuildNumber;
		lstrcpyn(Win32CSDVersion, OSVersionInfo.szCSDVersion, MAX_CSDVERSION_LEN);
	}
	else{
		throw "can not query window version information.";
	}
	return Win32Platform;
}

void CGlobal::CallHandleException(CObject* Sender){
	if(_ExeptionHandler != NULL && HandleException != NULL){
		(_ExeptionHandler->*(HandleException))(Sender);
	}
}
void CGlobal::SetExceptionHandler(CObject* Handler, THandleExceptionMethod Method){
	_ExeptionHandler = Handler;
	HandleException = Method;
}