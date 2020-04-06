
#include "stdinc.h"
#include "Splitter.hpp"
#include "Form.hpp"

IMPL_DYN_CLASS(CSplitter)
CSplitter::CSplitter(CComponent* AOwner):CGraphicControl(AOwner),
	ActiveControl(NULL),
	AutoSnap(TRUE),
	Beveled(FALSE),
	Brush(NULL),
	Control(NULL),
	LineDC(0),
	LineVisible(FALSE),
	MinSize(30),
	MaxSize(0),
	NewSize(0),
	OldSize(-1),
	PrevBrush(0),
	ResizeStyle(rsPattern),
	Split(0),
	INIT_EVENT(OldKeyDown),
	INIT_EVENT(CanResize),
	INIT_EVENT(Moved),
	INIT_EVENT(Paint)
{
	DownPos.x = 0;
	DownPos.y = 0;
	SetAlign(alLeft);
	SetWidth(3);
	SetCursor(crHSplit);
}

CSplitter::~CSplitter(){
	if(Brush != NULL)
		FreeAndNil((CObject **)&Brush);
}

void CSplitter::AllocateLineDC(){
	LineDC = GetDCEx(GetParent()->GetHandle(), 0, DCX_CACHE | DCX_CLIPSIBLINGS | DCX_LOCKWINDOWUPDATE);
	if(ResizeStyle == rsPattern){
		if(Brush == NULL){
			Brush = new CBrush();
			Brush->SetBitmap(AllocPatternBitmap(clBlack, clWhite));
		}
		PrevBrush = (HBRUSH)SelectObject(LineDC, Brush->GetHandle());
	}
}

void CSplitter::CalcSplitSize(INT X, INT Y, INT& NewSize, INT& Split){
	TAlign Align = GetAlign();
	if(Align == alLeft || Align == alRight)
		Split = X - DownPos.x;
	else
		Split = Y - DownPos.y;
	INT S = 0;
	if(Align == alLeft)
		S = Control->GetWidth() + Split;
	else if(Align == alRight)
		S = Control->GetWidth() - Split;
	else if(Align == alTop)
		S = Control->GetHeight() + Split;
	else if(Align == alBottom)
		S = Control->GetHeight() - Split;
	NewSize = S;
	if(S < MinSize)
		NewSize = MinSize;
	else if (S > MaxSize)
		NewSize = MaxSize;
	if(S != NewSize){
		if(Align == alRight || Align == alBottom)
			S = S - NewSize;
		else 
			S = NewSize - S;
		Split += S;
	}
}

void CSplitter::DrawLine(){
	LineVisible = !LineVisible;
	TPoint P = {GetLeft(), GetTop()};
	if(GetAlign() == alLeft || GetAlign() == alRight)
		P.x = GetLeft() + Split;
	else
		P.y = GetTop() + Split;

	PatBlt(LineDC, P.x, P.y, GetWidth(), GetHeight(), PATINVERT);
}

CControl* CSplitter::FindControl(){
	CControl* Result = NULL;
	TPoint P = {GetLeft(), GetTop()};
	TAlign Align = GetAlign();
	if(Align == alLeft)
		P.x--;
	else if(Align == alRight)
		P.x += GetWidth();
	else if(Align == alTop)
		P.y--;
	else if(Align == alBottom)
		P.y+=GetHeight();
	else return Result;
	for(INT I = 0; I < GetParent()->GetControlCount(); I++){
		Result = GetParent()->GetControl(I);
		if(Result->GetVisible() && Result->GetEnabled()){
			TRect R = Result->GetBoundsRect();
			if(R.right - R.left == 0)
				if(Align == alTop || Align == alLeft)
					R.left--;
				else
					R.right++;
			if (R.bottom - R.top == 0)
				if(Align == alTop || Align == alLeft)
					R.top--;
				else
					R.bottom++;
			if(PtInRect((const RECT *)&R, P))
				return Result;
		}
	}
	return  NULL;
}

void CSplitter::FocusKeyDown(CObject* Sender, WORD& Key, TShiftState Shift){
	if(Key == VK_ESCAPE)
		StopSizing();
	else if(OnOldKeyDown != NULL)
		CALL_EVENT(OldKeyDown)(Sender, Key, Shift);
}

void CSplitter::ReleaseLineDC(){
	if(PrevBrush != 0)
		SelectObject(LineDC, PrevBrush);
	ReleaseDC(GetParent()->GetHandle(), LineDC);
	if(Brush != NULL)
		FreeAndNil((CObject **)&Brush);
}

void CSplitter::SetBeveled(BOOL Value){
	Beveled = Value;
	Repaint();
}

void CSplitter::UpdateControlSize(){
	if(NewSize != OldSize){
		TAlign Align = GetAlign();
		if(Align == alLeft)
			Control->SetWidth(NewSize);
		else if(Align == alTop)
			Control->SetHeight(NewSize);
		else if(Align == alRight){
			GetParent()->DisableAlign();
			__try{
				Control->SetLeft(Control->GetLeft() + Control->GetWidth() - NewSize);
				Control->SetWidth(NewSize);
			}
			__finally{
				GetParent()->EnableAlign();
			}
		}
		else if(Align == alBottom){
			GetParent()->DisableAlign();
			__try{
				Control->SetTop(Control->GetTop() + Control->GetHeight() - NewSize);
				Control->SetHeight(NewSize);
			}
			__finally{
				GetParent()->EnableAlign();
			}
		}
		Update();
		if(OnMoved != NULL)
			CALL_EVENT(Moved)(this);
		OldSize = NewSize;
	}
}

