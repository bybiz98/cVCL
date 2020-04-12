
#include "stdinc.h"
#include "WinControl.hpp"
#include "ObjWndProc.hpp"
#include "SysInit.hpp"
#include "WinUtils.hpp"
#include "Form.hpp"
#include "ComboBox.hpp"
#include "Screen.hpp"

extern CControl *CaptureControl;

BOOL DoControlMsg(HWND ControlHandle, TMessage& Message){
	BOOL Result = FALSE;
	CWinControl* Control = FindControl(ControlHandle);
	if(Control != NULL){
		Message.Result = Control->Perform(Message.Msg + CN_BASE, Message.wParam, Message.lParam);
		Result = TRUE;
	}
	return Result;
}

IMPL_DYN_CLASS(CWinControl)
CWinControl* CreationControl = NULL;
LRESULT CWinControl::InitWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	CreationControl->Wnd = hWnd;
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)CreationControl->GetWndProc());
	if ((GetWindowLongPtr(hWnd, GWL_STYLE) && WS_CHILD != 0) && (GetWindowLongPtr(hWnd, GWL_ID) == 0)){
		SetWindowLongPtr(hWnd, GWL_ID, (LONG_PTR)hWnd);
	}
	//SetProp(hWnd, MakeIntAtom(ControlAtom), (HANDLE)CreationControl);
	//SetProp(hWnd, MakeIntAtom(WindowAtom), (HANDLE)CreationControl);
	CWinControl *cTmp = CreationControl;
	CreationControl = NULL;
	return cTmp->StdWndProc(message, wParam, lParam);
}

LPVOID CWinControl::GetWndProc(){
	return this->lpWndProc;
}

LRESULT CWinControl::StdWndProc(UINT message, WPARAM wParam, LPARAM lParam){
	TMessage Msg;
	Msg.Msg = message;
	Msg.wParam = wParam;
	Msg.lParam = lParam;
	Msg.Result = 0;
	WndProc(Msg);
	return Msg.Result;
}

CWinControl::CWinControl(CComponent* AOwner):CControl(AOwner),
	Wnd(0),
	ParentWindow(0),
	fnDefWndProc(NULL),
	Controls(NULL),
	WinControls(NULL),
	TabList(NULL),
	TabStop(TRUE),
	TabOrder(-1),
	Showing(FALSE),
	Ctl3D(TRUE),
	ParentCtl3D(TRUE),
	DoubleBuffered(FALSE),
	AlignLevel(0),
	BevelEdges(beLeft | beTop | beRight | beBottom),
	BevelInner(bvRaised),
	BevelOuter(bvLowered),
	BevelWidth(1),
	BevelKind(bkNone),
	BorderWidth(0),
	INIT_EVENT(KeyDown),
	INIT_EVENT(KeyPress),
	INIT_EVENT(KeyUp),
	INIT_EVENT(Enter),
	INIT_EVENT(Exit){
	lpWndProc = MakeObjectWndProc(this, (TObjectWndProc)&CWinControl::StdWndProc);
	Brush = new CBrush();
	Brush->SetColor(Color);
}

CWinControl::~CWinControl(){
	//MessageBox(0, TEXT("WinControl"), TEXT("CLOSE"), 0);
	delete Brush;
	FreeObjectWndProc(lpWndProc);
}

HWND CWinControl::GetHandle(){
	HandleNeeded();
	return Wnd;
}

void CWinControl::WndProc(TMessage& Message){
	CForm* Form = NULL;
	switch(Message.Msg){
		case WM_SETFOCUS:
			Form = GetParentForm(this);
			if(Form != NULL && !Form->SetFocusedControl(this))
				return;
			break;
		case WM_KILLFOCUS:
			if(IN_TEST(csFocusing, ControlState))
				return ;
			break;
		case WM_NCHITTEST:
			__super::WndProc(Message);
			if (Message.Result == HTTRANSPARENT && ControlAtPos(ScreenToClient(SmallPointToPoint(MAKEPOINTS(Message.lParam))), FALSE) != NULL)
				Message.Result = HTCLIENT;
			return ;
		case WM_CANCELMODE:
			if (GetCapture() == GetHandle() && GetCaptureControl() != NULL && GetCaptureControl()->GetParent() == this)
				GetCaptureControl()->Perform(WM_CANCELMODE, 0, 0);
			break;
		default:
			if(Message.Msg >= WM_MOUSEFIRST && Message.Msg <= WM_MOUSELAST){
				if(IsControlMouseMsg(*((PWMMouse)&Message))){
					/* Check HandleAllocated because IsControlMouseMsg might have freed the
					  window if user code executed something like SetParent(NULL). */
					if (Message.Result == 0 && HandleAllocated())
						DefWindowProc(Wnd, Message.Msg, Message.wParam, Message.lParam);
					return ;
				}
			}
			else if(Message.Msg >= WM_KEYFIRST && Message.Msg <= WM_KEYLAST){
				//if (Dragging())
				//	return ;
			}
	}
	__super::WndProc(Message);
}

void CWinControl::DefaultHandler(TMessage& Message){
	if(Wnd){
		if (Message.Msg == WM_CONTEXTMENU && GetParent() != NULL){
			Message.Result = GetParent()->Perform(Message.Msg, Message.wParam, Message.lParam);
			if (Message.Result != 0) 
				return ;
		}
		if(Message.Msg >= WM_CTLCOLORMSGBOX && Message.Msg <= WM_CTLCOLORSTATIC){
			Message.Result = SendMessage((HWND)Message.lParam, CN_BASE + Message.Msg, Message.wParam, Message.lParam);
		}
		else if(Message.Msg >= CN_CTLCOLORMSGBOX && Message.Msg <= CN_CTLCOLORSTATIC){
            SetTextColor((HDC)Message.wParam, ColorToRGB(Font->GetColor()));
            SetBkColor((HDC)Message.wParam, ColorToRGB(Brush->GetColor()));
            Message.Result = (LRESULT)Brush->GetHandle();
		}
		else{
			if (Message.Msg == GetGlobal().GetRM_GetObjectInstance()){
				Message.Result = (LRESULT)this;
			}else
				Message.Result = CallWindowProc(fnDefWndProc, Wnd, Message.Msg, Message.wParam, Message.lParam);
		}
		//if(Message.Msg == WM_SETTEXT)
		//	SendDockNotification(Message.Msg, Message.wParam, Message.lParam);
	}
	else
		__super::DefaultHandler(Message);
}

BOOL CWinControl::IsControlMouseMsg(TWMMouse& Message){
	CControl* Control = NULL;
	if(::GetCapture() == GetHandle()){
		if(CaptureControl != NULL && CaptureControl->GetParent() == this)
			Control = CaptureControl;
	}
	else
		Control = ControlAtPos(SmallPointToPoint(Message.Pos), FALSE);
	BOOL Result = FALSE;
	TPoint P;
	if(Control != NULL){
		P.x = Message.XPos - Control->GetLeft();
		P.y = Message.YPos - Control->GetTop();
		Message.Result = Control->Perform(Message.Msg, Message.Keys, (LPARAM)PointToSmallPoint(P));
		Result = TRUE;
	}
	return Result;
}

void CWinControl::SelectFirst(){
	CForm* Form = GetParentForm(this);
	if(Form != NULL){
		CWinControl* Control = FindNextControl(NULL, TRUE, TRUE, FALSE);
		if(Control == NULL)
			Control = FindNextControl(NULL, TRUE, FALSE, FALSE);
		if(Control != NULL)
			Form->SetActiveControl(Control);
	}
}

CWinControl* CWinControl::FindNextControl(CWinControl* CurControl, 
		BOOL GoForward, BOOL CheckTabStop, BOOL CheckParent){
	CWinControl* Result = NULL;
	CList List;
	GetTabOrderList(&List);
	if(List.GetCount() > 0){
		INT StartIndex = List.IndexOf(CurControl);
		if(StartIndex == -1)
			if(GoForward)
				StartIndex = List.GetCount() - 1;
			else 
				StartIndex = 0;
		INT I = StartIndex;
		do{
			if(GoForward){
				I++;
				if(I == List.GetCount())
					I = 0;
			}
			else {
				if(I == 0)
					I = List.GetCount();
				I--;
			}
			CurControl = (CWinControl *)List.Get(I);
			if(CurControl->CanFocus() && (!CheckTabStop || CurControl->GetTabStop()) &&
				(!CheckParent || (CurControl->GetParent() == this)))
				Result = CurControl;
		}while(!(Result != NULL || I == StartIndex));
	}
	return Result;
}

