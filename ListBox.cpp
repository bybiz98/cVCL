#include "stdinc.h"
#include "SysInit.hpp"
#include "WinUtils.hpp"
#include "ListBox.hpp"

class CListBoxStrings : public CStrings{
private:
	friend class CListBox;
    CListBox* ListBox;
protected:
    void Put(INT Index, String& S) override;
	String Get(INT Index) override;
	INT GetCount() override;
	CObject* GetObject(INT Index) override;
	void PutObject(INT Index, CObject* AObject) override;
	void SetUpdateState(BOOL Updating) override;
public:
	CListBoxStrings();
	virtual ~CListBoxStrings();
	INT Add(String& S) override;
	void Clear() override;
    void Delete(INT Index) override;
	void Exchange(INT Index1, INT Index2) override;
	INT IndexOf(String& S) override;
	void Insert(INT Index, String& S) override;
	void Move(INT CurIndex, INT NewIndex) override;

	REF_DYN_CLASS(CListBoxStrings)
};
DECLARE_DYN_CLASS(CListBoxStrings, CStrings)

IMPL_DYN_CLASS(CListBoxStrings)
CListBoxStrings::CListBoxStrings():ListBox(NULL){
}

CListBoxStrings::~CListBoxStrings(){
}

void CListBoxStrings::Put(INT Index, String& S){
	INT I = ListBox->GetItemIndex();
	ULONG_PTR TempData = ListBox->InternalGetItemData(Index);
	// Set the Item to 0 in case it is an object that gets freed during Delete
	ListBox->InternalSetItemData(Index, 0);
	Delete(Index);
	InsertObject(Index, S, NULL);
	ListBox->InternalSetItemData(Index, TempData);
	ListBox->SetItemIndex(I);
}

String CListBoxStrings::Get(INT Index){
	TListBoxStyle Style = ListBox->GetStyle();
	if(Style == lbVirtual || Style == lbVirtualOwnerDraw)
		return ListBox->DoGetData(Index);
	else{
		INT Len = (INT)SendMessage(ListBox->GetHandle(), LB_GETTEXTLEN, Index, 0);
		if(Len == LB_ERR)
			throw "clist index error.";//Error(SListIndexError, Index);
		INT Size = (Len + 1) * sizeof(TCHAR);
		LPTSTR Buf = (LPTSTR)malloc(Size);
		BufferHolder bufHolder(Buf);
		ZeroMemory(Buf, Size);
		if(Len != 0){
			Len = (INT)SendMessage(ListBox->GetHandle(), LB_GETTEXT, Index, (LPARAM)Buf);
			bufHolder.SwapBuffer(NULL);
			return String::CreateFor(Buf);//caution of LB_GETTEXTLEN isn't guaranteed to be accurate
		}
		return String(NULL);
	}
}

INT CListBoxStrings::GetCount(){
	return (INT)SendMessage(ListBox->GetHandle(), LB_GETCOUNT, 0, 0);
}

CObject* CListBoxStrings::GetObject(INT Index){
	TListBoxStyle Style = ListBox->GetStyle();
	if(Style == lbVirtual || Style == lbVirtualOwnerDraw)
		return ListBox->DoGetDataObject(Index);
	else {
		CObject* Result = (CObject *)ListBox->GetItemData(Index);
		if((INT)Result == LB_ERR)
			throw "clist index error"; //Error(SListIndexError, Index);
		return Result;
	}
}

void CListBoxStrings::PutObject(INT Index, CObject* AObject){
	TListBoxStyle Style = ListBox->GetStyle();
	if(Index != -1 && !(Style == lbVirtual || Style == lbVirtualOwnerDraw))
		ListBox->SetItemData(Index, (ULONG_PTR)AObject);
}

void CListBoxStrings::SetUpdateState(BOOL Updating){
	SendMessage(ListBox->GetHandle(), WM_SETREDRAW, !Updating, 0);
	if(!Updating)
		ListBox->Refresh();
}

