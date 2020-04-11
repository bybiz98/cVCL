#include "stdinc.h"
#include "Screen.hpp"
#include "SysInit.hpp"
#include "WinUtils.hpp"


CScreen* Screen = NULL;

CScreen* GetScreen(){
	if(Screen == NULL)
		Screen = new CScreen();
	return Screen;
}

BOOL CALLBACK EnumMonitorsProc(HMONITOR hm, HDC dc, LPRECT r, LPVOID Data){
	CList* L = (CList *)Data;
	CMonitor* M = new CMonitor();
	M->SetHandle(hm);
	M->SetMonitorNum(L->GetCount());
	L->Add(M);
	return TRUE;
}

INT CALLBACK EnumFontsProc(LOGFONT* LogFont, TEXTMETRIC* TextMetric,
	DWORD FontType, LPARAM Data){
	CStrings* S = (CStrings*)Data;
	if(S->GetCount() == 0 || S->Get(S->GetCount() -1) != LogFont->lfFaceName)
		S->Add(String(LogFont->lfFaceName));
	return 1;
}

IMPL_DYN_CLASS(CMonitor)
CMonitor::CMonitor():Handle(0),MonitorNum(0){
}

CMonitor::~CMonitor(){
}

INT CMonitor::GetLeft(){
	return GetBoundsRect().left;
}

INT CMonitor::GetHeight(){
	TRect r = GetBoundsRect();
	return r.bottom - r.top;
}

INT CMonitor::GetTop(){
	return GetBoundsRect().top;
}

INT CMonitor::GetWidth(){
	TRect r = GetBoundsRect();
	return r.right - r.left;
}

TRect CMonitor::GetBoundsRect(){
	MONITORINFO MonInfo;
	MonInfo.cbSize = sizeof(MonInfo);
	GetMonitorInfo(Handle, &MonInfo);
	return *((PRect)&(MonInfo.rcMonitor));
}

TRect CMonitor::GetWorkareaRect(){
	MONITORINFO MonInfo;
	MonInfo.cbSize = sizeof(MonInfo);
	GetMonitorInfo(Handle, &MonInfo);
	return *((PRect)&(MonInfo.rcWork));
}

