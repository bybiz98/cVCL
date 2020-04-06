#pragma once

#include "Object.hpp"
#include "List.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"

typedef BYTE TTextLayout;
#define tlTop		0x0
#define tlCenter	0x1
#define tlBottom	0x2

class cVCL_API CLabel : public CGraphicControl{
private:
	CWinControl* FocusControl;
    TAlignment Alignment;
	BOOL AutoSize;
	TTextLayout Layout;
	BOOL WordWrap;
	BOOL ShowAccelChar;
	BOOL TransparentSet;
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, MouseLeave)
	DECLARE_TYPE_EVENT(TNotifyEvent, MouseEnter)
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_TEXTCHANGED, CLabel::CMTextChanged)
		MSG_MAP_ENTRY(CM_FONTCHANGED, CLabel::CMFontChanged)
		MSG_MAP_ENTRY(CM_DIALOGCHAR, CLabel::CMDialogChar)
		MSG_MAP_ENTRY(CM_MOUSEENTER, CLabel::CMMouseEnter)
		MSG_MAP_ENTRY(CM_MOUSELEAVE, CLabel::CMMouseLeave)
	MSG_MAP_END()
	void CMTextChanged(TMessage& Message);
	void CMFontChanged(TMessage& Message);
	void CMDialogChar(TMessage& Message);
	void CMMouseEnter(TMessage& Message);
	void CMMouseLeave(TMessage& Message);
protected:
	virtual void AdjustBounds();
    void Paint() override;
	virtual void DoDrawText(TRect& Rect, DWORD Flags);
	virtual INT GetLabelText(LPTSTR Buffer, INT MaxLen);
	void Loaded() override;
	void Notification(CComponent* AComponent, TOperation Operation) override;
public:
	CLabel(CComponent* AOwner = NULL);
	virtual ~CLabel();
	
    void SetAutoSize(BOOL Value) override;
	DEFINE_GETTER(TAlignment, Alignment)
	VOID SetAlignment(TAlignment Value);
	DEFINE_GETTER(CWinControl*, FocusControl)
	VOID SetFocusControl(CWinControl* Control);
	DEFINE_GETTER(BOOL, ShowAccelChar)
	VOID SetShowAccelChar(BOOL Value);
	BOOL GetTransparent();
	VOID SetTransparent(BOOL Value);
	DEFINE_GETTER(TTextLayout, Layout)
	VOID SetLayout(TTextLayout Value);
	DEFINE_GETTER(BOOL, WordWrap)
	VOID SetWordWrap(BOOL Value);
	REF_DYN_CLASS(CLabel)
};
DECLARE_DYN_CLASS(CLabel, CGraphicControl)