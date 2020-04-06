#include "stdinc.h"
#include "CheckBox.hpp"

IMPL_DYN_CLASS(CCheckBox)
CCheckBox::CCheckBox(CComponent* AOwner):CButtonControl(AOwner),
	Alignment(taRightJustify),
	AllowGrayed(FALSE),
	State(cbUnchecked){
	SetBounds(GetLeft(), GetTop(), 97, 17);
	SetTabStop(TRUE);
	SetControlStyle(csSetCaption | csDoubleClicks);
}

CCheckBox::~CCheckBox(){
}

void CCheckBox::WMSize(TMessage& Message){
	INHERITED_MSG(Message);
	Invalidate();
}

void CCheckBox::CMCtl3DChanged(TMessage& Message){
	RecreateWnd();
}

void CCheckBox::CMDialogChar(TCMDialogChar& Message){
	String Text = GetTextString();
	if(IsAccel(Message.CharCode, Text.GetBuffer()) && CanFocus()){
		SetFocus();
		if(Focused())
			Toggle();
		Message.Result = 1;
	}
	else
		INHERITED_MSG(Message);
}

void CCheckBox::CNCommand(TWMCommand& Message){
	if(Message.NotifyCode == BN_CLICKED)
		Toggle();
}

void CCheckBox::Toggle(){
	if(GetState() == cbUnchecked){
		if(GetAllowGrayed())
			SetState(cbGrayed);
		else 
			SetState(cbChecked);
	}
	else if(GetState() == cbChecked)
		SetState(cbUnchecked);
	else if(GetState() == cbGrayed)
		SetState(cbChecked);
}

void CCheckBox::Click(){
	__super::Changed();
	__super::Click();
}

void CCheckBox::CreateParams(TCreateParams& Params){
	DWORD Alignments[2][2] = {{BS_LEFTTEXT, 0}, {0, BS_LEFTTEXT}};
	__super::CreateParams(Params);
	CreateSubClass(Params, TEXT("BUTTON"));
	Params.Style |= BS_3STATE | Alignments[UseRightToLeftAlignment()][Alignment];
	Params.WinClass.style &= ~(CS_HREDRAW | CS_VREDRAW);
}

void CCheckBox::CreateWnd(){
	__super::CreateWnd();
	SendMessage(GetHandle(), BM_SETCHECK, (WPARAM)State, 0);
}

BOOL CCheckBox::GetChecked(){
	return GetState() == cbChecked;
}

void CCheckBox::SetChecked(BOOL Value){
	if(Value)
		SetState(cbChecked);
	else
		SetState(cbUnchecked);
}

TAlignment CCheckBox::GetControlsAlignment(){
	if(!UseRightToLeftAlignment())
		return taRightJustify;
	else if(Alignment == taRightJustify)
		return taLeftJustify;
	else 
		return taRightJustify;

}

void CCheckBox::SetAlignment(TLeftRight Value){
	if(Alignment != Value){
		Alignment = Value;
		RecreateWnd();
	}
}

void CCheckBox::SetState(TCheckBoxState Value){
	if(State != Value){
		State = Value;
		if(HandleAllocated())
			SendMessage(GetHandle(), BM_SETCHECK, (WPARAM)State, 0);
		if(GetClicksDisabled())
			Click();
	}
}