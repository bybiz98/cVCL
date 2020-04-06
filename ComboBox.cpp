
#include "stdinc.h"
#include "ObjWndProc.hpp"
#include "ComboBox.hpp"
#include "WinUtils.hpp"
#include "SysInit.hpp"
#include "Form.hpp"

IMPL_DYN_CLASS(CCustomComboBoxStrings)
CCustomComboBoxStrings::CCustomComboBoxStrings():ComboBox(NULL){
}

CCustomComboBoxStrings::~CCustomComboBoxStrings(){

}

INT CCustomComboBoxStrings::GetCount(){
	return (INT)SendMessage(ComboBox->GetHandle(), CB_GETCOUNT, 0, 0);
}

String CCustomComboBoxStrings::Get(INT Index){
	String Result;
	INT Len = (INT)SendMessage(ComboBox->GetHandle(), CB_GETLBTEXTLEN, Index, 0);
	if(Len != CB_ERR){
		INT Size = (Len + 1) * sizeof(TCHAR);
		LPTSTR Buf = (LPTSTR)malloc(Size);
		ZeroMemory(Buf, Size);
		SendMessage(ComboBox->GetHandle(), CB_GETLBTEXT, Index, (LPARAM)Buf);
		return String::CreateFor(Buf);
	}
	return String(NULL);
}

CObject* CCustomComboBoxStrings::GetObject(INT Index){
	CObject* Result = (CObject*)SendMessage(ComboBox->GetHandle(), CB_GETITEMDATA, Index, 0);
	if((ULONG_PTR)Result == CB_ERR)
		throw "list index error.";//Error(SListIndexError, Index);
	return Result;
}

void CCustomComboBoxStrings::PutObject(INT Index, CObject* AObject){
	SendMessage(ComboBox->GetHandle(), CB_SETITEMDATA, Index, (LPARAM)AObject);
}

void CCustomComboBoxStrings::SetUpdateState(BOOL Updating){
	SendMessage(ComboBox->GetHandle(), WM_SETREDRAW, !Updating, 0);
	if(!Updating)
		ComboBox->Refresh();
}

void CCustomComboBoxStrings::Clear(){
	String S = ComboBox->GetTextString();
	SendMessage(ComboBox->GetHandle(), CB_RESETCONTENT, 0, 0);
	ComboBox->SetText(S.GetBuffer());
	ComboBox->Update();
}

void CCustomComboBoxStrings::Delete(INT Index){
	SendMessage(ComboBox->GetHandle(), CB_DELETESTRING, Index, 0);
}

INT CCustomComboBoxStrings::IndexOf(String& S){
	return (INT)SendMessage(ComboBox->GetHandle(), CB_FINDSTRINGEXACT, -1, (LPARAM)S.GetBuffer());
}


class CComboBoxStrings : public CCustomComboBoxStrings{
public:
	CComboBoxStrings();
	virtual ~CComboBoxStrings();
	INT Add(String& S) override;
	void Insert(INT Index, String& S) override;

	REF_DYN_CLASS(CComboBoxStrings)
};
DECLARE_DYN_CLASS(CComboBoxStrings, CCustomComboBoxStrings)

IMPL_DYN_CLASS(CComboBoxStrings)
CComboBoxStrings::CComboBoxStrings(){
}

CComboBoxStrings::~CComboBoxStrings(){
}

INT CComboBoxStrings::Add(String& S){
	INT Result = (INT)SendMessage(GetComboBox()->GetHandle(), CB_ADDSTRING, 0, (LPARAM)S.GetBuffer());
	if(Result < 0)
		throw "out of resource.";//raise EOutOfResources.Create(SInsertLineError);
	return Result;
}

void CComboBoxStrings::Insert(INT Index, String& S){
	if(SendMessage(GetComboBox()->GetHandle(), CB_INSERTSTRING, Index, (LPARAM)S.GetBuffer()) < 0)
		throw "out of resource.";//raise EOutOfResources.Create(SInsertLineError);
}

