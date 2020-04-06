#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "Edit.hpp"
#include "Button.hpp"
#include "Shape.hpp"

class cVCL_API CBtnEdit : public CEdit{
private:
    TPoint HitTest;
    CSpeedButton* InplaceButton;
	CShape* Shape;
	void SetEditRect();
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_SIZE, CBtnEdit::WMSize)
		MSG_MAP_ENTRY(WM_SETCURSOR, CBtnEdit::WMSetCursor)
		MSG_MAP_ENTRY(WM_NCHITTEST, CBtnEdit::WMNCHitTest)
		MSG_MAP_ENTRY(WM_CHAR, CBtnEdit::WMChar)
	MSG_MAP_END()
	void WMSize(TWMSize& Message);
	void WMSetCursor(TWMSetCursor& Message);
	void WMNCHitTest(TWMNCHitTest& Message);
	void WMChar(TWMChar& Message);
protected:
    void CreateParams(TCreateParams& Params) override;
	void CreateWnd() override;
public:
    CBtnEdit(CComponent* AOwner = NULL);
	virtual ~CBtnEdit();

	CSpeedButton* GetButton();
	TNotifyEvent GetOnButtonClick();
    void SetButtonEnabled(BOOL Value);
    void SetOnButtonClick(CObject* Obj, TNotifyEvent Value);
	BOOL GetButtonEnabled();

	REF_DYN_CLASS(CBtnEdit)
};
DECLARE_DYN_CLASS(CBtnEdit, CEdit)