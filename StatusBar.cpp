#include "stdinc.h"
#include "StatusBar.hpp"
#include "SysInit.hpp"
#include "CommonCtl.hpp"
#include "Screen.hpp"
#include "WinUtils.hpp"

IMPL_DYN_CLASS(CStatusPanel)
CStatusPanel::CStatusPanel(CCollection* Collection):CCollectionItem(Collection),
	Width(50),
	Alignment(alLeft),
	Bevel(pbLowered),
	BiDiMode(0),
	ParentBiDiMode(TRUE),
	Style(psText),
	UpdateNeeded(FALSE){
	Text = new String(TEXT(""));
	ParentBiDiModeChanged();
}

CStatusPanel::~CStatusPanel(){
	if(Text != NULL)
		delete Text;
}

BOOL CStatusPanel::IsBiDiModeStored(){
	return !ParentBiDiMode;
}

String CStatusPanel::GetDisplayName(){
	if(Text->Length() == 0)
		return __super::GetDisplayName();
	return *Text;
}

void CStatusPanel::Assign(CObject* Source){
	if(Source != NULL && Source->InstanceOf(CStatusPanel::_Class)){
		CStatusPanel* src = (CStatusPanel *)Source;
		*Text = src->GetText();
		SetWidth(src->GetWidth());
		SetAlignment(src->GetAlignment());
		SetBevel(src->GetBevel());
		SetStyle(src->GetStyle());
	}
	else
		__super::Assign(Source);
}

void CStatusPanel::ParentBiDiModeChanged(){
	if(ParentBiDiMode){
		if(GetOwner() != NULL){
			SetBiDiMode(((CStatusPanels *)GetOwner())->StatusBar->GetBiDiMode());
			ParentBiDiMode = TRUE;
		}
	}
}

BOOL CStatusPanel::UseRightToLeftAlignment(){
	return GetGlobal().GetSysLocaleMiddleEast() && (GetBiDiMode() == bdRightToLeft);
}

BOOL CStatusPanel::UseRightToLeftReading(){
	return GetGlobal().GetSysLocaleMiddleEast() && (GetBiDiMode() != bdLeftToRight);
}

void CStatusPanel::SetAlignment(TAlignment Value){
	if(Alignment != Value){
		Alignment = Value;
		Changed(FALSE);
	}
}

void CStatusPanel::SetBevel(TStatusPanelBevel Value){
	if(Bevel != Value){
		Bevel = Value;
		Changed(FALSE);
	}
}

void CStatusPanel::SetBiDiMode(TBiDiMode Value){
	if(Value != BiDiMode){
		BiDiMode = Value;
		ParentBiDiMode = FALSE;
		Changed(FALSE);
	}
}

void CStatusPanel::SetParentBiDiMode(BOOL Value){
	if(ParentBiDiMode != Value){
		ParentBiDiMode = Value;
		ParentBiDiModeChanged();
	}
}

void CStatusPanel::SetStyle(TStatusPanelStyle Value){
	if(Style != Value){
		Style = Value;
		Changed(FALSE);
	}
}

String CStatusPanel::GetText(){
	return *Text;
}

void CStatusPanel::SetText(String& Value){
	if(*Text != Value){
		*Text = Value;
		Changed(FALSE);
	}
}

void CStatusPanel::SetWidth(INT Value){
	if(Width != Value){
		Width = Value;
		Changed(TRUE);
	}
}

IMPL_DYN_CLASS(CStatusPanels)
	CStatusPanels::CStatusPanels(CStatusBar* StatusBar):CCollection((CCollectionItemClass*)(StatusBar == NULL ? CStatusPanel::_Class : StatusBar->GetPanelClass())){
    this->StatusBar = StatusBar;
}

CStatusPanels::~CStatusPanels(){
}

CComponent* CStatusPanels::GetOwner(){
	return StatusBar;
}

void CStatusPanels::Update(CCollectionItem* Item){
	if(Item != NULL)
		StatusBar->UpdatePanel(Item->GetIndex(), FALSE) ;
	else
		StatusBar->UpdatePanels(TRUE, FALSE);
}

CStatusPanel* CStatusPanels::Add(){
	return (CStatusPanel *)__super::Add();
}

CStatusPanel* CStatusPanels::AddItem(CStatusPanel* Item, INT Index){
	CStatusPanel* Result = NULL;
	if(Item == NULL)
		Result = StatusBar->CreatePanel();
	else
		Result = Item;
	if(Result != NULL){
		Result->SetCollection(this);
		if(Index < 0)
			Index = GetCount() - 1;
		Result->SetIndex(Index);
	}
	return Result;
}

