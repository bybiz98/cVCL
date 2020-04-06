#include "stdinc.h"
#include "SysInit.hpp"
#include "WinUtils.hpp"
#include "GroupBox.hpp"

IMPL_DYN_CLASS(CGroupBox)
CGroupBox::CGroupBox(CComponent* AOwner) : CCustomControl(AOwner){
	SetControlStyle(csAcceptsControls | csCaptureMouse | csClickEvents |
		csSetCaption | csDoubleClicks | csReplicatable | csParentBackground);
	SetBounds(GetLeft(), GetTop(), 185, 105);
}
CGroupBox::~CGroupBox(){
}

void CGroupBox::CMDialogChar(TCMDialogChar& Message){
	if(IsAccel(Message.CharCode, GetText()) && CanFocus()){
		SelectFirst();
		Message.Result = 1;
	}
	else INHERITED_MSG(Message);
}

void CGroupBox::CMTextChanged(TMessage& Message){
	Invalidate();
	Realign();
}


void CGroupBox::CMCtl3DChanged(TMessage& Message){
	INHERITED_MSG(Message);
	Invalidate();
	Realign();
}

void CGroupBox::WMSize(TMessage& Message){
	INHERITED_MSG(Message);
	Invalidate();
}

void CGroupBox::AdjustClientRect(TRect& Rect){
	__super::AdjustClientRect(Rect);
	GetCanvas()->SetFont(GetFont());
	Rect.top += GetCanvas()->TextHeight(TEXT("0"));
	InflateRect((LPRECT)&Rect, -1, -1);
	if(GetCtl3D())
		InflateRect((LPRECT)&Rect, -1, -1);
}

void CGroupBox::CreateParams(TCreateParams& Params){
	__super::CreateParams(Params);
	Params.WinClass.style &= ~(CS_HREDRAW | CS_VREDRAW);
}

void CGroupBox::Paint(){
	CCanvas* Canvas = GetCanvas();
	Canvas->SetFont(GetFont());
	/*
	if(ThemeServices->ThemesEnabled()){
		SIZE Size = {0,0};
		TRect CaptionRect = {0,0,0,0};
		if(GetText() != TEXT("")){
			GetTextExtentPoint32(Canvas->GetHandle(), GetText(), lstrlen(GetText()), &Size);
			CaptionRect = Rect(0, 0, Size.cx, Size.cy);
			if(!UseRightToLeftAlignment())
				OffsetRect((LPRECT)&CaptionRect, 8, 0);
			else
				OffsetRect((LPRECT)&CaptionRect, GetWidth() - 8 - CaptionRect.right, 0);
		}
		TRect OuterRect = GetClientRect();
		OuterRect.top = (CaptionRect.bottom - CaptionRect.top) / 2;
		ExcludeClipRect(Canvas->GetHandle(), CaptionRect.left, CaptionRect.top, CaptionRect.right, CaptionRect.bottom);
		TThemedButton Box = 0;
		if(GetEnabled())
			Box = tbGroupBoxNormal;
		else
			Box = tbGroupBoxDisabled;
		TThemedElementDetails Details = ThemeServices->GetElementDetails(Box);
		ThemeServices.DrawElement(Canvas->GetHandle(), Details, OuterRect);
		SelectClipRgn(Canvas.GetHandle(), 0);
		if(GetText() != TEXT(""))
			ThemeServices.DrawText(Canvas->GetHandle(), Details, GetText(), CaptionRect, DT_LEFT, 0);
	}
	else //*/
	{
		INT H = Canvas->TextHeight(TEXT("0"));
		TRect R = Rect(0, H / 2 - 1, GetWidth(), GetHeight());
		if(GetCtl3D()){
			R.left++;
			R.top++;
			Canvas->GetBrush()->SetColor(clBtnHighlight);
			Canvas->FrameRect(R);
			OffsetRect((LPRECT)&R, -1, -1);
			Canvas->GetBrush()->SetColor(clBtnShadow);
		}
		else
			Canvas->GetBrush()->SetColor(clWindowFrame);
		Canvas->FrameRect(R);
		String Text = GetTextString();
		if(Text.Length() > 0){
			if(!UseRightToLeftAlignment())
				R = Rect(8, 0, 0, H);
			else
				R = Rect(R.right - Canvas->TextWidth(GetText()) - 8, 0, 0, H);
			LONG Flags = DrawTextBiDiModeFlags(DT_SINGLELINE);
			DrawText(Canvas->GetHandle(), Text.GetBuffer(), Text.Length(), (LPRECT)&R, Flags | DT_CALCRECT);
			Canvas->GetBrush()->SetColor(GetColor());
			DrawText(Canvas->GetHandle(), Text.GetBuffer(), Text.Length(), (LPRECT)&R, Flags);
		}
	}
}

