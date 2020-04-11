#pragma once

#include "stdinc.h"
#include "Object.hpp"
#include "Component.hpp"
#include "Strings.hpp"
#include "WinControl.hpp"
#include "Form.hpp"

#pragma pack (1)
typedef struct _CursorRec *PCursorRec;
typedef struct _CursorRec{
	PCursorRec Next;
    INT Index;
    HCURSOR Handle;
} TCursorRec;
#pragma pack ()

typedef BYTE TMonitorDefaultTo;
#define mdNearest		0x0
#define mdNull			0x1
#define mdPrimary		0x2

#define IDC_NODROP		MAKEINTRESOURCE(32767)
#define IDC_DRAG		MAKEINTRESOURCE(32766)
#define IDC_HSPLIT		MAKEINTRESOURCE(32765)
#define IDC_VSPLIT		MAKEINTRESOURCE(32764)
#define IDC_MULTIDRAG	MAKEINTRESOURCE(32763)
#define IDC_SQLWAIT		MAKEINTRESOURCE(32762)
#define IDC_HANDPT		MAKEINTRESOURCE(32761)

class CScreen;
CScreen* GetScreen();

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

class cVCL_API CScreen : public CComponent{
private:
	friend class CForm;
	CStrings* Fonts;
	CStrings* Imes;
	String* DefaultIme;
	HKL DefaultKbLayout;
	INT PixelsPerInch;
	TCursor Cursor;
	INT CursorCount;
	CList* Forms;
	CList* CustomForms;
	CList* DataModules;
	CList* Monitors;
	PCursorRec CursorList;
	HCURSOR DefaultCursor;
	CWinControl* ActiveControl;
	CForm* ActiveCustomForm;
	CForm* ActiveForm;
	CWinControl* LastActiveControl;
	CForm* LastActiveCustomForm;
	CForm* FocusedForm;
	CList* SaveFocusedList;
	CFont* HintFont;
	CFont* IconFont;
	CFont* MenuFont;
	WORD AlignLevel;
	TControlState ControlState;
	BOOL AlignWork();
	void DoAlign(CList* AlignList, CForm* AForm, TAlign AAlign, TRect& Rect);
	BOOL InsertBefore(CForm* C1, CForm* C2, TAlign AAlign);
	void DoPosition(CForm* Form, TAlign AAlign, TRect& Rect);
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, ActiveControlChange)
	DECLARE_TYPE_EVENT(TNotifyEvent, ActiveFormChange)
private:
	void AlignForm(CForm* AForm);
	void AlignForms(CForm* AForm, TRect& Rect);
	//void AddDataModule(CDataModule* DataModule);
	void CreateCursors();
	void DeleteCursor(INT Index);
	void DestroyCursors();
	CMonitor* FindMonitor(HMONITOR Handle);
    void IconFontChanged(CObject* Sender);
	INT GetCustomFormCount();
	CForm* GetCustomForms(INT Index);
	//TODO CDataModule* GetDataModule(INT Index);
	INT GetDataModuleCount();
	String GetDefaultIME();
	INT GetDesktopTop();
	INT GetDesktopLeft();
	INT GetDesktopHeight();
	INT GetDesktopWidth();
	TRect GetDesktopRect();
	TRect GetWorkAreaRect();
	INT GetWorkAreaHeight();
	INT GetWorkAreaLeft();
	INT GetWorkAreaTop();
	INT GetWorkAreaWidth();
	CStrings* GetImes();
	INT GetHeight();
	CMonitor* GetMonitor(INT Index);
	INT GetMonitorCount();
	CStrings* GetFonts();
	void GetMetricSettings();
	INT GetWidth();
	void InsertCursor(INT Index, HCURSOR Handle);
	//TODO void RemoveDataModule(CDataModule* DataModule);
	void SetCursors(INT Index, HCURSOR Handle);
	void SetCursor(TCursor Value);
	void SetHintFont(CFont* Value);
	void SetIconFont(CFont* Value);
	void SetMenuFont(CFont* Value);
	void UpdateLastActive();
public:
	CScreen(CComponent* AOwner = NULL);
	virtual ~CScreen();
	void DisableAlign();
	void EnableAlign();
	CMonitor* MonitorFromPoint(TPoint& Point, TMonitorDefaultTo MonitorDefault = mdNearest);
    CMonitor* MonitorFromRect(TRect& Rect, TMonitorDefaultTo MonitorDefault = mdNearest);
    CMonitor* MonitorFromWindow(HWND Handle, TMonitorDefaultTo MonitorDefault = mdNearest);
	void Realign();
	void ResetFonts();
	void AddForm(CForm* AForm);
	void RemoveForm(CForm* AForm);
	HCURSOR GetCursors(INT Index);
	INT GetFormCount();
	CForm* GetForm(INT Index);

	DEFINE_GETTER(CFont*, MenuFont)
	DEFINE_GETTER(TCursor, Cursor)
	DEFINE_GETTER(INT, CursorCount)
	DEFINE_GETTER(CList*, SaveFocusedList)
	DEFINE_ACCESSOR(CForm*, FocusedForm);
	DEFINE_GETTER(CForm*, ActiveForm);

	REF_DYN_CLASS(CScreen)
};
DECLARE_DYN_CLASS(CScreen, CComponent)
