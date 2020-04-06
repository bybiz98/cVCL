
#include "stdinc.h"
#include "SysInit.hpp"
#include "WinUtils.hpp"
#include "Edit.hpp"

IMPL_DYN_CLASS(CEdit)
CEdit::CEdit(CComponent* AOwner) : CWinControl(AOwner),
	MaxLength(0),
	BorderStyle(bsSingle),
	PasswordChar(TCHAR('\0')),
	ReadOnly(FALSE),
	AutoSize(TRUE),
	AutoSelect(TRUE),
	HideSelection(TRUE),
	OEMConvert(FALSE),
	CharCase(ecNormal),
	Creating(FALSE),
	Modified(FALSE),
	INIT_EVENT(Change)
	{
	UINT EditStyle = csClickEvents | csSetCaption | csDoubleClicks | csFixedHeight;
	if(GetGlobal().GetNewStyleControls())
		SetControlStyle(EditStyle);
	else
		SetControlStyle(EditStyle | csFramed);
	SetBounds(GetLeft(), GetTop(), 121, 25);
	SetTabStop(TRUE);
	SetParentColor(FALSE);
	AdjustHeight();
}
CEdit::~CEdit(){
}

void CEdit::AdjustHeight(){
	HDC DC = GetDC(0);
	DCHolder dcHolder(DC, 0, 0);
	TEXTMETRIC SysMetrics;
	GetTextMetrics(DC, &SysMetrics);
	HFONT SaveFont = (HFONT)SelectObject(DC, GetFont()->GetHandle());
	dcHolder.SwapRestore(SaveFont);
	TEXTMETRIC Metrics;
	GetTextMetrics(DC, &Metrics);
	INT I = 0;
	if(GetGlobal().GetNewStyleControls()){
		if(GetCtl3D())
			I = 8;
		else
			I = 6;
		I = GetSystemMetrics(SM_CYBORDER) * I;
	}
	else {
		I = SysMetrics.tmHeight;
		if(I > Metrics.tmHeight)
			I = Metrics.tmHeight;
		I = I / 4 + GetSystemMetrics(SM_CYBORDER) * 4;
	}
	SetHeight(Metrics.tmHeight + I);
}

void CEdit::WMSetFont(TWMSetFont& Message){
	INHERITED_MSG(Message);
	if(GetGlobal().GetNewStyleControls() &&
		(GetWindowLongPtr(GetHandle(), GWL_STYLE) & ES_MULTILINE) == 0)
		SendMessage(GetHandle(), EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, 0);
}

void CEdit::CMCtl3DChanged(TMessage& Message){
	if(GetGlobal().GetNewStyleControls() && BorderStyle == bsSingle){
		UpdateHeight();
		RecreateWnd();
	}
	INHERITED_MSG(Message);
}

void CEdit::CMEnter(TCMGotFocus& Message){
	if(AutoSelect && !IN_TEST(csLButtonDown, GetControlState()) &&
		(GetWindowLongPtr(GetHandle(), GWL_STYLE) & ES_MULTILINE) == 0)
		SelectAll();
	INHERITED_MSG(Message);
}

void CEdit::CMFontChanged(TCMGotFocus& Message){
	INHERITED_MSG(Message);
	if(IN_TEST(csFixedHeight, GetControlStyle()) && !(IN_TEST(csDesigning, GetComponentState()) 
		&& IN_TEST(csLoading, GetComponentState())))
		AdjustHeight();
}

void CEdit::CNCommand(TWMCommand& Message){
	if (Message.NotifyCode == EN_CHANGE && !Creating)
		Change();
}

void CEdit::CMTextChanged(TMessage& Message){
	INHERITED_MSG(Message);
	if(!HandleAllocated() || (GetWindowLongPtr(GetHandle(), GWL_STYLE) &
		ES_MULTILINE) != 0)
		Change();
}

void CEdit::WMContextMenu(TWMContextMenu& Message){
	SetFocus();
	INHERITED_MSG(Message);
}