BOOL CMonitor::GetPrimary(){
	MONITORINFO MonInfo;
	MonInfo.cbSize = sizeof(MonInfo);
	GetMonitorInfo(Handle, &MonInfo);
	return (MonInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;
}


IMPL_DYN_CLASS(CScreen)
CScreen::CScreen(CComponent* AOwner) : CComponent(AOwner),
	Fonts(NULL),
	Imes(NULL),
	DefaultIme(NULL),
	Cursor(crDefault),
	CursorCount(0),
	CursorList(NULL),
	DefaultCursor(0),
	ActiveControl(NULL),
	ActiveCustomForm(NULL),
	ActiveForm(NULL),
	LastActiveControl(NULL),
	LastActiveCustomForm(NULL),
	FocusedForm(NULL),
	SaveFocusedList(NULL),
	AlignLevel(0),
	ControlState(0),
	INIT_EVENT(ActiveControlChange),
	INIT_EVENT(ActiveFormChange)
	{
	//TODO Classes.AddDataModule = AddDataModule;
	//Classes.RemoveDataModule = RemoveDataModule;
	CreateCursors();
	DefaultKbLayout = GetKeyboardLayout(0);
	Forms = new CList();
	CustomForms = new CList();
	DataModules = new CList();
	Monitors = new CList();
	SaveFocusedList = new CList();
	HDC DC = GetDC(0);
	PixelsPerInch = GetDeviceCaps(DC, LOGPIXELSY);
	ReleaseDC(0, DC);
	EnumDisplayMonitors(0, NULL, (MONITORENUMPROC)&EnumMonitorsProc, (LPARAM)Monitors);
	IconFont = new CFont();
	MenuFont = new CFont();
	HintFont = new CFont();
	GetMetricSettings();
	IconFont->SetOnChange(this, (TNotifyEvent)&CScreen::IconFontChanged);
	MenuFont->SetOnChange(this, (TNotifyEvent)&CScreen::IconFontChanged);
	HintFont->SetOnChange(this, (TNotifyEvent)&CScreen::IconFontChanged);
}

CScreen::~CScreen(){
	delete HintFont;
	delete MenuFont;
	delete IconFont;
	delete DataModules;
	delete CustomForms;
	delete Forms;
	if(Fonts != NULL)
		delete Fonts;
	if(Imes != NULL) 
		delete Imes;
	if(DefaultIme != NULL)
		delete DefaultIme;
	delete SaveFocusedList;
	if(Monitors != NULL){
		INT Count = Monitors->GetCount();
		for(INT I = 0; I < Count; I++){
			delete (CMonitor *)(Monitors->Get(I));
		}
	}
	delete Monitors;
	DestroyCursors();
	//Classes.AddDataModule = NULL;
	//Classes.RemoveDataModule = NULL;
}

void CScreen::AlignForm(CForm* AForm){
	if(AlignLevel != 0)
		ControlState |= csAlignmentNeeded;
	else {
		TRect Rect = {0};
		DisableAlign();
		__try{
			SystemParametersInfo(SPI_GETWORKAREA, 0, &Rect, 0);
			AlignForms(AForm, Rect);
		}
		__finally{
			ControlState &= ~csAlignmentNeeded;
			EnableAlign();
		}
	}
}

BOOL CScreen::AlignWork(){
	INT Count = GetCustomFormCount();
    for(INT I = Count - 1; I >= 0; I--){
		CForm* Form = (CForm *)CustomForms->Get(I);
		if(Form->GetParent() == NULL && !IN_TEST(csDesigning, GetComponentState()) &&
			Form->GetAlign() != alNone && Form->GetVisible() && Form->GetWindowState() != wsMinimized)
			return TRUE;
	}
	return FALSE;
}

BOOL CScreen::InsertBefore(CForm* C1, CForm* C2, TAlign AAlign){
	switch(AAlign){
		case alTop: return C1->GetTop() < C2->GetTop();
		case alBottom: return (C1->GetTop() + C1->GetHeight()) > (C2->GetTop() + C2->GetHeight());
		case alLeft: return C1->GetLeft() < C2->GetLeft();
		case alRight: return (C1->GetLeft() + C1->GetWidth()) > (C2->GetLeft() + C2->GetWidth());
	}
	return FALSE;
}

void CScreen::DoPosition(CForm* Form, TAlign AAlign, TRect& Rect){
	//NewLeft, NewTop, NewWidth, NewHeight: Integer;
	INT NewWidth = Rect.right - Rect.left;
	if(NewWidth < 0 || (AAlign == alLeft || AAlign == alRight))
		NewWidth = Form->GetWidth();
	INT NewHeight = Rect.bottom - Rect.top;
	if(NewHeight < 0 || (AAlign == alTop || AAlign == alBottom))
		NewHeight = Form->GetHeight();
	INT NewLeft = 0;
	INT NewTop = 0;
	if(AAlign == alTop && Form->GetWindowState() == wsMaximized){
		NewLeft = Form->GetLeft();
        NewTop = Form->GetTop();
        NewWidth = GetSystemMetrics(SM_CXMAXIMIZED);
	}
	else {
		NewLeft = Rect.left;
		NewTop = Rect.top;
	}
	switch(AAlign){
	case alTop: 
		Rect.top += NewHeight;
		break;
	case alBottom:
		Rect.bottom -= NewHeight;
		NewTop = Rect.bottom;
		break;
	case alLeft:
		Rect.left += NewWidth;
		break;
	case alRight:
		Rect.right -= NewWidth;
		NewLeft = Rect.right;
		break;
	}
	Form->SetBounds(NewLeft, NewTop, NewWidth, NewHeight);
	if(Form->GetWindowState() == wsMaximized){
		NewWidth -= NewLeft;
		NewHeight -= NewTop;
	}
    //Adjust client rect if Form didn't resize as we expected 
	if(Form->GetWidth() != NewWidth || Form->GetHeight() != NewHeight){
		switch(AAlign){
		case alTop:
			Rect.top -= NewHeight - Form->GetHeight();
			break;
		case alBottom:
			Rect.bottom += NewHeight - Form->GetHeight();
			break;
		case alLeft: 
			Rect.left -= NewWidth - Form->GetWidth();
			break;
		case alRight:
			Rect.right += NewWidth - Form->GetWidth();
			break;
		case alClient:
			Rect.right += NewWidth - Form->GetWidth();
			Rect.bottom += NewHeight - Form->GetHeight();
		}
	}
}

void CScreen::DoAlign(CList* AlignList, CForm* AForm, TAlign AAlign, TRect& Rect){
	AlignList->Clear();
	if(AForm != NULL && AForm->GetParent() == NULL && !IN_TEST(csDesigning, AForm->GetComponentState()) &&
		AForm->GetVisible() && AForm->GetAlign() == AAlign && 
		AForm->GetWindowState() != wsMinimized)
		AlignList->Add(AForm);
	INT Count = GetCustomFormCount();
	for(INT I = 0; I < Count; I++){
		CForm* Form = (CForm *)CustomForms->Get(I);
		if(Form->GetParent() == NULL && Form->GetAlign() == AAlign && 
			!IN_TEST(csDesigning, Form->GetComponentState()) && 
			Form->GetVisible() && Form->GetWindowState() != wsMinimized){
			if(Form == AForm)
				continue;
			INT J = 0;
			while (J < AlignList->GetCount() && !InsertBefore(Form, (CForm *)AlignList->Get(J), AAlign))
				J++;
			AlignList->Insert(J, Form);
		}
	}
	Count = AlignList->GetCount();
	for(INT I = 0; I < Count; I++)
		DoPosition((CForm *)AlignList->Get(I), AAlign, Rect);
}

void CScreen::AlignForms(CForm* AForm, TRect& Rect){
	if(AlignWork()){
		CList* AlignList = new CList();
		CObjectHolder alHolder(AlignList);
		DoAlign(AlignList, AForm, alTop, Rect);
		DoAlign(AlignList, AForm, alBottom, Rect);
		DoAlign(AlignList, AForm, alLeft, Rect);
		DoAlign(AlignList, AForm, alRight, Rect);
		DoAlign(AlignList, AForm, alClient, Rect);
	}
}

//void CScreen::AddDataModule(CDataModule* DataModule){}
void CScreen::CreateCursors(){
	LPTSTR CursorMap[] = {
		IDC_SIZEALL, IDC_HANDPT, IDC_HELP, IDC_APPSTARTING, IDC_NO, IDC_SQLWAIT,
		IDC_MULTIDRAG, IDC_VSPLIT, IDC_HSPLIT, IDC_NODROP, IDC_DRAG, IDC_WAIT,
		IDC_UPARROW, IDC_SIZEWE, IDC_SIZENWSE, IDC_SIZENS, IDC_SIZENESW, IDC_SIZEALL,
		IDC_IBEAM, IDC_CROSS, IDC_ARROW};
	DefaultCursor = LoadCursor(0, IDC_ARROW);
	INT LEN = sizeof(CursorMap) / sizeof(CursorMap[0]);
	HINSTANCE MainInstance = GetGlobal().GetMainInstance();
	for(INT I = 0; I < LEN; I++){
		TCursor Cur = I + crSizeAll;
		HINSTANCE Instance = 0;
		if((Cur >= crSQLWait && Cur <= crDrag) || (Cur == crHandPoint))
			Instance = GetGlobal().GetHInstance();
		HCURSOR hCursor = LoadCursor(Instance, CursorMap[I]);
		if(hCursor == 0 && Instance != 0 && Instance != MainInstance)
			hCursor = LoadCursor(MainInstance, CursorMap[I]);
		if(hCursor == 0 && Instance != 0)
			hCursor = LoadCursor(0, CursorMap[I]);
		InsertCursor(Cur, hCursor);
	}
}

void CScreen::DeleteCursor(INT Index){
	PCursorRec P = CursorList;
	PCursorRec Q = NULL;
	while(P != NULL && P->Index != Index){
		Q = P;
		P = P->Next;
	}
	if(P != NULL){
		DestroyCursor(P->Handle);
		if(Q == NULL)
			CursorList = P->Next;
		else
			Q->Next = P->Next;
		free(P);
	}
}

void CScreen::DestroyCursors(){
	PCursorRec P = CursorList;
	while(P != NULL){
		if((P->Index >= crSQLWait && P->Index <= crDrag) ||
			P->Index == crHandPoint || P->Index > 0)
			DestroyCursor(P->Handle);
		PCursorRec Next = P->Next;
		free(P);
		P = Next;
	};
	HCURSOR Hdl = LoadCursor(0, IDC_ARROW);
	if(Hdl != DefaultCursor)
		DestroyCursor(DefaultCursor);
}

CMonitor* CScreen::FindMonitor(HMONITOR Handle){
	CMonitor* Result = NULL;
	INT Count = GetMonitorCount();
	for(INT I = 0; I < Count; I++){
		CMonitor* M = (CMonitor *)Monitors->Get(I);
		if(M->GetHandle() == Handle){
			Result = M;
			break;
		}
	}
	return Result;
}

void CScreen::IconFontChanged(CObject* Sender){
	/* TODO Application.NotifyForms(CM_SYSFONTCHANGED);
	if(Sender == HintFont && Application != NULL && Application->GetShowHint()){
		Application->SetShowHint(FALSE);
		Application->SetShowHint(TRUE);
	}
	//*/
}

INT CScreen::GetCustomFormCount(){
	return CustomForms->GetCount();
}

CForm* CScreen::GetCustomForms(INT Index){
	return (CForm*)CustomForms->Get(Index);
}

HCURSOR CScreen::GetCursors(INT Index){
	HCURSOR Result = 0;
	if(Index != crNone){
		PCursorRec P = CursorList;
		while(P != NULL && P->Index != Index)
			P = P->Next;
		if(P == NULL)
			Result = DefaultCursor;
		else 
			Result = P->Handle;
	}
	return Result;
}

//TODO CDataModule* CScreen::GetDataModule(INT Index){}

INT CScreen::GetDataModuleCount(){
	return DataModules->GetCount();
}

String CScreen::GetDefaultIME(){
	GetImes();  // load Ime list, find default
	return *DefaultIme;
}

INT CScreen::GetDesktopTop(){
	return GetSystemMetrics(SM_YVIRTUALSCREEN);
}

INT CScreen::GetDesktopLeft(){
	return GetSystemMetrics(SM_XVIRTUALSCREEN);
}

INT CScreen::GetDesktopHeight(){
	return GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

INT CScreen::GetDesktopWidth(){
	return GetSystemMetrics(SM_CXVIRTUALSCREEN);
}

TRect CScreen::GetDesktopRect(){
	return Bounds(GetDesktopLeft(), GetDesktopTop(), GetDesktopWidth(), GetDesktopHeight());
}

TRect CScreen::GetWorkAreaRect(){
	TRect Result = {0};
	SystemParametersInfo(SPI_GETWORKAREA, 0, &Result, 0);
	return Result;
}

INT CScreen::GetWorkAreaHeight(){
	TRect Rect = GetWorkAreaRect();
	return Rect.bottom - Rect.top;
}

INT CScreen::GetWorkAreaLeft(){
	return GetWorkAreaRect().left;
}

INT CScreen::GetWorkAreaTop(){
	return GetWorkAreaRect().top;
}

INT CScreen::GetWorkAreaWidth(){
	TRect Rect = GetWorkAreaRect();
	return Rect.right - Rect.left;
}

CStrings* CScreen::GetImes(){
	LPTSTR KbLayoutRegkeyFmt = TEXT("System\\CurrentControlSet\\Control\\Keyboard Layouts\\%.8x");  // do not localize
	LPTSTR KbLayoutRegSubkey = TEXT("layout text"); // do not localize
	HKL KbList[64] = {0};
	if(Imes == NULL){
		Imes = new CStringList();
		LPTSTR DefIme = TEXT("");
		INT TotalKbLayout = GetKeyboardLayoutList(64, KbList);
		INT Len = lstrlen(KbLayoutRegkeyFmt) + 32;
		INT Size = (Len + 1) * sizeof(TCHAR);
		LPTSTR Buf = (LPTSTR)malloc(Size);
		BufferHolder bufHolder(Buf);
		ZeroMemory(Buf, Size);
		TCHAR ImeFileName[256] = {0};
		HKEY qKey;
		for(INT I = 0; I < TotalKbLayout; I++){
			if(ImmIsIME(KbList[I])){
				_snwprintf_s_l(Buf, Size, Len, KbLayoutRegkeyFmt, NULL, KbList[I]);
				if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, Buf, 0, KEY_READ, &qKey) == ERROR_SUCCESS){
					HKeyHolder keyHolder(qKey);
					DWORD Bufsize = sizeof(ImeFileName);
					if(RegQueryValueEx(qKey, KbLayoutRegSubkey, NULL, NULL, (LPBYTE)ImeFileName, &Bufsize) == ERROR_SUCCESS){
						Imes->AddObject(String(ImeFileName), (CObject *)KbList[I]);
						if(KbList[I] == DefaultKbLayout)
							DefIme = ImeFileName;
					}
				}
			}
		}
		DefaultIme = new String(DefIme);
		((CStringList *)Imes)->SetDuplicates(dupIgnore);
		((CStringList *)Imes)->SetSorted(TRUE);
	}
	return Imes;
}

