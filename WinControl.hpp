#pragma once

#include "Object.hpp"
#include "List.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "Control.hpp"

typedef struct{
    LPTSTR szTitle;
    DWORD Style;
    DWORD ExStyle;
	INT Left;
	INT Top;
	INT Width;
	INT Height;
	HWND WndParent;
    LPVOID lpParam;
    WNDCLASSEX WinClass;
	TCHAR WinClassName[64];
} TCreateParams;

typedef UINT TBorderWidth;
typedef BYTE TBevelCut;
#define bvNone		0x0
#define bvLowered	0x1
#define	bvRaised	0x2
#define bvSpace		0x3

typedef BYTE TBevelEdges; //TBevelEdges = set of TBevelEdge;
#define beLeft		0x1
#define beTop		0x2
#define beRight		0x4
#define beBottom	0x8

typedef BYTE TBevelKind;
#define bkNone		0x0
#define bkTile		0x1
#define bkSoft		0x2
#define bkFlat		0x3

typedef UINT TBevelWidth;

typedef void (CObject::*TKeyEvent)(CObject* Sender, WORD& Key, TShiftState Shift);
typedef void (CObject::*TKeyPressEvent)(CObject* Sender, TCHAR& Key);

class cVCL_API CWinControl : public CControl{
private:
	friend class CControl;
	WORD AlignLevel;
	TBevelEdges BevelEdges;
	TBevelCut BevelInner;
	TBevelCut BevelOuter;
    TBevelKind BevelKind;
	TBevelWidth BevelWidth;
    TBorderWidth BorderWidth;
	HWND Wnd;
	HWND ParentWindow;
	LPVOID lpWndProc;
	WNDPROC fnDefWndProc;
	BOOL TabStop;
	INT TabOrder;
	CList* Controls;
	CList* WinControls;
	CList* TabList;
	CBrush* Brush;
	BOOL Showing;
	BOOL Ctl3D;
	BOOL ParentCtl3D;
	BOOL DoubleBuffered;
	static LRESULT CALLBACK InitWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT StdWndProc(UINT message, WPARAM wParam, LPARAM lParam);

	void InvalidateFrame();
	void UpdateBounds();
	void RemoveFocus(BOOL Removing);
	void UpdateShowing();
	void UpdateTabOrder(INT Value);
	HWND PrecedingWindow(CWinControl* Control);
	void SetZOrderPosition(INT Position);

