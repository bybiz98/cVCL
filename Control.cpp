
#include "stdinc.h"
#include "SysInit.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "WinUtils.hpp"
#include "Form.hpp"

CWinControl* ObjectFromHWnd(HWND Handle){
	CWinControl* Result = NULL;
	DWORD OwningProcess = 0;
	if(GetWindowThreadProcessId(Handle, &OwningProcess) != 0 && OwningProcess == GetCurrentProcessId())
		Result = (CWinControl*)SendMessage(Handle, GetGlobal().GetRM_GetObjectInstance(), 0, 0);
	else
		Result = NULL;
	return Result;
}

CWinControl* FindVCLWindow(TPoint& Pos){
	HWND Handle = WindowFromPoint(Pos);
	CWinControl* Result = NULL;
	while(Handle != 0) {
		Result = FindControl(Handle);
		if(Result != NULL)
			return Result;
		Handle = GetParent(Handle);
	}
	return Result;
}

CControl* FindDragTarget(TPoint& Pos, BOOL AllowDisabled){
	CControl* Result = NULL;
	CWinControl* Window = FindVCLWindow(Pos);
	if(Window != NULL){
		Result = Window;
		CControl* Control = Window->ControlAtPos(Window->ScreenToClient(Pos), AllowDisabled);
		if(Control != NULL)
			Result = Control;
	}
	return Result;
}

CWinControl* FindControl(HWND Handle){
	DWORD OwningProcess = 0;
	CWinControl* Result = NULL;
	if(Handle != 0 && GetWindowThreadProcessId(Handle, &OwningProcess) != 0 
		&& OwningProcess == GetCurrentProcessId()){
		//if(GlobalFindAtom(PChar(ControlAtomString)) == ControlAtom)
		//	Result = Pointer(GetProp(Handle, MakeIntAtom(ControlAtom)))
		//else
			Result = ObjectFromHWnd(Handle);
	}
	return Result;
}

CControl *CaptureControl = NULL;

CControl* GetCaptureControl(){
	CControl* Result = FindControl(GetCapture());
	if(Result != NULL && CaptureControl != NULL && (CControl *)CaptureControl->GetParent() == Result)
		Result = CaptureControl;
	return Result;
}

void SetCaptureControl(CControl* Control){
	ReleaseCapture();
	CaptureControl = NULL;
	if(Control != NULL){
		if(!Control->InstanceOf(CWinControl::_Class)){
			if(Control->GetParent() == NULL)
				return ;
			CaptureControl = Control;
			Control = (CControl *)Control->GetParent();
		}
		SetCapture(((CWinControl *)Control)->GetHandle());
	}
}

void ChangeBiDiModeAlignment(TAlignment& Alignment){
	switch(Alignment){
		case taLeftJustify:  
			Alignment = taRightJustify; 
			break;
		case taRightJustify: 
			Alignment = taLeftJustify;
			break;
	}
}

//Form utility functions
CForm* GetParentForm(CControl* Control){
	CForm* Result = NULL;
	while(Control->GetParent() != NULL)
		Control = Control->GetParent();
	if(Control->InstanceOf(CForm::_Class))
		Result = (CForm *)Control;
	return Result;
}

CForm* ValidParentForm(CControl* Control){
	CForm* Result = GetParentForm(Control);
	if(Result == NULL)
		throw "parent is required.";//TODO EInvalidOperation.CreateFmt(SParentRequired, [Control.Name]);
	return Result;
}

UINT AnchorAlign[] = {
	akLeft | akTop,
    akLeft | akTop | akRight,
    akLeft | akRight | akBottom,
    akLeft | akTop | akBottom,
    akRight | akTop | akBottom,
    akLeft | akTop | akRight | akBottom,
    akLeft | akTop
};

IMPL_DYN_CLASS(CControl)
CControl::CControl(CComponent* AOwner):CMsgTarget(AOwner),
	Parent(NULL),
	Text(NULL),
	Left(CW_USEDEFAULT),
	Top(0),
	Width(CW_USEDEFAULT),
	Height(0),
	MinWidth(0),
	MinHeight(0),
	MaxWidth(0),
	MaxHeight(0),
	Visible(TRUE),
	Enabled(TRUE),
	Align(alNone),
	Anchors(akLeft | akTop),
	AutoSize(FALSE),
	ControlStyle(csCaptureMouse | csClickEvents | csSetCaption | csDoubleClicks),
	ControlState(0),
	Color(clWindow),
	Cursor(crDefault),
	BiDiMode(bdLeftToRight),
	AnchorMove(FALSE),
	ShowHint(TRUE),
	ParentColor(TRUE),
	ParentFont(TRUE),
	ParentShowHint(TRUE),
	ParentBiDiMode(TRUE),
	WheelAccumulator(0),
	INIT_EVENT(CanResize),
	INIT_EVENT(ConstrainedResize),
	INIT_EVENT(Resize),
	INIT_EVENT(Click),
	INIT_EVENT(DblClick),
	INIT_EVENT(MouseDown),
	INIT_EVENT(MouseMove),
	INIT_EVENT(MouseUp),
	INIT_EVENT(MouseWheel),
	INIT_EVENT(MouseWheelDown),
	INIT_EVENT(MouseWheelUp),
	INIT_EVENT(ContextPopup){
	Font = new CFont();
	Font->SetOnChange(this, (TNotifyEvent)&CControl::FontChanged);
	OriginalParentSize.x = 0;
	OriginalParentSize.y = 0;
	AnchorRules.x = 0;
	AnchorRules.y = 0;
}

CControl::~CControl(){
	//MessageBox(0, TEXT("Control"), TEXT("CLOSE"), 0);
	delete Font;
	if(Text != NULL){
		free(Text);
		Text = NULL;
	}
}

