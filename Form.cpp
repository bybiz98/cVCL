
#include "stdinc.h"
#include "SysInit.hpp"
#include "ObjWndProc.hpp"
#include "WinUtils.hpp"
#include "Form.hpp"
#include "Screen.hpp"

CForm* MainForm = NULL;
BOOL FocusMessages = TRUE;
INT FocusCount = 0;

typedef struct _TaskWindow {
	struct _TaskWindow * Next;
	HWND Window;
} TaskWindow, *PTaskWindow;

HWND TaskActiveWindow = 0;
HWND TaskFirstWindow = 0;
HWND TaskFirstTopMost = 0;
PTaskWindow TaskWindowList = NULL;

void DoneApplication(){
	if(MainForm != NULL && MainForm->GetHandle() != 0){
		ShowOwnedPopups(MainForm->GetHandle(), FALSE);
	}
	//GetGlobal().GetApplication()->SetShowHint(FALSE);
	GetGlobal().GetApplication()->Destroying();
	GetGlobal().GetApplication()->DestroyComponents();
}

BOOL CALLBACK DoDisableWindow(HWND Window, LPARAM Data){
	PTaskWindow P = NULL;
	if (Window != TaskActiveWindow && IsWindowVisible(Window) && IsWindowEnabled(Window)){
		P = (PTaskWindow)malloc(sizeof(TaskWindow));
		P->Next = TaskWindowList;
		P->Window = Window;
		TaskWindowList = P;
		EnableWindow(Window, FALSE);
	}
	return TRUE;
}

void EnableTaskWindows(PTaskWindow WindowList){
	PTaskWindow P = NULL;
	while(WindowList != NULL){
		P = WindowList;
		if(IsWindow(P->Window)){
			EnableWindow(P->Window, TRUE);
		}
		WindowList = P->Next;
		free(P);
	}
}

PTaskWindow DisableTaskWindows(HWND ActiveWindow){
	PTaskWindow Result = NULL;
	HWND SaveActiveWindow = TaskActiveWindow;
	PTaskWindow SaveWindowList = TaskWindowList;
	TaskActiveWindow = ActiveWindow;
	TaskWindowList = NULL;
	__try{
		__try{
			EnumThreadWindows(GetCurrentThreadId(), &DoDisableWindow, 0);
			Result = TaskWindowList;
		}
		__except(EXCEPTION_EXECUTE_HANDLER){
			EnableTaskWindows(TaskWindowList);
			throw;
		}
	}
	__finally{
		TaskWindowList = SaveWindowList;
		TaskActiveWindow = SaveActiveWindow;
	}
	return Result;
}

BOOL CALLBACK DoFindWindow(HWND Window, LPARAM Param){
	if(Window != TaskActiveWindow && Window != MainForm->GetHandle() &&
		IsWindowVisible(Window) && IsWindowEnabled(Window)){
		if((GetWindowLong(Window, GWL_EXSTYLE) & WS_EX_TOPMOST) == 0){
			if(TaskFirstWindow == 0)
				TaskFirstWindow = Window;
		}
		else {
			if(TaskFirstTopMost == 0)
				TaskFirstTopMost = Window;
		}
	}
	return TRUE;
}

HWND FindTopMostWindow(HWND ActiveWindow){
  TaskActiveWindow = ActiveWindow;
  TaskFirstWindow = 0;
  TaskFirstTopMost = 0;
  EnumThreadWindows(GetCurrentThreadId(), &DoFindWindow, 0);
  if(TaskFirstWindow != 0)
		return TaskFirstWindow;
  else
		return TaskFirstTopMost;
}

BOOL SendFocusMessage(HWND Window, WORD Msg){
	INT Count = FocusCount;
	SendMessage(Window, Msg, 0, 0);
	return FocusCount == Count;
}

typedef struct _CheckTaskInfo{
    HWND FocusWnd;
	BOOL Found;
} CheckTaskInfo, *PCheckTaskInfo;

BOOL CALLBACK CheckTaskWindow(HWND Window, LPARAM Data){
	BOOL Result = TRUE;
	if(((PCheckTaskInfo)Data)->FocusWnd == Window){
		Result = FALSE;
		((PCheckTaskInfo)Data)->Found = TRUE;
	}
	return Result;
}

BOOL ForegroundTask(){
	CheckTaskInfo Info;
	Info.FocusWnd = GetActiveWindow();
	Info.Found = FALSE;
	EnumThreadWindows(GetCurrentThreadId(), &CheckTaskWindow, (LPARAM)&Info);
	return Info.Found;
}


IMPL_DYN_CLASS(CForm)
CForm::CForm(CComponent* AOwner) : CWinControl(AOwner),
	ActiveControl(NULL),
	FocusedControl(NULL),
	BorderIcons(biSystemMenu | biMinimize | biMaximize),
	BorderStyle(bsSizeable),
	WindowState(wsNormal),
	FormState(0),
	SizeChanging(FALSE),
	ClientHandle(0),
	lpClientWndProc(NULL),
	fnDefClientWndProc(NULL),
	MDIChildren(NULL),
	KeyPreview(FALSE),
	Active(FALSE),
	ShowAction(saIgnore),
	FormStyle(fsNormal),
	Position(poDesigned),
	ModalResult(mrNone),
	ActiveOleControl(NULL),
	DefaultMonitor(dmActiveForm),
	//InCMParentBiDiModeChanged(FALSE),
	//PrintScale(poProportional),
	//AlphaBlendValue(255),
	//TransparentColorValue(0),
	//SnapBuffer(10),
	INIT_EVENT(Activate),
	INIT_EVENT(Close),
	INIT_EVENT(CloseQuery),
	INIT_EVENT(Deactivate),
	INIT_EVENT(Help),
	INIT_EVENT(Hide),
	INIT_EVENT(Paint),
	INIT_EVENT(ShortCut),
	INIT_EVENT(Show),
	INIT_EVENT(Create),
	INIT_EVENT(Destroy)
	{
	SetControlStyle(csAcceptsControls | csCaptureMouse | csClickEvents
		| csSetCaption | csDoubleClicks);
	SetBounds(0, 0, 320, 240);
	Icon = new CIcon();
	Icon->SetWidth(GetSystemMetrics(SM_CXSMICON));
	Icon->SetHeight(GetSystemMetrics(SM_CYSMICON));
	Icon->SetOnChange(this, (TNotifyEvent)&CForm::IconChanged);
	Canvas = new CControlCanvas();
	Canvas->SetControl(this);
	//TODO PixelsPerInch = GetScreen()->GetPixelsPerInch();
	//FloatingDockSiteClass = TWinControlClass(ClassType);
	SetVisible(FALSE);
	SetParentColor(FALSE);
	SetParentFont(FALSE);
	SetCtl3D(TRUE);
	GetScreen()->AddForm(this);
	DoCreate();
}
CForm::~CForm(){
	DoDestroy();
	//MessageBox(0, TEXT("Form"), TEXT("CLOSE"), 0);
    //TODO MergeMenu(FALSE);
    if(HandleAllocated())
		DestroyWindowHandle();
	//MessageBox(0, TEXT("Form1"), TEXT("CLOSE"), 0);
    GetScreen()->RemoveForm(this);
    delete Canvas;
	delete Icon;
    //FreeAndNil(ActionLists);
}