INT CScreen::GetHeight(){
	return GetSystemMetrics(SM_CYSCREEN);
}

CMonitor* CScreen::GetMonitor(INT Index){
	return (CMonitor*)Monitors->Get(Index);
}

INT CScreen::GetMonitorCount(){
	if(Monitors->GetCount() == 0)
		return GetSystemMetrics(SM_CMONITORS);
	else
		return Monitors->GetCount();
}

CStrings* CScreen::GetFonts(){
	if(Fonts = NULL){
		Fonts = new CStringList();
		HDC DC = GetDC(0);
		DCHolder dcHolder(DC, 0, 0);
		Fonts->Add(String(TEXT("Default")));
		LOGFONT LFont;
		ZeroMemory(&LFont, sizeof(LFont));
		LFont.lfCharSet = DEFAULT_CHARSET;
		EnumFontFamiliesEx(DC, &LFont, (FONTENUMPROC)&EnumFontsProc, (LPARAM)Fonts, 0);
		((CStringList *)Fonts)->SetSorted(TRUE);
	}
	return Fonts;
}

CForm* CScreen::GetForm(INT Index){
	return (CForm*)Forms->Get(Index);
}

INT CScreen::GetFormCount(){
	return Forms->GetCount();
}

void CScreen::GetMetricSettings(){
	BOOL SaveShowHint = FALSE;
	//if(Application != NULL)
	//	SaveShowHint = Application->GetShowHint();
	__try{
		//TODO if(Application != NULL)
		//TODO 	Application->SetShowHint(FALSE);
		LOGFONT LFont;
		ZeroMemory(&LFont, sizeof(LFont));
		if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LFont), &LFont, 0))
			IconFont->SetHandle(CreateFontIndirect(&LFont));
		else
			IconFont->SetHandle((HFONT)GetStockObject(SYSTEM_FONT));
		NONCLIENTMETRICS NonClientMetrics;
		ZeroMemory(&NonClientMetrics, sizeof(NonClientMetrics));
		NonClientMetrics.cbSize = sizeof(NonClientMetrics);
		if(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &NonClientMetrics, 0)){
			HintFont->SetHandle(CreateFontIndirect(&NonClientMetrics.lfStatusFont));
			MenuFont->SetHandle(CreateFontIndirect(&NonClientMetrics.lfMenuFont));
		}
		else{
			HintFont->SetSize(8);
			MenuFont->SetHandle((HFONT)GetStockObject(SYSTEM_FONT));
		}
		HintFont->SetColor(clInfoText);
		MenuFont->SetColor(clMenuText);
	}
	__finally{
		//TODO if(Application != NULL)
		//TODO 	Application->SetShowHint(SaveShowHint);
	}
}

