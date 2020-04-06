#include "stdinc.h"
#include "SysInit.hpp"
#include "List.hpp"
#include "Graphics.hpp"
#include "WinUtils.hpp"

COLORREF ColorToRGB(TColor Color){
	if((INT)Color < 0)
		return GetSysColor(Color & 0x000000FF);
	else
		return Color;
}

class GraphicsGlobal {
public:
	HPEN StockPen;
	HBRUSH StockBrush;
	HFONT StockFont;
	HICON StockIcon;
	INT ScreenLogPixels;
	HPALETTE SystemPalette16;
	RTL_CRITICAL_SECTION CounterLock;
	RTL_CRITICAL_SECTION BitmapImageLock;
	BOOL DDBsOnly;
	GraphicsGlobal();
	virtual ~GraphicsGlobal();
};

#pragma pack (1)
typedef struct {
    WORD palVersion;
    WORD palNumEntries;
	PALETTEENTRY palPalEntry[256];
}TMaxLogPalette, *PMaxLogPalette;
#pragma pack ()


HPALETTE CreateSystemPalette(const TColor Entries[]){
	TMaxLogPalette Pal;
	Pal.palVersion = 0x300;
	Pal.palNumEntries = 16;
	CopyMemory(Pal.palPalEntry, Entries, 16 * sizeof(TColor));
	HDC DC = GetDC(0);
	__try{
		INT SysPalSize = GetDeviceCaps(DC, SIZEPALETTE);
		/*{ Ignore the disk image of the palette for 16 color bitmaps.
			Replace with the first and last 8 colors of the system palette //*/
		if(SysPalSize >= 16){
			GetSystemPaletteEntries(DC, 0, 8, Pal.palPalEntry);
			//{ Is light and dark gray swapped? }
			if(*((PColor)&(Pal.palPalEntry[7])) == clSilver){
				GetSystemPaletteEntries(DC, SysPalSize - 8, 1, &(Pal.palPalEntry[7]));
				GetSystemPaletteEntries(DC, SysPalSize - 7, 7, &(Pal.palPalEntry[Pal.palNumEntries - 7]));
				GetSystemPaletteEntries(DC, 7, 1, &(Pal.palPalEntry[8]));
			}
			else
				GetSystemPaletteEntries(DC, SysPalSize - 8, 8, &(Pal.palPalEntry[Pal.palNumEntries - 8]));
		}
		else{}
	}
	__finally{
		ReleaseDC(0,DC);
	}
	return CreatePalette((LOGPALETTE *)&Pal);
}

GraphicsGlobal::GraphicsGlobal(){
	HDC DC = GetDC(0);
	ScreenLogPixels = GetDeviceCaps(DC, LOGPIXELSY);
	ReleaseDC(0,DC);
	StockPen = (HPEN)GetStockObject(BLACK_PEN);
	StockBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	StockFont = (HFONT)GetStockObject(SYSTEM_FONT);
	StockIcon = LoadIcon(0, IDI_APPLICATION);
	TColor Pal16[] = {clBlack, clMaroon, clGreen, clOlive, clNavy, clPurple, clTeal, clDkGray,
			clLtGray, clRed, clLime, clYellow, clBlue, clFuchsia, clAqua, clWhite};
	SystemPalette16 = CreateSystemPalette(Pal16);
	InitializeCriticalSection(&CounterLock);
	InitializeCriticalSection(&BitmapImageLock);
	DDBsOnly = FALSE;
}

GraphicsGlobal::~GraphicsGlobal(){
	DeleteCriticalSection(&BitmapImageLock);
	DeleteCriticalSection(&CounterLock);
	DeleteObject(SystemPalette16);
}

GraphicsGlobal GGlobal;

TFontData DefFontData = {
	0,
	0, 
	fpDefault,
	0,
	DEFAULT_CHARSET,
	TEXT("Arial")
};

CResourceManager FontManager(sizeof(TFontData));


IMPL_DYN_CLASS(CGraphicsObject)
CGraphicsObject::CGraphicsObject():OwnerLock(NULL),
	Resource(NULL),
	INIT_EVENT(Change){
}

CGraphicsObject::~CGraphicsObject(){
}

void CGraphicsObject::Changed(){
	if(OnChange != NULL){
		CALL_EVENT(Change)(this);
	}
}

void CGraphicsObject::Lock(){
	if(OwnerLock != NULL)
		EnterCriticalSection(OwnerLock);
}

void CGraphicsObject::Unlock(){
	if(OwnerLock != NULL)
		LeaveCriticalSection(OwnerLock);
}

BOOL CGraphicsObject::HandleAllocated(){
	return (Resource != NULL) && (Resource->Handle != 0);
}

PRTL_CRITICAL_SECTION CGraphicsObject::GetOwnerCriticalSection(){
	return OwnerLock;
}

void CGraphicsObject::SetOwnerCriticalSection(PRTL_CRITICAL_SECTION Value){
	OwnerLock = Value;
}

IMPL_DYN_CLASS(CFont)
void CFont::GetData(TFontData& FontData){
	FontData = Resource->Font;
	FontData.Handle = 0;
}

void CFont::SetData(const TFontData& FontData){
	CMethodLock Lock(this, (TLockMethod)&CGraphicsObject::Lock, (TLockMethod)&CGraphicsObject::Unlock);
	FontManager.ChangeResource(this, (LPVOID)&FontData);
}

void CFont::Changed(){
	__super::Changed();
	//if(FNotify != NULL)
	//	FNotify->Changed();
}

CFont::CFont(){
	DefFontData.Handle = 0;
	Resource = FontManager.AllocResource((LPVOID)&DefFontData);
	Color = clWindowText;
	PixelsPerInch = GGlobal.ScreenLogPixels;
}

CFont::~CFont(){
	FontManager.FreeResource(Resource);
}

void CFont::Assign(CFont* Source){
	CMethodLock ThisLock(this, (TLockMethod)&CGraphicsObject::Lock, (TLockMethod)&CGraphicsObject::Unlock);
	{
		CMethodLock SourceLock(Source, (TLockMethod)&CGraphicsObject::Lock, (TLockMethod)&CGraphicsObject::Unlock);
		FontManager.AssignResource(this, Source->Resource);
        SetColor(Source->GetColor());
		if(GetPixelsPerInch() != Source->GetPixelsPerInch())
			SetSize(Source->GetSize());
	}
}

HFONT CFont::GetHandle(){
	LOGFONT LogFont;
	if(Resource->Handle == 0){
		CMethodLock FMLock(&FontManager, (TLockMethod)&CResourceManager::Lock, (TLockMethod)&CResourceManager::Unlock);
		if(Resource->Handle == 0){
			LogFont.lfHeight = Resource->Font.Height;
			LogFont.lfWidth = 0; //have font mapper choose
			LogFont.lfEscapement = 0; //only straight fonts
			LogFont.lfOrientation = 0; //no rotation
			if(IN_TEST(fsBold, Resource->Font.Style)){
				LogFont.lfWeight = FW_BOLD;
			}
			else{
				LogFont.lfWeight = FW_NORMAL;
			}
			LogFont.lfItalic = IN_TEST(fsItalic, Resource->Font.Style);
			LogFont.lfUnderline = IN_TEST(fsUnderline, Resource->Font.Style);
			LogFont.lfStrikeOut = IN_TEST(fsStrikeOut, Resource->Font.Style);
			LogFont.lfCharSet = Resource->Font.Charset;
			if(::lstrcmpi(Resource->Font.Name, TEXT("Default")) == 0){
				lstrcpyn(LogFont.lfFaceName, DefFontData.Name, sizeof(DefFontData.Name) / sizeof(TCHAR));
			}
			else{
				lstrcpyn(LogFont.lfFaceName, Resource->Font.Name, sizeof(Resource->Font.Name) / sizeof(TCHAR));
			}
			LogFont.lfQuality  = DEFAULT_QUALITY;

			//Everything else as default
			LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
			LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			switch(Resource->Font.Pitch){
				case fpVariable: LogFont.lfPitchAndFamily = VARIABLE_PITCH; break;
				case fpFixed: LogFont.lfPitchAndFamily = FIXED_PITCH; break;
				default: LogFont.lfPitchAndFamily = DEFAULT_PITCH;
			}
			Resource->Handle = CreateFontIndirect(&LogFont);
		}
	}
	return (HFONT)(Resource->Handle);
}

TFontData GetFontData(HFONT Font){
	LOGFONT LogFont;
	TFontData Result = DefFontData;
	if(Font != 0){
		if(GetObject(Font, sizeof(LogFont), &LogFont) != 0){
			Result.Height = LogFont.lfHeight;
			if(LogFont.lfWeight >= FW_BOLD)
				Result.Style |= fsBold;
			if(LogFont.lfItalic == 1)
				Result.Style |= fsItalic;
			if(LogFont.lfUnderline == 1)
				Result.Style |= fsUnderline;
			if(LogFont.lfStrikeOut == 1)
				Result.Style |= fsStrikeOut;
			Result.Charset = LogFont.lfCharSet;
			lstrcpyn(Result.Name, LogFont.lfFaceName, sizeof(Result.Name) / sizeof(TCHAR));
			switch(LogFont.lfPitchAndFamily & 0xF){
				case VARIABLE_PITCH: Result.Pitch = fpVariable; break;
				case FIXED_PITCH: Result.Pitch = fpFixed; break;
				default:Result.Pitch = fpDefault;
			}
			Result.Handle = Font;
		}
	}
	return Result;
}

void CFont::SetHandle(HFONT Value){
	SetData(GetFontData(Value));
}

BYTE CFont::GetCharset(){
	return Resource->Font.Charset;
}

void CFont::SetCharset(BYTE Value){
	TFontData FontData;
	GetData(FontData);
	FontData.Charset = Value;
	SetData(FontData);
}

void CFont::SetColor(TColor Value){
	if(Color != Value){
		Color = Value;
		Changed();
	}
}

INT CFont::GetHeight(){
	return Resource->Font.Height;
}

void CFont::SetHeight(INT Value){
	TFontData FontData;
	GetData(FontData);
	FontData.Height = Value;
	SetData(FontData);
}

LPTSTR CFont::GetName(){
	return Resource->Font.Name;
}

void CFont::SetName(const LPTSTR Value){
	if(Value != NULL && ::lstrcmpi(Value, TEXT("")) == 0){
		TFontData FontData;
		GetData(FontData);
		lstrcpyn(FontData.Name, Value, sizeof(FontData.Name) / sizeof(TCHAR));
		SetData(FontData);
	}
}

BYTE CFont::GetPitch(){
	return Resource->Font.Pitch;
}

void CFont::SetPitch(BYTE Value){
	TFontData FontData;
	GetData(FontData);
	FontData.Pitch = Value;
	SetData(FontData);
}

INT CFont::GetSize(){
	return -(GetHeight() * 72 / PixelsPerInch);
}

void CFont::SetSize(BYTE Value){
	SetHeight(-(Value * PixelsPerInch / 72));
}

BYTE CFont::GetStyle(){
	return Resource->Font.Style;
}

void CFont::SetStyle(BYTE Value){
	TFontData FontData;
	GetData(FontData);
	FontData.Style = Value;
	SetData(FontData);
}

TPenData DefPenData = {
    0,
    clBlack,
    1,
    psSolid
};

CResourceManager PenManager(sizeof(TPenData));

IMPL_DYN_CLASS(CPen)
void CPen::GetData(TPenData& PenData){
	PenData = Resource->Pen;
	PenData.Handle = 0;
}

void CPen::SetData(const TPenData& PenData){
	CMethodLock FMLock(&PenManager, (TLockMethod)&CResourceManager::Lock, (TLockMethod)&CResourceManager::Unlock);
    PenManager.ChangeResource(this, (LPVOID)(&PenData));
}

CPen::CPen(){
	Resource = PenManager.AllocResource((LPVOID)(&DefPenData));
	Mode = pmCopy;
}

CPen::~CPen(){
	PenManager.FreeResource(Resource);
}

void CPen::Assign(CPen* Source){
	CMethodLock ThisLock(this, (TLockMethod)&CGraphicsObject::Lock, (TLockMethod)&CGraphicsObject::Unlock);
	{
		CMethodLock SourceLock(Source, (TLockMethod)&CGraphicsObject::Lock, (TLockMethod)&CGraphicsObject::Unlock);
		PenManager.AssignResource(this, Source->Resource);
        SetMode(Source->Mode);
	}
}

TColor CPen::GetColor(){
	return Resource->Pen.Color;
}

void CPen::SetColor(TColor Value){
	TPenData PenData;
	GetData(PenData);
	PenData.Color = Value;
	SetData(PenData);
}

HPEN CPen::GetHandle(){
	WORD PenStyles[] = {
		PS_SOLID, PS_DASH, PS_DOT, PS_DASHDOT, PS_DASHDOTDOT, PS_NULL, PS_INSIDEFRAME
	};
	LOGPEN LogPen;
	if(Resource->Handle == 0){
		CMethodLock FMLock(&PenManager, (TLockMethod)&CResourceManager::Lock, (TLockMethod)&CResourceManager::Unlock);
		if(Resource->Handle == 0){
			LogPen.lopnStyle = PenStyles[Resource->Pen.Style];
			LogPen.lopnWidth.x = Resource->Pen.Width;
			LogPen.lopnColor = ColorToRGB(Resource->Pen.Color);
			Resource->Handle = CreatePenIndirect(&LogPen);
		}
	}
	return (HPEN)(Resource->Handle);
}

void CPen::SetHandle(HPEN Value){
	TPenData PenData = DefPenData;
	PenData.Handle = Value;
	SetData(PenData);
}

SHORT CPen::GetMode(){
	return Mode;
}

void CPen::SetMode(SHORT Value){
	if(Mode != Value){
		Mode = Value;
		Changed();
	}
}

BYTE CPen::GetStyle(){
	return Resource->Pen.Style;
}

void CPen::SetStyle(BYTE Value){
	TPenData PenData;
	GetData(PenData);
	PenData.Style = Value;
	SetData(PenData);
}

INT CPen::GetWidth(){
	return Resource->Pen.Width;
}

void CPen::SetWidth(INT Value){
	if(Value != 0){
		TPenData PenData;
		GetData(PenData);
		PenData.Width = Value;
		SetData(PenData);
	}
}

TBrushData DefBrushData = {
    0,
    clWhite,
	NULL,
	bsSolid
};

CResourceManager BrushManager(sizeof(TBrushData));

IMPL_DYN_CLASS(CBrush)
void CBrush::GetData(TBrushData& BrushData){
	  BrushData = Resource->Brush;
	  BrushData.Handle = 0;
	  BrushData.Bitmap = NULL;
}

void CBrush::SetData(const TBrushData& BrushData){
	CMethodLock FMLock(&BrushManager, (TLockMethod)&CResourceManager::Lock, (TLockMethod)&CResourceManager::Unlock);
	BrushManager.ChangeResource(this, (LPVOID)(&BrushData));
}

CBrush::CBrush(){
	Resource = BrushManager.AllocResource(&DefBrushData);
}

CBrush::~CBrush(){
	BrushManager.FreeResource(Resource);
}

void CBrush::Assign(CBrush* Source){
	CMethodLock ThisLock(this, (TLockMethod)&CGraphicsObject::Lock, (TLockMethod)&CGraphicsObject::Unlock);
	{
		CMethodLock SourceLock(Source, (TLockMethod)&CGraphicsObject::Lock, (TLockMethod)&CGraphicsObject::Unlock);
		BrushManager.AssignResource(this, Source->Resource);
	}
}

CBitmap* CBrush::GetBitmap(){
	return Resource->Brush.Bitmap;
}

void CBrush::SetBitmap(CBitmap* Value){
	TBrushData BrushData = DefBrushData;
	BrushData.Bitmap = Value;
	SetData(BrushData);
}

TColor CBrush::GetColor(){
	return Resource->Brush.Color;
}

void CBrush::SetColor(TColor Value){
	TBrushData BrushData;
	GetData(BrushData);
	BrushData.Color = Value;
	if (BrushData.Style == bsClear)
		BrushData.Style = bsSolid;
	SetData(BrushData);
}

HBRUSH CBrush::GetHandle(){
	LOGBRUSH LogBrush;
	if(Resource->Handle == 0){
		CMethodLock FMLock(&BrushManager, (TLockMethod)&CResourceManager::Lock, (TLockMethod)&CResourceManager::Unlock);
		if(Resource->Handle == 0){
			if(Resource->Brush.Bitmap != NULL){
				LogBrush.lbStyle = BS_PATTERN;
				//TODO Resource->Brush.Bitmap->SetHandleType(bmDDB);
				//TODO LogBrush.lbHatch = (ULONG_PTR)Resource->Brush.Bitmap->GetHandle();
			}
			else{
				LogBrush.lbHatch = 0;
				switch(Resource->Brush.Style){
					case bsSolid: LogBrush.lbStyle = BS_SOLID; break;
					case bsClear: LogBrush.lbStyle = BS_HOLLOW; break;
					default: 
						LogBrush.lbStyle = BS_HATCHED;
						LogBrush.lbHatch = Resource->Brush.Style - bsHorizontal;
						break;
				}
			}
			LogBrush.lbColor = ColorToRGB(Resource->Brush.Color);
			Resource->Handle = CreateBrushIndirect(&LogBrush);
		}
	}
	return (HBRUSH)(Resource->Handle);
}

void CBrush::SetHandle(HBRUSH Value){
	TBrushData BrushData = DefBrushData;
	BrushData.Handle = Value;
	SetData(BrushData);
}

UINT CBrush::GetStyle(){
	return Resource->Brush.Style;
}

void CBrush::SetStyle(UINT Value){
	TBrushData BrushData;
	GetData(BrushData);
	BrushData.Style = Value;
	if (BrushData.Style == bsClear)
		BrushData.Color = clWhite;
	SetData(BrushData);
}


IMPL_DYN_CLASS(CCanvas)
void CCanvas::CreateBrush(){
	UnrealizeObject(Brush->GetHandle());
	SelectObject(DC, Brush->GetHandle());
	if(Brush->GetStyle() == bsSolid){
		SetBkColor(DC, ColorToRGB(Brush->GetColor()));
		SetBkMode(DC, OPAQUE);
	}
	else{
		//Win95 doesn't draw brush hatches if bkcolor = brush color
		//Since bkmode is transparent, nothing should use bkcolor anyway
		SetBkColor(DC, !ColorToRGB(Brush->GetColor()));
		SetBkMode(DC, TRANSPARENT);
	}
}

void CCanvas::CreateFont(){
	SelectObject(DC, Font->GetHandle());
	SetTextColor(DC, ColorToRGB(Font->GetColor()));
}

void CCanvas::CreatePen(){
	WORD PenModes[] = {
		R2_BLACK, R2_WHITE, R2_NOP, R2_NOT, R2_COPYPEN, R2_NOTCOPYPEN, R2_MERGEPENNOT,
		R2_MASKPENNOT, R2_MERGENOTPEN, R2_MASKNOTPEN, R2_MERGEPEN, R2_NOTMERGEPEN,
		R2_MASKPEN, R2_NOTMASKPEN, R2_XORPEN, R2_NOTXORPEN
	};
	SelectObject(DC, Pen->GetHandle());
	SetROP2(DC, PenModes[Pen->GetMode()]);
}