CStatusPanel* CStatusPanels::Insert(INT Index){
	return AddItem(NULL, Index);
}
	
CStatusPanel* CStatusPanels::GetItem(INT Index){
	return (CStatusPanel *)__super::GetItem(Index);
}

void CStatusPanels::SetItem(INT Index, CStatusPanel* Value){
	__super::SetItem(Index, Value);
}

IMPL_DYN_CLASS(CStatusBar)
CStatusBar::CStatusBar(CComponent* AOwner):CWinControl(AOwner),
	SizeGrip(TRUE),
	SimplePanel(FALSE),
	SizeGripValid(FALSE),
	AutoHint(FALSE),
	UseSystemFont(TRUE),
	INIT_EVENT(DrawPanel),
	INIT_EVENT(Hint),
	INIT_EVENT(CreatePanelClass){
	SimpleText = new String(TEXT(""));
	SetControlStyle(csCaptureMouse | csClickEvents | csDoubleClicks | csOpaque);
	SetColor(clBtnFace);
	SetHeight(19);
	SetAlign(alBottom);
	Panels = CreatePanels();
	Canvas = new CControlCanvas();
	((CControlCanvas *)Canvas)->SetControl(this);
	SetParentFont(FALSE);
	SyncToSystemFont();
}

CStatusBar::~CStatusBar(){
	delete Canvas;
	delete Panels;
	delete SimpleText;
}

void CStatusBar::DoRightToLeftAlignment(String& Str, TAlignment AAlignment, BOOL ARTLAlignment){
	if(ARTLAlignment)
		ChangeBiDiModeAlignment(AAlignment);
	if(AAlignment == taCenter)
		Str.Insert(String(TEXT("\t")), 0);
	else if(AAlignment == taRightJustify)
		Str.Insert(String(TEXT("\t\t")), 0);
}

void CStatusBar::SetPanels(CStatusPanels* Value){
	Panels->Assign(Value);
}

void CStatusBar::SetSimplePanel(BOOL Value){
	if(SimplePanel != Value){
		SimplePanel = Value;
		if(HandleAllocated())
			SendMessage(GetHandle(), SB_SIMPLE, SimplePanel, 0);
	}
}

void CStatusBar::UpdateSimpleText(){
	DWORD RTLReading[] = {0, SBT_RTLREADING};
	DoRightToLeftAlignment(*SimpleText, taLeftJustify, UseRightToLeftAlignment());
	if(HandleAllocated())
		SendMessage(GetHandle(), SB_SETTEXT, 255 | RTLReading[UseRightToLeftReading()],
			(LPARAM)SimpleText->GetBuffer());
}

void CStatusBar::SetSimpleText(String& Value){
	if(*SimpleText != Value){
		*SimpleText = Value;
		UpdateSimpleText();
	}
}

void CStatusBar::SetSizeGrip(BOOL Value){
	if(SizeGrip != Value){
		SizeGrip = Value;
		ValidateSizeGrip(TRUE);
	}
}

void CStatusBar::SyncToSystemFont(){
	if(UseSystemFont){
		//TODO SetFont(GetScreen()->GetHintFont());
		GetFont()->SetColor(clBtnText);
	}
}

void CStatusBar::UpdatePanel(INT Index, BOOL Repaint){
	TRect PanelRect = {0, 0, 0, 0};
	if(HandleAllocated()){
		CStatusPanel* Panel = GetPanels()->GetItem(Index);
		if(!Repaint){
			Panel->UpdateNeeded = TRUE;
			SendMessage(GetHandle(), SB_GETRECT, Index, (LPARAM)&PanelRect);
			InvalidateRect(GetHandle(), (const RECT *)&PanelRect, TRUE);
			return;
		}
		else if(!Panel->UpdateNeeded)
			return;
		Panel->UpdateNeeded = FALSE;
		INT Flags = 0;
		if(Panel->GetBevel() == pbNone)
			Flags = SBT_NOBORDERS;
		else if(Panel->GetBevel() == pbRaised)
			Flags = SBT_POPOUT;
		if(UseRightToLeftReading())
			Flags |= SBT_RTLREADING;
		if(Panel->Style == psOwnerDraw)
			Flags |= SBT_OWNERDRAW;
		String S = Panel->GetText();
		if(UseRightToLeftAlignment())
			DoRightToLeftAlignment(S, Panel->GetAlignment(), UseRightToLeftAlignment());
		else{
			if(Panel->GetAlignment() == taCenter)
				S.Insert(String(TEXT("\t")), 0);
			else if(Panel->GetAlignment() == taRightJustify)
				S.Insert(String(TEXT("\t\t")), 0);
		}
		SendMessage(GetHandle(), SB_SETTEXT, Panel->GetIndex() | Flags, (LPARAM)S.GetBuffer());
	}
}

