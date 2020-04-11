#pragma once

#include "Object.hpp"
#include "List.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "Graphics.hpp"
#include "Collection.hpp"
#include "CommonCtl.hpp"
#include "ImgList.hpp"
#include "Timer.hpp"

//TTreeNode

class CTreeView;
class CTreeNodes;

typedef BYTE TNodeState;
#define nsCut			0x0
#define nsDropHilited	0x1
#define nsFocused		0x2
#define nsSelected		0x3
#define nsExpanded		0x4

typedef BYTE TNodeAttachMode;
#define naAdd			0x0
#define	naAddFirst		0x1
#define naAddChild		0x2
#define naAddChildFirst	0x3
#define naInsert		0x4

typedef BYTE TAddMode;
#define taAddFirst		0x0
#define taAdd			0x1
#define taInsert		0x2

#pragma pack (1)
typedef struct _TreeNodeInfo{
	INT ImageIndex;
	INT SelectedIndex;
	INT StateIndex;
	INT OverlayIndex;
	LPVOID Data;
	INT Count;
	TCHAR Text[255];
} TTreeNodeInfo, *PTreeNodeInfo;
#pragma pack ()

class cVCL_API CTreeNode : public CObject{
private:
	friend class CTreeNodes;
	friend class CTreeView;
	friend class CTreeStrings;
	CTreeNodes* Owner;
	String* Text;
	LPVOID Data;
	HTREEITEM ItemId;
	TImageIndex ImageIndex;
	INT SelectedIndex;
	INT OverlayIndex;
	INT StateIndex;
	BOOL Deleting;
	BOOL InTree;
	BOOL CompareCount(INT CompareMe);
	BOOL DoCanExpand(BOOL Expand);
	void DoExpand(BOOL Expand);
	void ExpandItem(BOOL Expand, BOOL Recurse);
	void InternalMove(CTreeNode* ParentNode, CTreeNode* Node, HTREEITEM HItem, TAddMode AddMode);
	void ReadData(CStream* Stream, PTreeNodeInfo Info);
	void SetCut(BOOL Value);
	void SetDropTarget(BOOL Value);
	void SetItem(INT Index, CTreeNode* Value);
	void WriteData(CStream* Stream, PTreeNodeInfo Info);
protected:
	BOOL GetState(TNodeState NodeState);
	void SetState(TNodeState NodeState, BOOL Value);
	void SetSelectedBit(BOOL Value);
public:
    CTreeNode(CTreeNodes* AOwner = NULL);
	virtual ~CTreeNode();
	CTreeView* GetTreeView();
	BOOL AlphaSort(BOOL ARecurse = FALSE);
	void Assign(CObject* Source) override;
	void Collapse(BOOL Recurse);
	BOOL CustomSort(TTVCompare SortProc, LONG_PTR Data, BOOL ARecurse = FALSE);
	void Delete();
	void DeleteChildren();
	TRect DisplayRect(BOOL TextOnly);
	BOOL  EditText();
	void EndEdit(BOOL Cancel);
	void Expand(BOOL Recurse);
	CTreeNode* GetFirstChild(); //GetFirstChild conflicts with C++ macro
	HWND GetHandle();
	CTreeNode* GetLastChild();
	CTreeNode* GetNext();
	CTreeNode* GetNextChild(CTreeNode* Value);
	CTreeNode* GetNextSibling(); //GetNextSibling conflicts with C++ macro
	CTreeNode* GetNextVisible();
	CTreeNode* GetPrev();
	CTreeNode* GetPrevChild(CTreeNode* Value);
	CTreeNode* GetPrevSibling(); //GetPrevSibling conflicts with a C++ macro
	CTreeNode* GetPrevVisible();
	BOOL HasAsParent(CTreeNode* Value);
	INT IndexOf(CTreeNode* Value);
	void MakeVisible();
	virtual void MoveTo(CTreeNode* Destination, TNodeAttachMode Mode);
	BOOL IsFirstNode();
	void SetChildren(BOOL Value);
	void SetData(LPVOID Value);
	void SetExpanded(BOOL Value);
	void SetFocused(BOOL Value);
	void SetImageIndex(TImageIndex Value);
	void SetOverlayIndex(INT Value);
	void SetSelectedIndex(INT Value);
	void SetSelected(BOOL Value);
	void SetStateIndex(INT Value);
	void SetText(String& S);
	INT GetAbsoluteIndex();
	BOOL GetExpanded();
	INT GetLevel();
	CTreeNode* GetParent();
	BOOL GetChildren();
	BOOL GetCut();
	BOOL GetDropTarget();
	BOOL GetFocused();
	INT GetIndex();
	CTreeNode* GetItem(INT Index);
	BOOL GetSelected();
	INT GetCount();
	BOOL IsEqual(CTreeNode* Node);
	BOOL IsNodeVisible();
	