BYTE KeyboardStateToShiftState(const LPBYTE KeyboardState){
	BYTE Result = 0;
	if((*(KeyboardState + VK_SHIFT) & 0x80) != 0)	
		Result |= ssShift;
	if((*(KeyboardState + VK_CONTROL) & 0x80) != 0)	
		Result |= ssCtrl;
	if((*(KeyboardState + VK_MENU) & 0x80) != 0)	
		Result |= ssAlt;
	if((*(KeyboardState + VK_LBUTTON) & 0x80) != 0)	
		Result |= ssLeft;
	if((*(KeyboardState + VK_RBUTTON) & 0x80) != 0)	
		Result |= ssRight;
	if((*(KeyboardState + VK_MBUTTON) & 0x80) != 0)	
		Result |= ssMiddle;
	return Result;
}

VOID CControl::MouseWheelHandler(TMessage& Message){
	CForm* Form = GetParentForm(this);
	if(Form != NULL && Form != this)
		Form->MouseWheelHandler(TMessage(Message));
	else //with TMessage(Message) do
		Message.Result = Perform(CM_MOUSEWHEEL, Message.wParam, Message.lParam);
}

void CControl::WndProc(TMessage& Message){
	if(Message.Msg >= WM_MOUSEFIRST && Message.Msg <= WM_MOUSELAST){
		if(!IN_TEST(csDoubleClicks, ControlStyle) && 
			(Message.Msg == WM_LBUTTONDBLCLK || Message.Msg == WM_RBUTTONDBLCLK || Message.Msg == WM_MBUTTONDBLCLK))
			Message.Msg -= WM_LBUTTONDBLCLK - WM_LBUTTONDOWN;
		switch(Message.Msg){
			case WM_MOUSEMOVE:
				//Application.HintMouseMessage(this, Message);
				break;
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
				ControlState |= csLButtonDown;
				break;
			case WM_LBUTTONUP:
				ControlState &= ~csLButtonDown;
				break;
			default:
				if(GetGlobal().GetMouseWheelPresent() && GetGlobal().GetRegMouseWheelMessage() != 0 && 
					Message.Msg == GetGlobal().GetRegMouseWheelMessage()){
					BYTE KeyState[255];
					GetKeyboardState(KeyState);
					TCMMouseWheel WheelMsg;
					WheelMsg.Msg = Message.Msg;
					WheelMsg.ShiftState = KeyboardStateToShiftState(KeyState);
					WheelMsg.WheelDelta = (SHORT)Message.wParam;
					WheelMsg.Pos = MAKEPOINTS(Message.lParam);
					MouseWheelHandler(*((PMessage)&WheelMsg));
					return;
				}
		}
	}
	//else if(Message.Msg == CM_VISIBLECHANGED)
		//SendDockNotification(Message.Msg, Message.wParam, Message.lParam);
	Dispatch(Message);
}

void CControl::DefaultHandler(TMessage& Message){
	LPTSTR P = NULL;
	INT MaxLen = 0;
	switch(Message.Msg){
		case WM_GETTEXT:
			MaxLen = (INT)Message.wParam / sizeof(TCHAR);
			if (Text != NULL){
				P = lstrcpyn((LPTSTR)Message.lParam, Text, MaxLen);
			}
			else{
				P = lstrcpyn((LPTSTR)Message.lParam, TEXT(""), MaxLen);
			}
			if(P == NULL){
				Message.Result = 0;
			}
			else{
				Message.Result = lstrlen(P);
			}
			break;
		case WM_GETTEXTLENGTH:
			if(Text == NULL){
				Message.Result = 0;
			}
			else{
				Message.Result = lstrlen(Text);
			}
			break;
		case WM_SETTEXT:
			Message.Result = FALSE;
			MaxLen = lstrlen((LPTSTR)Message.lParam) + 1;
			P = (LPTSTR)malloc(MaxLen * sizeof(TCHAR));
			if(P == NULL){
				return ;
			}
			LPTSTR P1 = lstrcpyn(P, (LPTSTR)Message.lParam, MaxLen);
			if(P1 == NULL){
				free(P);
				return ;
			}
			if(Text != NULL)
				free(Text);
			Text = P;
			Message.Result = TRUE;
			//SendDockNotification(Msg, WParam, LParam);
			break;
	}
}

void CControl::WMLButtonDown(TWMLButtonDown& Message){
	SendCancelMode(this);
	INHERITED_MSG(Message);
	if(IN_TEST(csCaptureMouse, GetControlStyle()))
		SetMouseCapture(TRUE);
	if(IN_TEST(csClickEvents, GetControlStyle()))
		ControlState |= csClicked;
	DoMouseDown(Message, mbLeft, 0);
}

void CControl::WMNCLButtonDown(TWMNCLButtonDown& Message){
	SendCancelMode(this);
	INHERITED_MSG(Message);
}

void CControl::WMRButtonDown(TWMRButtonDown& Message){
	INHERITED_MSG(Message);
	DoMouseDown(Message, mbRight, 0);
}

void CControl::WMMButtonDown(TWMMButtonDown& Message){
	INHERITED_MSG(Message);
	DoMouseDown(Message, mbMiddle, 0);
}

void CControl::WMLButtonDblClk(TWMLButtonDblClk& Message){
	SendCancelMode(this);
	INHERITED_MSG(Message);
	if(IN_TEST(csCaptureMouse, GetControlStyle()))
		SetMouseCapture(TRUE);
	if(IN_TEST(csClickEvents, GetControlStyle()))
		DblClick();
	DoMouseDown(Message, mbLeft, ssDouble);
}

void CControl::WMRButtonDblClk(TWMRButtonDblClk& Message){
	INHERITED_MSG(Message);
	DoMouseDown(Message, mbRight, ssDouble);
}

void CControl::WMMButtonDblClk(TWMMButtonDblClk& Message){
	INHERITED_MSG(Message);
	DoMouseDown(Message, mbMiddle, ssDouble);
}

void CControl::WMMouseMove(TWMMouseMove& Message){
	INHERITED_MSG(Message);
	if(!IN_TEST(csNoStdEvents, GetControlStyle()))
		if(GetWidth() > 32768 || GetHeight() > 32768){
			TPoint pt = CalcCursorPos();
			MouseMove(KeysToShiftState((WORD)Message.Keys), pt.x, pt.y);
		}
		else
			MouseMove(KeysToShiftState((WORD)Message.Keys), Message.XPos, Message.YPos);

}

