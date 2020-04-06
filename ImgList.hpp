#pragma once
#include "stdinc.h"
#include "Commctrl.h"
#include "Object.hpp"
#include "Types.hpp"
#include "Stream.hpp"
#include "Graphics.hpp"

class CCustomImageList;

class cVCL_API CChangeLink : public CObject{
private:
	CCustomImageList* Sender;
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Change)
public:
	CChangeLink();
	virtual ~CChangeLink();
	virtual void Change();
	DEFINE_ACCESSOR(CCustomImageList*, Sender);

  	REF_DYN_CLASS(CChangeLink)
};
DECLARE_DYN_CLASS(CChangeLink, CObject)

typedef BYTE TDrawingStyle;
#define dsFocus			0x0
#define dsSelected		0x1
#define dsNormal		0x2
#define	dsTransparent	0x3

typedef BYTE TImageType;
#define	itImage			0x0
#define itMask			0x1

typedef BYTE TResType;
#define rtBitmap		0x0
#define rtCursor		0x1
#define rtIcon			0x2

typedef BYTE TOverlay; //0..3;

typedef BYTE TLoadResource, TLoadResources;
#define lrDefaultColor		0x1
#define	lrDefaultSize		0x2
#define lrFromFile			0x4
#define lrMap3DColors		0x8
#define lrTransparent		0x10
#define lrMonoChrome		0x20

typedef INT TImageIndex;

class cVCL_API CCustomImageList : public CComponent{
private:
	INT Height;
	INT Width;
	INT AllocBy;
	HIMAGELIST Handle;
	TDrawingStyle DrawingStyle;
	BOOL Masked;
	BOOL ShareImages;
	TImageType ImageType;
	TColor BkColor;
	TColor BlendColor;
	CList* Clients;
	CBitmap* Bitmap;
	CBitmap* MonoBitmap;
	BOOL Changed;
	INT UpdateCount;
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Change)
private:
    void BeginUpdate();
    void EndUpdate();
    void InitBitmap();
    void CheckImage(CGraphic* Image);
	void CopyImages(HIMAGELIST Value, INT Index = -1);
	void CreateImageList();
	BOOL Equal(CCustomImageList* IL);
	void FreeHandle();
	INT GetCount();
	HBITMAP GetBitmapHandle(HBITMAP Bitmap);
	TColor GetBkColor();
	HBITMAP GetImageHandle(CBitmap* Image, CBitmap* ImageDDB);
	void InsertImage(INT Index, CBitmap* Image, CBitmap* Mask, TColor MaskColor);
	BOOL InternalGetInstRes(HANDLE Instance, TResType ResType,
		LPTSTR Name, INT Width, TLoadResources LoadFlags,
		TColor MaskColor);
	void SetBkColor(TColor Value);
	void SetDrawingStyle(TDrawingStyle Value);
	void SetHandle(HIMAGELIST Value);
	void SetHeight(INT Value);
	void SetNewDimensions(HIMAGELIST Value);
	void SetWidth(INT Value);
	void ReadD2Stream(CStream* Stream);
	void ReadD3Stream(CStream* Stream);
protected:
    //void AssignTo(CObject* Dest) override;
	virtual void Change();
	//void DefineProperties(TFiler Filer) override;
    virtual void DoDraw(INT Index, CCanvas* Canvas, INT X, INT Y,
		UINT Style, BOOL Enabled = TRUE);
	void GetImages(INT Index, CBitmap* Image, CBitmap* Mask);
	void HandleNeeded();
	virtual void Initialize();
	virtual void ReadData(CStream* Stream);
	virtual void WriteData(CStream* Stream);
