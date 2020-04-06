#include "stdinc.h"
#include "Commctrl.h"
#include "Object.hpp"
#include "Types.hpp"
#include "Stream.hpp"
#include "SysInit.hpp"
#include "WinUtils.hpp"
#include "Graphics.hpp"
#include "ImgList.hpp"
#include "CommonCtl.hpp"

IMPL_DYN_CLASS(CChangeLink)
CChangeLink::CChangeLink():Sender(NULL),INIT_EVENT(Change){
}

CChangeLink::~CChangeLink(){
	if(Sender != NULL)
		Sender->UnRegisterChanges(this);
}

void CChangeLink::Change(){
	if(OnChange != NULL)
		CALL_EVENT(Change)(this);
}

LONG DrawingStyles[] = {ILD_FOCUS, ILD_SELECTED, ILD_NORMAL, ILD_TRANSPARENT};
LONG Images[] = {0, ILD_MASK};

DWORD GetRGBColor(TColor Value){
	DWORD Result = ColorToRGB(Value);
	if(Result == clNone)
		Result = CLR_NONE;
	else if(Result == clDefault)
		Result = CLR_DEFAULT;
	return Result;
}

TColor GetColor(DWORD Value){
	TColor Result = (TColor)Value;
	switch(Value){
	case CLR_NONE:
		Result = clNone;
		break;
	case CLR_DEFAULT:
		Result = clDefault;
		break;
	}
	return Result;
}


IMPL_DYN_CLASS(CCustomImageList)
CCustomImageList::CCustomImageList(CComponent* AOwner) : CComponent(AOwner),
	Height(16),
	Width(16),
	AllocBy(0),
	Handle(0),
	DrawingStyle(dsFocus),
	Masked(FALSE),
	ShareImages(FALSE),
	BkColor(clDefault),
	BlendColor(clDefault),
	Clients(NULL),
	Bitmap(NULL),
	MonoBitmap(NULL),
	Changed(FALSE),
	UpdateCount(0),
	INIT_EVENT(Change){
	Initialize();
}

CCustomImageList::CCustomImageList(INT AWidth, INT AHeight):Height(AHeight),
	Width(AWidth),
	AllocBy(0),
	Handle(0),
	DrawingStyle(dsFocus),
	Masked(FALSE),
	ShareImages(FALSE),
	BkColor(clDefault),
	BlendColor(clDefault),
	Clients(NULL),
	Bitmap(NULL),
	MonoBitmap(NULL),
	Changed(FALSE),
	UpdateCount(0),
	INIT_EVENT(Change){
	Initialize();
}

CCustomImageList::~CCustomImageList(){
	while(Clients != NULL && Clients->GetCount() > 0)
		UnRegisterChanges((CChangeLink *)Clients->Last());
	delete Bitmap;
	FreeHandle();
	delete Clients;
	Clients = NULL;
	if(MonoBitmap != NULL)
		delete MonoBitmap;
}

void CCustomImageList::BeginUpdate(){
	UpdateCount++;
}

void CCustomImageList::EndUpdate(){
	if(UpdateCount > 0)
		UpdateCount--;
	if(Changed){
		Changed = FALSE;
		Change();
	}
}

void CCustomImageList::InitBitmap(){
	HDC ScreenDC = GetDC(0);
	DCHolder dcHolder(ScreenDC, 0, 0);
	Bitmap->SetHandle(CreateCompatibleBitmap(ScreenDC, GetWidth(), GetHeight()));
	Bitmap->GetCanvas()->GetBrush()->SetColor(clBlack);
	Bitmap->GetCanvas()->FillRect(Rect(0, 0, GetWidth(), GetHeight()));
	if(MonoBitmap != NULL){
		delete MonoBitmap;
		MonoBitmap = NULL;
	}
}

void CCustomImageList::CheckImage(CGraphic* Image){
	if(Image != NULL && (Image->GetHeight() < Height || Image->GetWidth() < Width))
		throw "invalid image size";//raise EInvalidOperation.Create(SInvalidImageSize);
}

void CCustomImageList::CopyImages(HIMAGELIST Value, INT Index){
	TRect ARect = Rect(0, 0, GetWidth(), GetHeight());
	CMethodLock lock(this, (TLockMethod)&CCustomImageList::BeginUpdate, (TLockMethod)&CCustomImageList::EndUpdate);
	CBitmap* Image = new CBitmap();
	CObjectHolder imgHolder(Image);
	Image->SetHeight(Height);
	Image->SetWidth(Width);
	CBitmap* Mask = new CBitmap();
	CObjectHolder maskHolder(Mask);
	Mask->SetMonochrome(TRUE);
	Mask->SetHeight(Height);
	Mask->SetWidth(Width);
	INT Count = ImageList_GetImageCount(Value);
	for(INT I = 0; I < Count; I++){
		if(Index == -1 || Index == I){
			Image->GetCanvas()->FillRect(ARect);
			ImageList_Draw(Value, I, Image->GetCanvas()->GetHandle(), 0, 0, ILD_NORMAL);
			Mask->GetCanvas()->FillRect(ARect);
			ImageList_Draw(Value, I, Mask->GetCanvas()->GetHandle(), 0, 0, ILD_MASK);
			Add(Image, Mask);
		}
	}
}