BOOL CWinControl::DoKeyDown(TWMKey& Message){
	CForm* Form = GetParentForm(this);
	if(Form != NULL && Form != this && Form->GetKeyPreview() &&
		((CWinControl*)Form)->DoKeyDown(Message))
		return TRUE;
	TShiftState ShiftState = KeyDataToShiftState((LONG)Message.KeyData);
	if(!IN_TEST(csNoStdEvents, GetControlStyle())){
		KeyDown(Message.CharCode, ShiftState);
		if(Message.CharCode == 0)
			return TRUE;
	}
	return FALSE;
}

BOOL CWinControl::DoKeyPress(TWMKey& Message){
	BOOL Result = TRUE;
	CForm* Form = GetParentForm(this);
	if(Form != NULL && Form != this && Form->GetKeyPreview() &&
		Form->DoKeyPress(Message))
		return Result;
	if(!IN_TEST(csNoStdEvents, GetControlStyle())){
		TCHAR Ch = Message.CharCode;
		KeyPress(Ch);
		Message.CharCode = Ch;
		if(Message.CharCode == TCHAR('\0'))
			return Result;
	}
	return FALSE;
}

BOOL CWinControl::DoKeyUp(TWMKey& Message){
	CForm* Form = GetParentForm(this);
	if(Form != NULL && Form != this && Form->GetKeyPreview() && 
		((CWinControl *)Form)->DoKeyUp(Message))
		return TRUE;
	TShiftState ShiftState = KeyDataToShiftState((LONG)Message.KeyData);
	if(!IN_TEST(csNoStdEvents, GetControlStyle())){
		KeyUp(Message.CharCode, ShiftState);
		if(Message.CharCode == 0)
			return TRUE;
	}
	return FALSE;
}

void CWinControl::KeyDown(WORD& Key, TShiftState Shift){
	if(OnKeyDown != NULL)
		CALL_EVENT(KeyDown)(this, Key, Shift);
}

void CWinControl::KeyPress(TCHAR& Key){
	if(OnKeyPress != NULL){
		CALL_EVENT(KeyPress)(this, Key);
	}
}

void CWinControl::KeyUp(WORD& Key, TShiftState Shift){
	if(OnKeyUp != NULL)
		CALL_EVENT(KeyUp)(this, Key, Shift);
}

void CWinControl::GetTabOrderList(CList* List){
	if(TabList != NULL){
		INT Count = TabList->GetCount();
		for(INT I = 0; I < Count; I++){
			CWinControl* Control = (CWinControl *)TabList->Get(I);
			List->Add(Control);
			Control->GetTabOrderList(List);
		}
	}
}

BOOL CWinControl::ContainsControl(CControl* Control){
	while(Control != NULL && Control != this)
		Control = Control->GetParent();
	return Control != NULL;
}

HDC CWinControl::GetDeviceContext(HWND& WindowHandle){
	HDC Result = 0;
	if(IN_TEST(csDesigning, GetComponentState()))
		Result = GetDCEx(GetHandle(), 0, DCX_CACHE | DCX_CLIPSIBLINGS);
	else
		Result = GetDC(GetHandle());
	if(Result == 0)
		throw "Error creating window device context";
	WindowHandle = Wnd;
	return Result;
}

BOOL CWinControl::HandleAllocated(){
	return Wnd != 0;
}

void CWinControl::HandleNeeded(){
	if(Wnd == 0){
		if(GetParent() != NULL)
			GetParent()->HandleNeeded();
		CreateHandle();
	}
}

HWND CWinControl::PrecedingWindow(CWinControl* Control){
	INT Count = WinControls->GetCount();
	for(INT I = WinControls->IndexOf(Control) + 1; I < Count; I++){
		HWND Result = ((CWinControl *)WinControls->Get(I))->Wnd;
		if(Result != 0)
			return Result;
	}
	return HWND_TOP;
}

void CWinControl::SetZOrderPosition(INT Position){
	if(Parent != NULL){
		if(Parent->Controls != NULL)
			Position -= Parent->Controls->GetCount();
		INT I = Parent->WinControls->IndexOf(this);
		if(I >= 0){
			INT Count = Parent->WinControls->GetCount();
			if(Position < 0)
				Position = 0;
			if(Position >= Count)
				Position = Count - 1;
			if(Position != I){
				Parent->WinControls->Delete(I);
				Parent->WinControls->Insert(Position, this);
			}
		}
		if(Wnd != 0){
			HWND Pos = 0;
			if(Position == 0)
				Pos = HWND_BOTTOM;
			else if(Position == Parent->WinControls->GetCount() - 1)
				Pos = HWND_TOP;
			else if(Position > I)
				Pos = ((CWinControl *)Parent->WinControls->Get(Position + 1))->GetHandle();
			else if(Position < I)
				Pos = ((CWinControl *)Parent->WinControls->Get(Position))->GetHandle();
			else return ;
			SetWindowPos(Wnd, Pos, 0, 0, 0, 0, SWP_NOMOVE + SWP_NOSIZE);
		}
	}
}

void CWinControl::CreateHandle(){
	if(Wnd == 0){
		CreateWnd();
		//SetProp(Wnd, MakeIntAtom(ControlAtom), (HANDLE)this);
		//SetProp(Wnd, MakeIntAtom(WindowAtom), (HANDLE)this);
		if(GetParent() != NULL){
			SetWindowPos(Wnd, GetParent()->PrecedingWindow(this), 0, 0, 0, 0,
				SWP_NOMOVE + SWP_NOSIZE + SWP_NOACTIVATE);
		}
		INT ControlCount = GetControlCount();
		for(int i = 0; i < ControlCount; i++){
			GetControl(i)->UpdateAnchorRules();
		}
	}
}

void CWinControl::CreateParams(TCreateParams& Params){
	ZeroMemory(&Params, sizeof(TCreateParams));
	Params.szTitle = Text;
    Params.Style = WS_CHILD | WS_CLIPSIBLINGS;
	//AddBiDiModeExStyle(Params.ExStyle);
    if ((csAcceptsControls & ControlStyle) == csAcceptsControls){
		Params.Style = Params.Style | WS_CLIPCHILDREN;
		Params.ExStyle = Params.ExStyle | WS_EX_CONTROLPARENT;
	}
    if (!((csDesigning & GetComponentState()) == csDesigning) && !GetEnabled())
		Params.Style = Params.Style | WS_DISABLED;
    if (TabStop) 
		Params.Style = Params.Style | WS_TABSTOP;
    Params.Left = Left;
    Params.Top = Top;
    Params.Width = Width;
    Params.Height = Height;
    if (GetParent() != NULL)
      Params.WndParent = GetParent()->GetHandle();
	else
      Params.WndParent = ParentWindow;
	Params.WinClass.cbSize = sizeof(Params.WinClass);
    Params.WinClass.style = CS_VREDRAW + CS_HREDRAW + CS_DBLCLKS;
    Params.WinClass.lpfnWndProc = &DefWindowProc;
    Params.WinClass.hCursor = LoadCursor(0, IDC_ARROW);
    Params.WinClass.hbrBackground = 0;
    Params.WinClass.hInstance = GetGlobal().GetHInstance();
	lstrcpyn((LPTSTR)Params.WinClassName, this->GetClass()->GetName(), sizeof(Params.WinClassName) / sizeof(TCHAR));
}

void CWinControl::CreateSubClass(TCreateParams& Params, LPTSTR ControlClassName){
	DWORD CS_OFF = CS_OWNDC | CS_CLASSDC | CS_PARENTDC | CS_GLOBALCLASS;
	DWORD CS_ON = CS_VREDRAW | CS_HREDRAW;
	if(ControlClassName != NULL){
		HINSTANCE SaveInstance = Params.WinClass.hInstance;
		if(!GetClassInfoEx(GetGlobal().GetHInstance(), ControlClassName, (LPWNDCLASSEX)&(Params.WinClass)) &&
			!GetClassInfoEx(0, ControlClassName, (LPWNDCLASSEX)&(Params.WinClass)) &&
			!GetClassInfoEx(GetGlobal().GetMainInstance(), ControlClassName, (LPWNDCLASSEX)&(Params.WinClass)))
			GetClassInfoEx(Params.WinClass.hInstance, ControlClassName, (LPWNDCLASSEX)&(Params.WinClass));
		Params.WinClass.hInstance = SaveInstance;
		Params.WinClass.style = Params.WinClass.style & (~CS_OFF) | CS_ON;
	}
}

