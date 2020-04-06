#pragma once

#include "Object.hpp"
#include "List.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Strings.hpp"

typedef DWORD TControlStyle;
#define csAcceptsControls		0x00001
#define csCaptureMouse			0x00002
#define csDesignInteractive		0x00004
#define csClickEvents			0x00008
#define csFramed				0x00010
#define csSetCaption			0x00020
#define csOpaque				0x00040
#define csDoubleClicks			0x00080
#define csFixedWidth			0x00100
#define csFixedHeight			0x00200
#define csNoDesignVisible		0x00400
#define csReplicatable			0x00800
#define csNoStdEvents			0x01000
#define csDisplayDragImage		0x02000
#define csReflector				0x04000
#define csActionClient			0x08000
#define csMenuEvents			0x10000
#define csNeedsBorderPaint		0x20000
#define csParentBackground		0x40000

//ControlState
typedef WORD TControlState;
#define csLButtonDown			0x001
#define csClicked				0x002
#define csPalette				0x004
#define csReadingState			0x008
#define csAlignmentNeeded		0x010
#define csFocusing				0x020
#define csCreating				0x040
#define csPaintCopy				0x080
#define csCustomPaint			0x100
#define csDestroyingHandle		0x200
#define csDocking				0x400

//Align Type
typedef BYTE TAlign;
#define alNone					0x00
#define alTop					0x01
#define alBottom				0x02
#define alLeft					0x03
#define alRight					0x04
#define alClient				0x05
#define	alCustom				0x06

//AnchorKind
#define akLeft					0x1
#define akTop					0x2
#define akRight					0x4
#define akBottom				0x8

typedef BYTE TMouseButton;
#define mbLeft					0x1
#define mbRight					0x2
#define mbMiddle				0x4

//
typedef BYTE TShiftState;
#define ssShift					0x1
#define ssAlt					0x2
#define ssCtrl					0x4
#define ssLeft					0x8
#define ssRight					0x10
#define ssMiddle				0x20
#define ssDouble				0x40

typedef BYTE TAlignment;
#define taLeftJustify			0x0
#define taRightJustify			0x1
#define taCenter				0x2

typedef BYTE TLeftRight;
#define	taLeftJustify			0x0
#define	taRightJustify			0x1

//TBiDiMode 
#define bdLeftToRight				0x0
#define bdRightToLeft				0x1
#define	bdRightToLeftNoAlign		0x2
#define bdRightToLeftReadingOnly	0x3

typedef BYTE TBorderStyle;
#define bsNone					0x0
#define bsSingle				0x1
#define bsSizeable				0x2
#define	bsDialog				0x3
#define bsToolWindow			0x4
#define bsSizeToolWin			0x5

#define cHotkeyPrefix			TCHAR('&')

//TModalResult values
typedef INT TModalResult;
#define mrNone					0x0
#define mrOk					IDOK
#define mrCancel				IDCANCEL
#define mrAbort					IDABORT
#define mrRetry					IDRETRY
#define mrIgnore				IDIGNORE
#define mrYes					IDYES
#define mrNo					IDNO
#define mrAll					(mrNo + 1)
#define mrNoToAll				(mrAll + 1)
#define mrYesToAll				(mrNoToAll + 1)

typedef WORD THitTest,THitTests;
#define htAbove			0x0001
#define	htBelow			0x0002
#define htNowhere		0x0004
#define htOnItem		0x0008
#define htOnButton		0x0010
#define htOnIcon		0x0020
#define htOnIndent		0x0040
#define htOnLabel		0x0080
#define htOnRight		0x0100
#define htOnStateIcon	0x0200
#define htToLeft		0x0400
#define htToRight		0x0800

typedef BYTE TKeyboardState[256];
typedef TKeyboardState *PKeyboardState;

UINT AnchorAlign[];

typedef struct{
    CList* AlignList;
    UINT ControlIndex;
    UINT uAlign;
    UINT Scratch;
} TAlignInfo;