void CCustomImageList::CreateImageList(){
	LONG Mask[] = {0, ILC_MASK};
	Handle = ImageList_Create(GetWidth(), GetHeight(), ILC_COLORDDB | Mask[Masked],
		AllocBy, AllocBy);
	if(!HandleAllocated())
		throw "invalid image list";//raise EInvalidOperation.Create(SInvalidImageList);
	if(BkColor != clNone)
		SetBkColor(BkColor);
}

BOOL StreamsEqual(CMemoryStream* S1, CMemoryStream* S2){
	return S1->GetSize() == S2->GetSize() && memcmp(S1->GetMemory(), S2->GetMemory(), S1->GetSize());
}

BOOL CCustomImageList::Equal(CCustomImageList* IL){
	if(IL == NULL || GetCount() != IL->GetCount())
		return FALSE;
	if(GetCount() == 0 && IL->GetCount() == 0)
		return FALSE;
	CMemoryStream* MyImage = new CMemoryStream();
	CObjectHolder mimgHolder(MyImage);
	WriteData(MyImage);
	CMemoryStream* OtherImage = new CMemoryStream();
	CObjectHolder oimgHolder(OtherImage);
	IL->WriteData(OtherImage);
	return StreamsEqual(MyImage, OtherImage);
}

void CCustomImageList::FreeHandle(){
	if(HandleAllocated() && !GetShareImages())
		ImageList_Destroy(GetHandle());
	Handle = 0;
	Change();
}

INT CCustomImageList::GetCount(){
	if(HandleAllocated())
		return ImageList_GetImageCount(Handle);
	return 0;
}

HBITMAP CCustomImageList::GetBitmapHandle(HBITMAP Bitmap){
	if(Bitmap != 0)
		return Bitmap;
	else
		return this->Bitmap->GetHandle();
}

TColor CCustomImageList::GetBkColor(){
	if(HandleAllocated())
		return GetColor(ImageList_GetBkColor(GetHandle()));
	else
		return BkColor;
}

HIMAGELIST CCustomImageList::GetHandle(){
	HandleNeeded();
	return Handle;
}

HBITMAP CCustomImageList::GetImageHandle(CBitmap* Image, CBitmap* ImageDDB){
	HBITMAP Result = 0;
	CheckImage(Image);
	if(Image != NULL)
		if(Image->GetHandleType() == bmDDB)
			Result = Image->GetHandle();
		else{
			ImageDDB->Assign(Image);
			ImageDDB->SetHandleType(bmDDB);
			Result = ImageDDB->GetHandle();
		}
	else 
		Result = Bitmap->GetHandle();
	return Result;
}

void CCustomImageList::InsertImage(INT Index, CBitmap* Image, CBitmap* Mask, TColor MaskColor){
	CMethodLock Lock(this, (TLockMethod)&CCustomImageList::BeginUpdate, (TLockMethod)&CCustomImageList::EndUpdate);
	CBitmap* OldImage = new CBitmap();
	CObjectHolder oimgHolder(OldImage);
	OldImage->SetHeight(Height);
	OldImage->SetWidth(Width);
	CBitmap* OldMask = new CBitmap();
	CObjectHolder mimgHolder(OldMask);
	OldMask->SetMonochrome(TRUE);
	OldMask->SetHeight(Height);
	OldMask->SetWidth(Width);
	CCustomImageList* TempList = new CCustomImageList(5, 5);
	CObjectHolder tlHolder(TempList);
	TempList->Assign(this);
	Clear();
	if(Index > TempList->GetCount())
		throw "image index error."; //raise EInvalidOperation.Create(SImageIndexError);
	for(INT I = 0; I < Index; I++){
		TempList->GetImages(I, OldImage, OldMask);
		Add(OldImage, OldMask);
	}
	if(MaskColor != -1)
		AddMasked(Image, MaskColor);
	else
		Add(Image, Mask);
	INT Count = TempList->GetCount();
	for(INT I = Index; I < Count; I++){
		TempList->GetImages(I, OldImage, OldMask);
		Add(OldImage, OldMask);
	}
}

BOOL CCustomImageList::InternalGetInstRes(HANDLE Instance, TResType ResType,
	LPTSTR Name, INT Width, TLoadResources LoadFlags,
	TColor MaskColor){
	INT Flags = 0;
	INT ResMap[] = {IMAGE_BITMAP, IMAGE_CURSOR, IMAGE_ICON};
	if(IN_TEST(lrDefaultColor, LoadFlags))
		Flags |= LR_DEFAULTCOLOR;
	if(IN_TEST(lrDefaultSize, LoadFlags))
		Flags |= LR_DEFAULTSIZE;
	if(IN_TEST(lrFromFile, LoadFlags))
		Flags |= LR_LOADFROMFILE;
	if(IN_TEST(lrMap3DColors, LoadFlags))
		Flags |= LR_LOADMAP3DCOLORS;
	if(IN_TEST(lrTransparent, LoadFlags))
		Flags |= LR_LOADTRANSPARENT;
	if(IN_TEST(lrMonoChrome, LoadFlags))
		Flags |= LR_MONOCHROME;
	HIMAGELIST hImage = ImageList_LoadImage((HMODULE)Instance, Name, Width, AllocBy, 
		MaskColor, ResMap[ResType], Flags);
	if(hImage != 0){
		CopyImages(hImage);
		ImageList_Destroy(hImage);
		return TRUE;
	}
	return FALSE;
}