void CSplitter::UpdateSize(INT X, INT Y){
	CalcSplitSize(X, Y, NewSize, Split);
}

BOOL CSplitter::CanResize(INT& NewSize){
	BOOL Result = TRUE;
	if(OnCanResize != NULL)
		CALL_EVENT(CanResize)(this, NewSize, Result);
	return Result;
}

BOOL CSplitter::DoCanResize(INT& NewSize){
	BOOL Result = CanResize(NewSize);
	if(Result && (NewSize <= MinSize) && AutoSnap)
		NewSize = 0;
	return Result;
}

void CSplitter::MouseDown(TMouseButton Button, TShiftState Shift, INT X, INT Y){
	__super::MouseDown(Button, Shift, X, Y);
	if(Button == mbLeft){
		Control = FindControl();
		DownPos.x = X;
		DownPos.y = Y;
		if(Control != NULL){
			TAlign Align = GetAlign();
			if(Align == alLeft || Align == alRight){
				MaxSize = GetParent()->GetClientWidth() - MinSize;
				for(INT I = 0; I < GetParent()->GetControlCount(); I++){
					CControl* c = GetParent()->GetControl(I);
					if(c->GetVisible() && (c->GetAlign() == alLeft || c->GetAlign() == alRight))
						MaxSize -= c->GetWidth();
				}
				MaxSize += Control->GetWidth();
			}
			else {
				MaxSize = GetParent()->GetClientHeight() - MinSize;
				for(INT I = 0; I < GetParent()->GetControlCount(); I++){
					CControl* c = GetParent()->GetControl(I);
					if(c->GetAlign() == alTop || c->GetAlign() == alBottom)
						MaxSize -= c->GetHeight();
				}
				MaxSize += Control->GetHeight();
			}
			UpdateSize(X, Y);
			AllocateLineDC();
			CForm* Form = ValidParentForm(this);
			if(Form->GetActiveControl() != NULL){
				ActiveControl = Form->GetActiveControl();
				SetOnOldKeyDown(ActiveControl->GetOnKeyDownObj(), ActiveControl->GetOnKeyDown());
				ActiveControl->SetOnKeyDown(this, (TKeyEvent)&CSplitter::FocusKeyDown);
			}
			if(ResizeStyle == rsLine || ResizeStyle == rsPattern)
				DrawLine();
		}
	}
}

void CSplitter::MouseMove(TShiftState Shift, INT X, INT Y){
	__super::MouseMove(Shift, X, Y);
	INT N = 0, S = 0;
	if(IN_TEST(ssLeft, Shift) && Control != NULL){
		CalcSplitSize(X, Y, N, S);
		if(DoCanResize(N)){
			if(ResizeStyle == rsLine || ResizeStyle == rsPattern)
				DrawLine();
			NewSize = N;
			Split = S;
			if(ResizeStyle == rsUpdate)
				UpdateControlSize();
			if(ResizeStyle == rsLine || ResizeStyle == rsPattern)
				DrawLine();
		}
	}
}

void CSplitter::MouseUp(TMouseButton Button, TShiftState Shift, INT X, INT Y){
	__super::MouseUp(Button, Shift, X, Y);
	if(Control != NULL){
		if(ResizeStyle == rsLine || ResizeStyle == rsPattern)
			DrawLine();
		UpdateControlSize();
		StopSizing();
	}
}

void CSplitter::Paint(){
	TRect R = GetClientRect();
	CCanvas* Canvas = GetCanvas();
	Canvas->GetBrush()->SetColor(GetColor());
	Canvas->FillRect(GetClientRect());
	TAlign Align = GetAlign();
	if(Beveled){
		if(Align == alLeft || Align == alRight)
			InflateRect((LPRECT)&R, -1, 2);
		else
			InflateRect((LPRECT)&R, 2, -1);
		OffsetRect((LPRECT)&R, 1, 1);
		HBRUSH FrameBrush = CreateSolidBrush(ColorToRGB(clBtnHighlight));
		FrameRect(Canvas->GetHandle(), (const RECT *)&R, FrameBrush);
		DeleteObject(FrameBrush);
		OffsetRect((LPRECT)&R, -2, -2);
		FrameBrush = CreateSolidBrush(ColorToRGB(clBtnShadow));
		FrameRect(Canvas->GetHandle(), (const RECT *)&R, FrameBrush);
		DeleteObject(FrameBrush);
	}
	if(IN_TEST(csDesigning, GetComponentState())){
		Canvas->GetPen()->SetStyle(psDot);
		Canvas->GetPen()->SetMode(pmXor);
		Canvas->GetPen()->SetColor(0x00FFD8CE);
		Canvas->GetBrush()->SetStyle(bsClear);
		Canvas->Rectangle(0, 0, GetClientWidth(), GetClientHeight());
	}
	if(OnPaint != NULL)
		CALL_EVENT(Paint)(this);
}

void CSplitter::RequestAlign(){
	__super::RequestAlign();
	if(GetCursor() != crVSplit && GetCursor() != crHSplit)
		return ;
	if(GetAlign() == alBottom || GetAlign() == alTop)
		SetCursor(crVSplit);
	else
		SetCursor(crHSplit);
}

void CSplitter::StopSizing(){
	if(Control != NULL){
		if (LineVisible)
			DrawLine();
		Control = NULL;
		ReleaseLineDC();
		if(ActiveControl != NULL){
			ActiveControl->SetOnKeyDown(GetOnOldKeyDownObj(), GetOnOldKeyDown());
			ActiveControl = NULL;
		}
	}
	if(OnMoved != NULL)
		CALL_EVENT(Moved)(this);
}