typedef UINT TBiDiMode;

class CControl;
class CWinControl;
class CForm;

CWinControl* FindVCLWindow(TPoint& Pos);
CControl* FindDragTarget(TPoint& Pos, BOOL AllowDisabled);
CWinControl* FindControl(HWND Handle);
CControl* GetCaptureControl();
void SetCaptureControl(CControl* Control);

CForm* GetParentForm(CControl* Control);
CForm* ValidParentForm(CControl* Control);

void ChangeBiDiModeAlignment(TAlignment& Alignment);

BYTE KeysToShiftState(WORD Keys);
TShiftState KeyDataToShiftState(LONG KeyData);
TShiftState KeyboardStateToShiftState(const TKeyboardState KeyboardState);
TShiftState KeyboardStateToShiftState();

BOOL IsAccel(WORD VK, LPTSTR Str);
TCHAR GetHotKey(LPTSTR Text);

typedef void (CObject::*TCanResizeEvent)(CObject* Sender, INT& NewWidth, INT& NewHeight, BOOL& Resize);
typedef void (CObject::*TConstrainedResizeEvent)(CObject *Sender, INT& MinWidth, INT& MinHeight,
    INT& MaxWidth, INT& MaxHeight);
typedef BOOL (CObject::*TMouseWheelEvent)(CObject* Sender, BYTE Shift, INT WheelDelta, TPoint& MousePos);
typedef BOOL (CObject::*TMouseWheelUpDownEvent)(CObject* Sender, BYTE Shift, TPoint& MousePos);
typedef void (CObject::*TMouseEvent)(CObject* Sender, TMouseButton Button, TShiftState Shift, INT X, INT Y);
typedef void (CObject::*TMouseMoveEvent)(CObject* Sender, TShiftState Shift, INT X, INT Y);
typedef void (CObject::*TContextPopupEvent)(CObject* Sender, TPoint& MousePos, BOOL& Handled);

class cVCL_API CControl : public CMsgTarget{
private:
	friend class CWinControl;

	CWinControl* Parent;
	INT Left;
	INT Top;
	INT Width;
	INT Height;
	INT MinWidth;
	INT MinHeight;
	INT MaxWidth;
	INT MaxHeight;
	UINT ControlStyle;
	UINT ControlState;
	BOOL Visible;
	BOOL Enabled;
	TAlign Align;
	BYTE Anchors;
	BOOL AutoSize;
	LPTSTR Text;
	TColor Color;
	SHORT Cursor;
	BOOL AnchorMove;
	TPoint OriginalParentSize;
	TPoint AnchorRules;
	BOOL ParentColor;
	BOOL ParentFont;
	BOOL ShowHint;
	BOOL ParentShowHint;

	TBiDiMode BiDiMode;
    BOOL ParentBiDiMode;
	CFont* Font;
	INT WheelAccumulator;

	BOOL CheckNewSize(INT& NewWidth, INT& NewHeight);
	BOOL DoCanAutoSize(INT& NewWidth, INT& NewHeight);
	BOOL DoCanResize(INT& NewWidth, INT& NewHeight);
	void DoConstrainedResize(INT& NewWidth, INT& NewHeight);
	void ConstrainedResize(INT& MinWidth, INT& MinHeight, INT& MaxWidth, INT& MaxHeight);

	void FontChanged(CObject* Sender);
	void DoMouseDown(TWMMouse& Message, TMouseButton Button, TShiftState Shift);
	void DoMouseUp(TWMMouse& Message, TMouseButton Button);