void CStatusBar::UpdatePanels(BOOL UpdateRects, BOOL UpdateText){
	INT MaxPanelCount = 128;
	INT PanelEdges[128] = {0};
	if(HandleAllocated()){
		INT Count = GetPanels()->GetCount();
		if(UpdateRects){
			if(Count > MaxPanelCount)
				Count = MaxPanelCount;
			if(Count == 0){
				PanelEdges[0] = -1;
				SendMessage(GetHandle(), SB_SETPARTS, 1, (LPARAM)&PanelEdges);
				SendMessage(GetHandle(), SB_SETTEXT, 0, (LPARAM)TEXT(""));
			}
			else{
				INT PanelPos = 0;
				for(INT I = 0; I < Count - 1; I++){
					PanelPos += GetPanels()->GetItem(I)->GetWidth();
					PanelEdges[I] = PanelPos;
				}
				PanelEdges[Count - 1] = -1;
				SendMessage(GetHandle(), SB_SETPARTS, Count, (LPARAM)&PanelEdges);
			}
		}
		for(INT I = 0; I < Count; I++)
			UpdatePanel(I, UpdateText);
	}
}

void CStatusBar::CMBiDiModeChanged(TMessage& Message){
	INHERITED_MSG(Message);
	if(HandleAllocated())
		if(!GetSimplePanel()){
			for(INT Loop = 0; GetPanels()->GetCount(); Loop++){
				CStatusPanel* Panel = GetPanels()->GetItem(Loop);
				if(Panel->GetParentBiDiMode())
					Panel->ParentBiDiModeChanged();
			}
			UpdatePanels(TRUE, TRUE);
		}
		else
			UpdateSimpleText();
}

void CStatusBar::CMColorChanged(TMessage& Message){
	INHERITED_MSG(Message);
	RecreateWnd();
}

void CStatusBar::CMParentFontChanged(TMessage& Message){
	INHERITED_MSG(Message);
	if(UseSystemFont && GetParentFont())
		UseSystemFont = FALSE;
}

void CStatusBar::CMSysColorChange(TMessage& Message){
	INHERITED_MSG(Message);
	RecreateWnd();
}

void CStatusBar::CMWinIniChange(TMessage& Message){
	INHERITED_MSG(Message);
	if(Message.wParam == 0 || Message.wParam == SPI_SETNONCLIENTMETRICS)
		SyncToSystemFont();
}

void CStatusBar::CMSysFontChanged(TMessage& Message){
	INHERITED_MSG(Message);
	SyncToSystemFont();
}

void CStatusBar::CNDrawItem(TWMDrawItem& Message){
	LPDRAWITEMSTRUCT DrawItemStruct = Message.DrawItemStruct;
	INT SaveIndex = SaveDC(DrawItemStruct->hDC);
	DCRestorer Restorer(DrawItemStruct->hDC, SaveIndex);
	CMethodLock Lock(Canvas, (TLockMethod)&CCanvas::Lock, (TLockMethod)&CCanvas::UnLock);
    Canvas->SetHandle(DrawItemStruct->hDC);
	CanvasHandleCleaner CanvasCleaner(Canvas);
	Canvas->SetFont(GetFont());
	Canvas->GetBrush()->SetColor(clBtnFace);
    Canvas->GetBrush()->SetStyle(bsSolid);
	DrawPanel(GetPanels()->GetItem(DrawItemStruct->itemID), *((PRect)&DrawItemStruct->rcItem));
	Message.Result = 1;
}

void CStatusBar::WMEraseBkGnd(TWMEraseBkgnd& Message){
	/*
	if(ThemeServices->ThemesEnabled){
		TThemedElementDetails Details = ThemeServices->GetElementDetails(tsStatusRoot);
		ThemeServices->DrawElement(Message.DC, Details, ClientRect, nil);
		Message.Result = 1;
	}
	else //*/
		INHERITED_MSG(Message);
}

void CStatusBar::WMGetTextLength(TWMGetTextLength& Message){
	Message.Result = SimpleText->Length();
}