void CControl::WMLButtonUp(TWMLButtonUp& Message){
	INHERITED_MSG(Message);
	if(IN_TEST(csCaptureMouse, GetControlStyle()))
		SetMouseCapture(FALSE);
	if(IN_TEST(csClicked, GetControlState())){
		ControlState &= ~csClicked;
		if(PtInRect((const RECT *)&GetClientRect(), SmallPointToPoint(Message.Pos)))
			Click();
	}
	DoMouseUp(Message, mbLeft);
}

void CControl::WMRButtonUp(TWMRButtonUp& Message){
	INHERITED_MSG(Message);
	DoMouseUp(Message, mbRight);
}

void CControl::WMMButtonUp(TWMMButtonUp& Message){
	INHERITED_MSG(Message);
	DoMouseUp(Message, mbMiddle);
}

void CControl::WMCancelMode(TWMCancelMode& Message){
	INHERITED_MSG(Message);
	if(GetMouseCapture()){
		SetMouseCapture(FALSE);
		if(IN_TEST(csLButtonDown, GetControlState()))
			Perform(WM_LBUTTONUP, 0, INT(0xFFFFFFFF));
	}
	else 
		ControlState &= ~csLButtonDown;
}

void CControl::WMWindowPosChanged(TWMWindowPosChanged& Message){
  INHERITED_MSG(Message);
  // Update min/max width/height to actual extents control will allow 
  if(((csReading | csLoading) & GetComponentState()) == 0){
       if(MaxWidth > 0 && Width > MaxWidth) 
			MaxWidth = Width;
	   else if(MinWidth > 0 && Width < MinWidth)
			MinWidth = Width;
	   if (MaxHeight > 0 && Height > MaxHeight)
		   MaxHeight = Height;
	   else if (MinHeight > 0 && Height < MinHeight) 
		   MinHeight = Height;
	   //if(Message.WindowPos != NULL && HostDockSite != NULL && !IN_TEST(csDocking,ControlState) && 
		//   IN_TEST(SWP_NOSIZE, Message.WindowPos->flags) && Message.WindowPos->cx != 0 && 
		//   Message.WindowPos->cy != 0)
		//   CalcDockSizes();
  }
}

void CControl::CMVisibleChanged(TMessage& Message){
	if(!IN_TEST(csDesigning, GetComponentState()) || IN_TEST(csNoDesignVisible, ControlStyle))
		InvalidateControl(TRUE, Visible && IN_TEST(csOpaque, ControlStyle));
}

VOID CControl::SetColor(TColor Value){
	if (Color != Value){
		Color = Value;
		ParentColor = FALSE;
		Perform(CM_COLORCHANGED, 0, 0);
	}
}

void CControl::CMParentColorChanged(TMessage& Message){
	if(ParentColor){
		if(Message.wParam != 0)
			SetColor((UINT)Message.lParam);
		else
			SetColor(Parent->Color);
		ParentColor = TRUE;
	}
}

VOID CControl::SetFont(CFont* Value){
	Font->Assign(Value);
}

VOID CControl::SetParentColor(BOOL Value){
	if(ParentColor != Value){
		ParentColor = Value;
		if(Parent != NULL && !IN_TEST(csReading, GetComponentState()))
			Perform(CM_PARENTCOLORCHANGED, 0, 0);
	}
}

VOID CControl::SetParentFont(BOOL Value){
	if(ParentFont != Value){
		ParentFont = Value;
		if(Parent != NULL && !IN_TEST(csReading, GetComponentState()))
			Perform(CM_PARENTFONTCHANGED, 0, 0);
	}
}

void CControl::SetParentShowHint(BOOL Value){
	if(ParentShowHint != Value){
		ParentShowHint = Value;
		if(Parent != NULL && !IN_TEST(csReading, GetComponentState()))
			Perform(CM_PARENTSHOWHINTCHANGED, 0, 0);
	}
}

VOID CControl::SetAlign(TAlign Value){
	if(Align != Value){
		TAlign OldAlign = Align;
		Align = Value;
		Anchors = AnchorAlign[Value];
		if(!IN_TEST(csLoading, GetComponentState()) && (!IN_TEST(csDesigning, GetComponentState()) ||
			(GetParent() != NULL)) && (Value != alCustom) && (OldAlign != alCustom))
			if(IN_TEST(OldAlign, (alTop | alBottom)) == IN_TEST(Value, (alRight | alLeft)) &&
				!IN_TEST(OldAlign, (alNone | alClient)) && !IN_TEST(Value, (alNone | alClient)))
				SetBounds(GetLeft(), GetTop(), GetHeight(), GetWidth());
			else
				AdjustSize();
	}
	RequestAlign();
}

VOID CControl::SetCursor(TCursor Value){
	if(Cursor != Value){
		Cursor = Value;
		Perform(CM_CURSORCHANGED, 0, 0);
	}
}

BOOL CControl::GetEnabled(){
	return Enabled;
}

void CControl::SetEnabled(BOOL Value){
	if(Enabled != Value){
		Enabled = Value;
		Perform(CM_ENABLEDCHANGED, 0, 0);
	}
}

void CControl::CMParentFontChanged(TMessage& Message){
	if(ParentFont){
		if(Message.wParam != 0)
			SetFont((CFont *)Message.lParam);
		else
			SetFont(Parent->Font);
		ParentFont = TRUE;
	}
}

VOID CControl::SetShowHint(BOOL Value){
	if(ShowHint != Value){
		ShowHint = Value;
		ParentShowHint = FALSE;
		Perform(CM_SHOWHINTCHANGED, 0, 0);
	}
}

void CControl::CMParentShowHintChanged(TMessage& Message){
	if(ParentShowHint){
		SetShowHint(Parent->ShowHint);
		ParentShowHint = TRUE;
	}
}

VOID CControl::SetBiDiMode(TBiDiMode Value){
	if(BiDiMode != Value){
		BiDiMode = Value;
		ParentBiDiMode = FALSE;
		Perform(CM_BIDIMODECHANGED, 0, 0);
	}
}

