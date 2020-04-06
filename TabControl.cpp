#include "stdinc.h"
#include "WinUtils.hpp"
#include "SysInit.hpp"
#include "TabControl.hpp"
#include "Form.hpp"
#include "CommonCtl.hpp"

class CTabStrings : public CStrings{
private:
	friend class CTabControl;
    CTabControl* TabControl;
protected:
	String Get(INT Index) override;
	INT GetCount() override;
	CObject* GetObject(INT Index) override;
	void Put(INT Index, String& S) override;
	void PutObject(INT Index, CObject* AObject) override;
	void SetUpdateState(BOOL Updating) override;
public:
	CTabStrings();
	virtual ~CTabStrings();
	void Clear() override;
	void Delete(INT Index) override;
	void Insert(INT Index, String& S) override;

	REF_DYN_CLASS(CTabStrings)
};
DECLARE_DYN_CLASS(CTabStrings, CStrings)

IMPL_DYN_CLASS(CTabStrings)
CTabStrings::CTabStrings():TabControl(NULL){
}

CTabStrings::~CTabStrings(){
}

String CTabStrings::Get(INT Index){
	LONG RTL[] = {0, TCIF_RTLREADING};
	TCHAR Buffer[4096]={0};
	TCITEM TCItem;
	ZeroMemory(&TCItem, sizeof(TCItem));
	TCItem.mask = TCIF_TEXT | RTL[TabControl->UseRightToLeftReading()];
	TCItem.pszText = Buffer;
	TCItem.cchTextMax = 4096;
	if(SendMessage(TabControl->GetHandle(), TCM_GETITEM, Index,
		(LPARAM)&TCItem) == 0)
		throw "Failed to retrieve tab at index %d";//TabControlError(Format(sTabFailRetrieve, [Index]));
	return String(Buffer);
}

INT CTabStrings::GetCount(){
	return (INT)SendMessage(TabControl->GetHandle(), TCM_GETITEMCOUNT, 0, 0);
}

CObject* CTabStrings::GetObject(INT Index){
	TCITEM TCItem;
	ZeroMemory(&TCItem, sizeof(TCItem));
	TCItem.mask = TCIF_PARAM;
	if(SendMessage(TabControl->GetHandle(), TCM_GETITEM, Index, (LPARAM)&TCItem) == 0)
		throw "Failed to get object at index %d";//TabControlError(Format(sTabFailGetObject, [Index]));
	return (CObject *)TCItem.lParam;
}

void CTabStrings::Put(INT Index, String& S){
	LONG RTL[] = {0, TCIF_RTLREADING};
	TCITEM TCItem;
	ZeroMemory(&TCItem, sizeof(TCItem));
	TCItem.mask = TCIF_TEXT | RTL[TabControl->UseRightToLeftReading()] | TCIF_IMAGE;
	TCItem.pszText = S.GetBuffer();
	TCItem.iImage = TabControl->GetImageIndex(Index);
	if(SendMessage(TabControl->GetHandle(), TCM_SETITEM, Index, (LPARAM)&TCItem) == 0)
		throw "Failed to set tab %s at index %d";//TabControlError(Format(sTabFailSet, [S, Index]));
	TabControl->TabsChanged();
}

void CTabStrings::PutObject(INT Index, CObject* AObject){
	TCITEM TCItem;
	ZeroMemory(&TCItem, sizeof(TCItem));
	TCItem.mask = TCIF_PARAM;
	TCItem.lParam = (LPARAM)AObject;
	if(SendMessage(TabControl->GetHandle(), TCM_SETITEM, Index, (LPARAM)&TCItem) == 0)
		throw "Failed to set object at index %d";//TabControlError(Format(sTabFailSetObject, [Index]));
}

void CTabStrings::SetUpdateState(BOOL Updating){
	TabControl->SetUpdating(Updating);
	SendMessage(TabControl->GetHandle(), WM_SETREDRAW, !Updating, 0);
	if(!Updating){
		TabControl->Invalidate();
		TabControl->TabsChanged();
	}
}

void CTabStrings::Clear(){
	if(SendMessage(TabControl->GetHandle(), TCM_DELETEALLITEMS, 0, 0) == 0)
		throw "Failed to clear tab control";//TabControlError(sTabFailClear);
	TabControl->TabsChanged();
}

void CTabStrings::Delete(INT Index){
	if(SendMessage(TabControl->GetHandle(), TCM_DELETEITEM, Index, 0) == 0)
		throw "Failed to delete tab at index %d";//TabControlError(Format(sTabFailDelete, [Index]));
	TabControl->TabsChanged();
}

void CTabStrings::Insert(INT Index, String& S){
	LONG RTL[] = {0, TCIF_RTLREADING};
	TCITEM TCItem;
	ZeroMemory(&TCItem, sizeof(TCItem));
	TCItem.mask = TCIF_TEXT | RTL[TabControl->UseRightToLeftReading()] | TCIF_IMAGE;
	TCItem.pszText = S.GetBuffer();
	TCItem.iImage = TabControl->GetImageIndex(Index);
	if(SendMessage(TabControl->GetHandle(), TCM_INSERTITEM, Index, (LPARAM)&TCItem) < 0)
		throw "Failed to set tab %s at index %d";//TabControlError(Format(sTabFailSet, [S, Index]));
	TabControl->TabsChanged();
}