void CCustomImageList::SetBkColor(TColor Value){
	if(HandleAllocated())
		ImageList_SetBkColor(Handle, GetRGBColor(Value));
	else 
		BkColor = Value;
	Change();
}

void CCustomImageList::SetDrawingStyle(TDrawingStyle Value){
	if(Value != GetDrawingStyle()){
		DrawingStyle = Value;
		Change();
	}
}

void CCustomImageList::SetHandle(HIMAGELIST Value){
	FreeHandle();
	if(Value != 0){
		SetNewDimensions(Value);
		Handle = Value;
		Change();
	}
}

void CCustomImageList::SetHeight(INT Value){
	if(Value != GetHeight()){
		Height = Value;
		if(HandleAllocated())
			ImageList_SetIconSize(Handle, GetWidth(), GetHeight());
		Clear();
		InitBitmap();
		Change();
	}
}

void CCustomImageList::SetNewDimensions(HIMAGELIST Value){
	INT AWidth = GetWidth();
	INT AHeight = GetHeight();
	ImageList_GetIconSize(Value, &AWidth, &AHeight);
	Width = AWidth;
	Height = AHeight;
	InitBitmap();

}

void CCustomImageList::SetWidth(INT Value){
	if(Value != GetWidth()){
		Width = Value;
		if(HandleAllocated())
			ImageList_SetIconSize(Handle, GetWidth(), GetHeight());
		Clear();
		InitBitmap();
		Change();
	}
}

void CCustomImageList::ReadD2Stream(CStream* Stream){
	INT Size, Count;
	Stream->ReadBuffer(&Size, sizeof(Size));
	Stream->ReadBuffer(&Count, sizeof(Count));
	CBitmap* FullImage = new CBitmap();
	CObjectHolder fimgHolder(FullImage);
	size_t Pos = Stream->GetPosition();
	FullImage->LoadFromStream(Stream);
    Stream->SetPosition(Pos + Size);
	CBitmap* FullMask = new CBitmap();
	CObjectHolder mimgHolder(FullMask);
	FullMask->LoadFromStream(Stream);
	CBitmap* Image = new CBitmap();
	CObjectHolder imgHolder(Image);
	Image->SetWidth(GetWidth());
	Image->SetHeight(GetHeight());
	CBitmap* Mask = new CBitmap();
	CObjectHolder mskHolder(Mask);
	Mask->SetMonochrome(TRUE);
	Mask->SetWidth(GetWidth());
	Mask->SetHeight(GetHeight());
	TRect SrcRect = Rect(0, 0, GetWidth(), GetHeight());
	CMethodLock Lock(this, (TLockMethod)&CCustomImageList::BeginUpdate, (TLockMethod)&CCustomImageList::EndUpdate);
	INT HC = FullImage->GetHeight() / GetHeight();
	for(INT J = 0; J< HC; J++){
		if(Count == 0)
			break;
		INT WC = FullImage->GetWidth() / GetWidth();
		for(INT I = 0; I< WC; I++){
			if(Count == 0)
				break;
			Image->GetCanvas()->CopyRect(SrcRect, FullImage->GetCanvas(),
				Bounds(I * GetWidth(), J * GetHeight(), GetWidth(), GetHeight()));
			Mask->GetCanvas()->CopyRect(SrcRect, FullMask->GetCanvas(),
				Bounds(I * GetWidth(), J * GetHeight(), GetWidth(), GetHeight()));
            Add(Image, Mask);
            Count--;
		}
	}
}

void CCustomImageList::ReadD3Stream(CStream* Stream){
	// attempt a simple read
	CStreamAdapter* LAdapter = new CStreamAdapter(Stream);
	LAdapter->AddRef();
	CIntfObjectHolder adHolder(LAdapter);
	SetHandle(ImageList_Read(LAdapter));
	// if we were not successful then attempt to fix up the really old ComCtl stream
	if(!HandleAllocated()){
		// make a temp copy of the stream
		BOOL LRetry = FALSE;
		CMemoryStream* LTemp = new CMemoryStream();
		CObjectHolder ltHolder(LTemp);
		Stream->SetPosition(0);
		LTemp->LoadFromStream(Stream);
		// find the bad value imagelist header info
		LTemp->SetPosition(18);
		BYTE LValue = 0;
		BYTE LBitCount = 0;
		if(LTemp->Read(&LValue, 1) == 1 && LValue == 1){
			// find the bitcount data farther on into the BitmapInfoHeader
			LTemp->SetPosition(56);
			if(LTemp->Read(&LBitCount, 1) == 1){
				// correct the original value
				LValue = LValue | LBitCount;
				// back to the imagelist header info and patch it
				LTemp->SetPosition(18);
				LRetry = LTemp->Write(&LValue, 1) == 1;
			}
		}
		// reattempt the load
		if(LRetry){
			LTemp->SetPosition(0);
			CStreamAdapter* LAdapter = new CStreamAdapter(LTemp);
			LAdapter->AddRef();
			CIntfObjectHolder adHolder(LAdapter);
			SetHandle(ImageList_Read(LAdapter));
		}

	  // if we still didn't succeed then fail
	  if(HandleAllocated())
		  throw "image read fail.";//raise EReadError.CreateRes(@SImageReadFail);
	}
  //*/
}