	DEFINE_GETTER(LPVOID, Data)
	DEFINE_GETTER(BOOL, Deleting)
	DEFINE_GETTER(TImageIndex, ImageIndex)
	DEFINE_GETTER(HTREEITEM, ItemId)
	DEFINE_GETTER(INT, OverlayIndex)
	DEFINE_GETTER(CTreeNodes*, Owner)
	DEFINE_GETTER(INT, SelectedIndex)
	DEFINE_GETTER(INT, StateIndex)
	String GetText();

	REF_DYN_CLASS(CTreeNode)
};
DECLARE_DYN_CLASS(CTreeNode, CObject)

//TTreeNodes
#pragma pack (1)
typedef struct _TreeNodeCache{
	CTreeNode* CacheNode;
	INT CacheIndex;
} TTreeNodeCache, *PTreeNodeCache;
#pragma pack ()

class cVCL_API CTreeNodes : public CObject{
private:
	friend class CTreeNode;
	friend class CTreeView;
	friend class CTreeStrings;
	CTreeView* Owner;
	INT UpdateCount;
	TTreeNodeCache NodeCache;
	BOOL Reading;
	void AddedNode(CTreeNode* Value);
	HWND GetHandle();
	CTreeNode* GetNodeFromIndex(INT Index);
	void ReadData(CStream* Stream);
	void ReadDataImpl(CStream* Stream);
	void Repaint(CTreeNode* Node);
	void WriteData(CStream* Stream);
	void ClearCache();
	void WriteExpandedState(CStream* Stream);
	void ReadExpandedState(CStream* Stream);
protected:
	HTREEITEM AddItem(HTREEITEM Parent, HTREEITEM Target, TTVItem Item, TAddMode AddMode);
	//TODO void DefineProperties(TFiler Filer) override;
	TTVItem CreateItem(CTreeNode* Node);
	INT GetCount();
	void SetItem(INT Index, CTreeNode* Value);
	void SetUpdateState(BOOL Updating);
	DEFINE_GETTER(BOOL, Reading)
public:
    CTreeNodes(CTreeView* AOwner = NULL);
	virtual ~CTreeNodes();
	CTreeNode* AddChildFirst(CTreeNode* Parent, String& S);
	CTreeNode* AddChild(CTreeNode* Parent, String& S);
	CTreeNode* AddChildObjectFirst(CTreeNode* Parent, String& S, LPVOID Ptr);
	CTreeNode* AddChildObject(CTreeNode* Parent, String& S, LPVOID Ptr);
	CTreeNode* AddFirst(CTreeNode* Sibling, String& S);
	CTreeNode* Add(CTreeNode* Sibling, String& S);
	CTreeNode* AddObjectFirst(CTreeNode* Sibling, String& S, LPVOID Ptr);
	CTreeNode* AddObject(CTreeNode* Sibling, String& S, LPVOID Ptr);
	CTreeNode* AddNode(CTreeNode* Node, CTreeNode* Relative, String& S,
		LPVOID Ptr, TNodeAttachMode Method);
	BOOL AlphaSort(BOOL ARecurse = FALSE);
	void Assign(CObject* Source) override;
	void BeginUpdate();
	void Clear();
	BOOL CustomSort(TTVCompare SortProc, LONG_PTR Data, BOOL ARecurse = FALSE);
	void Delete(CTreeNode* Node);
	void EndUpdate();
	CTreeNode* GetFirstNode();
	CTreeNode* GetNode(HTREEITEM ItemId);
	CTreeNode* Insert(CTreeNode* Sibling, String& S);
	CTreeNode* InsertObject(CTreeNode* Sibling, String& S, LPVOID Ptr);
	CTreeNode* InsertNode(CTreeNode* Node, CTreeNode* Sibling, String& S, 
		LPVOID Ptr);
	DEFINE_GETTER(CTreeView*, Owner)