IMPL_DYN_CLASS(CCustomCombo)
CCustomCombo::CCustomCombo(CComponent* AOwner):CCustomListControl(AOwner),
	ItemHeight(16),
	DropDownCount(8),
	ItemIndex(-1),
	SaveIndex(-1),
	MaxLength(0),
	Items(NULL),
	EditHandle(0),
	ListHandle(0),
	DropHandle(0),
	DefEditProc(NULL),
	DefListProc(NULL),
	DroppingDown(FALSE),
	FocusChanged(FALSE),
	IsFocused(FALSE),
	INIT_EVENT(Change),
	INIT_EVENT(Select),
	INIT_EVENT(DropDown),
	INIT_EVENT(CloseUp)
	{
	DWORD ComboBoxStyle = csCaptureMouse | csSetCaption | csDoubleClicks |
		csFixedHeight | csReflector;
	if(GetGlobal().GetNewStyleControls())
		SetControlStyle(ComboBoxStyle);
	else
		SetControlStyle(ComboBoxStyle | csFramed);
	SetBounds(GetLeft(), GetTop(), 145, 25);
	SetTabStop(TRUE);
	SetParentColor(FALSE);
	Canvas = new CControlCanvas();
	((CControlCanvas *)Canvas)->SetControl(this);
	EditInstance = MakeObjectWndProc(this, (TObjectWndProc)&CCustomCombo::EditWndProc);
	ListInstance = MakeObjectWndProc(this, (TObjectWndProc)&CCustomCombo::ListWndProc);
}

CCustomCombo::~CCustomCombo(){
	if(HandleAllocated())
		DestroyWindowHandle();
	FreeObjectWndProc(ListInstance);
	FreeObjectWndProc(EditInstance);
	delete Canvas;
}

void CCustomCombo::WMCreate(TWMCreate& Message){
	INHERITED_MSG(Message);
	if(GetText() != NULL)
		SetWindowText(GetHandle(), GetText());
}

void CCustomCombo::CMCancelMode(TCMCancelMode& Message){
	if(Message.Sender != this)
		Perform(CB_SHOWDROPDOWN, 0, 0);
}

void CCustomCombo::CMCtl3DChanged(TMessage& Message){
	if(GetGlobal().GetNewStyleControls())
		RecreateWnd();
	INHERITED_MSG(Message);
}

void CCustomCombo::CNCommand(TWMCommand& Message){
	switch(Message.NotifyCode){
	case CBN_DBLCLK:
		DblClick();
		break;
	case CBN_EDITCHANGE:
		Change();
		break;
	case CBN_DROPDOWN:
        FocusChanged = FALSE;
		DropDown();
        AdjustDropDown();
        if(FocusChanged){
			PostMessage(GetHandle(), WM_CANCELMODE, 0, 0);
			if(!IsFocused)
				PostMessage(GetHandle(), CB_SHOWDROPDOWN, 0, 0);
		}
		break;
	case CBN_SELCHANGE:
		SetText(Items->Get(ItemIndex).GetBuffer());
		Click();
        Select();
		break;
	case CBN_CLOSEUP:
		CloseUp();
		break;
	case CBN_SETFOCUS:
		IsFocused = TRUE;
		FocusChanged = TRUE;
		//TODO SetIme();
		break;
	case CBN_KILLFOCUS:
		IsFocused = FALSE;
		FocusChanged = TRUE;
		//TODO ResetIme();
		break;
	}
}

void CCustomCombo::WMDrawItem(TWMDrawItem& Message){
	DefaultHandler(*((PMessage)&Message));
}

void CCustomCombo::WMMeasureItem(TWMMeasureItem& Message){
	DefaultHandler(*((PMessage)&Message));
}

void CCustomCombo::WMDeleteItem(TWMDeleteItem& Message){
	DefaultHandler(*((PMessage)&Message));
}

void CCustomCombo::WMGetDlgCode(TWMGetDlgCode& Message){
	INHERITED_MSG(Message);
	if(GetDroppedDown())
		Message.Result = Message.Result | DLGC_WANTALLKEYS;
}

