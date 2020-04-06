#pragma once

#include "Object.hpp"
#include "List.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "Edit.hpp"
#include "Graphics.hpp"

class CCustomCombo;
typedef void (CObject::*TDrawItemEvent)(CWinControl* Control, INT Index, TRect& Rect, TOwnerDrawState State);
typedef void (CObject::*TMeasureItemEvent)(CWinControl* Control, INT Index, INT& Height);

class cVCL_API CCustomComboBoxStrings : public CStrings{
private:
	CCustomCombo* ComboBox;
protected:
	INT GetCount() override;
	String Get(INT Index) override;
	CObject* GetObject(INT Index) override;
	void PutObject(INT Index, CObject* AObject) override;
	void SetUpdateState(BOOL Updating) override;
public:
	CCustomComboBoxStrings();
	virtual ~CCustomComboBoxStrings();
	void Clear() override;
	void Delete(INT Index) override;
	INT IndexOf(String& S) override;
	DEFINE_ACCESSOR(CCustomCombo*, ComboBox)

	REF_DYN_CLASS(CCustomComboBoxStrings)
};
DECLARE_DYN_CLASS_ABSTRACT(CCustomComboBoxStrings, CStrings)

class cVCL_API CCustomCombo : public CCustomListControl{
private:
    CCanvas* Canvas;
	INT MaxLength;
	INT DropDownCount;
	INT ItemIndex;
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Change)
	DECLARE_TYPE_EVENT(TNotifyEvent, Select)
	DECLARE_TYPE_EVENT(TNotifyEvent, DropDown)
	DECLARE_TYPE_EVENT(TNotifyEvent, CloseUp)
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_CREATE, CCustomCombo::WMCreate)
		MSG_MAP_ENTRY(CM_CANCELMODE, CCustomCombo::CMCancelMode)
		MSG_MAP_ENTRY(CM_CTL3DCHANGED, CCustomCombo::CMCtl3DChanged)
		MSG_MAP_ENTRY(CN_COMMAND, CCustomCombo::CNCommand)
		MSG_MAP_ENTRY(WM_DRAWITEM, CCustomCombo::WMDrawItem)
		MSG_MAP_ENTRY(WM_MEASUREITEM, CCustomCombo::WMMeasureItem)
		MSG_MAP_ENTRY(WM_DELETEITEM, CCustomCombo::WMDeleteItem)
		MSG_MAP_ENTRY(WM_GETDLGCODE, CCustomCombo::WMGetDlgCode)
	MSG_MAP_END()
	void WMCreate(TWMCreate& Message);
	void CMCancelMode(TCMCancelMode& Message);
	void CMCtl3DChanged(TMessage& Message);
	void CNCommand(TWMCommand& Message);
	void WMDrawItem(TWMDrawItem& Message);
	void WMMeasureItem(TWMMeasureItem& Message);
	void WMDeleteItem(TWMDeleteItem& Message);
	void WMGetDlgCode(TWMGetDlgCode& Message);
protected:
	INT ItemHeight;
	CStrings* Items;
	HWND EditHandle;
	HWND ListHandle;
	HWND DropHandle;
	LPVOID EditInstance;
	LPVOID DefEditProc;
	LPVOID ListInstance;
	LPVOID DefListProc;
	BOOL DroppingDown;
	BOOL FocusChanged;
	BOOL IsFocused;
	INT SaveIndex;
	virtual void AdjustDropDown();
	virtual void ComboWndProc(TMessage& Message, HWND ComboWnd, LPVOID ComboProc);
	void CreateWnd() override;
	LRESULT EditWndProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual CCustomComboBoxStringsClass* GetItemsClass() = 0;
	void WndProc(TMessage& Message) override;
	
	INT GetCount() override;
	INT GetItemIndex() override;
	LRESULT ListWndProc(UINT message, WPARAM wParam, LPARAM lParam);
	void Loaded() override;
	virtual void Change();
    virtual void Select();
	virtual void DropDown();
	virtual void CloseUp();
	void DestroyWindowHandle()override;
    virtual void SetItemIndex(INT Value);
