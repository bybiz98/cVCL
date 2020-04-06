#pragma once
#include "stdinc.h"
#include "Object.hpp"
#include "Types.hpp"
#include "Stream.hpp"
#include "Strings.hpp"

typedef DWORD TColor, *PColor;
#define clSystemColor				0xFF000000
#define clScrollBar					clSystemColor | COLOR_SCROLLBAR
#define clBackground				clSystemColor | COLOR_BACKGROUND
#define clActiveCaption				clSystemColor | COLOR_ACTIVECAPTION
#define clInactiveCaption			clSystemColor | COLOR_INACTIVECAPTION
#define clMenu						clSystemColor | COLOR_MENU
#define clWindow					clSystemColor | COLOR_WINDOW
#define clWindowFrame				clSystemColor | COLOR_WINDOWFRAME
#define clMenuText					clSystemColor | COLOR_MENUTEXT
#define clWindowText				clSystemColor | COLOR_WINDOWTEXT
#define clCaptionText				clSystemColor | COLOR_CAPTIONTEXT
#define clActiveBorder				clSystemColor | COLOR_ACTIVEBORDER
#define clInactiveBorder			clSystemColor | COLOR_INACTIVEBORDER
#define clAppWorkSpace				clSystemColor | COLOR_APPWORKSPACE
#define clHighlight					clSystemColor | COLOR_HIGHLIGHT
#define clHighlightText				clSystemColor | COLOR_HIGHLIGHTTEXT
#define clBtnFace					clSystemColor | COLOR_BTNFACE
#define clBtnShadow					clSystemColor | COLOR_BTNSHADOW
#define clGrayText					clSystemColor | COLOR_GRAYTEXT
#define clBtnText					clSystemColor | COLOR_BTNTEXT
#define clInactiveCaptionText		clSystemColor | COLOR_INACTIVECAPTIONTEXT
#define clBtnHighlight				clSystemColor | COLOR_BTNHIGHLIGHT
#define cl3DDkShadow				clSystemColor | COLOR_3DDKSHADOW
#define cl3DLight					clSystemColor | COLOR_3DLIGHT
#define clInfoText					clSystemColor | COLOR_INFOTEXT
#define clInfoBk					clSystemColor | COLOR_INFOBK
#define clHotLight					clSystemColor | COLOR_HOTLIGHT
#define clGradientActiveCaption		clSystemColor | COLOR_GRADIENTACTIVECAPTION
#define clGradientInactiveCaption	clSystemColor | COLOR_GRADIENTINACTIVECAPTION
#define clMenuHighlight				clSystemColor | COLOR_MENUHILIGHT
#define clMenuBar					clSystemColor | COLOR_MENUBAR

#define clBlack						0x000000
#define clMaroon					0x000080
#define clGreen						0x008000
#define clOlive						0x008080
#define clNavy						0x800000
#define clPurple					0x800080
#define clTeal						0x808000
#define clGray						0x808080
#define clSilver					0xC0C0C0
#define clRed						0x0000FF
#define clLime						0x00FF00
#define clYellow					0x00FFFF
#define clBlue						0xFF0000
#define clFuchsia					0xFF00FF
#define clAqua						0xFFFF00
#define clLtGray					0xC0C0C0
#define clDkGray					0x808080
#define clWhite						0xFFFFFF

#define StandardColorsCount			16

#define clMoneyGreen				0xC0DCC0
#define clSkyBlue					0xF0CAA6
#define clCream						0xF0FBFF
#define clMedGray					0xA4A0A0

#define ExtendedColorsCount			4

#define clNone						0x1FFFFFFF
#define clDefault					0x20000000

//copy mode
#define cmBlackness					BLACKNESS
#define cmDstInvert					DSTINVERT
#define cmMergeCopy					MERGECOPY
#define cmMergePaint				MERGEPAINT
#define cmNotSrcCopy				NOTSRCCOPY
#define cmNotSrcErase				NOTSRCERASE
#define cmPatCopy					PATCOPY
#define cmPatInvert					PATINVERT
#define cmPatPaint					PATPAINT
#define cmSrcAnd					SRCAND
#define cmSrcCopy					SRCCOPY
#define cmSrcErase					SRCERASE
#define cmSrcInvert					SRCINVERT
#define cmSrcPaint					SRCPAINT
#define cmWhiteness					WHITENESS