class CGroupButton : public CRadio{
private:
	BOOL InClick;
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CN_COMMAND, CGroupButton::CNCommand)
	MSG_MAP_END()
	void CNCommand(TWMCommand& Message);
protected:
    void KeyDown(WORD& Key, TShiftState Shift) override;
	void KeyPress(TCHAR& Key) override;
public:
    CGroupButton(CRadioGroup* RadioGroup = NULL);
	virtual ~CGroupButton();

	REF_DYN_CLASS(CGroupButton)
};
DECLARE_DYN_CLASS(CGroupButton, CRadio)

IMPL_DYN_CLASS(CGroupButton)
CGroupButton::CGroupButton(CRadioGroup* RadioGroup):CRadio(RadioGroup),
	InClick(FALSE){
	RadioGroup->GetButtons()->Add(this);
	SetVisible(FALSE);
	SetEnabled(RadioGroup->GetEnabled());
	SetParentShowHint(FALSE);
	SetOnClick(RadioGroup, (TNotifyEvent)&CRadioGroup::ButtonClick);
	SetParent(RadioGroup);
}

CGroupButton::~CGroupButton(){
	((CRadioGroup *)GetOwner())->GetButtons()->Remove(this);
}

void CGroupButton::CNCommand(TWMCommand& Message){
	if(!InClick){
		InClick = TRUE;
		__try{
			if((Message.NotifyCode == BN_CLICKED || 
				Message.NotifyCode == BN_DOUBLECLICKED) && 
				((CRadioGroup *)GetParent())->CanModify())
				INHERITED_MSG(Message);
		}
		__except(EXCEPTION_EXECUTE_HANDLER){
			GetGlobal().CallHandleException(this);
		}
		InClick = FALSE;
	}
}

void CGroupButton::KeyPress(TCHAR& Key){
	__super::KeyPress(Key);
	((CRadioGroup*)GetParent())->KeyPress(Key);
	if(Key == TCHAR(8) || Key == TCHAR(' ')){
		if(!((CRadioGroup *)GetParent())->CanModify())
			Key = TCHAR('\0');
	}
}

void CGroupButton::KeyDown(WORD& Key, TShiftState Shift){
	__super::KeyDown(Key, Shift);
	((CRadioGroup *)GetParent())->KeyDown(Key, Shift);
}

IMPL_DYN_CLASS(CRadioGroup)
CRadioGroup::CRadioGroup(CComponent* AOwner):CGroupBox(AOwner),
	ItemIndex(-1),
	Columns(1),
	Reading(FALSE),
	Updating(FALSE){
	SetControlStyle(csSetCaption | csDoubleClicks | csParentBackground);
	Buttons = new CList();
	Items = new CStringList();
	((CStringList *)Items)->SetOnChange(this, (TNotifyEvent)&CRadioGroup::ItemsChange);
}

CRadioGroup::~CRadioGroup(){
	SetButtonCount(0);
	((CStringList *)Items)->SetOnChange(NULL, NULL);
	delete Items;
	delete Buttons;
}

CRadio* CRadioGroup::GetButton(INT Index){
	return (CRadio*)Buttons->Get(Index);
}