INT CScreen::GetWidth(){
	return GetSystemMetrics(SM_CXSCREEN);
}

void CScreen::InsertCursor(INT Index, HCURSOR Handle){
	PCursorRec P = (PCursorRec)malloc(sizeof(TCursorRec));
	P->Next = CursorList;
	P->Index = Index;
	P->Handle = Handle;
	CursorList = P;
}

//TODO void CScreen::RemoveDataModule(CDataModule* DataModule){}

void CScreen::SetCursors(INT Index, HCURSOR Handle){
	if(Index == crDefault)
		if(Handle == 0)
			DefaultCursor = LoadCursor(0, IDC_ARROW);
		else
			DefaultCursor = Handle;
	else if(Index != crNone){
		DeleteCursor(Index);
		if(Handle != 0)
			InsertCursor(Index, Handle);
	}
}

void CScreen::SetCursor(TCursor Value){
	if(Value != Cursor){
		Cursor = Value;
		if(Value == crDefault){
			//Reset the cursor to the default by sending a WM_SETCURSOR to the window under the cursor
			TPoint P = {0,0};
			GetCursorPos(&P);
			HWND Handle = WindowFromPoint(P);
			if(Handle != 0 && GetWindowThreadProcessId(Handle, NULL) == GetCurrentThreadId()){
				LRESULT Code = SendMessage(Handle, WM_NCHITTEST, 0, (LPARAM)PointToSmallPoint(P));
				SendMessage(Handle, WM_SETCURSOR, (WPARAM)Handle, MAKELONG(Code, WM_MOUSEMOVE));
				return ;
			}
		}
		::SetCursor(GetCursors(Value));
	}
	CursorCount++;
}

