#pragma once

#include "Object.hpp"
#include "List.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "Graphics.hpp"

typedef WORD TListBoxStyle;
#define lbStandard				0x0
#define lbOwnerDrawFixed		0x1
#define lbOwnerDrawVariable		0x2
#define lbVirtual				0x3
#define lbVirtualOwnerDraw		0x4

typedef void (CObject::*TDrawItemEvent)(CWinControl* Control, INT Index, TRect& Rect, TOwnerDrawState State);
typedef void (CObject::*TMeasureItemEvent)(CWinControl* Control, INT Index, INT& Height);

typedef void (CObject::*TLBGetDataEvent)(CWinControl* Control, INT Index, String& Data);
typedef void (CObject::*TLBGetDataObjectEvent)(CWinControl* Control, INT Index, CObject** DataObject);
typedef INT (CObject::*TLBFindDataEvent)(CWinControl* Control, String& FindString);

class cVCL_API CListBox : public CCustomMultiSelectListControl{
private:
	BOOL AutoComplete;
	INT Count;
	CStrings* Items;
	String* Filter;
	UINT LastTime;
	TBorderStyle BorderStyle;
	CCanvas* Canvas;
	INT Columns;
	INT ItemHeight;
	INT OldCount;
	TListBoxStyle Style;
	BOOL IntegralHeight;
	BOOL Sorted;
	BOOL ExtendedSelect;
	INT TabWidth;
	CStringList* SaveItems;
	INT SaveTopIndex;
	INT SaveItemIndex;
	BOOL Moving;
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(LB_GETTEXT, CListBox::LBGetText)
		MSG_MAP_ENTRY(LB_GETTEXTLEN, CListBox::LBGetTextLen)
		MSG_MAP_ENTRY(WM_PAINT, CListBox::WMPaint)
		MSG_MAP_ENTRY(WM_SIZE, CListBox::WMSize)
		MSG_MAP_ENTRY(CN_COMMAND, CListBox::CNCommand)
		MSG_MAP_ENTRY(CN_DRAWITEM, CListBox::CNDrawItem)
		MSG_MAP_ENTRY(CN_MEASUREITEM, CListBox::CNMeasureItem)
		MSG_MAP_ENTRY(WM_LBUTTONDOWN, CListBox::WMLButtonDown)
		MSG_MAP_ENTRY(CM_CTL3DCHANGED, CListBox::CMCtl3DChanged)
	MSG_MAP_END()
	void LBGetText(TMessage& Message);
	void LBGetTextLen(TMessage& Message);
    void WMPaint(TWMPaint& Message);
	void WMSize(TWMSize& Message);
	void CNCommand(TWMCommand& Message);
	void CNDrawItem(TWMDrawItem& Message);
	void CNMeasureItem(TWMMeasureItem& Message);
	void WMLButtonDown(TWMLButtonDown& Message);
	void CMCtl3DChanged(TMessage& Message);
public:
	DECLARE_TYPE_EVENT(TDrawItemEvent, DrawItem)
	DECLARE_TYPE_EVENT(TMeasureItemEvent, MeasureItem)
	DECLARE_TYPE_EVENT(TLBGetDataEvent, Data)
	DECLARE_TYPE_EVENT(TLBFindDataEvent, DataFind)
	DECLARE_TYPE_EVENT(TLBGetDataObjectEvent, DataObject)
private:
    void SetColumnWidth();
protected:
    void CreateParams(TCreateParams& Params)override;
	void CreateWnd() override;
	void DestroyWnd() override;
	void WndProc(TMessage& Message) override;
	//void DragCanceled() override;
	virtual void DrawItem(INT Index, TRect& Rect, TOwnerDrawState State);
	INT GetSelCount() override;
	virtual void MeasureItem(INT Index, INT& Height);
	void KeyPress(TCHAR& Key) override;
	void SetMultiSelect(BOOL Value) override;
public:
	CListBox(CComponent* AOwner = NULL);
	virtual ~CListBox();
	void AddItem(LPTSTR Item, CObject* AObject) override;
	void Clear() override;
	void ClearSelection() override;
	void CopySelection(CCustomListControl* Destination) override;
	void DeleteSelected() override;
	INT ItemAtPos(TPoint& Pos, BOOL Existing);
	TRect ItemRect(INT Index);
	void SelectAll() override;
	INT GetItemIndex() override;
	void SetItemIndex(INT Value) override;
	virtual ULONG_PTR InternalGetItemData(INT Index);
	virtual void InternalSetItemData(INT Index, ULONG_PTR AData);
	String DoGetData(INT Index);
	INT DoFindData(String& Data);
	CObject* DoGetDataObject(INT Index);
	virtual ULONG_PTR GetItemData(INT Index);
	virtual void SetItemData(INT Index, ULONG_PTR AData);
	virtual void ResetContent();
	virtual void DeleteString(INT Index);
	
	INT GetCount() override;
	void SetCount(INT Value);
	void SetItems(CStrings* Value);
	BOOL GetSelected(INT Index);
	void SetSelected(INT Index, BOOL Value);
	INT GetScrollWidth();
	void SetScrollWidth(INT Value);
	INT GetTopIndex();
	void SetTopIndex(INT Value);
	void SetBorderStyle(TBorderStyle Value);
	void SetColumns(INT Value);
	void SetExtendedSelect(BOOL Value);
	void SetIntegralHeight(BOOL Value);
	INT GetItemHeight();
    void SetItemHeight(INT Value);
	void SetSorted(BOOL Value);
	void SetStyle(TListBoxStyle Value);	
	void SetTabWidth(INT Value);

	DEFINE_ACCESSOR(BOOL, AutoComplete)
	DEFINE_GETTER(CCanvas*, Canvas)
	DEFINE_GETTER(CStrings*, Items)
	DEFINE_GETTER(TBorderStyle, BorderStyle)
	DEFINE_GETTER(INT, Columns)
	DEFINE_GETTER(BOOL, ExtendedSelect)
	DEFINE_GETTER(INT, IntegralHeight)
	DEFINE_GETTER(BOOL, Sorted)
	DEFINE_GETTER(TListBoxStyle, Style)
	DEFINE_GETTER(INT, TabWidth)
	DEFINE_ACCESSOR(BOOL, Moving)
	
	REF_DYN_CLASS(CListBox)
};
DECLARE_DYN_CLASS(CListBox, CCustomMultiSelectListControl)