//Icon and cursor types
#define RC3_STOCKICON				0
#define RC3_ICON					1
#define RC3_CURSOR					2

class CBitmap;

COLORREF ColorToRGB(TColor Color);

HPALETTE CopyPalette(HPALETTE Palette);

CBitmap* AllocPatternBitmap(TColor BkColor, TColor FgColor);

#pragma pack (1)
typedef struct{
	HANDLE Handle;
}TResData, *PResData;
#pragma pack ()
//TFontPitch
#define fpDefault		0x1
#define fpVariable		0x2
#define fpFixed			0x4

//TFontStyle
#define fsBold			0x1
#define fsItalic		0x2
#define fsUnderline		0x4
#define fsStrikeOut		0x8

#define LF_FACESIZE		32

#pragma pack (1)
typedef struct{
	HFONT Handle;
	INT Height;
	BYTE Pitch;
	BYTE Style;
	BYTE Charset;
	TCHAR Name[LF_FACESIZE - 1];
}TFontData;
#pragma pack ()

//TPenStyle 
#define psSolid			0x00
#define psDash			0x01
#define psDot			0x02
#define psDashDot		0x03
#define	psDashDotDot	0x04
#define psClear			0x05
#define	psInsideFrame	0x06

//TPenMode
#define pmBlack			0x0000
#define	pmWhite			0x0001
#define pmNop			0x0002
#define pmNot			0x0003
#define pmCopy			0x0004
#define pmNotCopy		0x0005
#define pmMergePenNot	0x0006
#define pmMaskPenNot	0x0007
#define pmMergeNotPen	0x0008
#define pmMaskNotPen	0x0009
#define pmMerge			0x000A
#define pmNotMerge		0x000B
#define pmMask			0x000C
#define pmNotMask		0x000D
#define pmXor			0x000E
#define pmNotXor		0x000F

#pragma pack (1)
typedef struct{
	HPEN Handle;
	TColor Color;
	INT Width;
	BYTE Style;
}TPenData;
#pragma pack ()

//TBrushStyle 
#define bsSolid			0x00
#define bsClear			0x01
#define bsHorizontal	0x02
#define bsVertical		0x03
#define bsFDiagonal		0x04
#define bsBDiagonal		0x05
#define bsCross			0x06
#define bsDiagCross		0x07

#pragma pack (1)
typedef struct{
	HBRUSH Handle;
	TColor Color;
	CBitmap* Bitmap;
    BYTE Style;
}TBrushData, *PBrushData;

typedef struct _Resource *PResource;
typedef struct _Resource{
	PResource Next;
    UINT RefCount;
	HANDLE Handle;
	WORD HashCode;
	union{
		TResData Data;
		TFontData Font;
		TPenData Pen;
		TBrushData Brush;
	};
}TResource;
#pragma pack ()

class CResouceManager;
class cVCL_API CGraphicsObject : public CObject{
private:
	PRTL_CRITICAL_SECTION OwnerLock;
protected:
	friend class CResourceManager;
	PResource Resource;
	virtual void Changed();
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Change)
public:
	CGraphicsObject();
	virtual ~CGraphicsObject();
	BOOL HandleAllocated();
	PRTL_CRITICAL_SECTION GetOwnerCriticalSection();
	void SetOwnerCriticalSection(PRTL_CRITICAL_SECTION Value);
	void Lock();
	void Unlock();

	REF_DYN_CLASS(CGraphicsObject)
};
DECLARE_DYN_CLASS(CGraphicsObject, CObject)


class cVCL_API CFont : public CGraphicsObject{
private:
	TColor Color;
	INT PixelsPerInch;
	//IChangeNotifier FNotify;
	void GetData(TFontData& FontData);
	void SetData(const TFontData& FontData);
protected:
	void Changed() override;
public:
	CFont();
	virtual ~CFont();
	//property FontAdapter: IChangeNotifier read FNotify write FNotify;
	void Assign(CFont* Source);
    HFONT GetHandle();
	void SetHandle(HFONT Value);