void CScreen::SetHintFont(CFont* Value){
	HintFont->Assign(Value);
}

void CScreen::SetIconFont(CFont* Value){
	IconFont->Assign(Value);
}

void CScreen::SetMenuFont(CFont* Value){
	MenuFont->Assign(Value);
}

void CScreen::UpdateLastActive(){
	if(LastActiveCustomForm != ActiveCustomForm){
		LastActiveCustomForm = ActiveCustomForm;
		if(OnActiveFormChange != NULL)
			CALL_EVENT(ActiveFormChange)(this);
	}
	if(LastActiveControl != ActiveControl){
		LastActiveControl = ActiveControl;
		if(OnActiveControlChange != NULL)
			CALL_EVENT(ActiveControlChange)(this);
	}
}

void CScreen::DisableAlign(){
	AlignLevel++;
}

void CScreen::EnableAlign(){
	AlignLevel--;
	if(AlignLevel == 0 && IN_TEST(csAlignmentNeeded, ControlState))
		Realign();
}

DWORD MonitorDefaultFlags[] = {MONITOR_DEFAULTTONEAREST,
	MONITOR_DEFAULTTONULL, MONITOR_DEFAULTTOPRIMARY};
CMonitor* CScreen::MonitorFromPoint(TPoint& Point, TMonitorDefaultTo MonitorDefault){
	return FindMonitor(::MonitorFromPoint(Point, MonitorDefaultFlags[MonitorDefault]));
}