void CStatusBar::WMPaint(TWMPaint& Message){
	UpdatePanels(FALSE, TRUE);
	INHERITED_MSG(Message);
}

void CStatusBar::WMSize(TWMSize& Message){
	//Eat WM_SIZE message to prevent control from doing alignment
	if(!IN_TEST(csLoading, GetComponentState()))
		Resize();
	Repaint();
}

void CStatusBar::SetUseSystemFont(BOOL Value){
	if(UseSystemFont != Value){
		UseSystemFont = Value;
		if(Value){
			if(GetParentFont())
				SetParentFont(FALSE);
			SyncToSystemFont();
		}
	}
}

void CStatusBar::ValidateSizeGrip(BOOL ARecreate){
	//inherited;
	BOOL LSizeGripValid = FALSE;
	CForm* LForm = GetParentForm(this);
	if(LForm != NULL && (LForm->GetBorderStyle() == bsSizeable || LForm->GetBorderStyle() == bsSizeToolWin)){
		TPoint LPoint = ClientToParent(Point(GetWidth(), GetHeight()), LForm);
		LSizeGripValid = LPoint.x == LForm->GetClientWidth() && LPoint.y == LForm->GetClientHeight();
	}
	if(LSizeGripValid != SizeGripValid){
		SizeGripValid = LSizeGripValid;
		if(ARecreate)
			RecreateWnd();
	}
}

void CStatusBar::ChangeScale(INT M, INT D){//TODO override;
	/*
	if(GetUseSystemFont())// status bar size based on system font size
		SetScalingFlags(sfTop);
	__super::ChangeScale();
	//*/
}

CStatusPanel* CStatusBar::CreatePanel(){
	CStatusPanelClass* LClass = GetPanelClass();
	if(OnCreatePanelClass != NULL)
		CALL_EVENT(CreatePanelClass)(this, *LClass);
	CStatusPanel* Result = (CStatusPanel*)LClass->NewInstance();
	Result->SetCollection(GetPanels());
	return Result;
}

CStatusPanels* CStatusBar::CreatePanels(){
	return new CStatusPanels(this);
}

void CStatusBar::CreateParams(TCreateParams& Params){
	DWORD GripStyles[] = {CCS_TOP, SBARS_SIZEGRIP};
	InitCommonControl(ICC_BAR_CLASSES);
	__super::CreateParams(Params);
	CreateSubClass(Params, STATUSCLASSNAME);
	Params.Style |= GripStyles[SizeGrip && SizeGripValid];
	//if(ThemeServices->ThemesEnabled)
	//	Params.WinClass.style |= CS_HREDRAW | CS_VREDRAW;
	//else
		Params.WinClass.style &= ~(CS_HREDRAW | CS_VREDRAW);
}

void CStatusBar::CreateWnd(){
	ValidateSizeGrip(FALSE);
	__super::CreateWnd();
	SendMessage(GetHandle(), SB_SETBKCOLOR, 0, ColorToRGB(GetColor()));
	UpdatePanels(TRUE, FALSE);
	if(SimpleText->Length() != 0)
		SendMessage(GetHandle(), SB_SETTEXT, 255, (LPARAM)SimpleText->GetBuffer());
	if(SimplePanel)
		SendMessage(GetHandle(), SB_SIMPLE, 1, 0);
}

BOOL CStatusBar::DoHint(){
	if(OnHint != NULL){
		CALL_EVENT(Hint)(this);
		return TRUE;
	}
	else 
		return FALSE;
}

void CStatusBar::DrawPanel(CStatusPanel* Panel, TRect& Rect){
	if(OnDrawPanel != NULL)
		CALL_EVENT(DrawPanel)(this, Panel, Rect);
	else
		Canvas->FillRect(Rect);
}

CStatusPanelClass* CStatusBar::GetPanelClass(){
	return (CStatusPanelClass*)CStatusPanel::_Class;
}

BOOL CStatusBar::IsFontStored(){
	return !UseSystemFont && !GetParentFont();//TODO && !GetDesktopFont();
}

void CStatusBar::SetParent(CWinControl* AParent){
	__super::SetParent(AParent);
	//ValidateSizeGrip(False);
}

//TODO BOOL CStatusBar::ExecuteAction(CBasicAction* Action){}

//TODO void CStatusBar::FlipChildren(BOOL AllLevels) {}

void CStatusBar::SetBounds(INT ALeft, INT ATop, INT AWidth, INT AHeight){
	__super::SetBounds(ALeft, ATop, AWidth, AHeight);
	ValidateSizeGrip(TRUE);
}