public:
	CCustomImageList(CComponent* AOwner = NULL);
	CCustomImageList(INT AWidth, INT AHeight);
	virtual ~CCustomImageList();
	INT Add(CBitmap* Image, CBitmap* Mask);
	INT AddIcon(CIcon* Image);
	INT AddImage(CCustomImageList* Value, INT Index);
	void AddImages(CCustomImageList* Value);
	INT AddMasked(CBitmap* Image, TColor MaskColor);
	void Clear();
	void Delete(INT Index);
	void Draw(CCanvas* Canvas, INT X, INT Y, INT Index,
		BOOL Enabled = TRUE);
	void Draw(CCanvas* Canvas, INT X, INT Y, INT Index,
		TDrawingStyle ADrawingStyle, TImageType AImageType,
		BOOL Enabled = TRUE);
	void DrawOverlay(CCanvas* Canvas, INT X, INT Y,
		INT ImageIndex, TOverlay Overlay, BOOL Enabled = TRUE);
	void DrawOverlay(CCanvas* Canvas, INT X, INT Y,
		INT ImageIndex, TOverlay Overlay, TDrawingStyle ADrawingStyle,
		TImageType AImageType, BOOL Enabled = TRUE);
	BOOL FileLoad(TResType ResType, const LPTSTR Name, TColor MaskColor);
	BOOL GetBitmap(INT Index, CBitmap* Image);
	virtual TPoint GetHotSpot();
	void GetIcon(INT Index, CIcon* Image);
	void GetIcon(INT Index, CIcon* Image, TDrawingStyle ADrawingStyle,
		TImageType AImageType);
	HBITMAP GetImageBitmap();
	HBITMAP GetMaskBitmap();
	BOOL GetResource(TResType ResType, const LPTSTR Name,
		INT Width, TLoadResources LoadFlags, TColor MaskColor);
	BOOL GetInstRes(HANDLE Instance, TResType ResType, const LPTSTR Name,
		INT Width, TLoadResources LoadFlags, TColor MaskColor);
	BOOL GetInstRes(HANDLE Instance, TResType ResType, ULONG_PTR ResID,
		INT Width, TLoadResources LoadFlags, TColor MaskColor);
	BOOL HandleAllocated();
	void Insert(INT Index, CBitmap* Image, CBitmap* Mask);
	void InsertIcon(INT Index, CIcon* Image);
	void InsertMasked(INT Index, CBitmap* Image, TColor MaskColor);
	void Move(INT CurIndex, INT NewIndex);
	BOOL Overlay(INT ImageIndex, TOverlay Overlay);
	void RegisterChanges(CChangeLink* Value);
	BOOL ResourceLoad(TResType ResType, const LPTSTR Name, TColor MaskColor);
	BOOL ResInstLoad(HANDLE Instance, TResType ResType, 
		const LPTSTR Name, TColor MaskColor);
	void Replace(INT Index, CBitmap* Image, CBitmap* Mask);
	void ReplaceIcon(INT Index, CIcon* Image);
	void ReplaceMasked(INT Index, CBitmap* NewImage, TColor MaskColor);
	void UnRegisterChanges(CChangeLink* Value);
	
	HIMAGELIST GetHandle();

	DEFINE_ACCESSOR(INT, AllocBy)
	DEFINE_ACCESSOR(TColor, BlendColor)
	DEFINE_GETTER(TDrawingStyle, DrawingStyle)
	DEFINE_GETTER(INT, Height)
	DEFINE_ACCESSOR(TImageType, ImageType)
	DEFINE_ACCESSOR(BOOL, Masked)
	DEFINE_ACCESSOR(BOOL, ShareImages)
	DEFINE_GETTER(INT, Width)
	
  	REF_DYN_CLASS(CCustomImageList)
};
DECLARE_DYN_CLASS(CCustomImageList, CComponent)

class cVCL_API CDragImageList : public CCustomImageList{
private:
	TCursor DragCursor;
	BOOL Dragging;
	HWND DragHandle;
	TPoint DragHotspot;
	INT DragIndex;
	void CombineDragCursor();
protected:
    void Initialize() override;
public:
	CDragImageList(CComponent* AOwner = NULL);
	CDragImageList(INT AWidth, INT AHeight);
	virtual ~CDragImageList();
	BOOL BeginDrag(HWND Window, INT X, INT Y);
	BOOL DragLock(HWND Window, INT XPos, INT YPos);
	BOOL DragMove(INT X, INT Y);
	void DragUnlock();
	BOOL EndDrag();
	TPoint GetHotSpot() override;
	void HideDragImage();
	BOOL SetDragImage(INT Index, INT HotSpotX, INT HotSpotY);
	void ShowDragImage();
	DEFINE_GETTER(TCursor, DragCursor)
	DEFINE_GETTER(BOOL, Dragging)
	void SetDragCursor(TCursor Value);

  	REF_DYN_CLASS(CDragImageList)
};
DECLARE_DYN_CLASS(CDragImageList, CCustomImageList)

class cVCL_API CImageList : public CDragImageList{
public:
	CImageList(CComponent* AOwner = NULL);
	CImageList(INT AWidth, INT AHeight);
	virtual ~CImageList();

	REF_DYN_CLASS(CImageList)
};
DECLARE_DYN_CLASS(CImageList, CDragImageList)