	DEFINE_GETTER(TColor, Color);
	DEFINE_ACCESSOR(INT, PixelsPerInch);

	BYTE GetCharset();
	void SetCharset(BYTE Value);
	void SetColor(TColor Value);
	INT GetHeight();
	void SetHeight(INT Value);
	LPTSTR GetName();
	void SetName(LPTSTR Value);
	BYTE GetPitch();
	void SetPitch(BYTE Value);
	INT GetSize();
	void SetSize(BYTE Value);
	BYTE GetStyle();
	void SetStyle(BYTE Value);

	REF_DYN_CLASS(CFont)
};
DECLARE_DYN_CLASS(CFont, CGraphicsObject)

class cVCL_API CPen : public CGraphicsObject{
private:
	SHORT Mode;
	void GetData(TPenData& PenData);
	void SetData(const TPenData& PenData);
public:
    CPen();
    virtual ~CPen();
	void Assign(CPen* Source);
	TColor GetColor();
	void SetColor(TColor Value);
	HPEN GetHandle();
	void SetHandle(HPEN Value);
	SHORT GetMode();
	void SetMode(SHORT Value);
	BYTE GetStyle();
	void SetStyle(BYTE Value);
	INT GetWidth();
	void SetWidth(INT Value);

	REF_DYN_CLASS(CPen)
};
DECLARE_DYN_CLASS(CPen, CGraphicsObject)

class cVCL_API CBrush : public CGraphicsObject{
private:
    void GetData(TBrushData& BrushData);
	void SetData(const TBrushData& BrushData);
public:
    CBrush();
	virtual ~CBrush();
    void Assign(CBrush* Source);
	CBitmap* GetBitmap();
	void SetBitmap(CBitmap* Value);
	TColor GetColor();
	void SetColor(TColor Value);
	HBRUSH GetHandle();
	void SetHandle(HBRUSH Value);
	UINT GetStyle();
	void SetStyle(UINT Value);

	REF_DYN_CLASS(CBrush);
};
DECLARE_DYN_CLASS(CBrush, CGraphicsObject)

class cVCL_API CResourceManager : public CObject{
private:
    PResource ResList;
	RTL_CRITICAL_SECTION mLock;
	WORD ResDataSize;
public:
	CResourceManager(WORD AResDataSize = 0);
	virtual ~CResourceManager();
	PResource AllocResource(const LPVOID ResData);
	void FreeResource(PResource Resource);
	void ChangeResource(CGraphicsObject* GraphicsObject, const LPVOID ResData);
	void AssignResource(CGraphicsObject* GraphicsObject, PResource AResource);
	void Lock();
	void Unlock();

	REF_DYN_CLASS(CResourceManager)
};
DECLARE_DYN_CLASS(CResourceManager, CObject)

//TCanvasStates
typedef UINT TCanvasStates;
#define csHandleValid			0x01
#define csFontValid				0x02
#define csPenValid				0x04
#define csBrushValid			0x08
#define csAllValid				0x0F

//TCanvasOrientation
#define coLeftToRight			0x00
#define coRightToLeft			0x01

//TFillStyle
#define fsSurface				0x00
#define fsBorder				0x01

//TFillMode
#define fmAlternate				0x00
#define fmWinding				0x01

class CGraphic;

class cVCL_API CCanvas : public CObject{
private:
	TCanvasStates State;
	CFont* Font;
	CPen* Pen;
	CBrush* Brush;
	POINT PenPos;
	LONG CopyMode;
	RTL_CRITICAL_SECTION LockSection;
	INT LockCount;
	LONG TextFlags;
	void CreateBrush();
	void CreateFont();
	void CreatePen();
	void BrushChanged(CObject* ABrush);
	void DeselectHandles();
	void FontChanged(CObject* AFont);
	void PenChanged(CObject* APen);
protected:
	HDC DC;

    virtual void Changed();
	virtual void Changing();
	virtual void CreateHandle();
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Change)
	DECLARE_TYPE_EVENT(TNotifyEvent, Changing)
