
#include "stdinc.h"
#include "SysInit.hpp"
#include "WinApp.hpp"
#include "Form.hpp"

IMPL_DYN_CLASS(CWinApp)
CWinApp::CWinApp(CComponent* AOwner):CComponent(AOwner),
	MouseControl(NULL),
	Running(FALSE),
	Terminated(FALSE),
	INIT_EVENT(Message),
	INIT_EVENT(Idle){
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