public:
    CCustomCombo(CComponent* AOwner = NULL);
	virtual ~CCustomCombo();
	void AddItem(LPTSTR Item, CObject* AObject) override;
	void Clear() override;
	void ClearSelection() override;
	void CopySelection(CCustomListControl* Destination) override;
	void DeleteSelected() override;
	BOOL Focused() override;
	void SelectAll() override;
	BOOL GetDroppedDown();
	void SetDroppedDown(BOOL Value);
	virtual void SetItems(CStrings* Value);
	INT GetSelLength();
	void SetSelLength(INT Value);
	INT GetSelStart();
    void SetSelStart(INT Value);
	virtual void SetDropDownCount(INT Value);
	virtual INT GetItemCount() = 0;
	virtual INT GetItemHeight() = 0;
	virtual void SetItemHeight(INT Value);
    void SetMaxLength(INT Value);

	DEFINE_GETTER(CCanvas*, Canvas)
	DEFINE_GETTER(CStrings*, Items)
	DEFINE_GETTER(INT, DropDownCount)
    DEFINE_GETTER(HWND, EditHandle)
	DEFINE_GETTER(HWND, ListHandle)
	DEFINE_GETTER(INT, MaxLength)

	REF_DYN_CLASS(CCustomCombo)
};
DECLARE_DYN_CLASS_ABSTRACT(CCustomCombo, CCustomListControl)

typedef BYTE TComboBoxStyle;
#define	csDropDown			0x0
#define csSimple			0x1
#define csDropDownList		0x2
#define csOwnerDrawFixed	0x3
#define csOwnerDrawVariable	0x4

class cVCL_API CComboBox : public CCustomCombo{
private:
	BOOL AutoComplete;
	BOOL AutoDropDown;
	UINT LastTime;
	String* Filter;
	TEditCharCase CharCase;
	BOOL Sorted;
	TComboBoxStyle Style;
	CStringList* SaveItems;
	BOOL AutoCloseUp;

	BOOL HasSelectedText(DWORD& StartPos, DWORD& EndPos);
	void DeleteSelectedText();
public:
	DECLARE_TYPE_EVENT(TDrawItemEvent, DrawItem)
	DECLARE_TYPE_EVENT(TMeasureItemEvent, MeasureItem)
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_ERASEBKGND, CComboBox::WMEraseBkgnd)
		MSG_MAP_ENTRY(CM_PARENTCOLORCHANGED, CComboBox::CMParentColorChanged)
		MSG_MAP_ENTRY(CN_DRAWITEM, CComboBox::CNDrawItem)
		MSG_MAP_ENTRY(CN_MEASUREITEM, CComboBox::CNMeasureItem)
		MSG_MAP_ENTRY(WM_LBUTTONDOWN, CComboBox::WMLButtonDown)
		MSG_MAP_ENTRY(WM_PAINT, CComboBox::WMPaint)
		MSG_MAP_ENTRY(WM_NCCALCSIZE, CComboBox::WMNCCalcSize)
	MSG_MAP_END()
	void WMEraseBkgnd(TWMEraseBkgnd& Message);
	void CMParentColorChanged(TMessage& Message);
	void CNDrawItem(TWMDrawItem& Message);
	void CNMeasureItem(TWMMeasureItem& Message);
	void WMLButtonDown(TWMLButtonDown& Message);
	void WMPaint(TWMPaint& Message);
	void WMNCCalcSize(TWMNCCalcSize& Message);
protected:
	void CreateParams(TCreateParams& Params) override;
	void CreateWnd() override;
	void DestroyWnd() override;
	virtual void DrawItem(INT Index, TRect& Rect, TOwnerDrawState State);
	INT GetItemHeight() override;
	CCustomComboBoxStringsClass* GetItemsClass() override;
	void KeyPress(TCHAR& Key) override;
	virtual void MeasureItem(INT Index, INT& Height);
	BOOL SelectItem(String& AnItem);
	void WndProc(TMessage& Message) override;
	INT GetItemCount() override;
public:
	CComboBox(CComponent* AOwner = NULL);
	virtual ~CComboBox();
	void SetCharCase(TEditCharCase Value);
	String GetSelText();
    void SetSelText(String& Value);
	void SetSorted(BOOL Value);
	virtual void SetStyle(TComboBoxStyle Value);

	DEFINE_ACCESSOR(BOOL, AutoComplete)
	DEFINE_ACCESSOR(BOOL, AutoCloseUp)
	DEFINE_ACCESSOR(BOOL, AutoDropDown)
	DEFINE_GETTER(TEditCharCase, CharCase)
	DEFINE_GETTER(BOOL, Sorted)
	DEFINE_GETTER(TComboBoxStyle, Style)

	REF_DYN_CLASS(CComboBox)
};
DECLARE_DYN_CLASS(CComboBox, CCustomCombo)