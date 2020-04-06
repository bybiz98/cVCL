#include "stdinc.h"
#include <Strsafe.h>
#include "WinUtils.hpp"

DWORD PointToSmallPoint(POINT& pt){
	DWORD Result = 0;
	Result |= pt.x;
	Result |= pt.y << 16;
	return Result;
}

TSmallPoint PointToSmallPoint1(POINT& pt){
	TSmallPoint Result = {(SHORT)pt.x, (SHORT)pt.y};
	return Result;
}

TPoint SmallPointToPoint(TSmallPoint& pt){
	TPoint Ret;
	Ret.x = pt.x;
	Ret.y = pt.y;
	return Ret;
}

TRect Rect(INT Left, INT Top, INT Right, INT Bottom){
	TRect R;
	R.left = Left;
	R.top = Top;
	R.right = Right;
	R.bottom = Bottom;
	return R;
}

TPoint Point(INT X, INT Y){
	TPoint p;
	p.x = X;
	p.y = Y;
	return p;
}

TRect Bounds(INT Left, INT Top, INT Width, INT Height){
	TRect R;
	R.left = Left;
	R.top = Top;
	R.right = Left + Width;
	R.bottom = Top + Height;
	return R;
}

BOOL InvalidPoint(INT X, INT Y){
	return (X == -1) && (Y == -1);
}

TPoint CenterPoint(const TRect Rect){
	TPoint Result = {0, 0};
	Result.x = (Rect.right - Rect.left) / 2 + Rect.left;
	Result.y = (Rect.bottom - Rect.top) / 2 + Rect.top;
	return Result;
}

void MoveWindowOrg(HDC DC, INT DX, INT DY){
	TPoint P;
	GetWindowOrgEx(DC, &P);
	SetWindowOrgEx(DC, P.x - DX, P.y - DY, NULL);
}

VOID RaiseLastOSError(){
	LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process
	lstrlen((LPTSTR)lpMsgBuf);
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPTSTR)lpMsgBuf) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT(" failed with error %d: %s"), 
        dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
	//throw lpDisplayBuf;
     LocalFree(lpDisplayBuf);
}

BOOL Win32Check(BOOL RetVal){
	if(!RetVal)
		RaiseLastOSError();
	return RetVal;
}

LPVOID GlobalAllocPtr(UINT Flags, size_t Bytes){
	HGLOBAL H = GlobalAlloc(Flags, Bytes);
	return GlobalLock(H);
}

LPVOID GlobalReAllocPtr(LPVOID P, size_t Bytes, UINT Flags){
	HGLOBAL H = GlobalHandle(P);
	BOOL R = GlobalUnlock(H);
	H = GlobalReAlloc(H, Bytes, Flags);
	return GlobalLock(H);
}

HANDLE GlobalFreePtr(LPVOID P){
	HGLOBAL H = GlobalHandle(P);
	GlobalUnlock(H);
	return GlobalFree(H);
}

UINT GetFileVersion(LPTSTR AFileName){
	UINT Result = (UINT)-1;
	// GetFileVersionInfo modifies the filename parameter data while parsing.
	// Copy the string const into a local variable to create a writeable copy.
	String FileName(AFileName);
	DWORD Wnd;
	DWORD InfoSize = GetFileVersionInfoSize(FileName.GetBuffer(), &Wnd);
	if(InfoSize != 0){
		VS_FIXEDFILEINFO* VerBuf = (VS_FIXEDFILEINFO*)malloc(InfoSize);
		BufferHolder bufHolder(VerBuf);
		if(GetFileVersionInfo(FileName.GetBuffer(), Wnd, InfoSize, VerBuf)){
			VS_FIXEDFILEINFO* FI = NULL;
			UINT VerSize = 0;
			if(VerQueryValue(VerBuf, TEXT("\\"), (LPVOID *)&FI, (PUINT)&VerSize))
				Result = FI->dwFileVersionMS;
		}
	}
	return Result;
}

TPoint ClientToWindow(HWND Handle, INT X, INT Y){
	TRect Rect;
	TPoint Point;
	Point.x = X;
	Point.y = Y;
	ClientToScreen(Handle, &Point);
	GetWindowRect(Handle, (LPRECT)&Rect);
	Point.x = Point.x - Rect.left;
	Point.y = Point.y - Rect.top;
	return Point;
}

CCriticalSectionLock::CCriticalSectionLock(LPCRITICAL_SECTION cs){
	m_cs = cs;
	EnterCriticalSection(cs);
}

CCriticalSectionLock::~CCriticalSectionLock(){
	LeaveCriticalSection(m_cs);
}

CMethodLock::CMethodLock(CObject* obj, TLockMethod Lock, TLockMethod UnLock):
	Object(obj),
	Lock(Lock),
	LockRet(NULL),
	LockResult(NULL),
	UnLock(UnLock),
	TryLock(NULL),
	TryLockResult(FALSE){
	if(Lock != NULL)
		(obj->*Lock)();
}

CMethodLock::CMethodLock(CObject* obj, TLockMethodResult Lock, TLockMethod UnLock):
	Object(obj),
	Lock(NULL),
	LockRet(Lock),
	LockResult(NULL),
	UnLock(UnLock),
	TryLock(NULL),
	TryLockResult(FALSE){
	if(Lock != NULL)
		LockResult = (obj->*Lock)();
}
CMethodLock::CMethodLock(CObject* obj, TTryLockMethod ALock, TLockMethod UnLock):
	Object(obj),
	Lock(NULL),
	LockRet(NULL),
	LockResult(NULL),
	UnLock(UnLock),
	TryLock(ALock),
	TryLockResult(FALSE){
	if(ALock != NULL)
		TryLockResult = (obj->*ALock)();
}