	void InvalidateControl(BOOL IsVisible, BOOL IsOpaque);
	void SetClientSize(TPoint& Value);
	BOOL BackgroundClipped(TRect& Rect);
	void SetZOrderPosition(INT Position);
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_LBUTTONDOWN, CControl::WMLButtonDown)
		MSG_MAP_ENTRY(WM_NCLBUTTONDOWN, CControl::WMNCLButtonDown)
		MSG_MAP_ENTRY(WM_RBUTTONDOWN, CControl::WMRButtonDown)
		MSG_MAP_ENTRY(WM_MBUTTONDOWN, CControl::WMMButtonDown)
		MSG_MAP_ENTRY(WM_LBUTTONDBLCLK, CControl::WMLButtonDblClk)
		MSG_MAP_ENTRY(WM_RBUTTONDBLCLK, CControl::WMRButtonDblClk)
		MSG_MAP_ENTRY(WM_MBUTTONDBLCLK, CControl::WMMButtonDblClk)
		MSG_MAP_ENTRY(WM_MOUSEMOVE, CControl::WMMouseMove)
		MSG_MAP_ENTRY(WM_LBUTTONUP, CControl::WMLButtonUp)
		MSG_MAP_ENTRY(WM_RBUTTONUP, CControl::WMRButtonUp)
		MSG_MAP_ENTRY(WM_MBUTTONUP, CControl::WMMButtonUp)
		MSG_MAP_ENTRY(WM_MOUSEWHEEL, CControl::WMMouseWheel)
		MSG_MAP_ENTRY(WM_CANCELMODE, CControl::WMCancelMode)
		MSG_MAP_ENTRY(WM_WINDOWPOSCHANGED, CControl::WMWindowPosChanged)
		MSG_MAP_ENTRY(CM_VISIBLECHANGED, CControl::CMVisibleChanged)
		MSG_MAP_ENTRY(CM_PARENTCOLORCHANGED, CControl::CMParentColorChanged)
		MSG_MAP_ENTRY(CM_PARENTFONTCHANGED, CControl::CMParentFontChanged)
		MSG_MAP_ENTRY(CM_PARENTSHOWHINTCHANGED, CControl::CMParentShowHintChanged)
		MSG_MAP_ENTRY(CM_PARENTBIDIMODECHANGED, CControl::CMParentBiDiModeChanged)
		MSG_MAP_ENTRY(CM_BIDIMODECHANGED, CControl::CMBiDiModeChanged)
		MSG_MAP_ENTRY(CM_FONTCHANGED, CControl::CMFontChanged)
		MSG_MAP_ENTRY(CM_COLORCHANGED, CControl::CMColorChanged)
		MSG_MAP_ENTRY(CM_MOUSEWHEEL, CControl::CMMouseWheel)
		MSG_MAP_ENTRY(CM_HITTEST, CControl::CMHitTest)
		MSG_MAP_ENTRY(CM_ENABLEDCHANGED, CControl::CMEnabledChanged)
		MSG_MAP_ENTRY(WM_CONTEXTMENU, CControl::WMContextMenu)
	MSG_MAP_END()
	void WMLButtonDown(TWMLButtonDown& Message);
	void WMNCLButtonDown(TWMNCLButtonDown& Message);
	void WMRButtonDown(TWMRButtonDown& Message);
	void WMMButtonDown(TWMMButtonDown& Message);
	void WMLButtonDblClk(TWMLButtonDblClk& Message);
	void WMRButtonDblClk(TWMRButtonDblClk& Message);
	void WMMButtonDblClk(TWMMButtonDblClk& Message);
	void WMMouseMove(TWMMouseMove& Message);
	void WMLButtonUp(TWMLButtonUp& Message);
	void WMRButtonUp(TWMRButtonUp& Message);
	void WMMButtonUp(TWMMButtonUp& Message);
	void WMMouseWheel(TWMMouseWheel& Message);
	void WMCancelMode(TWMCancelMode& Message);
	void WMWindowPosChanged(TWMWindowPosChanged& Message);
	void CMVisibleChanged(TMessage& Message);
	void CMParentColorChanged(TMessage& Message);
	void CMParentFontChanged(TMessage& Message);
	void CMParentShowHintChanged(TMessage& Message);
	void CMParentBiDiModeChanged(TMessage& Message);
	void CMBiDiModeChanged(TMessage& Message);
	void CMFontChanged(TMessage& Message);
	void CMColorChanged(TMessage& Message);
	void CMMouseWheel(TCMMouseWheel& Message);
	void CMHitTest(TCMHitTest& Message);
	void CMEnabledChanged(TMessage& Message);
	void WMContextMenu(TWMContextMenu& Message);