//TODO void CCustomImageList::AssignTo(CObject* Dest){
//}

void CCustomImageList::Change(){
	Changed = TRUE;
	if(UpdateCount > 0)
		return ;
	if(Clients != NULL){
		INT Count = Clients->GetCount();
		for(INT I = 0; I < Count; I++)
			((CChangeLink *)(Clients->Get(I)))->Change();
	}
	if(OnChange != NULL)
		CALL_EVENT(Change)(this);
}

//void CCustomImageList::DefineProperties(TFiler Filer){
//}

#define ROP_DSPDxax		0x00E20746
void CCustomImageList::DoDraw(INT Index, CCanvas* Canvas, INT X, INT Y,
	UINT Style, BOOL Enabled){
	if(HandleAllocated()){
		if(Enabled)
			ImageList_DrawEx(GetHandle(), Index, Canvas->GetHandle(), X, Y, 0, 0,
				GetRGBColor(GetBkColor()), GetRGBColor(GetBlendColor()), Style);
		else{
			if(MonoBitmap == NULL){
				MonoBitmap = new CBitmap();
				MonoBitmap->SetMonochrome(TRUE);
				MonoBitmap->SetWidth(this->GetWidth());
				MonoBitmap->SetHeight(this->GetHeight());
			}
		}
		//Store masked version of image temporarily in FBitmap
		MonoBitmap->GetCanvas()->GetBrush()->SetColor(clWhite);
		MonoBitmap->GetCanvas()->FillRect(Rect(0, 0, GetWidth(), GetHeight()));
		ImageList_DrawEx(GetHandle(), Index, MonoBitmap->GetCanvas()->GetHandle(), 0,0,0,0,
			CLR_NONE, 0, ILD_NORMAL);
		TRect R = Rect(X, Y, X + GetWidth(), Y + GetHeight());
		HDC SrcDC = MonoBitmap->GetCanvas()->GetHandle();
		//Convert Black to clBtnHighlight
		Canvas->GetBrush()->SetColor(clBtnHighlight);
		HDC DestDC = Canvas->GetHandle();
		::SetTextColor(DestDC, clWhite);
		::SetBkColor(DestDC, clBlack);
		BitBlt(DestDC, X+1, Y+1, Width, Height, SrcDC, 0, 0, ROP_DSPDxax);
		//Convert Black to clBtnShadow
		Canvas->GetBrush()->SetColor(clBtnShadow);
		DestDC = Canvas->GetHandle();
		::SetTextColor(DestDC, clWhite);
		::SetBkColor(DestDC, clBlack);
		BitBlt(DestDC, X, Y, GetWidth(), GetHeight(), SrcDC, 0, 0, ROP_DSPDxax);
	}
}

void CCustomImageList::GetImages(INT Index, CBitmap* Image, CBitmap* Mask){
	TRect R = Rect(0, 0, GetWidth(), GetHeight());
	Image->GetCanvas()->GetBrush()->SetColor(clWhite);
	Image->GetCanvas()->FillRect(R);
    ImageList_Draw(GetHandle(), Index, Image->GetCanvas()->GetHandle(), 0, 0, ILD_NORMAL);
	Mask->GetCanvas()->GetBrush()->SetColor(clWhite);
	Mask->GetCanvas()->FillRect(R);
	ImageList_Draw(GetHandle(), Index, Mask->GetCanvas()->GetHandle(), 0, 0, ILD_MASK);
}

void CCustomImageList::HandleNeeded(){
	if(Handle == 0)
		CreateImageList();
}

#define MaxSize 32768
void CCustomImageList::Initialize(){
	Clients = new CList();
	if(GetHeight() < 1 || GetHeight() > MaxSize || GetWidth() < 1)
		throw "invalid image size"; //raise EInvalidOperation.Create(SInvalidImageSize);
	SetAllocBy(4);
	SetMasked(TRUE);
	SetDrawingStyle(dsNormal);
	SetImageType(itImage);
	BkColor = clNone;
	BlendColor = clNone;
	Bitmap = new CBitmap();
	InitBitmap();
}