void CForm::IconChanged(CObject* Sender){
	if(GetGlobal().GetNewStyleControls()){
		if(HandleAllocated() && BorderStyle != bsDialog)
			SendMessage(GetHandle(), WM_SETICON, 1, (LPARAM)GetIconHandle());
	}
	else
		if(IsIconic(GetHandle()))
			Invalidate();
}

HICON CForm::GetIconHandle(){
	HICON Result = Icon->GetHandle();
	if(Result == 0)
		Result = GetGlobal().GetAppIcon();
	return Result;
}

void CForm::DoCreate(){
	if(OnCreate != NULL){
		__try{
			CALL_EVENT(Create)(this);
		}
		__except(EXCEPTION_EXECUTE_HANDLER){
			if(!HandleCreateException())
				throw;
		}
	}
	if(IN_TEST(fsVisible, FormState))
		SetVisible(TRUE);
}

void CForm::DoDestroy(){
	if(OnDestroy != NULL){
		__try{
			CALL_EVENT(Destroy)(this);
		}
		__except(EXCEPTION_EXECUTE_HANDLER){
			GetGlobal().CallHandleException(this);
		}
	}
}

BOOL CForm::HandleCreateException(){
	GetGlobal().CallHandleException(this);
	return TRUE;
}

void CForm::CreateParams(TCreateParams& Params){
	__super::CreateParams(Params);
	//TODO InitAlphaBlending(Params);
	if(GetParent() == NULL && GetParentWindow() == 0){
		if(MainForm != NULL){
			Params.WndParent = MainForm->GetHandle();
		}
		else{
			Params.WndParent = 0;//TODO Application.Handle;
		}
		
		Params.Style &= ~(WS_CHILD | WS_GROUP | WS_TABSTOP);
	}
	Params.WinClass.style = CS_DBLCLKS;
	if(IN_TEST(csDesigning, GetComponentState()) && GetParent() == NULL)
		Params.Style |= WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX |
			WS_MAXIMIZEBOX | WS_SYSMENU;
	else {
		if(IN_TEST(Position, (poDefault | poDefaultPosOnly))){
			Params.Left = CW_USEDEFAULT;
			Params.Top = CW_USEDEFAULT;
		}
		TBorderIcons Icons = BorderIcons;
		TFormBorderStyle CreateStyle = BorderStyle;
		if(FormStyle == fsMDIChild && IN_TEST(CreateStyle, (bsNone | bsDialog)))
			CreateStyle = bsSizeable;
		switch(CreateStyle){
			case bsNone:
				if(GetParent() == NULL && GetParentWindow() == 0)
					Params.Style |= WS_POPUP;
				Icons = 0;
				break;
			case bsSingle:
			case bsToolWindow:
				Params.Style |= WS_CAPTION | WS_BORDER;
				break;
			case bsSizeable:
			case bsSizeToolWin:
				Params.Style |= WS_CAPTION | WS_THICKFRAME;
				if(IN_TEST(Position, (poDefault | poDefaultSizeOnly))){
					Params.Width = CW_USEDEFAULT;
					Params.Height = CW_USEDEFAULT;
				}
				break;
			case bsDialog:
				Params.Style |= WS_POPUP | WS_CAPTION;
				Params.ExStyle = WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE;
				//TODO AddBiDiModeExStyle(Params.ExStyle);
				if(!GetGlobal().GetNewStyleControls())
					Params.Style |= WS_DLGFRAME | DS_MODALFRAME;
				Icons &= biSystemMenu | biHelp;
				Params.WinClass.style = CS_DBLCLKS | CS_SAVEBITS | CS_BYTEALIGNWINDOW;
				break;
		}
		if(IN_TEST(CreateStyle, (bsToolWindow | bsSizeToolWin))){
			Params.ExStyle = WS_EX_TOOLWINDOW;
			//TODO AddBiDiModeExStyle(Params.ExStyle);
			Icons &= biSystemMenu;
		}
		if(IN_TEST(CreateStyle, (bsSingle | bsSizeable | bsNone))){
			if(FormStyle != fsMDIChild || IN_TEST(biSystemMenu, Icons)){
				if(IN_TEST(biMinimize, Icons))
					Params.Style |= WS_MINIMIZEBOX;
				if(IN_TEST(biMaximize, Icons))
					Params.Style |= WS_MAXIMIZEBOX;
			}
			if(WindowState == wsMinimized)
				Params.Style |= WS_MINIMIZE;
			else if(WindowState == wsMaximized)
				Params.Style |= WS_MAXIMIZE;
		}
		else
			WindowState = wsNormal;
		if(IN_TEST(biSystemMenu, Icons))
			Params.Style |= WS_SYSMENU;
		if(IN_TEST(biHelp, Icons))
			Params.ExStyle |= WS_EX_CONTEXTHELP;
		if(IN_TEST(csInline, GetComponentState()))
			Params.Style &= ~WS_CAPTION;
		if(FormStyle == fsMDIChild)
			Params.WinClass.lpfnWndProc = &DefMDIChildProc;
	}
}