protected:
	virtual void WndProc(TMessage& Message);
	void DefaultHandler(TMessage& Message) override;

	virtual BOOL CanResize(INT& NewWidth, INT& NewHeight);
	virtual BOOL CanAutoSize(INT& NewWidth, INT& NewHeight);
	virtual BOOL DoMouseWheel(BYTE Shift, INT WheelDelta, TPoint& MousePos);
	virtual BOOL DoMouseWheelDown(BYTE Shift, TPoint& MousePos);
    virtual BOOL DoMouseWheelUp(BYTE Shift, TPoint& MousePos);
	virtual void DoContextPopup(TPoint& MousePos, BOOL& Handled);

	void Changed();

	virtual VOID RequestAlign();
	virtual void Resize();
	
	virtual void Click();
	virtual void DblClick();
	virtual void MouseDown(TMouseButton Button, TShiftState Shift, INT X, INT Y);
	virtual void MouseMove(TShiftState Shift, INT X, INT Y);
	virtual void MouseUp(TMouseButton Button, TShiftState Shift, INT X, INT Y);

	void SendCancelMode(CControl* Sender);
	TPoint CalcCursorPos();
	virtual VOID SetAutoSize(BOOL Value);
	
	virtual void SetZOrder(BOOL TopMost);
	virtual BOOL PaletteChanged(BOOL Foreground);
	virtual HPALETTE GetPalette();

public:
	DECLARE_EVENT(CanResize)
	DECLARE_EVENT(ConstrainedResize)
	DECLARE_TYPE_EVENT(TNotifyEvent, Resize)
	DECLARE_TYPE_EVENT(TNotifyEvent, Click)
	DECLARE_TYPE_EVENT(TNotifyEvent, DblClick)
	DECLARE_TYPE_EVENT(TMouseEvent, MouseDown)
	DECLARE_TYPE_EVENT(TMouseMoveEvent, MouseMove)
	DECLARE_TYPE_EVENT(TMouseEvent, MouseUp)
	DECLARE_TYPE_EVENT(TMouseWheelEvent, MouseWheel)
	DECLARE_TYPE_EVENT(TMouseWheelUpDownEvent, MouseWheelDown)
	DECLARE_TYPE_EVENT(TMouseWheelUpDownEvent, MouseWheelUp)
	DECLARE_TYPE_EVENT(TContextPopupEvent, ContextPopup)