void CCanvas::BrushChanged(CObject* ABrush){
	if(IN_TEST(csBrushValid, State)){
		State &= ~csBrushValid;
		SelectObject(DC, GGlobal.StockBrush);
	}
}

void CCanvas::DeselectHandles(){
	if((DC != 0) && (State - (csPenValid | csBrushValid | csFontValid) != State)){
		SelectObject(DC, GGlobal.StockPen);
		SelectObject(DC, GGlobal.StockBrush);
		SelectObject(DC, GGlobal.StockFont);
		State &= ~(csPenValid | csBrushValid | csFontValid);
	}
}

void CCanvas::FontChanged(CObject* AFont){
	if(IN_TEST(csFontValid , State)){
		State &= ~csFontValid;
		SelectObject(DC, GGlobal.StockFont);
	}
}

void CCanvas::PenChanged(CObject* APen){
	if(IN_TEST(csPenValid, State)){
		State &= ~csPenValid;
		SelectObject(DC, GGlobal.StockPen);
	}
}

void CCanvas::Changed(){
	if(OnChange != NULL){
		CALL_EVENT(Change)(this);
	}
}

void CCanvas::Changing(){
	if(OnChanging != NULL){
		CALL_EVENT(Changing)(this);
	}
}

void CCanvas::CreateHandle(){

}

void CCanvas::RequiredState(TCanvasStates ReqState){
	TCanvasStates NeededState = ReqState & (~State);
	if(NeededState != 0){
		if(IN_TEST(csHandleValid, NeededState)){
			CreateHandle();
			if(DC == 0)
				throw "Canvas does not allow drawing";// raise EInvalidOperation.CreateRes(@SNoCanvasHandle);
		}
		if(IN_TEST(csFontValid, NeededState))
			CreateFont();
		if(IN_TEST(csPenValid, NeededState)) 
			CreatePen();
		if(IN_TEST(csBrushValid, NeededState))
			CreateBrush();
		State |= NeededState;
	}
}

CThreadList CanvasList;

CCanvas::CCanvas() : CObject(),
	DC(0),
	State(0),
	CopyMode(cmSrcCopy),
	LockCount(0),
	TextFlags(0),
	INIT_EVENT(Change),
	INIT_EVENT(Changing){
	InitializeCriticalSection(&LockSection);
	Font = new CFont();
	Font->SetOnChange(this, (TNotifyEvent)&CCanvas::FontChanged);
	Font->SetOwnerCriticalSection(&LockSection);
	Pen = new CPen();
	Pen->SetOnChange(this, (TNotifyEvent)&CCanvas::PenChanged);
	Pen->SetOwnerCriticalSection(&LockSection);
	Brush = new CBrush();
	Brush->SetOnChange(this, (TNotifyEvent)&CCanvas::BrushChanged);
	Brush->SetOwnerCriticalSection(&LockSection);
	CanvasList.Add(this);
	PenPos.x = 0;
	PenPos.y = 0;
}

CCanvas::~CCanvas(){
	CanvasList.Remove(this);
	SetHandle(0);
	delete Font;
	delete Pen;
	delete Brush;
	DeleteCriticalSection(&LockSection);
}

void CCanvas::Arc(INT X1, INT Y1, INT X2, INT Y2, INT X3, INT Y3, INT X4, INT Y4){
	Changing();
	RequiredState(csHandleValid | csPenValid | csBrushValid);
	::Arc(DC, X1, Y1, X2, Y2, X3, Y3, X4, Y4);
	Changed();
}

void GDIError(){
	BYTE Buf[256];
	DWORD ErrorCode = GetLastError();
	if((ErrorCode != 0) && (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		ErrorCode, LOCALE_USER_DEFAULT, (LPWSTR)Buf, sizeof(Buf), NULL) != 0))
		throw (LPWSTR)Buf;
	else
		throw "Out of system resources";
}

HGDIOBJ GDICheck(HGDIOBJ Value){
  if(Value == 0)
	  GDIError();
  return Value;
}

#define ROP_DSPDxax				0x00E20746
typedef DWORD TColorRef;
#define ROP_DstCopy				0x00AA0029
BOOL TransparentStretchBlt(HDC DstDC, INT DstX, INT DstY, INT DstW, INT DstH,
		HDC SrcDC, INT SrcX, INT SrcY, INT SrcW, INT SrcH, HDC MaskDC, INT MaskX,
		INT MaskY){
	HDC MemDC;
	HBITMAP MemBmp;
	HANDLE Save;
	TColorRef crText, crBack;
	HPALETTE SavePal;
	BOOL Result = TRUE;
	if((GetGlobal().GetWin32Platform() == VER_PLATFORM_WIN32_NT) && (SrcW == DstW) && (SrcH == DstH)){
		MemBmp = (HBITMAP)GDICheck(CreateCompatibleBitmap(SrcDC, 1, 1));
		MemBmp = (HBITMAP)SelectObject(MaskDC, MemBmp);
		__try{
			MaskBlt(DstDC, DstX, DstY, DstW, DstH, SrcDC, SrcX, SrcY, MemBmp, MaskX,
				MaskY, MAKEROP4(ROP_DstCopy, SRCCOPY));
		}
		__finally{
			MemBmp = (HBITMAP)SelectObject(MaskDC, MemBmp);
			DeleteObject(MemBmp);
		}
		return Result;
	}
	SavePal = 0;
	MemDC = (HDC)GDICheck(CreateCompatibleDC(0));
	__try{
		MemBmp = (HBITMAP)GDICheck(CreateCompatibleBitmap(SrcDC, SrcW, SrcH));
		Save = SelectObject(MemDC, MemBmp);
		SavePal = SelectPalette(SrcDC, GGlobal.SystemPalette16, FALSE);
		SelectPalette(SrcDC, SavePal, FALSE);
		if (SavePal != 0)
			SavePal = SelectPalette(MemDC, SavePal, TRUE);
		else
			SavePal = SelectPalette(MemDC, GGlobal.SystemPalette16, TRUE);
		RealizePalette(MemDC);
		StretchBlt(MemDC, 0, 0, SrcW, SrcH, MaskDC, MaskX, MaskY, SrcW, SrcH, SRCCOPY);
		StretchBlt(MemDC, 0, 0, SrcW, SrcH, SrcDC, SrcX, SrcY, SrcW, SrcH, SRCERASE);
		crText = SetTextColor(DstDC, 0x0);
		crBack = SetBkColor(DstDC, 0xFFFFFF);
		StretchBlt(DstDC, DstX, DstY, DstW, DstH, MaskDC, MaskX, MaskY, SrcW, SrcH, SRCAND);
		StretchBlt(DstDC, DstX, DstY, DstW, DstH, MemDC, 0, 0, SrcW, SrcH, SRCINVERT);
		SetTextColor(DstDC, crText);
		SetBkColor(DstDC, crBack);
		if(Save != 0)
			SelectObject(MemDC, Save);
		DeleteObject(MemBmp);
	}
	__finally{
		if (SavePal != 0)
			SelectPalette(MemDC, SavePal, FALSE);
		DeleteDC(MemDC);
	}
	return Result;
}

void CCanvas::BrushCopy(const TRect& Dest, CBitmap* Bitmap, const TRect& Source, TColor Color){
	if(Bitmap == NULL)
		return ;
	INT SrcW, SrcH, DstW, DstH;
	TColorRef crBack, crText;
	HDC MaskDC;
	CBitmap *Mask;
	HBITMAP MaskHandle;
	CMethodLock CanvasLock(this, (TLockMethod)&CCanvas::Lock, (TLockMethod)&CCanvas::UnLock);
	Changing();
    RequiredState(csHandleValid | csBrushValid);
	{
		CMethodLock BmpCanvasLock(Bitmap->GetCanvas(), (TLockMethod)&CCanvas::Lock, (TLockMethod)&CCanvas::UnLock);
		CObjectHolder MaskLock(NULL);
		DCHolder MaskDCLock;
		DstW = Dest.right - Dest.left;
		DstH = Dest.bottom - Dest.top;
		SrcW = Source.right - Source.left;
		SrcH = Source.bottom - Source.top;
		if (Bitmap->GetTransparentColor() == Color){
			Mask = NULL;
			MaskHandle = Bitmap->GetMaskHandle();
			MaskDC = CreateCompatibleDC(0);
			MaskHandle = (HBITMAP)SelectObject(MaskDC, MaskHandle);
			MaskDCLock.SwapDC(MaskDC);
			MaskDCLock.SwapRestore(MaskHandle);
		}
		else{
			Mask = new CBitmap();
			MaskLock.SwapObject(Mask);
			Mask->Assign(Bitmap);
			//Replace Color with black and all other colors with white 
			Mask->Mask(Color);
			Mask->GetCanvas()->RequiredState(csHandleValid);
			MaskDC = Mask->GetCanvas()->DC;
			MaskHandle = 0;
		}
		Bitmap->GetCanvas()->RequiredState(csHandleValid);
		//Draw transparently or use brush color to fill background
		if(Brush->GetStyle() == bsClear){
			TransparentStretchBlt(DC, Dest.left, Dest.top, DstW, DstH,
				Bitmap->GetCanvas()->DC, Source.left, Source.top, SrcW, SrcH,
				MaskDC, Source.left, Source.top);
		}
		else{
			StretchBlt(DC, Dest.left, Dest.top, DstW, DstH,
				Bitmap->GetCanvas()->DC, Source.left, Source.top, SrcW, SrcH, SRCCOPY);
			crText = SetTextColor(DC, 0);
			crBack = SetBkColor(DC, 0xFFFFFF);
			StretchBlt(DC, Dest.left, Dest.top, DstW, DstH,
				MaskDC, Source.left, Source.top, SrcW, SrcH, ROP_DSPDxax);
			SetTextColor(DC, crText);
			SetBkColor(DC, crBack);
		}
	}
    Changed();
}

void CCanvas::Chord(INT X1, INT Y1, INT X2, INT Y2, INT X3, INT Y3, INT X4, INT Y4){
	Changing();
	RequiredState(csHandleValid | csPenValid | csBrushValid);
	::Chord(DC, X1, Y1, X2, Y2, X3, Y3, X4, Y4);
	Changed();
}

void CCanvas::CopyRect(const TRect& Dest, CCanvas* Canvas, const TRect& Source){
	Changing();
	RequiredState(csHandleValid | csFontValid | csBrushValid);
	Canvas->RequiredState(csHandleValid | csBrushValid);
	StretchBlt(DC, Dest.left, Dest.top, Dest.right - Dest.left,
		Dest.bottom - Dest.top, Canvas->DC, Source.left, Source.top,
		Source.right - Source.left, Source.bottom - Source.top, CopyMode);
	Changed();
}

void CCanvas::Draw(INT X, INT Y, CGraphic* Graphic){
	if((Graphic != NULL) && (!Graphic->GetEmpty())){
		Changing();
		RequiredState(csHandleValid);
		SetBkColor(DC, ColorToRGB(Brush->GetColor()));
		SetTextColor(DC, ColorToRGB(Font->GetColor()));
		TRect R = Rect(X, Y, X + Graphic->GetWidth(), Y + Graphic->GetHeight());
		Graphic->Draw(this, R);
		Changed();
	}
}

void CCanvas::DrawFocusRect(const TRect& Rect){
	Changing();
	RequiredState(csHandleValid | csBrushValid);
	::DrawFocusRect(DC, (const RECT *)&Rect);
	Changed();
}

void CCanvas::Ellipse(INT X1, INT Y1, INT X2, INT Y2){
	Changing();
	RequiredState(csHandleValid | csPenValid | csBrushValid);
	::Ellipse(DC, X1, Y1, X2, Y2);
	Changed();
}

void CCanvas::Ellipse(const TRect& Rect){
	Ellipse(Rect.left, Rect.top, Rect.right, Rect.bottom);
}

void CCanvas::FillRect(const TRect& Rect){
	Changing();
	RequiredState(csHandleValid | csBrushValid);
	::FillRect(DC, (const RECT *)&Rect, Brush->GetHandle());
	Changed();
}

void CCanvas::FloodFill(INT X, INT Y, TColor Color, BYTE FillStyle){
	WORD FillStyles[] = {FLOODFILLSURFACE, FLOODFILLBORDER};
	Changing();
	RequiredState(csHandleValid | csBrushValid);
	::ExtFloodFill(DC, X, Y, Color, FillStyles[FillStyle]);
	Changed();
}

void CCanvas::FrameRect(const TRect& Rect){
	Changing();
	RequiredState(csHandleValid | csBrushValid);
	::FrameRect(DC, (const RECT *)&Rect, Brush->GetHandle());
	Changed();
}

BOOL CCanvas::HandleAllocated(){
	return DC != 0;
}

void CCanvas::LineTo(INT X, INT Y){
	Changing();
	RequiredState(csHandleValid | csPenValid | csBrushValid);
	::LineTo(DC, X, Y);
	Changed();
}

void CCanvas::Lock(){
	EnterCriticalSection(&GGlobal.CounterLock);
	LockCount++;
	LeaveCriticalSection(&GGlobal.CounterLock);
	EnterCriticalSection(&LockSection);
}

void CCanvas::MoveTo(INT X, INT Y){
	RequiredState(csHandleValid);
	::MoveToEx(DC, X, Y, NULL);
}

void CCanvas::Pie(INT X1, INT Y1, INT X2, INT Y2, INT X3, INT Y3, INT X4, INT Y4){
	Changing();
	RequiredState(csHandleValid | csPenValid | csBrushValid);
	::Pie(DC, X1, Y1, X2, Y2, X3, Y3, X4, Y4);
	Changed();
}

void CCanvas::Polygon(const POINT* Points, INT cpt){
	Changing();
	RequiredState(csHandleValid | csPenValid | csBrushValid);
	::Polygon(DC, Points, cpt);
	Changed();
}

void CCanvas::Polyline(const POINT* Points, INT cpt){
	Changing();
	RequiredState(csHandleValid | csPenValid | csBrushValid);
	::Polyline(DC, Points, cpt);
	Changed();
}

void CCanvas::PolyBezier(const POINT* Points, INT cpt){
	Changing();
	RequiredState(csHandleValid | csPenValid | csBrushValid);
	::PolyBezier(DC, Points, cpt);
	Changed();
}

void CCanvas::PolyBezierTo(const POINT* Points, INT cpt){
	Changing();
	RequiredState(csHandleValid | csPenValid | csBrushValid);
	::PolyBezierTo(DC, Points, cpt);
	Changed();
}

void CCanvas::Rectangle(INT X1, INT Y1, INT X2, INT Y2){
	Changing();
	RequiredState(csHandleValid | csBrushValid | csPenValid);
	::Rectangle(DC, X1, Y1, X2, Y2);
	Changed();
}

void CCanvas::Rectangle(const TRect& Rect){
	Rectangle(Rect.left, Rect.top, Rect.right, Rect.bottom);
}

void CCanvas::Refresh(){
	DeselectHandles();
}

void CCanvas::RoundRect(INT X1, INT Y1, INT X2, INT Y2, INT X3, INT Y3){
	Changing();
	RequiredState(csHandleValid | csBrushValid | csPenValid);
	::RoundRect(DC, X1, Y1, X2, Y2, X3, Y3);
	Changed();
}

void CCanvas::StretchDraw(const TRect& Rect, CGraphic* Graphic){
	if(Graphic != NULL){
		Changing();
		RequiredState(csAllValid);
		Graphic->Draw(this, Rect);
		Changed();
	}
}

SIZE CCanvas::TextExtent(const LPTSTR Text){
	RequiredState(csHandleValid | csFontValid);
	SIZE Result= {0, 0};
	::GetTextExtentPoint32(DC, Text, lstrlen(Text), &Result);
	return Result;
}

INT CCanvas::TextHeight(const LPTSTR Text){
	return TextExtent(Text).cy;
}

void CCanvas::TextOut(INT X, INT Y, const LPTSTR Text){
	Changing();
	RequiredState(csHandleValid | csFontValid | csBrushValid);
	if(GetCanvasOrientation() == coRightToLeft)
		X += TextWidth(Text) + 1;
	::ExtTextOut(DC, X, Y, TextFlags, NULL, Text, lstrlen(Text), NULL);
	MoveTo(X + TextWidth(Text), Y);
	Changed();
}

void CCanvas::TextRect(TRect& Rect, INT X, INT Y, const LPTSTR Text){
	Changing();
	RequiredState(csHandleValid | csFontValid | csBrushValid);
	LONG Options = ETO_CLIPPED | TextFlags;
	if (Brush->GetStyle() != bsClear)
		Options |= ETO_OPAQUE;
	if (((TextFlags & ETO_RTLREADING) != 0) && (GetCanvasOrientation() == coRightToLeft))
		X += TextWidth(Text) + 1;
	::ExtTextOut(DC, X, Y, Options, (const RECT *)&Rect, Text, lstrlen(Text), NULL);
	Changed();
}

INT CCanvas::TextWidth(const LPTSTR Text){
	return TextExtent(Text).cx;
}

BOOL CCanvas::TryLock(){
	BOOL Result = FALSE;
	EnterCriticalSection(&GGlobal.CounterLock);
	__try{
		Result = LockCount == 0;
		if(Result)
			Lock();
	}
	__finally{
		LeaveCriticalSection(&GGlobal.CounterLock);
	}
	return Result;
}

void CCanvas::UnLock(){
	LeaveCriticalSection(&LockSection);
	EnterCriticalSection(&GGlobal.CounterLock);
	LockCount--;
	LeaveCriticalSection(&GGlobal.CounterLock);
}

TRect CCanvas::GetClipRect(){
	TRect Result = {0, 0, 0, 0};
	RequiredState(csHandleValid);
	GetClipBox(DC, (LPRECT)&Result);
	return Result;
}

HDC CCanvas::GetHandle(){
	Changing();
	RequiredState(csAllValid);
	return DC;
}

void CCanvas::SetHandle(HDC Value){
	if(DC != Value){
		if(DC != 0){
			DeselectHandles();
			PenPos = GetPenPos();
			DC = 0;
			State &= ~csHandleValid;
		}
	}
	if(Value != 0){
		State |= csHandleValid;
		DC = Value;
		SetPenPos(PenPos);
	}
}

INT CCanvas::GetLockCount(){
	return LockCount;
}

BYTE CCanvas::GetCanvasOrientation(){
	BYTE Result = coLeftToRight;
	POINT Point = {0, 0};
	if((TextFlags & ETO_RTLREADING) != 0){
		GetWindowOrgEx(GetHandle(), (LPPOINT)&Point);
		if(Point.x != 0)
			Result = coRightToLeft;
	}
	return Result;
}

POINT CCanvas::GetPenPos(){
	POINT Point = {0, 0};
	RequiredState(csHandleValid);
	::GetCurrentPositionEx(DC, (LPPOINT)&Point);
	return Point;
}

