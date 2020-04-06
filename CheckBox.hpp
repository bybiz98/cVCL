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

typedef BYTE TCheckBoxState;
#define cbUnchecked		0x0
#define cbChecked		0x1
#define cbGrayed		0x2


class cVCL_API CCheckBox : public CButtonControl{
private:
	TLeftRight Alignment;
	BOOL AllowGrayed;
	TCheckBoxState State;
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_SIZE, CCheckBox::WMSize)
		MSG_MAP_ENTRY(CM_CTL3DCHANGED, CCheckBox::CMCtl3DChanged)
		MSG_MAP_ENTRY(CM_DIALOGCHAR, CCheckBox::CMDialogChar)
		MSG_MAP_ENTRY(CN_COMMAND, CCheckBox::CNCommand)
	MSG_MAP_END()
	void WMSize(TMessage& Message);
	void CMCtl3DChanged(TMessage& Message);
	void CMDialogChar(TCMDialogChar& Message);
	void CNCommand(TWMCommand& Message);
protected:
	virtual void Toggle();
	void Click() override;
	void CreateParams(TCreateParams& Params) override;
	void CreateWnd() override;
public:
    CCheckBox(CComponent* AOwner = NULL);
	virtual ~CCheckBox();

	BOOL GetChecked() override;
	void SetChecked(BOOL Value) override;
	TAlignment GetControlsAlignment() override;
	
	DEFINE_GETTER(TLeftRight, Alignment)
	DEFINE_ACCESSOR(BOOL, AllowGrayed)
	DEFINE_GETTER(TCheckBoxState, State)
	void SetAlignment(TLeftRight Value);
	void SetState(TCheckBoxState Value);

  	REF_DYN_CLASS(CCheckBox)
};
DECLARE_DYN_CLASS(CCheckBox, CButtonControl)