public:
	CCanvas();
	virtual ~CCanvas();

	DEFINE_ACCESSOR(HDC, DC);

	void RequiredState(TCanvasStates ReqState);

	void Arc(INT X1, INT Y1, INT X2, INT Y2, INT X3, INT Y3, INT X4, INT Y4);
	void BrushCopy(const TRect& Dest, CBitmap* Bitmap, const TRect& Source, TColor Color);
	void Chord(INT X1, INT Y1, INT X2, INT Y2, INT X3, INT Y3, INT X4, INT Y4);
	void CopyRect(const TRect& Dest, CCanvas* Canvas, const TRect& Source);
	void Draw(INT X, INT Y, CGraphic* Graphic);
	void DrawFocusRect(const TRect& Rect);
	void Ellipse(INT X1, INT Y1, INT X2, INT Y2);
	void Ellipse(const TRect& Rect);
	void FillRect(const TRect& Rect);
	void FloodFill(INT X, INT Y, TColor Color, BYTE FillStyle);
	void FrameRect(const TRect& Rect);
	BOOL HandleAllocated();
	void LineTo(INT X, INT Y);
	void Lock();
	void MoveTo(INT X, INT Y);
	void Pie(INT X1, INT Y1, INT X2, INT Y2, INT X3, INT Y3, INT X4, INT Y4);
	void Polygon(const POINT* Points, INT cpt);
	void Polyline(const POINT* Points, INT cpt);
	void PolyBezier(const POINT* Points, INT cpt);
	void PolyBezierTo(const POINT* Points, INT cpt);
	void Rectangle(INT X1, INT Y1, INT X2, INT Y2);
	void Rectangle(const TRect& Rect);
	void Refresh();
	void RoundRect(INT X1, INT Y1, INT X2, INT Y2, INT X3, INT Y3);
	void StretchDraw(const TRect& Rect, CGraphic* Graphic);
	SIZE TextExtent(const LPTSTR Text);
	INT TextHeight(const LPTSTR Text);
	void TextOut(INT X, INT Y, const LPTSTR Text);
	void TextRect(TRect& Rect, INT X, INT Y, const LPTSTR Text);
	INT TextWidth(const LPTSTR Text);
	BOOL TryLock();
	void UnLock();
    TRect GetClipRect();
    HDC GetHandle();
	void SetHandle(HDC Value);
	INT GetLockCount();
	BYTE GetCanvasOrientation();
	POINT GetPenPos();
	void SetPenPos(POINT& Value);
	INT GetPixel(INT X, INT Y);
	void SetPixel(INT X, INT Y, INT Value);
	DEFINE_ACCESSOR(LONG, TextFlags)
    CBrush* GetBrush();
	void SetBrush(CBrush* Value);
	LONG GetCopyMode();
	void SetCopyMode(LONG& Value);
	CFont* GetFont();
	void SetFont(CFont* Value);
	void SetPen(CPen* Value);
	CPen* GetPen();

	REF_DYN_CLASS(CCanvas)
};
DECLARE_DYN_CLASS(CCanvas, CObject)

typedef BYTE TProgressStage;
#define psStarting	0x1
#define psRunning	0x2
#define	psEnding	0x3

typedef void (CObject::*TProgressEvent)(CObject* Sender, TProgressStage Stage, BYTE PercentDone,
	BOOL RedrawNow, const TRect& R, LPTSTR Msg);

class cVCL_API CGraphic : public CObject{
private:
    BOOL Modified;
	BOOL PaletteModified;
protected:
	BOOL Transparent;
	virtual void Changed(CObject* Sender);
	virtual BOOL Equals(CGraphic* Graphic);
	virtual void Progress(CObject* Sender, TProgressStage Stage, BYTE PercentDone, BOOL RedrawNow, const TRect& R, LPTSTR Msg);
	virtual void ReadData(CStream* Stream);
	virtual void WriteData(CStream* Stream);
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Change)
	DECLARE_TYPE_EVENT(TProgressEvent, Progress)