INT CListBoxStrings::Add(String& S){
	INT Result = -1;
	TListBoxStyle Style = ListBox->GetStyle();
	if(Style == lbVirtual || Style == lbVirtualOwnerDraw)
		return Result;
	Result = (INT)SendMessage(ListBox->GetHandle(), LB_ADDSTRING, 0, (LPARAM)S.GetBuffer());
	if(Result < 0)
		throw "out of resource for inser list line";//raise EOutOfResources.Create(SInsertLineError);
	return Result;
}

void CListBoxStrings::Clear(){
	ListBox->ResetContent();
}

void CListBoxStrings::Delete(INT Index){
	ListBox->DeleteString(Index);
}

void CListBoxStrings::Exchange(INT Index1, INT Index2){
	TListBoxStyle Style = ListBox->GetStyle();
	if(Style == lbVirtual || Style == lbVirtualOwnerDraw)
		return ;
	CMethodLock Lock(this, (TLockMethod)&CListBoxStrings::BeginUpdate, (TLockMethod)&CListBoxStrings::EndUpdate);
	String TempString = Get(Index1);
	ULONG_PTR TempData = ListBox->InternalGetItemData(Index1);
    Put(Index1, Get(Index2));
    ListBox->InternalSetItemData(Index1, ListBox->InternalGetItemData(Index2));
	Put(Index2, TempString);
    ListBox->InternalSetItemData(Index2, TempData);
	if(ListBox->GetItemIndex() == Index1)
		ListBox->SetItemIndex(Index2);
	else if(ListBox->GetItemIndex() == Index2)
		ListBox->SetItemIndex(Index1);
}

INT CListBoxStrings::IndexOf(String& S){
	TListBoxStyle Style = ListBox->GetStyle();
	if(Style == lbVirtual || Style == lbVirtualOwnerDraw)
		return ListBox->DoFindData(S);
	else
		return (INT)SendMessage(ListBox->GetHandle(), LB_FINDSTRINGEXACT, -1, (LPARAM)S.GetBuffer());

}

void CListBoxStrings::Insert(INT Index, String& S){
	TListBoxStyle Style = ListBox->GetStyle();
	if(Style == lbVirtual || Style == lbVirtualOwnerDraw)
		return ;
	if(SendMessage(ListBox->GetHandle(), LB_INSERTSTRING, Index, (LPARAM)S.GetBuffer()) < 0)
		throw "out of resource, insert line error.";//raise EOutOfResources.Create(SInsertLineError);
}

class ListMovingRestorer{
private:
	CListBox* List;
public:
	ListMovingRestorer(CListBox* List);
	virtual ~ListMovingRestorer();
};

ListMovingRestorer::ListMovingRestorer(CListBox* AList){
	List = AList;
}

ListMovingRestorer::~ListMovingRestorer(){
	if(List != NULL)
		List->SetMoving(FALSE);
}

void CListBoxStrings::Move(INT CurIndex, INT NewIndex){
	TListBoxStyle Style = ListBox->GetStyle();
	if(Style == lbVirtual || Style == lbVirtualOwnerDraw)
		return ;
	CMethodLock Lock(this, (TLockMethod)&CListBoxStrings::BeginUpdate, (TLockMethod)&CListBoxStrings::EndUpdate);
	ListBox->SetMoving(TRUE);
	ListMovingRestorer restorer(ListBox);
	if(CurIndex != NewIndex){
		String TempString = Get(CurIndex);
		ULONG_PTR TempData = ListBox->InternalGetItemData(CurIndex);
		ListBox->InternalSetItemData(CurIndex, 0);
		Delete(CurIndex);
		Insert(NewIndex, TempString);
		ListBox->InternalSetItemData(NewIndex, TempData);
	}
}