void CCustomImageList::ReadData(CStream* Stream){
	FreeHandle();
	INT StreamPos = (INT)Stream->GetPosition();       // check stream signature to
	INT CheckInt1 = 0;
	INT CheckInt2 = 0;
	Stream->Read(&CheckInt1, sizeof(CheckInt1)); // determine a Delphi 2 or Delphi
	Stream->Read(&CheckInt2, sizeof(CheckInt2)); // 3 imagelist stream.  Delphi 2
	BYTE CheckByte1 = 0;
	BYTE CheckByte2 = 0;
	CheckByte1 = LOWORD(CheckInt1) & 0xFF;       // streams can be read, but only
	CheckByte2 = (LOWORD(CheckInt1) >> 8) & 0xFF;// 3 streams will be written
	Stream->SetPosition(StreamPos);
	if(CheckInt1 != CheckInt2 && CheckByte1 == 0x49 && CheckByte2 == 0x4C)
		ReadD3Stream(Stream);
	else
		ReadD2Stream(Stream);
}

INT CachedComCtrlVer = 0;
#define ComCtlVersionIE6	0x00060000
typedef HRESULT __stdcall TImageListWriteExProc(HIMAGELIST ImageList, DWORD Flags, IStream* Stream);
TImageListWriteExProc *ImageListWriteExProc = NULL;
void CCustomImageList::WriteData(CStream* Stream){
	if(CachedComCtrlVer == 0){
		CachedComCtrlVer = GetFileVersion(TEXT("comctl32.dll"));
		if(CachedComCtrlVer >= ComCtlVersionIE6){
			HMODULE ComCtrlHandle = GetModuleHandle(TEXT("comctl32.dll"));
			if(ComCtrlHandle != 0)
				ImageListWriteExProc = (TImageListWriteExProc *)GetProcAddress(ComCtrlHandle, "ImageList_WriteEx");//Do not localize
		}
	}
	CStreamAdapter* SA = new CStreamAdapter(Stream);
	SA->AddRef();
	CIntfObjectHolder saHolder(SA);
	//See if we should use the new API for writing image lists in the old format.
    if (ImageListWriteExProc != NULL){
		if(ImageListWriteExProc(GetHandle(), ILP_DOWNLEVEL, static_cast<IStream *>(SA)) != S_OK)
			throw "image write fail.";//raise EWriteError.CreateRes(@SImageWriteFail)
	}
    else if(ImageList_Write(GetHandle(), static_cast<IStream *>(SA)))
		throw "image write fail.";//raise EWriteError.CreateRes(@SImageWriteFail);
}

INT CCustomImageList::Add(CBitmap* Image, CBitmap* Mask){
	CBitmap* ImageDDB = new CBitmap();
	CObjectHolder imgHolder(ImageDDB);
	CBitmap* MaskDDB = new CBitmap();
	CObjectHolder maskHolder(MaskDDB);
	HandleNeeded();
	INT Result = ImageList_Add(Handle, GetImageHandle(Image, ImageDDB), 
		GetImageHandle(Mask, MaskDDB));
	Change();
	return Result;
}

INT CCustomImageList::AddIcon(CIcon* Image){
	INT Result = 0;
	if(Image == NULL)
		Result = Add(NULL, NULL);
	else {
		CheckImage(Image);
		Result = ImageList_AddIcon(GetHandle(), Image->GetHandle());
	}
	Change();
	return Result;
}

INT CCustomImageList::AddImage(CCustomImageList* Value, INT Index){
	INT Result = -1;
	if(Value != NULL){
		Result = GetCount();
		CopyImages(Value->GetHandle(), Index);
	}
	return Result;
}

void CCustomImageList::AddImages(CCustomImageList* Value){
	if(Value != NULL)
		CopyImages(Value->GetHandle());
}

INT CCustomImageList::AddMasked(CBitmap* Image, TColor MaskColor){
	INT Result = 0;
	CBitmap* ImageDDB = new CBitmap();
	CObjectHolder imgHolder(ImageDDB);
	if(GetMasked() && MaskColor != -1){
		CBitmap* T = new CBitmap();
		CObjectHolder tHolder(T);
		T->Assign(Image);
		T->SetTransparentColor(MaskColor);
		HandleNeeded();
		Result = ImageList_Add(Handle, GetImageHandle(Image, ImageDDB),
			GetBitmapHandle(T->GetMaskHandle()));
	}
    else 
		Result = ImageList_Add(GetHandle(), GetImageHandle(Image, ImageDDB), 0);
	Change();
	return Result;
}

void CCustomImageList::Clear(){
	Delete(-1);
}

void CCustomImageList::Delete(INT Index){
	if(Index >= GetCount())
		throw "image index error.";//raise EInvalidOperation.Create(SImageIndexError);
	if(HandleAllocated())
		ImageList_Remove(GetHandle(), Index);
	Change();
}

void CCustomImageList::Draw(CCanvas* Canvas, INT X, INT Y, INT Index,
	BOOL Enabled){
	Draw(Canvas, X, Y, Index, DrawingStyle, ImageType, Enabled);
}