void CRadioGroup::ArrangeButtons(){
	if(Buttons->GetCount() != 0 && !Reading){
		HDC DC = GetDC(0);
		HGDIOBJ SaveFont = SelectObject(DC, GetFont()->GetHandle());
		TEXTMETRIC Metrics;
		GetTextMetrics(DC, &Metrics);
		SelectObject(DC, SaveFont);
		ReleaseDC(0, DC);
		INT BtnCount = Buttons->GetCount();
		INT ButtonsPerCol = (BtnCount + Columns - 1) / Columns;
		INT ButtonWidth = (GetWidth() - 10) / Columns;
		INT I = GetHeight() - Metrics.tmHeight - 5;
		INT ButtonHeight = I / ButtonsPerCol;
		INT TopMargin = Metrics.tmHeight + 1 + (I % ButtonsPerCol) / 2;
		HDWP DeferHandle = BeginDeferWindowPos(Buttons->GetCount());
		__try{
			for(I = 0; I < BtnCount; I++){
				CGroupButton* GroupButton = (CGroupButton*)Buttons->Get(I);
				GroupButton->SetBiDiMode(GetBiDiMode());
				INT ALeft = (I / ButtonsPerCol) * ButtonWidth + 8;
				if(GroupButton->UseRightToLeftAlignment())
					ALeft = GetClientWidth() - ALeft - ButtonWidth;
				DeferHandle = DeferWindowPos(DeferHandle, GroupButton->GetHandle(), 0,
					ALeft, (I % ButtonsPerCol) * ButtonHeight + TopMargin,
					ButtonWidth, ButtonHeight,
					SWP_NOZORDER | SWP_NOACTIVATE);
				GroupButton->SetVisible(TRUE);
			}
		}
		__finally{
			EndDeferWindowPos(DeferHandle);
		}
	}
}

void CRadioGroup::ButtonClick(CObject* Sender){
	if(!Updating){
		ItemIndex = Buttons->IndexOf(this);
		Changed();
		Click();
	}
}

void CRadioGroup::ItemsChange(CObject* Sender){
	if(!Reading){
		if(ItemIndex >= Items->GetCount())
			ItemIndex = Items->GetCount() - 1;
		UpdateButtons();
	}
}

void CRadioGroup::SetButtonCount(INT Value){
	while(Buttons->GetCount() < Value)
		new CGroupButton(this);
	while(Buttons->GetCount() > Value)
		delete (CGroupButton*)Buttons->Last();
}

void CRadioGroup::SetColumns(INT Value){
	if(Value < 1)
		Value = 1;
	if(Value > 16)
		Value = 16;
	if(Columns != Value){
		Columns = Value;
		ArrangeButtons();
		Invalidate();
	}
}

void CRadioGroup::SetItemIndex(INT Value){
	if(Reading)
		ItemIndex = Value;
	else{
		if(Value < -1)
			Value = -1;
		if(Value >= Buttons->GetCount())
			Value = Buttons->GetCount() - 1;
		if(ItemIndex != Value){
			if(ItemIndex >= 0)
				((CGroupButton *)Buttons->Get(ItemIndex))->SetChecked(FALSE);
			ItemIndex = Value;
			if(ItemIndex >= 0)
				((CGroupButton *)Buttons->Get(ItemIndex))->SetChecked(TRUE);
		}
	}
}

void CRadioGroup::SetItems(CStrings* Value){
	Items->Assign(*Value);
}

void CRadioGroup::UpdateButtons(){
	INT Count = Items->GetCount();
	SetButtonCount(Count);
	for(INT I = 0; I < Count; I++)
		((CGroupButton *)Buttons->Get(I))->SetText(Items->Get(I).GetBuffer());
	if(ItemIndex >= 0){
		Updating = TRUE;
		((CGroupButton *)Buttons->Get(ItemIndex))->SetChecked(TRUE);
		Updating = FALSE;
	}
	ArrangeButtons();
	Invalidate();
}

void CRadioGroup::CMEnabledChanged(TMessage& Message){
	INHERITED_MSG(Message);
	INT Count = Buttons->GetCount();
	for(INT I = 0; I < Count; I++)
		((CGroupButton *)Buttons->Get(I))->SetEnabled(GetEnabled());
}

void CRadioGroup::CMFontChanged(TMessage& Message){
	INHERITED_MSG(Message);
	ArrangeButtons();
}

void CRadioGroup::WMSize(TWMSize& Message){
	INHERITED_MSG(Message);
	ArrangeButtons();
}

void CRadioGroup::Loaded(){
	__super::Loaded();
	ArrangeButtons();
}

BOOL CRadioGroup::CanModify(){
	return TRUE;
}

//TODO void CRadioGroup::ReadState(CReader* Reader){}
//TODO void CRadioGroup::GetChildren(TGetChildProc Proc, CComponent* Root){}
//TODO void CRadioGroup::FlipChildren(BOOL AllLevels){}
