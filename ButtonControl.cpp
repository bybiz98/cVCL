#include "stdinc.h"
#include "ButtonControl.hpp"
#include "SysInit.hpp"

IMPL_DYN_CLASS(CButtonControl)
CButtonControl::CButtonControl(CComponent* AOwner):CWinControl(AOwner),
	ClicksDisabled(FALSE),
	WordWrap(FALSE){
	//TODOif(Global::SysLocaleFastEast && Global::Win32Platform == VER_PLATFORM_WIN32_NT)
	//	SetImeMode(imDisable);
}

CButtonControl::~CButtonControl(){
}

void CButtonControl::CNCtlColorStatic(TWMCtlColorStatic& Message){
	/*TODO if(ThemeServices.ThemesEnabled){
		DrawParentBackground(GetHandle(), Message.ChildDC, NULL, FALSE);
		//{ Return an empty brush to prevent Windows from overpainting we just have created. }
		Message.Result = (LRESULT)GetStockObject(NULL_BRUSH);
	}
    else //*/
		INHERITED_MSG(Message);
}

void CButtonControl::WMEraseBkGnd(TWMEraseBkgnd& Message){
	//{ Under theme services the background is drawn in CN_CTLCOLORSTATIC. }
	/*TODO if(ThemeServices.ThemesEnabled)
		Message.Result = 1;
	else //*/
		INHERITED_MSG(Message);
}

//TODO void CButtonControl::ActionChange(CObject* Sender, BOOL CheckDefaults){}
//CControlActionLinkClass* CButtonControl::GetActionLinkClass(){}

void CButtonControl::WndProc(TMessage& Message){
	if(Message.Msg== WM_LBUTTONDOWN || Message.Msg == WM_LBUTTONDBLCLK){
		if(!IN_TEST(csDesigning, GetComponentState()) && !Focused()){
			ClicksDisabled = TRUE;
			::SetFocus(GetHandle());
			ClicksDisabled = FALSE;
			if(!Focused())
				return ;
		}
	}
	else if(Message.Msg == CN_COMMAND){
		if(ClicksDisabled)
			return ;
	}
	__super::WndProc(Message);
}

void CButtonControl::CreateParams(TCreateParams& Params){
	__super::CreateParams(Params);
	if(WordWrap)
		Params.Style |= BS_MULTILINE;
}

void CButtonControl::SetWordWrap(BOOL Value){
	if(WordWrap != Value){
		WordWrap = Value;
		RecreateWnd();
	}
}

BOOL CButtonControl::GetChecked(){
	return FALSE;
}

void CButtonControl::SetChecked(BOOL Value){
}