void CCustomImageList::Draw(CCanvas* Canvas, INT X, INT Y, INT Index,
	TDrawingStyle ADrawingStyle, TImageType AImageType,
	BOOL Enabled){
	if(HandleAllocated())
		DoDraw(Index, Canvas, X, Y, DrawingStyles[ADrawingStyle] |
			Images[AImageType], Enabled);

}

void CCustomImageList::DrawOverlay(CCanvas* Canvas, INT X, INT Y,
	INT ImageIndex, TOverlay Overlay, BOOL Enabled){
	DrawOverlay(Canvas, X, Y, ImageIndex, Overlay, dsNormal, itImage, Enabled);
}

void CCustomImageList::DrawOverlay(CCanvas* Canvas, INT X, INT Y,
	INT ImageIndex, TOverlay Overlay, TDrawingStyle ADrawingStyle,
	TImageType AImageType, BOOL Enabled){
	if(HandleAllocated()){
		INT Index = IndexToOverlayMask(Overlay + 1);
		DoDraw(ImageIndex, Canvas, X, Y, DrawingStyles[ADrawingStyle] |
			Images[AImageType] | ILD_OVERLAYMASK & Index, Enabled);
	}
}

BOOL CCustomImageList::FileLoad(TResType ResType, const LPTSTR Name, TColor MaskColor){
	return GetResource(ResType, Name, Width, lrFromFile, MaskColor);
}

BOOL CCustomImageList::GetBitmap(INT Index, CBitmap* Image){
	BOOL Result = Image != NULL && HandleAllocated() && Index > -1 && Index < GetCount();
	if(Result){
		Image->SetHeight(Height);
		Image->SetWidth(Width);
		Draw(Image->GetCanvas(), 0, 0, Index);
	}
	return Result;
}

TPoint CCustomImageList::GetHotSpot(){
	TPoint p;
	p.x = 0;
	p.y = 0;
	return p;
}

void CCustomImageList::GetIcon(INT Index, CIcon* Image){
	GetIcon(Index, Image, DrawingStyle, ImageType);
}

void CCustomImageList::GetIcon(INT Index, CIcon* Image, TDrawingStyle ADrawingStyle,
	TImageType AImageType){
	if(Image != NULL && HandleAllocated())
		Image->SetHandle(ImageList_GetIcon(GetHandle(), Index,
			DrawingStyles[ADrawingStyle] | Images[AImageType]));
}

HBITMAP CCustomImageList::GetImageBitmap(){
	HBITMAP Result = 0;
	_IMAGEINFO Info;
	if(GetCount() > 0 && ImageList_GetImageInfo(GetHandle(), 0, &Info)){
		Result = Info.hbmImage;
		DeleteObject(Info.hbmMask);
	}
	return Result;
}

HBITMAP CCustomImageList::GetMaskBitmap(){
	HBITMAP Result = 0;
	_IMAGEINFO Info;
	if(GetCount() > 0 && ImageList_GetImageInfo(GetHandle(), 0, &Info)){
		Result = Info.hbmMask;
		DeleteObject(Info.hbmImage);
	}
	return Result;
}

BOOL CCustomImageList::GetResource(TResType ResType, const LPTSTR Name,
	INT Width, TLoadResources LoadFlags, TColor MaskColor){
	return GetInstRes(GetGlobal().GetMainInstance(), ResType, Name, Width, LoadFlags, MaskColor);
}

BOOL CCustomImageList::GetInstRes(HANDLE Instance, TResType ResType, const LPTSTR Name,
	INT Width, TLoadResources LoadFlags, TColor MaskColor){
	return InternalGetInstRes(Instance, ResType, Name, Width,
		LoadFlags, MaskColor);
}

BOOL CCustomImageList::GetInstRes(HANDLE Instance, TResType ResType, ULONG_PTR ResID,
	INT Width, TLoadResources LoadFlags, TColor MaskColor){
	return InternalGetInstRes(Instance, ResType, (LPTSTR)ResID, Width, LoadFlags, MaskColor);
}

BOOL CCustomImageList::HandleAllocated(){
	return Handle != 0;
}

void CCustomImageList::Insert(INT Index, CBitmap* Image, CBitmap* Mask){
	InsertImage(Index, Image, Mask, -1);
}

void CCustomImageList::InsertIcon(INT Index, CIcon* Image){
	CMethodLock Lock(this, (TLockMethod)&CCustomImageList::BeginUpdate, (TLockMethod)&CCustomImageList::EndUpdate);
	CCustomImageList* TempList = new CCustomImageList(5, 5);
	CObjectHolder tlHolder(TempList);
    TempList->Assign(this);
	Clear();
	if(Index > TempList->GetCount())
		throw "image index error";//raise EInvalidOperation.Create(SImageIndexError);
    CIcon* Icon = new CIcon();
	CObjectHolder iconHolder(Icon);
	for (INT I = 0; I < Index; I++){
		TempList->GetIcon(I, Icon);
		AddIcon(Icon);
	}
    AddIcon(Image);
	INT Count = TempList->GetCount();
    for(INT I = Index; I < Count; I++){
		TempList->GetIcon(I, Icon);
		AddIcon(Icon);
	}
}