void CWinControl::CreateWindowHandle(const TCreateParams& Params){
	Wnd = CreateWindowEx(Params.ExStyle, Params.WinClassName, Params.szTitle, Params.Style,
		Params.Left, Params.Top, Params.Width, Params.Height, Params.WndParent, 0, 
		Params.WinClass.hInstance, Params.lpParam);
}

void CWinControl::CreateWnd(){
	TCreateParams Params;
	CreateParams(Params);
	if ((Params.WndParent == 0) && (Params.Style & WS_CHILD) != 0){
		CComponent* Owner = GetOwner();
		if ((Owner != NULL) && ((csReading & Owner->GetComponentState()) == csReading) &&
			(Owner->InstanceOf(CWinControl::_Class)))
			Params.WndParent = ((CWinControl *)Owner)->GetHandle();
      else
		  throw "Control ''%s'' has no parent window";
		//raise EInvalidOperation.CreateFmt(SParentRequired, [Name]);
	}

	fnDefWndProc = Params.WinClass.lpfnWndProc;
	WNDCLASSEX wcTemp;
	BOOL ClassRegistered = GetClassInfoEx(Params.WinClass.hInstance, Params.WinClassName, &wcTemp);
    if (!ClassRegistered || (wcTemp.lpfnWndProc != &InitWndProc)){
		if (ClassRegistered) 
			UnregisterClass(Params.WinClassName, Params.WinClass.hInstance);
		Params.WinClass.lpfnWndProc = &InitWndProc;
		Params.WinClass.lpszClassName = Params.WinClassName;
		if (RegisterClassEx(&(Params.WinClass)) == 0)
			RaiseLastOSError();
	}
    CreationControl = this;
    CreateWindowHandle(Params);
    if (Wnd == 0)
		RaiseLastOSError();
	if(Params.WinClass.lpszMenuName != NULL){
		HMENU hMenu = ::GetMenu(Wnd);
		if(hMenu == NULL){
			if(Params.WinClass.hInstance != GetGlobal().GetHInstance()){
				hMenu = ::LoadMenu(GetGlobal().GetHInstance(), Params.WinClass.lpszMenuName);
			}
			if(hMenu == NULL && Params.WinClass.hInstance != GetGlobal().GetMainInstance()){
				hMenu = ::LoadMenu(GetGlobal().GetMainInstance(), Params.WinClass.lpszMenuName);
			}
			if(hMenu != NULL){
				::SetMenu(Wnd, hMenu);
			}
		}
	}
    if (((GetWindowLongPtr(Wnd, GWL_STYLE) & WS_CHILD) != 0) && (GetWindowLongPtr(Wnd, GWL_ID) == 0))
		SetWindowLongPtr(Wnd, GWL_ID, (LONG_PTR)Wnd);
	if(Text != NULL){
		free(Text);
		Text = NULL;
	}
	UpdateBounds();
	Perform(WM_SETFONT, (WPARAM)Font->GetHandle(), 1);
	if(AutoSize)
		AdjustSize();
}

BOOL GetControlAtPos(const POINT& Pos, BOOL AllowDisabled, CControl *AControl){
	POINT P;
	P.x = Pos.x - AControl->GetLeft();
	P.y = Pos.y - AControl->GetTop();
	BOOL Result = PtInRect((CONST RECT *)&(AControl->GetClientRect()), P) &&
                (
				IN_TEST(csDesigning,AControl->GetComponentState()) && (AControl->GetVisible() || !IN_TEST(csNoDesignVisible, AControl->GetControlStyle())) 
				||
                (AControl->GetVisible() && (AControl->GetEnabled() || AllowDisabled) && (AControl->Perform(CM_HITTEST, 0, PointToSmallPoint(P)) != 0)) 
				);
	return Result;
}
CControl* CWinControl::ControlAtPos(const POINT& Pos, BOOL AllowDisabled, BOOL AllowWinControls){
	CControl* LControl = NULL;
	if (AllowWinControls && WinControls != NULL){
		for(int i = WinControls->GetCount() - 1; i >= 0; i--){
			if(GetControlAtPos(Pos, AllowDisabled, (CControl*)WinControls->Get(i))){
				LControl = (CControl*)WinControls->Get(i);
				break;
			}
		}
	}
	if (Controls != NULL && LControl == NULL){
		for (int i = Controls->GetCount() - 1; i >= 0; i--){
			if (GetControlAtPos(Pos, AllowDisabled, (CControl *)Controls->Get(i))){
				LControl = (CControl *)Controls->Get(i);
				break;
			}
		}
	}
	return LControl;
}

TRect CWinControl::GetClientRect(){
	TRect Result;
	::GetClientRect(GetHandle(), (LPRECT)&Result);
	return Result;
}

TPoint CWinControl::GetClientOrigin(){
	TPoint Result;
	Result.x = 0;
	Result.y = 0;
	::ClientToScreen(GetHandle(), &Result);
	return Result;
}

INT CWinControl::GetControlCount(){
	INT Result = 0;
	if(Controls != NULL)
		Result += Controls->GetCount();
	if(WinControls != NULL)
		Result += WinControls->GetCount();
	return Result;
}

CControl* CWinControl::GetControl(INT Index){
	INT N = 0;
	CControl* Result = NULL;
	if(Controls != NULL)
		N = Controls->GetCount();
	else 
		N = 0;
	if(Index < N)
		Result = (CControl *)Controls->Get(Index);
	else
		Result = (CControl *)WinControls->Get(Index - N);
	return Result;
}

void CWinControl::DisableAlign(){
	AlignLevel++;
}

void CWinControl::EnableAlign(){
	AlignLevel--;
	if(AlignLevel == 0){
		if ((csAlignmentNeeded & ControlState) == csAlignmentNeeded)
			Realign();
	}
}
void CWinControl::Realign(){
	AlignControl(NULL);
}

void CWinControl::AlignControl(CControl* AControl){
	if (!HandleAllocated() || ((csDestroying & GetComponentState()) == csDestroying))
		return;
	if (AlignLevel != 0)
		ControlState |= csAlignmentNeeded;
	else{
		DisableAlign();
		__try{
			TRect Rect = GetClientRect();
			AlignControls(AControl, Rect);
		}
		__finally{
			ControlState &= !csAlignmentNeeded;
			EnableAlign();
		}
	}
}

BOOL CWinControl::AlignWork(){
	BOOL Result = TRUE;
	for(int i = GetControlCount() - 1; i >= 0; i--){
		CControl* AControl = GetControl(i);
		if((AControl->Align != alNone) || (AControl->Anchors != akLeft + akTop))
			return TRUE;
	}
    return FALSE;
}

void CWinControl::AdjustClientRect(TRect& Rect){
}

void CWinControl::DoAlign(CList& AlignList, CControl* AControl, TRect& Rect, UINT AAlign){
	AlignList.Clear();
	if ((AControl != NULL) && ((AAlign == alNone) || AControl->GetVisible() || ((AControl->GetComponentState() & csDesigning) == csDesigning) &&
			!((AControl->GetControlStyle() & csNoDesignVisible) == csNoDesignVisible)) && (AControl->GetAlign() == AAlign))
		AlignList.Add(AControl);
	int ControlCount = GetControlCount();
    for (int i = 0; i < ControlCount; i++){
		CControl* aControl = GetControl(i);
		if((aControl->GetAlign() == AAlign) && ((AAlign == alNone) || (aControl->GetVisible() ||
			((aControl->GetControlStyle() & (csAcceptsControls | csNoDesignVisible)) == 
			(csAcceptsControls | csNoDesignVisible))) ||
			((aControl->GetComponentState() & csDesigning) == csDesigning) &&
			((aControl->GetControlStyle() & csNoDesignVisible) != csNoDesignVisible))){
			if (aControl == AControl)
				continue;
			INT j = 0;
			while ((j < AlignList.GetCount()) && !InsertBefore(aControl,
				(CControl *)AlignList.Get(j), AAlign))
				j++;
			AlignList.Insert(j, aControl);
		}
	}
	int AlignCount = AlignList.GetCount();
	TAlignInfo AlignInfo;
    for(int i = 0; i < AlignCount; i++){
      AlignInfo.AlignList = &AlignList;
      AlignInfo.ControlIndex = i;
      AlignInfo.uAlign = AAlign;
      DoPosition((CControl *)AlignList.Get(i), AAlign, AlignInfo, Rect);
	}
}