public:
	CGraphic();
	virtual ~CGraphic();
	
	DEFINE_GETTER(BOOL, Modified)
	DEFINE_ACCESSOR(BOOL,PaletteModified)

	virtual void Draw(CCanvas* ACanvas, const TRect& Rect) = 0;
	virtual void LoadFromFile(const LPTSTR Filename);
	virtual void SaveToFile(const LPTSTR Filename);
	virtual void LoadFromStream(CStream* Stream) = 0;
	virtual void SaveToStream(CStream* Stream) = 0;
	virtual void LoadFromClipboardFormat(WORD AFormat, HANDLE AData, HPALETTE APalette) = 0;
	virtual void SaveToClipboardFormat(WORD& AFormat, HANDLE& AData, HPALETTE& APalette) = 0;
	virtual BOOL GetEmpty() = 0;
	virtual INT GetHeight() = 0;
	virtual void SetHeight(INT Value) = 0;
	void SetModified(BOOL Value);
	virtual HPALETTE GetPalette();
	virtual void SetPalette(HPALETTE Value);
	virtual BOOL GetTransparent();
	virtual void SetTransparent(BOOL Value);
	virtual INT GetWidth() = 0;
	virtual void SetWidth(INT Value) = 0;

	REF_DYN_CLASS(CGraphic)
};
DECLARE_DYN_CLASS_ABSTRACT(CGraphic, CObject)

class cVCL_API CSharedImage : public CObject{
private:
    INT RefCount;
protected:
	virtual void FreeHandle() = 0;
public:
	CSharedImage();
	virtual ~CSharedImage();
	void Reference();
	void Release();
	DEFINE_GETTER(INT, RefCount)

	REF_DYN_CLASS(CSharedImage)
};
DECLARE_DYN_CLASS_ABSTRACT(CSharedImage, CObject)

class cVCL_API CBitmapImage : public CSharedImage{
private:
	friend class CBitmap;
	friend class CBitmapCanvas;
	friend void DeselectBitmap(HBITMAP AHandle);
    HBITMAP Handle;     // DDB or DIB handle, used for drawing
    HBITMAP MaskHandle; // DDB handle
    HPALETTE Palette;
    HBITMAP DIBHandle;  // DIB handle corresponding to TDIBSection
    DIBSECTION FDIB;
	CMemoryStream* SaveStream; // Save original RLE stream until image is modified
    BOOL OS2Format;  // Write BMP file header, color table in OS/2 format
    BOOL Halftone;   // FPalette is halftone; don't write to file
protected:
    void FreeHandle() override;
public:
	CBitmapImage();
    virtual ~CBitmapImage();

	REF_DYN_CLASS(CBitmapImage)
};
DECLARE_DYN_CLASS(CBitmapImage, CSharedImage)


typedef BYTE TBitmapHandleType;
#define bmDIB				0x01
#define bmDDB				0x02

typedef  WORD TPixelFormat;
#define pfDevice			0x0000
#define pf1Bit				0x0001
#define pf4Bit				0x0002
#define pf8Bit				0x0003
#define pf15Bit				0x0004
#define pf16Bit				0x0005
#define pf24Bit				0x0006
#define pf32Bit				0x0007
#define	pfCustom			0x0008

typedef BYTE TTransparentMode;
#define tmAuto				0x00
#define tmFixed				0x01

class cVCL_API CBitmapCanvas : public CCanvas{
private:
	friend void DeselectBitmap(HBITMAP AHandle);
	CBitmap* Bitmap;
	HBITMAP OldBitmap;
	HPALETTE OldPalette;
public:
    CBitmapCanvas(CBitmap* ABitmap = NULL);
	void FreeContext();
    void CreateHandle() override;
	virtual ~CBitmapCanvas();

	REF_DYN_CLASS(CBitmapCanvas)
};
DECLARE_DYN_CLASS(CBitmapCanvas, CCanvas)