LPVOID CMethodLock::GetLockResult(){
	return this->LockResult;
}

BOOL CMethodLock::GetTryLockResult(){
	return this->TryLockResult;
}
TLockMethod CMethodLock::SwapUnLock(TLockMethod NewUnLock){
	TLockMethod Old = this->UnLock;
	this->UnLock = NewUnLock;
	return Old;
}

CMethodLock::~CMethodLock(){
	if(UnLock != NULL && (TryLock == NULL || TryLockResult)){
		(Object->*UnLock)();
	}
}

CObjectHolder::CObjectHolder(CObject* Value):obj(Value){
}

CObject* CObjectHolder::SwapObject(CObject* Value){
	CObject* ret = this->obj;
	this->obj = Value;
	return ret;
}

CObjectHolder::~CObjectHolder(){
	if(this->obj != NULL){
		delete this->obj;
	}
}

CIntfObjectHolder::CIntfObjectHolder(IUnknown* Value):intf(Value){
}
CIntfObjectHolder::~CIntfObjectHolder(){
	if(this->intf != NULL){
		intf->Release();
	}
}
IUnknown* CIntfObjectHolder::SwapObject(IUnknown* Value){
	IUnknown* Ret = intf;
	intf = Value;
	return Ret;
}


DCHolder::DCHolder(HDC AValue, HANDLE ABackup, HWND ARefWnd):
	DC(AValue),
	Restore(ABackup),
	RefWnd(ARefWnd){
}

HDC DCHolder::SwapDC(HDC AValue){
	HDC ret = this->DC;
	this->DC = AValue;
	return ret;
}

HANDLE DCHolder::SwapRestore(HANDLE ABackup){
	HANDLE ret = this->Restore;
	this->Restore = ABackup;
	return ret;
}

HWND DCHolder::SwapRefWnd(HWND ARefWnd){
	HWND ret = this->RefWnd;
	this->RefWnd = ARefWnd;
	return ret;
}

DCHolder::~DCHolder(){
	if (this->DC != 0){
		if (this->Restore != 0) 
			SelectObject(this->DC, this->Restore);
		if(this->RefWnd != INVALID_HANDLE_VALUE)
			ReleaseDC(this->RefWnd, this->DC);
		else
			DeleteDC(this->DC);
	}
}

DCRestorer::DCRestorer(HDC ADC, INT ASaveIndex){
	DC = ADC;
	SaveIndex = ASaveIndex;
}

DCRestorer::~DCRestorer(){
	RestoreDC(DC, SaveIndex);
}

GDIOBJRestorer::GDIOBJRestorer(HDC AValue, HGDIOBJ ABackup){
	DC = AValue;
	Restore = ABackup;
}

HDC GDIOBJRestorer::SwapDC(HDC AValue){
	HDC Ret = DC;
	DC = AValue;
	return Ret;
}

HGDIOBJ GDIOBJRestorer::SwapRestore(HGDIOBJ ABackup){
	HGDIOBJ Ret = Restore;
	this->Restore = ABackup;
	return Ret;
}

GDIOBJRestorer::~GDIOBJRestorer(){
	if(DC != 0 && Restore != 0){
		SelectObject(DC, Restore);
	}
}

PALETTERestorer::PALETTERestorer(HDC AValue, HPALETTE ABackup, BOOL ABool){
	DC = AValue;
	Restore = ABackup;
	this->ABool = ABool;
}

HDC PALETTERestorer::SwapDC(HDC AValue){
	HDC Ret = DC;
	DC = AValue;
	return Ret;
}

HPALETTE PALETTERestorer::SwapRestore(HPALETTE ABackup){
	HPALETTE Ret = Restore;
	Restore = ABackup;
	return Ret;
}

PALETTERestorer::~PALETTERestorer(){
	if(DC != 0 && Restore != 0)
		SelectPalette(DC, Restore, ABool);
}

GDIOBJHolder::GDIOBJHolder(HGDIOBJ AObj):Object(AObj){
}

GDIOBJHolder::~GDIOBJHolder(){
	if(Object != 0)
		DeleteObject(Object);
}

HGDIOBJ GDIOBJHolder::SwapObject(HGDIOBJ AObj){
	HGDIOBJ Ret = Object;
	Object = AObj;
	return Ret;
}

BufferHolder::BufferHolder(LPVOID ABuffer):Buffer(ABuffer){
}

LPVOID BufferHolder::SwapBuffer(LPVOID ABuffer){
	LPVOID Result = this->Buffer;
	this->Buffer = ABuffer;
	return Result;
}

BufferHolder::~BufferHolder(){
	if(this->Buffer != NULL){
		free(this->Buffer);
		this->Buffer = NULL;
	}
}

CanvasHandleCleaner::CanvasHandleCleaner(CCanvas* ACanvas):Canvas(ACanvas){
}

CanvasHandleCleaner::~CanvasHandleCleaner(){
	if(Canvas != NULL)
		Canvas->SetHandle(0);
}

HKeyHolder::HKeyHolder(HKEY AKey){
	hKey = AKey;
}

HKeyHolder::~HKeyHolder(){
	RegCloseKey(hKey);
}