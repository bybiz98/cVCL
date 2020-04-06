#include "stdinc.h"
#include "SysInit.hpp"
#include "WinUtils.hpp"
#include "Panel.hpp"

IMPL_DYN_CLASS(CPanel)
CPanel::CPanel(CComponent* AOwner):CCustomControl(AOwner),
	Alignment(taCenter),
	BorderStyle(bsNone),
	FullRepaint(TRUE),
	AutoSizeDocking(FALSE),
	BevelInner(bvNone),
	BevelOuter(bvNone),
	BevelWidth(0),
	BorderWidth(0),
	Locked(FALSE),
	ParentBackgroundSet(FALSE)
	{
	SetControlStyle(csAcceptsControls | csCaptureMouse | csClickEvents |
		csSetCaption | csOpaque | csDoubleClicks | csReplicatable);
	//When themes are on in an application default to making TCustomPanel's paint with their ParentBackground
	//TODO if(ThemeServices->GetThemesEnabled())
	//	SetControlStyle((GetControlStyle() | csParentBackground) & (~csOpaque));
	SetBounds(GetLeft(), GetTop(), 185, 41);
	SetBevelOuter(bvRaised);
	SetBevelWidth(1);
	SetColor(clBtnFace);
	//TODO SetUseDockManager(TRUE);
}

CPanel::~CPanel(){
}

void CPanel::CMBorderChanged(TMessage& Message){
	INHERITED_MSG(Message);
	Invalidate();
}

void CPanel::CMTextChanged(TMessage& Message){
	Invalidate();
}

void CPanel::CMCtl3DChanged(TMessage& Message){
	if(GetGlobal().GetNewStyleControls() && BorderStyle == bsSingle)
		RecreateWnd();
	INHERITED_MSG(Message);
}

void CPanel::CMIsToolControl(TMessage& Message){
	if(!Locked)
		Message.Result = 1;
}

void CPanel::WMWindowPosChanged(TWMWindowPosChanged& Message){
	if(GetFullRepaint() || GetTextString().Length() >= 0)
		Invalidate();
	else{
		INT BevelPixels = GetBorderWidth();
		if(GetBevelInner() != bvNone)
			BevelPixels += GetBevelWidth();
		if(GetBevelOuter() != bvNone)
			BevelPixels += GetBevelWidth();
		TRect Rect = {0, 0, 0, 0};
		if(BevelPixels > 0){
			Rect.right = GetWidth();
			Rect.bottom = GetHeight();
			if(Message.WindowPos->cx != Rect.right){
				Rect.top = 0;
				Rect.left = Rect.right - BevelPixels - 1;
				InvalidateRect(GetHandle(), (const RECT *)&Rect, TRUE);
			}
			if(Message.WindowPos->cy != Rect.bottom){
				Rect.left = 0;
				Rect.top = Rect.bottom - BevelPixels - 1;
				InvalidateRect(GetHandle(), (const RECT *)&Rect, TRUE);
			}
		}
	}
	INHERITED_MSG(Message);
}

void CPanel::CMDockClient(TCMDockClient& Message){
	/* TODO
	if(GetAutoSize()){
		AutoSizeDocking = TRUE;
		__try{
			TRect R = Message->DockSource->GetDockRect();
			switch(GetAlign()){
			case alLeft:
				if(GetWidth() == 0)
					SetWidth(R.right - R.left);
				break;
			case alRight:
				if(GetWidth() == 0){
					INT Dim = R.right - R.left;
					SetBounds(GetLeft() - Dim, GetTop(), Dim, GetHeight());
				}
				break;
			case alTop:
				if(GetHeight() == 0)
					SetHeight(R.bottom - R.top);
				break;
			case alBottom:
				if(GetHeight() == 0){
					INT Dim = R.bottom - R.top;
					SetBounds(GetLeft(), GetTop() - Dim, GetWidth(), Dim);
				}
				break;
			}
			INHERITED_MSG(Message);
			return;
		}
		__finally{
			AutoSizeDocking = FALSE;
		}
	}//*/
	INHERITED_MSG(Message);
}

void CPanel::SetAlignment(TAlignment Value){
	Alignment = Value;
	Invalidate();
}

void CPanel::SetBevelInner(TPanelBevel Value){
	BevelInner = Value;
	Realign();
	Invalidate();
}

void CPanel::SetBevelOuter(TPanelBevel Value){
	BevelOuter = Value;
	Realign();
	Invalidate();
}

void CPanel::SetBevelWidth(TBevelWidth Value){
	BevelWidth = Value;
	Realign();
	Invalidate();
}

void CPanel::SetBorderWidth(TBorderWidth Value){
	BorderWidth = Value;
	Realign();
	Invalidate();
}

void CPanel::SetBorderStyle(TBorderStyle Value){
	if(BorderStyle != Value){
		BorderStyle = Value;
		RecreateWnd();
	}
}