IMPL_DYN_CLASS(CTabControl)
CTabControl::CTabControl(CComponent* AOwner):CWinControl(AOwner),
	HotTrack(FALSE),
	Images(NULL),
	MultiLine(FALSE),
	MultiSelect(FALSE),
	OwnerDraw(FALSE),
	RaggedRight(FALSE),
	SaveTabIndex(0),
	SaveTabs(NULL),
	ScrollOpposite(FALSE),
	Style(tsTabs),
	TabPosition(tpTop),
	Updating(FALSE),
	INIT_EVENT(Change),
	INIT_EVENT(Changing),
	INIT_EVENT(DrawTab),
	INIT_EVENT(GetImageIndex){
	SetBounds(GetLeft(), GetTop(), 289, 193);
	SetTabStop(TRUE);
	SetControlStyle(csAcceptsControls | csDoubleClicks);
	Tabs = new CTabStrings();
	((CTabStrings *)Tabs)->TabControl = this;
	Canvas = new CControlCanvas();
	((CControlCanvas *)Canvas)->SetControl(this);
	ImageChangeLink = new CChangeLink();
	ImageChangeLink->SetOnChange(this, (TNotifyEvent)&CTabControl::ImageListChange);
	TabSize.x = 0;
	TabSize.y = 0;
	SavedAdjustRect.left = 0;
	SavedAdjustRect.top = 0;
	SavedAdjustRect.right = 0;
	SavedAdjustRect.bottom = 0;
}

CTabControl::~CTabControl(){
	FreeAndNil((CObject**)&Canvas);
	FreeAndNil((CObject**)&Tabs);
	if(SaveTabs != NULL)
		FreeAndNil((CObject**)&SaveTabs);
	FreeAndNil((CObject**)&ImageChangeLink);
}

TRect CTabControl::GetDisplayRect(){
	TRect Result = GetClientRect();
	SendMessage(GetHandle(), TCM_ADJUSTRECT, 0, (LPARAM)&Result);
	if(GetTabPosition() == tpTop)
		Result.top += 2;
	return Result;
}

INT CTabControl::GetTabIndex(){
	return (INT)SendMessage(GetHandle(), TCM_GETCURSEL, 0, 0);
}

void CTabControl::SetHotTrack(BOOL Value){
	if(HotTrack != Value){
		HotTrack = Value;
		RecreateWnd();
	}
}

void CTabControl::SetImages(CCustomImageList* Value){
	if(Images != NULL)
		Images->UnRegisterChanges(ImageChangeLink);
	Images = Value;
	if(Images != NULL){
		Images->RegisterChanges(ImageChangeLink);
		Images->FreeNotification(this);
		Perform(TCM_SETIMAGELIST, 0, (LPARAM)Images->GetHandle());
	}
	else 
		Perform(TCM_SETIMAGELIST, 0, 0);
}

void CTabControl::SetMultiLine(BOOL Value){
	if(InternalSetMultiLine(Value))
		RecreateWnd();
}

void CTabControl::SetMultiSelect(BOOL Value){
	if(MultiSelect != Value){
		MultiSelect = Value;
		RecreateWnd();
	}
}

void CTabControl::SetOwnerDraw(BOOL Value){
	if(OwnerDraw != Value){
		OwnerDraw = Value;
		RecreateWnd();
	}
}

void CTabControl::SetRaggedRight(BOOL Value){
	if(RaggedRight != Value){
		RaggedRight = Value;
		SetComCtlStyle(this, TCS_RAGGEDRIGHT, Value);
	}
}

void CTabControl::SetScrollOpposite(BOOL Value){
	if(ScrollOpposite != Value){
		ScrollOpposite = Value;
		if(Value)
			MultiLine = Value;
		RecreateWnd();
	}
}

void CTabControl::SetStyle(TTabStyle Value){
	if(Style != Value){
		if(Value != tsTabs && TabPosition != tpTop)
			throw "invalid tab style";//raise EInvalidOperation.Create(SInvalidTabStyle);
		SetParentBackground(Value == tsTabs);
		Style = Value;
		RecreateWnd();
	}
}

void CTabControl::SetTabHeight(SHORT Value){
	if(TabSize.y != Value){
		if(Value < 0)
			throw "property out of range.";//raise EInvalidOperation.CreateFmt(SPropertyOutOfRange, [Self.Classname]);
		TabSize.y = Value;
		UpdateTabSize();
	}
}

void CTabControl::SetTabPosition(TTabPosition Value){
	if(TabPosition != Value){
		if(Value != tpTop && Style != tsTabs)
			throw "invalid tab position.";//raise EInvalidOperation.Create(SInvalidTabPosition);
		TabPosition = Value;
		if(!MultiLine && (Value == tpLeft || Value == tpRight))
			InternalSetMultiLine(TRUE);
		RecreateWnd();
	}
}

void CTabControl::SetTabs(CStrings* Value){
	Tabs->Assign(*Value);
}

void CTabControl::SetTabWidth(SHORT Value){
	if(TabSize.x != Value){
		if(Value < 0)
			throw "property out of range.";//raise EInvalidOperation.CreateFmt(SPropertyOutOfRange, [Self.Classname]);
		INT OldValue = TabSize.x;
		TabSize.x = Value;
		if(OldValue == 0 || Value == 0)
			RecreateWnd();
		else
			UpdateTabSize();
	}
}

