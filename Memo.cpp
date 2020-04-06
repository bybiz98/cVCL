
#include "stdinc.h"
#include "SysInit.hpp"
#include "Memo.hpp"

IMPL_DYN_CLASS(CMemoStrings)
CMemoStrings::CMemoStrings():Memo(NULL){
}
CMemoStrings::~CMemoStrings(){
}

String CMemoStrings::Get(INT Index){
	ULONG Cnt = (ULONG)SendMessage(Memo->GetHandle(), EM_GETLINE, Index, 0);
	LPTSTR Buf = (LPTSTR)malloc((Cnt + 1) * sizeof(TCHAR));
	*((LPWORD)Buf) = (WORD)Cnt;
	SendMessage(Memo->GetHandle(), EM_GETLINE, Index, (LPARAM)Buf);
	*(Buf + Cnt) = TCHAR('\0');
	return String::CreateFor(Buf);
}

INT CMemoStrings::GetCount(){
	INT Result = 0;
	if(Memo->HandleAllocated() || Memo->GetText() != NULL){
		Result = (INT)SendMessage(Memo->GetHandle(), EM_GETLINECOUNT, 0, 0);
		if(SendMessage(Memo->GetHandle(), EM_LINELENGTH, 
			SendMessage(Memo->GetHandle(),EM_LINEINDEX, Result - 1, 0), 0) == 0)
			Result--;
	}
	return Result;
}

String CMemoStrings::GetTextStr(){
	return Memo->GetTextString();
}

void CMemoStrings::Put(INT Index, String& S){
	INT SelStart = (INT)SendMessage(Memo->GetHandle(), EM_LINEINDEX, Index, 0);
	if(SelStart >= 0){
		SendMessage(Memo->GetHandle(), EM_SETSEL, SelStart, SelStart +
			SendMessage(Memo->GetHandle(), EM_LINELENGTH, SelStart, 0));
		SendMessage(Memo->GetHandle(), EM_REPLACESEL, 0, (LPARAM)S.GetBuffer());
	}
}

void CMemoStrings::SetTextStr(String& Value){
	String NewText = Value;//TODO AdjustLineBreaks(Value);
	if(NewText.Length() != Memo->GetTextLen() || NewText != Memo->GetTextString()){
		if(SendMessage(Memo->GetHandle(), WM_SETTEXT, 0, (LPARAM)NewText.GetBuffer()) == 0)
			throw "Text exceeds memo capacity";//raise EInvalidOperation.Create(SInvalidMemoSize);
		Memo->Perform(CM_TEXTCHANGED, 0, 0);
	}
}

void CMemoStrings::SetUpdateState(BOOL Updating){
	if(Memo->HandleAllocated()){
		SendMessage(Memo->GetHandle(), WM_SETREDRAW, (WPARAM)(!Updating), 0);
		if(!Updating){// WM_SETREDRAW causes visibility side effects in memo controls
			Memo->Perform(CM_SHOWINGCHANGED,0,0); // This reasserts the visibility we want
			Memo->Refresh();
		}
	}
}

void CMemoStrings::Clear(){
	Memo->Clear();
}

void CMemoStrings::Delete(INT Index){
	INT SelStart = (INT)SendMessage(Memo->GetHandle(), EM_LINEINDEX, Index, 0);
	if(SelStart >= 0){
		INT SelEnd = (INT)SendMessage(Memo->GetHandle(), EM_LINEINDEX, Index + 1, 0);
		if(SelEnd < 0)
			SelEnd = SelStart + (INT)SendMessage(Memo->GetHandle(), EM_LINELENGTH, SelStart, 0);
		SendMessage(Memo->GetHandle(), EM_SETSEL, SelStart, SelEnd);
		SendMessage(Memo->GetHandle(), EM_REPLACESEL, 0, (LPARAM)TEXT(""));
	}
}

void CMemoStrings::Insert(INT Index, String& S){
	if(Index >= 0){
		INT SelStart = (INT)SendMessage(Memo->GetHandle(), EM_LINEINDEX, Index, 0);
		String Line;
		if(SelStart >= 0)
			Line = S + TEXT("\r\n");
		else {
			SelStart = (INT)SendMessage(Memo->GetHandle(), EM_LINEINDEX, Index - 1, 0);
			if(SelStart < 0)
				return ;
			INT LineLen = (INT)SendMessage(Memo->GetHandle(), EM_LINELENGTH, SelStart, 0);
			if(LineLen == 0)
				return;
			SelStart += LineLen;
			Line = TEXT("\r\n") + S;
		}
		SendMessage(Memo->GetHandle(), EM_SETSEL, SelStart, SelStart);
		SendMessage(Memo->GetHandle(), EM_REPLACESEL, 0, (LPARAM)Line.GetBuffer());
	}
}