void CCustomCombo::AdjustDropDown(){
	INT Count = GetItemCount();
	if(Count > GetDropDownCount())
		Count = GetDropDownCount();
	if(Count < 1)
		Count = 1;
	DroppingDown = TRUE;
	__try{
		SetWindowPos(DropHandle, 0, 0, 0, GetWidth(), GetItemHeight() * Count + GetHeight() + 2, 
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_HIDEWINDOW);
	}
	__finally{
		DroppingDown = FALSE;
	}
	SetWindowPos(DropHandle, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_SHOWWINDOW);

}
int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep){
	PEXCEPTION_RECORD SavedExceptRec = (ep)->ExceptionRecord;
	PCONTEXT SavedContext = (ep)->ContextRecord;
	//EXCEPTION_ACCESS_VIOLATION
	char Buf[255] = {0};
	_itoa_s(SavedExceptRec->ExceptionCode, Buf, 16);
	MessageBoxA(0, Buf, Buf, 0);
	return EXCEPTION_CONTINUE_SEARCH;//EXCEPTION_EXECUTE_HANDLER;
}
void CCustomCombo::ComboWndProc(TMessage& Message, HWND ComboWnd, LPVOID ComboProc){
	__try{
		switch(Message.Msg){
			case WM_SETFOCUS:{
				CForm* Form = GetParentForm(this);
				if(Form != NULL && !Form->SetFocusedControl(this))
					return;
				break;
			}
			case WM_KILLFOCUS:
				if(IN_TEST(csFocusing, GetControlState()))
					return;
				break;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if(ComboWnd != ListHandle && DoKeyDown(*((PWMKey)&Message)))
					return;
				break;
			case WM_CHAR:{
				PWMKey KeyMsg = (PWMKey)&Message;
				if(DoKeyPress(*KeyMsg))
					return ;
				if((KeyMsg->CharCode == VK_RETURN || KeyMsg->CharCode == VK_ESCAPE) && GetDroppedDown()){
					SetDroppedDown(FALSE);
					return;
				}
				break;
			}
			case WM_KEYUP:
			case WM_SYSKEYUP:
				if(DoKeyUp(*(PWMKey)&Message))
					return ;
				break;
			case WM_MOUSEMOVE:
				//TODO Application.HintMouseMessage(this, Message);
			case WM_RBUTTONUP:
				if(FALSE){//TODO HasPopup(this)){
					PWMRButtonUp BtnupMsg = (PWMRButtonUp)&Message;
					TPoint Point = {BtnupMsg->Pos.x, BtnupMsg->Pos.y};
					MapWindowPoints(ComboWnd, GetHandle(), (LPPOINT)&Point, 1);
					BtnupMsg->Pos.x = (SHORT)Point.x;
					BtnupMsg->Pos.y = (SHORT)Point.y;
					WndProc(Message);
					return;
				}
				break;
			case WM_GETDLGCODE:
				if(GetDroppedDown()){
					Message.Result = DLGC_WANTALLKEYS;
					return ;
				}
				break;
			case WM_NCHITTEST:
				if(IN_TEST(csDesigning, GetComponentState())){
					Message.Result = HTTRANSPARENT;
					return;
				}
				break;
			case CN_KEYDOWN:
			case CN_CHAR:
			case CN_SYSKEYDOWN:
			case CN_SYSCHAR:
				WndProc(Message);
				return;
		}
		Message.Result = CallWindowProc((WNDPROC)ComboProc, ComboWnd, Message.Msg, Message.wParam, Message.lParam);
		if(Message.Msg == WM_LBUTTONDBLCLK && IN_TEST(csDoubleClicks, GetControlStyle()))
			DblClick();
	}
	//__except(EXCEPTION_EXECUTE_HANDLER){
	__except(filter(GetExceptionCode(), GetExceptionInformation())){
		GetGlobal().CallHandleException(this);
	}
}

void CCustomCombo::CreateWnd(){
	__super::CreateWnd();
	SendMessage(GetHandle(), CB_LIMITTEXT, MaxLength, 0);
	EditHandle = 0;
	ListHandle = 0;
}

LRESULT CCustomCombo::EditWndProc(UINT message, WPARAM wParam, LPARAM lParam){
	TMessage Message;
	Message.Msg = message;
	Message.wParam = wParam;
	Message.lParam = lParam;
	Message.Result = 0;
	if(Message.Msg == WM_SYSCOMMAND){
		WndProc(Message);
		return Message.Result;
	}
	else if(Message.Msg >= WM_KEYFIRST && Message.Msg <= WM_KEYLAST){
		CForm* Form = GetParentForm(this);
		if(Form != NULL && Form->WantChildKey(this, Message))
			return Message.Result;
	}
	ComboWndProc(Message, EditHandle, DefEditProc);
	if(Message.Msg == WM_LBUTTONDOWN || Message.Msg == WM_LBUTTONDBLCLK){
		/*TODO
		if(GetDragMode() == dmAutomatic){
			TPoint P;
			GetCursorPos((LPPOINT)&P);
			P = ScreenToClient(P);
			SendMessage(EditHandle, WM_LBUTTONUP, 0,(LPARAM)PointToSmallPoint(P));
			BeginDrag(FALSE);
		}//*/
	}
	else if(Message.Msg == WM_SETFONT){
		if(GetGlobal().GetNewStyleControls())
			SendMessage(EditHandle, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, 0);
	}
	return Message.Result;
}