void CPanel::CreateParams(TCreateParams& Params){
	DWORD BorderStyles[] = {0, WS_BORDER};
	__super::CreateParams(Params);
	Params.Style |= BorderStyles[BorderStyle];
	if(GetGlobal().GetNewStyleControls() && GetCtl3D() && BorderStyle == bsSingle){
		Params.Style &= ~WS_BORDER;
		Params.ExStyle |= WS_EX_CLIENTEDGE;
	}
	Params.WinClass.style &= ~(CS_HREDRAW | CS_VREDRAW);
}

void CPanel::AdjustClientRect(TRect& Rect){
	__super::AdjustClientRect(Rect);
	InflateRect((LPRECT)&Rect, 0-GetBorderWidth(), 0-GetBorderWidth());
	INT BevelSize = 0;
	if(GetBevelOuter() != bvNone)
		BevelSize += GetBevelWidth();
	if(GetBevelInner() != bvNone)
		BevelSize += GetBevelWidth();
	InflateRect((LPRECT)&Rect, -BevelSize, -BevelSize);
}

BOOL CPanel::CanAutoSize(INT& NewWidth, INT& NewHeight){
	return !AutoSizeDocking && __super::CanAutoSize(NewWidth, NewHeight);
}

void AdjustColors(TPanelBevel Bevel, TColor& TopColor, TColor& BottomColor){
	TopColor = clBtnHighlight;
	if(Bevel == bvLowered)
		TopColor = clBtnShadow;
	BottomColor = clBtnShadow;
	if(Bevel == bvLowered)
		BottomColor = clBtnHighlight;
}

void CPanel::Paint(){
	TColor TopColor = crDefault;
	TColor BottomColor = crDefault;
	TRect Rect = GetClientRect();
	if(GetBevelOuter() != bvNone){
		AdjustColors(GetBevelOuter(), TopColor, BottomColor);
		Frame3D(GetCanvas(), Rect, TopColor, BottomColor, GetBevelWidth());
	}
	Frame3D(GetCanvas(), Rect, GetColor(), GetColor(), GetBorderWidth());
	if(GetBevelInner() != bvNone){
		AdjustColors(GetBevelInner(), TopColor, BottomColor);
		Frame3D(GetCanvas(), Rect, TopColor, BottomColor, GetBevelWidth());
	}
	CCanvas* Canvas = GetCanvas();
	if(//TODO !ThemeServices->GetThemesEnabled() || 
		!GetParentBackground()){
		Canvas->GetBrush()->SetColor(GetColor());
		Canvas->FillRect(Rect);
	}
	Canvas->GetBrush()->SetStyle(bsClear);
	Canvas->SetFont(GetFont());
	INT FontHeight = Canvas->TextHeight(TEXT("W"));
	Rect.top = ((Rect.bottom + Rect.top) - FontHeight) / 2;
	Rect.bottom = Rect.top + FontHeight;
	UINT Alignments[] = {DT_LEFT, DT_RIGHT, DT_CENTER};
	UINT Flags = DT_EXPANDTABS | DT_VCENTER | Alignments[Alignment];
    Flags = DrawTextBiDiModeFlags(Flags);
	DrawText(Canvas->GetHandle(), GetTextString().GetBuffer(), -1, (LPRECT)&Rect, Flags);
}

void CPanel::SetParentBackground(BOOL Value){
	//CCustomPanel needs to not have csOpaque when painting with the ParentBackground in Themed applications 
	if(Value)
		SetControlStyle(GetControlStyle() & (~csOpaque));
	else
		SetControlStyle(GetControlStyle() | csOpaque);
	ParentBackgroundSet = TRUE;
	__super::SetParentBackground(Value);
}

TAlignment CPanel::GetControlsAlignment(){
	return Alignment;
}

void Frame3D(CCanvas* Canvas, TRect& Rect, TColor TopColor, TColor BottomColor, INT Width){
	Canvas->GetPen()->SetWidth(1);
	Rect.bottom--;
	Rect.right--;
	TPoint tmp;
	TPoint Pts[3];
	while(Width > 0){
		Width--;
		Canvas->GetPen()->SetColor(TopColor);
		Pts[0] = Point(Rect.left, Rect.bottom);
		Pts[1] = Rect.TopLeft;
		Pts[2] = Point(Rect.right, Rect.top);
		Canvas->Polyline((const POINT *)&(Pts[0]), 3);
		Canvas->GetPen()->SetColor(BottomColor);
		Pts[0].x--;
		tmp = Pts[0];
		Pts[0] = Pts[2];
		Pts[2] = tmp;
		Pts[1] = Rect.BottomRight;
		Canvas->Polyline((const POINT *)&(Pts[0]), 3);
		InflateRect((LPRECT)&Rect, -1, -1);
	}
	Rect.bottom++;
	Rect.right++;
}
	