void CCanvas::SetPenPos(POINT& Value){
	MoveTo(Value.x, Value.y);
}

INT CCanvas::GetPixel(INT X, INT Y){
	RequiredState(csHandleValid);
	return ::GetPixel(DC, X, Y);
}

void CCanvas::SetPixel(INT X, INT Y, INT Value){
	Changing();
	RequiredState(csHandleValid | csPenValid);
	::SetPixel(DC, X, Y, ColorToRGB(Value));
	Changed();
}

CBrush* CCanvas::GetBrush(){
	return Brush;
}

void CCanvas::SetBrush(CBrush* Value){
	Brush->Assign(Value);
}

LONG CCanvas::GetCopyMode(){
	return CopyMode;
}

void CCanvas::SetCopyMode(LONG& Value){
	CopyMode = Value;
}

CFont* CCanvas::GetFont(){
	return Font;
}

void CCanvas::SetFont(CFont* Value){
	Font->Assign(Value);
}

void CCanvas::SetPen(CPen* Value){
	Pen->Assign(Value);
}

CPen* CCanvas::GetPen(){
	return Pen;
}


IMPL_DYN_CLASS(CResourceManager)
CResourceManager::CResourceManager(WORD AResDataSize):
	ResList(NULL),
	ResDataSize(AResDataSize){
	InitializeCriticalSection(&mLock);
}

CResourceManager::~CResourceManager(){
	DeleteCriticalSection(&mLock);
}
INT ResInfoSize = sizeof(TResource) - sizeof(TFontData);

PResource CResourceManager::AllocResource(const LPVOID ResData){
	WORD ResHash = GetHashCode(ResData, ResDataSize);
	CMethodLock ThisLock(this, (TLockMethod)&CResourceManager::Lock, (TLockMethod)&CResourceManager::Unlock);
    PResource Result = ResList;
	while(Result != NULL && (Result->HashCode != ResHash ||
		memcmp(&(Result->Data), ResData, ResDataSize)))
		Result = Result->Next;
	if(Result == NULL){
		Result = (PResource)malloc(ResDataSize + ResInfoSize);
		Result->Next = ResList;
		Result->RefCount = 0;
		Result->Handle = ((PResData)ResData)->Handle;
		Result->HashCode = ResHash;
		CopyMemory(&(Result->Data), ResData, ResDataSize);
		ResList = Result;
	}
	
    (Result->RefCount)++;
	return Result;
}

void CResourceManager::FreeResource(PResource Resource){
	if(Resource != NULL){
		BOOL DeleteIt = FALSE;
		{//for Lock
			CMethodLock ThisLock(this, (TLockMethod)&CResourceManager::Lock, (TLockMethod)&CResourceManager::Unlock);
			(Resource->RefCount)--;
			DeleteIt = Resource->RefCount == 0;
			if(DeleteIt){
				if(Resource == ResList)
					ResList = Resource->Next;
				else {
					PResource P = ResList;
					while(P->Next != Resource)
						P = P->Next;
					P->Next = Resource->Next;
				}
			}
		}
		if(DeleteIt){// this is outside the critsect to minimize lock time
			if(Resource->Handle != 0)
				DeleteObject(Resource->Handle);
			free(Resource);
		}
	}
}

void CResourceManager::ChangeResource(CGraphicsObject* GraphicsObject, const LPVOID ResData){
	CMethodLock ThisLock(this, (TLockMethod)&CResourceManager::Lock, (TLockMethod)&CResourceManager::Unlock);
	// prevent changes to GraphicsObject.FResource pointer between steps
    PResource P = GraphicsObject->Resource;
    GraphicsObject->Resource = AllocResource(ResData);
    if(GraphicsObject->Resource != P)
		GraphicsObject->Changed();
    FreeResource(P);
}

void CResourceManager::AssignResource(CGraphicsObject* GraphicsObject, PResource AResource){
	CMethodLock ThisLock(this, (TLockMethod)&CResourceManager::Lock, (TLockMethod)&CResourceManager::Unlock);
	PResource P = GraphicsObject->Resource;
    if(P != AResource){
		(AResource->RefCount)++;
		GraphicsObject->Resource = AResource;
		GraphicsObject->Changed();
		FreeResource(P);
	}
}

void CResourceManager::Lock(){
	EnterCriticalSection(&mLock);
}

void CResourceManager::Unlock(){
	LeaveCriticalSection(&mLock);
}

IMPL_DYN_CLASS(CGraphic)
void CGraphic::Changed(CObject* Sender){
	Modified = TRUE;
	if(OnChange != NULL)
		CALL_EVENT(Change)(this);
}

BOOL CGraphic::Equals(CGraphic* Graphic){
	BOOL Result = (Graphic != NULL) && (this->GetClass() == Graphic->GetClass());
	if(GetEmpty() || Graphic->GetEmpty()){
		Result = GetEmpty() && Graphic->GetEmpty();
		return Result;
	}
	if(Result){
		CMemoryStream MyImage;
		WriteData(&MyImage);
		{
			CMemoryStream GraphicsImage;
			Graphic->WriteData(&GraphicsImage);
			Result = (MyImage.GetSize() == GraphicsImage.GetSize()) &&
				memcmp(MyImage.GetMemory(), GraphicsImage.GetMemory(), MyImage.GetSize());
		}
	}
	return Result;
}

void CGraphic::Progress(CObject* Sender, TProgressStage Stage, BYTE PercentDone, BOOL RedrawNow, const TRect& R, LPTSTR Msg){
	if(OnProgress != NULL)
		CALL_EVENT(Progress)(Sender, Stage, PercentDone, RedrawNow, R, Msg);
}


void CGraphic::ReadData(CStream* Stream){
	LoadFromStream(Stream);
}

void CGraphic::WriteData(CStream* Stream){
	SaveToStream(Stream);
}

CGraphic::CGraphic():
	Modified(FALSE),
	PaletteModified(FALSE),
	Transparent(FALSE),
	INIT_EVENT(Change),
	INIT_EVENT(Progress){
}

CGraphic::~CGraphic(){
}

void CGraphic::LoadFromFile(const LPTSTR Filename){
	CFileStream Stream(Filename, fmOpenRead | fmShareDenyWrite);
	LoadFromStream(&Stream);
}

void CGraphic::SaveToFile(const LPTSTR Filename){
	CFileStream Stream(Filename, fmCreate);
	SaveToStream(&Stream);
}

void CGraphic::SetModified(BOOL Value){
	if(Value)
		Changed(this);
	else
		Modified = FALSE;
}

HPALETTE CGraphic::GetPalette(){
	return 0;
}

void CGraphic::SetPalette(HPALETTE Value){
}
	
BOOL CGraphic::GetTransparent(){
	return Transparent;
}

void CGraphic::SetTransparent(BOOL Value){
	if(Value != Transparent){
		Transparent = Value;
		Changed(this);
	}
}

IMPL_DYN_CLASS(CSharedImage)
CSharedImage::CSharedImage():
	RefCount(0){
}

CSharedImage::~CSharedImage(){
}

void CSharedImage::Reference(){
	RefCount++;
}

void CSharedImage::Release(){
	if(this != NULL){
		RefCount--;
		if(RefCount == 0){
			FreeHandle();
			delete this;
		}
	}
}

CThreadList BitmapCanvasList;

IMPL_DYN_CLASS(CBitmapCanvas)
void CBitmapCanvas::FreeContext(){
	if(DC != 0){
		CMethodLock ThisLock(this, (TLockMethod)&CCanvas::Lock, (TLockMethod)&CCanvas::UnLock);
		if(OldBitmap != 0)
			SelectObject(DC, OldBitmap);
		if(OldPalette != 0)
			SelectObject(DC, OldPalette);
		HDC H = DC;
		this->SetHandle(0);
		DeleteDC(H);
		BitmapCanvasList.Remove(this);
	}
}
void CBitmapCanvas::CreateHandle(){
	if(Bitmap != NULL){
		CMethodLock ThisLock(this, (TLockMethod)&CCanvas::Lock, (TLockMethod)&CCanvas::UnLock);
		Bitmap->HandleNeeded();
		DeselectBitmap(Bitmap->Image->Handle);
		Bitmap->PaletteNeeded();
		HDC H = CreateCompatibleDC(0);
		if(Bitmap->Image->Handle != 0)
			OldBitmap = (HBITMAP)SelectObject(H, Bitmap->Image->Handle);
		else
			OldBitmap = 0;
		if(Bitmap->Image->Palette != 0){
			OldPalette = SelectPalette(H, Bitmap->Image->Palette, TRUE);
			RealizePalette(H);
		}
		else
			OldPalette = 0;
		this->SetHandle(H);
		BitmapCanvasList.Add(this);
	}
}

CBitmapCanvas::CBitmapCanvas(CBitmap* ABitmap):
	Bitmap(ABitmap),
	OldBitmap(0),
	OldPalette(0){
}
CBitmapCanvas::~CBitmapCanvas(){
	FreeContext();
}

/* DeselectBitmap is called to ensure that a bitmap handle is not
  selected into any memory DC anywhere in the system.  If the bitmap
  handle is in use by a locked canvas, DeselectBitmap must wait for
  the canvas to unlock. */
void DeselectBitmap(HBITMAP AHandle){
	if(AHandle == 0)
		return;
	CMethodLock CanvasListLock(&BitmapCanvasList, (TLockMethodResult)&CThreadList::LockList, (TLockMethod)&CThreadList::UnlockList);
	CList* List = (CList *)CanvasListLock.GetLockResult();
	for (INT I = List->GetCount() - 1; I >= 0; I--){
		CBitmapCanvas * BmpCanvas = (CBitmapCanvas *)List->Get(I);
		if((BmpCanvas->Bitmap != NULL) && (BmpCanvas->Bitmap->Image->Handle == AHandle))
			BmpCanvas->FreeContext();
	}
}
void InternalDeletePalette(HPALETTE Pal){
	if(Pal != 0 && Pal != GGlobal.SystemPalette16)
		DeleteObject(Pal);
}
class InternalPaletteHolder{
private:
	HPALETTE Pal;
public:
	InternalPaletteHolder(HPALETTE APal = 0){
		Pal = APal;
	}
	virtual ~InternalPaletteHolder(){
		if(Pal != 0)
			InternalDeletePalette(Pal);
	}
	HPALETTE SwapPalette(HPALETTE APal){
		HPALETTE Ret = Pal;
		Pal = APal;
		return Ret;
	}
};

IMPL_DYN_CLASS(CBitmapImage)
CBitmapImage::CBitmapImage():
	Handle(0),
	MaskHandle(0),
	Palette(0),
	DIBHandle(0),
	SaveStream(NULL),
	OS2Format(FALSE),
	Halftone(FALSE){
	ZeroMemory(&FDIB, sizeof(FDIB));
}

CBitmapImage::~CBitmapImage(){
	if(DIBHandle != 0){
		DeselectBitmap(DIBHandle);
		DeleteObject(DIBHandle);
		DIBHandle = 0;
	}
	FreeHandle();
	if(FDIB.dshSection != 0)
		CloseHandle(FDIB.dshSection);
	if(SaveStream != NULL)
		FreeAndNil((CObject **)&SaveStream);
}

void CBitmapImage::FreeHandle(){
	if((Handle != 0) && (Handle != DIBHandle)){
		DeselectBitmap(Handle);
		DeleteObject(Handle);
	}
	if(MaskHandle != 0){
		DeselectBitmap(MaskHandle);
		DeleteObject(MaskHandle);
		MaskHandle = 0;
	}
	if(Palette != 0)
		InternalDeletePalette(Palette);
	Handle = 0;
	Palette = 0;
}

INT AdjustColor(INT I){
	return I == 0 ? MAXINT : I;
}

BOOL BetterSize(const TIconRec& Old, const TIconRec& New, const TPoint& IconSize){
	INT NewX = New.Width - IconSize.x;
	INT NewY = New.Height - IconSize.y;
	INT OldX = Old.Width - IconSize.x;
	INT OldY = Old.Height - IconSize.y;
	return ::abs(NewX) <= ::abs(OldX) && (NewX <= 0 || NewX <= OldX) &&
       ::abs(NewY) <= ::abs(OldY) && (NewY <= 0 || NewY <= OldY);
}

INT BytesPerScanline(INT PixelsPerScanline, INT BitsPerPixel, INT Alignment){
	Alignment--;
	INT Result = ((PixelsPerScanline * BitsPerPixel) + Alignment) & (~Alignment);
	Result = Result / 8;
	return Result;
}

INT GetDInColors(WORD BitCount){
	return (BitCount == 1 || BitCount == 4 || BitCount == 8)
		? (1 << BitCount) : 0;
}

HBITMAP DupBits(HBITMAP Src, const TPoint& Size, BOOL Mono){
	HDC Mem1 = CreateCompatibleDC(0);
	DCHolder d1Holder(Mem1);
	HDC Mem2 = CreateCompatibleDC(0);
	DCHolder d2Holder(Mem2);
	BITMAP bmp;
	GetObject(Src, sizeof(bmp), &bmp);
	HBITMAP Result = 0;
	if(Mono)
		Result = CreateBitmap(Size.x, Size.y, 1, 1, NULL);
	else {
		HDC DC = GetDC(0);
		if(DC == 0)
			GDIError();
		DCHolder dHolder(DC, 0, 0);
		Result = CreateCompatibleBitmap(DC, Size.x, Size.y);
		if(Result == 0)
			GDIError();
	}
	if(Result != 0){
		HBITMAP Old1 = (HBITMAP)SelectObject(Mem1, Src);
		d1Holder.SwapRestore(Old1);
		HBITMAP Old2 = (HBITMAP)SelectObject(Mem2, Result);
		d2Holder.SwapRestore(Old2);
		StretchBlt(Mem2, 0, 0, Size.x, Size.y, Mem1, 0, 0, bmp.bmWidth,
			bmp.bmHeight, SRCCOPY);
	}
	return Result;
}

void TwoBitsFromDIB(BITMAPINFOHEADER& BI, HBITMAP& XorBits, HBITMAP& AndBits, const TPoint& IconSize){
	typedef LONG TLongArray[2];
	typedef TLongArray *PLongArray;
	BI.biHeight = BI.biHeight >> 1;
	BI.biSizeImage = BytesPerScanline(BI.biWidth, BI.biBitCount, 32) * BI.biHeight;
	INT NumColors = GetDInColors(BI.biBitCount);
	HDC DC = GetDC(0);
	if(DC == 0)
		throw "Out of system resources";
	DCHolder dcHolder(DC);
	LPVOID Bits = (LPVOID)((LONG_PTR)&BI + sizeof(BI) + NumColors * sizeof(RGBQUAD));
	HBITMAP Temp = (HBITMAP)GDICheck(CreateDIBitmap(DC, &BI, CBM_INIT, Bits, (const BITMAPINFO *)&BI, DIB_RGB_COLORS));
	{
		GDIOBJHolder tmpHolder(Temp);
		XorBits = DupBits(Temp, IconSize, FALSE);
	}
	Bits = (LPVOID)((LONG_PTR)Bits + BI.biSizeImage);
	BI.biBitCount = 1;
	BI.biSizeImage = BytesPerScanline(BI.biWidth, BI.biBitCount, 32) * BI.biHeight;
	BI.biClrUsed = 2;
	BI.biClrImportant = 2;
	PLongArray Colors = (PLongArray)((LONG_PTR)&BI + sizeof(BI));
    (*Colors)[0] = 0;
	(*Colors)[1] = 0xFFFFFF;
	Temp = (HBITMAP)GDICheck(CreateDIBitmap(DC, &BI, CBM_INIT, Bits, (const BITMAPINFO *)&BI, DIB_RGB_COLORS));
	{
		GDIOBJHolder tmp1Holder(Temp);
		AndBits = DupBits(Temp, IconSize, TRUE);
	}
}

typedef RGBTRIPLE TRGBTripleArray[255];
typedef TRGBTripleArray *PRGBTripleArray;
typedef RGBQUAD TRGBQuadArray[255];
typedef TRGBQuadArray *PRGBQuadArray;

//{ RGBTripleToQuad performs in-place conversion of an OS2 color table into a DIB color table.   }
void RGBTripleToQuad(LPVOID ColorTable){
	PRGBTripleArray P3 = (PRGBTripleArray)ColorTable;
	PRGBQuadArray P4 = (PRGBQuadArray)P3;
	for(INT I = 255; I >= 1; I--){  // don't move zeroth item 
		(*P4)[I].rgbRed = (*P3)[I].rgbtRed;// order is significant for last item moved
		(*P4)[I].rgbGreen = (*P3)[I].rgbtGreen;
		(*P4)[I].rgbBlue = (*P3)[I].rgbtBlue;
		(*P4)[I].rgbReserved = 0;
	}
	(*P4)[0].rgbReserved = 0;
}

//{ RGBQuadToTriple performs the inverse of RGBTripleToQuad. }
void RGBQuadToTriple(LPVOID ColorTable, INT ColorCount){
	PRGBTripleArray P3 = (PRGBTripleArray)ColorTable;
	PRGBQuadArray P4 = (PRGBQuadArray)P3;
	for(INT I = 1; I < ColorCount; I++){// don't move zeroth item
		(*P3)[I].rgbtRed = (*P4)[I].rgbRed;
		(*P3)[I].rgbtGreen = (*P4)[I].rgbGreen;
		(*P3)[I].rgbtBlue = (*P4)[I].rgbBlue;
	}
	if(ColorCount < 256){
		ZeroMemory(&((*P3)[ColorCount]), (256 - ColorCount) * sizeof(RGBTRIPLE));
		ColorCount = 256;   // OS2 color tables always have 256 entries
	}
}

void ByteSwapColors(LPRGBQUAD Colors, INT Count){
	unsigned char tmp = 0;
	for(INT i = 0; i < Count; i++){
		tmp = ((unsigned char *)Colors)[0];
		((unsigned char *)Colors)[0] = ((unsigned char *)Colors)[2];
		((unsigned char *)Colors)[2] = tmp;
		Colors++;
	}
}

BOOL SystemPaletteOverride(PMaxLogPalette Pal){
	BOOL Result = FALSE;
	if(GGlobal.SystemPalette16 != 0){
		HDC DC = GetDC(0);
		DCHolder dcHolder(DC, 0, 0);
		INT SysPalSize = GetDeviceCaps(DC, SIZEPALETTE);
		if(SysPalSize >= 16){
			//{ Ignore the disk image of the palette for 16 color bitmaps. 
			//Replace with the first and last 8 colors of the system palette }
			GetPaletteEntries(GGlobal.SystemPalette16, 0, 8, (LPPALETTEENTRY)(Pal->palPalEntry));
			GetPaletteEntries(GGlobal.SystemPalette16, 8, 8, (LPPALETTEENTRY)&(Pal->palPalEntry[Pal->palNumEntries - 8]));
			Result = TRUE;
		}
	}
	return Result;
}