IMPL_DYN_CLASS(CListBox)
CListBox::CListBox(CComponent* AOwner) : CCustomMultiSelectListControl(AOwner),
	AutoComplete(TRUE),
	Count(0),
	Filter(NULL),
	LastTime(0),
	Columns(0),
	ItemHeight(16),
	BorderStyle(bsSingle),
	ExtendedSelect(TRUE),
	Style(lbStandard),
	IntegralHeight(FALSE),
	Sorted(FALSE),
	TabWidth(0),
	SaveItems(NULL),
	SaveTopIndex(0),
	SaveItemIndex(0),
	Moving(FALSE),
	OldCount(-1)
	{
	TControlStyle ListBoxStyle = csSetCaption | csDoubleClicks | csOpaque;
	if(GetGlobal().GetNewStyleControls())
		SetControlStyle(ListBoxStyle);
	else
		SetControlStyle(ListBoxStyle | csFramed);
	SetBounds(GetLeft(), GetTop(), 121, 97);
	SetTabStop(TRUE);
	SetParentColor(FALSE);
	Items = new CListBoxStrings();
	((CListBoxStrings *)Items)->ListBox = this;
	Canvas = new CControlCanvas();
	((CControlCanvas *)Canvas)->SetControl(this);
}

CListBox::~CListBox(){
	delete Canvas;
	delete Items;
	if(SaveItems != NULL)
		delete SaveItems;
	if(Filter != NULL)
		delete Filter;
}

void CListBox::LBGetText(TMessage& Message){
	if(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw){
		INT Cnt = (INT)Message.wParam;
		if(OnData != NULL && Cnt > -1 && Cnt < Count){
			String S;
			CALL_EVENT(Data)(this, (INT)Message.wParam, S);
			lstrcpyn((LPTSTR)Message.lParam, S.GetBuffer(), S.Length());
			Message.Result = S.Length();
		}
		else 
			Message.Result = LB_ERR;
	}
	else
		INHERITED_MSG(Message);

}

void CListBox::LBGetTextLen(TMessage& Message){
	if(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw){
		INT Cnt = (INT)Message.wParam;
		if(OnData != NULL && Cnt > -1 && Cnt < Count){
			String S;
			CALL_EVENT(Data)(this, (INT)Message.wParam, S);
			Message.Result = S.Length();
		}
		else Message.Result = LB_ERR;
	}
	else
		INHERITED_MSG(Message);
}

void CListBox::WMPaint(TWMPaint& Message){
	if(Message.DC != 0){
		//Listboxes don't allow paint "sub-classing" like the other windows controls so we have to do it ourselves.
		//Initialize drawing records
		TWMDrawItem DrawItemMsg;
		DRAWITEMSTRUCT DrawItemStruct;
		ZeroMemory(&DrawItemMsg, sizeof(DrawItemMsg));
		ZeroMemory(&DrawItemStruct, sizeof(DrawItemStruct));
		DrawItemMsg.Msg = CN_DRAWITEM;
		DrawItemMsg.DrawItemStruct = &DrawItemStruct;
		DrawItemMsg.Ctl = GetHandle();
		DrawItemStruct.CtlType = ODT_LISTBOX;
		DrawItemStruct.itemAction = ODA_DRAWENTIRE;
		DrawItemStruct.itemState = 0;
		DrawItemStruct.hDC = Message.DC;
		DrawItemStruct.CtlID = (UINT)GetHandle();
		DrawItemStruct.hwndItem = GetHandle();
		
		//Intialize measure records
		TWMMeasureItem MeasureItemMsg;
		MEASUREITEMSTRUCT MeasureItemStruct;
		ZeroMemory(&MeasureItemMsg, sizeof(MeasureItemMsg));
		ZeroMemory(&MeasureItemStruct, sizeof(MeasureItemStruct));
		MeasureItemMsg.Msg = CN_MEASUREITEM;
		MeasureItemMsg.IDCtl = GetHandle();
		MeasureItemMsg.MeasureItemStruct = &MeasureItemStruct;
		MeasureItemStruct.CtlType = ODT_LISTBOX;
		MeasureItemStruct.CtlID = (UINT)GetHandle();
		//Draw the listbox
		INT Y = 0;
		INT I = GetTopIndex();
		TRect R = {0,0,0,0};
		GetClipBox(Message.DC, (LPRECT)&R);
		INT H = GetHeight();
		INT W = GetWidth();
		while(Y < H){
			MeasureItemStruct.itemID = I;
			if(I < Items->GetCount())
				MeasureItemStruct.itemData = (ULONG_PTR)Items->GetObject(I);
			MeasureItemStruct.itemWidth = W;
			MeasureItemStruct.itemHeight = ItemHeight;
			DrawItemStruct.itemData = MeasureItemStruct.itemData;
			DrawItemStruct.itemID = I;
			Dispatch(*((PMessage)&MeasureItemMsg));
			DrawItemStruct.rcItem.left = 0;
			DrawItemStruct.rcItem.top = Y;
			DrawItemStruct.rcItem.right = MeasureItemStruct.itemWidth;
			DrawItemStruct.rcItem.bottom = Y + MeasureItemStruct.itemHeight;
			Dispatch(*((PMessage)&DrawItemMsg));
			Y += MeasureItemStruct.itemHeight;
			I++;
			if(I >= Items->GetCount())
				break;
		}
	}
	else 
		INHERITED_MSG(Message);

}

