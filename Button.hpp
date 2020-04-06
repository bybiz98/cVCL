#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "ButtonControl.hpp"

class cVCL_API CButton : public CButtonControl{
private:
	BOOL Default;
	BOOL Cancel;
	BOOL Active;
	TModalResult ModalResult;
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_DIALOGKEY, CButton::CMDialogKey)
		MSG_MAP_ENTRY(CM_DIALOGCHAR, CButton::CMDialogChar)
		MSG_MAP_ENTRY(CM_FOCUSCHANGED, CButton::CMFocusChanged)
		MSG_MAP_ENTRY(CN_COMMAND, CButton::CNCommand)
		MSG_MAP_ENTRY(CN_CTLCOLORBTN, CButton::CNCtlColorBtn)
		MSG_MAP_ENTRY(WM_ERASEBKGND, CButton::WMEraseBkgnd)
	MSG_MAP_END()
	void CMDialogKey(TCMDialogKey& Message);
	void CMDialogChar(TCMDialogChar& Message);
	void CMFocusChanged(TCMFocusChanged& Message);
	void CNCommand(TWMCommand& Message);
	void CNCtlColorBtn(TWMCtlColorBtn& Message);
	void WMEraseBkgnd(TWMEraseBkgnd& Message);
protected:
	void CreateParams(TCreateParams& Params) override;
	void CreateWnd() override;
public:
    CButton(CComponent* AOwner = NULL);
	virtual ~CButton();
	void Click() override;
	BOOL UseRightToLeftAlignment() override;

	DEFINE_ACCESSOR(BOOL, Cancel)
	DEFINE_GETTER(BOOL, Default)
	DEFINE_ACCESSOR(TModalResult, ModalResult)
    virtual void SetButtonStyle(BOOL ADefault);
	void SetDefault(BOOL Value);

  	REF_DYN_CLASS(CButton)
};
DECLARE_DYN_CLASS(CButton, CButtonControl)


typedef BYTE TButtonLayout;
#define blGlyphLeft			0x0
#define blGlyphRight		0x1
#define blGlyphTop			0x2
#define	blGlyphBottom		0x3
typedef BYTE TNumGlyphs;
typedef BYTE TButtonState;
#define bsUp				0x0
#define bsDisabled			0x1
#define bsDown				0x2
#define bsExclusive			0x3
typedef BYTE TButtonStyle;
#define bsAutoDetect		0x0
#define bsWin31				0x1
#define bsNew				0x2

class cVCL_API CSpeedButton : public CGraphicControl{
private:
	INT GroupIndex;
	LPVOID Glyph;
	BOOL Down;
	BOOL Dragging;
	BOOL AllowAllUp;
	TButtonLayout Layout;
	INT Spacing;
	BOOL Transparent;
	INT Margin;
	BOOL Flat;
	BOOL MouseInControl;
	void GlyphChanged(CObject* Sender);
	void UpdateExclusive();
	void SetGlyph(CBitmap* Value);
	TNumGlyphs GetNumGlyphs();
	void SetNumGlyphs(TNumGlyphs Value);
	void UpdateTracking();

protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_LBUTTONDBLCLK, CSpeedButton::WMLButtonDblClk)
		MSG_MAP_ENTRY(CM_ENABLEDCHANGED, CSpeedButton::CMEnabledChanged)
		MSG_MAP_ENTRY(CM_BUTTONPRESSED, CSpeedButton::CMButtonPressed)
		MSG_MAP_ENTRY(CM_DIALOGCHAR, CSpeedButton::CMDialogChar)
		MSG_MAP_ENTRY(CM_FONTCHANGED, CSpeedButton::CMFontChanged)
		MSG_MAP_ENTRY(CM_TEXTCHANGED, CSpeedButton::CMTextChanged)
		MSG_MAP_ENTRY(CM_SYSCOLORCHANGE, CSpeedButton::CMSysColorChange)
		MSG_MAP_ENTRY(CM_MOUSEENTER, CSpeedButton::CMMouseEnter)
		MSG_MAP_ENTRY(CM_MOUSELEAVE, CSpeedButton::CMMouseLeave)
	MSG_MAP_END()
	void WMLButtonDblClk(TWMLButtonDown& Message);
	void CMEnabledChanged(TMessage& Message);
	void CMButtonPressed(TMessage& Message);
	void CMDialogChar(TCMDialogChar& Message);
	void CMFontChanged(TMessage& Message);
	void CMTextChanged(TMessage& Message);
	void CMSysColorChange(TMessage& Message);
	void CMMouseEnter(TMessage& Message);
	void CMMouseLeave(TMessage& Message);