public:
	CControl(CComponent* AOwner = NULL);
	virtual ~CControl();

	LRESULT Perform(UINT Msg, WPARAM WParam, LPARAM LParam);
	virtual VOID MouseWheelHandler(TMessage& Message);

	void UpdateAnchorRules();
	virtual void AdjustSize();

	virtual HDC GetDeviceContext(HWND& WindowHandle);
	DWORD DrawTextBiDiModeFlags(DWORD Flags);
	DWORD DrawTextBiDiModeFlagsReadingOnly();
	virtual BOOL UseRightToLeftAlignment();
	virtual TAlignment GetControlsAlignment();

	BOOL UseRightToLeftReading();

	virtual void SetBounds(INT ALeft, INT ATop, INT AWidth, INT AHeight);
	virtual void Invalidate();
	
	TRect GetBoundsRect();
	void Show();
    virtual void Update();
	void Refresh();
    virtual void Repaint();
	void BringToFront();

	BOOL GetMouseCapture();
	void SetMouseCapture(BOOL Value);

	virtual TRect GetClientRect();
	virtual INT GetClientHeight();
	virtual INT GetClientWidth();
	virtual void SetClientHeight(INT Value);
	virtual void SetClientWidth(INT Value);

	TPoint ScreenToClient(TPoint& Point);
	TPoint ClientToParent(TPoint& Point, CWinControl* AParent = NULL);
	TPoint ClientToScreen(const TPoint Point);
    
	virtual TPoint GetClientOrigin();
	
	virtual VOID VisibleChanging();
	
	DEFINE_GETTER(CWinControl*, Parent)
	DEFINE_GETTER(INT, MinWidth)
	DEFINE_GETTER(INT, MinHeight)
	DEFINE_GETTER(INT, MaxWidth)
	DEFINE_GETTER(INT, MaxHeight)
	DEFINE_GETTER(TAlign, Align)
	DEFINE_ACCESSOR(UINT, ControlStyle)
	DEFINE_ACCESSOR(UINT, ControlState)
	DEFINE_GETTER(BOOL, Visible)
	DEFINE_GETTER(INT, Left)
	DEFINE_GETTER(INT, Top)
	DEFINE_GETTER(INT, Width)
	DEFINE_GETTER(INT, Height)
	DEFINE_GETTER(TColor, Color)
	DEFINE_GETTER(BOOL, ShowHint)
	DEFINE_GETTER(TBiDiMode, BiDiMode)
	DEFINE_GETTER(CFont*, Font)
	DEFINE_GETTER(BOOL, ParentColor)
	DEFINE_GETTER(BOOL, ParentFont)
	DEFINE_GETTER(LPTSTR, Text)
	DEFINE_GETTER(BOOL, AutoSize)
	DEFINE_GETTER(TCursor, Cursor)

	virtual VOID SetParent(CWinControl* AParent);
	VOID SetVisible(BOOL Value);
	VOID SetLeft(INT Value);
	VOID SetTop(INT Value);
	VOID SetWidth(INT Value);
	VOID SetHeight(INT Value);
	VOID SetText(LPTSTR Text);
	VOID SetColor(TColor Value);
	VOID SetShowHint(BOOL Value);
	VOID SetBiDiMode(TBiDiMode Value);
	VOID SetFont(CFont* Value);
	VOID SetParentColor(BOOL Value);
	VOID SetParentFont(BOOL Value);
	void SetParentShowHint(BOOL Value);
	VOID SetAlign(TAlign Value);
	VOID SetCursor(TCursor Value);

	virtual BOOL GetEnabled();
	virtual void SetEnabled(BOOL Value);

	INT GetTextLen();
	INT GetTextBuf(LPTSTR Buffer, INT BufSize);
	VOID SetTextBuf(LPTSTR Text);
	String GetTextString();

	REF_DYN_CLASS(CControl)
};
DECLARE_DYN_CLASS(CControl, CMsgTarget)

class cVCL_API CGraphicControl : public CControl{
private:
	CCanvas* Canvas;
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_PAINT, CGraphicControl::WMPaint)
	MSG_MAP_END()
	void WMPaint(TWMPaint& Message);
protected:
    virtual void Paint();
public:
	CGraphicControl(CComponent* AOwner = NULL);
	virtual ~CGraphicControl();
	DEFINE_GETTER(CCanvas*, Canvas)

	REF_DYN_CLASS(CGraphicControl)
};
DECLARE_DYN_CLASS(CGraphicControl, CControl)

class cVCL_API CControlCanvas : public CCanvas{
private:
	friend class CGraphicControl;
	//friend class CCustomControl;
    CControl* Control;
    HDC DeviceContext;
	HWND WindowHandle;
protected:
    void CreateHandle() override;
public:
	CControlCanvas();
	virtual ~CControlCanvas();
	void FreeHandle();
	void UpdateTextFlags();
	DEFINE_GETTER(CControl*, Control);
	void SetControl(CControl* AControl);
	
	REF_DYN_CLASS(CControlCanvas)
};
DECLARE_DYN_CLASS(CControlCanvas, CCanvas)