void CTabControl::ImageListChange(CObject* Sender){
	Perform(TCM_SETIMAGELIST, 0, (LPARAM)((CCustomImageList*)Sender)->GetHandle());
}

BOOL CTabControl::InternalSetMultiLine(BOOL Value){
	BOOL Result = MultiLine != Value;
	if(Result){
		if(!Value && (TabPosition == tpLeft || TabPosition == tpRight))
			throw "tab must be multi line";//TabControlError(sTabMustBeMultiLine);
		MultiLine = Value;
		if(!Value)
			ScrollOpposite = FALSE;
	}
	return Result;
}

void CTabControl::TabsChanged(){
	if(!Updating){
		if(HandleAllocated())
			SendMessage(GetHandle(), WM_SIZE, SIZE_RESTORED,
				((WORD)GetWidth()) & (((WORD)GetHeight()) << 16));
		Realign();
	}
}

void CTabControl::UpdateTabSize(){
	SendMessage(GetHandle(), TCM_SETITEMSIZE, 0, MAKELPARAM(TabSize.x, TabSize.y));
	TabsChanged();
}

void CTabControl::CMFontChanged(TMessage& Message){
	INHERITED_MSG(Message);
	if(HandleAllocated())
		Perform(WM_SIZE, 0, 0);
}

void CTabControl::CMSysColorChange(TMessage& Message){
	INHERITED_MSG(Message);
	if(!IN_TEST(csLoading, GetComponentState())){
		Message.Msg = WM_SYSCOLORCHANGE;
		DefaultHandler(Message);
	}
}

void CTabControl::CMTabStopChanged(TMessage& Message){
	if(!IN_TEST(csDesigning, GetComponentState()))
		RecreateWnd();
}

void CTabControl::CNNotify(TWMNotify& Message){
	if(Message.NMHdr->code == TCN_SELCHANGE)
		Change();
	else if(Message.NMHdr->code == TCN_SELCHANGING){
		Message.Result = 1;
		if(CanChange())
			Message.Result = 0;
	}
}

void CTabControl::CMDialogChar(TCMDialogChar& Message){
	INT Count = Tabs->GetCount();
	for(INT I = 0; I <Count; I++){
		if(IsAccel(Message.CharCode, Tabs->Get(I).GetBuffer()) && CanShowTab(I) && CanFocus()){
			Message.Result = 1;
			if(CanChange()){
				SetTabIndex(I);
				Change();
			}
			return ;
		}
	}
	INHERITED_MSG(Message);
}

void CTabControl::CNDrawItem(TWMDrawItem& Message){
	
	HDC DC = Message.DrawItemStruct->hDC;
	INT SaveIndex = SaveDC(DC);
	DCRestorer dcRestorer(DC, SaveIndex);
	CMethodLock cvasLock(Canvas, (TLockMethod)&CCanvas::Lock, (TLockMethod)&CCanvas::UnLock);
	Canvas->SetHandle(DC);
	CanvasHandleCleaner dcCleaner(Canvas);
	Canvas->SetFont(GetFont());
	Canvas->SetBrush(GetBrush());
	DrawTab(Message.DrawItemStruct->itemID, *((PRect)&(Message.DrawItemStruct->rcItem)),
		(Message.DrawItemStruct->itemState & ODS_SELECTED) != 0);
	Message.Result = 1;
}

void CTabControl::TCMAdjustRect(TMessage& Message){
	//Major hack around a problem in the Windows tab control. Don't try this
	//at home. The tab control (4.71) will AV when in a TCM_ADJUSTRECT message
	//when the height of the control is the same as the height of the tab (or the
	//width of the control for tpBottom). This hack will return the last value
	//successfully returned if an exception is encountered. This allows the
	//control to function but the AV is still generated and and reported by the
	//debugger.
	__try{
		INHERITED_MSG(Message);
		if(GetTabPosition() != tpTop && Message.wParam == 0)
			SavedAdjustRect = *((PRect)Message.lParam);
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		*((PRect)Message.lParam) = SavedAdjustRect;
	}
}

void CTabControl::WMDestroy(TWMDestroy& Message){
	if(Tabs != NULL && Tabs->GetCount() > 0){
		SaveTabs = new CStringList();
		SaveTabs->Assign(*Tabs);
		SaveTabIndex = GetTabIndex();
	}
	HWND FocusHandle = GetFocus();
	if(FocusHandle != 0 && (FocusHandle == GetHandle() ||
		IsChild(GetHandle(), FocusHandle)))
		::SetFocus(0);
	INHERITED_MSG(Message);
	SetWnd(0);
}

void CTabControl::WMNotifyFormat(TMessage& Message){
	Message.Result = DefWindowProc(GetHandle(), Message.Msg, Message.wParam, Message.lParam);
}

void CTabControl::WMSize(TMessage& Message){
	INHERITED_MSG(Message);
	RedrawWindow(GetHandle(), NULL, 0, RDW_INVALIDATE | RDW_ERASE);
}

void CTabControl::AdjustClientRect(TRect& Rect){
	Rect = GetDisplayRect();
	__super::AdjustClientRect(Rect);
}

