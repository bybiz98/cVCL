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

class cVCL_API CRadio : public CButtonControl{
private:
	TLeftRight Alignment;
	BOOL Checked;
	void TurnSiblingsOff();
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_CTL3DCHANGED, CRadio::CMCtl3DChanged)
		MSG_MAP_ENTRY(CM_DIALOGCHAR, CRadio::CMDialogChar)
		MSG_MAP_ENTRY(CN_COMMAND, CRadio::CNCommand)
	MSG_MAP_END()
	void CMCtl3DChanged(TMessage& Message);
	void CMDialogChar(TCMDialogChar& Message);
	void CNCommand(TWMCommand& Message);
protected:
	void CreateParams(TCreateParams& Params) override;
	void CreateWnd() override;
public:
    CRadio(CComponent* AOwner = NULL);
	virtual ~CRadio();

	BOOL GetChecked() override;
	void SetChecked(BOOL Value) override;
	TAlignment GetControlsAlignment() override;
	
	DEFINE_GETTER(TLeftRight, Alignment)
	void SetAlignment(TLeftRight Value);

  	REF_DYN_CLASS(CRadio)
};
DECLARE_DYN_CLASS(CRadio, CButtonControl)