void CWinControl::AlignControls(CControl* AControl, TRect& Rect){
	//if FDockSite and FUseDockManager and (FDockManager <> nil) then
	//	FDockManager.ResetBounds(False);
	if (AlignWork()){
		AdjustClientRect(Rect);
		CList AlignList;
		DoAlign(AlignList, AControl, Rect, alTop);
		DoAlign(AlignList, AControl, Rect, alBottom);
		DoAlign(AlignList, AControl, Rect, alLeft);
		DoAlign(AlignList, AControl, Rect, alRight);
		DoAlign(AlignList, AControl, Rect, alClient);
		DoAlign(AlignList, AControl, Rect, alCustom);
		DoAlign(AlignList, AControl, Rect, alNone);// Move anchored controls
		ControlsAligned();
	}
	if (Showing)
		AdjustSize();
}

void CWinControl::ControlsAligned(){
}


BOOL CWinControl::InsertBefore(CControl* C1, CControl* C2, UINT AAlign){
    switch(AAlign){
      case alTop: return C1->Top < C2->Top;
      case alBottom: return (C1->Top + C1->Height) >= (C2->Top + C2->Height);
      case alLeft: return C1->Left < C2->Left;
      case alRight: return (C1->Left + C1->Width) >= (C2->Left + C2->Width);
      case alCustom: return CustomAlignInsertBefore(C1, C2);
	}
	return FALSE;
}

BOOL CWinControl::CustomAlignInsertBefore(CControl* C1, CControl* C2){
	return FALSE;
}

void CWinControl::DoPosition(CControl* Control, UINT AAlign, TAlignInfo& AlignInfo, TRect& Rect){
	int NewLeft = 0;
	int NewTop = 0;
	int NewWidth = 0;
	int NewHeight = 0;
	//with Rect do
	//begin
	if ((AAlign == alNone) || (Control->Anchors != AnchorAlign[AAlign])){
		//with Control do
		if ((Control->OriginalParentSize.x != 0) && (Control->OriginalParentSize.y != 0)){
			int NewLeft = Control->Left;
			int NewTop = Control->Top;
			int NewWidth = Control->Width;
			int NewHeight = Control->Height;
			POINT ParentSize;
			if (Control->Parent->HandleAllocated())
				ParentSize = Control->Parent->GetClientRect().BottomRight;
			else {
				ParentSize.x = Control->Parent->Width;
				ParentSize.y = Control->Parent->Height;
			}
			if ((Control->Anchors & akRight) == akRight){
				if ((Control->Anchors & akLeft) == akLeft)
					// The AnchorRules.X is the original width
					NewWidth = ParentSize.x - (Control->OriginalParentSize.x - Control->AnchorRules.x);
				else
					// The AnchorRules.X is the original left
					NewLeft = ParentSize.x - (Control->OriginalParentSize.x - Control->AnchorRules.x);
			}
			else if (!((Control->Anchors & akLeft) == akLeft))
				// The AnchorRules.X is the original middle of the control
				NewLeft = MulDiv(Control->AnchorRules.x, ParentSize.x, Control->OriginalParentSize.x) - NewWidth / 2;
			if ((Control->Anchors & akBottom) == akBottom){
				if ((Control->Anchors & akTop) == akTop)
					// The AnchorRules.Y is the original height
					NewHeight = ParentSize.y - (Control->OriginalParentSize.y - Control->AnchorRules.y);
				else
					// The AnchorRules.Y is the original top
					NewTop = ParentSize.y - (Control->OriginalParentSize.y - Control->AnchorRules.y);
			}
			else if ((Control->Anchors & akTop) != akTop)
				// The AnchorRules.Y is the original middle of the control
				NewTop = MulDiv(Control->AnchorRules.y, ParentSize.x, Control->OriginalParentSize.y) - NewHeight / 2;
			Control->AnchorMove = TRUE;
			__try{
				Control->SetBounds(NewLeft, NewTop, NewWidth, NewHeight);
			}__finally{
				Control->AnchorMove = FALSE;
			}
		}
		if (AAlign == alNone)
			return;
	}

	NewWidth = Rect.right - Rect.left;
	if((NewWidth < 0) || (AAlign == alLeft || AAlign == alRight || AAlign == alCustom))
		NewWidth = Control->Width;
	NewHeight = Rect.bottom - Rect.top;
	if((NewHeight < 0) || (AAlign == alTop || AAlign == alBottom || AAlign == alCustom))
		NewHeight = Control->Height;
	NewLeft = Rect.left;
	NewTop = Rect.top;
	switch(AAlign){
	case alTop: Rect.top += NewHeight; break;
	case alBottom: Rect.bottom -= NewHeight; NewTop = Rect.bottom; break;
	case alLeft: Rect.left += NewWidth; break;
	case alRight: Rect.right -= NewWidth; NewLeft = Rect.right; break;
	case alCustom: NewLeft = Control->Left; NewTop = Control->Top; 
		CustomAlignPosition(Control, NewLeft, NewTop, NewWidth, NewHeight, Rect, AlignInfo);
		break;
	}
	Control->AnchorMove = TRUE;
	__try{
		Control->SetBounds(NewLeft, NewTop, NewWidth, NewHeight);
	}__finally{
		Control->AnchorMove = FALSE;
	}

	if ((Control->Width != NewWidth) || (Control->Height != NewHeight))
		switch(AAlign){
		case alTop: Rect.top -= NewHeight - Control->Height; break;
		case alBottom: Rect.bottom += NewHeight - Control->Height; break;
		case alLeft: Rect.left -= NewWidth - Control->Width; break;
		case alRight: Rect.right += NewWidth - Control->Width; break;
		case alClient: Rect.right += NewWidth - Control->Width;
			Rect.bottom += NewHeight - Control->Height;
			break;
	}
}

void CWinControl::CustomAlignPosition(CControl* Control, INT& NewLeft, INT& NewTop, INT& NewWidth, INT &NewHeight, 
	TRect& AlignRect, const TAlignInfo& AlignInfo){

}

void CWinControl::SetBounds(INT ALeft, INT ATop, INT AWidth, INT AHeight){
	WINDOWPLACEMENT WindowPlacement;
	if(ALeft != Left || ATop != Top || AWidth != Width || AHeight != Height){
		if(HandleAllocated() && !IsIconic(Wnd))
			SetWindowPos(Wnd, 0, ALeft, ATop, AWidth, AHeight, SWP_NOZORDER + SWP_NOACTIVATE);
		else {
			Left = ALeft;
			Top = ATop;
			Width = AWidth;
			Height = AHeight;
			if(HandleAllocated()){
				WindowPlacement.length = sizeof(WindowPlacement);
				GetWindowPlacement(Wnd, &WindowPlacement);
				WindowPlacement.rcNormalPosition = *((LPRECT)&GetBoundsRect());
				SetWindowPlacement(Wnd, &WindowPlacement);
			}
		}
		UpdateAnchorRules();
		RequestAlign();
	}
}

void CWinControl::WMWindowPosChanged(TWMWindowPosChanged& Message){
	BOOL Framed = Ctl3D && IN_TEST(csFramed, ControlStyle) && Parent != NULL &&
		!IN_TEST(SWP_NOREDRAW, Message.WindowPos->flags);
	BOOL Moved = !IN_TEST(SWP_NOMOVE, Message.WindowPos->flags) &&
		IsWindowVisible(Wnd);
	BOOL Sized = !IN_TEST(SWP_NOSIZE, Message.WindowPos->flags) &&
		IsWindowVisible(Wnd);
	if(Framed && (Moved || Sized))
		InvalidateFrame();
	if(!IN_TEST(csDestroyingHandle, ControlState))
		UpdateBounds();
	INHERITED_MSG(Message);
	if(Framed && (Moved || Sized || ((Message.WindowPos->flags & (SWP_SHOWWINDOW | SWP_HIDEWINDOW)) != 0)))
		InvalidateFrame();
}

void CWinControl::CMVisibleChanged(TMessage& Message){
	if(Visible && Parent != NULL)
		RemoveFocus(FALSE);
	if(!IN_TEST(csDesigning, GetComponentState()) || IN_TEST(csNoDesignVisible, ControlStyle))
		UpdateControlState();
}