	void Insert(CControl* AControl);
	void Remove(CControl* AControl);
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_WINDOWPOSCHANGED, CWinControl::WMWindowPosChanged)
		MSG_MAP_ENTRY(CM_VISIBLECHANGED, CWinControl::CMVisibleChanged)
		MSG_MAP_ENTRY(CM_SHOWINGCHANGED, CWinControl::CMShowingChanged)
		MSG_MAP_ENTRY(CM_CONTROLLISTCHANGE, CWinControl::CMControlListChange)
		MSG_MAP_ENTRY(CM_SHOWHINTCHANGED, CWinControl::CMShowHintChanged)
		MSG_MAP_ENTRY(CM_BIDIMODECHANGED, CWinControl::CMBiDiModeChanged)
		MSG_MAP_ENTRY(CM_RECREATEWND, CWinControl::CMRecreateWnd)
		MSG_MAP_ENTRY(CM_FONTCHANGED, CWinControl::CMFontChanged)
		MSG_MAP_ENTRY(CM_DIALOGKEY, CWinControl::CMDialogKey)
		MSG_MAP_ENTRY(CM_DIALOGCHAR, CWinControl::CMDialogChar)
		MSG_MAP_ENTRY(WM_PAINT, CWinControl::WMPaint)
		MSG_MAP_ENTRY(WM_ERASEBKGND, CWinControl::WMEraseBkgnd)
		MSG_MAP_ENTRY(CM_COLORCHANGED, CWinControl::CMColorChanged)
		MSG_MAP_ENTRY(WM_CHAR, CWinControl::WMChar)
		MSG_MAP_ENTRY(CN_KEYDOWN, CWinControl::CNKeyDown)
		MSG_MAP_ENTRY(WM_SETCURSOR, CWinControl::WMSetCursor)
		MSG_MAP_ENTRY(CN_KEYUP, CWinControl::CNKeyUp)
		MSG_MAP_ENTRY(CN_CHAR, CWinControl::CNChar)
		MSG_MAP_ENTRY(CN_SYSKEYDOWN, CWinControl::CNSysKeyDown)
		MSG_MAP_ENTRY(CN_SYSCHAR, CWinControl::CNSysChar)
		MSG_MAP_ENTRY(WM_COMMAND, CWinControl::WMCommand)
		MSG_MAP_ENTRY(WM_NOTIFY, CWinControl::WMNotify)
		MSG_MAP_ENTRY(CM_ENABLEDCHANGED, CWinControl::CMEnabledChanged)
		MSG_MAP_ENTRY(WM_HSCROLL, CWinControl::WMHScroll)
		MSG_MAP_ENTRY(WM_VSCROLL, CWinControl::WMVScroll)
		MSG_MAP_ENTRY(WM_COMPAREITEM, CWinControl::WMCompareItem)
		MSG_MAP_ENTRY(WM_DELETEITEM, CWinControl::WMDeleteItem)
		MSG_MAP_ENTRY(WM_DRAWITEM, CWinControl::WMDrawItem)
		MSG_MAP_ENTRY(WM_MEASUREITEM, CWinControl::WMMeasureItem)
		MSG_MAP_ENTRY(CM_FOCUSCHANGED, CWinControl::CMFocusChanged)
		MSG_MAP_ENTRY(CM_ENTER, CWinControl::CMEnter)
		MSG_MAP_ENTRY(CM_EXIT, CWinControl::CMExit)
		MSG_MAP_ENTRY(CM_BORDERCHANGED, CWinControl::CMBorderChanged)
		MSG_MAP_ENTRY(CM_CURSORCHANGED, CWinControl::CMCursorChanged)
		MSG_MAP_ENTRY(WM_DESTROY, CWinControl::WMDestroy)
		MSG_MAP_ENTRY(WM_NCDESTROY, CWinControl::WMNCDestroy)
		MSG_MAP_ENTRY(CM_INVALIDATE, CWinControl::CMInvalidate)
		MSG_MAP_ENTRY(WM_SIZE, CWinControl::WMSize)
		MSG_MAP_ENTRY(WM_MOVE, CWinControl::WMMove)
		MSG_MAP_ENTRY(WM_CONTEXTMENU, CWinControl::WMContextMenu)
	MSG_MAP_END()
	void WMWindowPosChanged(TWMWindowPosChanged& Message);
	void CMVisibleChanged(TMessage& Message);
	void CMShowingChanged(TMessage& Message);
	void CMControlListChange(TMessage& Message);
	void CMShowHintChanged(TMessage& Message);
	void CMBiDiModeChanged(TMessage& Message);
	void CMRecreateWnd(TMessage& Message);
	void CMFontChanged(TMessage& Message);
	void CMDialogKey(TCMDialogKey& Message);
	void CMDialogChar(TCMDialogChar& Message);
	void WMPaint(TWMPaint& Message);
	void WMEraseBkgnd(TWMEraseBkgnd& Message);
	void CMColorChanged(TMessage& Message);
	void WMChar(TWMChar& Message);
	void WMSetCursor(TWMSetCursor& Message);
	void CNKeyDown(TWMKeyDown& Message);
	void CNKeyUp(TWMKeyUp& Message);
	void CNChar(TWMChar& Message);
	void CNSysKeyDown(TWMKeyDown& Message);
	void CNSysChar(TWMChar& Message);
	void WMCommand(TWMCommand& Message);
	void WMNotify(TWMNotify& Message);
	void CMEnabledChanged(TMessage& Message);
	void WMHScroll(TWMHScroll& Message);
    void WMVScroll(TWMVScroll& Message);
    void WMCompareItem(TWMCompareItem& Message);
    void WMDeleteItem(TWMDeleteItem& Message);
    void WMDrawItem(TWMDrawItem& Message);
    void WMMeasureItem(TWMMeasureItem& Message);
	void CMFocusChanged(TCMFocusChanged& Message);
	void CMEnter(TCMEnter& Message);
	void CMExit(TCMExit& Message);
	void CMBorderChanged(TMessage& Message);
	void CMCursorChanged(TMessage& Message);
	void WMDestroy(TWMDestroy& Message);
	void WMNCDestroy(TWMNCDestroy& Message);
	void CMInvalidate(TMessage& Message);
	void WMSize(TWMSize& Message);
	void WMMove(TWMMove& Message);
	void WMContextMenu(TWMContextMenu& Message);