void CForm::CreateWindowHandle(const TCreateParams& Params){
	if(GetFormStyle() == fsMDIChild && !IN_TEST(csDesigning, GetComponentState())){
		if(MainForm == NULL || MainForm->ClientHandle == 0)
			throw "there is no MDI form exists";
		MDICREATESTRUCT CreateStruct;
		CreateStruct.szClass = Params.WinClassName;
		CreateStruct.szTitle = Params.szTitle;
		CreateStruct.hOwner = GetGlobal().GetHInstance();
		CreateStruct.x = Params.Left;
		CreateStruct.y = Params.Top;
		CreateStruct.cx = Params.Width;
		CreateStruct.cy = Params.Height;
		CreateStruct.style = Params.Style;
		CreateStruct.lParam = (LPARAM)Params.lpParam;
		SetWnd((HWND)SendMessage(MainForm->ClientHandle, WM_MDICREATE, 0, (LPARAM)&CreateStruct));
		FormState |= fsCreatedMDIChild;
	}
	else{
		TCreateParams NewParams = Params;
		NewParams.ExStyle &= ~WS_EX_LAYERED;
		__super::CreateWindowHandle(NewParams);
		FormState &= ~fsCreatedMDIChild;
	}
	SetLayeredAttribs();
}

void CForm::CreateWnd(){
	__super::CreateWnd();
	if(GetGlobal().GetNewStyleControls())
		if(BorderStyle != bsDialog)
			SendMessage(GetHandle(), WM_SETICON, 1, (LPARAM)GetIconHandle());
		else
			SendMessage(GetHandle(), WM_SETICON, 1, 0);
	CLIENTCREATESTRUCT ClientCreateStruct;
	if(!IN_TEST(csDesigning, GetComponentState()))
		switch(FormStyle){
			case fsMDIForm:
				ClientCreateStruct.idFirstChild = 0xff00;
				ClientCreateStruct.hWindowMenu = 0;
				ClientCreateStruct.hWindowMenu = 0;
				//if(WindowMenu != NULL)
				//	ClientCreateStruct.hWindowMenu = WindowMenu->GetHandle();
				ClientHandle = ::CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("MDICLIENT"), 
					NULL, WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP |
					WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | WS_CLIPSIBLINGS |
					MDIS_ALLCHILDSTYLES, 0, 0, GetClientWidth(), GetClientHeight(), GetHandle(), 0,
					GetGlobal().GetHInstance(), &ClientCreateStruct);
				lpClientWndProc = MakeObjectWndProc(this, (TObjectWndProc)&CForm::ClientWndProc);
				fnDefClientWndProc = (WNDPROC)GetWindowLongPtr(ClientHandle, GWLP_WNDPROC);
				SetWindowLongPtr(ClientHandle, GWLP_WNDPROC, (LONG_PTR)lpClientWndProc);
				break;
			case fsStayOnTop:
				SetWindowPos(GetHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE |
					SWP_NOSIZE | SWP_NOACTIVATE);
				break;
	}
}

void CForm::DestroyWindowHandle(){
	if(IN_TEST(fsCreatedMDIChild, FormState))
		SendMessage(MainForm->ClientHandle, WM_MDIDESTROY, (WPARAM)GetHandle(), 0);
	else
		__super::DestroyWindowHandle();
	ClientHandle = 0;
}

void CForm::WndProc(TMessage& Message){
	switch(Message.Msg){
	case WM_ACTIVATE:
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		if(!FocusMessages)
			return;
		if(Message.Msg == WM_SETFOCUS && !IN_TEST(csDesigning, GetComponentState())){
			HWND FocusHandle = 0;
			if(GetFormStyle() == fsMDIForm){
				if(GetActiveMDIChild() != NULL)
					FocusHandle = GetActiveMDIChild()->GetHandle();
			}
			else if(ActiveControl != NULL && ActiveControl != this)
				FocusHandle = ActiveControl->GetHandle();
			if(FocusHandle != 0){
				::SetFocus(FocusHandle);
				return;
			}
		}
		break;
	case CM_EXIT:
		//if (HostDockSite != NULL)
		//	Deactivate();
		break;
	case CM_ENTER:
		//if(HostDockSite != NULL)
		//	Activate();
		break;
	case WM_WINDOWPOSCHANGING:
		if(((csLoading | csDesigning) & GetComponentState()) == csLoading){
			if((GetPosition() == poDefault || GetPosition() == poDefaultPosOnly) &&
				(GetWindowState() != wsMaximized))
				((LPWINDOWPOS)Message.lParam)->flags |= SWP_NOMOVE;
			if((GetPosition() == poDefault || GetPosition() == poDefaultSizeOnly) && 
				(GetBorderStyle() == bsSizeable || GetBorderStyle() == bsSizeToolWin))
				((LPWINDOWPOS)Message.lParam)->flags |= SWP_NOSIZE;
		}
		break;
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpDrawStruct = (LPDRAWITEMSTRUCT)Message.lParam;
			/*TODO
			if(lpDrawStruct->CtlType == ODT_MENU && Menu != NULL){
				CMenuItem* MenuItem = Menu->FindItem(lpDrawStruct->itemID, fkCommand);
				if(MenuItem != NULL){
					CControlCanvas* Canvas = new CControlCanvas();
					CObjectHolder cavsHolder(Canvas);
					CanvasHandleCleaner cavsCleaner(Canvas);
					INT SaveIndex = SaveDC(lpDrawStruct->hDC);
					DCRestorer dcRestore(lpDrawStruct->hDC, SaveIndex);
					Canvas->SetHandle(lpDrawStruct->hDC);
					Canvas->SetFont(GetScreen()->GetMenuFont());
					Menus.DrawMenuItem(MenuItem, Canvas, lpDrawStruct->rcItem,
						lpDrawStruct->itemState);
					return ;
				}
			}
			//*/
		}
		break;
	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpDrawStruct = (LPMEASUREITEMSTRUCT)Message.lParam;
			/*
			if(lpDrawStruct->CtlType == ODT_MENU && Menu != NULL){
				CMenuItem* MenuItem = Menu->FindItem(lpDrawStruct->itemID, fkCommand);
				if(MenuItem != NULL){
					HDC DC = GetWindowDC(GetHandle());
					DCHolder dcHolder(DC, 0, GetHandle());
					CControlCanvas* Canvas = new CControlCanvas();
					CObjectHolder cavsHolder(Canvas);
					CanvasHandleCleaner cavsCleaner(Canvas);
					INT SaveIndex = SaveDC(DC);
					DCRestorer dcRestore(lpDrawStruct->hDC, SaveIndex);
					Canvas->SetHandle(DC);
					Canvas->SetFont(GetScreen()->GetMenuFont());
					((CMenuItemAccess *)MenuItem)->MeasureItem(Canvas,
						(INT)lpDrawStruct->itemWidth, (INT)lpDrawStruct->itemHeight);
					return ;
				}
			}//*/
		}
		break;
	default:
		if (Message.Msg == GetGlobal().GetRM_TaskbarCreated()){
			Perform(CM_WININICHANGE, 0, 0);
			Perform(CM_SYSCOLORCHANGE, 0, 0);
			Perform(CM_SYSFONTCHANGED, 0, 0);
			Perform(CM_PARENTCOLORCHANGED, 0, 0);
			Perform(CM_PARENTFONTCHANGED, 0, 0);
			Perform(CM_PARENTBIDIMODECHANGED, 0, 0);
		}
	}
	__super::WndProc(Message);
}