void CCustomImageList::InsertMasked(INT Index, CBitmap* Image, TColor MaskColor){
	InsertImage(Index, Image, NULL, MaskColor);
}

void CCustomImageList::Move(INT CurIndex, INT NewIndex){
	if(CurIndex != NewIndex){
		CBitmap* Image = new CBitmap();
		CObjectHolder imgHolder(Image);
		Image->SetHeight(Height);
		Image->SetWidth(Width);
		CBitmap* Mask = new CBitmap();
		CObjectHolder maskHolder(Mask);
		Mask->SetHeight(Height);
		Mask->SetWidth(Width);
		GetImages(CurIndex, Image, Mask);
		Delete(CurIndex);
        Insert(NewIndex, Image, Mask);
	}
}

BOOL CCustomImageList::Overlay(INT ImageIndex, TOverlay Overlay){
	if(HandleAllocated())
		return ImageList_SetOverlayImage(Handle, ImageIndex, Overlay + 1);
	return FALSE;
}

void CCustomImageList::RegisterChanges(CChangeLink* Value){
	Value->SetSender(this);
	if(Clients != NULL)
		Clients->Add(Value);
}

BOOL CCustomImageList::ResourceLoad(TResType ResType, const LPTSTR Name, TColor MaskColor){
	BOOL Result = FALSE;
	if(GetGlobal().GetHInstance() == GetGlobal().GetMainInstance())
		Result = GetInstRes(GetGlobal().GetMainInstance(), ResType, Name, Width, 0, MaskColor);
	else{
		/* TODO
		LibModule = LibModuleList;
    while LibModule <> nil do
      with LibModule^ do
      begin
        Result := GetInstRes(ResInstance, ResType, Name, Width, [], MaskColor);
        if not Result and (Instance <> ResInstance) then
          Result := GetInstRes(Instance, ResType, Name, Width, [], MaskColor);
        if Result then Exit;
        LibModule := LibModule.Next;
      end;
	  //*/
	}
	return Result;
}

BOOL CCustomImageList::ResInstLoad(HANDLE Instance, TResType ResType, 
	const LPTSTR Name, TColor MaskColor){
	return GetInstRes(Instance, ResType, Name, Width, 0, MaskColor);
}

void CCustomImageList::Replace(INT Index, CBitmap* Image, CBitmap* Mask){
	CBitmap* ImageDDB = new CBitmap();
	CObjectHolder imgHolder(ImageDDB);
	CBitmap* MaskDDB = new CBitmap();
	CObjectHolder maskHolder(MaskDDB);
	if(HandleAllocated() && !ImageList_Replace(GetHandle(), Index,
		GetImageHandle(Image, ImageDDB), GetImageHandle(Mask, MaskDDB)))
		throw "invalid operation: replace image";//raise EInvalidOperation.Create(SReplaceImage);
	Change();
}

void CCustomImageList::ReplaceIcon(INT Index, CIcon* Image){
	if(HandleAllocated()){
		if(Image == NULL)
			Replace(Index, NULL, NULL);
		else{
			CheckImage(Image);
			if(ImageList_ReplaceIcon(GetHandle(), Index, Image->GetHandle()) == -1)
				throw "invalid operation : replace image.";//raise EInvalidOperation.Create(SReplaceImage);
		}
	}
	Change();
}

class TempIndexDeleter{
private:
	CCustomImageList* L;
	INT V;
public:
	TempIndexDeleter(CCustomImageList* AL, INT A):L(AL),V(A){};
	virtual ~TempIndexDeleter(){
		L->Delete(V);
	}
};
void CCustomImageList::ReplaceMasked(INT Index, CBitmap* NewImage, TColor MaskColor){
	if(HandleAllocated()){
		CheckImage(NewImage);
		INT TempIndex = AddMasked(NewImage, MaskColor);
		if(TempIndex != -1){
			TempIndexDeleter Deleter(this, TempIndex);
			CBitmap* Image = new CBitmap();
			CObjectHolder imgHolder(Image);
			Image->SetHeight(Height);
			Image->SetWidth(Width);
			CBitmap* Mask = new CBitmap();
			CObjectHolder maskHolder(Mask);
			Mask->SetMonochrome(TRUE);
			Mask->SetHeight(Height);
			Mask->SetWidth(Width);
			ImageList_Draw(GetHandle(), TempIndex, Image->GetCanvas()->GetHandle(), 0, 0, ILD_NORMAL);
			ImageList_Draw(GetHandle(), TempIndex, Mask->GetCanvas()->GetHandle(), 0, 0, ILD_MASK);
			if(!ImageList_Replace(GetHandle(), Index, Image->GetHandle(), Mask->GetHandle()))
				throw "invalid operation: replace image.";//raise EInvalidOperation.Create(SReplaceImage);
		}
		else
			throw "invalid operation replace image.";//raise EInvalidOperation.Create(SReplaceImage);
	}
	Change();
}