void CCustomCombo::WndProc(TMessage& Message){
	//for auto drag mode, let listbox handle itself, instead of TControl
	/*TODO
	if(!IN_TEST(csDesigning, GetComponentState()) && 
		((Message.Msg == WM_LBUTTONDOWN || Message.Msg == WM_LBUTTONDBLCLK)) && !Dragging()){
		if(GetDragMode() == dmAutomatic){
			if(IsControlMouseMsg(*((PWMMouse)&Message)))
				return ;
			SetControlState(GetControlState() | csLButtonDown);
			Dispatch(Message);//overrides TControl's BeginDrag
			return;
		}
	}//*/
	if(Message.Msg == WM_SIZE){
		//Prevent TWinControl from handling WM_SIZE when adjusting drop-down listbox size.
		if(DroppingDown){
			DefaultHandler(Message);
			return;
		}
	}
	else if(Message.Msg >= WM_CTLCOLORMSGBOX && Message.Msg <= WM_CTLCOLORSTATIC){
		SetTextColor((HDC)Message.wParam, ColorToRGB(GetFont()->GetColor()));
		SetBkColor((HDC)Message.wParam, ColorToRGB(GetBrush()->GetColor()));
		Message.Result = (LRESULT)GetBrush()->GetHandle();
		return;
	}
	else if(Message.Msg == WM_CHAR){
		PWMKey KeyMsg = (PWMKey)&Message;
		if(DoKeyPress(*KeyMsg))
			return;
		if((KeyMsg->CharCode = VK_RETURN || KeyMsg->CharCode == VK_ESCAPE) && GetDroppedDown()){
			SetDroppedDown(FALSE);
			return;
		}
	}
	__super::WndProc(Message);
}

void CCustomCombo::SetItemHeight(INT Value){
	if(Value > 0){
		ItemHeight = Value;
		RecreateWnd();
	}
}

INT CCustomCombo::GetCount(){
	return GetItemCount();
}

INT CCustomCombo::GetItemIndex(){
	if(IN_TEST(csLoading, GetComponentState()))
		return ItemIndex;
	else
		return (INT)SendMessage(GetHandle(), CB_GETCURSEL, 0, 0);
}

BOOL CCustomCombo::GetDroppedDown(){
	return (BOOL)SendMessage(GetHandle(), CB_GETDROPPEDSTATE, 0, 0);
}

INT CCustomCombo::GetSelLength(){
	DWORD StartPos = 0;
	DWORD EndPos = 0;
	SendMessage(GetHandle(), CB_GETEDITSEL, (WPARAM)StartPos, (LPARAM)&EndPos);
	return EndPos - StartPos;
}

INT CCustomCombo::GetSelStart(){
	DWORD StartPos = 0;
	SendMessage(GetHandle(), CB_GETEDITSEL, (WPARAM)&StartPos, NULL);
	return (INT)StartPos;
}

LRESULT CCustomCombo::ListWndProc(UINT message, WPARAM wParam, LPARAM lParam){
	TMessage Message;
	Message.Msg = message;
	Message.wParam = wParam;
	Message.lParam = lParam;
	Message.Result = 0;
	ComboWndProc(Message, ListHandle, DefListProc);
	return Message.Result;
}

void CCustomCombo::Loaded(){
	__super::Loaded();
	if(ItemIndex != -1)
		SetItemIndex(ItemIndex);
}

void CCustomCombo::Change(){
	__super::Changed();
	if(OnChange != NULL)
		CALL_EVENT(Change)(this);
}

void CCustomCombo::Select(){
	if(OnSelect != NULL)
		CALL_EVENT(Select)(this);
	else
		Change();
}

void CCustomCombo::DropDown(){
	if(OnDropDown != NULL)
		CALL_EVENT(DropDown)(this);
}

void CCustomCombo::CloseUp(){
	if(OnCloseUp != NULL)
		CALL_EVENT(CloseUp)(this);
}

void CCustomCombo::DestroyWindowHandle(){
	__super::DestroyWindowHandle();
	//must be cleared after the main handle is destroyed as messages are sent
    //to our internal WndProcs when the main handle is destroyed and we should not
    //have NULL handles when we receive those messages.
	EditHandle = 0;
	ListHandle = 0;
	DropHandle = 0;
}

void CCustomCombo::SetDroppedDown(BOOL Value){
	SendMessage(GetHandle(), CB_SHOWDROPDOWN, Value, 0);
	TRect R = GetClientRect();
	InvalidateRect(GetHandle(), (const RECT *)&R, TRUE);
}

void CCustomCombo::SetSelLength(INT Value){
	DWORD StartPos = 0;
	DWORD EndPos = 0;
	SendMessage(GetHandle(), CB_GETEDITSEL, (WPARAM)&StartPos, (LPARAM)&EndPos);
	EndPos = StartPos + Value;
	SendMessage(GetHandle(), CB_SETEDITSEL, 0, MAKELPARAM(StartPos, EndPos));
}

void CCustomCombo::SetSelStart(INT Value){
	SendMessage(GetHandle(), CB_SETEDITSEL, 0, MAKELPARAM(Value, Value));
}

void CCustomCombo::SetMaxLength(INT Value){
	if(Value < 0)
		Value = 0;
	if(MaxLength != Value){
		MaxLength = Value;
		if(HandleAllocated())
			SendMessage(GetHandle(), CB_LIMITTEXT, Value, 0);
	}
}