void DoNestedActivation(UINT Msg, CWinControl* Control, CForm* Form){
	if(Control == NULL)
		return;
	//{ Find the closest parent which is a form }
	while(Control->GetParent() != NULL && !Control->InstanceOf(CForm::_Class))
		Control = Control->GetParent();
	if(Control != NULL && Control != Form)
		SendMessage(Control->GetHandle(), Msg, 0, 0);
}

void CForm::Deactivate(){
	DoNestedActivation(CM_DEACTIVATE, ActiveControl, this);
	if(OnDeactivate != NULL)
		CALL_EVENT(Deactivate)(this);
}

void CForm::SetFormStyle(TFormStyle Value){
	if(FormStyle != Value){
		if(Value == fsMDIChild && Position == poDesigned)
			SetPosition(poDefault);
		if(!IN_TEST(csDesigning, GetComponentState()))
			DestroyHandle();
		TFormStyle OldStyle = FormStyle;
		FormStyle = Value;
		if((Value == fsMDIForm || OldStyle == fsMDIForm) && !GetCtl3D())
			SetColor(NormalColor());
		if(!IN_TEST(csDesigning, GetComponentState()))
			UpdateControlState();
		if(Value == fsMDIChild)
			SetVisible(TRUE);
	}
}

TColor CForm::NormalColor(){
	TColor Result = clWindow;
	if(FormStyle == fsMDIForm)
		Result = clAppWorkSpace;
	return Result;
}

void CForm::SetPosition(TPosition Value){
	if(Position != Value){
		Position = Value;
		if(!IN_TEST(csDesigning, GetComponentState()) && HandleAllocated())
			RecreateWnd();
	}
}

void CForm::SetWindowState(TWindowState Value){
	INT ShowCommands[]={SW_SHOWNORMAL, SW_MINIMIZE, SW_SHOWMAXIMIZED};
	if(WindowState != Value){
		WindowState = Value;
		if(!IN_TEST(csDesigning, GetComponentState()) && GetShowing())
			ShowWindow(GetHandle(), ShowCommands[Value]);
	}
}

void CForm::SetBorderIcons(TBorderIcons Value){
	if(BorderIcons != Value){
		BorderIcons = Value;
		if(!IN_TEST(csDesigning, GetComponentState()))
			RecreateWnd();
	}
}

void CForm::SetBorderStyle(TFormBorderStyle Value){
	if(BorderStyle != Value){
		BorderStyle = Value;
		//TODO AutoScroll = BorderStyle == bsSizeable | BorderStyle == bsSizeToolWin;
		if(!IN_TEST(csDesigning, GetComponentState()))
			RecreateWnd();
	}
}

void CForm::SetActiveControl(CWinControl* Control){
	if(ActiveControl != Control){
		if(!(Control == NULL || Control != this &&
			GetParentForm(Control) == this && (IN_TEST(csLoading, GetComponentState()) ||
			Control->CanFocus())))
			throw "can not focus."; //TODO raise EInvalidOperation.Create(SCannotFocus);
		ActiveControl = Control;
		if(!IN_TEST(csLoading, GetComponentState())){
			if(Active)
				SetWindowFocus();
			ActiveChanged();
		}
	}
}

BOOL CForm::SetFocusedControl(CWinControl* Control){
	BOOL Result = FALSE;
	FocusCount++;
	//TODO if(Designer == NULL)
		if(Control != this)
			ActiveControl = Control;
		else
			ActiveControl = NULL;
	CScreen* Screen = GetScreen();
	Screen->ActiveControl = Control;
	Screen->ActiveCustomForm = this;
	Screen->CustomForms->Remove(this);
	Screen->CustomForms->Insert(0, this);
	if(this->InstanceOf(CForm::_Class)){
		Screen->ActiveForm = (CForm *)this;
		Screen->Forms->Remove(this);
		Screen->Forms->Insert(0, this);
	}
	else 
		Screen->ActiveForm = NULL;
	if(!IN_TEST(csFocusing, Control->GetControlState())){
		Control->SetControlState(Control->GetControlState() | csFocusing);
		__try{
			if(Screen->FocusedForm != this){
				if(Screen->FocusedForm != NULL){
					HWND FocusHandle = Screen->FocusedForm->GetHandle();
					Screen->FocusedForm = NULL;
					if(!SendFocusMessage(FocusHandle, CM_DEACTIVATE))
						return Result;
				}
				Screen->FocusedForm = this;
				if(!SendFocusMessage(GetHandle(), CM_ACTIVATE))
					return Result;
			}
			if(FocusedControl == NULL)
				FocusedControl = this;
			CWinControl* TempControl = NULL;
			if(FocusedControl != Control){
				while (FocusedControl != NULL && !FocusedControl->ContainsControl(Control)){
					HWND FocusHandle = FocusedControl->GetHandle();
					FocusedControl = FocusedControl->GetParent();
					if(!SendFocusMessage(FocusHandle, CM_EXIT))
						return Result;
				}
				while(FocusedControl != Control){
					TempControl = Control;
					while(TempControl->GetParent() != FocusedControl)
						TempControl = TempControl->GetParent();
					FocusedControl = TempControl;
					if(!SendFocusMessage(TempControl->GetHandle(), CM_ENTER))
						return Result;
				}
			}
			/*
			TempControl = Control->GetParent();
			while(TempControl != NULL){
				if(TempControl->InstanceOf(CScrollingWinControl::_Class))
					((CScrollingWinControl *)TempControl)->AutoScrollInView(Control);
				TempControl = TempControl->GetParent();
			}
			//*/
			Perform(CM_FOCUSCHANGED, 0, (LPARAM)Control);
			if(GetActiveOleControl() != NULL && GetActiveOleControl() != Control)	
				GetActiveOleControl()->Perform(CM_UIDEACTIVATE, 0, 0);
		}
		__finally{
			Control->SetControlState(Control->GetControlState() & (~csFocusing));
		}
		Screen->UpdateLastActive();
		Result = TRUE;
	}
	return Result;
}