void CCustomImageList::UnRegisterChanges(CChangeLink* Value){
	if(Clients != NULL){
		INT Count = Clients->GetCount();
		for(INT I = 0; I < Count; I++){
			if(Clients->Get(I) == Value){
				Value->SetSender(NULL);
				Clients->Delete(I);
				break;
			}
		}
	}
}

IMPL_DYN_CLASS(CDragImageList)
CDragImageList::CDragImageList(CComponent* AOwner) : CCustomImageList(AOwner),
	DragCursor(crDefault),
	Dragging(FALSE),
	DragHandle(0),
	DragIndex(0){
	DragHotspot.x = 0;
	DragHotspot.y = 0;
}

CDragImageList::CDragImageList(INT AWidth, INT AHeight): CCustomImageList(AWidth, AHeight),
	DragCursor(crDefault),
	Dragging(FALSE),
	DragHandle(0),
	DragIndex(0){
	DragHotspot.x = 0;
	DragHotspot.y = 0;
}

CDragImageList::~CDragImageList(){
}

void CDragImageList::CombineDragCursor(){
	if(GetDragCursor() != crNone){
		TPoint Point;
		HIMAGELIST TempList = ImageList_Create(GetSystemMetrics(SM_CXCURSOR),
			GetSystemMetrics(SM_CYCURSOR), ILC_MASK, 1, 1);
		__try{
			//TODO ImageList_AddIcon(TempList, Screen->GetCursor(DragCursor));
			//ImageList_AddIcon(TempList, Screen->GetCursor(DragCursor));
			ImageList_SetDragCursorImage(TempList, 0, 0, 0);
			ImageList_GetDragImage(NULL, &Point);
			ImageList_SetDragCursorImage(TempList, 1, Point.x, Point.y);
		}
		__finally{
			ImageList_Destroy(TempList);
		}
	}
}

void CDragImageList::Initialize(){
	__super::Initialize();
	SetDragCursor(crNone);
}
	
BOOL CDragImageList::BeginDrag(HWND Window, INT X, INT Y){
	BOOL Result = FALSE;
	if(HandleAllocated()){
		if(!GetDragging())
			SetDragImage(DragIndex, DragHotspot.x, DragHotspot.y);
		CombineDragCursor();
		Result = DragLock(Window, X, Y);
		if(Result)
			ShowCursor(FALSE);
	}
	return Result;
}

BOOL CDragImageList::DragLock(HWND Window, INT XPos, INT YPos){
	BOOL Result = FALSE;
	if(HandleAllocated() && Window != DragHandle){
		DragUnlock();
		DragHandle = Window;
		TPoint pt = ClientToWindow(DragHandle, XPos, YPos);
		Result = ImageList_DragEnter(DragHandle, pt.x, pt.y);
	}
	return Result;
}

BOOL CDragImageList::DragMove(INT X, INT Y){
	if(HandleAllocated()){
		TPoint pt = ClientToWindow(DragHandle, X, Y);
		return ImageList_DragMove(pt.x, pt.y);
	}
	return FALSE;
}

void CDragImageList::DragUnlock(){
	if(HandleAllocated() && DragHandle != 0){
		ImageList_DragLeave(DragHandle);
		DragHandle = 0;
	}
}

BOOL CDragImageList::EndDrag(){
	BOOL Result = FALSE;
	if(HandleAllocated() && GetDragging()){
		DragUnlock();
		ImageList_EndDrag();
		Result = TRUE;
		Dragging = FALSE;
		SetDragCursor(crNone);
		ShowCursor(TRUE);
	}
	else
		Result = FALSE;
	return Result;

}

TPoint CDragImageList::GetHotSpot(){
	TPoint Result = __super::GetHotSpot();
	if(HandleAllocated() && GetDragging())
		ImageList_GetDragImage(NULL, &Result);
	return Result;
}

void CDragImageList::HideDragImage(){
	if(HandleAllocated())
		ImageList_DragShowNolock(FALSE);
}

BOOL CDragImageList::SetDragImage(INT Index, INT HotSpotX, INT HotSpotY){
	BOOL Result = FALSE;
	if(HandleAllocated()){
		DragIndex = Index;
		DragHotspot.x = HotSpotX;
		DragHotspot.y = HotSpotY;
		ImageList_BeginDrag(GetHandle(), Index, HotSpotX, HotSpotY);
		Result = TRUE;
		Dragging = Result;
	}
	else
		Result = FALSE;
	return Result;
}

void CDragImageList::ShowDragImage(){
	if(HandleAllocated())
		ImageList_DragShowNolock(TRUE);
}

void CDragImageList::SetDragCursor(TCursor Value){
	if(Value != GetDragCursor()){
		DragCursor = Value;
		if(GetDragging())
			CombineDragCursor();
	}
}

IMPL_DYN_CLASS(CImageList)
CImageList::CImageList(CComponent* AOwner) : CDragImageList(AOwner){
}
CImageList::CImageList(INT AWidth, INT AHeight):CDragImageList(AWidth, AHeight){
}
CImageList::~CImageList(){
}