class cVCL_API CBitmap : public CGraphic{
private:
	friend class CBitmapCanvas;
	friend void DeselectBitmap(HBITMAP AHandle);
    CBitmapImage* Image;
	CCanvas* Canvas;
	BOOL IgnorePalette;
	BOOL MaskBitsValid;
	BOOL MaskValid;
	TColor TransparentColor;
	TTransparentMode TransparentMode;
	void Changing(CObject* Sender);
	void CopyImage(HBITMAP AHandle, HPALETTE APalette, DIBSECTION DIB);
	void DIBNeeded();
	void FreeContext();
	void NewImage(HBITMAP NewHandle, HPALETTE NewPalette,
		const DIBSECTION& NewDIB, BOOL OS2Format, CStream* RLEStream = NULL);
	void ReadStream(CStream* Stream, LONG Size);
	void ReadDIB(CStream* Stream, LONG ImageSize, PBITMAPFILEHEADER bmf = NULL);
	BOOL TransparentColorStored();
	void WriteStream(CStream* Stream, BOOL WriteSize);
protected:
    void Changed(CObject* Sender) override;
	void Draw(CCanvas* ACanvas, const TRect& Rect) override;
	BOOL GetEmpty() override;
	void MaskHandleNeeded();
	void ReadData(CStream* Stream) override;
	void WriteData(CStream* Stream) override;
public:
	CBitmap();
	virtual ~CBitmap();
    void HandleNeeded();
	void PaletteNeeded();
	void Assign(CObject* Source) override;
    void Dormant();
    void FreeImage();
	BOOL HandleAllocated();
	void LoadFromClipboardFormat(WORD AFormat, HANDLE AData, HPALETTE APalette) override;
	void LoadFromStream(CStream* Stream) override;
	void LoadFromResourceName(HANDLE Instance, const LPTSTR ResName);
	void LoadFromResourceID(HANDLE Instance, INT ResID);
	void Mask(TColor TransparentColor);
	HBITMAP ReleaseHandle();
    HBITMAP ReleaseMaskHandle();
    HPALETTE ReleasePalette();
	void SaveToClipboardFormat(WORD& Format, HANDLE& Data, HPALETTE& APalette) override;
	void SaveToStream(CStream* Stream) override;

	INT GetWidth() override;
	void SetWidth(INT Value) override;
	INT GetHeight() override;
	void SetHeight(INT Value) override;
	
	HPALETTE GetPalette() override;
	void SetPalette(HPALETTE Value) override;
	CCanvas* GetCanvas();
	virtual HBITMAP GetHandle();
	void SetHandle(HBITMAP Value);
	TBitmapHandleType GetHandleType();
	virtual void SetHandleType(TBitmapHandleType Value);
	virtual HBITMAP GetMaskHandle();
	void SetMaskHandle(HBITMAP Value);
	BOOL GetMonochrome();
	void SetMonochrome(BOOL Value);
	TPixelFormat GetPixelFormat();
	void SetPixelFormat(TPixelFormat Value);
    LPVOID GetScanline(INT Row);
	TColor GetTransparentColor();
	void SetTransparentColor(TColor Value);
    DEFINE_GETTER(TTransparentMode, TransparentMode)
	void SetTransparentMode(TTransparentMode Value);
	DEFINE_ACCESSOR(BOOL, IgnorePalette)

	REF_DYN_CLASS(CBitmap)
};
DECLARE_DYN_CLASS(CBitmap, CGraphic)

#pragma pack (1)
typedef struct {
	WORD Reserved;
	WORD wType;
	WORD Count;
} TCursorOrIcon, *PCursorOrIcon;
typedef struct{
	BYTE Width;
	BYTE Height;
	WORD Colors;
	WORD Reserved1;
	WORD Reserved2;
	DWORD DIBSize;
	DWORD DIBOffset;
} TIconRec, *PIconRec;
#pragma pack ()

class cVCL_API CIconImage : public CSharedImage{
private:
	friend class CIcon;
	HICON Handle;
	CCustomMemoryStream* MemoryImage;
	TPoint Size;
protected:
    void FreeHandle() override;
public:
	CIconImage();
    virtual ~CIconImage();

	REF_DYN_CLASS(CIconImage)
};
DECLARE_DYN_CLASS(CIconImage, CSharedImage)

class cVCL_API CIcon : public CGraphic{
private:
    CIconImage* Image;
    TPoint RequestedSize;
	void HandleNeeded();
	void ImageNeeded();
	void NewImage(HICON NewHandle, CMemoryStream* NewImage);
public:
	CIcon();
	virtual ~CIcon();

