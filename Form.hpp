#pragma once

#include "stdinc.h"
#include "Object.hpp"
#include "Component.hpp"
#include "WinControl.hpp"

//TBorderIcons
typedef BYTE TBorderIcons;
#define biSystemMenu			0x1
#define biMinimize				0x2
#define biMaximize				0x4
#define biHelp					0x8

//TFormBorderStyle = (bsNone, bsSingle, bsSizeable, bsDialog, bsToolWindow,bsSizeToolWin);
typedef BYTE TFormBorderStyle;

typedef BYTE TWindowState;
#define wsNormal				0x0
#define wsMinimized				0x1
#define wsMaximized				0x2

typedef BYTE TShowAction;
#define saIgnore				0x0
#define saRestore				0x1
#define saMinimize				0x2
#define saMaximize				0x3

typedef BYTE TFormStyle;
#define fsNormal				0x0
#define fsMDIChild				0x1
#define fsMDIForm				0x2
#define fsStayOnTop				0x3

typedef BYTE TPosition;
#define poDesigned				0x0
#define poDefault				0x1
#define poDefaultPosOnly		0x2
#define poDefaultSizeOnly		0x3
#define poScreenCenter			0x4
#define poDesktopCenter			0x5
#define poMainFormCenter		0x6
#define poOwnerFormCenter		0x7

typedef BYTE TCloseAction;
#define caNone					0x1
#define	caHide					0x2
#define caFree					0x4
#define caMinimize				0x8

typedef BYTE TFormState;
#define fsCreating				0x1
#define fsVisible				0x2
#define fsShowing				0x4
#define fsModal					0x8
#define fsCreatedMDIChild		0x10
#define fsActivated				0x20

typedef void (CObject::*TCloseEvent)(CObject* Sender, TCloseAction& Action);
typedef void (CObject::*TCloseQueryEvent)(CObject* Sender, BOOL& CanClose);
typedef void (CObject::*TShortCutEvent)(TWMKey& Msg, BOOL& Handled);
typedef BOOL (CObject::*THelpEvent)(WORD Command, LONG Data, BOOL& CallHelp);

class cVCL_API CForm : public CWinControl {
private:
	CWinControl* ActiveControl;
	CWinControl* FocusedControl;
	BOOL Active;
	TBorderIcons BorderIcons;
	TFormBorderStyle BorderStyle;
	BOOL SizeChanging;
	TWindowState WindowState;
	TShowAction ShowAction;
	TFormStyle FormStyle;
	TPosition Position;
	CControlCanvas* Canvas;
	CIcon* Icon;
	TModalResult ModalResult;
	TFormState FormState;
	HWND ClientHandle;
	LPVOID lpClientWndProc;
	WNDPROC fnDefClientWndProc;
	CList* MDIChildren;
	BOOL KeyPreview;
	CWinControl* ActiveOleControl;

	void ClientWndProc(TMessage& Message);
	void IconChanged(CObject* Sender);
	HICON GetIconHandle();
	void SetLayeredAttribs();
	TColor NormalColor();
	void ModifySystemMenu();
	void SetWindowFocus();
	CForm* GetActiveMDIChild();
	void CloseModal();
protected:
		MSG_MAP_BEGIN()
			MSG_MAP_ENTRY(WM_CLOSE, CForm::WMClose)
			MSG_MAP_ENTRY(WM_NCCREATE, CForm::WMNCCreate)
			MSG_MAP_ENTRY(WM_NEXTDLGCTL, CForm::WMNextDlgCtl)
			MSG_MAP_ENTRY(WM_ACTIVATE, CForm::WMActivate)
			MSG_MAP_ENTRY(WM_SIZE, CForm::WMSize)
			MSG_MAP_ENTRY(CM_DIALOGKEY, CForm::CMDialogKey)
			MSG_MAP_ENTRY(CM_DEACTIVATE, CForm::CMDeactivate)
			MSG_MAP_ENTRY(CM_ACTIVATE, CForm::CMActivate)
			MSG_MAP_ENTRY(CM_SHOWINGCHANGED, CForm::CMShowingChanged)
		MSG_MAP_END()
	void WMClose(TWMClose& Message);
	void WMNCCreate(TWMNCCreate& Message);
	void WMNextDlgCtl(TWMNextDlgCtl& Message);
	void WMActivate(TWMActivate& Message);
	void WMSize(TWMSize& Message);
	void CMDialogKey(TCMDialogKey& Message);
	void CMDeactivate(TCMDeactivate& Message);
	void CMActivate(TCMActivate& Message);
	void CMShowingChanged(TMessage& Message);
protected:
	virtual void DoCreate();
	virtual void DoDestroy();
	virtual BOOL HandleCreateException();

