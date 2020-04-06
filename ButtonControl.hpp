#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"

class cVCL_API CButtonControl : public CWinControl{
private:
    BOOL ClicksDisabled;
	BOOL WordWrap;
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CN_CTLCOLORSTATIC, CButtonControl::CNCtlColorStatic)
		MSG_MAP_ENTRY(WM_ERASEBKGND, CButtonControl::WMEraseBkGnd)
	MSG_MAP_END()
	void CNCtlColorStatic(TWMCtlColorStatic& Message);
	void WMEraseBkGnd(TWMEraseBkgnd& Message);
protected:
	//TODO void ActionChange(CObject* Sender, BOOL CheckDefaults) override;
    //CControlActionLinkClass* GetActionLinkClass() override;
	void WndProc(TMessage& Message) override;
	void CreateParams(TCreateParams& Params) override;
public:
    CButtonControl(CComponent* AOwner = NULL);
	virtual ~CButtonControl();
	DEFINE_ACCESSOR(BOOL, ClicksDisabled)
	DEFINE_GETTER(BOOL, WordWrap)
	void SetWordWrap(BOOL Value);
	virtual BOOL GetChecked();
    virtual void SetChecked(BOOL Value);

  	REF_DYN_CLASS(CButtonControl)
};
DECLARE_DYN_CLASS(CButtonControl, CWinControl)