void CWinControl::CMShowingChanged(TMessage& Message){
	UINT ShowFlags[] = {
		SWP_NOSIZE + SWP_NOMOVE + SWP_NOZORDER + SWP_NOACTIVATE + SWP_HIDEWINDOW,
		SWP_NOSIZE + SWP_NOMOVE + SWP_NOZORDER + SWP_NOACTIVATE + SWP_SHOWWINDOW
	};
	SetWindowPos(Wnd, 0, 0, 0, 0, 0, ShowFlags[Showing]);
}

void CWinControl::CMControlListChange(TMessage& Message){
	if(Parent != NULL)
		Parent->WndProc(Message);
}

void CWinControl::CMShowHintChanged(TMessage& Message){
	INHERITED_MSG(Message);
	NotifyControls(CM_PARENTSHOWHINTCHANGED);
}

void CWinControl::CMBiDiModeChanged(TMessage& Message){
	INHERITED_MSG(Message);
	if(GetGlobal().GetSysLocaleMiddleEast() && Message.wParam == 0)
		RecreateWnd();
	NotifyControls(CM_PARENTBIDIMODECHANGED);
}

void CWinControl::CMRecreateWnd(TMessage& Message){
	BOOL WasFocused = Focused();
	DestroyHandle();
	UpdateControlState();
	if(WasFocused && Wnd != 0)
		::SetFocus(Wnd);
}

void CWinControl::CMFontChanged(TMessage& Message){
	INHERITED_MSG(Message);
	if(HandleAllocated())
		Perform(WM_SETFONT, (WPARAM)Font->GetHandle(), 0);
	NotifyControls(CM_PARENTFONTCHANGED);
}

void CWinControl::CMDialogKey(TCMDialogKey& Message){
	Broadcast(*((PMessage)&Message));
}
void CWinControl::CMDialogChar(TCMDialogChar& Message){
	Broadcast(*((PMessage)&Message));
}

void CWinControl::CMColorChanged(TMessage& Message){
	INHERITED_MSG(Message);
	Brush->SetColor(Color);
	NotifyControls(CM_PARENTCOLORCHANGED);
}

void CWinControl::WMChar(TWMChar& Message){
	if(!DoKeyPress(Message))
		INHERITED_MSG(Message);
}

void CWinControl::CNKeyDown(TWMKeyDown& Message){
	Message.Result = 1;
	//TODO UpdateUIState(Message.CharCode);
    //if(IsMenuKey(Message)) return ;
    if(!IN_TEST(csDesigning, GetComponentState())){
		if(Perform(CM_CHILDKEY, Message.CharCode, (LPARAM)this) != 0) return;
		INT Mask = 0;
		if(Message.CharCode == VK_TAB)
			Mask = DLGC_WANTTAB;
		else if(Message.CharCode == VK_LEFT || Message.CharCode == VK_RIGHT || 
			Message.CharCode == VK_UP || Message.CharCode == VK_DOWN)
			Mask = DLGC_WANTARROWS;
		else if(Message.CharCode == VK_RETURN || Message.CharCode == VK_EXECUTE || 
			Message.CharCode == VK_ESCAPE || Message.CharCode == VK_CANCEL)
			Mask = DLGC_WANTALLKEYS;
		if(Mask != 0 && Perform(CM_WANTSPECIALKEY, Message.CharCode, 0) == 0 && 
			(Perform(WM_GETDLGCODE, 0, 0) & Mask) == 0 && GetParentForm(this)->Perform(CM_DIALOGKEY,
				Message.CharCode, Message.KeyData) != 0) 
			return;
	}
	Message.Result = 0;
}

void CWinControl::WMSetCursor(TWMSetCursor& Message){
	if(Message.CursorWnd == GetHandle()){
		if(Message.HitTest == HTCLIENT){
			TCursor Cursor1 = GetScreen()->GetCursor();
            if(Cursor1 == crDefault){
				TPoint P = {0, 0};
				GetCursorPos((LPPOINT)&P);
				CControl* AControl = ControlAtPos(ScreenToClient(P), FALSE);
				if(AControl != NULL)
					if(IN_TEST(csDesigning, AControl->GetComponentState()))
						Cursor1 = crArrow;
					else
						Cursor1 = AControl->GetCursor();
				if(Cursor1 == crDefault)
					if(IN_TEST(csDesigning, GetComponentState()))
						Cursor1 = crArrow;
					else
						Cursor1 = Cursor;
			}
            if(Cursor1 != crDefault){
				::SetCursor(GetScreen()->GetCursors(Cursor1));
				Message.Result = 1;
				return ;
			}
		}
		else if(Message.HitTest ==  HTERROR){
			/* TODO 
			if(Message.MouseMsg == WM_LBUTTONDOWN && GetGlobal()->GetAppHandle() != 0 && 
				GetForegroundWindow() != GetLastActivePopup(GetGlobal()->GetAppHandle())){
				GetGlobal()->BringAppToFront();//Application.BringToFront;
				return;
			}//*/
		}
	}
	INHERITED_MSG(Message);
}

void CWinControl::CNKeyUp(TWMKeyUp& Message){
	if(!IN_TEST(csDesigning, GetComponentState())){
		WORD CharCode = Message.CharCode;
		if(CharCode == VK_TAB ||
			CharCode == VK_LEFT || CharCode == VK_RIGHT || 
			CharCode == VK_UP || CharCode == VK_DOWN ||
			CharCode == VK_LEFT || CharCode == VK_RIGHT || 
			CharCode == VK_UP || CharCode == VK_DOWN)
			Message.Result = Perform(CM_WANTSPECIALKEY, CharCode, 0);
	}
}

void CWinControl::CNChar(TWMChar& Message){
	if(!IN_TEST(csDesigning, GetComponentState())){
		Message.Result = 1;
		if((Perform(WM_GETDLGCODE, 0, 0) & DLGC_WANTCHARS) == 0 &&
			GetParentForm(this)->Perform(CM_DIALOGCHAR,
				Message.CharCode, Message.KeyData) != 0)
			return ;
		Message.Result = 0;
	}
}

void CWinControl::CNSysKeyDown(TWMKeyDown& Message){
	Message.Result = 1;
	//TODO if(IsMenuKey(Message)) return;
	if(!IN_TEST(csDesigning, GetComponentState())){
		if(Perform(CM_CHILDKEY, Message.CharCode, (LPARAM)this) != 0) 
			return;
		if(GetParentForm(this)->Perform(CM_DIALOGKEY, Message.CharCode, Message.KeyData) != 0)
			return;
	}
	Message.Result = 0;
}

void CWinControl::CNSysChar(TWMChar& Message){
	if(!IN_TEST(csDesigning, GetComponentState())){
		if(Message.CharCode != VK_SPACE)
			Message.Result = GetParentForm(this)->Perform(CM_DIALOGCHAR,
				Message.CharCode, Message.KeyData);
	}
}

void CWinControl::WMCommand(TWMCommand& Message){
	if(!DoControlMsg(Message.Ctl, *((PMessage)&Message)))
		INHERITED_MSG(Message);
}
void CWinControl::WMNotify(TWMNotify& Message){
	if(!DoControlMsg(Message.NMHdr->hwndFrom, *((PMessage)&Message)))
		INHERITED_MSG(Message);
}

void CWinControl::CMEnabledChanged(TMessage& Message){
	if(!GetEnabled() && GetParent() != NULL)
		RemoveFocus(FALSE);
	if(HandleAllocated() && !IN_TEST(csDesigning, GetComponentState()))
		EnableWindow(GetHandle(), GetEnabled());
}

void CWinControl::WMHScroll(TWMHScroll& Message){
	if(!DoControlMsg(Message.ScrollBar, *((PMessage)&Message)))
		INHERITED_MSG(Message);
}

void CWinControl::WMVScroll(TWMVScroll& Message){
	if(!DoControlMsg(Message.ScrollBar, *((PMessage)&Message)))
		INHERITED_MSG(Message);
}

void CWinControl::WMCompareItem(TWMCompareItem& Message){
	if(!DoControlMsg((HWND)Message.CompareItemStruct->CtlID, *((PMessage)&Message)))
		INHERITED_MSG(Message);
}