void CCustomCombo::SetDropDownCount(INT Value){
	DropDownCount = Value;
}

void CCustomCombo::SetItemIndex(INT Value){
	if(IN_TEST(csLoading, GetComponentState()))
		ItemIndex = Value;
	else
		if(GetItemIndex() != Value)
			SendMessage(GetHandle(), CB_SETCURSEL, Value, 0);
}

void CCustomCombo::SetItems(CStrings* Value){
	if(Items != NULL)
		Items->Assign(*Value);
	else
		Items = Value;
}

void CCustomCombo::AddItem(LPTSTR Item, CObject* AObject){
	Items->AddObject(String(Item), AObject);
}

void CCustomCombo::Clear(){
	SetTextBuf(TEXT(""));
	Items->Clear();
	SaveIndex = -1;
}

void CCustomCombo::ClearSelection(){
	SetItemIndex(-1);
}

void CCustomCombo::CopySelection(CCustomListControl* Destination){
	if(ItemIndex != -1)
		Destination->AddItem(Items->Get(ItemIndex).GetBuffer(), Items->GetObject(ItemIndex));
}

void CCustomCombo::DeleteSelected(){
	if(GetItemIndex() != -1)
		Items->Delete(GetItemIndex());
}

BOOL CCustomCombo::Focused(){
	if(HandleAllocated()){
		HWND FocusedWnd = GetFocus();
		return FocusedWnd == EditHandle || FocusedWnd == ListHandle;
	}
	return FALSE;
}

void CCustomCombo::SelectAll(){
	SendMessage(GetHandle(), CB_SETEDITSEL, 0, 0xFFFF0000);
}

IMPL_DYN_CLASS(CComboBox)
CComboBox::CComboBox(CComponent* AOwner):CCustomCombo(AOwner),
	Style(csDropDown),
	LastTime(0),
	AutoComplete(TRUE),
	AutoCloseUp(FALSE),
	AutoDropDown(FALSE),
	CharCase(ecNormal),
	Sorted(FALSE),
	SaveItems(NULL),
	INIT_EVENT(DrawItem),
	INIT_EVENT(MeasureItem){
	Items = (CCustomComboBoxStrings *)GetItemsClass()->NewInstance();
	((CCustomComboBoxStrings *)Items)->SetComboBox(this);
	ItemHeight = 16;
	Filter = new String(NULL);
}

CComboBox::~CComboBox(){
	delete Items;
	if(SaveItems != NULL)
		delete SaveItems;
	if(Filter != NULL)
		delete Filter;
}

void CComboBox::WMEraseBkgnd(TWMEraseBkgnd& Message){
	if(GetStyle() == csSimple){
		FillRect(Message.DC, (const RECT *)&GetClientRect(), GetParent()->GetBrush()->GetHandle());
		Message.Result = 1;
	}
	else
		DefaultHandler(*((PMessage)&Message));
}

void CComboBox::CMParentColorChanged(TMessage& Message){
	INHERITED_MSG(Message);
	if(GetGlobal().GetNewStyleControls() && GetStyle() < csDropDownList)
		Invalidate();
}

void CComboBox::CNDrawItem(TWMDrawItem& Message){
	TOwnerDrawState State = Message.DrawItemStruct->itemState & 0xffff;
	if((Message.DrawItemStruct->itemState & ODS_COMBOBOXEDIT) != 0)
		State |= odComboBoxEdit;
    if((Message.DrawItemStruct->itemState & ODS_DEFAULT) != 0)
		State |= odDefault;
    GetCanvas()->SetHandle(Message.DrawItemStruct->hDC);
	CanvasHandleCleaner cavsCleaner(GetCanvas());
    GetCanvas()->SetFont(GetFont());
	GetCanvas()->SetBrush(GetBrush());
    if((INT)Message.DrawItemStruct->itemID >= 0 && IN_TEST(odSelected, State)){
		GetCanvas()->GetBrush()->SetColor(clHighlight);
		GetCanvas()->GetFont()->SetColor(clHighlightText);
	}
    if(Message.DrawItemStruct->itemID >= 0)
		DrawItem(Message.DrawItemStruct->itemID, *((PRect)&(Message.DrawItemStruct->rcItem)), State);
	else
		GetCanvas()->FillRect(*((PRect)&(Message.DrawItemStruct->rcItem)));
	if(IN_TEST(odFocused, State))
		DrawFocusRect(Message.DrawItemStruct->hDC, &(Message.DrawItemStruct->rcItem));
}