BOOL CTabControl::CanChange(){
	BOOL Result = TRUE;
	if(OnChanging != NULL)
		CALL_EVENT(Changing)(this, Result);
	return Result;
}

BOOL CTabControl::CanShowTab(INT TabIndex){
	return TRUE;
}

void CTabControl::Change(){
	if(OnChange != NULL)
		CALL_EVENT(Change)(this);
}

void CTabControl::CreateParams(TCreateParams& Params){
	DWORD AlignStyles[2][4] = {
		{0, TCS_BOTTOM, TCS_VERTICAL, TCS_VERTICAL | TCS_RIGHT},
		{0, TCS_BOTTOM, TCS_VERTICAL | TCS_RIGHT, TCS_VERTICAL}
	};
	DWORD TabStyles[] = {TCS_TABS, TCS_BUTTONS, TCS_BUTTONS | TCS_FLATBUTTONS};
	DWORD RRStyles[] = {0, TCS_RAGGEDRIGHT};
	InitCommonControl(ICC_TAB_CLASSES);
	__super::CreateParams(Params);
	CreateSubClass(Params, WC_TABCONTROL);
	Params.Style |= WS_CLIPCHILDREN |
		AlignStyles[UseRightToLeftAlignment()][TabPosition] |
		TabStyles[Style] | RRStyles[RaggedRight];
	if(!GetTabStop())
		Params.Style |= TCS_FOCUSNEVER;
	if(MultiLine)
		Params.Style |= TCS_MULTILINE;
	if(MultiSelect)
		Params.Style |= TCS_MULTISELECT;
	if(OwnerDraw)
		Params.Style |= TCS_OWNERDRAWFIXED;
	if(TabSize.x != 0)
		Params.Style |= TCS_FIXEDWIDTH;
	if(HotTrack && !IN_TEST(csDesigning, GetComponentState()))
		Params.Style |= TCS_HOTTRACK;
	if(ScrollOpposite)
		Params.Style |= TCS_SCROLLOPPOSITE;
	Params.WinClass.style &= ~(CS_HREDRAW | CS_VREDRAW);
	Params.WinClass.style |= CS_DBLCLKS;
}

void CTabControl::CreateWnd(){
	__super::CreateWnd();
	if(Images != NULL && Images->HandleAllocated())
		Perform(TCM_SETIMAGELIST, 0, (LPARAM)Images->GetHandle());
	if(TabSize.x != 0 && TabSize.y != 0)
		UpdateTabSize();
	if(SaveTabs != NULL){
		Tabs->Assign(*SaveTabs);
		SetTabIndex(SaveTabIndex);
		delete SaveTabs;
		SaveTabs = NULL;
	}
}

void CTabControl::DrawTab(INT TabIndex, TRect& Rect, BOOL Active){
	if(OnDrawTab != NULL)
		CALL_EVENT(DrawTab)(this, TabIndex, Rect, Active);
	else
		Canvas->FillRect(Rect);
}

INT CTabControl::GetImageIndex(INT TabIndex){
	INT Result = TabIndex;
	if(OnGetImageIndex != NULL)
		CALL_EVENT(GetImageIndex)(this, TabIndex, Result);
	return Result;
}

void CTabControl::Loaded(){
	__super::Loaded();
	if(Images != NULL)
		UpdateTabImages();
}

void CTabControl::UpdateTabImages(){
	TCITEM TCItem;
	ZeroMemory(&TCItem, sizeof(TCItem));
	TCItem.mask = TCIF_IMAGE;
	INT Count = Tabs->GetCount();
	for(INT I = 0; I < Count; I++){
		TCItem.iImage = GetImageIndex(I);
		if(SendMessage(GetHandle(), TCM_SETITEM, I, (LPARAM)&TCItem) == 0)
			throw "Failed to set tab %s at index %d";//TabControlError(Format(sTabFailSet, [FTabs[I], I]));
	}
	TabsChanged();
}

INT CTabControl::IndexOfTabAt(INT X, INT Y){
	INT Result = -1;
	TCHITTESTINFO HitTest;
	if(PtInRect((const RECT*)&GetClientRect(), Point(X, Y))){
		HitTest.flags = 0;
		HitTest.pt.x = X;
		HitTest.pt.y = Y;
		Result = TabCtrl_HitTest(GetHandle(), &HitTest);
	}
	return Result;
}

THitTests CTabControl::GetHitTestInfoAt(INT X, INT Y){
	THitTests Result = 0;
	TCHITTESTINFO HitTest;
	if(PtInRect((const RECT*)&GetClientRect(), Point(X, Y))){
		HitTest.flags = 0;
		HitTest.pt.x = X;
		HitTest.pt.y = Y;
		if(TabCtrl_HitTest(GetHandle(), &HitTest) != -1){
			if((HitTest.flags & TCHT_NOWHERE) != 0)
				Result |= htNowhere;
			if((HitTest.flags & TCHT_ONITEM) == TCHT_ONITEM)
				Result |= htOnItem;
			else{
				if((HitTest.flags & TCHT_ONITEM) != 0)
					Result |= htOnItem;
				if((HitTest.flags & TCHT_ONITEMICON) != 0)
					Result |= htOnIcon;
				if((HitTest.flags & TCHT_ONITEMLABEL) != 0)
					Result |= htOnLabel;
			}
		}
		else
			Result = htNowhere;
	}
	return Result;
}