void CWinControl::WMDeleteItem(TWMDeleteItem& Message){
	if(!DoControlMsg((HWND)Message.DeleteItemStruct->CtlID, *((PMessage)&Message)))
		INHERITED_MSG(Message);
}

void CWinControl::WMDrawItem(TWMDrawItem& Message){
	if(!DoControlMsg((HWND)Message.DrawItemStruct->CtlID, *((PMessage)&Message)))
		INHERITED_MSG(Message);
}

void CWinControl::WMMeasureItem(TWMMeasureItem& Message){
	if(!DoControlMsg((HWND)Message.MeasureItemStruct->CtlID, *((PMessage)&Message)))
		INHERITED_MSG(Message);
}

void CWinControl::CMFocusChanged(TCMFocusChanged& Message){
	Broadcast(*((PMessage)&Message));
}

void CWinControl::CMEnter(TCMEnter& Message){
	/*TODO
	if(Global::SysLocaleMiddleEast)
		if(UseRightToLeftReading()){
			if(Application->BiDiKeyboard != TEXT(""))
				LoadKeyboardLayout((LPTSTR)Application->BiDiKeyboard, KLF_ACTIVATE);
		}
		else if(Application->NonBiDiKeyboard != TEXT(""))
			LoadKeyboardLayout((LPTSTR)Application->NonBiDiKeyboard, KLF_ACTIVATE);
	//*/
	DoEnter();
}

void CWinControl::CMExit(TCMExit& Message){
	DoExit();
}