void CComboBox::CNMeasureItem(TWMMeasureItem& Message){
	Message.MeasureItemStruct->itemHeight = ItemHeight;
	if(Style == csOwnerDrawVariable){
		INT Ht = (INT)Message.MeasureItemStruct->itemHeight;
		MeasureItem(Message.MeasureItemStruct->itemID, Ht);
		Message.MeasureItemStruct->itemHeight = (UINT)Ht;
	}
}

void CComboBox::WMLButtonDown(TWMLButtonDown& Message){
	/*TODO
	if(GetDragMode() == dmAutomatic && GetStyle() == csDropDownList && 
		Message.XPos < (GetWidth() - GetSystemMetrics(SM_CXHSCROLL))){
		SetFocus();
		BeginDrag(FALSE);
		return ;
	}
	//*/
	INHERITED_MSG(Message);
	if(GetMouseCapture()){
		CForm* Form = GetParentForm(this);
		if(Form != NULL && Form->GetActiveControl() != this)
			SetMouseCapture(FALSE);
	}
}

void CComboBox::WMPaint(TWMPaint& Message){
	INT InnerStyles[] = {0, BDR_SUNKENINNER, BDR_RAISEDINNER, 0};
	INT OuterStyles[] = {0, BDR_SUNKENOUTER, BDR_RAISEDOUTER, 0};
	INT EdgeStyles[] = {0, 0, BF_SOFT, BF_FLAT};
	INT Ctl3DStyles[] = {BF_MONO, 0};
	/*
  EdgeSize: Integer;
  WinStyle: Longint;
  C: TControlCanvas;
  R: TRect;//*/
	INHERITED_MSG(Message);
	if(GetBevelKind() == bkNone)
		return;
	CControlCanvas* C = new CControlCanvas();
	CObjectHolder cHolder(C);
	C->SetControl(this);
	TRect R = GetClientRect();
	C->GetBrush()->SetColor(GetColor());
	C->FrameRect(R);
	InflateRect((LPRECT)&R,-1,-1);
	C->FrameRect(R);
	INT EdgeSize = 0;
	if(GetBevelKind() != bkNone){
		if(GetBevelInner() != bvNone)
			EdgeSize += GetBevelWidth();
		if(GetBevelOuter() != bvNone)
			EdgeSize += GetBevelWidth();
		if(EdgeSize == 0){
			R = GetClientRect();
			C->GetBrush()->SetColor(GetColor());
			C->FrameRect(R);
			InflateRect((LPRECT)&R,-1,-1);
			C->FrameRect(R);
		}
        R = GetClientRect();
		TRect bR = GetBoundsRect();
		LONG_PTR WinStyle = GetWindowLongPtr(GetHandle(), GWL_STYLE);
		if(IN_TEST(beLeft, GetBevelEdges()))
			bR.left -= EdgeSize;
		if(IN_TEST(beTop, GetBevelEdges()))
			bR.top -= EdgeSize;
		if(IN_TEST(beRight, GetBevelEdges()))
			bR.right += EdgeSize;
		if((WinStyle & WS_VSCROLL) != 0)
			bR.right += GetSystemMetrics(SM_CYVSCROLL);
		if(IN_TEST(beBottom, GetBevelEdges()))
			bR.bottom += EdgeSize;
		if((WinStyle & WS_HSCROLL) != 0)
			bR.bottom += GetSystemMetrics(SM_CXHSCROLL);
		DrawEdge(C->GetHandle(), (LPRECT)&R, InnerStyles[GetBevelInner()] | OuterStyles[GetBevelOuter()],
			(BYTE)GetBevelEdges() | EdgeStyles[GetBevelKind()] | Ctl3DStyles[GetCtl3D()] | BF_ADJUST);
		R.left = R.right - GetSystemMetrics(SM_CXHTHUMB) - 1;
		if(GetDroppedDown())
			DrawFrameControl(C->GetHandle(), (LPRECT)&R, DFC_SCROLL, DFCS_FLAT | DFCS_SCROLLCOMBOBOX);
		else
			DrawFrameControl(C->GetHandle(), (LPRECT)&R, DFC_SCROLL, DFCS_FLAT | DFCS_SCROLLCOMBOBOX);
	}
}

void CComboBox::WMNCCalcSize(TWMNCCalcSize& Message){
}

void CComboBox::CreateParams(TCreateParams& Params){
	DWORD ComboBoxStyles[] = {
		CBS_DROPDOWN, CBS_SIMPLE, CBS_DROPDOWNLIST,
		CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED,
		CBS_DROPDOWNLIST | CBS_OWNERDRAWVARIABLE};
	DWORD CharCases[] = {0, CBS_UPPERCASE, CBS_LOWERCASE};
	DWORD Sorts[] = {0, CBS_SORT};
	__super::CreateParams(Params);
	CreateSubClass(Params, TEXT("COMBOBOX"));
	Params.Style |= WS_VSCROLL | CBS_HASSTRINGS | CBS_AUTOHSCROLL |
		ComboBoxStyles[Style] | Sorts[Sorted] | CharCases[CharCase];
}