	REF_DYN_CLASS(CTreeNodes)
};
DECLARE_DYN_CLASS(CTreeNodes, CObject)


class cVCL_API CTreeStrings : public CStrings{
private:
    CTreeNodes* Owner;
protected:
	String Get(INT Index) override;
	LPTSTR GetBufStart(LPTSTR Buffer, INT& Level);
	INT GetCount() override;
	CObject* GetObject(INT Index) override;
	void PutObject(INT Index, CObject* AObject) override;
	void SetUpdateState(BOOL Updating) override;
public:
	CTreeStrings(CTreeNodes* AOwner = NULL);
	virtual ~CTreeStrings();
	INT Add(String& S) override;
	void Clear() override;
	void Delete(INT Index) override;
	void Insert(INT Index, String& S) override;
	void LoadTreeFromStream(CStream* Stream);
	void SaveTreeToStream(CStream* Stream);
	DEFINE_GETTER(CTreeNodes*, Owner)
	
	REF_DYN_CLASS(CTreeStrings)
};
DECLARE_DYN_CLASS(CTreeStrings, CStrings)

//TCustomTreeView
typedef BYTE TSortType;
#define stNone			0x0
#define stData			0x1
#define stText			0x2
#define stBoth			0x3

typedef BYTE TMultiSelectStyles, TMultiSelectStyle;
#define msControlSelect	0x1
#define	msShiftSelect	0x2
#define msVisibleOnly	0x4
#define msSiblingOnly	0x8

typedef WORD TCustomDrawState;
#define cdsSelected			0x1
#define cdsGrayed			0x2
#define cdsDisabled			0x4
#define cdsChecked			0x8
#define cdsFocused			0x10
#define cdsDefault			0x20
#define cdsHot				0x40
#define cdsMarked			0x80
#define cdsIndeterminate	x0100

typedef BYTE TCustomDrawStage;
#define cdPrePaint		0x0
#define cdPostPaint		0x1
#define cdPreErase		0x2
#define cdPostErase		0x3

typedef BYTE TCustomDrawTarget;
#define dtControl		0x0
#define dtItem			0x1
#define dtSubItem		0x2

typedef void (CObject::*TTVChangingEvent)(CObject* Sender, CTreeNode* Node, BOOL& AllowChange);
typedef void (CObject::*TTVChangedEvent)(CObject* Sender, CTreeNode* Node);
typedef void (CObject::*TTVEditingEvent)(CObject* Sender, CTreeNode* Node, BOOL& AllowEdit);
typedef void (CObject::*TTVEditedEvent)(CObject* Sender, CTreeNode* Node, String& S);
typedef void (CObject::*TTVExpandingEvent)(CObject* Sender, CTreeNode* Node, BOOL& AllowExpansion);
typedef void (CObject::*TTVCollapsingEvent)(CObject* Sender, CTreeNode* Node, BOOL& AllowCollapse);
typedef void (CObject::*TTVExpandedEvent)(CObject* Sender, CTreeNode* Node);
typedef void (CObject::*TTVCompareEvent)(CObject* Sender, CTreeNode* Node1, CTreeNode* Node2,
	INT Data, INT& Compare);
typedef void (CObject::*TTVCustomDrawEvent)(CTreeView* Sender, TRect& ARect, BOOL& DefaultDraw);
typedef void (CObject::*TTVCustomDrawItemEvent)(CTreeView* Sender, CTreeNode* Node, 
	TCustomDrawState State, BOOL& DefaultDraw);