void CForm::SetLayeredAttribs(){
	/*
	const
  cUseAlpha: array [Boolean] of Integer = (0, LWA_ALPHA);
  cUseColorKey: array [Boolean] of Integer = (0, LWA_COLORKEY);
var
  AStyle: Integer;
begin
  if not (csDesigning in ComponentState) and
    (Assigned(SetLayeredWindowAttributes)) and HandleAllocated then
  begin
    AStyle := GetWindowLong(Handle, GWL_EXSTYLE);
    if FAlphaBlend or FTransparentColor then
    begin
      if (AStyle and WS_EX_LAYERED) = 0 then
        SetWindowLong(Handle, GWL_EXSTYLE, AStyle or WS_EX_LAYERED);
      SetLayeredWindowAttributes(Handle, FTransparentColorValue, FAlphaBlendValue,
        cUseAlpha[FAlphaBlend] or cUseColorKey[FTransparentColor]);
    end
    else
    begin
      SetWindowLong(Handle, GWL_EXSTYLE, AStyle and not WS_EX_LAYERED);
      RedrawWindow(Handle, nil, 0, RDW_ERASE or RDW_INVALIDATE or RDW_FRAME or RDW_ALLCHILDREN);
    end;
  end;
  //*/
}

void CForm::ClientWndProc(TMessage& Message){
}

void CForm::Close(){
	TCloseAction CloseAction = 0;
	if(IN_TEST(fsModal, FormState))
		ModalResult = mrCancel;
	else if(CloseQuery()){
		if(FormStyle == fsMDIChild){
			if(IN_TEST(biMinimize, BorderIcons))
				CloseAction = caMinimize;
			else 
				CloseAction = caNone;
		}
		else
			CloseAction = caHide;
		DoClose(CloseAction);
		if(CloseAction != caNone){
			if(MainForm == this)
				PostQuitMessage(0);//TODO Application.Terminate
			else if(CloseAction == caHide)
				Hide();
			else if(CloseAction == caMinimize)
				SetWindowState(wsMinimized);
			else Release();
		}
	}
}

BOOL CForm::CloseQuery(){
	BOOL Result = TRUE;
	if(FormStyle == fsMDIForm){
		Result = FALSE;
		INT Count = GetMDIChildCount();
		for(INT I = 0; I < Count; I++)
		  if(!GetMDIChildren(I)->CloseQuery())
			  return Result;
	}
	Result = TRUE;
	if(OnCloseQuery != NULL)
		CALL_EVENT(CloseQuery)(this, Result);
	return Result;
}

void CForm::Hide(){
	SetVisible(FALSE);
}

void CForm::Show(){
	SetVisible(TRUE);
	BringToFront();
}