void CListBox::WMSize(TWMSize& Message){
	INHERITED_MSG(Message);
	SetColumnWidth();
}

void CListBox::CNCommand(TWMCommand& Message){
	if(Message.NotifyCode == LBN_SELCHANGE){
		__super::Changed();
		Click();
	}
	else if(Message.NotifyCode == LBN_DBLCLK)
		DblClick();
}

void CListBox::CNDrawItem(TWMDrawItem& Message){
	LPDRAWITEMSTRUCT diStruct = Message.DrawItemStruct;
	TOwnerDrawState State = (TOwnerDrawState)(diStruct->itemState & 0xffff);
	Canvas->SetHandle(diStruct->hDC);
	CanvasHandleCleaner cvasCleaner(Canvas);
    Canvas->SetFont(GetFont());
	Canvas->SetBrush(GetBrush());
    if((INT)diStruct->itemID >= 0 && IN_TEST(odSelected, State)){
		Canvas->GetBrush()->SetColor(clHighlight);
		Canvas->GetFont()->SetColor(clHighlightText);
	}
	if((INT)diStruct->itemID >= 0)
		DrawItem(diStruct->itemID, *((PRect)&(diStruct->rcItem)), State);
	else
		Canvas->FillRect(*((PRect)&(diStruct->rcItem)));
	if(IN_TEST(odFocused, State))
		DrawFocusRect(diStruct->hDC, &diStruct->rcItem);
}

void CListBox::CNMeasureItem(TWMMeasureItem& Message){
	LPMEASUREITEMSTRUCT miStruct = Message.MeasureItemStruct;
	miStruct->itemHeight = ItemHeight;
	if(Style == lbOwnerDrawVariable){
		INT i = miStruct->itemHeight;
		MeasureItem(miStruct->itemID, i);
		miStruct->itemHeight = i;
	}
}

void CListBox::WMLButtonDown(TWMLButtonDown& Message){
	TShiftState ShiftState = KeysToShiftState((WORD)Message.Keys);
	/* TODO
	if(GetDragMode() == dmAutomatic && MultiSelect){
		if(!IN_TEST(ssShift, ShiftState) || IN_TEST(ssCtrl, ShiftState)){
			INT ItemNo = ItemAtPos(SmallPointToPoint(Message.Pos), TRUE);
			if(ItemNo >= 0 && GetSelected(ItemNo)){
				BeginDrag (FALSE);
				return;
			}
		}
	}//*/
	INHERITED_MSG(Message);
	/* TODO
	if(GetDragMode() == dmAutomatic && !(MultiSelect &&
		(IN_TEST(ssCtrl, ShiftState) || IN_TEST(ssShift, ShiftState))))
		BeginDrag(FALSE);
		//*/

}

void CListBox::CMCtl3DChanged(TMessage& Message){
	if(GetGlobal().GetNewStyleControls() && BorderStyle == bsSingle)
		RecreateWnd();
	INHERITED_MSG(Message);
}

void CListBox::SetColumnWidth(){

}