protected:
	void WndProc(TMessage& Message) override;
	void DefaultHandler(TMessage& Message) override;

	virtual void CreateParams(TCreateParams& Params);
	void CreateSubClass(TCreateParams& Params, LPTSTR ControlClassName);
	virtual void CreateWindowHandle(const TCreateParams& Params);
	virtual void CreateWnd();
	virtual void ShowControl(CControl* AControl);
	void SetZOrder(BOOL TopMost) override;
	void DestroyHandle();
	virtual void DestroyWnd();
	virtual void DestroyWindowHandle();
	void RecreateWnd();

	void NotifyControls(UINT Msg);

	void PaintHandler(TWMPaint& Message);
	virtual void PaintWindow(HDC DC);
	void PaintControls(HDC DC, CControl* First);

	virtual void DoEnter();
	virtual void DoExit();

	BOOL IsControlMouseMsg(TWMMouse& Message);

	void SelectFirst();
	CWinControl* FindNextControl(CWinControl* CurControl, 
		BOOL GoForward, BOOL CheckTabStop, BOOL CheckParent);

	BOOL DoKeyDown(TWMKey& Message);
	BOOL DoKeyPress(TWMKey& Message);
	BOOL DoKeyUp(TWMKey& Message);
public:
	DECLARE_TYPE_EVENT(TKeyEvent, KeyDown)
	DECLARE_TYPE_EVENT(TKeyPressEvent, KeyPress)
	DECLARE_TYPE_EVENT(TKeyEvent, KeyUp)
	DECLARE_TYPE_EVENT(TNotifyEvent, Enter)
	DECLARE_TYPE_EVENT(TNotifyEvent, Exit)
