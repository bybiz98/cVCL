#include "stdinc.h"
#include "WinUtils.hpp"
#include "Label.hpp"

IMPL_DYN_CLASS(CLabel)
CLabel::CLabel(CComponent* AOwner):CGraphicControl(AOwner),
	FocusControl(NULL),
	Alignment(taLeftJustify),
	AutoSize(TRUE),
	Layout(tlCenter),
	WordWrap(FALSE),
	ShowAccelChar(TRUE),
	TransparentSet(FALSE),
	INIT_EVENT(MouseLeave),
	INIT_EVENT(MouseEnter){
	SetControlStyle(GetControlStyle() | csReplicatable);
	SetBounds(0, 0, 65, 17);
	//if ThemeServices.ThemesEnabled then
	//	ControlStyle := ControlStyle - [csOpaque]
	//else
	//	ControlStyle := ControlStyle + [csOpaque];
	SetControlStyle(GetControlStyle() | csOpaque);
}

CLabel::~CLabel(){
}

void CLabel::CMTextChanged(TMessage& Message){
	Invalidate();
	AdjustBounds();
}
void CLabel::CMFontChanged(TMessage& Message){
	INHERITED_MSG(Message);
	AdjustBounds();
}
void CLabel::CMDialogChar(TMessage& Message){
	/*
	if(FocusControl != NULL && GetEnabled() && GetShowAccelChar() &&
		IsAccel(Message.CharCode, Caption)){
		if(FocusControl->GetCanFocus()){
			FocusControl->SetFocus();
			Message.Result = 1;
		}
	}
	//*/
}
void CLabel::CMMouseEnter(TMessage& Message){
	INHERITED_MSG(Message);
	if(OnMouseEnter != NULL)
		CALL_EVENT(MouseEnter)(this);
}
void CLabel::CMMouseLeave(TMessage& Message){
	INHERITED_MSG(Message);
	if(OnMouseLeave != NULL)
		CALL_EVENT(MouseLeave)(this);
}

void CLabel::AdjustBounds(){
	WORD WordWraps[] = {0, DT_WORDBREAK};
	if(!IN_TEST(csReading, GetComponentState()) && AutoSize){
		TRect Rect = GetClientRect();
		HDC DC = GetDC(0);
		GetCanvas()->SetHandle(DC);
		DoDrawText(Rect, (DT_EXPANDTABS | DT_CALCRECT) | WordWraps[WordWrap]);
		GetCanvas()->SetHandle(0);
		ReleaseDC(0, DC);
		INT X = GetLeft();
		TAlignment AAlignment = Alignment;
		if(UseRightToLeftAlignment())
			ChangeBiDiModeAlignment(AAlignment);
		if(AAlignment == taRightJustify)
			X += GetWidth() - Rect.right;
		SetBounds(X, GetTop(), Rect.right, Rect.bottom);
	}
}

void CLabel::Paint(){
	CCanvas* Canvas = GetCanvas();
	CBrush* Brush = Canvas->GetBrush();
	TRect Rect = GetClientRect();
    if(!GetTransparent()){
		Brush->SetColor(this->GetColor());
		Brush->SetStyle(bsSolid);
		Canvas->FillRect(Rect);
	}
	Brush->SetStyle(bsClear);
	WORD Alignments[] = {DT_LEFT, DT_RIGHT, DT_CENTER};
	WORD WordWraps[] = {0, DT_WORDBREAK};
    //{ DoDrawText takes care of BiDi alignments }
    DWORD DrawStyle = DT_EXPANDTABS | WordWraps[WordWrap] | Alignments[Alignment];
    //{ Calculate vertical layout }
    if(Layout != tlTop){
		TRect CalcRect = Rect;
		DoDrawText(CalcRect, DrawStyle | DT_CALCRECT);
		if(Layout == tlBottom)
			OffsetRect((LPRECT)&Rect, 0, GetHeight() - CalcRect.bottom);
		else OffsetRect((LPRECT)&Rect, 0, (GetHeight() - CalcRect.bottom) / 2);
	}
	DoDrawText(Rect, DrawStyle);
}

void CLabel::DoDrawText(TRect& Rect, DWORD Flags){
	//INT Len = this->GetTextLen();
	//INT Size = (Len + 1 ) * sizeof(TCHAR);
	//LPTSTR Text = (LPTSTR)malloc(Size);
	//BufferHolder BufHolder(Text);
	//ZeroMemory(Text, Size);
	//this->GetTextBuf(Text, Size);
	LPTSTR Text = GetText();
	INT Len = lstrlen(Text);
	if((Flags & DT_CALCRECT) != 0){
		if(Len == 0)
			Text = TEXT(" ");
		else if(ShowAccelChar && Text[0] == TCHAR('&') && Text[2] == TCHAR('\0'))
			Text = TEXT("& ");
		
	}
	if(!ShowAccelChar)
		Flags = Flags | DT_NOPREFIX;
	Flags = DrawTextBiDiModeFlags(Flags);
	CCanvas* Canvas = GetCanvas();
	Canvas->SetFont(GetFont());
	if(!GetEnabled()){
		OffsetRect((LPRECT)&Rect, 1, 1);
		Canvas->GetFont()->SetColor(clBtnHighlight);
		DrawText(Canvas->GetHandle(), Text, Len, (LPRECT)&Rect, Flags);
		OffsetRect((LPRECT)&Rect, -1, -1);
		Canvas->GetFont()->SetColor(clBtnShadow);
		DrawText(Canvas->GetHandle(), Text, Len, (LPRECT)&Rect, Flags);
	}
	else 
		DrawText(Canvas->GetHandle(), Text, Len, (LPRECT)&Rect, Flags);
}

INT CLabel::GetLabelText(LPTSTR Buffer, INT BufSize){
	return this->GetTextBuf(Buffer, BufSize);
}

void CLabel::Loaded(){
	__super::Loaded();
	AdjustBounds();
}

void CLabel::Notification(CComponent* AComponent, TOperation Operation){
	__super::Notification(AComponent, Operation);
	if(Operation == opRemove || AComponent == FocusControl)
		FocusControl = NULL;
}

void CLabel::SetAutoSize(BOOL Value){
	if(AutoSize != Value){
		AutoSize = Value;
		AdjustBounds();
	}
}

BOOL CLabel::GetTransparent(){
	return !IN_TEST(csOpaque, GetControlStyle());
}

VOID CLabel::SetTransparent(BOOL Value){
	if(GetTransparent() != Value){
		if(Value)
			SetControlStyle(GetControlStyle() & (~csOpaque));
		else
			SetControlStyle(GetControlStyle() | csOpaque);
		Invalidate();
	}
	TransparentSet = TRUE;
}

VOID CLabel::SetAlignment(TAlignment Value){
	if(Alignment != Value){
		Alignment = Value;
		Invalidate();
	}
}

VOID CLabel::SetFocusControl(CWinControl* Control){
	FocusControl = Control;
	if(Control != NULL) 
		Control->FreeNotification(this);
}

VOID CLabel::SetShowAccelChar(BOOL Value){
	if(ShowAccelChar != Value){
		ShowAccelChar = Value;
		Invalidate();
	}
}

VOID CLabel::SetLayout(TTextLayout Value){
	if(Layout != Value){
		Layout = Value;
		Invalidate();
	}
}

VOID CLabel::SetWordWrap(BOOL Value){
	if(WordWrap != Value){
		WordWrap = Value;
		AdjustBounds();
		Invalidate();
	}
}