void CComboBox::CreateWnd(){
	__super::CreateWnd();
	DropHandle = GetHandle();
	if(SaveItems != NULL){
		Items->Assign(*SaveItems);
		delete SaveItems;
		SaveItems = NULL;
		if(SaveIndex != -1){
			if(Items->GetCount() < SaveIndex)
				SaveIndex = Items->GetCount();
			SendMessage(GetHandle(), CB_SETCURSEL, SaveIndex, 0);
		}
	}
	if(Style == csDropDown || Style == csSimple){
		HWND ChildHandle = GetWindow(GetHandle(), GW_CHILD);
		if(ChildHandle != 0){
			if(Style == csSimple){
				ListHandle = ChildHandle;
				DefListProc = (LPVOID)GetWindowLongPtr(ListHandle, GWLP_WNDPROC);
				SetWindowLongPtr(ListHandle, GWLP_WNDPROC, (LONG_PTR)ListInstance);
				ChildHandle = GetWindow(ChildHandle, GW_HWNDNEXT);
			}
			EditHandle = ChildHandle;
			DefEditProc = (LPVOID)GetWindowLongPtr(EditHandle, GWLP_WNDPROC);
			SetWindowLongPtr(EditHandle, GWLP_WNDPROC, (LONG_PTR)EditInstance);
		}
	}
	if(GetGlobal().GetNewStyleControls() && EditHandle != 0)
		SendMessage(EditHandle, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, 0);
}

void CComboBox::DestroyWnd(){
	if(Items->GetCount() > 0){
		SaveIndex = GetItemIndex();
		SaveItems = new CStringList();
		SaveItems->Assign(*Items);
	}
	__super::DestroyWnd();
}

void CComboBox::DrawItem(INT Index, TRect& Rect, TOwnerDrawState State){
	((CControlCanvas*)GetCanvas())->UpdateTextFlags();
	if(OnDrawItem != NULL)
		CALL_EVENT(DrawItem)(this, Index, Rect, State);
	else{
		GetCanvas()->FillRect(Rect);
		if(Index >= 0)
			GetCanvas()->TextOut(Rect.left + 2, Rect.top, Items->Get(Index).GetBuffer());
	}
}

INT CComboBox::GetItemHeight(){
	if(Style == csOwnerDrawFixed || Style == csOwnerDrawVariable)
		return ItemHeight;
	else
		return (INT)Perform(CB_GETITEMHEIGHT, 0, 0);
}

CCustomComboBoxStringsClass* CComboBox::GetItemsClass(){
	return (CCustomComboBoxStringsClass *)CComboBoxStrings::_Class;
}

BOOL CComboBox::HasSelectedText(DWORD& StartPos, DWORD& EndPos){
	SendMessage(GetHandle(), CB_GETEDITSEL, (WPARAM)&StartPos, (LPARAM)&EndPos);
    return EndPos > StartPos;
}

void CComboBox::DeleteSelectedText(){
	DWORD StartPos = 0;
	DWORD EndPos = 0;
	String OldText = GetTextString();
    SendMessage(GetHandle(), CB_GETEDITSEL, (WPARAM)&StartPos, (LPARAM)&EndPos);
    OldText.Delete(StartPos, EndPos - StartPos);
    SendMessage(GetHandle(), CB_SETCURSEL, -1, 0);
	SetText(OldText.GetBuffer());
    SendMessage(GetHandle(), CB_SETEDITSEL, 0, MAKELPARAM(StartPos, StartPos));
}