void CEdit::CreateParams(TCreateParams& Params){
	DWORD Passwords[] = {0, ES_PASSWORD};
	DWORD ReadOnlys[] = {0, ES_READONLY};
	DWORD CharCases[] = {0, ES_UPPERCASE, ES_LOWERCASE};
	DWORD HideSelections[] = {ES_NOHIDESEL, 0};
	DWORD OEMConverts[] = {0, ES_OEMCONVERT};
	DWORD BorderStyles[] = {0, WS_BORDER};
	__super::CreateParams(Params);
	CreateSubClass(Params, TEXT("EDIT"));
	Params.Style |= (ES_AUTOHSCROLL | ES_AUTOVSCROLL) |
		BorderStyles[BorderStyle - 1] | Passwords[PasswordChar != TEXT('\0') ? 1 : 0] |
		ReadOnlys[ReadOnly ? 1 : 0] | CharCases[CharCase ? 1 : 0] |
		HideSelections[HideSelection ? 1 : 0] | OEMConverts[OEMConvert ? 1 : 0];
	if(GetGlobal().GetNewStyleControls() && GetCtl3D() && BorderStyle == bsSingle){
		Params.Style &= ~WS_BORDER;
		Params.ExStyle |= WS_EX_CLIENTEDGE;
	}
}

void CEdit::CreateWindowHandle(const TCreateParams& Params){
	if(GetGlobal().GetSysLocaleFastEast() && GetGlobal().GetWin32Platform() != VER_PLATFORM_WIN32_NT && 
		IN_TEST(ES_READONLY, Params.Style)){
		// Work around Far East Win95 API/IME bug.
		TCreateParams P = Params;
		P.Style &= ~ES_READONLY;
		__super::CreateWindowHandle(P);
		if(GetWnd() != 0)
			SendMessage(GetWnd(), EM_SETREADONLY, TRUE, 0);
	}
	else
		__super::CreateWindowHandle(Params);
}

void CEdit::CreateWnd(){
	Creating = TRUE;
	__try{
		__super::CreateWnd();
	}
	__finally{
		Creating = FALSE;
	}
	DoSetMaxLength(MaxLength);
	SetModified(Modified);
	if(PasswordChar != TCHAR('\0'))
		SendMessage(GetHandle(), EM_SETPASSWORDCHAR, PasswordChar, 0);
	UpdateHeight();
}

void CEdit::DestroyWnd(){
	Modified = GetModified();
	__super::DestroyWnd();
}

void CEdit::DefaultHandler(TMessage& Message){
	if(Message.Msg == WM_SETFOCUS && GetGlobal().GetWin32Platform() == VER_PLATFORM_WIN32_WINDOWS &&
		!IsWindow(((PWMSetFocus)&Message)->FocusedWnd))
		((PWMSetFocus)&Message)->FocusedWnd = 0;
	__super::DefaultHandler(Message);
}

void CEdit::UpdateHeight(){
	if(AutoSize && GetBorderStyle() == bsSingle){
		SetControlStyle(GetControlStyle() | csFixedHeight);
		AdjustHeight();
	}
	else
		SetControlStyle(GetControlStyle() & (~csFixedHeight));
}

void CEdit::SetAutoSize(BOOL Value){
	if(AutoSize != Value){
		AutoSize = Value;
		UpdateHeight();
	}
}

void CEdit::SetBorderStyle(TBorderStyle Value){
	if(BorderStyle != Value){
		BorderStyle = Value;
		UpdateHeight();
		RecreateWnd();
	}
}

void CEdit::SetCharCase(TEditCharCase Value){
	if(CharCase != Value){
		CharCase = Value;
		RecreateWnd();
	}
}

void CEdit::SetHideSelection(BOOL Value){
	if(HideSelection != Value){
		HideSelection = Value;
		RecreateWnd();
	}
}

void CEdit::SetMaxLength(INT Value){
	if(MaxLength != Value){
		MaxLength = Value;
		if(HandleAllocated())
			DoSetMaxLength(Value);
	}
}

void CEdit::SetOEMConvert(BOOL Value){
	if(OEMConvert != Value){
		OEMConvert = Value;
		RecreateWnd();
	}
}

void CEdit::DoSetMaxLength(INT Value){
	SendMessage(GetHandle(), EM_LIMITTEXT, Value, 0);
}