void CControl::CMParentBiDiModeChanged(TMessage& Message){
	if(ParentBiDiMode){
		if(Parent != NULL)
			SetBiDiMode(Parent->GetBiDiMode());
		ParentBiDiMode = TRUE;
	}
}

void CControl::CMBiDiModeChanged(TMessage& Message){
	if (GetGlobal().GetSysLocaleMiddleEast() && Message.wParam == 0) 
		Invalidate();
}

void CControl::CMFontChanged(TMessage& Message){
	Invalidate();
}

void CControl::CMColorChanged(TMessage& Message){
	Invalidate();
}

void CControl::CMMouseWheel(TCMMouseWheel& Message){
	Message.Result = 0;
	if(DoMouseWheel(Message.ShiftState, Message.WheelDelta, SmallPointToPoint(Message.Pos)))
      Message.Result = 1;
	else if(GetParent() != NULL)
		Message.Result = GetParent()->Perform(CM_MOUSEWHEEL, Message.wParam, Message.lParam);
}

BYTE KeysToShiftState(WORD Keys){
	BYTE Result = 0;
	if(IN_TEST(MK_SHIFT, Keys)) Result |= ssShift;
	if(IN_TEST(MK_CONTROL, Keys)) Result |= ssCtrl;
	if(IN_TEST(MK_LBUTTON, Keys)) Result |= ssLeft;
	if(IN_TEST(MK_RBUTTON, Keys)) Result |= ssRight;
	if(IN_TEST(MK_MBUTTON, Keys)) Result |= ssMiddle;
	if(GetKeyState(VK_MENU) < 0) Result |= ssAlt;
	return Result;
}

#define AltMask 0x20000000
TShiftState KeyDataToShiftState(LONG KeyData){
	TShiftState Result = 0;
	if(GetKeyState(VK_SHIFT) < 0)
		Result |= ssShift;
	if(GetKeyState(VK_CONTROL) < 0)
		Result |=  ssCtrl;
	if((KeyData & AltMask) != 0)
		Result |= Result, ssAlt;
	return Result;
}
TShiftState KeyboardStateToShiftState(const TKeyboardState KeyboardState){
	TShiftState Result = 0;
	if((KeyboardState[VK_SHIFT] & 0x80) != 0)
		Result |= ssShift;
	if((KeyboardState[VK_CONTROL] & 0x80) != 0)
		Result |= ssCtrl;
	if((KeyboardState[VK_MENU] & 0x80) != 0)
		Result |= ssAlt;
	if((KeyboardState[VK_LBUTTON] & 0x80) != 0)
		Result |= ssLeft;
	if((KeyboardState[VK_RBUTTON] & 0x80) != 0)
		Result |= ssRight;
	if((KeyboardState[VK_MBUTTON] & 0x80) != 0)
		Result |= ssMiddle;
	return Result;
}

TShiftState KeyboardStateToShiftState(){
	TKeyboardState KeyState = {0};
	GetKeyboardState(KeyState);
	return KeyboardStateToShiftState(KeyState);
}

BOOL IsAccel(WORD VK, LPTSTR Str){
	return (TCHAR)VK == GetHotKey(Str);
}

TCHAR GetHotKey(LPTSTR Text){
	TCHAR Result = TCHAR('\0');
	INT I = 0;
	INT L = lstrlen(Text);
	while(I < L){
		//if(*(Text + I) in LeadBytes)
		//	I++;
		//else 
		if(*(Text + I) == cHotkeyPrefix && (L - I > 1)){
			I++;
			if(*(Text + I) != cHotkeyPrefix)
				Result = *(Text + I); // keep going there may be another one
		}
		I++;
	}
	return Result;
}

void CControl::WMMouseWheel(TWMMouseWheel& Message){
	if(!GetGlobal().GetMouseWheelPresent()){
		GetGlobal().SetMouseWheelPresent(TRUE);
		//Mouse.SettingChanged(SPI_GETWHEELSCROLLLINES);
	}
	((PCMMouseWheel)&Message)->ShiftState = KeysToShiftState(Message.Keys);
	MouseWheelHandler(*((PMessage)&Message));
	if(Message.Result == 0)
		INHERITED_MSG(Message);
}

void CControl::CMHitTest(TCMHitTest& Message){
	Message.Result = HTCLIENT;
}

void CControl::CMEnabledChanged(TMessage& Message){
	Invalidate();
}

void CControl::WMContextMenu(TWMContextMenu& Message){
	if(Message.Result != 0)
		return ;
	if(IN_TEST(csDesigning, GetComponentState())){
		INHERITED_MSG(Message);
		return ;
	}

	TPoint Temp = {0, 0};
	TPoint Pt = SmallPointToPoint(Message.Pos);
	if(InvalidPoint(Pt.x, Pt.y))
		Temp = Pt;
	else{
		Temp = ScreenToClient(Pt);
		TRect Rect = GetClientRect();
		if(!PtInRect((RECT *)&Rect, Temp)){
			INHERITED_MSG(Message);
			return;
		}
	}
	BOOL Handled = FALSE;
	DoContextPopup(Temp, Handled);
	Message.Result = Handled;
	if(Handled)
		return ;
	/*
	PopupMenu = GetPopupMenu();
	if(PopupMenu != NULL && PopupMenu->GetAutoPopup()){
		SendCancelMode(NULL);
		PopupMenu.PopupComponent = this;
		if(InvalidPoint(Pt.x, Pt.y))
			TPoint Pt = ClientToScreen(Point(0, 0));
		PopupMenu.Popup(Pt.X, Pt.Y);
		Message.Result = 1;
	}//*/

	if(Message.Result == 0)
		INHERITED_MSG(Message);
}

LRESULT CControl::Perform(UINT Msg, WPARAM WParam, LPARAM LParam){
	TMessage Message;
	Message.Msg = Msg;
	Message.wParam = WParam;
	Message.lParam = LParam;
	Message.Result = 0;
	if(this != NULL){
		WndProc(Message);
	}
	return Message.Result;
}