void CComboBox::KeyPress(TCHAR& Key){
	__super::KeyPress(Key);
	if(!GetAutoComplete())
		return ;
	if(GetStyle() == csDropDown || GetStyle() == csSimple){
		*Filter = String(GetText());
	}
	else{
		if(GetTickCount() - LastTime >= 500)
			*Filter = String(TEXT(""));
		LastTime = GetTickCount();
	}
	if(Key == VK_ESCAPE)
		return ;
	else if(Key == VK_TAB){
		if(AutoDropDown && GetDroppedDown())
			SetDroppedDown(FALSE);
	}
	else if(Key == VK_BACK){
		DWORD StartPos = 0;
		DWORD EndPos = 0;
		if(HasSelectedText(StartPos, EndPos))
			DeleteSelectedText();
        else{
			String SaveText = GetTextString();
			if((GetStyle() == csDropDown || GetStyle() == csSimple) && SaveText.Length() > 0){
				DWORD LastByte = StartPos;
				//TODO while(ByteType(SaveText, LastByte) == mbTrailByte)
				//	LastByte--;
				String OldText = SaveText.SubString(0, LastByte);
				SendMessage(GetHandle(), CB_SETCURSEL, -1, 0);
				SetText((OldText + SaveText.SubString(EndPos)).GetBuffer());
				SendMessage(GetHandle(), CB_SETEDITSEL, 0, MAKELPARAM(LastByte - 1, LastByte - 1));
				*Filter = GetTextString();
			}
			else{
				/*TODO
				while(ByteType(*Filter, Filter->Length()) == mbTrailByte)
					Filter->Delete(Filter->Length() - 1, 1);
				//*/
				Filter->Delete(Filter->Length() - 1, 1);
				
			}
			Key = TCHAR('\0');
			Change();
		}
	}// case
	else{
		if(AutoDropDown && !GetDroppedDown())
			SetDroppedDown(TRUE);
		DWORD StartPos = 0;
		DWORD EndPos = 0;
		String SaveText(NULL);
		if(HasSelectedText(StartPos, EndPos))
			SaveText = Filter->SubString(0, StartPos) + Key;
		else
			SaveText = *Filter + Key;
		/*TODO
		if(Key in LeadBytes){
			MSG Msg;
			if(PeekMessage(Msg, GetHandle(), 0, 0, PM_NOREMOVE) && Msg.Message == WM_CHAR){
				if(SelectItem(SaveText + (TCHAR)Msg.wParam)){
					PeekMessage(Msg, GetHandle(), 0, 0, PM_REMOVE);
					Key = TCHAR('\0');
				}
			}
		}
		else //*/
			if(SelectItem(SaveText))
				Key = TCHAR('\0');
	}// case
}

void CComboBox::MeasureItem(INT Index, INT& Height){
	if(OnMeasureItem != NULL)
		CALL_EVENT(MeasureItem)(this, Index, Height);
}

BOOL CComboBox::SelectItem(String& AnItem){
	if(AnItem.Length() == 0){
		SetItemIndex(-1);
		Change();
		return FALSE;
	}
	INT Idx = (INT)SendMessage(GetHandle(), CB_FINDSTRING, -1, (LPARAM)AnItem.GetBuffer());
	BOOL Result = Idx != CB_ERR;
	if(!Result)
		return Result;
	BOOL ValueChange = Idx != GetItemIndex();
	if(GetAutoCloseUp() && Items->IndexOf(AnItem) != -1)
		SendMessage(GetHandle(), CB_SHOWDROPDOWN, 0, 0);
	SendMessage(GetHandle(), CB_SETCURSEL, Idx, 0);
	if(GetStyle() == csDropDown || GetStyle() == csSimple){
		SetText((AnItem + Items->Get(Idx).SubString(AnItem.Length())).GetBuffer());
		SendMessage(GetHandle(), CB_SETEDITSEL, 0, MAKELPARAM(AnItem.Length(), GetTextLen()));
	}
	else{
		SetItemIndex(Idx);
		*Filter = AnItem;
	}
	if(ValueChange){
		Click();
		Select();
	}
	return Result;
}

void CComboBox::WndProc(TMessage& Message){
	if(Message.Msg >= CN_CTLCOLORMSGBOX && Message.Msg <= CN_CTLCOLORSTATIC){
		if(!GetGlobal().GetNewStyleControls() && GetStyle() < csDropDownList){
			Message.Result = (LRESULT)GetParent()->GetBrush()->GetHandle();
			return ;
		}
	}
	__super::WndProc(Message);
}

INT CComboBox::GetItemCount(){
	return Items->GetCount();;
}

void CComboBox::SetCharCase(TEditCharCase Value){
	if(CharCase != Value){
		CharCase = Value;
		RecreateWnd();
	}
}

String CComboBox::GetSelText(){
	String Result(TEXT(""));
	if(Style < csDropDownList){
		INT StartPos = GetSelStart();
		Result = GetTextString().SubString(StartPos, StartPos + GetSelLength());
	}
	return Result;
}

void CComboBox::SetSelText(String& Value){
	if(Style < csDropDownList){
		HandleNeeded();
		SendMessage(EditHandle, EM_REPLACESEL, 0, (LPARAM)Value.GetBuffer());
	}
}

void CComboBox::SetSorted(BOOL Value){
	if(Sorted != Value){
		Sorted = Value;
		RecreateWnd();
	}
}

void CComboBox::SetStyle(TComboBoxStyle Value){
	if(Style != Value){
		Style = Value;
		if(Value == csSimple)
			SetControlStyle(GetControlStyle() & (~csFixedHeight));
		else
			SetControlStyle(GetControlStyle() | csFixedHeight);
		RecreateWnd();
	}
}