    void Draw(CCanvas* ACanvas, const TRect& Rect) override;
	BOOL GetEmpty() override;
	INT GetHeight() override;
	INT GetWidth() override;
	void SetHeight(INT Value) override;
	void SetTransparent(BOOL Value) override;
	void SetWidth(INT Value) override;

	void Assign(CIcon* Source);
	BOOL HandleAllocated();
	void LoadFromClipboardFormat(WORD AFormat, HANDLE AData, HPALETTE APalette) override;
	void LoadFromStream(CStream* Stream) override;
	HICON ReleaseHandle();
	void SaveToClipboardFormat(WORD& Format, HANDLE& Data, HPALETTE& APalette) override;
	void SaveToStream(CStream* Stream) override;
    HICON GetHandle();
	void SetHandle(HICON Value);

	REF_DYN_CLASS(CIcon)
};
DECLARE_DYN_CLASS(CIcon, CGraphic)

class CMetafile;
class cVCL_API CMetafileCanvas : public CCanvas{
private:
	CMetafile* Metafile;
public:
	CMetafileCanvas(CMetafile* AMetafile = NULL, HDC ReferenceDevice = 0, 
		LPTSTR CreateBy = NULL, LPTSTR Description = NULL);
	virtual ~CMetafileCanvas();
  
	REF_DYN_CLASS(CMetafileCanvas)
};
DECLARE_DYN_CLASS(CMetafileCanvas, CCanvas)


class cVCL_API CMetafileImage : public CSharedImage{
private:
	friend class CMetafile;
	HENHMETAFILE Handle;
    INT Width;      // FWidth and FHeight are in 0.01 mm logical pixels
    INT Height;     // These are converted to device pixels in TMetafile
    HPALETTE Palette;
    WORD Inch;          // Used only when writing WMF files.
    INT TempWidth;  // FTempWidth and FTempHeight are in device pixels
    INT TempHeight; // Used only when width/height are set when FHandle = 0
protected:
    void FreeHandle() override;
public:
	CMetafileImage();
    virtual ~CMetafileImage();

	REF_DYN_CLASS(CMetafileImage)
};
DECLARE_DYN_CLASS(CMetafileImage, CSharedImage)

class cVCL_API CMetafile : public CGraphic{
private:
	friend class CMetafileCanvas;
	CMetafileImage* Image;
	BOOL Enhanced;

    void NewImage();
    void UniqueImage();
public:
    CMetafile();
	virtual ~CMetafile();
	BOOL GetEmpty() override;
	INT GetHeight() override;
	HPALETTE GetPalette() override;
    INT GetWidth() override;
    void Draw(CCanvas* ACanvas, const TRect& Rect) override;
    void ReadData(CStream* Stream) override;
	void ReadEMFStream(CStream* Stream);
	void ReadWMFStream(CStream* Stream, INT Length);
	void SetHeight(INT Value) override;
	void SetTransparent(BOOL Value) override;
	void SetWidth(INT Value) override;
	BOOL TestEMF(CStream* Stream);
    void WriteData(CStream* Stream) override;
	void WriteEMFStream(CStream* Stream);
	void WriteWMFStream(CStream* Stream);
	void Clear();
	BOOL HandleAllocated();
	void LoadFromStream(CStream* Stream) override;
	void SaveToFile(const LPTSTR Filename) override;
	void SaveToStream(CStream* Stream) override;
	void LoadFromClipboardFormat(WORD AFormat, HANDLE AData, HPALETTE APalette) override;
	void SaveToClipboardFormat(WORD& AFormat, HANDLE& AData, HPALETTE& APalette) override;
	void Assign(CObject* Source) override;
	HENHMETAFILE ReleaseHandle();
	String GetAuthor();
	String GetDescription();
	DEFINE_ACCESSOR(BOOL, Enhanced)
    HENHMETAFILE GetHandle();
	void SetHandle(HENHMETAFILE Value);
	INT GetMMWidth();
	void SetMMWidth(INT Value);
	INT GetMMHeight();
	void SetMMHeight(INT Value);
	WORD GetInch();
	void SetInch(WORD Value);

	REF_DYN_CLASS(CMetafile)
};
DECLARE_DYN_CLASS(CMetafile, CGraphic)
