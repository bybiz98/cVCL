
#include "stdinc.h"
#include "SysInit.hpp"
#include "WinApp.hpp"
#include "Form.hpp"
#include "Screen.hpp"

IMPL_DYN_CLASS(CWinApp)
CWinApp::CWinApp(CComponent* AOwner):CComponent(AOwner),
	MouseControl(NULL),
	Running(FALSE),
	Terminated(FALSE),
	ModalLevel(0),
	INIT_EVENT(Message),
	INIT_EVENT(Idle),
	INIT_EVENT(ModalBegin),
	INIT_EVENT(ModalEnd){
	GetGlobal().SetApplication(this);
}

CWinApp::~CWinApp(){

}

void CWinApp::Run(){
	Running = TRUE;
	__try{

		do{
			__try{
				HandleMessage();
			}
			__except(EXCEPTION_EXECUTE_HANDLER){
				HandleException(this);
				//HandleException(this, GetExceptionCode(), GetExceptionInformation());
			}
		}while(!Terminated);
	}
	__finally{
		Running = FALSE;
	}
}

BOOL CWinApp::ProcessMessage(MSG& Msg){
	BOOL Handled = FALSE;
	BOOL Result = FALSE;
	if(PeekMessage(&Msg, 0, 0, 0, PM_REMOVE)){
		Result = TRUE;
		if(Msg.message != WM_QUIT){
			Handled = FALSE;
			if(OnMessage != NULL)
				CALL_EVENT(Message)(Msg, Handled);
			if(/*!IsHintMsg(Msg) && //*/!Handled/* && !IsMDIMsg(Msg) //*/
				&& !IsKeyMsg(Msg)/* && !IsDlgMsg(Msg)//*/)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
		else 
			Terminated = TRUE;
	}
	return Result;
}

void CWinApp::HandleMessage(){
	MSG Msg;
	if(!ProcessMessage(Msg))
		Idle(Msg);
}

void CWinApp::ModalStarted(){
	ModalLevel++;
	if (ModalLevel == 1 && OnModalBegin != NULL)
		CALL_EVENT(ModalBegin)(this);
}

void CWinApp::ModalFinished(){
	ModalLevel--;
	if(ModalLevel == 0 && OnModalEnd != NULL)
		CALL_EVENT(ModalEnd)(this);
}


BOOL AppVisible = FALSE;
void SetVisible(HWND FHandle, BOOL Value){
	WORD ShowFlags[] = {
		SWP_NOSIZE + SWP_NOMOVE + SWP_NOZORDER + SWP_NOACTIVATE + SWP_HIDEWINDOW,
		SWP_NOSIZE + SWP_NOMOVE + SWP_NOZORDER + SWP_NOACTIVATE + SWP_SHOWWINDOW
	};
    // Dont auto-update visibility if somebody else has hidden app window
    if (IsWindowVisible(FHandle) == AppVisible && AppVisible != Value){
		SetWindowPos(FHandle, 0, 0, 0, 0, 0, ShowFlags[Value]);
		AppVisible = Value;
	}
}

void CWinApp::UpdateVisible(){
	INT i = 0;
	CForm* Form = NULL;
	if(MainForm != NULL && MainForm->HandleAllocated()){
		for(i = 0; i < GetScreen()->GetFormCount(); i++){
			Form = GetScreen()->GetForm(i);
			if(Form->GetVisible() && ((Form->GetParentWindow() == 0) || Form->HandleAllocated() ||
				!IsChild(Form->GetHandle(), Form->GetParentWindow()))){
					SetVisible(MainForm->GetHandle(), TRUE);
					return;
			}
		}
		SetVisible(MainForm->GetHandle(), FALSE);
	}
}


void CWinApp::Idle(const MSG& Msg){
	CControl* Control = DoMouseIdle();
	//if(ShowHint && MouseControl == NULL)
	//	CancelHint();
	//Application.Hint = GetLongHint(GetHint(Control));
	BOOL Done = TRUE;
	__try{
		if(OnIdle != NULL)
			CALL_EVENT(Idle)(this, Done);
		if(Done)
			DoActionIdle();
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		HandleException(this);
	}
	//if (GetCurrentThreadId() == MainThreadID && CheckSynchronize())
	//	Done = FALSE;
	if(Done)
		WaitMessage();
}

BOOL CWinApp::IsKeyMsg(MSG& Msg){
	BOOL Result = FALSE;
	if(Msg.message >= WM_KEYFIRST && Msg.message <= WM_KEYLAST){
		HWND Wnd = GetCapture();
		if(Wnd == 0){
			Wnd = Msg.hwnd;
			if(MainForm != NULL && Wnd == MainForm->GetClientHandle())
				Wnd = MainForm->GetHandle();
			else {
				// Find the nearest VCL component.  Non-VCL windows wont know what
				// to do with CN_BASE offset messages anyway.
				// TOleControl.WndProc needs this for TranslateAccelerator
				while(FindControl(Wnd) == NULL && Wnd != 0)
					Wnd = GetParent(Wnd);
				if(Wnd == 0)
					Wnd = Msg.hwnd;
			}
			if(SendMessage(Wnd, CN_BASE + Msg.message, Msg.wParam, Msg.lParam) != 0)
				Result = TRUE;
		}
		else if ((HINSTANCE)GetWindowLongPtr(Wnd, GWLP_HINSTANCE) == GetGlobal().GetHInstance()){
			if(SendMessage(Wnd, CN_BASE + Msg.message, Msg.wParam, Msg.lParam) != 0)
				Result = TRUE;
		}
	}
	return Result;
}

void CWinApp::HandleException(CObject* Sender){
	if(GetCapture() != 0)
		SendMessage(GetCapture(), WM_CANCELMODE, 0, 0);
}

void CWinApp::DoActionIdle(){
}

CControl* CWinApp::DoMouseIdle(){
	TPoint P;
	GetCursorPos((LPPOINT)&P);
	CControl* Result = FindDragTarget(P, TRUE);
	CControl* CaptureControl = GetCaptureControl();
	if(MouseControl != Result){
		if((MouseControl != NULL && CaptureControl == NULL) ||
			(CaptureControl != NULL && MouseControl == CaptureControl))
			MouseControl->Perform(CM_MOUSELEAVE, 0, 0);
		MouseControl = Result;
		if((MouseControl != NULL && CaptureControl == NULL) ||
			(CaptureControl != NULL && MouseControl == CaptureControl))
			MouseControl->Perform(CM_MOUSEENTER, 0, 0);
	}
	return Result;
}