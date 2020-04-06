#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "ImgList.hpp"
#include "Strings.hpp"

typedef WORD THitTest,THitTests;
#define htAbove			0x0001
#define	htBelow			0x0002
#define htNowhere		0x0004
#define htOnItem		0x0008
#define htOnButton		0x0010
#define htOnIcon		0x0020
#define htOnIndent		0x0040
#define htOnLabel		0x0080
#define htOnRight		0x0100
#define htOnStateIcon	0x0200
#define htToLeft		0x0400
#define htToRight		0x0800

typedef BYTE TTabPosition;
#define tpTop			0x0
#define tpBottom		0x1
#define tpLeft			0x2
#define tpRight			0x3

typedef BYTE TTabStyle;
#define tsTabs			0x0
#define tsButtons		0x1
#define tsFlatButtons	0x2

class CTabControl;
typedef void (CObject::*TTabChangingEvent)(CObject* Sender, BOOL& AllowChange);
typedef void (CObject::*TDrawTabEvent)(CTabControl* Control, INT TabIndex, TRect& Rect, BOOL Active);
typedef void (CObject::*TTabGetImageEvent)(CObject* Sender, INT TabIndex, INT& ImageIndex);

class cVCL_API CTabControl : public CWinControl{
private:
    CCanvas* Canvas;
	BOOL HotTrack;
	CChangeLink* ImageChangeLink;
	CCustomImageList* Images;
	BOOL MultiLine;
	BOOL MultiSelect;
	BOOL OwnerDraw;
	BOOL RaggedRight;
	INT SaveTabIndex;
	CStringList* SaveTabs;
	BOOL ScrollOpposite;
	TTabStyle Style;
    TTabPosition TabPosition;
	CStrings* Tabs;
    TSmallPoint TabSize;
	BOOL Updating;
	TRect SavedAdjustRect;
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Change);
	DECLARE_TYPE_EVENT(TTabChangingEvent, Changing);
	DECLARE_TYPE_EVENT(TDrawTabEvent, DrawTab);
	DECLARE_TYPE_EVENT(TTabGetImageEvent, GetImageIndex);
private:
	void ImageListChange(CObject* Sender);
	BOOL InternalSetMultiLine(BOOL Value);
	void UpdateTabSize();
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_FONTCHANGED, CTabControl::CMFontChanged)
		MSG_MAP_ENTRY(CM_SYSCOLORCHANGE, CTabControl::CMSysColorChange)
		MSG_MAP_ENTRY(CM_TABSTOPCHANGED, CTabControl::CMTabStopChanged)
		MSG_MAP_ENTRY(CN_NOTIFY, CTabControl::CNNotify)
		MSG_MAP_ENTRY(CM_DIALOGCHAR, CTabControl::CMDialogChar)
		MSG_MAP_ENTRY(CN_DRAWITEM, CTabControl::CNDrawItem)
		MSG_MAP_ENTRY(TCM_ADJUSTRECT, CTabControl::TCMAdjustRect)
		MSG_MAP_ENTRY(WM_DESTROY, CTabControl::WMDestroy)
		MSG_MAP_ENTRY(WM_NOTIFYFORMAT, CTabControl::WMNotifyFormat)
		MSG_MAP_ENTRY(WM_SIZE, CTabControl::WMSize)
	MSG_MAP_END()
	void CMFontChanged(TMessage& Message);
	void CMSysColorChange(TMessage& Message);
	void CMTabStopChanged(TMessage& Message);
	void CNNotify(TWMNotify& Message);
	void CMDialogChar(TCMDialogChar& Message);
	void CNDrawItem(TWMDrawItem& Message);
	void TCMAdjustRect(TMessage& Message);
	void WMDestroy(TWMDestroy& Message);
	void WMNotifyFormat(TMessage& Message);
	void WMSize(TMessage& Message);
protected:
	void AdjustClientRect(TRect& Rect) override;
	virtual BOOL CanChange();
	virtual BOOL CanShowTab(INT TabIndex);
	virtual void Change();
	void CreateParams(TCreateParams& Params) override;
	void CreateWnd() override;
	virtual void DrawTab(INT TabIndex, TRect& Rect, BOOL Active);
	void Loaded() override;
	void UpdateTabImages();