typedef void (CObject::*TTVAdvancedCustomDrawEvent)(CTreeView* Sender, TRect& ARect,
	TCustomDrawStage Stage, BOOL& DefaultDraw);
typedef void (CObject::*TTVAdvancedCustomDrawItemEvent)(CTreeView* Sender, CTreeNode* Node,
	TCustomDrawState State, TCustomDrawStage Stage, BOOL& PaintImages,
	BOOL& DefaultDraw);
typedef void (CObject::*TTVCreateNodeClassEvent)(CTreeView* Sender, CTreeNodeClass** NodeClass);

class cVCL_API CTreeView : public CWinControl{
private:
	friend class CTreeNode;
	friend class CTreeNodes;
	BOOL AutoExpand;
	TBorderStyle BorderStyle;
	CCanvas* Canvas;
	BOOL CanvasChanged;
	LPVOID DefEditProc;
	BOOL Dragged;
	CDragImageList* DragImage;
	CTreeNode* DragNode;
	HWND EditHandle;
	LPVOID EditInstance;
	BOOL HideSelection;
	BOOL HotTrack;
	CChangeLink* ImageChangeLink;
	CCustomImageList* Images;
	CTreeNode* LastDropTarget;
	CMemoryStream* MemStream;
	CTreeNode* RClickNode;
	BOOL RightClickSelect;
	BOOL ManualNotify;
	BOOL ReadOnly;
	BOOL RowSelect;
	INT SaveIndex;
	INT SaveIndent;
	CStringList* SaveItems;
	INT SaveTopIndex;
	BOOL ShowButtons;
	BOOL ShowLines;
	BOOL ShowRoot;
	TSortType SortType;
	BOOL StateChanging;
	CCustomImageList* StateImages;
	CChangeLink* StateChangeLink;
	BOOL ToolTips;
	CTreeNodes* TreeNodes;
	String* WideText;
	BOOL MultiSelect;
	TMultiSelectStyle MultiSelectStyle;
	CList* Selections;
	CList* SaveIndexes;
	CTreeNode* ShiftAnchor;
	BOOL Selecting;
	BOOL SelectChanged;
	HFONT OurFont;
	HFONT StockFont;
	BOOL CreateWndRestores;