IMPL_DYN_CLASS(CMemo)
CMemo::CMemo(CComponent* AOwner):CEdit(AOwner),
	WordWrap(TRUE),
	WantReturns(TRUE),
	Alignment(taLeftJustify),
	ScrollBars(ssNone),
	WantTabs(FALSE){
	SetBounds(GetLeft(), GetTop(), 185, 89);
	SetAutoSize(FALSE);
	Lines = new CMemoStrings();
	((CMemoStrings *)Lines)->Memo = this;
	SetParentBackground(FALSE);
}

CMemo::~CMemo(){
	delete Lines;
}

void CMemo::WMGetDlgCode(TWMGetDlgCode& Message){
	INHERITED_MSG(Message);
	if(WantTabs)
		Message.Result |= DLGC_WANTTAB;
	else 
		Message.Result &= ~DLGC_WANTTAB;
	if(!WantReturns)
		Message.Result &= ~DLGC_WANTALLKEYS;
}

void CMemo::WMNCDestroy(TWMNCDestroy& Message){
	INHERITED_MSG(Message);
}

void CMemo::CreateParams(TCreateParams& Params){
	DWORD Alignments[2][3] = {
		{ES_LEFT, ES_RIGHT, ES_CENTER},
		{ES_RIGHT, ES_LEFT, ES_CENTER}
	};
	DWORD ScrollBar[] = {0, WS_HSCROLL, WS_VSCROLL, WS_HSCROLL | WS_VSCROLL};
	DWORD WordWraps[] = {0, ES_AUTOHSCROLL};
	__super::CreateParams(Params);
	Params.Style &= ~WordWraps[WordWrap];
	Params.Style |= ES_MULTILINE | Alignments[UseRightToLeftAlignment()][Alignment] | ScrollBar[ScrollBars];
}

void CMemo::CreateWindowHandle(const TCreateParams& Params){
	if(GetGlobal().GetSysLocaleFastEast() && GetGlobal().GetWin32Platform() != VER_PLATFORM_WIN32_NT &&
		IN_TEST(ES_READONLY, Params.Style)){
		// Work around Far East Win95 API/IME bug.
		SetWnd(CreateWindowEx(Params.ExStyle, Params.WinClassName, TEXT(""),
			Params.Style & (~ES_READONLY), Params.Left, Params.Top, Params.Width, 
			Params.Height, Params.WndParent, 0, GetGlobal().GetHInstance(), Params.lpParam));
		if(GetWnd() != 0)
			SendMessage(GetWnd(), EM_SETREADONLY, TRUE, 0);
	}
	else 
		SetWnd(CreateWindowEx(Params.ExStyle, Params.WinClassName, TEXT(""), Params.Style,
			Params.Left, Params.Top, Params.Width, Params.Height, 
			Params.WndParent, 0, GetGlobal().GetHInstance(), Params.lpParam));
	SendMessage(GetWnd(), WM_SETTEXT, 0, (LPARAM)GetText());
}

void CMemo::KeyPress(TCHAR& Key){
	__super::KeyPress(Key);
	if(Key == VK_RETURN && !WantReturns)
		Key = TCHAR('\0');
}

void CMemo::Loaded(){
	__super::Loaded();
	SetModified(FALSE);
}

TAlignment CMemo::GetControlsAlignment(){
	return Alignment;
}

TPoint CMemo::GetCaretPos(){
	TPoint Result;
	SendMessage(GetHandle(), EM_GETSEL, 0, (LPARAM)(&Result.x));
	Result.x = HIWORD(SendMessage(GetHandle(), EM_GETSEL, 0, 0));
	Result.y = (LONG)SendMessage(GetHandle(), EM_LINEFROMCHAR, Result.x, 0);
	Result.x = Result.x - (LONG)SendMessage(GetHandle(), EM_LINEINDEX, -1, 0);
	return Result;
}

void CMemo::SetCaretPos(TPoint& Value){
	INT CharIdx = (INT)SendMessage(GetHandle(), EM_LINEINDEX, Value.y, 0) + Value.x;
	SendMessage(GetHandle(), EM_SETSEL, CharIdx, CharIdx);
}

void CMemo::SetLines(CStrings* Value){
	Lines->Assign(*Value);
}

void CMemo::SetAlignment(TAlignment Value){
	if(Alignment != Value){
		Alignment = Value;
		RecreateWnd();
	}
}

void CMemo::SetScrollBars(TScrollStyle Value){
	if(ScrollBars != Value){
		ScrollBars = Value;
		RecreateWnd();
	}
}

void CMemo::SetWordWrap(BOOL Value){
	if(WordWrap != Value){
		WordWrap = Value;
		RecreateWnd();
	}
}
