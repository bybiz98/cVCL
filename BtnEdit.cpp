#include "stdinc.h"
#include "Button.hpp"
#include "ImgList.hpp"
#include "WinUtils.hpp"
#include "SysInit.hpp"
#include "Edit.hpp"
#include "BtnEdit.hpp"

IMPL_DYN_CLASS(CBtnEdit)
CBtnEdit::CBtnEdit(CComponent* AOwner){
	InplaceButton = new CSpeedButton(this);
	InplaceButton->SetAlign(alRight);
	InplaceButton->SetWidth(GetHeight());
	InplaceButton->GetGlyph()->LoadFromResourceName(GetGlobal().GetHInstance(), TEXT("EDITBTN"));
	InplaceButton->GetGlyph()->SetTransparent(TRUE);
	InplaceButton->SetVisible(TRUE);
	InplaceButton->SetParent(this);
	Shape = new CShape(this);
	Shape->SetWidth(1);
	Shape->GetPen()->SetColor(cl3DLight);
	Shape->SetParent(this);
	Shape->SetAlign(alRight);
	HitTest.x = 0;
	HitTest.y = 0;
}

CBtnEdit::~CBtnEdit(){
	Destroying();
	delete InplaceButton;
	delete Shape;
}

TNotifyEvent CBtnEdit::GetOnButtonClick(){
	return InplaceButton->GetOnClick();
}

void CBtnEdit::SetButtonEnabled(BOOL Value){
	InplaceButton->SetEnabled(Value);
}

void CBtnEdit::SetOnButtonClick(CObject* Obj, TNotifyEvent Value){
	InplaceButton->SetOnClick(Obj, Value);
}

BOOL CBtnEdit::GetButtonEnabled(){
	return InplaceButton->GetEnabled();
}

void CBtnEdit::SetEditRect(){
	TRect Loc;
	SendMessage(GetHandle(), EM_GETRECT, 0, (LPARAM)&Loc);
	Loc.bottom = GetClientHeight() + 1; //+1 is workaround for windows paint bug
	Loc.right = GetClientWidth() - InplaceButton->GetWidth() - 2;
	Loc.top = 0;
	Loc.left = 0;
	SendMessage(GetHandle(), EM_SETRECTNP, 0, (LPARAM)&Loc);
	SendMessage(GetHandle(), EM_GETRECT, 0, (LPARAM)&Loc); //debug
}

void CBtnEdit::WMSize(TWMSize& Message){
	INHERITED_MSG(Message);
	if(InplaceButton != NULL){
		INT iBorder = 0;
		if(GetCtl3D())
			iBorder = GetSystemMetrics(SM_CYEDGE);
		else
			iBorder = GetSystemMetrics(SM_CYBORDER);
		InplaceButton->SetWidth(GetHeight() - iBorder);
		SetEditRect();
	}
}

void CBtnEdit::WMSetCursor(TWMSetCursor& Message){
	HitTest = ScreenToClient(HitTest);
	if(InplaceButton != NULL && Message.HitTest == HTCLIENT && 
		PtInRect((const RECT *)&(InplaceButton->GetBoundsRect()), HitTest))
		Message.HitTest =HTBORDER;
	INHERITED_MSG(Message);
}

void CBtnEdit::WMNCHitTest(TWMNCHitTest& Message){
	INHERITED_MSG(Message);
	HitTest = SmallPointToPoint(Message.Pos);
}

void CBtnEdit::WMChar(TWMChar& Message){
	if(Message.CharCode == VK_RETURN)
		__super::DoKeyPress(Message);
	else
		INHERITED_MSG(Message);
}

void CBtnEdit::CreateParams(TCreateParams& Params){
	__super::CreateParams(Params);
	//Params.Style := Params.Style and not WS_BORDER;
	Params.Style |= ES_MULTILINE | WS_CLIPCHILDREN;
}

void CBtnEdit::CreateWnd(){
	__super::CreateWnd();
	SetEditRect();
}

CSpeedButton* CBtnEdit::GetButton(){
	return InplaceButton;
}