	BOOL NodeInList(CList* LNodes, CTreeNode* ANode);
	CTreeNode* SiblingNotInList(CList* LNodes, CTreeNode* ANode);
public:
	DECLARE_TYPE_EVENT(TTVAdvancedCustomDrawEvent, AdvancedCustomDraw)
	DECLARE_TYPE_EVENT(TTVAdvancedCustomDrawItemEvent, AdvancedCustomDrawItem)
	DECLARE_TYPE_EVENT(TTVChangedEvent, CancelEdit)
	DECLARE_TYPE_EVENT(TTVChangedEvent, Change)
	DECLARE_TYPE_EVENT(TTVChangingEvent, Changing)
	DECLARE_TYPE_EVENT(TTVExpandedEvent, Collapsed)
	DECLARE_TYPE_EVENT(TTVCollapsingEvent, Collapsing)
	DECLARE_TYPE_EVENT(TTVCompareEvent, Compare)
	DECLARE_TYPE_EVENT(TTVCustomDrawEvent, CustomDraw)
	DECLARE_TYPE_EVENT(TTVCustomDrawItemEvent, CustomDrawItem)
	DECLARE_TYPE_EVENT(TTVExpandedEvent, Deletion)
	DECLARE_TYPE_EVENT(TTVExpandedEvent, Addition)
	DECLARE_TYPE_EVENT(TTVEditingEvent, Editing)
	DECLARE_TYPE_EVENT(TTVEditedEvent, Edited)
    DECLARE_TYPE_EVENT(TTVExpandedEvent, Expanded)
    DECLARE_TYPE_EVENT(TTVExpandingEvent, Expanding)
    DECLARE_TYPE_EVENT(TTVExpandedEvent, GetImageIndex)
    DECLARE_TYPE_EVENT(TTVExpandedEvent, GetSelectedIndex)
	DECLARE_TYPE_EVENT(TTVCreateNodeClassEvent, CreateNodeClass)
protected:
    void MarkCanvasChanged(CObject* Sender);
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_COLORCHANGED, CTreeView::CMColorChanged)
		MSG_MAP_ENTRY(CM_CTL3DCHANGED, CTreeView::CMCtl3DChanged)
		MSG_MAP_ENTRY(CM_FONTCHANGED, CTreeView::CMFontChanged)
		MSG_MAP_ENTRY(CM_DRAG, CTreeView::CMDrag)
		MSG_MAP_ENTRY(CN_NOTIFY, CTreeView::CNNotify)
		MSG_MAP_ENTRY(WM_LBUTTONDOWN, CTreeView::WMLButtonDown)
		MSG_MAP_ENTRY(WM_NOTIFY, CTreeView::WMNotify)
		MSG_MAP_ENTRY(WM_CONTEXTMENU, CTreeView::WMContextMenu)
		MSG_MAP_ENTRY(CM_SYSCOLORCHANGE, CTreeView::CMSysColorChange)
	MSG_MAP_END()
	void CMColorChanged(TMessage& Message);
	void CMCtl3DChanged(TMessage& Message);
	void CMFontChanged(TMessage& Message);
	void CMDrag(TCMDrag& Message);
	void CNNotify(TWMNotify& Message);
	void WMLButtonDown(TWMLButtonDown& Message);
	void WMNotify(TWMNotify& Message);
	void WMContextMenu(TWMContextMenu& Message);
	void CMSysColorChange(TMessage& Message);
    LRESULT EditWndProc(UINT Message, WPARAM wParam, LPARAM lParam);
	//TODO void DoDragOver(CDragObject* Source, INT X, INT Y, BOOL CanDrop);
	void NodeDeselect(INT Index);
	void NodeSelect(CTreeNode* Node, INT At = 0);
	void FinishSelection(CTreeNode* Node, TShiftState ShiftState);
	void ControlSelectNode(CTreeNode* Node);
	void ShiftSelectNode(CTreeNode* Node, BOOL Backward, BOOL Deselect = TRUE);
	void ControlShiftSelectNode(CTreeNode* Node, BOOL Backward);
	CTreeNode* GetDropTarget();
	CTreeNode* GetNodeFromItem(TTVItem& Item);
	void ImageListChange(CObject* Sender);
	void OnChangeTimer(CObject* Sender);
protected:
	CTimer* ChangeTimer;
	virtual BOOL CanEdit(CTreeNode* Node);
	virtual BOOL CanChange(CTreeNode* Node);
	virtual BOOL CanCollapse(CTreeNode* Node);
	virtual BOOL CanExpand(CTreeNode* Node);
	virtual void Change(CTreeNode* Node);
	virtual void Collapse(CTreeNode* Node);
	virtual CTreeNode* CreateNode();
	virtual CTreeNodes* CreateNodes();
	void CreateParams(TCreateParams& Params) override;
	void CreateWnd() override;
	virtual BOOL CustomDraw(TRect& ARect, TCustomDrawStage Stage);
	virtual BOOL CustomDrawItem(CTreeNode* Node, TCustomDrawState State,
		TCustomDrawStage Stage, BOOL& PaintImages);
	virtual void Delete(CTreeNode* Node);
	virtual void Added(CTreeNode* Node);
	virtual void DestroyWnd() override;
	//TODO void DoEndDrag(CObject* Target, INT X, INT Y) override;
	//TODO void DoStartDrag(TDragObject& DragObject) override;
	virtual void Edit(TTVItem& Item);
	virtual void Expand(CTreeNode* Node);
	//TODO CDragImageList* GetDragImages() override;
	virtual void GetImageIndex(CTreeNode* Node);
	virtual void GetSelectedIndex(CTreeNode* Node);
	virtual BOOL IsCustomDrawn(TCustomDrawTarget Target, TCustomDrawStage Stage);
	void Loaded() override;
	void Notification(CComponent* AComponent, TOperation Operation) override;
	//TODO void SetDragMode(TDragMode Value) override;
    void WndProc(TMessage& Message) override;
	void ValidateSelection();
	void InvalidateSelectionsRects();
	void MouseDown(TMouseButton Button, TShiftState Shift, INT X, INT Y) override;
	void DoEnter() override;
	void DoExit() override;