void InitializeBitmapInfoHeader(HBITMAP Bitmap, BITMAPINFOHEADER& BI, INT Colors){
	DIBSECTION DS;
	DS.dsBmih.biSize = 0;
	INT Bytes = GetObject(Bitmap, sizeof(DS), &DS);
	if(Bytes == 0)
		throw "invalid bitmap";
	else if(Bytes >= (sizeof(DS.dsBm) + sizeof(DS.dsBmih)) &&
		DS.dsBmih.biSize >= DWORD(sizeof(DS.dsBmih)))
		BI = DS.dsBmih;
	else{
		ZeroMemory(&BI, sizeof(BI));
		BI.biSize = sizeof(BI);
		BI.biWidth = DS.dsBm.bmWidth;
		BI.biHeight = DS.dsBm.bmHeight;
	}
	if(Colors == 2)
		BI.biBitCount = 1;
	else if(Colors >= 3 && Colors <= 16){
		BI.biBitCount = 4;
        BI.biClrUsed = Colors;
	}
	else if(Colors >= 17 && Colors <= 256){
		BI.biBitCount = 8;
		BI.biClrUsed = Colors;
	}
	else
		BI.biBitCount = DS.dsBm.bmBitsPixel * DS.dsBm.bmPlanes;
	BI.biPlanes = 1;
	if(BI.biClrImportant > BI.biClrUsed)
		BI.biClrImportant = BI.biClrUsed;
	if(BI.biSizeImage == 0)
		BI.biSizeImage = BytesPerScanline(BI.biWidth, BI.biBitCount, 32) * abs(BI.biHeight);
}                        

void InternalGetDIBSizes(HBITMAP Bitmap, DWORD& InfoHeaderSize, DWORD& ImageSize, INT Colors){
	BITMAPINFOHEADER BI;
	InitializeBitmapInfoHeader(Bitmap, BI, Colors);
	if(BI.biBitCount > 8){
		InfoHeaderSize = sizeof(BITMAPINFOHEADER);
		if((BI.biCompression & BI_BITFIELDS) != 0)
			InfoHeaderSize += 12;
	}
	else if(BI.biClrUsed == 0)
		InfoHeaderSize = sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * ((ULONG_PTR)1 << BI.biBitCount);
	else
		InfoHeaderSize = sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * BI.biClrUsed;
	ImageSize = BI.biSizeImage;
}

BOOL InternalGetDIB(HBITMAP Bitmap, HPALETTE Palette, BITMAPINFO& BitmapInfo, LPVOID Bits, INT Colors){
	InitializeBitmapInfoHeader(Bitmap, BitmapInfo.bmiHeader, Colors);
	HPALETTE OldPal = 0;
	HDC DC = CreateCompatibleDC(0);
	DCHolder dcHolder(DC);
	if(Palette != 0){
		OldPal = SelectPalette(DC, Palette, FALSE);
		dcHolder.SwapRestore(OldPal);
		RealizePalette(DC);
	}
	return GetDIBits(DC, Bitmap, 0, BitmapInfo.bmiHeader.biHeight, Bits,
			&BitmapInfo, DIB_RGB_COLORS) != 0;
}
HPALETTE PaletteFromDIBColorTable(HANDLE DIBHandle, LPVOID ColorTable, INT ColorCount){
	//Pal: TMaxLogPalette;
	HPALETTE Result = 0;
	TMaxLogPalette Pal;
	Pal.palVersion = 0x300;
	if(DIBHandle != 0){
		HDC DC = CreateCompatibleDC(0);
		HGDIOBJ Save = SelectObject(DC, DIBHandle);
		Pal.palNumEntries = GetDIBColorTable(DC, 0, 256, (RGBQUAD *)Pal.palPalEntry);
		SelectObject(DC, Save);
		DeleteDC(DC);
	}
	else{
		Pal.palNumEntries = ColorCount;
		CopyMemory(Pal.palPalEntry, ColorTable, ColorCount * 4);
	}
	if(Pal.palNumEntries == 0)
		return Result;
	if(Pal.palNumEntries != 16 || !SystemPaletteOverride(&Pal))
		ByteSwapColors((LPRGBQUAD)Pal.palPalEntry, Pal.palNumEntries);
	Result = CreatePalette((PLOGPALETTE)&Pal);
	return Result;
}

INT PaletteToDIBColorTable(HPALETTE Pal,LPRGBQUAD ColorTable, INT ColorTableLength){
	INT Result = 0;
	if((Pal == 0) || (GetObject(Pal, sizeof(Result), &Result) == 0) || (Result == 0))
		return Result;
	if(Result > ColorTableLength)
		Result = ColorTableLength;
	GetPaletteEntries(Pal, 0, Result, (LPPALETTEENTRY)ColorTable);
	ByteSwapColors(ColorTable, Result);
	return Result;
}

void UpdateDIBColorTable(HBITMAP DIBHandle, HPALETTE Pal, DIBSECTION& DIB){
	RGBQUAD Colors[256];
	if(DIBHandle != 0 && DIB.dsBmih.biBitCount <= 8){
		INT ColorCount = PaletteToDIBColorTable(Pal, Colors, 256);
		if(ColorCount == 0)
			return ;
		HDC ScreenDC = GetDC(0);
		DCHolder srcDCHolder(ScreenDC, 0, 0);
		HDC DC = CreateCompatibleDC(ScreenDC);
		DCHolder dcHolder(DC);
		HGDIOBJ OldBM = SelectObject(DC, DIBHandle);
		dcHolder.SwapRestore(OldBM);
		SetDIBColorTable(DC, 0, ColorCount, Colors);
	}
}

void FixupBitFields(DIBSECTION *DIB){
	if (((DIB->dsBmih.biCompression & BI_BITFIELDS) != 0) && (DIB->dsBitfields[0] == 0)){
		if (DIB->dsBmih.biBitCount == 16){
			// fix buggy 16 bit color drivers
			DIB->dsBitfields[0] = 0xF800;
			DIB->dsBitfields[1] = 0x07E0;
			DIB->dsBitfields[2] = 0x001F;
		}
		else if (DIB->dsBmih.biBitCount == 32){
			// fix buggy 32 bit color drivers
			DIB->dsBitfields[0] = 0x00FF0000;
			DIB->dsBitfields[1] = 0x0000FF00;
			DIB->dsBitfields[2] = 0x000000FF;
		}
	}
}