BOOL CEdit::GetModified(){
	BOOL Result = Modified;
	if(HandleAllocated())
		Result = SendMessage(GetHandle(), EM_GETMODIFY, 0, 0) != 0;
	return Result;
}

void CEdit::SetModified(BOOL Value){
	if(HandleAllocated())
		SendMessage(GetHandle(), EM_SETMODIFY, Value, 0);
	else
		Modified = Value;
}

void CEdit::SetPasswordChar(TCHAR Value){
	if(PasswordChar != Value){
		PasswordChar = Value;
		if(HandleAllocated()){
			SendMessage(GetHandle(), EM_SETPASSWORDCHAR, PasswordChar, 0);
			SetTextBuf(GetText());
		}
	}
}

void CEdit::SetReadOnly(BOOL Value){
	if(ReadOnly != Value){
		ReadOnly = Value;
		if(HandleAllocated())
			SendMessage(GetHandle(), EM_SETREADONLY, Value, 0);
	}
}

BOOL CEdit::GetCanUndo(){
	BOOL Result = FALSE;
	if(HandleAllocated())
		Result = SendMessage(GetHandle(), EM_CANUNDO, 0, 0) != 0;
	return Result;
}

INT CEdit::GetSelStart(){
	DWORD Result = 0;
	SendMessage(GetHandle(), EM_GETSEL, (WPARAM)&Result, 0);
	return Result;
}

void CEdit::SetSelStart(INT Value){
	SendMessage(GetHandle(), EM_SETSEL, Value, Value);
}

INT CEdit::GetSelLength(){
	DWORD StartPos;
	DWORD EndPos;
	SendMessage(GetHandle(), EM_GETSEL, (WPARAM)&StartPos, (LPARAM)&EndPos);
	return EndPos - StartPos;
}

void CEdit::SetSelLength(INT Value){
	DWORD StartPos;
	DWORD EndPos;
	SendMessage(GetHandle(), EM_GETSEL, (WPARAM)&StartPos, (LPARAM)&EndPos);
	EndPos = StartPos + Value;
	SendMessage(GetHandle(), EM_SETSEL, StartPos, EndPos);
	SendMessage(GetHandle(), EM_SCROLLCARET, 0,0);
}

void CEdit::Clear(){
	SetWindowText(GetHandle(), TEXT(""));
}

void CEdit::ClearSelection(){
	SendMessage(GetHandle(), WM_CLEAR, 0, 0);
}

void CEdit::CopyToClipboard(){
	SendMessage(GetHandle(), WM_COPY, 0, 0);
}

void CEdit::CutToClipboard(){
	SendMessage(GetHandle(), WM_CUT, 0, 0);
}

void CEdit::PasteFromClipboard(){
	SendMessage(GetHandle(), WM_PASTE, 0, 0);
}

void CEdit::Undo(){
	SendMessage(GetHandle(), WM_UNDO, 0, 0);
}

void CEdit::ClearUndo(){
	SendMessage(GetHandle(), EM_EMPTYUNDOBUFFER, 0, 0);
}

void CEdit::SelectAll(){
	SendMessage(GetHandle(), EM_SETSEL, 0, -1);
}

INT CEdit::GetSelTextBuf(LPVOID Buffer, INT BufSize){
	INT StartPos = GetSelStart();
	INT Result = GetSelLength();
	INT Len = GetTextLen() + 1;;
	INT Size = Len * sizeof(TCHAR);
	LPTSTR P = (LPTSTR)malloc(Size);
	BufferHolder pHolder(P);
    GetTextBuf((LPTSTR)P, Size);
	INT BufLen = BufSize / sizeof(TCHAR);
    if(Result > BufLen)
		Result = BufLen;
	if(lstrcpyn((LPTSTR)Buffer, P + StartPos, Result) == NULL)
		Result = 0;
	return Result;
}

void CEdit::SetSelTextBuf(LPTSTR Buffer){
	SendMessage(GetHandle(), EM_REPLACESEL, 0, (LPARAM)Buffer);
}

void CEdit::Change(){
	//__super::Changed();
	if(OnChange != NULL)
		CALL_EVENT(Change)(this);
}