public:
    CTreeView(CComponent* AOwner = NULL);
	virtual ~CTreeView();
	BOOL AlphaSort(BOOL ARecurse = TRUE);
	BOOL CustomSort(TTVCompare SortProc, LONG_PTR Data, BOOL ARecurse = TRUE);
	void FullCollapse();
	void FullExpand();
	THitTests GetHitTestInfoAt(INT X, INT Y);
	CTreeNode* GetNodeAt(INT X, INT Y);
	BOOL IsEditing();
	void LoadFromFile(String& FileName);
	void LoadFromStream(CStream* Stream);
	void SaveToFile(String& FileName);
	void SaveToStream(CStream* Stream);

	DEFINE_GETTER(CCanvas*, Canvas)

	virtual void Select(CTreeNode* Node, TShiftState ShiftState = 0);
	virtual void Select(CTreeNode* Nodes[], INT Len);
	virtual void Select(CList* Nodes);
	virtual void Deselect(CTreeNode* Node);
	virtual void Subselect(CTreeNode* Node, BOOL Validate = FALSE);
	virtual void ClearSelection(BOOL KeepPrimary = FALSE);
	CTreeNode* GetSelections(CList* AList);
	virtual CTreeNode* FindNextToSelect();
	void SelectNode(CTreeNode* Node);
	INT GetChangeDelay();
	INT GetIndent();
	CTreeNode* GetSelected();
	UINT GetSelectionCount();
	CTreeNode* GetSelection(INT Index);
	CTreeNode* GetTopItem();
	void SetAutoExpand(BOOL Value);
	void SetBorderStyle(TBorderStyle Value);
	void SetButtonStyle(BOOL Value);
	void SetChangeDelay(INT Value);
	void SetDropTarget(CTreeNode* Value);
	void SetHideSelection(BOOL Value);
	void SetHotTrack(BOOL Value);
	void SetImageList(HIMAGELIST Value, INT Flags);
	void SetIndent(INT Value);
	void SetImages(CImageList* Value);
	void SetLineStyle(BOOL Value);
	void SetMultiSelect(BOOL Value);
	void SetMultiSelectStyle(TMultiSelectStyle Value);
	void SetReadOnly(BOOL Value);
	void SetRootStyle(BOOL Value);
	void SetRowSelect(BOOL Value);
	void SetSelected(CTreeNode* Value);
	void SetSortType(TSortType Value);
	void SetStateImages(CImageList* Value);
	void SetToolTips(BOOL Value);
	void SetTreeNodes(CTreeNodes* Value);
	void SetTopItem(CTreeNode* Value);
	
	
	DEFINE_GETTER(BOOL, AutoExpand)
	DEFINE_GETTER(TBorderStyle, BorderStyle)
	DEFINE_ACCESSOR(BOOL, CreateWndRestores)
	DEFINE_GETTER(BOOL, HideSelection)
	DEFINE_GETTER(BOOL, HotTrack)
	DEFINE_GETTER(CCustomImageList*, Images)
	DEFINE_GETTER(CTreeNodes*, TreeNodes)
	DEFINE_GETTER(BOOL, MultiSelect)
	DEFINE_GETTER(TMultiSelectStyle, MultiSelectStyle)
	DEFINE_GETTER(BOOL, ReadOnly)
	DEFINE_ACCESSOR(BOOL, RightClickSelect);
	DEFINE_GETTER(BOOL, RowSelect)
	DEFINE_GETTER(BOOL, ShowButtons)
	DEFINE_GETTER(BOOL, ShowLines)
	DEFINE_GETTER(BOOL, ShowRoot)
	DEFINE_GETTER(TSortType, SortType)
	DEFINE_GETTER(CCustomImageList*, StateImages)
	DEFINE_GETTER(BOOL, ToolTips)

	REF_DYN_CLASS(CTreeView)
};
DECLARE_DYN_CLASS(CTreeView, CWinControl)