protected:
	TButtonState State;
	//void ActionChange(CObject* Sender, BOOL CheckDefaults) override;
	//CControlActionLinkClass* GetActionLinkClass() override;
	HPALETTE GetPalette() override;
	void Loaded() override;
	void MouseDown(TMouseButton Button, TShiftState Shift, INT X, INT Y) override;
	void MouseMove(TShiftState Shift, INT X, INT Y) override;
	void MouseUp(TMouseButton Button, TShiftState Shift, INT X, INT Y) override;
	void Paint() override;
	DEFINE_GETTER(BOOL, MouseInControl)
public:
	CSpeedButton(CComponent* AOwner = NULL);
	virtual ~CSpeedButton();
	void Click() override;
	
	CBitmap* GetGlyph();
	
	DEFINE_GETTER(BOOL, Down)
	DEFINE_GETTER(BOOL, AllowAllUp)
	DEFINE_GETTER(INT, GroupIndex)
	DEFINE_GETTER(BOOL, Transparent)
	void SetDown(BOOL Value);
	void SetFlat(BOOL Value);
	void SetAllowAllUp(BOOL Value);
	void SetGroupIndex(INT Value);
	void SetLayout(TButtonLayout Value);
	void SetSpacing(INT Value);
	void SetTransparent(BOOL Value);
	void SetMargin(INT Value);

  	REF_DYN_CLASS(CSpeedButton)
};
DECLARE_DYN_CLASS(CSpeedButton, CGraphicControl)

typedef WORD TBitBtnKind;
#define bkCustom		0x0
#define bkOK			0x1
#define bkCancel		0x2
#define bkHelp			0x3
#define bkYes			0x4
#define bkNo			0x5
#define bkClose			0x6
#define bkAbort			0x7
#define bkRetry			0x8
#define bkIgnore		0x9
#define bkAll			0x10

class cVCL_API CBitBtn : public CButton{
private:
	CCanvas* Canvas;
	LPVOID Glyph;
	TButtonStyle Style;
	TBitBtnKind Kind;
	TButtonLayout Layout;
	INT Spacing;
	INT Margin;
	BOOL IsFocused;
	BOOL ModifiedGlyph;
	BOOL MouseInControl;
	void DrawItem(DRAWITEMSTRUCT& DrawItemStruct);
	void GlyphChanged(CObject* Sender);
	BOOL IsCustom();
	BOOL IsCustomCaption();
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CN_MEASUREITEM, CBitBtn::CNMeasureItem)
		MSG_MAP_ENTRY(CN_DRAWITEM, CBitBtn::CNDrawItem)
		MSG_MAP_ENTRY(CM_FONTCHANGED, CBitBtn::CMFontChanged)
		MSG_MAP_ENTRY(CM_ENABLEDCHANGED, CBitBtn::CMEnabledChanged)
		MSG_MAP_ENTRY(CM_MOUSEENTER, CBitBtn::CMMouseEnter)
		MSG_MAP_ENTRY(CM_MOUSELEAVE, CBitBtn::CMMouseLeave)
		MSG_MAP_ENTRY(WM_LBUTTONDBLCLK, CBitBtn::WMLButtonDblClk)
	MSG_MAP_END()
	void CNMeasureItem(TWMMeasureItem& Message);
	void CNDrawItem(TWMDrawItem& Message);
	void CMFontChanged(TMessage& Message);
	void CMEnabledChanged(TMessage& Message);
	void CMMouseEnter(TMessage& Message);
	void CMMouseLeave(TMessage& Message);
	void WMLButtonDblClk(TWMLButtonDblClk& Message);
protected:
	void ActionChange(CObject* Sender, BOOL CheckDefaults);
	void CreateHandle() override;
	void CreateParams(TCreateParams& Params) override;
	HPALETTE GetPalette() override;
	void SetButtonStyle(BOOL ADefault) override;
public:
	CBitBtn(CComponent* AOwner = NULL);
	virtual ~CBitBtn();
	void Click() override;

	DEFINE_GETTER(TButtonLayout, Layout)
	DEFINE_GETTER(INT, Margin)
	DEFINE_GETTER(TButtonStyle, Style)
	DEFINE_GETTER(INT, Spacing)
	void SetGlyph(CBitmap* Value);
	CBitmap* GetGlyph();
    TNumGlyphs GetNumGlyphs();
    void SetNumGlyphs(TNumGlyphs Value);
	void SetStyle(TButtonStyle Value);
	void SetKind(TBitBtnKind Value);
	TBitBtnKind GetKind();
	void SetLayout(TButtonLayout Value);
	void SetSpacing(INT Value);
	void SetMargin(INT Value);

	REF_DYN_CLASS(CBitBtn)
};
DECLARE_DYN_CLASS(CBitBtn, CButton)