void CListBox::CreateParams(TCreateParams& Params){
	typedef DWORD TSelects[2];
	typedef TSelects *PSelects;
	DWORD Styles[] = {0, LBS_OWNERDRAWFIXED, LBS_OWNERDRAWVARIABLE, LBS_OWNERDRAWFIXED, LBS_OWNERDRAWFIXED};
	DWORD Sorteds[] = {0, LBS_SORT};
	DWORD MultiSelects[] = {0, LBS_MULTIPLESEL};
	DWORD ExtendSelects[] = {0, LBS_EXTENDEDSEL};
	DWORD IntegralHeights[] = {LBS_NOINTEGRALHEIGHT, 0};
	DWORD MultiColumns[] = {0, LBS_MULTICOLUMN};
	DWORD TabStops[] = {0, LBS_USETABSTOPS};
	DWORD CSHREDRAW[] = {CS_HREDRAW, 0};
	DWORD Data[] = {LBS_HASSTRINGS, LBS_NODATA};
	DWORD BorderStyles[] = {0, WS_BORDER};
	PSelects Selects = NULL;
	__super::CreateParams(Params);
	//*
	CreateSubClass(Params, TEXT("LISTBOX"));
	Selects = &MultiSelects;
	if(ExtendedSelect)
		Selects = &ExtendSelects;
	Params.Style |= WS_HSCROLL | WS_VSCROLL |
		Data[(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw)] |
		LBS_NOTIFY | Styles[Style] | Sorteds[Sorted] |
		(*Selects)[MultiSelect] | IntegralHeights[IntegralHeight] |
		MultiColumns[Columns != 0] || BorderStyles[BorderStyle] |
		TabStops[TabWidth != 0];
	
	if(GetGlobal().GetNewStyleControls() && GetCtl3D() && BorderStyle == bsSingle){
		Params.Style &= ~WS_BORDER;
		Params.ExStyle |= WS_EX_CLIENTEDGE;
	}
	Params.WinClass.style &= ~(CSHREDRAW[UseRightToLeftAlignment()] | CS_VREDRAW);//*/
}

void CListBox::CreateWnd(){
	INT W = GetWidth();
	INT H = GetHeight();
	__super::CreateWnd();
	SetWindowPos(GetHandle(), 0, GetLeft(), GetTop(), W, H, SWP_NOZORDER | SWP_NOACTIVATE);
	if(TabWidth != 0)
		SendMessage(GetHandle(), LB_SETTABSTOPS, 1, (LPARAM)&TabWidth);
	SetColumnWidth();
	if(OldCount != -1 || SaveItems != NULL){
		if(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw)
			SetCount(OldCount);
		if(SaveItems != NULL){
			Items->Assign(*SaveItems);
			FreeAndNil((CObject **)&SaveItems);
		}
		SetTopIndex(SaveTopIndex);
		SetItemIndex(SaveItemIndex);
		OldCount = -1;
	}
}

void CListBox::DestroyWnd(){
	if(Items->GetCount() > 0){
		if(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw)
			OldCount = Items->GetCount();
		else{
			SaveItems = new CStringList();
			SaveItems->Assign(*Items);
		}
		SaveTopIndex = GetTopIndex();
		SaveItemIndex = GetItemIndex();
	}
	__super::DestroyWnd();
}

String CListBox::DoGetData(INT Index){
	String Result;
	if(OnData != NULL)
		CALL_EVENT(Data)(this, Index, Result);
	return Result;
}

CObject* CListBox::DoGetDataObject(INT Index){
	CObject* Result = NULL;
	if(OnDataObject != NULL)
		CALL_EVENT(DataObject)(this, Index, &Result);
	return Result;
}

INT CListBox::DoFindData(String& Data){
	if(OnDataFind != NULL)
		return CALL_EVENT(DataFind)(this, Data);
	return -1;
}