CMonitor* CScreen::MonitorFromRect(TRect& Rect, TMonitorDefaultTo MonitorDefault){
	return FindMonitor(::MonitorFromRect((LPRECT)&Rect, MonitorDefaultFlags[MonitorDefault]));
}

CMonitor* CScreen::MonitorFromWindow(HWND Handle, TMonitorDefaultTo MonitorDefault){
	return FindMonitor(::MonitorFromWindow(Handle, MonitorDefaultFlags[MonitorDefault]));
}

void CScreen::Realign(){
	AlignForm(NULL);
}

void CScreen::ResetFonts(){
	FreeAndNil((CObject **)&Fonts);
}

void CScreen::AddForm(CForm* AForm){
	CustomForms->Add(AForm);
	if(AForm->InstanceOf(CForm::_Class)){
		Forms->Add(AForm);
		GetGlobal().GetApplication()->UpdateVisible();
	}
}

void CScreen::RemoveForm(CForm* AForm){
	CustomForms->Remove(AForm);
	Forms->Remove(AForm);
	GetGlobal().GetApplication()->UpdateVisible();
	//TODO if(CustomForms->GetCount() == 0 && GetGlobal().GetApplication()->GetHintWindow() != NULL)
	//TODO	GetGlobal().GetApplication()->GetHintWindow()->ReleaseHandle();
}