void CControl::UpdateAnchorRules(){
	if (!AnchorMove && !((csLoading & GetComponentState()) == csLoading)){
		if (Anchors == (akLeft | akTop)){
			OriginalParentSize.x = 0;
			OriginalParentSize.y = 0;
			return;
		}
		if((akRight & Anchors) == akRight){
			if ((akLeft & Anchors) == akLeft)
				AnchorRules.x = Width;
			else
				AnchorRules.x = Left;
		}
		else
			AnchorRules.x = Left + Width / 2;
		if ((akBottom & Anchors) == akBottom){
			if ((akTop & Anchors) == akTop)
				AnchorRules.y = Height;
			else
				AnchorRules.y = Top;
		}
		else
			AnchorRules.y = Top + Height / 2;
		CWinControl* pParent = GetParent();
		if (pParent != NULL){
			//if ((csReading & GetParent()->GetComponentState()) == csReading){
			//	if (!((csDesigning & GetComponentState()) == csDesigning))
			//		OriginalParentSize = GetParent()->GetDesignSize();
			//}
			//else 
			if (pParent->HandleAllocated())
				OriginalParentSize = pParent->GetClientRect().BottomRight;
			else {
				OriginalParentSize.x = pParent->GetWidth();
				OriginalParentSize.y = pParent->GetHeight();
			}
		}
	}
}

VOID CControl::RequestAlign(){
	if (GetParent() != NULL) 
		GetParent()->AlignControl(this);
}

void CControl::AdjustSize(){
	  if ((csLoading & GetComponentState()) == 0)
		  SetBounds(Left, Top, Width, Height);
}

BOOL CControl::UseRightToLeftReading(){
	return GetGlobal().GetSysLocaleMiddleEast() && (BiDiMode != bdLeftToRight);
}

HDC CControl::GetDeviceContext(HWND& WindowHandle){
	if(Parent == NULL)
		throw "Control ''%s'' has no parent window";
	HDC Result = Parent->GetDeviceContext(WindowHandle);
	SetViewportOrgEx(Result, Left, Top, NULL);
	IntersectClipRect(Result, 0, 0, Width, Height);
	return Result;
}

DWORD CControl::DrawTextBiDiModeFlags(DWORD Flags){
	DWORD Result = Flags;
	//{ do not change center alignment }
	if(UseRightToLeftAlignment())
		if(IN_TEST(DT_RIGHT, Result))
			Result &= ~DT_RIGHT;// { removing DT_RIGHT, makes it DT_LEFT }
		else if(!IN_TEST(DT_CENTER, Result))
			Result |= DT_RIGHT;
	Result |= DrawTextBiDiModeFlagsReadingOnly();
	return Result;
}

DWORD CControl::DrawTextBiDiModeFlagsReadingOnly(){
	if(UseRightToLeftReading())
		return DT_RTLREADING;
	else 
		return 0;
}

BOOL CControl::UseRightToLeftAlignment(){
	return GetGlobal().GetSysLocaleMiddleEast() && (BiDiMode == bdRightToLeft);
}

TAlignment CControl::GetControlsAlignment(){
	return taLeftJustify;
}

BOOL CControl::CheckNewSize(INT& NewWidth, INT& NewHeight){
	BOOL Result = FALSE;
	INT W = NewWidth;
	INT H = NewHeight;
	if (DoCanResize(W, H)){
		INT W2 = W;
		INT H2 = H;
		Result = !AutoSize || (DoCanAutoSize(W2, H2) && (W2 == W) && (H2 == H)) || DoCanResize(W2, H2);
		if (Result){
			NewWidth = W2;
			NewHeight = H2;
		}
	}
	return Result;
}

void CControl::Changed(){
	  Perform(CM_CHANGED, 0, (LPARAM)this);
}

BOOL CControl::CanAutoSize(INT& NewWidth, INT& NewHeight){
	return TRUE;
}

BOOL CControl::DoCanAutoSize(INT& NewWidth, INT& NewHeight){
	BOOL Result = FALSE;
	if (Align != alClient){
		INT W = NewWidth;
		INT H = NewHeight;
		Result = CanAutoSize(W, H);
		if (Align == alNone || Align == alLeft || Align == alRight)
			NewWidth = W;
		if (Align == alNone || Align == alTop || Align == alBottom)
			NewHeight = H;
	}
	else 
		Result = TRUE;
	return Result;
}

BOOL CControl::DoMouseWheel(BYTE Shift, INT WheelDelta, TPoint& MousePos){
	BOOL Result = FALSE;
	if(OnMouseWheel != NULL)
		Result = CALL_EVENT(MouseWheel)(this, Shift, WheelDelta, MousePos);
	if(!Result){
		BOOL IsNeg = FALSE;
		WheelAccumulator += WheelDelta;
		while(::abs(WheelAccumulator) >= WHEEL_DELTA){
			IsNeg = WheelAccumulator < 0;
			WheelAccumulator = ::abs(WheelAccumulator) - WHEEL_DELTA;
			if(IsNeg){
				if(WheelAccumulator != 0)
					WheelAccumulator = -WheelAccumulator;
				Result = DoMouseWheelDown(Shift, MousePos);
			}
			else 
				Result = DoMouseWheelUp(Shift, MousePos);
		}
	}
	return Result;
}

BOOL CControl::DoMouseWheelDown(BYTE Shift, TPoint& MousePos){
	BOOL Result = FALSE;
	if(OnMouseWheelDown != NULL)
		Result = CALL_EVENT(MouseWheelDown)(this, Shift, MousePos);
	return Result;
}

BOOL CControl::DoMouseWheelUp(BYTE Shift, TPoint& MousePos){
	BOOL Result = FALSE;
	if(OnMouseWheelUp != NULL)
		Result = CALL_EVENT(MouseWheelUp)(this, Shift, MousePos);
	return Result;
}

void CControl::DoContextPopup(TPoint& MousePos, BOOL& Handled){
	if(OnContextPopup != NULL)
		CALL_EVENT(ContextPopup)(this, MousePos, Handled);
}