void CListBox::WndProc(TMessage& Message){
	//for auto drag mode, let listbox handle itself, instead of TControl
	/* TODO
	if(!IN_TEST(csDesigning, GetComponentState()) && (Message.Msg == WM_LBUTTONDOWN ||
		Message.Msg == WM_LBUTTONDBLCLK) && !GetDragging()){
		if(GetDragMode() == dmAutomatic){
			if(IsControlMouseMsg(*((PWMMouse)&Message)))
				return ;
			SetControlState(GetControlState() | csLButtonDown);
			Dispatch(Message); //overrides TControl's BeginDrag
			return ;
		}
	}//*/
	__super::WndProc(Message);
}

//void CListBox::DragCanceled(){}

void CListBox::DrawItem(INT Index, TRect& Rect, TOwnerDrawState State){
	if(OnDrawItem != NULL)
		CALL_EVENT(DrawItem)(this, Index, Rect, State);
	else{
		Canvas->FillRect(Rect);
		if(Index < GetCount()){
			DWORD Flags = DrawTextBiDiModeFlags(DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
			if(UseRightToLeftAlignment())
				Rect.left += 2;
			else
				Rect.right -= 2;
			String Data;
			if(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw)
				Data = DoGetData(Index);
			else
				Data = Items->Get(Index);
			DrawText(Canvas->GetHandle(), Data.GetBuffer(), Data.Length(), (LPRECT)&Rect, Flags);
		}
	}
}

INT CListBox::GetSelCount(){
	return (INT)SendMessage(GetHandle(), LB_GETSELCOUNT, 0, 0);
}

void CListBox::MeasureItem(INT Index, INT& Height){
	if(OnMeasureItem != NULL)
		CALL_EVENT(MeasureItem)(this, Index, Height);
}

ULONG_PTR CListBox::InternalGetItemData(INT Index){
	return GetItemData(Index);
}

void CListBox::InternalSetItemData(INT Index, ULONG_PTR AData){
	SetItemData(Index, AData);
}

ULONG_PTR CListBox::GetItemData(INT Index){
	return SendMessage(GetHandle(), LB_GETITEMDATA, Index, 0);
}

INT CListBox::GetItemIndex(){
	if(GetMultiSelect())
		return (INT)SendMessage(GetHandle(), LB_GETCARETINDEX, 0, 0);
	else
		return (INT)SendMessage(GetHandle(), LB_GETCURSEL, 0, 0);
}

void CListBox::KeyPress(TCHAR& Key){
	__super::KeyPress(Key);
	if(!AutoComplete)
		return ;
	if(GetTickCount() - LastTime >= 500){
		if(Filter != NULL)
			delete Filter;
		Filter = new String();
	}
	LastTime = GetTickCount();
	if(Key != VK_BACK){
		/* TODO
		if(Key in LeadBytes){
			MSG Msg;
			if(PeekMessage(&Msg, GetHandle(), WM_CHAR, WM_CHAR, PM_REMOVE)){
				Filter = Filter + Key + (TCHAR)Msg.wParam;
				Key = TCHAR('\0');
			}
		}
		else //*/
			*Filter = *Filter + Key;
	}
	else{
		/* TODO
		while(ByteType(Filter->GetBuffer(), Filter->Length()) == mbTrailByte)
			Filter->Delete(Filter->Length() - 1, 1);
		Filter->Delete(Filter->Length() - 1, 1);
		//*/
	}
	if(Filter->Length() > 0){
		INT Idx = 0;
		if(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw)
			Idx = DoFindData(*Filter);
		else
			Idx = (INT)SendMessage(GetHandle(), LB_FINDSTRING, -1, (LPARAM)Filter->GetBuffer());
		if(Idx != LB_ERR){
			if(GetMultiSelect()){
				ClearSelection();
				SendMessage(GetHandle(), LB_SELITEMRANGE, 1, MAKELPARAM(Idx, Idx));
			}
			SetItemIndex(Idx);
			Click();
		}
		if(!(Key == VK_RETURN || Key == VK_BACK || Key == VK_ESCAPE))
			Key = TCHAR('\0');  // Clear so that the listbox's default search mechanism is disabled
	}
	else{
		SetItemIndex(0);
		Click();
	}
}

void CListBox::SetItemData(INT Index, ULONG_PTR AData){
	SendMessage(GetHandle(), LB_SETITEMDATA, Index, AData);
}

void CListBox::ResetContent(){
	if(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw)
		return ;
	SendMessage(GetHandle(), LB_RESETCONTENT, 0, 0);
}

void CListBox::DeleteString(INT Index){
	SendMessage(GetHandle(), LB_DELETESTRING, Index, 0);
}

void CListBox::SetMultiSelect(BOOL Value){
	if(MultiSelect != Value){
		MultiSelect = Value;
		RecreateWnd();
	}
}

void CListBox::SetItemIndex(INT Value){
	if(GetItemIndex() != Value)
		if(GetMultiSelect())
			SendMessage(GetHandle(), LB_SETCARETINDEX, Value, 0);
		else 
			SendMessage(GetHandle(), LB_SETCURSEL, Value, 0);
}

void CListBox::AddItem(LPTSTR Item, CObject* AObject){
	String S(Item);
	Items->AddObject(S, AObject);
}

void CListBox::Clear(){
	Items->Clear();
}

void CListBox::ClearSelection(){
	if(GetMultiSelect()){
		INT Count = Items->GetCount();
		for(INT I = 0; I < Count; I++)
			SetSelected(I, FALSE);
	}
	else
		SetItemIndex(-1);
}

void CListBox::CopySelection(CCustomListControl* Destination){
	if(GetMultiSelect()){
		INT Count = Items->GetCount();
		for(INT I = 0; I < Count; I++)
			if(GetSelected(I))
				Destination->AddItem(Items->Get(I).GetBuffer(), Items->GetObject(I));
	}
	else{
		INT Index = GetItemIndex();
		if(Index != -1)
			Destination->AddItem(Items->Get(Index).GetBuffer(), Items->GetObject(Index));
	}
}

void CListBox::DeleteSelected(){
	if(GetMultiSelect()){
		for(INT I = Items->GetCount() - 1; I >= 0; I--)
			if(GetSelected(I))
				Items->Delete(I);
	}
	else{
		INT Index = GetItemIndex();
		if(Index != -1)
			Items->Delete(Index);
	}
}

INT CListBox::ItemAtPos(TPoint& Pos, BOOL Existing){
	INT Result = 0;
	if(PtInRect((const RECT *)&GetClientRect(), Pos)){
		Result = GetTopIndex();
		TRect ItemRect = {0,0,0,0};
		INT Count = Items->GetCount();
		while(Result < Count){
			Perform(LB_GETITEMRECT, Result, (LPARAM)&ItemRect);
			if(PtInRect((const RECT *)&ItemRect, Pos))
				return Result;
			Result++;
		}
		if(!Existing)
			return Result;
	}
	return -1;
}

TRect CListBox::ItemRect(INT Index){
	TRect Result = {0,0,0,0};
	INT Count = Items->GetCount();
	if(Index == 0 || Index < Count)
		Perform(LB_GETITEMRECT, Index, (LPARAM)&Result);
	else if(Index == Count){
		Perform(LB_GETITEMRECT, Index - 1, (LPARAM)&Result);
		OffsetRect((LPRECT)&Result, 0, Result.bottom - Result.top);
	}
	return Result;
}

void CListBox::SelectAll(){
	if(MultiSelect){
		INT Count = Items->GetCount();
		for(INT I = 0; I < Count; I++)
			SetSelected(I, TRUE);
	}
}

INT CListBox::GetCount(){
	if(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw)
		return Count;
	else
		return Items->GetCount();
}

void CListBox::SetCount(INT Value){
	if(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw){
		// Limited to 32767 on Win95/98 as per Win32 SDK
		LRESULT Error = SendMessage(GetHandle(), LB_SETCOUNT, Value, 0);
		if(Error != LB_ERR && Error != LB_ERRSPACE)
			Count = Value;
		else
			throw "error setting count.";//raise Exception.CreateFmt(SErrorSettingCount, [Name]);
	}
	else
		throw "list box must be virtual.";//raise Exception.CreateFmt(SListBoxMustBeVirtual, [Name]);

}

void CListBox::SetItems(CStrings* Value){
	if(GetStyle() == lbVirtual)
		SetStyle(lbStandard);
	else if(GetStyle() == lbVirtualOwnerDraw)
		SetStyle(lbOwnerDrawFixed);
	Items->Assign(*Value);
}

BOOL CListBox::GetSelected(INT Index){
	LRESULT R = SendMessage(GetHandle(), LB_GETSEL, Index, 0);
	if(R == LB_ERR)
		throw "list index error.";//raise EListError.CreateResFmt(@SListIndexError, [Index]);
	return (BOOL)R;
}

void CListBox::SetSelected(INT Index, BOOL Value){
	if(MultiSelect){
		if(SendMessage(GetHandle(), LB_SETSEL, (WPARAM)Value, Index) == LB_ERR)
			throw "list index error.";//raise EListError.CreateResFmt(@SListIndexError, [Index]);
	}
	else
		if(Value){
			if(SendMessage(GetHandle(), LB_SETCURSEL, Index, 0) == LB_ERR)
				throw "list index error.";//raise EListError.CreateResFmt(@SListIndexError, [Index])
		}
		else
			SendMessage(GetHandle(), LB_SETCURSEL, -1, 0);
}

INT CListBox::GetScrollWidth(){
	return (INT)SendMessage(GetHandle(), LB_GETHORIZONTALEXTENT, 0, 0);
}

void CListBox::SetScrollWidth(INT Value){
	if(Value != GetScrollWidth())
		SendMessage(GetHandle(), LB_SETHORIZONTALEXTENT, Value, 0);
}

INT CListBox::GetTopIndex(){
	return (INT)SendMessage(GetHandle(), LB_GETTOPINDEX, 0, 0);
}

void CListBox::SetTopIndex(INT Value){
	if(GetTopIndex() != Value)
		SendMessage(GetHandle(), LB_SETTOPINDEX, Value, 0);
}

void CListBox::SetBorderStyle(TBorderStyle Value){
	if(BorderStyle != Value){
		BorderStyle = Value;
		RecreateWnd();
	}
}

void CListBox::SetColumns(INT Value){
	if(Columns != Value)
		if(Columns == 0 || Value == 0){
			Columns = Value;
			RecreateWnd();
		}
		else{
			Columns = Value;
			if(HandleAllocated())
				SetColumnWidth();
		}
}

void CListBox::SetExtendedSelect(BOOL Value){
	if(Value != ExtendedSelect){
		ExtendedSelect = Value;
		RecreateWnd();
	}
}

void CListBox::SetIntegralHeight(BOOL Value){
	if(Value != IntegralHeight){
		IntegralHeight = Value;
		RecreateWnd();
		RequestAlign();
	}
}

INT CListBox::GetItemHeight(){
	INT Result = ItemHeight;
	if(HandleAllocated() && Style == lbStandard){
		TRect R = {0, 0, 0, 0};
		Perform(LB_GETITEMRECT, 0, (LPARAM)&R);
		Result = R.bottom - R.top;
	}
	return Result;
}

void CListBox::SetItemHeight(INT Value){
	if(ItemHeight != Value && Value > 0){
		ItemHeight = Value;
		RecreateWnd();
	}
}

void CListBox::SetSorted(BOOL Value){
	if(GetStyle() == lbVirtual || GetStyle() == lbVirtualOwnerDraw)
		return;
	if(Sorted != Value){
		Sorted = Value;
		RecreateWnd();
	}
}

void CListBox::SetStyle(TListBoxStyle Value){
	if(Style != Value){
		if(Value == lbVirtual || Value == lbVirtualOwnerDraw){
			Items->Clear();
			SetSorted(FALSE);
		}
		Style = Value;
		RecreateWnd();
	}
}

void CListBox::SetTabWidth(INT Value){
	if(Value < 0)
		Value = 0;
	if(TabWidth != Value){
		TabWidth = Value;
		RecreateWnd();
	}
}