TRect CTabControl::TabRect(INT Index){
	TRect Result = {0, 0, 0, 0};
	TabCtrl_GetItemRect(GetHandle(), Index, (RECT *)&Result);
	return Result;
}

INT CTabControl::RowCount(){
	return TabCtrl_GetRowCount(GetHandle());
}

void CTabControl::ScrollTabs(INT Delta){
	TRect Rect = {0, 0, 0, 0};
	TPoint P = {0, 0};
	HWND Wnd = FindWindowEx(GetHandle(), 0, TEXT("msctls_updown32"), NULL);
	if(Wnd != 0){
		::GetClientRect(Wnd, (LPRECT)&Rect);
		if(Delta < 0)
			P.x = Rect.left + 2;
		else
			P.x = Rect.right - 2;
		P.y = Rect.top + 2;
		for(INT I = 0; I < abs(Delta); I++){
			SendMessage(Wnd, WM_LBUTTONDOWN, 0, MAKELPARAM(P.x, P.y));
			SendMessage(Wnd, WM_LBUTTONUP, 0, MAKELPARAM(P.x, P.y));
		}
	}
}

void CTabControl::Notification(CComponent* AComponent, TOperation Operation){
	__super::Notification(AComponent, Operation);
	if(Operation == opRemove && AComponent == Images)
		SetImages(NULL);
}

void CTabControl::SetTabIndex(INT Value){
	SendMessage(GetHandle(), TCM_SETCURSEL, Value, 0);
}

SHORT CTabControl::GetTabHeight(){
	return TabSize.y;
}

SHORT CTabControl::GetTabWidth(){
	return TabSize.x;
}

IMPL_DYN_CLASS(CTabSheet)
CTabSheet::CTabSheet(CComponent* AOwner):CWinControl(AOwner),
	PageControl(NULL),
	ImageIndex(0),
	TabVisible(TRUE),
	Highlighted(FALSE),
	TabShowing(FALSE),
	INIT_EVENT(Hide),
	INIT_EVENT(Show){
	SetAlign(alClient);
	SetControlStyle(GetControlStyle() | csAcceptsControls | csNoDesignVisible | csParentBackground);
	SetVisible(FALSE);
}

CTabSheet::~CTabSheet(){
	if(PageControl != NULL){
		if(PageControl->UndockingPage == this)
			PageControl->UndockingPage = NULL;
		PageControl->RemovePage(this);
	}
}

INT CTabSheet::GetPageIndex(){
	if(PageControl != NULL)
		return PageControl->Pages->IndexOf(this);
	else
		return -1;
}

INT CTabSheet::GetTabIndex(){
	INT Result = 0;
	if(!TabShowing)
		Result--;
	else{
		for(INT I = 0; I < GetPageIndex(); I++)
			if(((CTabSheet*)PageControl->Pages->Get(I))->TabShowing)
				Result++;
	}
	return Result;
}

void CTabSheet::SetHighlighted(BOOL Value){
	if(!IN_TEST(csReading, GetComponentState()))
		SendMessage(PageControl->GetHandle(), TCM_HIGHLIGHTITEM, (WPARAM)GetTabIndex(),
			MAKELPARAM((WORD)Value, 0));
	Highlighted = Value;
}

void CTabSheet::SetImageIndex(TImageIndex Value){
	if(ImageIndex != Value){
		ImageIndex = Value;
		if(TabShowing)
			PageControl->UpdateTab(this);
	}

}

void CTabSheet::SetPageControl(CPageControl* APageControl){
	if(PageControl != APageControl){
		if(PageControl != NULL)
			PageControl->RemovePage(this);
		SetParent(APageControl);
		if(APageControl != NULL)
			APageControl->InsertPage(this);
	}
}

void CTabSheet::SetPageIndex(INT Value){
	if(PageControl != NULL){
		INT MaxPageIndex = PageControl->Pages->GetCount() - 1;
		if(Value != MaxPageIndex)
			throw "%d is an invalid PageIndex value.  PageIndex must be between 0 and %d";//raise EListError.CreateResFmt(@SPageIndexError, [Value, MaxPageIndex]);
		INT I = GetTabIndex();
		PageControl->Pages->Move(GetPageIndex(), Value);
		if(I >= 0)
			PageControl->MoveTab(I, GetTabIndex());
	}
}

void CTabSheet::SetTabShowing(BOOL Value){
	if(TabShowing != Value){
		if(Value){
			TabShowing = TRUE;
			PageControl->InsertTab(this);
		}
		else{
			INT Index = GetTabIndex();
			TabShowing = FALSE;
			PageControl->DeleteTab(this, Index);
		}
	}
}

void CTabSheet::SetTabVisible(BOOL Value){
	if(TabVisible != Value){
		TabVisible = Value;
		UpdateTabShowing();
	}
}

void CTabSheet::UpdateTabShowing(){
	SetTabShowing((PageControl != NULL) && TabVisible);
}

void CTabSheet::CMTextChanged(TMessage& Message){
	if(TabShowing)
		PageControl->UpdateTab(this);
}