public:
	CTabControl(CComponent* AOwner = NULL);
	virtual ~CTabControl();
	INT IndexOfTabAt(INT X, INT Y);
	THitTests GetHitTestInfoAt(INT X, INT Y);
	TRect TabRect(INT Index);
	INT RowCount();
	void ScrollTabs(INT Delta);
	void Notification(CComponent* AComponent, TOperation Operation) override;
	virtual void SetTabIndex(INT Value);
	virtual INT GetImageIndex(INT TabIndex);
	void TabsChanged();

	SHORT GetTabHeight();
	SHORT GetTabWidth();
	
	TRect GetDisplayRect();
	INT GetTabIndex();
	void SetHotTrack(BOOL Value);
	void SetImages(CCustomImageList* Value);
	void SetMultiLine(BOOL Value);
	void SetMultiSelect(BOOL Value);
	void SetOwnerDraw(BOOL Value);
	void SetRaggedRight(BOOL Value);
	void SetScrollOpposite(BOOL Value);
	void SetStyle(TTabStyle Value);
	void SetTabHeight(SHORT Value);
	void SetTabPosition(TTabPosition Value);
	void SetTabs(CStrings* Value);
	void SetTabWidth(SHORT Value);
    
	DEFINE_GETTER(CCanvas*, Canvas)
	DEFINE_GETTER(BOOL, HotTrack)
	DEFINE_GETTER(CCustomImageList*, Images)
	DEFINE_GETTER(BOOL, MultiLine)
	DEFINE_GETTER(BOOL, MultiSelect)
	DEFINE_GETTER(BOOL, OwnerDraw)
	DEFINE_GETTER(BOOL, RaggedRight)
	DEFINE_GETTER(BOOL, ScrollOpposite)
	DEFINE_GETTER(TTabStyle, Style)
	DEFINE_GETTER(TTabPosition, TabPosition)
	DEFINE_GETTER(CStrings*, Tabs)
	DEFINE_ACCESSOR(BOOL, Updating)

	REF_DYN_CLASS(CTabControl)
};
DECLARE_DYN_CLASS(CTabControl, CWinControl)

class CPageControl;
class cVCL_API CTabSheet : public CWinControl{
private:
	friend class CPageControl;
	TImageIndex ImageIndex;
	CPageControl* PageControl;
	BOOL TabVisible;
	BOOL TabShowing;
	BOOL Highlighted;
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Hide);
	DECLARE_TYPE_EVENT(TNotifyEvent, Show);
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_TEXTCHANGED, CTabSheet::CMTextChanged)
		MSG_MAP_ENTRY(CM_SHOWINGCHANGED, CTabSheet::CMShowingChanged)
		MSG_MAP_ENTRY(WM_NCPAINT, CTabSheet::WMNCPaint)
		MSG_MAP_ENTRY(WM_PRINTCLIENT, CTabSheet::WMPrintClient)
	MSG_MAP_END()
	void CMTextChanged(TMessage& Message);
	void CMShowingChanged(TMessage& Message);
	void WMNCPaint(TWMNCPaint& Message);
	void WMPrintClient(TWMPrintClient& Message);
protected:
	void CreateParams(TCreateParams& Params) override;
	virtual void DoHide();
	virtual void DoShow();
	//TODO void ReadState(CReader* Reader) override;
public:
	CTabSheet(CComponent* AOwner = NULL);
	virtual ~CTabSheet();
	INT GetPageIndex();
	INT GetTabIndex();
	void SetHighlighted(BOOL Value);
	void SetImageIndex(TImageIndex Value);
	void SetPageControl(CPageControl* APageControl);
	void SetPageIndex(INT Value);
	void SetTabShowing(BOOL Value);
	void SetTabVisible(BOOL Value);
	void UpdateTabShowing();

	DEFINE_GETTER(CPageControl*, PageControl)
	DEFINE_GETTER(BOOL, Highlighted)
	DEFINE_GETTER(TImageIndex, ImageIndex)
	DEFINE_GETTER(BOOL, TabVisible)

	REF_DYN_CLASS(CTabSheet)
};
DECLARE_DYN_CLASS(CTabSheet, CWinControl)