void CWinControl::CMBorderChanged(TMessage& Message){
	INHERITED_MSG(Message);
	if(HandleAllocated()){
		SetWindowPos(GetHandle(), 0, 0,0,0,0, SWP_NOACTIVATE |
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		if(GetVisible())
			Invalidate();
	}
}


void CWinControl::CMCursorChanged(TMessage& Message){
	TPoint P = {0, 0};
	if(GetCapture() == 0){
		GetCursorPos((LPPOINT)&P);
		if(FindDragTarget(P, FALSE) == this)
			Perform(WM_SETCURSOR, (WPARAM)GetHandle(), (LPARAM)HTCLIENT);
	}
}

void CWinControl::WMDestroy(TWMDestroy& Message){
	INHERITED_MSG(Message);
	//TODO RemoveProp(Wnd, MakeIntAtom(ControlAtom));
	//RemoveProp(Wnd, MakeIntAtom(WindowAtom));
}

void CWinControl::WMNCDestroy(TWMNCDestroy& Message){
	INHERITED_MSG(Message);
	Wnd = 0;
	Showing = FALSE;
}

void CWinControl::CMInvalidate(TMessage& Message){
	if(HandleAllocated()){
		if(GetParent() != NULL)
			GetParent()->Perform(CM_INVALIDATE, 1, 0);
		if(Message.wParam == 0){
			InvalidateRect(Wnd, NULL, !IN_TEST(csOpaque, GetControlStyle()));
			//Invalidate child windows which use the parentbackground when themed
			/* TODO
			if(ThemeServices->GetThemesEnabled())
				for(INT I = 0; I < GetControlCount(); I++)
					if(IN_TEST(csParentBackground, GetControl(I)->GetControlStyle()))
						GetControl(I)->Invalidate();
			//*/
		}
	}
}

void CWinControl::WMSize(TWMSize& Message){
	UpdateBounds();
	INHERITED_MSG(Message);
	Realign();
	if(!IN_TEST(csLoading, GetComponentState()))
		Resize();
}

void CWinControl::WMMove(TWMMove& Message){
	INHERITED_MSG(Message);
	UpdateBounds();
}

void CWinControl::WMContextMenu(TWMContextMenu& Message){
	if(Message.Result != 0)
		return ;
	CControl* Ctrl = ControlAtPos(ScreenToClient(SmallPointToPoint(Message.Pos)), FALSE);
	if(Ctrl != NULL)
		Message.Result = Ctrl->Perform(WM_CONTEXTMENU, 0, Message.lParam);
	if(Message.Result == 0)
		INHERITED_MSG(Message);
}

void CWinControl::DoEnter(){
	if(OnEnter != NULL)
		CALL_EVENT(Enter)(this);
}

void CWinControl::DoExit(){
	if(OnExit != NULL)
		CALL_EVENT(Exit)(this);
}

void CWinControl::WMEraseBkgnd(TWMEraseBkgnd& Message){
	//with ThemeServices do
	//	if ThemesEnabled and Assigned(Parent) and (csParentBackground in FControlStyle) then begin
	//		{ Get the parent to draw its background into the control's background. }
	//		DrawParentBackground(Handle, Message.DC, nil, False);
	//	end
    //else
	{
     // { Only erase background if we're not doublebuffering or painting to memory. }
      if(!DoubleBuffered || ((PMessage)&Message)->wParam == ((PMessage)&Message)->lParam)
		  FillRect(Message.DC, (const RECT *)&GetClientRect(), Brush->GetHandle());
	}
	Message.Result = TRUE;
}

void CWinControl::WMPaint(TWMPaint& Message){
	if(!DoubleBuffered || Message.DC != 0){
		if(!IN_TEST(csCustomPaint, ControlState) && GetControlCount() == 0)
			INHERITED_MSG(Message);
		else
			PaintHandler(Message);
	}
	else{
		HDC DC = GetDC(0);
		TRect ClientRect = GetClientRect();
		HBITMAP MemBitmap = CreateCompatibleBitmap(DC, ClientRect.right, ClientRect.bottom);
		ReleaseDC(0, DC);
		HDC MemDC = CreateCompatibleDC(0);
		HBITMAP OldBitmap = (HBITMAP)SelectObject(MemDC, MemBitmap);
		__try{
			PAINTSTRUCT PS;
			DC = BeginPaint(GetHandle(), &PS);
			Perform(WM_ERASEBKGND, (WPARAM)MemDC, (LPARAM)MemDC);
			Message.DC = MemDC;
			WMPaint(Message);
			Message.DC = 0;
			BitBlt(DC, 0, 0, ClientRect.right, ClientRect.bottom, MemDC, 0, 0, SRCCOPY);
			EndPaint(GetHandle(), &PS);
		}
		__finally{
			SelectObject(MemDC, OldBitmap);
			DeleteDC(MemDC);
			DeleteObject(MemBitmap);
		}
	}
}

void CWinControl::PaintHandler(TWMPaint& Message){
	PAINTSTRUCT PS;
	HDC DC = Message.DC;
	if(DC == 0)
		DC = BeginPaint(GetHandle(), &PS);
	__try{
		if(Controls == NULL)
			PaintWindow(DC);
		else {
			INT SaveIndex = SaveDC(DC);
			INT Clip = SIMPLEREGION;
			INT Count = Controls->GetCount();
			for(INT I = 0; I < Count; I++){
				CControl* Control = (CControl *)Controls->Get(I);
				if((Control->GetVisible() || IN_TEST(csDesigning, Control->GetComponentState()) &&
					!IN_TEST(csNoDesignVisible, Control->GetControlStyle())) &&
					IN_TEST(csOpaque, Control->GetControlStyle())){
					Clip = ExcludeClipRect(DC, Control->GetLeft(), Control->GetTop(), 
						Control->GetLeft() + Control->GetWidth(), Control->GetTop() + Control->GetHeight());
					if(Clip == NULLREGION)
						break;
				}
			}
			if(Clip != NULLREGION)
				PaintWindow(DC);
			RestoreDC(DC, SaveIndex);
		}
		PaintControls(DC, NULL);
	}
	__finally{
		if(Message.DC == 0)
			EndPaint(GetHandle(), &PS);
	}
}

void CWinControl::PaintWindow(HDC DC){
	TMessage Message;
	Message.Msg = WM_PAINT;
	Message.wParam = (WPARAM)DC;
	Message.lParam = 0;
	Message.Result = 0;
	DefaultHandler(Message);
}

void CWinControl::PaintControls(HDC DC, CControl* First){
//	if DockSite and UseDockManager and (DockManager <> nil) then
//		DockManager.PaintSite(DC);
	if(Controls != NULL){
		INT I = 0;
		if(First != NULL){
			I = Controls->IndexOf(First);
			if(I < 0) I = 0;
		}
		INT Count = Controls->GetCount();
		while(I < Count){
			CControl* Control = (CControl *)Controls->Get(I);
			if ((Control->GetVisible() || IN_TEST(csDesigning, Control->GetComponentState()) &&
				!IN_TEST(csNoDesignVisible, Control->GetControlStyle())) &&
				RectVisible(DC, (LPRECT)&Rect(Control->GetLeft(), Control->GetTop(), 
					Control->GetLeft() + Control->GetWidth(), Control->GetTop() + Control->GetHeight()))){
				if(IN_TEST(csPaintCopy, this->GetControlState()))
					Control->ControlState |= csPaintCopy;
				INT SaveIndex = SaveDC(DC);
				MoveWindowOrg(DC, Control->GetLeft(), Control->GetTop());
				IntersectClipRect(DC, 0, 0, Control->GetWidth(), Control->GetHeight());
				Control->Perform(WM_PAINT, (WPARAM)DC, 0);
				RestoreDC(DC, SaveIndex);
				Control->ControlState &= ~csPaintCopy;
			}
			I++;
		}
	}
	if(WinControls != NULL){
		INT Count = WinControls->GetCount();
		for(INT I = 0; I < Count; I++){
			CWinControl* Control = (CWinControl *)WinControls->Get(I);
			if(Control->Ctl3D && IN_TEST(csFramed, Control->GetControlStyle()) &&
				(Control->GetVisible() || IN_TEST(csDesigning, Control->GetComponentState()) &&
				!IN_TEST(csNoDesignVisible, Control->GetControlStyle()))){
				HBRUSH FrameBrush = CreateSolidBrush(ColorToRGB(clBtnShadow));
				FrameRect(DC, (LPRECT)&Rect(Control->GetLeft() - 1, Control->GetTop() - 1, 
					Control->GetLeft() + Control->GetWidth(), Control->GetTop() + Control->GetHeight()), FrameBrush);
				DeleteObject(FrameBrush);
				FrameBrush = CreateSolidBrush(ColorToRGB(clBtnHighlight));
				FrameRect(DC, (LPRECT)&Rect(Control->GetLeft(), Control->GetTop(), 
					Control->GetLeft() + Control->GetWidth() + 1, Control->GetTop() + Control->GetHeight() + 1), FrameBrush);
				DeleteObject(FrameBrush);
			}
		}
	}
}

BOOL CWinControl::Focused(){
	return Wnd != 0 && ::GetFocus() == Wnd;
}

void CWinControl::SetFocus(){
	CForm* Parent = GetParentForm(this);
	if(Parent != NULL)
		Parent->FocusControl(this);
	else if(GetParentWindow() != 0)
		::SetFocus(GetHandle());
	else
		ValidParentForm(this);
}

BOOL CWinControl::CanFocus(){
	BOOL Result = FALSE;
	CForm* Form = GetParentForm(this);
	if(Form != NULL){
		CWinControl* Control = this;
		while(Control != Form){
			if(!(Control->Visible && Control->GetEnabled()))
				return Result;
			Control = Control->GetParent();
		}
		Result = TRUE;
	}
	return Result;
}

void CWinControl::NotifyControls(UINT Msg){
	TMessage Message;
	Message.Msg = Msg;
	Message.wParam = 0;
	Message.lParam = 0;
	Message.Result = 0;
	Broadcast(Message);
}

void CWinControl::Broadcast(TMessage& Message){
	INT Count = GetControlCount();
	for(INT I = 0; I < Count; I++){
		GetControl(I)->WndProc(Message);
		if(Message.Result != 0)
			return;
	}
}

void CWinControl::Update(){
	if(HandleAllocated())
		UpdateWindow(Wnd);
}

void CWinControl::Invalidate(){
	Perform(CM_INVALIDATE, 0, 0);
}

void CWinControl::Repaint(){
	Invalidate();
	Update();
}

void CWinControl::ShowControl(CControl* AControl){
	if(Parent != NULL)
		Parent->ShowControl(this);
}

void CWinControl::SetZOrder(BOOL TopMost){
	HWND WindowPos[] = {HWND_BOTTOM, HWND_TOP};
	INT N = 0;
	INT M = 0;
	if(Parent != NULL){
		if(TopMost)
			N = Parent->WinControls->GetCount() - 1;
		else N = 0;
		if(Parent->Controls != NULL)
			M = Parent->Controls->GetCount();
		SetZOrderPosition(M + N);
	}
	else
		if(Wnd != 0)
			SetWindowPos(Wnd, WindowPos[TopMost], 0, 0, 0, 0,
				SWP_NOMOVE + SWP_NOSIZE);
}

void CWinControl::UpdateControlState(){
	CWinControl* Control = this;
	while(Control->Parent != NULL){
		Control = Control->Parent;
		if(!(Control->Showing))
			return ;
	}
	if((Control->InstanceOf(CForm::_Class) || Control->ParentWindow != 0))
		UpdateShowing();
}

void CWinControl::UpdateShowing(){
	BOOL ShowControl = (Visible || IN_TEST(csDesigning,GetComponentState()) &&
		!IN_TEST(csNoDesignVisible, ControlStyle)) && !IN_TEST(csReadingState, ControlState);
	if(ShowControl){
		if(Wnd == 0)
			CreateHandle();
		if(WinControls != NULL){
			INT Count = WinControls->GetCount();
			for(INT i = 0; i < Count; i++)
				((CWinControl *)WinControls->Get(i))->UpdateShowing();
		}
	}
	if(Wnd != 0 && Showing != ShowControl){
		Showing = ShowControl;
		__try{
			Perform(CM_SHOWINGCHANGED, 0, 0);
		}
		__except(EXCEPTION_EXECUTE_HANDLER){
			Showing = !ShowControl;
			throw ;
		}
	}
}

void CWinControl::RemoveFocus(BOOL Removing){
	//Form = GetParentForm(this);
	//if (Form != NULL)
	//	Form.DefocusControl(this, Removing);
}

void CWinControl::InvalidateFrame(){
	TRect R = GetBoundsRect();
	InflateRect((LPRECT)&R, 1, 1);
	InvalidateRect(GetParent()->Wnd, (LPRECT)&R, TRUE);
}

void CWinControl::UpdateBounds(){
	HWND ParentHandle;
	RECT Rect;
	WINDOWPLACEMENT WindowPlacement;
	if(IsIconic(Wnd)){
		WindowPlacement.length = sizeof(WindowPlacement);
		GetWindowPlacement(Wnd, &WindowPlacement);
		Rect = WindowPlacement.rcNormalPosition;
	}
	else
		GetWindowRect(Wnd, &Rect);
	if(IN_TEST(WS_CHILD, GetWindowLongPtr(Wnd, GWL_STYLE))){
		ParentHandle = (HWND)GetWindowLongPtr(Wnd, GWLP_HWNDPARENT);
		if(ParentHandle != 0){
			PRect pRect = (PRect)&Rect;
			::ScreenToClient(ParentHandle, &(pRect->TopLeft));
			::ScreenToClient(ParentHandle, &(pRect->BottomRight));
		}
	}
	Left = Rect.left;
	Top = Rect.top;
	Width = Rect.right - Rect.left;
	Height = Rect.bottom - Rect.top;
	UpdateAnchorRules();
}

void CWinControl::InsertControl(CControl* AControl){
	//AControl->ValidateContainer(this);
	Perform(CM_CONTROLLISTCHANGE, (WPARAM)AControl, (LPARAM)TRUE);
	Insert(AControl);
	if(!IN_TEST(csReading, AControl->GetComponentState())){
		AControl->Perform(CM_PARENTCOLORCHANGED, 0, 0);
		AControl->Perform(CM_PARENTFONTCHANGED, 0, 0);
		AControl->Perform(CM_PARENTSHOWHINTCHANGED, 0, 0);
		AControl->Perform(CM_PARENTBIDIMODECHANGED, 0, 0);
		if(AControl->InstanceOf(CWinControl::_Class)){
			AControl->Perform(CM_PARENTCTL3DCHANGED, 0, 0);
			UpdateControlState();
		}
		else if(HandleAllocated())
			AControl->Invalidate();
		AlignControl(AControl);
	}
	Perform(CM_CONTROLCHANGE, (WPARAM)AControl, (LPARAM)TRUE);
}
void CWinControl::RemoveControl(CControl* AControl){
	Perform(CM_CONTROLCHANGE, (WPARAM)AControl, (LPARAM)FALSE);
	if(AControl->InstanceOf(CWinControl::_Class)){
		CWinControl* winControl = (CWinControl *)AControl;
		winControl->RemoveFocus(TRUE);
		winControl->DestroyHandle();
	}
	else if(HandleAllocated())
		AControl->InvalidateControl(AControl->Visible, FALSE);
	Remove(AControl);
	Perform(CM_CONTROLLISTCHANGE, (WPARAM)AControl, (LPARAM)FALSE);
	Realign();
}

//List helpers
CList* ListAdd(CList* List, LPVOID Item){
	if(List == NULL)
		List = new CList();
	List->Add(Item);
	return List;
}

CList* ListRemove(CList* List, LPVOID Item){
	List->Remove(Item);
	if(List->GetCount() == 0){
		delete List;
		List = NULL;
	}
	return List;
}

void CWinControl::Insert(CControl* AControl){
	if(AControl != NULL){
		if(AControl->InstanceOf(CWinControl::_Class)){
			WinControls = ListAdd(WinControls, AControl);
			TabList = ListAdd(TabList, AControl);
		}
		else
			Controls = ListAdd(Controls, AControl);
		AControl->Parent = this;
	}
}

void CWinControl::Remove(CControl* AControl){
	if(AControl != NULL && AControl->InstanceOf(CWinControl::_Class)){
		TabList = ListRemove(TabList, AControl);
		WinControls = ListRemove(WinControls, AControl);
	}
	else
		Controls = ListRemove(Controls, AControl);
	AControl->Parent = NULL;
}

void CWinControl::DestroyHandle(){
	if(Wnd != 0){
		if(WinControls != NULL){
			INT Count = WinControls->GetCount();
			for(INT I = 0; I < Count; I++)
				((CWinControl *)WinControls->Get(I))->DestroyHandle();
		}
		DestroyWnd();
	}
}

void CWinControl::DestroyWnd(){
	INT Len = GetTextLen();
	if(Len < 1)
		Text = NULL;
	else{
		INT Size = (Len + 1) * sizeof(TCHAR);
		Text = (LPTSTR)malloc(Size);
		GetTextBuf(Text, Size);
	}
	//FreeDeviceContexts;
	DestroyWindowHandle();
}

void CWinControl::DestroyWindowHandle(){
	ControlState |= csDestroyingHandle;
	__try{
		//TCHAR g[64];
		//ZeroMemory(g, sizeof(g));
		//MessageBox(0, TEXT("Destroy Wnd"), _itot((INT)Wnd, g, 10),0);
		if(!::DestroyWindow(Wnd))
			RaiseLastOSError();
	}
	__finally{
		ControlState &= ~csDestroyingHandle;
	}
	Wnd = 0;
}

void CWinControl::RecreateWnd(){
	if(Wnd != 0)
		Perform(CM_RECREATEWND, 0, 0);
}

VOID CWinControl::SetCtl3D(BOOL Value){
	if(Ctl3D != Value){
		Ctl3D = Value;
		ParentCtl3D = FALSE;
		Perform(CM_CTL3DCHANGED, 0, 0);
	}
}

VOID CWinControl::SetParentCtl3D(BOOL Value){
	if(ParentCtl3D != Value){
		ParentCtl3D = Value;
		if(Parent != NULL && !IN_TEST(csReading, GetComponentState()))
			Perform(CM_PARENTCTL3DCHANGED, 0, 0);
	}
}
VOID CWinControl::SetParentWindow(HWND Value){
	if(Parent == NULL && ParentWindow != Value){
		if(Wnd != 0 && ParentWindow != 0 && Value != 0){
			ParentWindow = Value;
			::SetParent(Wnd, Value);
			if(GetGlobal().GetWin32MajorVersion() >= 5 && GetGlobal().GetWin32Platform() == VER_PLATFORM_WIN32_NT)
				Perform(WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0);
		}
		else{
			DestroyHandle();
			ParentWindow = Value;
		}
		UpdateControlState();
	}
}

VOID CWinControl::SetTabStop(BOOL Value){
	DWORD Style = 0;
	if(TabStop != Value){
		TabStop = Value;
		if(HandleAllocated()){
			Style = GetWindowLongPtr(Wnd, GWL_STYLE) & (~WS_TABSTOP);
			if(Value)
				Style |= WS_TABSTOP;
			SetWindowLongPtr(Wnd, GWL_STYLE, Style);
		}
		Perform(CM_TABSTOPCHANGED, 0, 0);
	}
}

INT CWinControl::GetTabOrder(){
	if(Parent != NULL)
		return Parent->TabList->IndexOf(this);
	else
		return -1;
}

VOID CWinControl::SetTabOrder(INT Value){
	if(IN_TEST(csReadingState, GetControlState()))
		TabOrder = Value;
	else
		UpdateTabOrder(Value);
}
void CWinControl::UpdateTabOrder(INT Value){
	INT CurIndex = GetTabOrder();
	if(CurIndex >= 0){
		INT Count = Parent->TabList->GetCount();
		if(Value < 0)
			Value = 0;
		if(Value >= Count)
			Value = Count - 1;
		if(Value != CurIndex){
			Parent->TabList->Delete(CurIndex);
			Parent->TabList->Insert(Value, this);
		}
	}
}

BOOL CWinControl::GetParentBackground(){
	return IN_TEST(csParentBackground, GetControlStyle());
}

void CWinControl::SetParentBackground(BOOL Value){
	if(GetParentBackground() != Value){
		if(Value)
			SetControlStyle(GetControlStyle() | csParentBackground);
		else
			SetControlStyle(GetControlStyle() & (~csParentBackground));
		Invalidate();
	}
}

void CWinControl::SetBevelInner(TBevelCut Value){
	if(Value != BevelInner){
		BevelInner = Value;
		Perform(CM_BORDERCHANGED, 0, 0);
	}
}

void CWinControl::SetBevelOuter(TBevelCut Value){
	if(Value != BevelOuter){
		BevelOuter = Value;
        Perform(CM_BORDERCHANGED, 0, 0);
	}
}

void CWinControl::SetBevelEdges(TBevelEdges Value){
	if(Value != BevelEdges){
		BevelEdges = Value;
		Perform(CM_BORDERCHANGED, 0, 0);
	}
}

void CWinControl::SetBevelKind(TBevelKind Value){
	if(Value != BevelKind){
		BevelKind = Value;
		Perform(CM_BORDERCHANGED, 0, 0);
	}
}

void CWinControl::SetBevelWidth(TBevelWidth Value){
	if(Value != BevelWidth){
		BevelWidth = Value;
		Perform(CM_BORDERCHANGED, 0, 0);
	}
}

void CWinControl::SetBorderWidth(TBorderWidth Value){
	if(BorderWidth != Value){
		BorderWidth = Value;
		Perform(CM_BORDERCHANGED, 0, 0);
	}
}


IMPL_DYN_CLASS(CCustomControl)
CCustomControl::CCustomControl(CComponent* AOwner):CWinControl(AOwner), Canvas(NULL){
	Canvas = new CControlCanvas();
	((CControlCanvas *)Canvas)->SetControl(this);
}

CCustomControl::~CCustomControl(){
	delete Canvas;
}

void CCustomControl::WMPaint(TWMPaint& Message){
	SetControlState(GetControlState() | csCustomPaint);
	INHERITED_MSG(Message);
	SetControlState(GetControlState() & (~csCustomPaint));
}
	
void CCustomControl::Paint(){
}

void CCustomControl::PaintWindow(HDC DC){
	CMethodLock Lock(Canvas, (TLockMethod)&CCanvas::Lock, (TLockMethod)&CCanvas::UnLock);
	Canvas->SetHandle(DC);
	CanvasHandleCleaner cnvasCleaner(Canvas);
	((CControlCanvas *)Canvas)->UpdateTextFlags();
	Paint();
}

IMPL_DYN_CLASS(CCustomListControl)
CCustomListControl::CCustomListControl(CComponent* AOwner):CWinControl(AOwner){
}

CCustomListControl::~CCustomListControl(){
}

void CCustomListControl::MoveSelection(CCustomListControl* Destination){
	CopySelection(Destination);
	DeleteSelected();
}

IMPL_DYN_CLASS(CCustomMultiSelectListControl)
CCustomMultiSelectListControl::CCustomMultiSelectListControl(CComponent* AOwner):CCustomListControl(AOwner),
	MultiSelect(FALSE){
}

CCustomMultiSelectListControl::~CCustomMultiSelectListControl(){
}