HBITMAP CopyBitmap(HBITMAP Handle, HPALETTE NewPalette, HPALETTE OldPalette, DIBSECTION *DIB, CCanvas *pCanvas){
	HBITMAP Result = 0;
	if((DIB->dsBmih.biSize != 0) && ((DIB->dsBmih.biWidth == 0) || (DIB->dsBmih.biHeight == 0))) 
		return Result;
	if((DIB->dsBmih.biSize == 0) && ((DIB->dsBm.bmWidth == 0) || (DIB->dsBm.bmHeight == 0))) 
		return Result;
	DeselectBitmap(Handle);
	DIBSECTION SrcDIB;
	ZeroMemory(&SrcDIB, sizeof(SrcDIB));
	SrcDIB.dsBmih.biSize = 0;
	if(Handle != 0) {
		INT S = GetObject(Handle, sizeof(SrcDIB), &SrcDIB);
		if(S == 0)
			RaiseLastOSError();
		if(S < sizeof(SrcDIB.dsBm)){
			throw "invalid bitmap ";
		}
	}
	DCHolder SrcDCHolder;
	HDC ScreenDC = (HDC)GDICheck((HDC)GetDC(0));
	SrcDCHolder.SwapDC(ScreenDC);
	DCHolder niDCHolder;
	HDC NewImageDC = (HDC)GDICheck((HDC)CreateCompatibleDC(ScreenDC));
	niDCHolder.SwapDC(NewImageDC);
	if(DIB->dsBmih.biSize < sizeof(DIB->dsBmih)){
		if((DIB->dsBm.bmPlanes | DIB->dsBm.bmBitsPixel) == 1) //monochrome
			Result = (HBITMAP)GDICheck(CreateBitmap(DIB->dsBm.bmWidth, DIB->dsBm.bmHeight, 1, 1, NULL));
		else // Create DDB
			Result = (HBITMAP)GDICheck(CreateCompatibleBitmap(ScreenDC, DIB->dsBm.bmWidth, DIB->dsBm.bmHeight));
	}
	else {// Create DIB
		PBITMAPINFO BI;
		BufferHolder BIHolder;
		BI = (PBITMAPINFO)malloc(sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
		BIHolder.SwapBuffer(BI);
		DIB->dsBmih.biSize = sizeof(BI->bmiHeader);
		DIB->dsBmih.biPlanes = 1;
		if(DIB->dsBmih.biBitCount == 0)
			DIB->dsBmih.biBitCount = GetDeviceCaps(ScreenDC, BITSPIXEL) * GetDeviceCaps(ScreenDC, PLANES);
		BI->bmiHeader = DIB->dsBmih;
		DIB->dsBm.bmWidth = DIB->dsBmih.biWidth;
		DIB->dsBm.bmHeight = DIB->dsBmih.biHeight;
		if(DIB->dsBmih.biBitCount <= 8){
			if((DIB->dsBmih.biBitCount == 1) && ((Handle == 0) || (SrcDIB.dsBm.bmBits == NULL))){
				// set mono DIB to white/black when converting from DDB.
				*((int *)&BI->bmiColors[0]) = 0;
				*((int *)&BI->bmiColors[1]) = 0xffffff;
			}
			else if(NewPalette != 0){
				PaletteToDIBColorTable(NewPalette, BI->bmiColors, 256);
			}
			else if(Handle != 0){
				HBITMAP NewScr = (HBITMAP)SelectObject(NewImageDC, Handle);
				if((SrcDIB.dsBmih.biSize > 0) && (SrcDIB.dsBm.bmBits != NULL))
					DIB->dsBmih.biClrUsed = GetDIBColorTable(NewImageDC, 0, 256, BI->bmiColors);
				else
					GetDIBits(NewImageDC, Handle, 0, abs(DIB->dsBmih.biHeight), NULL, BI, DIB_RGB_COLORS);
				SelectObject(NewImageDC, NewScr);
			}
		}
		else if(((DIB->dsBmih.biBitCount == 16) || (DIB->dsBmih.biBitCount == 32)) && 
			((DIB->dsBmih.biCompression & BI_BITFIELDS) != 0)){
			FixupBitFields(DIB);
			CopyMemory(DIB->dsBitfields, BI->bmiColors, sizeof(DIB->dsBitfields));
		}
		LPVOID BitsMem = NULL;
		Result = (HBITMAP)GDICheck(CreateDIBSection(ScreenDC, BI, DIB_RGB_COLORS, &BitsMem, 0, 0));
		if(BitsMem == NULL)
			throw "gdi error";
		if((Handle != 0) && (SrcDIB.dsBm.bmWidth == DIB->dsBmih.biWidth) &&
			(SrcDIB.dsBm.bmHeight == DIB->dsBmih.biHeight) && (DIB->dsBmih.biBitCount > 8)){
			GetDIBits(NewImageDC, Handle, 0, abs(DIB->dsBmih.biHeight), BitsMem, BI, DIB_RGB_COLORS);
			return Result;
		}
	}
	GDICheck(Result);
	HBITMAP NewScr = (HBITMAP)GDICheck(SelectObject(NewImageDC, Result));
	try{
		HPALETTE Pal1 = 0;
		HPALETTE Pal2 = 0;
		if(NewPalette != 0){
			Pal1 = SelectPalette(NewImageDC, NewPalette, FALSE);
			RealizePalette(NewImageDC);
		}
		try{
			if(pCanvas != NULL){
				TRect rc = Rect(0, 0, DIB->dsBm.bmWidth, DIB->dsBm.bmHeight);
				FillRect(NewImageDC, (RECT *)&rc, pCanvas->GetBrush()->GetHandle());
				SetTextColor(NewImageDC, ColorToRGB(pCanvas->GetFont()->GetColor()));
				SetBkColor(NewImageDC, ColorToRGB(pCanvas->GetBrush()->GetColor()));
				if((DIB->dsBmih.biBitCount == 1) && (DIB->dsBm.bmBits != NULL)){
					INT MonoColors[2];
					MonoColors[0] = ColorToRGB(pCanvas->GetFont()->GetColor());
					MonoColors[1] = ColorToRGB(pCanvas->GetBrush()->GetColor());
					SetDIBColorTable(NewImageDC, 0, 2, (RGBQUAD *)&MonoColors);
				}
			}
			else{
				PatBlt(NewImageDC, 0, 0, DIB->dsBm.bmWidth, DIB->dsBm.bmHeight, WHITENESS);
			}
			if(Handle != 0){
				DCHolder OldImgDCHolder;
				HDC OldImageDC = (HDC)GDICheck(CreateCompatibleDC(ScreenDC));
				OldImgDCHolder.SwapDC(OldImageDC);
				HBITMAP OldScr = (HBITMAP)GDICheck(SelectObject(OldImageDC, Handle));
				if(OldPalette != 0){
					Pal2 = SelectPalette(OldImageDC, OldPalette, FALSE);
					RealizePalette(OldImageDC);
				}
				if(pCanvas != NULL){
					SetTextColor(OldImageDC, ColorToRGB(pCanvas->GetFont()->GetColor()));
					SetBkColor(OldImageDC, ColorToRGB(pCanvas->GetBrush()->GetColor()));
				}
				BitBlt(NewImageDC, 0, 0, DIB->dsBm.bmWidth, DIB->dsBm.bmHeight, OldImageDC, 0, 0, SRCCOPY);
				if(OldPalette != 0)
					SelectPalette(OldImageDC, Pal2, TRUE);
				GDICheck(SelectObject(OldImageDC, OldScr));
			}
		}catch(...){
			if (NewPalette != 0)
				SelectPalette(NewImageDC, Pal1, TRUE);
			throw;
		}
		if (NewPalette != 0)
			SelectPalette(NewImageDC, Pal1, TRUE);
	}catch(...){
		DeleteObject(Result);
		throw;
	}
	return Result;
}

HPALETTE CopyPalette(HPALETTE Palette){
	if(Palette == 0)
		return 0;
	INT PaletteSize = 0;
	if(GetObject(Palette, sizeof(PaletteSize), &PaletteSize) == 0)
		return 0;
	if(PaletteSize == 0)
		return 0;
	BufferHolder LogPalHolder;
	PLOGPALETTE pLogPal = (PLOGPALETTE)malloc(sizeof(LOGPALETTE) + PaletteSize * sizeof(PALETTEENTRY));
	LogPalHolder.SwapBuffer(pLogPal);
	pLogPal->palVersion = 0x300;
	pLogPal->palNumEntries = PaletteSize;
	GetPaletteEntries(Palette, 0, PaletteSize, pLogPal->palPalEntry);
	return CreatePalette(pLogPal);
}


HBITMAP CopyBitmapAsMask(HBITMAP Handle, HPALETTE Palette, COLORREF TransparentColor){
	HBITMAP Result = 0;
	DIBSECTION DIB;
	if(Handle != 0 && GetObject(Handle, sizeof(DIB), &DIB) != 0){
		DeselectBitmap(Handle);
		HDC ScreenDC = (HDC)GDICheck(GetDC(0));
		DCHolder srcDCHolder(ScreenDC, 0, 0);
		HDC MonoDC = (HDC)GDICheck(CreateCompatibleDC(ScreenDC));
		DCHolder monoDCHolder(MonoDC);
		Result = CreateBitmap(DIB.dsBm.bmWidth, DIB.dsBm.bmHeight, 1, 1, NULL);
		if(Result != 0){
			HGDIOBJ SaveMono = SelectObject(MonoDC, Result);
			GDIOBJRestorer saveMonoRestorer(MonoDC, SaveMono);
			if(TransparentColor == clNone)
				PatBlt(MonoDC, 0, 0, DIB.dsBm.bmWidth, DIB.dsBm.bmHeight, BLACKNESS);
			else {
				HDC BitmapDC = (HDC)GDICheck(CreateCompatibleDC(ScreenDC));
				DCHolder bmpDCHolder(BitmapDC);
				//{ Convert DIB to DDB }
				GDIOBJHolder handleHolder;
				if(DIB.dsBm.bmBits != NULL){
					DIB.dsBmih.biSize = 0;
					Handle = CopyBitmap(Handle, Palette, Palette, &DIB, NULL);
					handleHolder.SwapObject(Handle);
				};
				HGDIOBJ SaveBitmap = SelectObject(BitmapDC, Handle);
				bmpDCHolder.SwapRestore(SaveBitmap);
				if(Palette != 0){
					SelectPalette(BitmapDC, Palette, FALSE);
					RealizePalette(BitmapDC);
					SelectPalette(MonoDC, Palette, FALSE);
					RealizePalette(MonoDC);
				}
				COLORREF BkColor = SetBkColor(BitmapDC, TransparentColor);
				BitBlt(MonoDC, 0, 0, DIB.dsBm.bmWidth, DIB.dsBm.bmHeight, BitmapDC, 0, 0, SRCCOPY);
				SetBkColor(BitmapDC, BkColor);
			}
		}
	}
	return Result;
}

IMPL_DYN_CLASS(CBitmap)
CBitmap::CBitmap():Canvas(NULL),
	IgnorePalette(FALSE),
	MaskBitsValid(FALSE),
	MaskValid(FALSE),
	TransparentColor(clDefault),
	TransparentMode(tmAuto){
	Image = new CBitmapImage();
	Image->Reference();
	if(GGlobal.DDBsOnly)
		SetHandleType(bmDDB);
}

CBitmap::~CBitmap(){
	FreeContext();
	delete Image;
	delete Canvas;
}
void CBitmap::Changing(CObject* Sender){
	FreeImage();
	Image->FDIB.dsBmih.biClrUsed = 0;
	Image->FDIB.dsBmih.biClrImportant = 0;
	if(Image->SaveStream != NULL)
		FreeAndNil((CObject **)&(Image->SaveStream));
}

void CBitmap::CopyImage(HBITMAP AHandle, HPALETTE APalette, DIBSECTION DIB){
	FreeContext();
	InternalPaletteHolder PalHolder;
	GDIOBJHolder bmpHolder;
	HBITMAP NewHandle = 0;
	HPALETTE NewPalette = 0;
	if(APalette == GGlobal.SystemPalette16)
		NewPalette = APalette;
	else
		NewPalette = CopyPalette(APalette);
	PalHolder.SwapPalette(NewPalette);
	NewHandle = CopyBitmap(AHandle, APalette, NewPalette, &DIB, Canvas);
	bmpHolder.SwapObject(NewHandle);
	NewImage(NewHandle, NewPalette, DIB, Image->OS2Format);
	PalHolder.SwapPalette(0);
	bmpHolder.SwapObject(0);
}

void CBitmap::DIBNeeded(){
	if((Image->Handle == 0) || (Image->DIBHandle != 0))
		return;
	PaletteNeeded();
	if(Image->FDIB.dsBmih.biSize == 0){
		GetObject(Image->Handle, sizeof(Image->FDIB), &(Image->FDIB));
		Image->FDIB.dsBmih.biSize = sizeof(Image->FDIB.dsBmih);
		Image->FDIB.dsBmih.biWidth = Image->FDIB.dsBm.bmWidth;
        Image->FDIB.dsBmih.biHeight = Image->FDIB.dsBm.bmHeight;
        Image->FDIB.dsBmih.biPlanes = 1;
        Image->FDIB.dsBmih.biBitCount = Image->FDIB.dsBm.bmPlanes * Image->FDIB.dsBm.bmBitsPixel;
	}
	Image->DIBHandle = CopyBitmap(Image->Handle, Image->Palette, Image->Palette, &(Image->FDIB), NULL);
}

void CBitmap::FreeContext(){
	if (Canvas != NULL)
		((CBitmapCanvas *)Canvas)->FreeContext();
}

void CBitmap::NewImage(HBITMAP NewHandle, HPALETTE NewPalette,
	const DIBSECTION& NewDIB, BOOL OS2Format, CStream* RLEStream){
	CBitmapImage* Image = new CBitmapImage();
	CObjectHolder imgHolder(Image);
	Image->Handle = NewHandle;
	Image->Palette = NewPalette;
	Image->FDIB = NewDIB;
	Image->OS2Format = OS2Format;
	if(Image->FDIB.dsBm.bmBits != NULL)
		Image->DIBHandle = Image->Handle;
	else
		Image->SaveStream = (CMemoryStream *)RLEStream;
	imgHolder.SwapObject(NULL);
	{//!! replace with InterlockedExchange()
		CCriticalSectionLock Lock(&GGlobal.BitmapImageLock);
		this->Image->Release();
		this->Image = Image;
		this->Image->Reference();
	}
	MaskValid = FALSE;
}

void CBitmap::ReadStream(CStream* Stream, LONG Size){
	FreeContext();
	if(Size == 0){
		DIBSECTION DIB;
		ZeroMemory(&DIB, sizeof(DIB));
		NewImage(0, 0, DIB, FALSE);
	}
	else{
		BITMAPFILEHEADER Bmf;
		LONG lLen = sizeof(Bmf);
		Stream->ReadBuffer(&Bmf, lLen);
		if(Bmf.bfType != 0x4D42)
			throw "invalid bitmap stream";
		ReadDIB(Stream, Size - sizeof(Bmf), &Bmf);
	}
}

void CBitmap::ReadDIB(CStream* Stream, LONG ImageSize, PBITMAPFILEHEADER bmf){
	BYTE DIBPalSizes[] = {sizeof(RGBQUAD), sizeof(RGBTRIPLE)};
	HPALETTE Pal = 0;
	HBITMAP BMHandle = 0;
	CStream* RLEStream = NULL;
	DWORD HeaderSize = 0;
	BITMAPFILEHEADER vbmf;
	BITMAPCOREHEADER Os2Header;
	LPVOID ColorTable = NULL;
	Stream->Read(&HeaderSize, sizeof(HeaderSize));
	BOOL Os2Format = HeaderSize == sizeof(Os2Header);
	if(Os2Format)
		HeaderSize = sizeof(BITMAPINFOHEADER);
	LPBITMAPINFO BitmapInfo = (LPBITMAPINFO)malloc(HeaderSize + 12 + 256 * sizeof(RGBQUAD));
	BufferHolder bitinfoHolder(BitmapInfo);
	CObjectHolder rleHolder(NULL);
	if(Os2Format){// convert OS2 DIB to Win DIB
		Stream->Read(&(Os2Header.bcWidth), sizeof(Os2Header) - sizeof(HeaderSize));
		ZeroMemory(&(BitmapInfo->bmiHeader), sizeof(BitmapInfo->bmiHeader));
		BitmapInfo->bmiHeader.biWidth = Os2Header.bcWidth;
		BitmapInfo->bmiHeader.biHeight = Os2Header.bcHeight;
        BitmapInfo->bmiHeader.biPlanes = Os2Header.bcPlanes;
        BitmapInfo->bmiHeader.biBitCount = Os2Header.bcBitCount;
		ImageSize -= sizeof(Os2Header);
	}
	else{// support bitmap headers larger than TBitmapInfoHeader
		Stream->Read((LPVOID)((LPBYTE)BitmapInfo + sizeof(HeaderSize)), HeaderSize - sizeof(HeaderSize));
		ImageSize -= HeaderSize;
		if(BitmapInfo->bmiHeader.biCompression != BI_BITFIELDS && BitmapInfo->bmiHeader.biCompression != BI_RGB){
			// Preserve funky non-DIB data (like RLE) until modified
			RLEStream = new CMemoryStream();
			rleHolder.SwapObject(RLEStream);
			if(bmf == NULL){
				ZeroMemory(&vbmf, sizeof(vbmf));
				vbmf.bfType = 0x4D42;
				vbmf.bfSize = ImageSize + HeaderSize;
				bmf = &vbmf;
			}
			RLEStream->Write(bmf, sizeof(*bmf));
			RLEStream->Write(&HeaderSize, sizeof(HeaderSize));
			RLEStream->Write((LPVOID)((LPBYTE)BitmapInfo + sizeof(HeaderSize)), HeaderSize - sizeof(HeaderSize));
			RLEStream->CopyFrom(Stream, ImageSize);
			//{ Cast ImageSize (long word) to integer to avoid integer overflow when negating. }
			RLEStream->Seek(-((INT)ImageSize), soFromEnd);
			Stream = RLEStream;  // the rest of the proc reads from RLEStream
		}
	}
	BitmapInfo->bmiHeader.biSize = HeaderSize;
	ColorTable = (LPVOID)((LPBYTE)BitmapInfo + HeaderSize);
	if(BitmapInfo->bmiHeader.biPlanes != 1)//{ check number of planes. DIBs must be 1 color plane (packed pixels) }
		throw "Invalid Bitmap";
	// 3 DWORD color element bit masks (ie 888 or 565) can precede colors
    // TBitmapInfoHeader sucessors include these masks in the headersize
	if(HeaderSize == sizeof(BITMAPINFOHEADER) && 
		(BitmapInfo->bmiHeader.biBitCount == 16 || BitmapInfo->bmiHeader.biBitCount == 32) &&
		BitmapInfo->bmiHeader.biCompression == BI_BITFIELDS){
		Stream->ReadBuffer(ColorTable, 3 * sizeof(DWORD));
		ColorTable = (LPBYTE)ColorTable + 3 * sizeof(DWORD);
		ImageSize -= 3 * sizeof(DWORD);
	}
	// Read the color palette
	if(BitmapInfo->bmiHeader.biClrUsed == 0)
		BitmapInfo->bmiHeader.biClrUsed = GetDInColors(BitmapInfo->bmiHeader.biBitCount);
	Stream->ReadBuffer(ColorTable, BitmapInfo->bmiHeader.biClrUsed * DIBPalSizes[Os2Format]);
	ImageSize -= BitmapInfo->bmiHeader.biClrUsed * DIBPalSizes[Os2Format];
	// biSizeImage can be zero. If zero, compute the size.
	if(BitmapInfo->bmiHeader.biSizeImage == 0)// top-down DIBs have negative height
		BitmapInfo->bmiHeader.biSizeImage = BytesPerScanline(BitmapInfo->bmiHeader.biWidth, 
			BitmapInfo->bmiHeader.biBitCount, 32) * abs(BitmapInfo->bmiHeader.biHeight);
	if(BitmapInfo->bmiHeader.biSizeImage < (DWORD)ImageSize)
		ImageSize = BitmapInfo->bmiHeader.biSizeImage;
	//{ convert OS2 color table to DIB color table }
    if(Os2Format)
		RGBTripleToQuad(ColorTable);
	HDC DC = (HDC)GDICheck(GetDC(0));
	DCHolder dcHolder(DC, 0, (HWND)0);
	LPVOID BitsMem = NULL;
	if(BitmapInfo->bmiHeader.biCompression != BI_RGB && BitmapInfo->bmiHeader.biCompression != BI_BITFIELDS
		|| GGlobal.DDBsOnly){
		BitsMem = malloc(ImageSize);
		BufferHolder bitsMemHolder(BitsMem);
		Stream->ReadBuffer(BitsMem, ImageSize);
        HDC MemDC = (HDC)GDICheck(CreateCompatibleDC(DC));
		DCHolder mdcHolder(MemDC);
		HBITMAP hBMP = CreateCompatibleBitmap(DC, 1, 1);
		GDIOBJHolder bmpHolder(hBMP);
		HBITMAP OldBMP = (HBITMAP)SelectObject(MemDC, hBMP);
		GDIOBJRestorer oldBMPRestorer(MemDC, OldBMP);
		PALETTERestorer oldPalRestorer(MemDC, 0, TRUE);
		HPALETTE OldPal = 0;
		if(BitmapInfo->bmiHeader.biClrUsed > 0){
			Pal = PaletteFromDIBColorTable(0, ColorTable, BitmapInfo->bmiHeader.biClrUsed);
			OldPal = SelectPalette(MemDC, Pal, FALSE);
			oldPalRestorer.SwapRestore(OldPal);
			RealizePalette(MemDC);
		}
		BMHandle = CreateDIBitmap(MemDC, &(BitmapInfo->bmiHeader), CBM_INIT, BitsMem,
            BitmapInfo, DIB_RGB_COLORS);
		if(BMHandle == 0){
			if(GetLastError() == 0)
				throw "Invalid Bitmap";
			else 
				RaiseLastOSError();
		}
	}
	else {
		BMHandle = CreateDIBSection(DC, BitmapInfo, DIB_RGB_COLORS, &BitsMem, 0, 0);
		if(BMHandle == 0 || BitsMem == NULL){
			if(GetLastError() == 0)
				throw "Invalid Bitmap";
			else
				RaiseLastOSError();
		}
		GDIOBJHolder bmhHolder(BMHandle);
		Stream->ReadBuffer(BitsMem, ImageSize);
		bmhHolder.SwapObject(0);
	}
	// Hi-color DIBs don't preserve color table, so create palette now
    if(BitmapInfo->bmiHeader.biBitCount > 8 && BitmapInfo->bmiHeader.biClrUsed > 0 && Pal == 0)
		Pal = PaletteFromDIBColorTable(0, ColorTable, BitmapInfo->bmiHeader.biClrUsed);
	DIBSECTION DIB;
	ZeroMemory(&DIB, sizeof(DIB));
	GetObject(BMHandle, sizeof(DIB), &DIB);
	// GetObject / CreateDIBSection don't preserve these info values
	DIB.dsBmih.biXPelsPerMeter = BitmapInfo->bmiHeader.biXPelsPerMeter;
    DIB.dsBmih.biYPelsPerMeter = BitmapInfo->bmiHeader.biYPelsPerMeter;
    DIB.dsBmih.biClrUsed = BitmapInfo->bmiHeader.biClrUsed;
    DIB.dsBmih.biClrImportant = BitmapInfo->bmiHeader.biClrImportant;
	NewImage(BMHandle, Pal, DIB, Os2Format, RLEStream);
	SetPaletteModified(GetPalette() != 0);
	Changed(this);
}

BOOL CBitmap::TransparentColorStored(){
	return TransparentMode == tmFixed;
}

void CBitmap::WriteStream(CStream* Stream, BOOL WriteSize){
	BITMAPFILEHEADER BMF;
	ZeroMemory(&BMF, sizeof(BMF));
	BMF.bfType = 0x4D42;
	if(Image->SaveStream != NULL){
		DWORD Size = (DWORD)Image->SaveStream->GetSize();
		if(WriteSize)
			Stream->WriteBuffer(&Size, sizeof(Size));
		Stream->Write(Image->SaveStream->GetMemory(), Image->SaveStream->GetSize());
		return;
	}
	DIBNeeded();
	DWORD Size = 0;
	DWORD ColorCount = 0;
	DWORD HeaderSize = 0;
	BITMAPCOREHEADER BC;
	ZeroMemory(&BC, sizeof(BC));
	RGBQUAD Colors[256];
	if(Image->DIBHandle != 0){
		InternalGetDIBSizes(Image->DIBHandle, HeaderSize, Size, Image->FDIB.dsBmih.biClrUsed);
		if(Image->OS2Format){// OS2 format cannot have partial palette
			HeaderSize = sizeof(BC);
			if(Image->FDIB.dsBmih.biBitCount <= 8)
				HeaderSize += sizeof(RGBTRIPLE) * ((ULONG_PTR)1 << Image->FDIB.dsBmih.biBitCount);
		}
		Size += HeaderSize + sizeof(BMF);
		ZeroMemory(&BMF, sizeof(BMF));
		BMF.bfType = 0x4D42;
		GetCanvas()->RequiredState(csHandleValid);
		HGDIOBJ Save = GDICheck(SelectObject(Canvas->GetDC(), Image->DIBHandle));
		ColorCount = GetDIBColorTable(Canvas->GetDC(), 0, 256, Colors);
		SelectObject(Canvas->GetDC(), Save);
		// GetDIBColorTable always reports the full palette; trim it back for partial palettes
		if (0 < Image->FDIB.dsBmih.biClrUsed && Image->FDIB.dsBmih.biClrUsed < ColorCount)
			ColorCount = Image->FDIB.dsBmih.biClrUsed;
		if (!Image->OS2Format && ColorCount == 0 && Image->Palette != 0 && !Image->Halftone){
			ColorCount = PaletteToDIBColorTable(Image->Palette, Colors, 256);
			if(Image->FDIB.dsBmih.biBitCount > 8){// optional color palette for hicolor images (non OS2)
				Size += ColorCount * sizeof(RGBQUAD);
				HeaderSize += ColorCount * sizeof(RGBQUAD);
			}
		}
		BMF.bfSize = Size;
		BMF.bfOffBits = sizeof(BMF) + HeaderSize;
	}
	if(WriteSize)
		Stream->WriteBuffer(&Size, sizeof(Size));
	if(Size > 0){
		FixupBitFields(&(Image->FDIB));
		if(ColorCount != 0){
			if(Image->FDIB.dsBmih.biClrUsed == 0 || Image->FDIB.dsBmih.biClrUsed != ColorCount)
				Image->FDIB.dsBmih.biClrUsed = ColorCount;
			if(Image->OS2Format)
				RGBQuadToTriple(Colors, (INT)ColorCount);
		}
		if(Image->OS2Format){
			BC.bcSize = sizeof(BC);
			BC.bcWidth = (WORD)Image->FDIB.dsBmih.biWidth;
			BC.bcHeight = (WORD)Image->FDIB.dsBmih.biHeight;
			BC.bcPlanes = 1;
			BC.bcBitCount = Image->FDIB.dsBmih.biBitCount;
			Stream->WriteBuffer(&BMF, sizeof(BMF));
			Stream->WriteBuffer(&BC, sizeof(BC));
		}
		else{
			Stream->WriteBuffer(&BMF, sizeof(BMF));
			Stream->WriteBuffer(&(Image->FDIB.dsBmih), sizeof(Image->FDIB.dsBmih));
			if(Image->FDIB.dsBmih.biBitCount > 8 && (Image->FDIB.dsBmih.biCompression & BI_BITFIELDS) != 0)
				Stream->WriteBuffer(&(Image->FDIB.dsBitfields), 12);
		}
		BYTE PalSize[] = {sizeof(RGBQUAD), sizeof(RGBTRIPLE)};
		Stream->WriteBuffer(Colors, ColorCount * PalSize[Image->OS2Format]);
		Stream->WriteBuffer(Image->FDIB.dsBm.bmBits, Image->FDIB.dsBmih.biSizeImage);
	}
}

void CBitmap::Changed(CObject* Sender){
	MaskBitsValid = FALSE;
	__super::Changed(Sender);
}

void CBitmap::Draw(CCanvas* ACanvas, const TRect& Rect){
	ACanvas->RequiredState(csAllValid);
	PaletteNeeded();
	PALETTERestorer palRestorer(ACanvas->GetDC(), 0, TRUE);
	if(Image->Palette != 0){
		HPALETTE OldPalette = SelectPalette(ACanvas->GetDC(), Image->Palette, TRUE);
		palRestorer.SwapRestore(OldPalette);
		RealizePalette(ACanvas->GetDC());
	}
	INT BPP = GetDeviceCaps(ACanvas->GetDC(), BITSPIXEL) * GetDeviceCaps(ACanvas->GetDC(), PLANES);
	BOOL DoHalftone = BPP <= 8 && BPP < (Image->FDIB.dsBm.bmBitsPixel * Image->FDIB.dsBm.bmPlanes);
	TPoint pt;
	if(DoHalftone){
		GetBrushOrgEx(ACanvas->GetDC(), &pt);
		SetStretchBltMode(ACanvas->GetDC(), HALFTONE);
		SetBrushOrgEx(ACanvas->GetDC(), pt.x, pt.y, &pt);
	}
	else if(GetMonochrome())
		SetStretchBltMode(ACanvas->GetHandle(), STRETCH_DELETESCANS);
	//{ Call MaskHandleNeeded prior to creating the canvas handle since
	//it causes FreeContext to be called. }
	if(GetTransparent())
		MaskHandleNeeded();
	GetCanvas()->RequiredState(csAllValid);
	if(GetTransparent()){
		HDC MaskDC = (HDC)GDICheck(CreateCompatibleDC(0));
		DCHolder maskDcHolder(MaskDC);
        HGDIOBJ Save = SelectObject(MaskDC, Image->MaskHandle);
		maskDcHolder.SwapRestore(Save);
		TransparentStretchBlt(ACanvas->GetDC(), Rect.left, Rect.top, Rect.right - Rect.left,
            Rect.bottom - Rect.top, GetCanvas()->GetDC(), 0, 0, Image->FDIB.dsBm.bmWidth,
            Image->FDIB.dsBm.bmHeight, MaskDC, 0, 0);
	}
	else
        StretchBlt(ACanvas->GetDC(), Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top,
          GetCanvas()->GetDC(), 0, 0, Image->FDIB.dsBm.bmWidth, Image->FDIB.dsBm.bmHeight, ACanvas->GetCopyMode());
}

BOOL CBitmap::GetEmpty(){
	return Image->Handle == 0 && Image->DIBHandle == 0 && Image->SaveStream == NULL;
}

INT CBitmap::GetHeight(){
	return abs(Image->FDIB.dsBm.bmHeight);
}

HPALETTE CBitmap::GetPalette(){
	PaletteNeeded();
	return Image->Palette;
}

INT CBitmap::GetWidth(){
	return Image->FDIB.dsBm.bmWidth;
}

void CBitmap::MaskHandleNeeded(){
	if(MaskValid && MaskBitsValid)
		return;
	//{ Delete existing mask if any }
	if(Image->MaskHandle != 0){
		DeselectBitmap(Image->MaskHandle);
		DeleteObject(Image->MaskHandle);
		Image->MaskHandle = 0;
	}
    FreeContext();
    HandleNeeded();  // may change FImage instance pointer
	// use new FImage from here on
	Image->MaskHandle = CopyBitmapAsMask(Image->Handle, Image->Palette, GetTransparentColor());
	MaskValid = TRUE;
	MaskBitsValid = TRUE;
}

void CBitmap::ReadData(CStream* Stream){
	DWORD Size = 0;
	Stream->Read(&Size, sizeof(Size));
	ReadStream(Stream, Size);
}

void CBitmap::SetHeight(INT Value){
	if(Image->FDIB.dsBm.bmHeight != Value){
		HandleNeeded();
		DIBSECTION DIB = Image->FDIB;
		DIB.dsBm.bmHeight = Value;
		DIB.dsBmih.biHeight = Value;
		CopyImage(Image->Handle, Image->Palette, DIB);
		Changed(this);
	}
}

void CBitmap::SetPalette(HPALETTE Value){
	if(Image->Palette != Value){
		if(Value == 0 && Image->GetRefCount() == 1){
			InternalDeletePalette(Image->Palette);
			Image->Palette = 0;
		}
		else{
			FreeContext();
			HandleNeeded();
			DIBSECTION DIB = Image->FDIB;
			HBITMAP AHandle = CopyBitmap(Image->Handle, Image->Palette, Value, &DIB, NULL);
			GDIOBJHolder hdHolder(AHandle);
			NewImage(AHandle, Value, DIB, Image->OS2Format);
			hdHolder.SwapObject(0);
		}
	}
    UpdateDIBColorTable(Image->DIBHandle, Value, Image->FDIB);
    SetPaletteModified(TRUE);
	Changed(this);
}

void CBitmap::SetWidth(INT Value){
	if(Image->FDIB.dsBm.bmWidth != Value){
		HandleNeeded();
		DIBSECTION DIB = Image->FDIB;
		DIB.dsBm.bmWidth = Value;
		DIB.dsBmih.biWidth = Value;
		CopyImage(Image->Handle, Image->Palette, DIB);
		Changed(this);
	}
}

void CBitmap::WriteData(CStream* Stream){
	WriteStream(Stream, TRUE);
}

void CBitmap::HandleNeeded(){
	if(Image->Handle == 0 && Image->DIBHandle == 0 && Image->SaveStream != NULL){
		Image->SaveStream->SetPosition(0);
		TNotifyEvent vChange = GetOnChange();
		CObject* obj = GetOnChangeObj();
		__try{
			SetOnChange(NULL, NULL);
			LoadFromStream(Image->SaveStream);  // Current FImage may be destroyed here
		}
		__finally{
			SetOnChange(obj, vChange);
		}
	}
	if(Image->Handle == 0)
		Image->Handle = Image->DIBHandle;
}

void CBitmap::PaletteNeeded(){
	if(IgnorePalette || Image->Palette != 0 || Image->DIBHandle == 0)
		return ;
	if(Image->Handle == Image->DIBHandle)
		DeselectBitmap(Image->DIBHandle);
	Image->Palette = PaletteFromDIBColorTable(Image->DIBHandle, NULL, 1 << Image->FDIB.dsBmih.biBitCount);
	if(Image->Palette != 0)
		return ;
	HDC DC = (HDC)GDICheck(GetDC(0));
	Image->Halftone = Image->Halftone ||
		((GetDeviceCaps(DC, BITSPIXEL) * GetDeviceCaps(DC, PLANES)) <
		(Image->FDIB.dsBm.bmBitsPixel * Image->FDIB.dsBm.bmPlanes));
	if(Image->Halftone)
		Image->Palette = CreateHalftonePalette(DC);
    ReleaseDC(0, DC);
    if(Image->Palette == 0)
		IgnorePalette = TRUE;
}

void CBitmap::Assign(CObject* Source){
	if(Source == NULL || Source->InstanceOf(CBitmap::_Class)){
		CCriticalSectionLock csLock(&GGlobal.BitmapImageLock);
		if(Source != NULL){
			((CBitmap *)Source)->Image->Reference();
			Image->Release();
			Image = ((CBitmap *)Source)->Image;
			Transparent = ((CBitmap *)Source)->Transparent;
			TransparentColor = ((CBitmap *)Source)->TransparentColor;
			TransparentMode = ((CBitmap *)Source)->TransparentMode;
		}
		else {
			DIBSECTION DIB;
			ZeroMemory(&DIB, sizeof(DIB));
			NewImage(0, 0, DIB, FALSE);
		}
		SetPaletteModified(GetPalette() != 0);
		Changed(this);
	}
	else 
		__super::Assign(Source);
}

void CBitmap::Dormant(){
	CMemoryStream* s = new CMemoryStream();
	SaveToStream(s);
	s->SetSize(s->GetSize());  // compact to minimum buffer
	DIBSECTION DIB = Image->FDIB;
	DIB.dsBm.bmBits = NULL;
	FreeContext(); // InternalDeletePalette requires this
	FreeAndNil((CObject **)&Canvas);
	NewImage(0, 0, DIB, Image->OS2Format, s);
}

/*FreeImage:
  If there are multiple references to the image, create a unique copy of the image.
  If FHandle = FDIBHandle, the DIB memory will be updated when the drawing
  handle is drawn upon, so no changes are needed to maintain image integrity.
  If FHandle <> FDIBHandle, the DIB will not track with changes made to
  the DDB, so destroy the DIB handle (but keep the DIB pixel format info).  //*/
void CBitmap::FreeImage(){
	if(Image->GetRefCount() > 1){
		HandleNeeded();
		HPALETTE P = 0;
		if(Image->Halftone) 
			P = 0;
		else
			P = Image->Palette;
		CopyImage(Image->Handle, P, Image->FDIB);
	}
    else if(Image->Handle != 0 && Image->Handle != Image->DIBHandle){
		if(Image->DIBHandle != 0)
			if(!DeleteObject(Image->DIBHandle))
				GDIError();
		Image->DIBHandle = 0;
		Image->FDIB.dsBm.bmBits = NULL;
	}
}

BOOL CBitmap::HandleAllocated(){
	return Image != NULL && Image->Handle != 0;
}

void CBitmap::LoadFromClipboardFormat(WORD AFormat, HANDLE AData, HPALETTE APalette){
	if(AFormat != CF_BITMAP || AData == 0)
		throw "Unknown clipboard format.";
	FreeContext();
	DIBSECTION DIB;
	ZeroMemory(&DIB, sizeof(DIB));
	GetObject(AData, sizeof(DIB), &DIB);
	if(DIB.dsBm.bmBits == NULL)
		DIB.dsBmih.biSize = 0;
	CopyImage((HBITMAP)AData, APalette, DIB);
	Image->OS2Format = FALSE;
	SetPaletteModified(GetPalette() != 0);
	Changed(this);
}

void CBitmap::LoadFromStream(CStream* Stream){
	ReadStream(Stream, (LONG)(Stream->GetSize() - Stream->GetPosition()));
}

void CBitmap::LoadFromResourceName(HANDLE Instance, const LPTSTR ResName){
	CCustomMemoryStream* Stream = new CResourceStream(Instance, ResName, RT_BITMAP);
	CObjectHolder holder(Stream);
    ReadDIB(Stream, (LONG)Stream->GetSize());
}

void CBitmap::LoadFromResourceID(HANDLE Instance, INT ResID){
	CCustomMemoryStream* Stream = new CResourceStream(Instance, ResID, RT_BITMAP);
	CObjectHolder holder(Stream);
    ReadDIB(Stream, (LONG)Stream->GetSize());
}

void CBitmap::Mask(TColor TransparentColor){
	InternalPaletteHolder palHolder;
	GDIOBJHolder hdHolder;
	FreeContext();
    HandleNeeded();
    HBITMAP NewHandle = CopyBitmapAsMask(Image->Handle, Image->Palette,
		ColorToRGB(TransparentColor));
	hdHolder.SwapObject(NewHandle);
	DIBSECTION DIB;
	ZeroMemory(&DIB, sizeof(DIB));
	GetObject(NewHandle, sizeof(DIB), &DIB);
	HPALETTE NewPalette = 0;
	if(Image->Palette == GGlobal.SystemPalette16)
		NewPalette = Image->Palette;
    else
		NewPalette = CopyPalette(Image->Palette);
	palHolder.SwapPalette(NewPalette);
    NewImage(NewHandle, NewPalette, DIB, Image->OS2Format);
	palHolder.SwapPalette(0);
	hdHolder.SwapObject(0);
	Changed(this);
}

HBITMAP CBitmap::ReleaseHandle(){
	HandleNeeded();
	Changing(this);
	HBITMAP Result = Image->Handle;
    if(Image->Handle == Image->DIBHandle){
		Image->DIBHandle = 0;
		Image->FDIB.dsBm.bmBits = NULL;
	}
	Image->Handle = 0;
	return Result;
}

HBITMAP CBitmap::ReleaseMaskHandle(){
	HBITMAP Result = GetMaskHandle();
	Image->MaskHandle = 0;
	return Result;
}

HPALETTE CBitmap::ReleasePalette(){
	HandleNeeded();
	Changing(this);
	HPALETTE Result = Image->Palette;
	Image->Palette = 0;
	return Result;
}

void CBitmap::SaveToClipboardFormat(WORD& Format, HANDLE& Data, HPALETTE& APalette){
	Format = CF_BITMAP;
	HandleNeeded();
	DIBSECTION DIB = Image->FDIB;
	DIB.dsBmih.biSize = 0;   // copy to device bitmap
    DIB.dsBm.bmBits = NULL;
	Data = CopyBitmap(Image->Handle, Image->Palette, Image->Palette, &DIB, Canvas);
	GDIOBJHolder dtHolder(Data);
	APalette = CopyPalette(Image->Palette);
	dtHolder.SwapObject(0);
}

void CBitmap::SaveToStream(CStream* Stream){
	WriteStream(Stream, FALSE);
}

CCanvas* CBitmap::GetCanvas(){
	if(Canvas == NULL){
		HandleNeeded();
		if(Canvas == NULL){// possible recursion
			Canvas = new CBitmapCanvas(this);
			Canvas->SetOnChange(this, (TNotifyEvent)&CBitmap::Changed);
			Canvas->SetOnChanging(this, (TNotifyEvent)&CBitmap::Changing);
		}
	}
	return Canvas;
}

/*Since the user might modify the contents of the HBITMAP it must not be
  shared by another TBitmap when given to the user nor should it be selected
  into a DC.//*/
HBITMAP CBitmap::GetHandle(){
	FreeContext();
	HandleNeeded();
	Changing(this);
	return Image->Handle;
}

void CBitmap::SetHandle(HBITMAP Value){
	if(Image->Handle != Value){
		FreeContext();
		DIBSECTION DIB;
		ZeroMemory(&DIB, sizeof(DIB));
		if(Value != 0)
			GetObject(Value, sizeof(DIB), &DIB);
		HPALETTE APalette = 0;
		if(Image->GetRefCount() == 1){
			APalette = Image->Palette;
			Image->Palette = 0;
		}
		else if(Image->Palette == GGlobal.SystemPalette16)
			APalette = GGlobal.SystemPalette16;
        else
			APalette = CopyPalette(Image->Palette);
		InternalPaletteHolder palHolder(APalette);
		NewImage(Value, APalette, DIB, FALSE);
		palHolder.SwapPalette(0);
		Changed(this);
	}
}

TBitmapHandleType CBitmap::GetHandleType(){
	if(Image->Handle == 0 || Image->Handle == Image->DIBHandle)
		if(Image->DIBHandle == 0)
			if(Image->FDIB.dsBmih.biSize == 0)
				return bmDDB;
			else
				return bmDIB;
		else
			return bmDIB;
	else
		return bmDDB;
}

void CBitmap::SetHandleType(TBitmapHandleType Value){
	if(Value == GetHandleType())
		return ;
	if(Image->Handle == 0 && Image->DIBHandle == 0)
		if(Value == bmDDB)
			Image->FDIB.dsBmih.biSize = 0;
		else
			Image->FDIB.dsBmih.biSize = sizeof(Image->FDIB.dsBmih);
	else {
		if(Value == bmDIB){
			if(Image->DIBHandle != 0 && Image->DIBHandle == Image->Handle)
				return;
			FreeContext();
			PaletteNeeded();
			DIBNeeded();
			if(Image->GetRefCount() == 1){
				HBITMAP AHandle = Image->DIBHandle;
				Image->DIBHandle = 0;
				HPALETTE NewPalette = Image->Palette;
				Image->Palette = 0;
				NewImage(AHandle, NewPalette, Image->FDIB, Image->OS2Format);
			}
			else
				CopyImage(Image->DIBHandle, Image->Palette, Image->FDIB);
		}
		else{
			if(Image->Handle != 0 && Image->Handle != Image->DIBHandle)
				return;
			FreeContext();
			PaletteNeeded();
			DIBSECTION DIB = Image->FDIB;
			DIB.dsBmih.biSize = 0;   // flag to tell CopyBitmap to create a DDB
			BOOL DoCopy = Image->GetRefCount() == 1;
			HPALETTE NewPalette = 0;
			if(DoCopy)
				NewPalette = Image->Palette;
			else
				NewPalette = CopyPalette(Image->Palette);
			HBITMAP AHandle = CopyBitmap(Image->DIBHandle, Image->Palette, NewPalette, &DIB, NULL);
			if(DoCopy)
				Image->Handle = AHandle;
			else
				NewImage(AHandle, NewPalette, DIB, Image->OS2Format);
		}
		Changed(this);
	}
}

HBITMAP CBitmap::GetMaskHandle(){
	MaskHandleNeeded();
	return Image->MaskHandle;
}

void CBitmap::SetMaskHandle(HBITMAP Value){
	if(Image->MaskHandle != Value){
		Image->MaskHandle = Value;
		MaskValid = TRUE;
		MaskBitsValid = TRUE;
	}
}

BOOL CBitmap::GetMonochrome(){
	return Image->FDIB.dsBm.bmPlanes == 1 && Image->FDIB.dsBm.bmBitsPixel == 1;
}

void CBitmap::SetMonochrome(BOOL Value){
	if(Value != (Image->FDIB.dsBmih.biPlanes == 1 && Image->FDIB.dsBmih.biBitCount == 1)){
		HandleNeeded();
		DIBSECTION DIB = Image->FDIB;
		DIB.dsBmih.biSize = 0;// request DDB handle
		DIB.dsBmih.biPlanes = Value;  // 0 = request screen BMP format
        DIB.dsBmih.biBitCount = Value;
        DIB.dsBm.bmPlanes = Value;
        DIB.dsBm.bmBitsPixel = Value;
		CopyImage(Image->Handle, Image->Palette, DIB);
		Changed(this);
	}
}

TPixelFormat CBitmap::GetPixelFormat(){
	TPixelFormat Result = pfCustom;
	if(GetHandleType() == bmDDB)
		Result = pfDevice;
	else{
		switch(Image->FDIB.dsBmih.biBitCount){
			case 1:
				Result = pf1Bit;
				break;
			case 4: 
				Result = pf4Bit;
				break;
			case 8: 
				Result = pf8Bit;
				break;
			case 16: 
				if(Image->FDIB.dsBmih.biCompression == BI_RGB)
					Result = pf15Bit;
				else if(Image->FDIB.dsBmih.biCompression == BI_BITFIELDS)
					if(Image->FDIB.dsBitfields[1] == 0x7E0)
						Result = pf16Bit;
				break;
			case 24:
				Result = pf24Bit;
				break;
			case 32:
				if(Image->FDIB.dsBmih.biCompression == BI_RGB)
					Result = pf32Bit;
		}
	}
	return Result;
}

void CBitmap::SetPixelFormat(TPixelFormat Value){
	if(Value == GetPixelFormat())
		return ;
	
	switch(Value){
		case pfDevice:
			SetHandleType(bmDDB);
			return;
		case pfCustom: 
			throw "invalid pixel format";
		default:{
			DIBSECTION DIB;
			ZeroMemory(&DIB, sizeof(DIB));
			DIB.dsBm = Image->FDIB.dsBm;
			BYTE BitCounts[] = {1,4,8,16,16,24,32};//pf1Bit..pf32Bit
			GDIOBJHolder palHolder;
			DIB.dsBm.bmBits = NULL;
			DIB.dsBmih.biSize = sizeof(DIB.dsBmih);
			DIB.dsBmih.biWidth = DIB.dsBm.bmWidth;
			DIB.dsBmih.biHeight = DIB.dsBm.bmHeight;
			DIB.dsBmih.biPlanes = 1;
			DIB.dsBmih.biBitCount = BitCounts[Value - pf1Bit];
			HPALETTE Pal = Image->Palette;
			switch(Value){
				case pf4Bit: 
					Pal = GGlobal.SystemPalette16;
					break;
				case pf8Bit:{
					HDC DC = (HDC)GDICheck(GetDC(0));
					Pal = CreateHalftonePalette(DC);
					palHolder.SwapObject(Pal);
					ReleaseDC(0, DC);
				}
				case pf16Bit:{
					DIB.dsBmih.biCompression = BI_BITFIELDS;
					DIB.dsBitfields[0] = 0xF800;
					DIB.dsBitfields[1] = 0x07E0;
					DIB.dsBitfields[2] = 0x001F;
				}
			}
			CopyImage(GetHandle(), Pal, DIB);
			SetPaletteModified(Pal != 0);
			Changed(this);
		}
	}
}

LPVOID CBitmap::GetScanline(INT Row){
	Changing(this);
	if(Row < 0 || Row >= Image->FDIB.dsBm.bmHeight)
		throw "invalid scan line operation.";//InvalidOperation(@SScanLine);
	DIBNeeded();
	GdiFlush();
	if(Image->FDIB.dsBmih.biHeight > 0)// bottom-up DIB
		Row = Image->FDIB.dsBmih.biHeight - Row - 1;
    return (LPVOID)((LPBYTE)(Image->FDIB.dsBm.bmBits) + Row * BytesPerScanline(
		Image->FDIB.dsBmih.biWidth, Image->FDIB.dsBmih.biBitCount, 32));
}

TColor CBitmap::GetTransparentColor(){
	TColor Result = 0;
	if(TransparentColor == clDefault){
		if(GetMonochrome())
			Result = clWhite;
		else
			Result = GetCanvas()->GetPixel(0, GetHeight() - 1);
	}
	else 
		Result = ColorToRGB(TransparentColor);
	Result |= 0x02000000;
	return Result;
}

void CBitmap::SetTransparentColor(TColor Value){
	if(Value != TransparentColor){
		if(Value == clDefault)
			TransparentMode = tmAuto;
		else
			TransparentMode = tmFixed;
		TransparentColor = Value;
		if(Image->GetRefCount() > 1){
			HandleNeeded();
			CopyImage(Image->Handle, Image->Palette, Image->FDIB);
		}
		Changed(this);
	}
}

void CBitmap::SetTransparentMode(TTransparentMode Value){
	if(Value != TransparentMode){
		if(Value == tmAuto)
			SetTransparentColor(clDefault);
		else
			SetTransparentColor(GetTransparentColor());
	}
}

IMPL_DYN_CLASS(CIconImage)
CIconImage::CIconImage():Handle(0),
	MemoryImage(NULL){
	Size.x = 0;
	Size.y = 0;
}
CIconImage::~CIconImage(){
	delete MemoryImage;
}

void CIconImage::FreeHandle(){
	if(Handle != 0)
		DestroyIcon(Handle);
	Handle = 0;
}

IMPL_DYN_CLASS(CIcon)
CIcon::CIcon(){
	Transparent = TRUE;
	Image = new CIconImage();
	Image->Reference();
	RequestedSize.x = 0;
	RequestedSize.y = 0;
}

CIcon::~CIcon(){
	Image->Release();
}

void ReadIcon(CStream* Stream, HICON& Icon, INT ImageCount, INT StartOffset, const TPoint& RequestedSize,
	TPoint& IconSize){
	typedef TIconRec TIconRecArray[300];
	typedef TIconRecArray *PIconRecArray;

	INT HeaderLen = sizeof(TIconRec) * ImageCount;
	PIconRecArray List = (PIconRecArray)malloc(HeaderLen);
	BufferHolder ListHolder(List);
	ZeroMemory(List, HeaderLen);
	Stream->Read(List, HeaderLen);
	if((RequestedSize.x | RequestedSize.y) == 0){
		IconSize.x = GetSystemMetrics(SM_CXICON);
		IconSize.y = GetSystemMetrics(SM_CYICON);
	}
	else
		IconSize = RequestedSize;
	HDC DC = GetDC(0);
    if(DC == 0)
		throw "Out of system resources";
	WORD BitsPerPixel = 0;
	INT Colors = 0;
	{
		DCHolder dcHolder(DC, 0, (HWND)0);
		BitsPerPixel = GetDeviceCaps(DC, PLANES) * GetDeviceCaps(DC, BITSPIXEL);
		if (BitsPerPixel > 8)
			Colors = MAXINT;
		else
			Colors = 1 << BitsPerPixel;
	}
	//Find the image that most closely matches (<=) the current screen color
    //depth and the requested image size.
	INT Index = 0;
    INT BestColor = AdjustColor((*List)[0].Colors);
	for(INT N = 1; N < ImageCount; N++){
		INT C1 = AdjustColor((*List)[N].Colors);
		if(C1 <= Colors && C1 >= BestColor &&
			BetterSize((*List)[Index], (*List)[N], IconSize)){
			Index = N;
			BestColor = C1;
		}
	}
	IconSize.x = (*List)[Index].Width;
	IconSize.y = (*List)[Index].Height;
	LPBITMAPINFOHEADER BI = (LPBITMAPINFOHEADER)malloc((*List)[Index].DIBSize);
	BufferHolder biHolder(BI);
	ZeroMemory(BI, (*List)[Index].DIBSize);
	Stream->Seek((*List)[Index].DIBOffset - (HeaderLen + StartOffset), 1);
	Stream->Read(BI, (*List)[Index].DIBSize);
	HBITMAP XorBits, AndBits;
	TwoBitsFromDIB(*BI, XorBits, AndBits, IconSize);
	BITMAP XorInfo, AndInfo;
    GetObject(AndBits, sizeof(BITMAP), &AndInfo);
    GetObject(XorBits, sizeof(BITMAP), &XorInfo);
	INT AndLen = AndInfo.bmWidthBytes * AndInfo.bmHeight * AndInfo.bmPlanes;
	INT XorLen = XorInfo.bmWidthBytes * XorInfo.bmHeight * XorInfo.bmPlanes;
	INT Length = AndLen + XorLen;
	LPVOID ResData = malloc(Length);
	BufferHolder rdHodler(ResData);
	ZeroMemory(ResData, Length);
	LPVOID AndMem = ResData;
	LPVOID XorMem = (LPVOID)((LONG_PTR)ResData + AndLen);
	GetBitmapBits(AndBits, AndLen, AndMem);
    GetBitmapBits(XorBits, XorLen, XorMem);
    DeleteObject(XorBits);
    DeleteObject(AndBits);
	Icon = CreateIcon(GetGlobal().GetHInstance(), IconSize.x, IconSize.y,
            (BYTE)XorInfo.bmPlanes, (BYTE)XorInfo.bmBitsPixel, (const BYTE *)AndMem, (const BYTE *)XorMem);
	if(Icon == 0)
		GDIError();
}

void WinError(){}

void CheckBool(BOOL Result){
	if(!Result)
		WinError();
}

void WriteIcon(CStream* Stream, HICON Icon, BOOL WriteLength){
	TCursorOrIcon CI;
	TIconRec List;
	ZeroMemory(&CI, sizeof(CI));
	ZeroMemory(&List, sizeof(List));
	ICONINFO IconInfo;
	CheckBool(GetIconInfo(Icon, &IconInfo));
	GDIOBJHolder colorHolder(IconInfo.hbmColor);
	GDIOBJHolder maskHolder(IconInfo.hbmMask);
	DWORD MonoInfoSize, ColorInfoSize;
	DWORD MonoBitsSize, ColorBitsSize;
    InternalGetDIBSizes(IconInfo.hbmMask, MonoInfoSize, MonoBitsSize, 2);
    InternalGetDIBSizes(IconInfo.hbmColor, ColorInfoSize, ColorBitsSize, 16);
    LPVOID MonoInfo = malloc(MonoInfoSize);
	BufferHolder mnInfoHolder(MonoInfo);
	ZeroMemory(MonoInfo, MonoInfoSize);
    LPVOID MonoBits = malloc(MonoBitsSize);
	BufferHolder mnBitsHolder(MonoBits);
	ZeroMemory(MonoBits, MonoBitsSize);
    LPVOID ColorInfo = malloc(ColorInfoSize);
	BufferHolder clrInfoHolder(ColorInfo);
	ZeroMemory(ColorInfo, ColorInfoSize);
    LPVOID ColorBits = malloc(ColorBitsSize);
	BufferHolder clrBitsHolder(ColorBits);
	ZeroMemory(ColorBits, ColorBitsSize);
    InternalGetDIB(IconInfo.hbmMask, 0, *((PBITMAPINFO)MonoInfo), MonoBits, 2);
    InternalGetDIB(IconInfo.hbmColor, 0, *((PBITMAPINFO)ColorInfo), ColorBits, 16);
	if(WriteLength){
		INT Length = sizeof(CI) + sizeof(List) + ColorInfoSize +
			ColorBitsSize + MonoBitsSize;
        Stream->Write(&Length, sizeof(Length));
	}
	CI.wType = RC3_ICON;
	CI.Count = 1;
	Stream->Write(&CI, sizeof(CI));
	PBITMAPINFOHEADER bmpColorInfo = (PBITMAPINFOHEADER)ColorInfo;
	List.Width = (BYTE)bmpColorInfo->biWidth;
	List.Height = (BYTE)bmpColorInfo->biHeight;
	List.Colors = bmpColorInfo->biPlanes * bmpColorInfo->biBitCount;
	List.DIBSize = ColorInfoSize + ColorBitsSize + MonoBitsSize;
	List.DIBOffset = sizeof(CI) + sizeof(List);
	Stream->Write(&List, sizeof(List));
	bmpColorInfo->biHeight += bmpColorInfo->biHeight;//{ color height includes mono bits }
	Stream->Write(ColorInfo, ColorInfoSize);
	Stream->Write(ColorBits, ColorBitsSize);
	Stream->Write(MonoBits, MonoBitsSize);
}


void CIcon::HandleNeeded(){
	if(Image->Handle != 0)
		return;
	if(Image->MemoryImage == NULL)
		return;
	TCursorOrIcon CI;
	Image->MemoryImage->SetPosition(0);
	Image->MemoryImage->ReadBuffer(&CI, sizeof(CI));
	HICON NewHandle = 0;
	switch(CI.wType){
		case RC3_STOCKICON: 
			NewHandle = GGlobal.StockIcon;
			break;
		case RC3_ICON: 
			ReadIcon(Image->MemoryImage, NewHandle, CI.Count, sizeof(CI), RequestedSize, Image->Size);
			break;
		default:
			throw "invalid icon";
	}
	Image->Handle = NewHandle;
}

void CIcon::ImageNeeded(){
	if(Image->MemoryImage != NULL)
		return;
	if(Image->Handle == 0)
		throw "invalid Icon";
	CMemoryStream* ImageT = new CMemoryStream();
	CObjectHolder itHolder(ImageT);

	if(GetHandle() == GGlobal.StockIcon){
		TCursorOrIcon CI;
		ZeroMemory(&CI, sizeof(CI));
		ImageT->WriteBuffer(&CI, sizeof(CI));
	}
	else 
		WriteIcon(ImageT, GetHandle(), FALSE);
	itHolder.SwapObject(NULL);
	Image->MemoryImage = ImageT;
}

void CIcon::NewImage(HICON NewHandle, CMemoryStream* NewImage){
	CIconImage* AImage = new CIconImage();
	CObjectHolder aiHolder(AImage);
	AImage->Handle = NewHandle;
	AImage->MemoryImage = NewImage;
	aiHolder.SwapObject(NULL);
	AImage->Reference();
	Image->Release();
	Image = AImage;
}

void CIcon::Draw(CCanvas* ACanvas, const TRect& Rect){
	ACanvas->RequiredState(csHandleValid);
	DrawIconEx(ACanvas->GetHandle(), Rect.TopLeft.x, Rect.TopLeft.y, GetHandle(), 0, 0, 0, 0, DI_NORMAL);
}

BOOL CIcon::GetEmpty(){
	return Image->Handle == 0 && Image->MemoryImage == NULL;
}

INT CIcon::GetHeight(){
	INT Result = Image->Size.y;
	if(Result == 0)
		Result = GetSystemMetrics(SM_CYICON);
	return Result;
}

INT CIcon::GetWidth(){
	INT Result = Image->Size.x;
	if(Result == 0)
		Result = GetSystemMetrics(SM_CXICON);
	return Result;
}

void CIcon::SetHeight(INT Value){
	if(Image->Handle == 0)
		RequestedSize.y = Value;
	else
		throw "Cannot change the size of an icon";
}

void CIcon::SetTransparent(BOOL Value){
	// Ignore assignments to this property.
	// Icons are always transparent.
}

void CIcon::SetWidth(INT Value){
	if(Image->Handle == 0)
		RequestedSize.x = Value;
	else
		throw "Cannot change the size of an icon";

}

void CIcon::Assign(CIcon* Source){
	if(Source != NULL){
		Source->Image->Reference();
		Image->Release();
		Image = Source->Image;
	}
	else
		NewImage(0, NULL);
	Changed(this);
}

BOOL CIcon::HandleAllocated(){
	return Image != NULL && Image->Handle != 0;
}

void CIcon::LoadFromClipboardFormat(WORD AFormat, HANDLE AData, HPALETTE APalette){
	throw "Clipboard does not support Icons";
}

void CIcon::LoadFromStream(CStream* Stream){
	CMemoryStream* AImage = new CMemoryStream();
	CObjectHolder aiHolder(AImage);
	AImage->SetSize(Stream->GetSize() - Stream->GetPosition());
	Stream->ReadBuffer(AImage->GetMemory(), AImage->GetSize());
	TCursorOrIcon CI;
	AImage->ReadBuffer(&CI, sizeof(CI));
	if(!IN_TEST(CI.wType, (RC3_STOCKICON | RC3_ICON)))
		throw "invalid Icon";
	NewImage(0, AImage);
	aiHolder.SwapObject(NULL);
	Changed(this);
}

HICON CIcon::ReleaseHandle(){
	if(Image->GetRefCount() > 1)
		NewImage(CopyIcon(Image->Handle), NULL);
	HICON Result = Image->Handle;
	Image->Handle = 0;
	Changed(this);
	return Result;
}

void CIcon::SaveToClipboardFormat(WORD& Format, HANDLE& Data, HPALETTE& APalette){
	throw "Clipboard does not support Icons";
}

void CIcon::SaveToStream(CStream* Stream){
	ImageNeeded();
	Stream->WriteBuffer(Image->MemoryImage->GetMemory(), Image->MemoryImage->GetSize());
}

HICON CIcon::GetHandle(){
	HandleNeeded();
	return Image->Handle;
}

void CIcon::SetHandle(HICON Value){
	NewImage(Value, NULL);
	Changed(this);
}

IMPL_DYN_CLASS(CMetafileCanvas)
CMetafileCanvas::CMetafileCanvas(CMetafile* AMetafile, HDC ReferenceDevice,
	LPTSTR CreateBy, LPTSTR Description){
	Metafile = AMetafile;
	HDC RefDC = ReferenceDevice;
	DCHolder refDcHolder;
	if(ReferenceDevice == 0){
		RefDC = ::GetDC(0);
		refDcHolder.SwapDC(RefDC);
		refDcHolder.SwapRefWnd(0);
	}
    if(Metafile->GetMMWidth() == 0){
		if(Metafile->GetWidth() == 0)
			Metafile->SetMMWidth(GetDeviceCaps(RefDC, HORZSIZE)*100);
		else
			Metafile->SetMMWidth(MulDiv(Metafile->GetWidth(),
				GetDeviceCaps(RefDC, HORZSIZE)*100, GetDeviceCaps(RefDC, HORZRES)));
	}
    if(Metafile->GetMMHeight() == 0){
		if(Metafile->GetHeight() == 0)
			Metafile->SetMMHeight(GetDeviceCaps(RefDC, VERTSIZE)*100);
		else
			Metafile->SetMMHeight(MulDiv(Metafile->GetHeight(),
				GetDeviceCaps(RefDC, VERTSIZE)*100, GetDeviceCaps(RefDC, VERTRES)));
	}
    TRect R = Rect(0,0,Metafile->GetMMWidth(),Metafile->GetMMHeight());
	LPTSTR P = NULL;
	BufferHolder pHolder;
	INT L1 = CreateBy == NULL ? 0 : lstrlen(CreateBy);
	INT L2 = Description == NULL ? 0 : lstrlen(Description);
    if(L1 > 0 || L2 > 0){
		INT Len = L1 + L2 + 4;
		P = (LPTSTR)malloc(Len * sizeof(TCHAR));
		pHolder.SwapBuffer(P);
		ZeroMemory(P, Len * sizeof(TCHAR));
		if(L1 > 0)
			lstrcpyn(P, CreateBy, L1 + 1);
		if(L2 > 0)
			lstrcpyn(P + L1 + 1, Description, L2 + 1);
	}
	HDC Temp = (HDC)CreateEnhMetaFile(RefDC, NULL, (const RECT *)&R, P);
    if(Temp == 0)
		GDIError();
	SetHandle(Temp);
}

CMetafileCanvas::~CMetafileCanvas(){
	HDC Temp = GetHandle();
	SetHandle(0);
	Metafile->SetHandle(CloseEnhMetaFile(Temp));
}

IMPL_DYN_CLASS(CMetafileImage)
CMetafileImage::CMetafileImage():Handle(0),
	Width(0),
	Height(0),
	Palette(0),
	Inch(0),
	TempWidth(0),
	TempHeight(0){
}

CMetafileImage::~CMetafileImage(){
	if(Handle != 0)
		DeleteEnhMetaFile(Handle);
	if(Palette != 0)
		InternalDeletePalette(Palette);
}

void CMetafileImage::FreeHandle(){
}

#define HundredthMMPerInch 2540
IMPL_DYN_CLASS(CMetafile)
CMetafile::CMetafile():Image(NULL),Enhanced(TRUE){
  Transparent = TRUE;
  Assign(NULL);
}

CMetafile::~CMetafile(){
	if(Image != NULL)
		Image->Release();
}

void CMetafile::NewImage(){
	if(Image != NULL)
		Image->Release();
	Image = new CMetafileImage();
	Image->Reference();
}

void CMetafile::UniqueImage(){
	if(Image == NULL)
		NewImage();
	else{
		if(Image->GetRefCount() > 1){
			CMetafileImage* NewImage = new CMetafileImage();
			if(Image->Handle != 0)
				NewImage->Handle = CopyEnhMetaFile(Image->Handle, NULL);
			NewImage->Height = Image->Height;
			NewImage->Width  = Image->Width;
			NewImage->Inch = Image->Inch;
			NewImage->TempWidth = Image->TempWidth;
			NewImage->TempHeight = Image->TempHeight;
			Image->Release();
			Image = NewImage;
			Image->Reference();
		}
	}
}

BOOL CMetafile::GetEmpty(){
	return Image == NULL;
}

INT CMetafile::GetHeight(){
	INT Result = 0;
	if(Image == NULL)
		NewImage();
	if(Image->Inch == 0){
		if(Image->Handle == 0)
			Result = Image->TempHeight;
		else{//convert 0.01mm units to referenceDC device pixels 
			ENHMETAHEADER EMFHeader;
			GetEnhMetaFileHeader(Image->Handle, sizeof(EMFHeader), &EMFHeader);
			Result = MulDiv(Image->Height, //metafile height in 0.01mm
				EMFHeader.szlDevice.cy, //device height in pixels
				EMFHeader.szlMillimeters.cy*100);//device height in mm

		}
	}
	else //for WMF files, convert to font dpi based device pixels
		Result = MulDiv(Image->Height, GGlobal.ScreenLogPixels, HundredthMMPerInch);
	return Result;
}

HPALETTE CMetafile::GetPalette(){
	if(Image == NULL || Image->Handle == 0)
		return 0;
	if(Image->Palette == 0){
		INT Count = GetEnhMetaFilePaletteEntries(Image->Handle, 0, NULL);
		if(Count == 0)
			return 0;
		else if(Count > 256)
			Count &= 0xFF;
		InternalDeletePalette(Image->Palette);
		TMaxLogPalette LogPal;
		LogPal.palVersion = 0x300;
		LogPal.palNumEntries = Count;
		GetEnhMetaFilePaletteEntries(Image->Handle, Count, LogPal.palPalEntry);
		Image->Palette = CreatePalette((PLOGPALETTE)&LogPal);
	}
	return Image->Palette;
}

INT CMetafile::GetWidth(){
	INT Result = 0;
	ENHMETAHEADER EMFHeader;
	if(Image == NULL)
		NewImage();
	if(Image->Inch == 0){
		if(Image->Handle == 0)
			Result = Image->TempWidth;
		else{ //convert 0.01mm units to referenceDC device pixels }
			GetEnhMetaFileHeader(Image->Handle, sizeof(EMFHeader), &EMFHeader);
			Result = MulDiv(Image->Width, //metafile width in 0.01mm
				EMFHeader.szlDevice.cx,  //device width in pixels
				EMFHeader.szlMillimeters.cx*100); //device width in 0.01mm
		}
	}
	else //for WMF files, convert to font dpi based device pixels
		Result = MulDiv(Image->Width, GGlobal.ScreenLogPixels, HundredthMMPerInch);
	return Result;
}

void CMetafile::Draw(CCanvas* ACanvas, const TRect& Rect){
	if(Image == NULL)
		return;
	HPALETTE MetaPal = GetPalette();
	HPALETTE OldPal = 0;
	if(MetaPal != 0){
		OldPal = SelectPalette(ACanvas->GetHandle(), MetaPal, TRUE);
		RealizePalette(ACanvas->GetHandle());
	}
	TRect R = Rect;
	R.right--;  // Metafile rect includes right and bottom coords
	R.bottom--;
	PlayEnhMetaFile(ACanvas->GetHandle(), Image->Handle, (RECT *)&R);
	if(MetaPal != 0)
		SelectPalette(ACanvas->GetHandle(), OldPal, TRUE);
}

void CMetafile::ReadData(CStream* Stream){
	DWORD Length = 0;
	Stream->Read(&Length, sizeof(Length));
	if(Length <= 4)
		Assign(NULL);
	else
		if(TestEMF(Stream))
			ReadEMFStream(Stream);
		else
			ReadWMFStream(Stream, Length - sizeof(Length));
	SetPaletteModified(GetPalette() != 0);
	Changed(this);
}

void CMetafile::ReadEMFStream(CStream* Stream){
	NewImage();
	ENHMETAHEADER EnhHeader;
	Stream->ReadBuffer(&EnhHeader, sizeof(EnhHeader));
	if(EnhHeader.dSignature != ENHMETA_SIGNATURE)
		throw "Invalid Metafile"; //InvalidMetafile;
	LPVOID Buf = malloc(EnhHeader.nBytes);
	BufferHolder bufHolder(Buf);
	CopyMemory(Buf, &EnhHeader, sizeof(EnhHeader));
	Stream->ReadBuffer((LPVOID)((LPBYTE)Buf + sizeof(EnhHeader)),
		EnhHeader.nBytes - sizeof(EnhHeader));
	Image->Handle = SetEnhMetaFileBits(EnhHeader.nBytes, (const BYTE *)Buf);
	if(Image->Handle == 0)
		throw "Invalid Metafile"; //InvalidMetafile;
	Image->Inch = 0;
	Image->Width = EnhHeader.rclFrame.right - EnhHeader.rclFrame.left;//in 0.01 mm units
	Image->Height = EnhHeader.rclFrame.bottom - EnhHeader.rclFrame.top;
	SetEnhanced(TRUE);
}

#define WMFKEY		0x9AC6CDD7
#define WMFWORD		0xCDD7
#pragma pack (1)
typedef struct{
	DWORD Key;
	SHORT Handle;
	SMALL_RECT Box;
	WORD Inch;
	DWORD Reserved;
	WORD CheckSum;
} TMetafileHeader, *PMetafileHeader;
#pragma pack ()

WORD ComputeAldusChecksum(TMetafileHeader& WMF){
	WORD Result = 0;
	LPWORD pW = (LPWORD)&WMF;
	LPWORD pEnd = &(WMF.CheckSum);
	while(pW < pEnd){
		Result ^= *pW;
		pW++;
	}
	return Result;
}

void CMetafile::ReadWMFStream(CStream* Stream, INT Length){
	NewImage();
	TMetafileHeader WMF;
	Stream->Read(&WMF, sizeof(WMF));
	if(WMF.Key != WMFKEY || ComputeAldusChecksum(WMF) != WMF.CheckSum)
		throw "Invalid Metafile.";//InvalidMetafile;
	Length -= sizeof(WMF);
	LPVOID Bitmem = malloc(Length);
	BufferHolder bmHolder(Bitmem);
	Stream->Read(Bitmem, Length);
	Image->Inch = WMF.Inch;
	if(WMF.Inch == 0)
		WMF.Inch = 96;
	Image->Width = MulDiv(WMF.Box.Right - WMF.Box.Left,HundredthMMPerInch,WMF.Inch);
	Image->Height = MulDiv(WMF.Box.Bottom - WMF.Box.Top,HundredthMMPerInch,WMF.Inch);
	METAFILEPICT MFP;
	MFP.mm = MM_ANISOTROPIC;
	MFP.xExt = 0;
	MFP.yExt = 0;
	MFP.hMF = 0;
    Image->Handle = SetWinMetaFileBits(Length, (const BYTE *)Bitmem, 0, &MFP);
    if(Image->Handle == 0){
		RaiseLastOSError();
		throw "invalid Metafile";//InvalidMetafile;
	}
    // Get the maximum extent actually used by the metafile output
    // and re-convert the wmf data using the new extents.
    // This helps preserve whitespace margins in WMFs
	ENHMETAHEADER EMFHeader;
    GetEnhMetaFileHeader(Image->Handle, sizeof(EMFHeader), &EMFHeader);
	MFP.mm = MM_ANISOTROPIC;
	MFP.xExt = EMFHeader.rclFrame.right;
	MFP.yExt = EMFHeader.rclFrame.bottom;
	MFP.hMF = 0;
    DeleteEnhMetaFile(Image->Handle);
    Image->Handle = SetWinMetaFileBits(Length, (const BYTE *)Bitmem, 0, &MFP);
    if(Image->Handle == 0)
		throw "invalid metafile";//InvalidMetafile;
	SetEnhanced(FALSE);
}

void CMetafile::SetHeight(INT Value){
	if(Image == NULL)
		NewImage();
	if(Image->Inch == 0){
		if(Image->Handle == 0)
			Image->TempHeight = Value;
		else{//convert device pixels to 0.01mm units
			ENHMETAHEADER EMFHeader;
			GetEnhMetaFileHeader(Image->Handle, sizeof(EMFHeader), &EMFHeader);
			SetMMHeight(MulDiv(Value, //metafile height in pixels
				EMFHeader.szlMillimeters.cy*100, //device height in 0.01mm 
				EMFHeader.szlDevice.cy)); //device height in pixels 
		}
	}
	else
		SetMMHeight(MulDiv(Value, HundredthMMPerInch, GGlobal.ScreenLogPixels));

}

void CMetafile::SetTransparent(BOOL Value){
	// Ignore assignments to this property.
	// Metafiles must always be considered transparent.
}

void CMetafile::SetWidth(INT Value){
	if(Image == NULL)
		NewImage();
	if(Image->Inch == 0){
		if(Image->Handle == 0)
			Image->TempWidth = Value;
		else{ //convert device pixels to 0.01mm units
			ENHMETAHEADER EMFHeader;
			GetEnhMetaFileHeader(Image->Handle, sizeof(EMFHeader), &EMFHeader);
			SetMMWidth(MulDiv(Value, //metafile width in pixels
				EMFHeader.szlMillimeters.cx*100, //device width in mm
				EMFHeader.szlDevice.cx)); //device width in pixels
		}
	}
	else
		SetMMWidth(MulDiv(Value, HundredthMMPerInch, GGlobal.ScreenLogPixels));

}

BOOL CMetafile::TestEMF(CStream* Stream){
	LONG Size = (LONG)(Stream->GetSize() - Stream->GetPosition());
	ENHMETAHEADER Header;
	if(Size > sizeof(Header)){
		Stream->Read(&Header, sizeof(Header));
		Stream->Seek(0-sizeof(Header), soFromCurrent);
	}
	return Size > sizeof(Header) &&
		Header.iType == EMR_HEADER && Header.dSignature == ENHMETA_SIGNATURE;

}

void CMetafile::WriteData(CStream* Stream){
	if(Image != NULL){
		INT SavePos = 0;
		Stream->Write(&SavePos, sizeof(SavePos));
		SavePos = (INT)(Stream->GetPosition() - sizeof(SavePos));
		if(GetEnhanced())
			WriteEMFStream(Stream);
		else
			WriteWMFStream(Stream);
		Stream->Seek(SavePos, soFromBeginning);
		SavePos = (INT)(Stream->GetSize() - SavePos);
		Stream->Write(&SavePos, sizeof(SavePos));
		Stream->Seek(0, soFromEnd);
	}
}

void CMetafile::WriteEMFStream(CStream* Stream){
	if(Image == NULL)
		return;
	LONG Length = GetEnhMetaFileBits(Image->Handle, 0, NULL);
	if(Length == 0)
		return ;
	LPVOID Buf = malloc(Length);
	BufferHolder bufHolder(Buf);
	GetEnhMetaFileBits(Image->Handle, Length, (LPBYTE)Buf);
	Stream->WriteBuffer(Buf, Length);
}

void CMetafile::WriteWMFStream(CStream* Stream){
	if(Image == NULL)
		return ;
	TMetafileHeader WMF;
	ZeroMemory(&WMF, sizeof(WMF));
	WMF.Key = WMFKEY;
	if(Image->Inch == 0)
        WMF.Inch = 96; //WMF defaults to 96 units per inch
	else
		WMF.Inch = Image->Inch;
	WMF.Box.Right = MulDiv(Image->Width, WMF.Inch, HundredthMMPerInch);
	WMF.Box.Bottom = MulDiv(Image->Height, WMF.Inch, HundredthMMPerInch);
	WMF.CheckSum = ComputeAldusChecksum(WMF);
	HDC RefDC = GetDC(0);
	DCHolder dcHolder(RefDC, 0, 0);
	UINT Length = GetWinMetaFileBits(Image->Handle, 0, NULL, MM_ANISOTROPIC, RefDC);
	LPVOID Bits = malloc(Length);
	BufferHolder bitHolder(Bits);
	if(GetWinMetaFileBits(Image->Handle, Length, (LPBYTE)Bits, MM_ANISOTROPIC, RefDC) < Length)
		GDIError();
	Stream->WriteBuffer(&WMF, sizeof(WMF));
	Stream->WriteBuffer(Bits, Length);
}

void CMetafile::Clear(){
	NewImage();
}

BOOL CMetafile::HandleAllocated(){
	return Image != NULL && Image->Handle != 0;
}

void CMetafile::LoadFromStream(CStream* Stream){
	if(TestEMF(Stream))
		ReadEMFStream(Stream);
	else
		ReadWMFStream(Stream, (INT)(Stream->GetSize() - Stream->GetPosition()));
	SetPaletteModified(GetPalette() != 0);
	Changed(this);
}

void CMetafile::SaveToFile(LPTSTR Filename){
	BOOL SaveEnh = GetEnhanced();
	__try{
		LPTSTR P = _tcsrchr(Filename, TCHAR('.'));
		if(lstrcmpi(P, TEXT(".wmf")) == 0)
			SetEnhanced(FALSE); //For 16 bit compatibility
		__super::SaveToFile(Filename);
	}
	__finally{
		SetEnhanced(SaveEnh);
	}
}

void CMetafile::SaveToStream(CStream* Stream){
	if(Image != NULL)
		if(GetEnhanced())
			WriteEMFStream(Stream);
		else
			WriteWMFStream(Stream);
}

void CMetafile::LoadFromClipboardFormat(WORD AFormat, HANDLE AData, HPALETTE APalette){
	ENHMETAHEADER EnhHeader;
	AData = GetClipboardData(CF_ENHMETAFILE); // OS will convert WMF to EMF
	if(AData == 0)
		throw "unknown clipboard format";//InvalidGraphic(@SUnknownClipboardFormat);
	NewImage();
	Image->Handle = CopyEnhMetaFile((HENHMETAFILE)AData, NULL);
	GetEnhMetaFileHeader(Image->Handle, sizeof(EnhHeader), &EnhHeader);
	Image->Width = EnhHeader.rclFrame.right - EnhHeader.rclFrame.left;
	Image->Height = EnhHeader.rclFrame.bottom - EnhHeader.rclFrame.top;
	Image->Inch = 0;
	SetEnhanced(TRUE);
	SetPaletteModified(GetPalette() != 0);
	Changed(this);
}

void CMetafile::SaveToClipboardFormat(WORD& AFormat, HANDLE& AData, HPALETTE& APalette){
	if(Image == NULL)
		return ;
	AFormat = CF_ENHMETAFILE;
	APalette = 0;
	AData = CopyEnhMetaFile(Image->Handle, NULL);
}

void CMetafile::Assign(CObject* Source){
	if(Source == NULL || Source->InstanceOf(CMetafile::_Class)){
		HPALETTE Pal = 0;
		if(Image != NULL){
			Pal = Image->Palette;
			Image->Release();
		}
		if(Source != NULL){
			Image = ((CMetafile *)Source)->Image;
			Enhanced = ((CMetafile *)Source)->GetEnhanced();
		}
		else{
			Image = new CMetafileImage();
			Enhanced = TRUE;
			
		}
		Image->Reference();
		SetPaletteModified(Pal != GetPalette() && GetPalette() != 0);
		Changed(this);
	}
	else
		__super::Assign(Source);

}

HENHMETAFILE CMetafile::ReleaseHandle(){
	UniqueImage();
	HENHMETAFILE Result = Image->Handle;
	Image->Handle = 0;
	return Result;
}

String CMetafile::GetAuthor(){
	String Result;
	if(Image == NULL || Image->Handle == 0)
		return Result;
	INT Temp = GetEnhMetaFileDescription(Image->Handle, 0, NULL);
	if(Temp <= 0)
		return Result;
	LPTSTR Buf = (LPTSTR)malloc(Temp);
	GetEnhMetaFileDescription(Image->Handle, Temp, Buf);
	return String::CreateFor(Buf);
}

String CMetafile::GetDescription(){
	String Result;
	if(Image == NULL || Image->Handle == 0)
		return Result;
	INT Temp = GetEnhMetaFileDescription(Image->Handle, 0, NULL);
	if(Temp <= 0)
		return Result;
	LPTSTR Buf = (LPTSTR)malloc(Temp);
	BufferHolder bufHolder(Buf);
	GetEnhMetaFileDescription(Image->Handle, Temp, Buf);
	Buf += lstrlen(Buf) + 1;
	return String(Buf);
}

HENHMETAFILE CMetafile::GetHandle(){
	if(Image != NULL)
		return Image->Handle;
	else 
		return 0;
}

void CMetafile::SetHandle(HENHMETAFILE Value){
	ENHMETAHEADER EnhHeader;
	if(Value != 0 && GetEnhMetaFileHeader(Value, sizeof(EnhHeader), &EnhHeader) == 0)
		throw "invalid Metafile"; //InvalidMetafile;
	UniqueImage();
	if(Image->Handle != 0)
		DeleteEnhMetaFile(Image->Handle);
	InternalDeletePalette(Image->Palette);
	Image->Palette = 0;
	Image->Handle = Value;
	Image->TempWidth = 0;
	Image->TempHeight = 0;
	if(Value != 0){
		Image->Width = EnhHeader.rclFrame.right - EnhHeader.rclFrame.left;
		Image->Height = EnhHeader.rclFrame.bottom - EnhHeader.rclFrame.top;
	}
	SetPaletteModified(GetPalette() != 0);
	Changed(this);
}

INT CMetafile::GetMMWidth(){
	if(Image == NULL)
		NewImage();
	return Image->Width;
}

void CMetafile::SetMMWidth(INT Value){
	if(Image == NULL)
		NewImage();
	Image->TempWidth = 0;
	if(Image->Width != Value){
		UniqueImage();
		Image->Width = Value;
		Changed(this);
	}
}

INT CMetafile::GetMMHeight(){
	if(Image == NULL)
		NewImage();
	return Image->Height;
}

void CMetafile::SetMMHeight(INT Value){
	if(Image == NULL)
		NewImage();
	Image->TempHeight = 0;
	if(Image->Height != Value){
		UniqueImage();
		Image->Height = Value;
		Changed(this);
	}
}

WORD CMetafile::GetInch(){
	WORD Result = 0;
	if(Image != NULL)
		Result = Image->Inch;
	return Result;
}

void CMetafile::SetInch(WORD Value){
	if(Image == NULL)
		NewImage();
	if(Image->Inch != Value){
		UniqueImage();
		Image->Inch = Value;
		Changed(this);
	}
}
#pragma pack (1)
typedef struct _Pattern *PPattern;
typedef struct _Pattern{
	PPattern Next;
    CBitmap* Bitmap;
	COLORREF BkColorRef;
	COLORREF FgColorRef;
} TPattern;
#pragma pack ()

class CPatternManager : public CObject{
private:
	PPattern List;
	RTL_CRITICAL_SECTION mLock;
    CBitmap* CreateBitmap(TColor BkColor, TColor FgColor);
public:
    CPatternManager();
    virtual ~CPatternManager();
    PPattern AllocPattern(COLORREF BkColor, COLORREF FgColor);
	void FreePatterns();
	void Lock();
	void Unlock();

	REF_DYN_CLASS(CPatternManager)
};
DECLARE_DYN_CLASS(CPatternManager, CObject)

IMPL_DYN_CLASS(CPatternManager)
CPatternManager::CPatternManager(){
	InitializeCriticalSection(&mLock);
}
CPatternManager::~CPatternManager(){
	FreePatterns();
	DeleteCriticalSection(&mLock);
}
CBitmap* CPatternManager::CreateBitmap(TColor BkColor, TColor FgColor){
	CBitmap* Result = new CBitmap();
	CObjectHolder retHolder(Result);
	Result->SetWidth(8);
	Result->SetHeight(8);
	Result->GetCanvas()->GetBrush()->SetStyle(bsSolid);
	Result->GetCanvas()->GetBrush()->SetColor(BkColor);
	Result->GetCanvas()->FillRect(Rect(0, 0, Result->GetWidth(), Result->GetHeight()));
	for(INT Y = 0; Y <= 8; Y++)
		for(INT X = 0; X <= 8; X++)
			if((Y % 2) == (X % 2)) //toggles between even/odd pixles
				Result->GetCanvas()->SetPixel(X, Y, FgColor); //on even/odd rows
	Result->Dormant();
	retHolder.SwapObject(NULL);
	return Result;
}

PPattern CPatternManager::AllocPattern(COLORREF BkColor, COLORREF FgColor){
	CMethodLock L(this, (TLockMethod)&CPatternManager::Lock, (TLockMethod)&CPatternManager::Unlock);
	PPattern Result = List;
	while(Result != NULL && (Result->BkColorRef != BkColor ||
		Result->FgColorRef != FgColor))
		Result = Result->Next;
	if(Result == NULL){
		Result = (PPattern)malloc(sizeof(TPattern));
		Result->Next = List;
		Result->Bitmap = CreateBitmap(BkColor, FgColor);
		Result->BkColorRef = BkColor;
		Result->FgColorRef = FgColor;
		List = Result;
	}
	return Result;
}

void CPatternManager::FreePatterns(){
	PPattern P = NULL;
	while (List != NULL){
		P = List;
		{
			CMethodLock L(this, (TLockMethod)&CPatternManager::Lock, (TLockMethod)&CPatternManager::Unlock);
			List = P->Next;
		}
		if(P->Bitmap != NULL)
			delete P->Bitmap;
		free(P);
	}

}

void CPatternManager::Lock(){
	EnterCriticalSection(&mLock);
}

void CPatternManager::Unlock(){
	LeaveCriticalSection(&mLock);
}

CPatternManager PatternManager;

CBitmap* AllocPatternBitmap(TColor BkColor, TColor FgColor){
	//if(PatternManager != NULL)
		return PatternManager.AllocPattern(ColorToRGB(BkColor),
			ColorToRGB(FgColor))->Bitmap;
	//return NULL;
}