BOOL CControl::CanResize(INT& NewWidth, INT& NewHeight){
	BOOL Result = TRUE;
	if (OnCanResize != NULL){
		CALL_EVENT(CanResize)(this, NewWidth, NewHeight, Result);
	}
	return Result;
}

void CControl::ConstrainedResize(INT& MinWidth, INT& MinHeight, INT& MaxWidth, INT& MaxHeight){
	if(OnConstrainedResize != NULL)
		CALL_EVENT(ConstrainedResize)(this, MinWidth, MinHeight, MaxWidth, MaxHeight);
}

void CControl::FontChanged(CObject* Sender){
	ParentFont = FALSE;
	//DesktopFont = False;
	//if(Font->GetHeight() != FontHeight){
	//	ScalingFlags |= sfFont;
	//	FontHeight = Font->GetHeight();
	//}
	Perform(CM_FONTCHANGED, 0, 0);
}

void CControl::DoMouseDown(TWMMouse& Message, TMouseButton Button, TShiftState Shift){
	if(!IN_TEST(csNoStdEvents, GetControlStyle()))
		if(GetWidth() > 32768 || GetHeight() > 32768){
			TPoint pt = CalcCursorPos();
			MouseDown(Button, KeysToShiftState((WORD)Message.Keys) + Shift, pt.x, pt.y);
		}
		else
			MouseDown(Button, KeysToShiftState((WORD)Message.Keys) + Shift, Message.XPos, Message.YPos);

}
void CControl::DoMouseUp(TWMMouse& Message, TMouseButton Button){
	if(!IN_TEST(csNoStdEvents, GetControlStyle()))
		MouseUp(Button, KeysToShiftState((WORD)Message.Keys), Message.XPos, Message.YPos);
}

TPoint CControl::CalcCursorPos(){
	TPoint Result;
	GetCursorPos(&Result);
	Result = ScreenToClient(Result);
	return Result;
}

void CControl::DoConstrainedResize(INT& NewWidth, INT& NewHeight){
	INT minWidth = MinWidth > 0 ? MinWidth : 0;
	INT minHeight = MinHeight > 0 ? MinHeight : 0;
	INT maxWidth = MaxWidth > 0 ? MaxWidth : 0;
	INT maxHeight = MaxHeight > 0 ? MaxHeight : 0;
	ConstrainedResize(minWidth, minHeight, maxWidth, maxHeight);
	if(maxWidth > 0 && NewWidth > maxWidth)
		NewWidth = maxWidth;
	else if(minWidth > 0 && NewWidth < minWidth)
		NewWidth = minWidth;
	if(maxHeight > 0 && NewHeight > maxHeight)
		NewHeight = maxHeight;
	else if(minHeight > 0 && NewHeight < minHeight)
		NewHeight = minHeight;
}

BOOL CControl::DoCanResize(INT& NewWidth, INT& NewHeight){
	BOOL Result = CanResize(NewWidth, NewHeight);
	if (Result)
		DoConstrainedResize(NewWidth, NewHeight);
	return Result;
}

TRect CControl::GetBoundsRect(){
	TRect Rect;
	Rect.left = Left;
	Rect.top = Top;
	Rect.bottom = Top + Height;
	Rect.right = Left + Width;
	return Rect;
}