public:
	CWinControl(CComponent* AOwner = NULL);
	virtual ~CWinControl();
	
	void DisableAlign();
	void EnableAlign();
	void Realign();
	void AlignControl(CControl* AControl);
	virtual void AlignControls(CControl* AControl, TRect& Rect);
	BOOL AlignWork();
	void DoAlign(CList& AlignList, CControl* AControl, TRect& Rect, UINT AAlign);
	virtual void ControlsAligned();
	BOOL InsertBefore(CControl* C1, CControl* C2, UINT AAlign);
	void DoPosition(CControl* Control, UINT AAlign, TAlignInfo& AlignInfo, TRect& Rect);
	virtual BOOL CustomAlignInsertBefore(CControl* C1, CControl* C2);
	virtual void CustomAlignPosition(CControl* Control, INT& NewLeft, INT& NewTop, INT& NewWidth, INT& NewHeight,
		TRect& AlignRect, const TAlignInfo& AlignInfo);

	void SetBounds(INT ALeft, INT ATop, INT AWidth, INT AHeight) override;
	void UpdateControlState();
	void Broadcast(TMessage& Message);

	BOOL ContainsControl(CControl* Control);

	HDC GetDeviceContext(HWND& WindowHandle) override;

	BOOL HandleAllocated();
	void HandleNeeded(); 
	virtual void CreateHandle();
	void Invalidate() override;
	void Repaint() override;

	virtual BOOL Focused();
	virtual void SetFocus();
	virtual BOOL CanFocus();
	virtual void GetTabOrderList(CList* List);

	void Update() override;
	void InsertControl(CControl* AControl);
	void RemoveControl(CControl* AControl);
	
	virtual void KeyDown(WORD& Key, TShiftState Shift);
	virtual void KeyPress(TCHAR& Key);
	virtual void KeyUp(WORD& Key, TShiftState Shift);

	CControl* ControlAtPos(const POINT& Pos, BOOL AllowDisabled, BOOL AllowWinControls = FALSE);
	TRect GetClientRect() override;
	TPoint GetClientOrigin() override;
	virtual void AdjustClientRect(TRect& Rect);

	INT GetControlCount();
	CControl* GetControl(INT Index);
	DEFINE_GETTER(CList*, Controls)
	DEFINE_GETTER(BOOL, Ctl3D)
	DEFINE_GETTER(BOOL, ParentCtl3D)
	DEFINE_GETTER(HWND, ParentWindow)
	DEFINE_ACCESSOR(HWND, Wnd)
	DEFINE_GETTER(BOOL, Showing)
	DEFINE_GETTER(BOOL, TabStop)
	DEFINE_ACCESSOR(BOOL, DoubleBuffered)
	DEFINE_GETTER(CBrush*, Brush)
	DEFINE_GETTER(TBevelEdges, BevelEdges)
	DEFINE_GETTER(TBevelCut, BevelInner)
	DEFINE_GETTER(TBevelCut, BevelOuter)
    DEFINE_GETTER(TBevelKind, BevelKind)
	DEFINE_GETTER(TBevelWidth, BevelWidth)
    DEFINE_GETTER(TBorderWidth, BorderWidth)

	void SetBevelInner(TBevelCut Value);
	void SetBevelOuter(TBevelCut Value);
	void SetBevelEdges(TBevelEdges Value);
    void SetBevelKind(TBevelKind Value);
    void SetBevelWidth(TBevelWidth Value);
    void SetBorderWidth(TBorderWidth Value);

	HWND GetHandle();
	LPVOID GetWndProc();

	VOID SetCtl3D(BOOL Value);
	VOID SetParentCtl3D(BOOL Value);
	VOID SetParentWindow(HWND Value);
	VOID SetTabStop(BOOL Value);
	INT GetTabOrder();
	VOID SetTabOrder(INT Value);
	BOOL GetParentBackground();
	virtual VOID SetParentBackground(BOOL Value);
	REF_DYN_CLASS(CWinControl)
};
DECLARE_DYN_CLASS(CWinControl, CControl)

class cVCL_API CCustomControl : public CWinControl{
private:
	CCanvas* Canvas;
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_PAINT, CCustomControl::WMPaint)
	MSG_MAP_END()
	void WMPaint(TWMPaint& Message);
protected:
    virtual void Paint();
	void PaintWindow(HDC DC) override;
public:
	CCustomControl(CComponent* AOwner = NULL);
	virtual ~CCustomControl();

	DEFINE_GETTER(CCanvas*, Canvas)

	REF_DYN_CLASS(CCustomControl)
};
DECLARE_DYN_CLASS(CCustomControl, CWinControl)

class cVCL_API CCustomListControl : public CWinControl{
public:
	CCustomListControl(CComponent* AOwner = NULL);
	virtual ~CCustomListControl();
    virtual void AddItem(LPTSTR Item, CObject* AObject) = 0;
	virtual void Clear() = 0;
	virtual void ClearSelection() = 0;
	virtual void CopySelection(CCustomListControl* Destination) = 0;
	virtual void DeleteSelected() = 0;
	virtual void MoveSelection(CCustomListControl* Destination);
	virtual void SelectAll() = 0;
	virtual INT GetItemIndex() = 0;
	virtual void SetItemIndex(INT Value) = 0;
	virtual INT GetCount() = 0;

	REF_DYN_CLASS(CCustomListControl)
};
DECLARE_DYN_CLASS_ABSTRACT(CCustomListControl, CWinControl)

class cVCL_API CCustomMultiSelectListControl : public CCustomListControl{
protected:
	BOOL MultiSelect;
public:
	CCustomMultiSelectListControl(CComponent* AOwner = NULL);
	virtual ~CCustomMultiSelectListControl();
	DEFINE_GETTER(BOOL, MultiSelect)
	virtual void SetMultiSelect(BOOL Value) = 0;
	virtual INT GetSelCount() = 0;

	REF_DYN_CLASS(CCustomMultiSelectListControl)
};
DECLARE_DYN_CLASS_ABSTRACT(CCustomMultiSelectListControl, CCustomListControl)