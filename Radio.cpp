#include "stdinc.h"
#include "Radio.hpp"

IMPL_DYN_CLASS(CRadio)
CRadio::CRadio(CComponent* AOwner):CButtonControl(AOwner),
	Alignment(taRightJustify),
	Checked(FALSE){
	SetBounds(GetLeft(), GetTop(), 113, 17);
	SetControlStyle(csSetCaption | csDoubleClicks);
}

CRadio::~CRadio(){
}

void CRadio::CMCtl3DChanged(TMessage& Message){
	RecreateWnd();
}

void CRadio::CMDialogChar(TCMDialogChar& Message){
	String Text = GetTextString();
	if(IsAccel(Message.CharCode, Text.GetBuffer()) && CanFocus()){
		SetFocus();
		Message.Result = 1;
	}
	else
		INHERITED_MSG(Message);
}

void CRadio::CNCommand(TWMCommand& Message){
	if(Message.NotifyCode == BN_CLICKED)
		SetChecked(TRUE);
	else if(Message.NotifyCode == BN_DOUBLECLICKED)
		DblClick();
}

void CRadio::CreateParams(TCreateParams& Params){
	DWORD Alignments[2][2] = {{BS_LEFTTEXT, 0}, {0, BS_LEFTTEXT}};
	__super::CreateParams(Params);
	CreateSubClass(Params, TEXT("BUTTON"));
	Params.Style |= BS_RADIOBUTTON | Alignments[UseRightToLeftAlignment()][Alignment];
}

void CRadio::CreateWnd(){
	__super::CreateWnd();
	SendMessage(GetHandle(), BM_SETCHECK, (WPARAM)Checked, 0);
}

BOOL CRadio::GetChecked(){
	return Checked;
}

void CRadio::TurnSiblingsOff(){
	if(GetParent() != NULL){
		CWinControl* Parent = GetParent();
		INT Count = Parent->GetControlCount();
		for(INT I = 0; I < Count; I++){
			CControl* Sibling = Parent->GetControl(I);
			if(Sibling != this && Sibling->InstanceOf(CRadio::_Class)){
				CRadio* rSibling = (CRadio *)Sibling;
				//TODO if(rSibling->GetAction() != NULL && rSibling->GetAction()->InstanceOf(CCustomAction::_Class) &&
				//	((CCustomAction *)rSibling->GetAction())->GetAutoCheck())
				//	((CCustomAction *)rSibling->GetAction())->SetChecked(FALSE);
				rSibling->SetChecked(FALSE);
			}
		}
	}
}

void CRadio::SetChecked(BOOL Value){
	if(Checked != Value){
		Checked = Value;
		SetTabStop(Value);
		if(HandleAllocated())
			SendMessage(GetHandle(), BM_SETCHECK, (WPARAM)Checked, 0);
		if(Value){
			TurnSiblingsOff();
			__super::Changed();
			if(!GetClicksDisabled())
				Click();
		}
	}
}

TAlignment CRadio::GetControlsAlignment(){
	if(!UseRightToLeftAlignment())
		return taRightJustify;
	else if(Alignment == taRightJustify)
		return taLeftJustify;
	else
		return taRightJustify;
}

void CRadio::SetAlignment(TLeftRight Value){
	if(Alignment != Value){
		Alignment = Value;
		RecreateWnd();
	}
}