void CTabSheet::CMShowingChanged(TMessage& Message){
	INHERITED_MSG(Message);
	if(GetShowing()){
		__try{
			DoShow();
		}
		__except(EXCEPTION_EXECUTE_HANDLER){
			GetGlobal().CallHandleException(this);
		}
	}
	else {
		if(!GetShowing()){
			__try{
				DoHide();
			}
			__except(EXCEPTION_EXECUTE_HANDLER){
				GetGlobal().CallHandleException(this);
			}
		}
	}
}

void CTabSheet::WMNCPaint(TWMNCPaint& Message){
	/*TODO
	if(ThemeServices->GetThemesEnabled() && GetBorderWidth() > 0){
		HDC DC = GetWindowDC(GetHandle());
		__try{
			TRect DrawRect = GetClientRect();
			OffsetRect((LPRECT)&DrawRect, GetBorderWidth(), GetBorderWidth());
			ExcludeClipRect(DC, DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom);
			SetWindowOrgEx(DC, -GetBorderWidth(), -GetBorderWidth(), NULL);
			//TODO Details := GetElementDetails(ttBody);
			//DrawParentBackground(Handle, DC, @Details, False);
		}
		__finally{
			ReleaseDC(Handle, DC);
		}
		Message.Result = 0;
	}
    else //*/
		INHERITED_MSG(Message);
}

void CTabSheet::WMPrintClient(TWMPrintClient& Message){
	/* TODO
	if(ThemeServices->ThemesEnabled){
		DrawParentBackground(Handle, Message.DC, nil, False);
		Message.Result = 1;
	}
	else //*/
		INHERITED_MSG(Message);

}

void CTabSheet::CreateParams(TCreateParams& Params){
	__super::CreateParams(Params);
	//if(!ThemeServices.ThemesAvailable)
	Params.WinClass.style &= ~(CS_HREDRAW | CS_VREDRAW);
}

void CTabSheet::DoHide(){
	if(OnHide != NULL)
		CALL_EVENT(Hide)(this);
}

void CTabSheet::DoShow(){
	if(OnShow != NULL)
		CALL_EVENT(Show)(this);
}

//TODO void CTabSheet::ReadState(CReader* Reader){}

IMPL_DYN_CLASS(CPageControl)
CPageControl::CPageControl(CComponent* AOwner) : CTabControl(AOwner),
	ActivePage(NULL),
	NewDockSheet(NULL),
	UndockingPage(NULL),
	InSetActivePage(FALSE){
	SetControlStyle(csDoubleClicks | csOpaque);
	Pages = new CList();
}

CPageControl::~CPageControl(){
	INT Count = Pages->GetCount();
	for(INT I = 0; I < Count; I++)
		((CTabSheet *)Pages->Get(I))->PageControl = NULL;
	delete Pages;
}

void CPageControl::ChangeActivePage(CTabSheet* Page){
	if(ActivePage != Page){
		CForm* ParentForm = GetParentForm(this);
		if(ParentForm != NULL && ActivePage != NULL && 
			ActivePage->ContainsControl(ParentForm->GetActiveControl())){
			ParentForm->SetActiveControl(ActivePage);
			if(ParentForm->GetActiveControl() != ActivePage){
				SetTabIndex(ActivePage->GetTabIndex());
				return ;
			}
		}
		if(Page != NULL){
			Page->BringToFront();
			Page->SetVisible(TRUE);
			if(ParentForm != NULL && ActivePage != NULL &&
				ParentForm->GetActiveControl() == ActivePage){
				if(Page->CanFocus())
					ParentForm->SetActiveControl(Page);
				else
					ParentForm->SetActiveControl(this);
			}
		}
		if(ActivePage != NULL)
			ActivePage->SetVisible(FALSE);
		ActivePage = Page;
		if(ParentForm != NULL && ActivePage != NULL && 
			ParentForm->GetActiveControl() == ActivePage)
			ActivePage->SelectFirst();
	}
}

void CPageControl::DeleteTab(CTabSheet* Page, INT Index){
	BOOL UpdateIndex = Page == GetActivePage();
	GetTabs()->Delete(Index);
	if(UpdateIndex){
		if(Index >= GetTabs()->GetCount())
			INT Index = GetTabs()->GetCount() - 1;
		SetTabIndex(Index);
	}
	UpdateActivePage();
}

INT CPageControl::GetActivePageIndex(){
	if(GetActivePage() != NULL)
		return GetActivePage()->GetPageIndex();
	else
		return -1;
}

CControl* CPageControl::GetDockClientFromMousePos(TPoint& MousePos){
	CControl* Result = NULL;
	/*TODO
	TCHITTESTINFO HitTestInfo;
	if(GetDockSite()){
		HitTestInfo.pt = MousePos;
		INT HitIndex = SendMessage(GetHandle(), TCM_HITTEST, 0, (LPARAM)&HitTestInfo);
		if(HitIndex >= 0){
			CTabSheet* Page = NULL;
			for(INT i = 0; i <= HitIndex; i++)
				Page = FindNextPage(Page, TRUE, TRUE);
			if(Page != NULL && Page->GetControlCount() > 0){
				Result = Page->GetControl(0);
				if(Result->GetHostDockSite() != this)
					Result = NULL;
			}
		}
	}//*/
	return Result;
}

