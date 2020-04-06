#pragma once

#include "stdinc.h"
#include "Types.hpp"
#include "Object.hpp"
#include "Graphics.hpp"

DWORD cVCL_API PointToSmallPoint(POINT& pt);

TSmallPoint cVCL_API PointToSmallPoint1(POINT& pt);

TPoint cVCL_API SmallPointToPoint(TSmallPoint& pt);

TPoint cVCL_API Point(INT X, INT Y);

TRect cVCL_API Rect(INT Left, INT Top, INT Right, INT Bottom);

TRect cVCL_API Bounds(INT Left, INT Top, INT Width, INT Height);

BOOL cVCL_API InvalidPoint(INT X, INT Y);

TPoint cVCL_API CenterPoint(const TRect Rect);

void cVCL_API MoveWindowOrg(HDC DC, INT DX, INT DY);

VOID cVCL_API RaiseLastOSError();

BOOL cVCL_API Win32Check(BOOL RetVal);

UINT cVCL_API GetFileVersion(LPTSTR AFileName);

TPoint cVCL_API ClientToWindow(HWND Handle, INT X, INT Y);

LPVOID cVCL_API GlobalAllocPtr(UINT Flags, size_t Bytes);

LPVOID cVCL_API GlobalReAllocPtr(LPVOID P, size_t Bytes, UINT Flags);

HANDLE cVCL_API GlobalFreePtr(LPVOID P);

class cVCL_API CCriticalSectionLock{
private:
	LPCRITICAL_SECTION m_cs;

public:
	CCriticalSectionLock(LPCRITICAL_SECTION cs);
	virtual ~CCriticalSectionLock();
};

typedef void (CObject::*TLockMethod)();
typedef LPVOID (CObject::*TLockMethodResult)();
typedef BOOL (CObject::*TTryLockMethod)();

class cVCL_API CMethodLock{
private:
	CObject* Object;
	TLockMethod Lock;
	TLockMethodResult LockRet;
	TTryLockMethod TryLock;
	TLockMethod UnLock;
	LPVOID LockResult;
	BOOL TryLockResult;
public:
	CMethodLock(CObject* obj, TLockMethod Lock, TLockMethod UnLock);
	CMethodLock(CObject* obj, TLockMethodResult Lock, TLockMethod UnLock);
	CMethodLock(CObject* obj, TTryLockMethod Lock, TLockMethod UnLock);
	virtual ~CMethodLock();
	LPVOID GetLockResult();
	BOOL GetTryLockResult();
	TLockMethod SwapUnLock(TLockMethod NewUnLock);
};

class cVCL_API CObjectHolder{
private:
	CObject* obj;
public:
	CObjectHolder(CObject* Value = NULL);
	virtual ~CObjectHolder();
	CObject* SwapObject(CObject* Value);
};

class cVCL_API CIntfObjectHolder{
private:
	IUnknown* intf;
public:
	CIntfObjectHolder(IUnknown* Value = NULL);
	virtual ~CIntfObjectHolder();
	IUnknown* SwapObject(IUnknown* Value);
};

class cVCL_API DCHolder{
private:
	HDC DC;
	HANDLE Restore;
	HWND RefWnd;
public:
	DCHolder(HDC AValue = 0, HANDLE ABackup = 0, HWND ARefWnd = (HWND)INVALID_HANDLE_VALUE);
	HDC SwapDC(HDC AValue);
	HANDLE SwapRestore(HANDLE ABackup);
	HWND SwapRefWnd(HWND ARefWnd);
	virtual ~DCHolder();
};

class cVCL_API DCRestorer{
private:
	HDC DC;
	INT SaveIndex;
public:
	DCRestorer(HDC ADC = 0, INT ASaveIndex = 0);
	virtual ~DCRestorer();
};

class cVCL_API GDIOBJRestorer{
private:
	HDC DC;
	HGDIOBJ Restore;
public:
	GDIOBJRestorer(HDC AValue = 0, HGDIOBJ ABackup = 0);
	HDC SwapDC(HDC AValue);
	HGDIOBJ SwapRestore(HGDIOBJ ABackup);
	~GDIOBJRestorer();
};

class cVCL_API PALETTERestorer{
private:
	HDC DC;
	HPALETTE Restore;
	BOOL ABool;
public:
	PALETTERestorer(HDC AValue = 0, HPALETTE ABackup = 0, BOOL ABool = FALSE);
	HDC SwapDC(HDC AValue);
	HPALETTE SwapRestore(HPALETTE ABackup);
	~PALETTERestorer();
};

class cVCL_API GDIOBJHolder{
private:
	HGDIOBJ Object;
public:
	GDIOBJHolder(HGDIOBJ AObj = 0);
	virtual ~GDIOBJHolder();
	HGDIOBJ SwapObject(HGDIOBJ AObj);
};

class cVCL_API BufferHolder{
private:
	LPVOID Buffer;
public:
	BufferHolder(LPVOID ABuffer = NULL);
	LPVOID SwapBuffer(LPVOID ABuffer);
	virtual ~BufferHolder();
};

class cVCL_API CanvasHandleCleaner{
private:
	CCanvas* Canvas;
public:
	CanvasHandleCleaner(CCanvas* ACanvas);
	virtual ~CanvasHandleCleaner();
};

class cVCL_API HKeyHolder{
private:
	HKEY hKey;
public:
	HKeyHolder(HKEY AKey);
	virtual ~HKeyHolder();
};