	void CreateParams(TCreateParams& Params) override;
    void CreateWindowHandle(const TCreateParams& Params) override;
    void CreateWnd() override;
    void DestroyWindowHandle() override;
	void WndProc(TMessage& Message) override;
	virtual void Deactivate();
	virtual void DoClose(TCloseAction& Action);
	virtual void DoHide();
    virtual void DoShow();


	virtual void Paint();
	void PaintWindow(HDC DC) override;

	virtual void Activate();
	virtual void ActiveChanged();

	virtual void Resizing(TWindowState State);

	void SelectNext(CWinControl* CurControl, BOOL GoForward, BOOL CheckTabStop);
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Activate)
	DECLARE_TYPE_EVENT(TCloseEvent, Close)
	DECLARE_TYPE_EVENT(TCloseQueryEvent, CloseQuery)
	DECLARE_TYPE_EVENT(TNotifyEvent, Deactivate)
	DECLARE_TYPE_EVENT(THelpEvent, Help)
	DECLARE_TYPE_EVENT(TNotifyEvent, Hide)
	DECLARE_TYPE_EVENT(TNotifyEvent, Paint)
	DECLARE_TYPE_EVENT(TShortCutEvent, ShortCut)
	DECLARE_TYPE_EVENT(TNotifyEvent, Show)
	DECLARE_TYPE_EVENT(TNotifyEvent, Create)
	DECLARE_TYPE_EVENT(TNotifyEvent, Destroy)
public:
	CForm(CComponent* AOwner = NULL);
	virtual ~CForm();

	void MouseWheelHandler(TMessage& Message) override;

	void BringToFront();
	void SendToBack();
	void FocusControl(CWinControl* Control);
	void SetFocus() override;
	void SendCancelMode(CControl* Sender);

	void Close();
	virtual BOOL CloseQuery();
	void Hide();
	void Show();
	void Release();

	virtual INT ShowModal();
	void SetWindowToMonitor();
	
	INT GetMDIChildCount();
	CForm* GetMDIChildren(INT I);

	virtual BOOL WantChildKey(CControl* Child, TMessage& Message);
	void SetActive(BOOL Value);

	DEFINE_GETTER(TFormStyle, FormStyle)
	DEFINE_GETTER(TPosition, Position)
	DEFINE_GETTER(TWindowState, WindowState)
	DEFINE_GETTER(TBorderIcons, BorderIcons)
	DEFINE_GETTER(TFormBorderStyle, BorderStyle)
	DEFINE_GETTER(CWinControl*, ActiveControl)
	DEFINE_GETTER(HWND, ClientHandle)
	DEFINE_ACCESSOR(BOOL, KeyPreview)
	DEFINE_ACCESSOR(TModalResult, ModalResult)
	DEFINE_GETTER(BOOL, Active)
	DEFINE_GETTER(CCanvas*, Canvas)
	DEFINE_ACCESSOR(CWinControl*, ActiveOleControl)

	void SetFormStyle(TFormStyle Value);
	void SetPosition(TPosition Value);
	void SetWindowState(TWindowState Value);
	void SetBorderIcons(TBorderIcons Value);
	void SetBorderStyle(TFormBorderStyle Value);
	void SetActiveControl(CWinControl* Control);
	virtual BOOL SetFocusedControl(CWinControl* Control);

	REF_DYN_CLASS(CForm)
};
DECLARE_DYN_CLASS(CForm, CWinControl)

extern cVCL_API CForm* MainForm;