class cVCL_API CPageControl : public CTabControl{
private:
	friend class CTabSheet;
	CList* Pages;
	CTabSheet* ActivePage;
	CTabSheet* NewDockSheet;
	CTabSheet* UndockingPage;
	BOOL InSetActivePage;
	void DeleteTab(CTabSheet* Page, INT Index);
	void InsertPage(CTabSheet* Page);
	void InsertTab(CTabSheet* Page);
	void MoveTab(INT CurIndex, INT NewIndex);
	void SetActivePageIndex(INT Value);
	void ChangeActivePage(CTabSheet* Page);
	CControl* GetDockClientFromMousePos(TPoint& MousePos);
	void UpdateTab(CTabSheet* Page);
	void UpdateTabHighlights();
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_DESIGNHITTEST, CPageControl::CMDesignHitTest)
		MSG_MAP_ENTRY(CM_DIALOGKEY, CPageControl::CMDialogKey)
		MSG_MAP_ENTRY(CM_DOCKCLIENT, CPageControl::CMDockClient)
		MSG_MAP_ENTRY(CM_DOCKNOTIFICATION, CPageControl::CMDockNotification)
		MSG_MAP_ENTRY(CM_UNDOCKCLIENT, CPageControl::CMUnDockClient)
		MSG_MAP_ENTRY(WM_LBUTTONDOWN, CPageControl::WMLButtonDown)
		MSG_MAP_ENTRY(WM_LBUTTONDBLCLK, CPageControl::WMLButtonDblClk)
		MSG_MAP_ENTRY(WM_ERASEBKGND, CPageControl::WMEraseBkGnd)
	MSG_MAP_END()
	void CMDesignHitTest(TCMDesignHitTest& Message);
	void CMDialogKey(TCMDialogKey& Message);
	void CMDockClient(TCMDockClient& Message);
	void CMDockNotification(TCMDockNotification& Message);
	void CMUnDockClient(TCMUnDockClient& Message);
	void WMLButtonDown(TWMLButtonDown& Message);
	void WMLButtonDblClk(TWMLButtonDblClk& Message);
	void WMEraseBkGnd(TWMEraseBkgnd& Message);
protected:
	BOOL CanShowTab(INT TabIndex) override;
	void Change() override;
	//TODO void DoAddDockClient(CControl* Client, TRect& ARect) override;
	//void DockOver(CDragDockObject* Source, INT X, INT Y, TDragState State, BOOL& Accept) override;
	//void DoRemoveDockClient(CControl* Client) override;
	//void GetChildren(TGetChildProc* Proc, CComponent* Root) override;
	INT GetImageIndex(INT TabIndex) override;
	CTabSheet* GetPageFromDockClient(CControl* Client);
	//TODO void GetSiteInfo(CControl* Client, TRect& InfluenceRect, TPoint& MousePos, BOOL& CanDock) override;
	void Loaded() override;
	void SetChildOrder(CComponent* Child, INT Order) override;
	void SetTabIndex(INT Value) override;
	void ShowControl(CControl* AControl) override;
	virtual void UpdateActivePage();
public:
    CPageControl(CComponent* AOwner = NULL);
	virtual ~CPageControl();
	CTabSheet* FindNextPage(CTabSheet* CurPage, BOOL GoForward, BOOL CheckTabVisible);
	void SelectNextPage(BOOL GoForward, BOOL CheckTabVisible = TRUE);
	
	void RemovePage(CTabSheet* Page);
	void SetActivePage(CTabSheet* Page);
	CTabSheet* GetPage(INT Index);
	INT GetPageCount();
	INT GetActivePageIndex();
	
	DEFINE_GETTER(CTabSheet*, ActivePage)

	REF_DYN_CLASS(CPageControl)
};
DECLARE_DYN_CLASS(CPageControl, CTabControl)