CTabSheet* CPageControl::GetPage(INT Index){
	return (CTabSheet *)Pages->Get(Index);
}

INT CPageControl::GetPageCount(){
	return Pages->GetCount();
}

void CPageControl::InsertPage(CTabSheet* Page){
	Pages->Add(Page);
	Page->PageControl = this;
	Page->UpdateTabShowing();
}

void CPageControl::InsertTab(CTabSheet* Page){
	GetTabs()->InsertObject(Page->GetTabIndex(), Page->GetTextString(), Page);
	UpdateActivePage();
}

void CPageControl::MoveTab(INT CurIndex, INT NewIndex){
	GetTabs()->Move(CurIndex, NewIndex);
}

void CPageControl::RemovePage(CTabSheet* Page){
	CTabSheet* NextSheet = FindNextPage(Page, TRUE, !IN_TEST(csDesigning, GetComponentState()));
	if(NextSheet == Page)
		NextSheet = NULL;
	Page->SetTabShowing(FALSE);
	Page->PageControl = NULL;
	Pages->Remove(Page);
	SetActivePage(NextSheet);
}

void CPageControl::SetActivePageIndex(INT Value){
	if(Value > -1 && Value < GetPageCount())
		SetActivePage((CTabSheet *)Pages->Get(Value));
	else
		SetActivePage(NULL);
}

void CPageControl::UpdateTab(CTabSheet* Page){
	GetTabs()->Put(Page->GetTabIndex(), Page->GetTextString());
}

void CPageControl::UpdateTabHighlights(){
	INT Count = GetPageCount();
	for(INT I = 0; I < Count; I++)
		((CTabSheet *)Pages->Get(I))->SetHighlighted(((CTabSheet *)Pages->Get(I))->Highlighted);
}

void CPageControl::CMDesignHitTest(TCMDesignHitTest& Message){
	TCHITTESTINFO HitTestInfo;
	ZeroMemory(&HitTestInfo, sizeof(HitTestInfo));
	HitTestInfo.pt = SmallPointToPoint(Message.Pos);
	INT HitIndex = (INT)SendMessage(GetHandle(), TCM_HITTEST, 0, (LPARAM)&HitTestInfo);
	if(HitIndex >= 0 && HitIndex != GetTabIndex())
		Message.Result = 1;
}

void CPageControl::CMDialogKey(TCMDialogKey& Message){
	if((Focused() || ::IsChild(GetHandle(), ::GetFocus())) &&
		Message.CharCode == VK_TAB && GetKeyState(VK_CONTROL) < 0){
		SelectNextPage(GetKeyState(VK_SHIFT) >= 0);
		Message.Result = 1;
	}
	else
		INHERITED_MSG(Message);
}

void CPageControl::CMDockClient(TCMDockClient& Message){
	INHERITED_MSG(Message);
	/*TODO
	Message.Result = 0;
	CTabSheet* NewDockSheet = new CTabSheet(this);
	try
    try
      DockCtl := Message.DockSource.Control;
      if DockCtl is TCustomForm then
        FNewDockSheet.Caption := TCustomForm(DockCtl).Caption;
      FNewDockSheet.PageControl := Self;
      DockCtl.Dock(Self, Message.DockSource.DockRect);
    except
      FNewDockSheet.Free;
      raise;
    end;
    IsVisible := DockCtl.Visible;
    FNewDockSheet.TabVisible := IsVisible;
    if IsVisible then ActivePage := FNewDockSheet;
    DockCtl.Align := alClient;
  finally
    FNewDockSheet := nil;
  end;
  //*/
}

void CPageControl::CMDockNotification(TCMDockNotification& Message){
	CTabSheet* Page = GetPageFromDockClient(Message.Client);
	if(Page != NULL){
		if(Message.NotifyRec->ClientMsg == WM_SETTEXT){
			LPTSTR S = (LPTSTR)Message.NotifyRec->MsgLParam;
			INT Len = -1;
			//Search for first CR/LF and end string there 
			for(INT I = 0; I < lstrlen(S); I++){
				if(*(S + I)==TCHAR('\r') || *(S + I)==TCHAR('\n')){
					Len = I;
					break;
				}
			}
			String SS(S, 0, Len);
			Page->SetText(SS.GetBuffer());
		}
		else if(Message.NotifyRec->ClientMsg == CM_VISIBLECHANGED){
			Page->SetTabVisible((BOOL)Message.NotifyRec->MsgWParam);
		}
	}
	INHERITED_MSG(Message);
}

void CPageControl::CMUnDockClient(TCMUnDockClient& Message){
	Message.Result = 0;
	CTabSheet* Page = GetPageFromDockClient(Message.Client);
	if(Page != NULL){
		UndockingPage = Page;
		Message.Client->SetAlign(alNone);
	}
}

void CPageControl::WMLButtonDown(TWMLButtonDown& Message){
	INHERITED_MSG(Message);
	/*TODO
	CControl* DockCtl = GetDockClientFromMousePos(SmallPointToPoint(Message.Pos));
	if(DockCtl != NULL && GetStyle() == tsTabs)
		DockCtl->BeginDrag(FALSE);
	//*/
}