BOOL CControl::BackgroundClipped(TRect& Rect){
	CControl* AControl = NULL;
	TRect R;
	CList* Controls = Parent->GetControls();
	INT I = Controls->IndexOf(this);
	while (I > 0){
		I--;
		AControl = (CControl*)Controls->Get(I);
		if(AControl->Visible && ((csOpaque & AControl->ControlStyle) == csOpaque)){
			IntersectRect((LPRECT)&R, (LPRECT)&Rect, (LPRECT)&GetBoundsRect());
			if(EqualRect((LPRECT)&R, (LPRECT)&Rect)){
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CControl::InvalidateControl(BOOL IsVisible, BOOL IsOpaque){
	CWinControl* pParent = GetParent();
	if((IsVisible || IN_TEST(csDesigning, GetComponentState()) && 
		!IN_TEST(csNoDesignVisible, ControlStyle)) && pParent != NULL && pParent->HandleAllocated()) {
		TRect Rect = GetBoundsRect();
		InvalidateRect(pParent->GetHandle(), (LPRECT)&Rect, !(IsOpaque || 
			IN_TEST(csOpaque, pParent->ControlStyle) || BackgroundClipped(Rect)));
	}
}

void CControl::Invalidate(){
	InvalidateControl(Visible, (csOpaque & ControlStyle) == csOpaque);
}

void CControl::Show(){
	if(Parent != NULL)
		Parent->ShowControl(this);
	if(!IN_TEST(csDesigning, GetComponentState()) || IN_TEST(csNoDesignVisible, ControlStyle))
		SetVisible(TRUE);
}
void CControl::Update(){
	if(Parent != NULL)
		Parent->Update();
}

void CControl::Resize(){
	if(OnResize != NULL){
		CALL_EVENT(Resize)(this);
	}
}

void CControl::Click(){
	/*Call OnClick if assigned and not equal to associated action's OnExecute.
    If associated action's OnExecute assigned then call it, otherwise, call
    OnClick. */
	/* TODO if(OnClick != NULL && Action != NULL && OnClick != Action->OnExecute)
		CALL_EVENT(Click)(this);
	else if(!IN_TEST(csDesigning, GetComponentState()) && ActionLink != NULL)
		ActionLink->Execute(this);
	else //*/ 
		if(OnClick != NULL)
			CALL_EVENT(Click)(this);
}
void CControl::DblClick(){
	if(OnDblClick != NULL)
		CALL_EVENT(DblClick)(this);
}

void CControl::MouseDown(TMouseButton Button, TShiftState Shift, INT X, INT Y){
	if(OnMouseDown != NULL)
		CALL_EVENT(MouseDown)(this, Button, Shift, X, Y);
}

void CControl::MouseMove(TShiftState Shift, INT X, INT Y){
	if(OnMouseMove != NULL)
		CALL_EVENT(MouseMove)(this, Shift, X, Y);
}

void CControl::MouseUp(TMouseButton Button, TShiftState Shift, INT X, INT Y){
	if(OnMouseUp != NULL)
		CALL_EVENT(MouseUp)(this, Button, Shift, X, Y);
}

void CControl::SendCancelMode(CControl* Sender){
	CControl* Control = this;
	while(Control != NULL){
		if(Control->InstanceOf(CForm::_Class))
			((CForm *)Control)->SendCancelMode(Sender);
		Control = Control->GetParent();
	}
}

VOID CControl::SetAutoSize(BOOL Value){
	if(AutoSize != Value){
		AutoSize = Value;
		if(Value)
			AdjustSize();
	}
}

void CControl::SetBounds(INT ALeft, INT ATop, INT AWidth, INT AHeight){
	if(CheckNewSize(AWidth, AHeight) &&
	  ((ALeft != Left) || (ATop != Top) ||
	  (AWidth != Width) || (AHeight != Height))){
		InvalidateControl(Visible, FALSE);
		Left = ALeft;
		Top = ATop;
		Width = AWidth;
		Height = AHeight;
		UpdateAnchorRules();
		Invalidate();
		Perform(WM_WINDOWPOSCHANGED, 0, 0);
		RequestAlign();
		if ((csLoading & GetComponentState()) == 0)
			Resize();
	}
}


TPoint CControl::GetClientOrigin(){
	if(GetParent() == NULL)
		throw "control's parent is required.";//EInvalidOperation.CreateFmt(SParentRequired, [Name
	TPoint Result = GetParent()->GetClientOrigin();
	Result.x += Left;
	Result.y += Top;
	return Result;
}

TRect CControl::GetClientRect(){
	TRect Result;
	Result.left = 0;
	Result.top = 0;
	Result.right = GetWidth();
	Result.bottom = GetHeight();
	return Result;
}

INT CControl::GetClientHeight(){
	return GetClientRect().bottom;
}

INT CControl::GetClientWidth(){
	return GetClientRect().right;
}

void CControl::SetClientHeight(INT Value){
	TPoint p = {GetClientWidth(), Value};
	SetClientSize(p);
}

void CControl::SetClientWidth(INT Value){
	TPoint p = {Value, GetClientHeight()};
	SetClientSize(p);
}

void CControl::SetClientSize(TPoint& Value){
	TRect Client = GetClientRect();
	SetBounds(Left, Top, Width - Client.right + Value.x, Height -
		Client.bottom + Value.y);
}

TPoint CControl::ScreenToClient(TPoint& Point){
	TPoint Origin = GetClientOrigin();
	Origin.x = Point.x - Origin.x;
	Origin.y = Point.y - Origin.y;
	return Origin;
}

TPoint CControl::ClientToParent(TPoint& Point, CWinControl* AParent){
	if(AParent == NULL)
		AParent = GetParent();
	if(AParent == NULL)
		throw "invalid operation, parent required.";//raise EInvalidOperation.CreateFmt(SParentRequired, [Name]);
	TPoint Result = Point;
	Result.x += GetLeft();
	Result.y += GetTop();
	CWinControl* LParent = GetParent();
	while(LParent != NULL){
		if(LParent->GetParent() != NULL){
			Result.x += LParent->GetLeft();
			Result.y += LParent->GetTop();
		}
		if(LParent == AParent)
			break;
		else
			LParent = LParent->GetParent();
	}
	if(LParent == NULL)
		throw "invalid operation, given parent is not a parent";//raise EInvalidOperation.CreateFmt(SParentGivenNotAParent, [Name]);
	return Result;
}

TPoint CControl::ClientToScreen(const TPoint Point){
	TPoint Origin = GetClientOrigin();
	TPoint Result = {Point.x + Origin.x, Point.y + Origin.y};
	return Result;
}

VOID CControl::VisibleChanging(){
}

VOID CControl::SetParent(CWinControl* AParent){
	if(Parent != AParent){
		if (AParent == this)
			throw "can not set parent to self.";// EInvalidOperation.CreateRes(@SControlParentSetToSelf);
		if(Parent != NULL)
			Parent->RemoveControl(this);
		if(AParent != NULL){
			AParent->InsertControl(this);
			UpdateAnchorRules();
		}
	}
}

VOID CControl::SetVisible(BOOL Value){
	if(Visible != Value){
		VisibleChanging();
		Visible = Value;
		Perform(CM_VISIBLECHANGED, (WPARAM)Value, 0);
		RequestAlign();
	}
}

VOID CControl::SetLeft(INT Value){
	SetBounds(Value, Top, Width, Height);
}

VOID CControl::SetTop(INT Value){
	SetBounds(Left, Value, Width, Height);
}

VOID CControl::SetWidth(INT Value){
	SetBounds(Left, Top, Value, Height);
}

VOID CControl::SetHeight(INT Value){
	SetBounds(Left, Top, Width, Value);
}

VOID CControl::SetText(LPTSTR Text){
	SetTextBuf(Text);
}

INT CControl::GetTextLen(){
	return (INT)Perform(WM_GETTEXTLENGTH, 0, 0);
}

INT CControl::GetTextBuf(LPTSTR Buffer, INT BufSize){
	return (INT)Perform(WM_GETTEXT, BufSize, (LPARAM)Buffer);
}

VOID CControl::SetTextBuf(LPTSTR Text){
	Perform(WM_SETTEXT, 0, (LPARAM)Text);
	Perform(CM_TEXTCHANGED, 0, 0);
}

String CControl::GetTextString(){
	INT Len = GetTextLen();
	INT Size = (Len + 1) * sizeof(TCHAR);
	LPTSTR Buf = (LPTSTR )malloc(Size);
	GetTextBuf(Buf, Size);
	*(Buf + Len) = TCHAR('\0');
	return String::CreateFor(Buf);
}

void CControl::SetZOrder(BOOL TopMost){
	if(GetParent() != NULL){
		if(TopMost)
			SetZOrderPosition(Parent->Controls->GetCount() - 1);
		else
			SetZOrderPosition(0);
	}
}

void CControl::SetZOrderPosition(INT Position){
	if(Parent != NULL){
		INT I = Parent->Controls->IndexOf(this);
		if(I >= 0){
			INT Count = Parent->Controls->GetCount();
			if(Position < 0)
				Position = 0;
			if(Position >= Count)
				Position = Count - 1;
			if(Position != I){
				Parent->Controls->Delete(I);
				Parent->Controls->Insert(Position, this);
				InvalidateControl(GetVisible(), TRUE);
				CForm* ParentForm = ValidParentForm(this);
				if(IN_TEST(csPalette, ParentForm->GetControlState()))
					((CControl *)ParentForm)->PaletteChanged(TRUE);
			}
		}
	}
}

HPALETTE CControl::GetPalette(){
	return 0;
}

BOOL CControl::PaletteChanged(BOOL Foreground){
	BOOL Result = FALSE;
	if(!GetVisible())
		return Result;
	HPALETTE Palette = GetPalette();
	if(Palette != 0){
		HWND WindowHandle = 0;
		HDC DC = GetDeviceContext(WindowHandle);
		HPALETTE OldPalette = SelectPalette(DC, Palette, !Foreground);
		if(RealizePalette(DC) != 0)
			Invalidate();
		SelectPalette(DC, OldPalette, TRUE);
		ReleaseDC(WindowHandle, DC);
		Result = TRUE;
	}
	return Result;
}

void CControl::Refresh(){
	Repaint();
}

void CControl::Repaint(){
	if((GetVisible() || IN_TEST(csDesigning, GetComponentState()) &&
		!IN_TEST(csNoDesignVisible, GetControlStyle())) && GetParent() != NULL &&
		GetParent()->HandleAllocated()){
		if(IN_TEST(csOpaque, GetControlStyle())){
			HWND Wnd = GetParent()->GetHandle();
			HDC DC = GetDC(Wnd);
			DCHolder dcHolder(DC, 0, Wnd);
			IntersectClipRect(DC, GetLeft(), GetTop(), GetLeft() + GetWidth(), GetTop() + GetHeight());
			GetParent()->PaintControls(DC, this);
		}
		else{
			Invalidate();
			Update();
		}
	}
}

void CControl::BringToFront(){
	SetZOrder(TRUE);
}

BOOL CControl::GetMouseCapture(){
	return GetCaptureControl() == this;
}

void CControl::SetMouseCapture(BOOL Value){
	if(GetMouseCapture() != Value)
		if(Value)
			SetCaptureControl(this);
		else 
			SetCaptureControl(NULL);
}


IMPL_DYN_CLASS(CGraphicControl)
CGraphicControl::CGraphicControl(CComponent* AOwner):CControl(AOwner),Canvas(NULL){
	Canvas = new CControlCanvas();
	((CControlCanvas *)Canvas)->SetControl(this);
}

CGraphicControl::~CGraphicControl(){
	if(CaptureControl == this)
		SetCaptureControl(NULL);
	delete Canvas;
}
/*
class CanvasHandleCleaner{
private:
	CCanvas* Canvas;
public:
	CanvasHandleCleaner(CCanvas* ACanvas):Canvas(ACanvas){
	}
	virtual ~CanvasHandleCleaner(){
		if(Canvas != NULL)
			Canvas->SetHandle(0);
	}
};
//*/

void CGraphicControl::WMPaint(TWMPaint& Message){
	if(Message.DC != 0){
		CMethodLock Lock(Canvas, (TLockMethod)&CCanvas::Lock, (TLockMethod)&CCanvas::UnLock);
		Canvas->SetHandle(Message.DC);
		CanvasHandleCleaner Cleaner(Canvas);
		Paint();
	}
}
void CGraphicControl::Paint(){
}

#define CanvasListCacheSize	4
CThreadList ControlCanvasList;
// Free the first available device context
void FreeDeviceContext(){
	CMethodLock Lock(&ControlCanvasList, (TLockMethodResult)&CThreadList::LockList, (TLockMethod)&CThreadList::UnlockList);
	CList* List = (CList*)Lock.GetLockResult();
	INT Count = List->GetCount();
	for(INT I = 0; I < Count; I++){
		CControlCanvas* Canvas = (CControlCanvas*)List->Get(I);
		{
			CMethodLock CanvasTryLock(Canvas, (TTryLockMethod)&CCanvas::TryLock, (TLockMethod)&CCanvas::UnLock);
			if(CanvasTryLock.GetTryLockResult()){
				Canvas->FreeHandle();
				return;
			}
		}
	}
}

IMPL_DYN_CLASS(CControlCanvas)
CControlCanvas::CControlCanvas():
	Control(NULL), 
	DeviceContext(0), 
	WindowHandle(0){
}
CControlCanvas::~CControlCanvas(){
	FreeHandle();
}

void CControlCanvas::CreateHandle(){
	if(Control == NULL)
		__super::CreateHandle();
	else {
		if(DeviceContext == 0){
			CMethodLock Lock(&ControlCanvasList, (TLockMethodResult)&CThreadList::LockList, (TLockMethod)&CThreadList::UnlockList);
			CList* List = (CList*)Lock.GetLockResult();
			if(List->GetCount() >= CanvasListCacheSize)
				FreeDeviceContext();
			DeviceContext = Control->GetDeviceContext(WindowHandle);
			List->Add(this);
		}
	}
    SetHandle(DeviceContext);
    UpdateTextFlags();
}

void CControlCanvas::FreeHandle(){
	if(DeviceContext != 0){
		SetHandle(0);
		ControlCanvasList.Remove(this);
		ReleaseDC(WindowHandle, DeviceContext);
		DeviceContext = 0;
	}
}

void CControlCanvas::UpdateTextFlags(){
	if(Control == NULL)
		return ;
	if(Control->UseRightToLeftReading())
		SetTextFlags(GetTextFlags() | ETO_RTLREADING);
	else
		SetTextFlags(GetTextFlags() & (~ETO_RTLREADING));
}

void CControlCanvas::SetControl(CControl* AControl){
	if(Control != AControl){
		FreeHandle();
		Control = AControl;
	}
}