void CForm::CloseModal(){
	TCloseAction CloseAction;
	__try{
		CloseAction = caNone;
		if(CloseQuery()){
			CloseAction = caHide;
			DoClose(CloseAction);
		}
		switch(CloseAction){
			case caNone:
				ModalResult = 0;
				break;
			case caFree:
				Release();
				break;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		ModalResult = 0;
		GetGlobal().GetApplication()->HandleException(this);
	}
}


void CForm::SetWindowToMonitor(){
	HMONITOR AppMon = 0, WinMon = 0;
	INT I = 0, J = 0;
	INT ALeft = 0, ATop = 0;
	if((DefaultMonitor != dmDesktop) && (MainForm != NULL)){
		AppMon = 0;
		if(DefaultMonitor == dmMainForm)
			AppMon = MainForm->GetMonitor()->GetHandle();
		else if((DefaultMonitor == dmActiveForm) && (GetScreen()->GetActiveCustomForm() != NULL))
			AppMon = GetScreen()->GetActiveCustomForm()->GetMonitor()->GetHandle();
		else if(DefaultMonitor == dmPrimary)
			AppMon = GetScreen()->GetMonitor(0)->GetHandle();
		WinMon = GetMonitor()->GetHandle();
		for(I = 0; I < GetScreen()->GetMonitorCount(); I++){
			if(GetScreen()->GetMonitor(I)->GetHandle() == AppMon){
				if (AppMon != WinMon){
					for(J = 0; J < GetScreen()->GetMonitorCount(); J++){
						if(GetScreen()->GetMonitor(J)->GetHandle() == WinMon){
							CMonitor* MonitorI = GetScreen()->GetMonitor(I);
							CMonitor* MonitorJ = GetScreen()->GetMonitor(J);
							if(Position == poScreenCenter){
								SetBounds(MonitorI->GetLeft() + ((MonitorI->GetWidth() - GetWidth()) / 2),
									MonitorI->GetTop() + ((MonitorI->GetHeight() - GetHeight()) / 2),
									GetWidth(), GetHeight());
							}
							else if (Position == poMainFormCenter){
								SetBounds(MonitorI->GetLeft() + ((MonitorI->GetWidth() - GetWidth()) / 2),
									MonitorI->GetTop() + ((MonitorI->GetHeight() - GetHeight()) / 2),
									GetWidth(), GetHeight());
							}
							else {
								ALeft = MonitorI->GetLeft() + GetLeft() - MonitorJ->GetLeft();
								if(ALeft + GetWidth() > MonitorI->GetLeft() + MonitorI->GetWidth())
									ALeft = MonitorI->GetLeft() + MonitorI->GetWidth() - GetWidth();
								ATop = MonitorI->GetTop() + GetTop() - MonitorJ->GetTop();
								if(ATop + GetHeight() > MonitorI->GetTop() + MonitorI->GetHeight())
									ATop = MonitorI->GetTop() + MonitorI->GetHeight() - GetHeight();
								SetBounds(ALeft, ATop, GetWidth(), GetHeight());
							}
						}
					}
				}
			}
		}
	}
}

CMonitor* CForm::GetMonitor(){
	INT I = 0;
	HMONITOR HM = MonitorFromWindow(GetHandle(), MONITOR_DEFAULTTONEAREST);
	for(I = 0; I < GetScreen()->GetMonitorCount(); I++){
		if(GetScreen()->GetMonitor(I)->GetHandle() == HM){
			return GetScreen()->GetMonitor(I);
		}
	}
	//if we get here, the Monitors array has changed, so we need to clear and reinitialize it
	GetScreen()->RefreshMonitors();

	for(I = 0; I < GetScreen()->GetMonitorCount(); I++){
		if(GetScreen()->GetMonitor(I)->GetHandle() == HM){
			return GetScreen()->GetMonitor(I);
		}
	}
	return NULL;
}

INT CForm::ShowModal(){
	INT Result = 0;
	//CancelDrag();
	if(GetVisible() || !GetEnabled() || IN_TEST(fsModal, FormState) || (FormStyle == fsMDIChild))
		throw "Cannot make a visible window modal";
	if(GetCapture() != 0){
		SendMessage(GetCapture(), WM_CANCELMODE, 0, 0);
	}
	ReleaseCapture();
	GetGlobal().GetApplication()->ModalStarted();
	__try{
		FormState |= fsModal;
		HWND ActiveWindow = GetActiveWindow();
		INT SaveFocusCount = FocusCount;
		GetScreen()->GetSaveFocusedList()->Insert(0, GetScreen()->GetFocusedForm());
		GetScreen()->SetFocusedForm(this);
		TCursor SaveCursor = GetScreen()->GetCursor();
		GetScreen()->SetCursor(crDefault);
		INT SaveCount = GetScreen()->GetCursorCount();
		PTaskWindow WindowList = DisableTaskWindows(0);
		__try{
			Show();
			__try{
				SendMessage(GetHandle(), CM_ACTIVATE, 0, 0);
				ModalResult = 0;
				do{
					GetGlobal().GetApplication()->HandleMessage();
					if(GetGlobal().GetApplication()->GetTerminated()){
						ModalResult = mrCancel;
					}
					else{
						if(ModalResult != 0){
							CloseModal();
						}
					}
				}while(ModalResult == 0);
				Result = ModalResult;
				SendMessage(GetHandle(), CM_DEACTIVATE, 0, 0);
				if(GetActiveWindow() != GetHandle()){
					ActiveWindow = 0;
				}
			}__finally{
				Hide();
			}
		}
		__finally{
			if(GetScreen()->GetCursorCount() == SaveCount){
				GetScreen()->SetCursor(SaveCursor);
			}
			else{
				GetScreen()->SetCursor(crDefault);
			}
			EnableTaskWindows(WindowList);
			if (GetScreen()->GetSaveFocusedList()->GetCount() > 0){
				GetScreen()->SetFocusedForm((CForm *)GetScreen()->GetSaveFocusedList()->First());
				GetScreen()->GetSaveFocusedList()->Remove(GetScreen()->GetFocusedForm());
			}
			else{
				GetScreen()->SetFocusedForm(NULL);
			}
			if(ActiveWindow != 0){
				SetActiveWindow(ActiveWindow);
			}
			FocusCount = SaveFocusCount;
			//*/
			FormState &= !fsModal;
		}
	}
	__finally{
		GetGlobal().GetApplication()->ModalFinished();
	}
	return Result;
}

void CForm::Release(){
	PostMessage(GetHandle(), CM_RELEASE, 0, 0);
}

void CForm::BringToFront(){
	SetZOrder(TRUE);
}

void CForm::SendToBack(){
	SetZOrder(FALSE);
}

void CForm::FocusControl(CWinControl* Control){
	BOOL WasActive = Active;
	SetActiveControl(Control);
	if(!WasActive)
		SetFocus();
}

void CForm::SetFocus(){
	if(!Active){
		if(!(GetVisible() && GetEnabled()))
			throw "can not focus.";//TODO raise EInvalidOperation.Create(SCannotFocus);
		SetWindowFocus();
	}
}

void CForm::SendCancelMode(CControl* Sender){
	if(GetActive() && ActiveControl != NULL)
		ActiveControl->Perform(CM_CANCELMODE, 0, (LPARAM)Sender);
	//if(GetFormStyle() == fsMDIForm && ActiveMDIChild != NULL)
	//	ActiveMDIChild->SendCancelMode(Sender);
}

void CForm::DoClose(TCloseAction& Action){
	if(OnClose != NULL)
		CALL_EVENT(Close)(this, Action);
}

void CForm::DoHide(){
	if(OnHide != NULL)
		CALL_EVENT(Hide)(this);
}

void CForm::DoShow(){
	if(OnShow != NULL)
		CALL_EVENT(Show)(this);
}

void CForm::Paint(){
	if(OnPaint != NULL)
		CALL_EVENT(Paint)(this);
}

void CForm::PaintWindow(HDC DC){
	CMethodLock Lock(Canvas, (TLockMethod)&CCanvas::Lock, (TLockMethod)&CCanvas::UnLock);
	Canvas->SetHandle(DC);
	CanvasHandleCleaner Cleaner(Canvas);
	/*
	if(Designer != NULL)
		Designer->PaintGrid();
	else //*/
		Paint();
}

void CForm::Activate(){
	DoNestedActivation(CM_ACTIVATE, ActiveControl, this);
	if(OnActivate != NULL) 
		CALL_EVENT(Activate)(this);
}

void CForm::ActiveChanged(){
}

void CForm::Resizing(TWindowState State){
	if(!IN_TEST(csDesigning, GetComponentState())){
		WindowState = State;
	}
	if(State != wsMinimized){
		RequestAlign();
	}
	//if FOleForm <> nil then FOleForm.OnResize;
}

void CForm::SelectNext(CWinControl* CurControl, BOOL GoForward, BOOL CheckTabStop){
	CurControl = FindNextControl(CurControl, GoForward, CheckTabStop, !CheckTabStop);
	if(CurControl != NULL)
		CurControl->SetFocus();
}

INT CForm::GetMDIChildCount(){
	INT Result = 0;
	if(FormStyle == fsMDIForm && ClientHandle != 0){
		Result = MDIChildren == NULL ? 0 : MDIChildren->GetCount();
		/*
		INT Count = MDIChildren == NULL ? 0 : MDIChildren->GetCount();
		for(INT I = 0; I < Count; I++)
			if(Screen.Forms[I].FormStyle == fsMDIChild)
				Result++;
		//*/
	}
	return Result;
}
CForm* CForm::GetMDIChildren(INT I){
	if(MDIChildren == NULL){
		return NULL;
	}
	else{
		return (CForm*)MDIChildren->Get(I);
	}
}

void CForm::WMClose(TWMClose& Message){
	Close();
}

void CForm::ModifySystemMenu(){
	HMENU SysMenu = 0;
	if(BorderStyle != bsNone && IN_TEST(biSystemMenu, BorderIcons) && 
		FormStyle != fsMDIChild){
		//{ Modify the system menu to look more like it's s'pose to }
		SysMenu = GetSystemMenu(GetHandle(), FALSE);
		if(BorderStyle == bsDialog){
			//{ Make the system menu look like a dialog which has only Move and Close }
			DeleteMenu(SysMenu, SC_TASKLIST, MF_BYCOMMAND);
			DeleteMenu(SysMenu, 7, MF_BYPOSITION);
			DeleteMenu(SysMenu, 5, MF_BYPOSITION);
			DeleteMenu(SysMenu, SC_MAXIMIZE, MF_BYCOMMAND);
			DeleteMenu(SysMenu, SC_MINIMIZE, MF_BYCOMMAND);
			DeleteMenu(SysMenu, SC_SIZE, MF_BYCOMMAND);
			DeleteMenu(SysMenu, SC_RESTORE, MF_BYCOMMAND);
		}
		else {
			//{ Else just disable the Minimize and Maximize items if the
			//	corresponding FBorderIcon is not present }
			if(!IN_TEST(biMinimize, BorderIcons))
				EnableMenuItem(SysMenu, SC_MINIMIZE, MF_BYCOMMAND | MF_GRAYED);
			if(!IN_TEST(biMaximize, BorderIcons))
				EnableMenuItem(SysMenu, SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
		}
	}
}

void CForm::SetWindowFocus(){
	CWinControl* FocusControl = NULL;
	if(ActiveControl != NULL)// && Designer == NULL)
		FocusControl = ActiveControl;
	else
		FocusControl = this;
	::SetFocus(FocusControl->GetHandle());
	if(::GetFocus() == FocusControl->GetHandle())
		FocusControl->Perform(CM_UIACTIVATE, 0, 0);
}

CForm* CForm::GetActiveMDIChild(){
	CForm* Result = NULL;
	if(GetFormStyle() == fsMDIForm && ClientHandle != 0)
		Result = (CForm *)FindControl((HWND)SendMessage(ClientHandle, WM_MDIGETACTIVE, 0, 0));
	return Result;
}

void CForm::WMNCCreate(TWMNCCreate& Message){
	INHERITED_MSG(Message);
	//TODOSetMenu(FMenu);
	if(!IN_TEST(csDesigning, GetComponentState()))
		ModifySystemMenu();
}

void CForm::WMNextDlgCtl(TWMNextDlgCtl& Message){
	if(Message.Handle)
		::SetFocus(Message.CtlFocus);
	else
		SelectNext(ActiveControl, !(BOOL)Message.CtlFocus, TRUE);
}

void CForm::WMActivate(TWMActivate& Message){
	if(GetFormStyle() != fsMDIForm || IN_TEST(csDesigning, GetComponentState()))
		SetActive(Message.Active != WA_INACTIVE);
}

void CForm::WMSize(TWMSize& Message){
	INHERITED_MSG(Message);
	TWindowState NewState = wsNormal;
	switch(Message.SizeType){
		case SIZENORMAL: 
			NewState = wsNormal;
			break;
		case SIZEICONIC:
			NewState = wsMinimized;
			break;
		case SIZEFULLSCREEN:
			NewState = wsMaximized;
			break;
	}
	Resizing(NewState);
	/*
	Inc(FAutoRangeCount);
	try
    inherited;
    case Message.SizeType of
      SIZENORMAL: 
      SIZEICONIC: 
      : 
    end;
    
  finally
    Dec(FAutoRangeCount);
  end;
  FUpdatingScrollBars := True;
  try
    CalcAutoRange;
  finally
    FUpdatingScrollBars := False;
  end;
  if FHorzScrollBar.Visible or FVertScrollBar.Visible then
    UpdateScrollBars;
	//*/
}

void CForm::CMDialogKey(TCMDialogKey& Message){
	if(GetKeyState(VK_MENU) >= 0){
		if(Message.CharCode == VK_TAB){
			if(GetKeyState(VK_CONTROL) >= 0){
				SelectNext(ActiveControl, GetKeyState(VK_SHIFT) >= 0, TRUE);
				Message.Result = 1;
				return;
			}
		}
		else if(Message.CharCode == VK_LEFT || Message.CharCode == VK_RIGHT || 
			Message.CharCode == VK_UP || Message.CharCode == VK_DOWN){
			if(ActiveControl != NULL){
				((CForm *)ActiveControl->GetParent())->SelectNext(ActiveControl,
					(Message.CharCode == VK_RIGHT) || (Message.CharCode == VK_DOWN), FALSE);
				Message.Result = 1;
			}
			return;
		}
	}
	INHERITED_MSG(Message);
}

void CForm::CMDeactivate(TCMDeactivate& Message){
	if(!IN_TEST(csReading, GetComponentState()))
		Deactivate();
	else
		FormState &= ~fsActivated;

}

void CForm::CMActivate(TCMActivate& Message){
	if(!IN_TEST(csReading, GetComponentState()))
		Activate();
	else
		FormState |= fsActivated;
}

void CForm::CMShowingChanged(TMessage& Message){
	INT ShowCommands[] = {SW_SHOWNORMAL, SW_SHOWMINNOACTIVE, SW_SHOWMAXIMIZED};

	INT X = 0, Y = 0;
	CForm* CenterForm = NULL;
	HWND NewActiveWindow = 0;
	if(!IN_TEST(csDesigning, GetComponentState()) && IN_TEST(fsShowing, FormState))
		throw "Cannot change Visible in OnShow or OnHide";
	GetGlobal().GetApplication()->UpdateVisible();
	FormState |= fsShowing;
	__try{
		if(!IN_TEST(csDesigning, GetComponentState())){
			if(GetShowing()){
				__try{
					DoShow();
				}
				__except(EXCEPTION_EXECUTE_HANDLER){
					GetGlobal().GetApplication()->HandleException(this);
				}
				if((Position == poScreenCenter) || ((Position == poMainFormCenter) && (FormStyle == fsMDIChild))){
					if(FormStyle == fsMDIChild){
						X = (MainForm->GetClientWidth() - GetWidth()) / 2;
						Y = (MainForm->GetClientHeight() - GetHeight()) / 2;
					}
					else{
						X = (GetScreen()->GetWidth() - GetWidth()) / 2;
						Y = (GetScreen()->GetHeight() - GetHeight()) / 2;
					}
					if(X < 0) X = 0;
					if(Y < 0) Y = 0;
					SetBounds(X, Y, GetWidth(), GetHeight());
					if(GetVisible())
						SetWindowToMonitor();
				}
				else if(IN_TEST(Position, poMainFormCenter | poOwnerFormCenter)){
					CenterForm = MainForm;
					if((Position == poOwnerFormCenter) && (GetOwner()->InstanceOf(CForm::_Class)))
						CenterForm = (CForm*)GetOwner();
					if(CenterForm != NULL){
						X = ((CenterForm->GetWidth() - GetWidth()) / 2) + CenterForm->GetLeft();
						Y = ((CenterForm->GetHeight() - GetHeight()) / 2) + CenterForm->GetTop();
					}
					else{
						X = (GetScreen()->GetWidth() - GetWidth()) / 2;
						Y = (GetScreen()->GetHeight() - GetHeight()) / 2;
					}
					if(X < 0) X = 0;
					if(Y < 0) Y = 0;
					SetBounds(X, Y, GetWidth(), GetHeight());
					if(GetVisible())
						SetWindowToMonitor();
				}
				else if (Position == poDesktopCenter){
					if(FormStyle == fsMDIChild){
						X = (MainForm->GetClientWidth() - GetWidth()) / 2;
						Y = (MainForm->GetClientHeight() - GetHeight()) / 2;
					}
					else {
						X = (GetScreen()->GetDesktopWidth() - GetWidth()) / 2;
						Y = (GetScreen()->GetDesktopHeight() - GetHeight()) / 2;
					}
					if(X < 0) X = 0;
					if(Y < 0) Y = 0;
					SetBounds(X, Y, GetWidth(), GetHeight());
				}
				Position = poDesigned;
				if(FormStyle == fsMDIChild){
					//Fake a size message to get MDI to behave 
					if (WindowState == wsMaximized){
						SendMessage(MainForm->GetClientHandle(), WM_MDIRESTORE, (WPARAM)GetHandle(), 0);
						ShowWindow(GetHandle(), SW_SHOWMAXIMIZED);
					}
					else{
						ShowWindow(GetHandle(), ShowCommands[WindowState]);
						CallWindowProc(&DefMDIChildProc, GetHandle(), WM_SIZE, SIZE_RESTORED,
							GetWidth() | (GetHeight() << 16));
						BringToFront();
					}
					SendMessage(MainForm->GetClientHandle(), WM_MDIREFRESHMENU, 0, 0);
				}
				else ShowWindow(GetHandle(), ShowCommands[WindowState]);
			}
			else{
				__try{
					DoHide();
				}
				__except(EXCEPTION_EXECUTE_HANDLER){
					GetGlobal().GetApplication()->HandleException(this);
				}
				//if(GetScreen()->GetActiveForm() == this)
				//	MergeMenu(FALSE);
				if(FormStyle == fsMDIChild)
					DestroyHandle();
				else if(IN_TEST(fsModal, FormState))
					SetWindowPos(GetHandle(), 0, 0, 0, 0, 0, SWP_HIDEWINDOW |
						SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
				else{
					NewActiveWindow = 0;
					if(GetActiveWindow() == GetHandle() && !IsIconic(GetHandle())){
						NewActiveWindow = FindTopMostWindow(GetHandle());
					}
					if(NewActiveWindow != 0){
						SetWindowPos(GetHandle(), 0, 0, 0, 0, 0, SWP_HIDEWINDOW |
							SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
						SetActiveWindow(NewActiveWindow);
					}
					else {
						ShowWindow(GetHandle(), SW_HIDE);
					}
				}
			}
		}
	}
	__finally{
		FormState &= !fsShowing;
	}
}

void CForm::MouseWheelHandler(TMessage& Message){
	if(FocusedControl != NULL)
		Message.Result = FocusedControl->Perform(CM_MOUSEWHEEL, Message.wParam, Message.lParam);
    else
      __super::MouseWheelHandler(Message);
}

BOOL CForm::WantChildKey(CControl* Child, TMessage& Message){
	return FALSE;
}

void CForm::SetActive(BOOL Value){
	Active = Value;
	if(GetActiveOleControl() != NULL)
		GetActiveOleControl()->Perform(CM_DOCWINDOWACTIVATE, (WPARAM)Value, 0);
	if(Value){
		if(GetActiveControl() == NULL && !IN_TEST(csDesigning, GetComponentState()))
			SetActiveControl(FindNextControl(NULL, TRUE, TRUE, FALSE));
		//MergeMenu(TRUE);
		SetWindowFocus();
	}
}