void CPageControl::WMLButtonDblClk(TWMLButtonDblClk& Message){
	INHERITED_MSG(Message);
	/*TODO
	CControl* DockCtl = GetDockClientFromMousePos(SmallPointToPoint(Message.Pos));
	if(DockCtl != NULL)
		DockCtl->ManualDock(NULL, NULL, alNone);
	//*/
}

void CPageControl::WMEraseBkGnd(TWMEraseBkgnd& Message){
	if(/*TODO !ThemeServices.ThemesEnabled || */!GetParentBackground())
		INHERITED_MSG(Message);
	else
		Message.Result = 1;

}

BOOL CPageControl::CanShowTab(INT TabIndex){
	return ((CTabSheet *)Pages->Get(TabIndex))->GetEnabled();
}

void CPageControl::Change(){
	if(GetTabIndex() >= 0)
		UpdateActivePage();
	/*TODO
	if(IN_TEST(csDesigning, GetComponentState())){
		CForm* Form = GetParentForm(this);
		if(Form != NULL && Form->GetDesigner() != NULL)
			Form->GetDesigner()->GetModified();
	}//*/
	__super::Change();
}

//void CPageControl::DoAddDockClient(CControl* Client, TRect& ARect){}
//void CPageControl::DockOver(CDragDockObject* Source, INT X, INT Y, TDragState State, BOOL& Accept){}
//void CPageControl::DoRemoveDockClient(CControl* Client){}
//void CPageControl::GetChildren(TGetChildProc* Proc, CComponent* Root){}

INT CPageControl::GetImageIndex(INT TabIndex){
	if(GetOnGetImageIndex() != NULL)
		return __super::GetImageIndex(TabIndex);
	else {
		//For a PageControl, TabIndex refers to visible tabs only. The control doesn't store
		INT Visible = 0;
		INT NotVisible = 0;
		INT Count = Pages->GetCount();
		for(INT I = 0; I < Count; I++){
			if(!GetPage(I)->GetTabVisible())
				NotVisible++;
			else
				Visible++;
			if(Visible == TabIndex + 1)
				break;
		}
		return GetPage(TabIndex + NotVisible)->GetImageIndex();
	}
}

CTabSheet* CPageControl::GetPageFromDockClient(CControl* Client){
	/*TODO
	INT Count = GetPageCount();
	for(INT I = 0; I < Count; I++){
		if(Client->GetParent() == GetPage(I) && Client->GetHostDockSite() == this){
			return GetPage(I);
		}
	}//*/
	return NULL;
}

//void CPageControl::GetSiteInfo(CControl* Client, TRect& InfluenceRect, TPoint& MousePos, BOOL& CanDock){}

void CPageControl::Loaded(){
	__super::Loaded();
	UpdateTabHighlights();
}

void CPageControl::SetActivePage(CTabSheet* Page){
	if(Page != NULL && Page->PageControl != this)
		return;
	InSetActivePage = TRUE;
	__try{
		ChangeActivePage(Page);
		if(Page == NULL)
			SetTabIndex(-1);
		else
			if(Page == ActivePage)
				SetTabIndex(Page->GetTabIndex());
	}
	__finally{
		InSetActivePage = FALSE;
	}
}

void CPageControl::SetChildOrder(CComponent* Child, INT Order){
	((CTabSheet *)Child)->SetPageIndex(Order);
}

void CPageControl::SetTabIndex(INT Value){
	__super::SetTabIndex(Value);
	if(!InSetActivePage && Value >= 0 && Value < Pages->GetCount() && 
		GetPage(Value)->GetTabVisible()){
		SetActivePage(GetPage(Value));
	}
}

void CPageControl::ShowControl(CControl* AControl){
	if(AControl != NULL && AControl->InstanceOf(CTabSheet::_Class) && 
		((CTabSheet *)AControl)->GetPageControl() == this)
		SetActivePage((CTabSheet*)AControl);
	__super::ShowControl(AControl);
}

void CPageControl::UpdateActivePage(){
	if(GetTabIndex() >= 0)
		SetActivePage((CTabSheet*)GetTabs()->GetObject(GetTabIndex()));
	else
		SetActivePage(NULL);
}

CTabSheet* CPageControl::FindNextPage(CTabSheet* CurPage, BOOL GoForward, BOOL CheckTabVisible){
	CTabSheet* Result = NULL;
	if(Pages->GetCount() != 0){
		INT StartIndex = Pages->IndexOf(CurPage);
		if(StartIndex == -1)
			if(GoForward)
				StartIndex = Pages->GetCount() - 1;
			else 
				StartIndex = 0;
		INT I = StartIndex;
		do{
			if(GoForward){
				I++;
				if(I == Pages->GetCount())
					I = 0;
			}
			else{
				if(I == 0)
					I = Pages->GetCount();
				I--;
			}
			Result = (CTabSheet*)Pages->Get(I);
			if(!CheckTabVisible || Result->GetTabVisible())
				return Result;
		}while(I != StartIndex);
	}
	return NULL;
}

void CPageControl::SelectNextPage(BOOL GoForward, BOOL CheckTabVisible){
	CTabSheet* Page = FindNextPage(GetActivePage(), GoForward, CheckTabVisible);
	if(Page != NULL && Page != GetActivePage() && CanChange()){
		SetActivePage(Page);
		Change();
	}
}
