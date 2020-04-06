#include "stdinc.h"
#include "TreeView.hpp"
#include "WinUtils.hpp"
#include "Stream.hpp"
#include "ObjWndProc.hpp"
#include "SysInit.hpp"

IMPL_DYN_CLASS(CTreeNode)
CTreeNode::CTreeNode(CTreeNodes* AOwner):OverlayIndex(-1),
	StateIndex(-1),
	Data(NULL),
	ItemId(0),
	ImageIndex(-1),
	SelectedIndex(-1),
	Deleting(FALSE),
	InTree(FALSE){
	this->Owner = Owner;
	Text = new String(NULL);
}

CTreeNode::~CTreeNode(){
	Owner->ClearCache();
	Deleting = TRUE;
	if(Owner->GetOwner() != NULL){
		Owner->GetOwner()->Selections->Remove(this);
		if(Owner->GetOwner()->LastDropTarget == this)
			Owner->GetOwner()->LastDropTarget = NULL;
	}
	CTreeNode* Node = GetParent();
	INT  CheckValue = -1;
	if(Node != NULL && !Node->GetDeleting()){
		if(Node->IndexOf(this) != -1)
			CheckValue = 1;
		else
			CheckValue = 0;
		if(Node->CompareCount(CheckValue)){
			SetExpanded(FALSE);
			Node->SetChildren(FALSE);
		}
	}
	if(Owner->GetOwner() != NULL)
		Owner->GetOwner()->Delete(this);
	if(ItemId != 0)
		TreeView_DeleteItem(GetHandle(), ItemId);
	Data = NULL;
	delete Text;
}

BOOL CTreeNode::CompareCount(INT CompareMe){
	INT Count = 0;
	BOOL Result = FALSE;
	CTreeNode* Node = GetFirstChild();
	while(Node != NULL){
		Count++;
		Node = Node->GetNextChild(Node);
		if(Count > CompareMe)
			return Result;
	}
	if(Count == CompareMe)
		Result = TRUE;
	return Result;
}

BOOL CTreeNode::DoCanExpand(BOOL Expand){
	if(!GetDeleting())
	if(GetChildren()){
		if(Expand)
			return GetTreeView()->CanExpand(this);
		else
			return GetTreeView()->CanCollapse(this);
	}
	return FALSE;
}

void CTreeNode::DoExpand(BOOL Expand){
	if(!GetDeleting() && GetChildren()){
		if(Expand)
			GetTreeView()->Expand(this);
		else
			GetTreeView()->Collapse(this);
	}
}

void CTreeNode::ExpandItem(BOOL Expand, BOOL Recurse){
	if(!GetDeleting())
    if(Recurse){
		CTreeNode* Node = this;
		do{
			Node->ExpandItem(Expand, FALSE);
			Node = Node->GetNext();
		}while(Node != NULL && Node->HasAsParent(this));
	}
	else{
		GetTreeView()->ManualNotify = TRUE;
		__try{
			INT Flag = 0;
			if(Expand){
				if(DoCanExpand(TRUE)){
					Flag = TVE_EXPAND;
					DoExpand(TRUE);
				}
			}
			else{
				if(DoCanExpand(FALSE)){
					Flag = TVE_COLLAPSE;
					DoExpand(FALSE);
				}
			}
			if(Flag != 0)
				TreeView_Expand(GetHandle(), ItemId, Flag);
		}
		__finally{
			GetTreeView()->ManualNotify = FALSE;
		}
	}
}

INT CTreeNode::GetAbsoluteIndex(){
	if(Owner->NodeCache.CacheNode == this)
		return Owner->NodeCache.CacheIndex;
	else{
		if(IsFirstNode())
			return 0;
		else{
			INT Result = -1;
			CTreeNode* Node = this;
			while(Node != NULL){
				Result++;
				Node = Node->GetPrev();
			}
			return Result;
		}
	}
}

BOOL CTreeNode::GetExpanded(){
	return GetState(nsExpanded);
}

INT CTreeNode::GetLevel(){
	INT Result = 0;
	CTreeNode* Node = GetParent();
	while(Node != NULL){
		Result++;
		Node = Node->GetParent();
	}
	return Result;
}

CTreeNode* CTreeNode::GetParent(){
	CTreeNode* Result = NULL;
	if(GetHandle() != 0 && ItemId != 0)
		Result = Owner->GetNode(TreeView_GetParent(GetHandle(), ItemId));
	return Result;
}

BOOL CTreeNode::GetChildren(){
	BOOL Result = FALSE;
	if(!GetDeleting()){
		TTVItem Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_CHILDREN;
		Item.hItem = ItemId;
		if(TreeView_GetItem(GetHandle(), &Item))
			Result = Item.cChildren > 0;
	}
	return Result;
}

BOOL CTreeNode::GetCut(){
	return GetState(nsCut);
}

BOOL CTreeNode::GetDropTarget(){
	return GetState(nsDropHilited);
}

BOOL CTreeNode::GetFocused(){
	return GetState(nsFocused);
}

INT CTreeNode::GetIndex(){
	INT Result = -1;
	CTreeNode* Node = this;
	while(Node != NULL){
		Result++;
		Node = Node->GetPrevSibling();
	}
	return Result;
}

CTreeNode* CTreeNode::GetItem(INT Index){
	CTreeNode* Result = GetFirstChild();
	while(Result != NULL && Index > 0){
		Result = GetNextChild(Result);
		Index++;
	}
	if(Result == NULL)
		throw "tree view error. list index error.";//TreeViewError(Format(SListIndexError, [Index]));
	return Result;
}

BOOL CTreeNode::GetSelected(){
	return GetState(nsSelected);
}

INT CTreeNode::GetCount(){
	INT Result = 0;
	CTreeNode* Node = GetFirstChild();
	while(Node != NULL){
		Result++;
		Node = Node->GetNextChild(Node);
	}
	return Result;
}

CTreeView* CTreeNode::GetTreeView(){
	return Owner->GetOwner();
}

void CTreeNode::InternalMove(CTreeNode* ParentNode, CTreeNode* Node, HTREEITEM HItem, TAddMode AddMode){
	Owner->ClearCache();
	HTREEITEM NodeId = 0;
	if(AddMode == taInsert && Node != NULL)
		NodeId = Node->ItemId;
	BOOL Children = GetChildren();
	BOOL IsSelected = GetSelected();
	if(GetParent() != NULL && GetParent()->CompareCount(1)){
		GetParent()->SetExpanded(FALSE);
		GetParent()->SetChildren(FALSE);
	}
	TTVItem TreeViewItem;
	ZeroMemory(&TreeViewItem, sizeof(TreeViewItem));
	TreeViewItem.mask = TVIF_PARAM;
	TreeViewItem.hItem = ItemId;
    TreeViewItem.lParam = 0;
	TreeView_SetItem(GetHandle(), &TreeViewItem);
	HItem = Owner->AddItem(HItem, NodeId, Owner->CreateItem(this), AddMode);
	if(HItem == NULL)
		throw "out of resource.";//EOutOfResources.Create(sInsertError);
	for(INT I = GetCount() - 1; I >= 0; I--)
		GetItem(I)->InternalMove(this, NULL, HItem, taAddFirst);
	TreeView_DeleteItem(GetHandle(), ItemId);
	ItemId = HItem;
	Assign(this);
	SetChildren(Children);
	SetSelected(IsSelected);
}

BOOL CTreeNode::IsEqual(CTreeNode* Node){
	return (*Text == *(Node->Text) && Data == Node->Data);
}

BOOL CTreeNode::IsNodeVisible(){
	TRect Rect = {0, 0, 0, 0};
	return !GetDeleting() && TreeView_GetItemRect(GetHandle(), ItemId, &Rect, TRUE);
}

void CTreeNode::ReadData(CStream* Stream, PTreeNodeInfo Info){
	INT Size = 0;
	Owner->ClearCache();
	Stream->ReadBuffer(&Size, sizeof(Size));
	Stream->ReadBuffer(Info, Size);
	*Text = String(Info->Text);
	SetImageIndex(Info->ImageIndex);
	SetSelectedIndex(Info->SelectedIndex);
	SetStateIndex(Info->StateIndex);
	SetOverlayIndex(Info->OverlayIndex);
	SetData(Info->Data);
	INT ItemCount = Info->Count;
	CTreeNode* LNode = NULL;
	String s(TEXT(""));
	for(INT I = 0; I < ItemCount; I++){
		LNode = Owner->AddChild(this, s);
		LNode->ReadData(Stream, Info);
		Owner->GetOwner()->Added(LNode);
	}
}

void CTreeNode::SetChildren(BOOL Value){
	if(!GetDeleting()){
		TTVItem Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_CHILDREN;
		Item.hItem = ItemId;
		Item.cChildren = Value;
		TreeView_SetItem(GetHandle(), &Item);
	}
}

void CTreeNode::SetCut(BOOL Value){
	if(Value != GetCut())
		SetState(nsCut, Value);
}

void CTreeNode::SetData(LPVOID Value){
	if(!GetDeleting() && Value != Data){
		Data = Value;
		TSortType SortType = GetTreeView()->GetSortType();
		if((SortType == stData && SortType == stBoth) && GetTreeView()->OnCompare != NULL &&
			!GetDeleting() && InTree){
			if(GetParent() != NULL)
				GetParent()->AlphaSort();
			else
				GetTreeView()->AlphaSort(FALSE);
		}
	}
}

void CTreeNode::SetDropTarget(BOOL Value){
	if(GetHandle() != 0 && ItemId != 0)
    if(Value)
		TreeView_SelectDropTarget(GetHandle(), ItemId);
	else
		if(GetDropTarget())
			TreeView_SelectDropTarget(GetHandle(), 0);
}

void CTreeNode::SetItem(INT Index, CTreeNode* Value){
	GetItem(Index)->Assign(Value);
}

void CTreeNode::SetExpanded(BOOL Value){
	if(Value != GetExpanded()){
		if(Value)
			Expand(FALSE);
		else
			Collapse(FALSE);
	}
}

void CTreeNode::SetFocused(BOOL Value){
	if(Value != GetFocused())
		SetState(nsFocused, Value);
}

void CTreeNode::SetImageIndex(TImageIndex Value){
	if(!GetDeleting() && Value != ImageIndex){
		ImageIndex = Value;
		TTVItem Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_IMAGE | TVIF_HANDLE;
		Item.hItem = ItemId;
		if(Owner->GetOwner()->OnGetImageIndex != NULL)
			Item.iImage = I_IMAGECALLBACK;
		else
			Item.iImage = ImageIndex;
		TreeView_SetItem(GetHandle(), &Item);
	}
}

void CTreeNode::SetOverlayIndex(INT Value){
	if(!GetDeleting() && Value != GetOverlayIndex()){
		OverlayIndex = Value;
		TTVItem Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_STATE | TVIF_HANDLE;
		Item.stateMask = TVIS_OVERLAYMASK;
		Item.hItem = ItemId;
		Item.state = IndexToOverlayMask(OverlayIndex + 1);
		TreeView_SetItem(GetHandle(), &Item);
	}
}

void CTreeNode::SetSelectedIndex(INT Value){
	if(!GetDeleting() && Value != GetSelectedIndex()){
		SelectedIndex = Value;
		TTVItem Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_SELECTEDIMAGE | TVIF_HANDLE;
		Item.hItem = ItemId;
		if(Owner->GetOwner()->OnGetSelectedIndex != NULL)
			Item.iSelectedImage = I_IMAGECALLBACK;
		else
			Item.iSelectedImage = SelectedIndex;
		TreeView_SetItem(GetHandle(), &Item);
	}
}

void CTreeNode::SetSelected(BOOL Value){
	if(!GetDeleting() && GetHandle() != 0 && ItemId != NULL)
    if(Value != GetSelected()){
		if(Value)
			TreeView_SelectItem(GetHandle(), ItemId);
		else
			if(GetSelected())
				TreeView_SelectItem(GetHandle(), NULL);
	}
	else if(GetTreeView()->GetMultiSelect() && GetTreeView()->Selections->GetCount() > 1)
		GetTreeView()->Select(this, 0);
}

void CTreeNode::SetStateIndex(INT Value){
	if(!GetDeleting() && Value != GetStateIndex()){
		StateIndex = Value;
		if(Value >= 0)
			Value--;
		TTVItem Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_STATE | TVIF_HANDLE;
		Item.stateMask = TVIS_STATEIMAGEMASK;
		Item.hItem = ItemId;
		Item.state = (UINT)IndexToStateImageMask(Value + 1);
		TreeView_SetItem(GetHandle(), &Item);
	}
}

void CTreeNode::SetText(String& S){
	if(!GetDeleting() && S != *Text){
		*Text = S;
		TTVItem Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_TEXT;
		Item.hItem = ItemId;
		Item.pszText = LPSTR_TEXTCALLBACK;
		TreeView_SetItem(GetHandle(), &Item);
		TSortType SortType = GetTreeView()->GetSortType();
		if((SortType == stText || SortType == stBoth) && InTree){
			if(GetParent() != NULL)
				GetParent()->AlphaSort();
			else
				GetTreeView()->AlphaSort(FALSE);
		}
	}
}

void CTreeNode::WriteData(CStream* Stream, PTreeNodeInfo Info){
	INT L = Text->Length();
	if(L > 255)
		L = 255;
	INT Size = sizeof(TTreeNodeInfo) + L - 255;
	lstrcpyn(Info->Text, Text->GetBuffer(), 255);
	Info->ImageIndex = GetImageIndex();
	Info->SelectedIndex = GetSelectedIndex();
	Info->OverlayIndex = GetOverlayIndex();
	Info->StateIndex = GetStateIndex();
	Info->Data = GetData();
	INT ItemCount = GetCount();
	Info->Count = ItemCount;
	Stream->WriteBuffer(&Size, sizeof(Size));
	Stream->WriteBuffer(Info, Size);
	for(INT I = 0; I < ItemCount; I++)
		GetItem(I)->WriteData(Stream, Info);
}

BOOL CTreeNode::GetState(TNodeState NodeState){
	if(!GetDeleting()){
		TTVItem Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_STATE;
		Item.hItem = ItemId;
		if(TreeView_GetItem(GetHandle(), &Item)){
			switch(NodeState){
			case nsCut:
				return (Item.state & TVIS_CUT) != 0;
			case nsFocused:
				return (Item.state & TVIS_FOCUSED) != 0;
			case nsSelected:
				return (Item.state & TVIS_SELECTED) != 0;
			case nsExpanded:
				return (Item.state & TVIS_EXPANDED) != 0;
			case nsDropHilited:
				return (Item.state & TVIS_DROPHILITED) != 0;
			}
		}
	}
	return FALSE;
}

void CTreeNode::SetState(TNodeState NodeState, BOOL Value){
	if(!GetDeleting()){
		TTVItem Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_STATE;
		Item.hItem = ItemId;
		switch(NodeState){
		case nsCut:
			Item.stateMask = TVIS_CUT;
			break;
		case nsFocused:
			Item.stateMask = TVIS_FOCUSED;
			break;
		case nsSelected:
			Item.stateMask = TVIS_SELECTED;
			break;
		case nsExpanded:
			Item.stateMask = TVIS_EXPANDED;
			break;
		case nsDropHilited:
			Item.stateMask = TVIS_DROPHILITED;
			break;
		}
		if(Value)
			Item.state = Item.stateMask & DWORD(-1);
		else
			Item.state = DWORD(0);
		TreeView_SetItem(GetHandle(), &Item);
	}
}

void CTreeNode::SetSelectedBit(BOOL Value){
	SetState(nsSelected, Value);
}

BOOL CTreeNode::AlphaSort(BOOL ARecurse){
	return CustomSort(NULL, 0, ARecurse);
}

void CTreeNode::Assign(CObject* Source){
	Owner->ClearCache();
	if(!GetDeleting() && Source->InstanceOf(CTreeNode::_Class)){
		CTreeNode* Node = (CTreeNode *)Source;
		SetText(Node->GetText());
		SetData(Node->GetData());
		SetImageIndex(Node->GetImageIndex());
		SetSelectedIndex(Node->GetSelectedIndex());
		SetStateIndex(Node->GetStateIndex());
		SetOverlayIndex(Node->GetOverlayIndex());
		SetFocused(Node->GetFocused());
		SetDropTarget(Node->GetDropTarget());
		SetCut(Node->GetCut());
		SetChildren(Node->GetChildren());
	}
	else
		__super::Assign(Source);

}

void CTreeNode::Collapse(BOOL Recurse){
	ExpandItem(FALSE, Recurse);
}

INT CALLBACK DefaultTreeViewSort(CTreeNode* Node1, CTreeNode* Node2, INT lParam){
	INT Result = 0;
	if(Node1->GetTreeView()->GetOnCompare() != NULL)
		CALL_EVENT_EXTERNAL(Node1->GetTreeView(), Compare)(Node1->GetTreeView(), Node1, Node2, lParam, Result);
	else
		Result = lstrcmp(Node1->GetText().GetBuffer(), Node2->GetText().GetBuffer());
	return Result;
}

BOOL CTreeNode::CustomSort(TTVCompare SortProc, LONG_PTR Data, BOOL ARecurse){
	BOOL Result = FALSE;
	if(!GetDeleting()){
		Owner->ClearCache();
		TTVSortCB SortCB;
		ZeroMemory(&SortCB, sizeof(SortCB));
		if(SortProc == NULL)
			SortCB.lpfnCompare = (TTVCompare)&DefaultTreeViewSort;
		else 
			SortCB.lpfnCompare = SortProc;
		SortCB.hParent = ItemId;
		SortCB.lParam = Data;
		Result = TreeView_SortChildrenCB(GetHandle(), &SortCB, 0);
		if(ARecurse){
			CTreeNode* LNode = GetFirstChild();
			while(LNode != NULL){
				if(LNode->GetChildren())
					LNode->CustomSort(SortProc, Data, TRUE);
				LNode = LNode->GetNextSibling();
			}
		}
	}
	return Result;
}

void CTreeNode::Delete(){
	if(!GetDeleting())
		delete this;
}

void CTreeNode::DeleteChildren(){
	GetOwner()->ClearCache();
	if(!GetDeleting())
		TreeView_Expand(GetTreeView()->GetHandle(), ItemId, TVE_COLLAPSE | TVE_COLLAPSERESET);
	SetChildren(FALSE);
}

TRect CTreeNode::DisplayRect(BOOL TextOnly){
	TRect Result = {0, 0, 0, 0};
	if(!GetDeleting())
		TreeView_GetItemRect(GetHandle(), ItemId, &Result, TextOnly);
	return Result;
}

BOOL CTreeNode::EditText(){
	return GetHandle() != 0 && ItemId != 0 && TreeView_EditLabel(GetHandle(), ItemId) != 0;
}

void CTreeNode::EndEdit(BOOL Cancel){
	if(!GetDeleting())
		TreeView_EndEditLabelNow(GetHandle(), Cancel);
}

void CTreeNode::Expand(BOOL Recurse){
	ExpandItem(TRUE, Recurse);
}

CTreeNode* CTreeNode::GetFirstChild(){ //GetFirstChild conflicts with C++ macro
	CTreeNode* Result = NULL;
	if(GetHandle() != 0 && ItemId != NULL)
		Result = Owner->GetNode(TreeView_GetChild(GetHandle(), ItemId));
	return Result;
}

HWND CTreeNode::GetHandle(){
	return GetTreeView()->GetHandle();
}

CTreeNode* CTreeNode::GetLastChild(){
	CTreeNode* Result = GetFirstChild();
	if(Result != NULL){
		CTreeNode* Node = Result;
		do{
			Result = Node;
			Node = Result->GetNextSibling();
		}while(Node != NULL);
	}
	return Result;
}

CTreeNode* CTreeNode::GetNext(){
	CTreeNode* Result = NULL;
	if(GetHandle() != 0 && ItemId != 0){
		HTREEITEM NodeID = TreeView_GetChild(GetHandle(), ItemId);
		if(NodeID == 0)
			NodeID = TreeView_GetNextSibling(GetHandle(), ItemId);
		HTREEITEM ParentID = ItemId;
		while(NodeID == 0 && ParentID != 0){
			ParentID = TreeView_GetParent(GetHandle(), ParentID);
			NodeID = TreeView_GetNextSibling(GetHandle(), ParentID);
		}
		Result = Owner->GetNode(NodeID);
	}
	return Result;
}

CTreeNode* CTreeNode::GetNextChild(CTreeNode* Value){
	if(Value != NULL)
		return Value->GetNextSibling();
	else 
		return NULL;
}

CTreeNode* CTreeNode::GetNextSibling(){ //GetNextSibling conflicts with C++ macro
	CTreeNode* Result = NULL;
	if(GetHandle() != 0 && ItemId != 0)
		Result = Owner->GetNode(TreeView_GetNextSibling(GetHandle(), ItemId));
	return Result;
}

CTreeNode* CTreeNode::GetNextVisible(){
	CTreeNode* Result = NULL;
	if(GetHandle() != 0 && ItemId != NULL && IsNodeVisible())
		Result = Owner->GetNode(TreeView_GetNextVisible(GetHandle(), ItemId));
	return Result;
}

CTreeNode* CTreeNode::GetPrev(){
	CTreeNode* Result = GetPrevSibling();
	if(Result != NULL){
		CTreeNode* Node = Result;
		do{
			Result = Node;
			Node = Result->GetLastChild();
		}while(Node != NULL);
	}
	else
		Result = GetParent();
	return Result;
}

CTreeNode* CTreeNode::GetPrevChild(CTreeNode* Value){
	if(Value != NULL)
		return Value->GetPrevSibling();
	else
		return NULL;
}

CTreeNode* CTreeNode::GetPrevSibling(){ //GetPrevSibling conflicts with a C++ macro
	CTreeNode* Result = NULL;
	if(GetHandle() != 0 && ItemId != 0)
		Result = Owner->GetNode(TreeView_GetPrevSibling(GetHandle(), ItemId));
	return Result;
}

CTreeNode* CTreeNode::GetPrevVisible(){
	CTreeNode* Result = NULL;
	if(GetHandle() != 0 && ItemId != 0 && IsNodeVisible())
		Result = Owner->GetNode(TreeView_GetPrevVisible(GetHandle(), ItemId));
	return Result;
}

BOOL CTreeNode::HasAsParent(CTreeNode* Value){
	BOOL Result = FALSE;
	if(Value != NULL){
		if(GetParent() == NULL)
			Result = FALSE;
		else 
			if(GetParent() == Value)
				Result = TRUE;
			else 
				Result = GetParent()->HasAsParent(Value);
	}
	else 
		Result = TRUE;
	return Result;
}

INT CTreeNode::IndexOf(CTreeNode* Value){
	INT Result = -1;
	CTreeNode* Node = GetFirstChild();
	while(Node != NULL){
		Result++;
		if(Node == Value)
			break;
		Node = GetNextChild(Node);
	}
	if(Node == NULL)
		Result = -1;
	return Result;
}

void CTreeNode::MakeVisible(){
	if(GetHandle() != 0 && ItemId != 0)
		TreeView_EnsureVisible(GetHandle(), ItemId);
}

void CTreeNode::MoveTo(CTreeNode* Destination, TNodeAttachMode Mode){
	if(!GetDeleting()){
		CTreeView* TreeView = GetTreeView();
		CObject* OldChangingObj = TreeView->GetOnChangingObj();
		TTVChangingEvent OldOnChanging = TreeView->GetOnChanging();
		CObject* OldChangeObj = TreeView->GetOnChangeObj();
		TTVChangedEvent OldOnChange = TreeView->GetOnChange();
		TreeView->SetOnChanging(NULL, NULL);
		TreeView->SetOnChange(NULL, NULL);
		__try{
			if(Destination == NULL || !Destination->HasAsParent(this)){
				TAddMode AddMode = taAdd;
				CTreeNode* Node = NULL;
				if(Destination != NULL && !(Mode == naAddChild || Mode == naAddChildFirst))
					Node = Destination->GetParent();
				else
					Node = Destination;
				if(Mode == naAdd || Mode == naAddChild)
					AddMode = taAdd;
				else if(Mode == naAddFirst || Mode == naAddChildFirst)
					AddMode = taAddFirst;
				else if(Mode == naInsert){
					Destination = Destination->GetPrevSibling();
					if(Destination == NULL)
						AddMode = taAddFirst;
					else 
						AddMode = taInsert;
				}
				HTREEITEM HItem = 0;
				if(Node != NULL)
					HItem = Node->GetItemId();
				if(Destination != this)
					InternalMove(Node, Destination, HItem, AddMode);
				Node = GetParent();
				if(Node != NULL){
					Node->SetChildren(TRUE);
					Node->SetExpanded(TRUE);
				}
			}
		}
		__finally{
			TreeView->SetOnChanging(OldChangingObj, OldOnChanging);
			TreeView->SetOnChange(OldChangeObj, OldOnChange);
		}
	}
}

BOOL CTreeNode::IsFirstNode(){
	return !GetDeleting() && (GetParent() == NULL) && (GetPrevSibling() == NULL);
}

String CTreeNode::GetText(){
	return *Text;
}


IMPL_DYN_CLASS(CTreeStrings)
CTreeStrings::CTreeStrings(CTreeNodes* AOwner):Owner(AOwner){
}
CTreeStrings::~CTreeStrings(){
}

String CTreeStrings::Get(INT Index){
	TCHAR TabChar = TCHAR(9);
	String Result(TEXT(""));
	CTreeNode* Node = Owner->GetNodeFromIndex(Index);
	INT Level = Node->GetLevel();
	for(INT I = 0; I < Level; I++)
		Result = Result + TabChar;
	return Result + Node->GetText();
}

LPTSTR CTreeStrings::GetBufStart(LPTSTR Buffer, INT& Level){
	Level = 0;
	while(*Buffer == TCHAR(' ') || *Buffer == TCHAR(9)){
		Buffer++;
		Level++;
	}
	return Buffer;
}

CObject* CTreeStrings::GetObject(INT Index){
	return (CObject *)Owner->GetNodeFromIndex(Index)->GetData();
}

void CTreeStrings::PutObject(INT Index,CObject* AObject){
	Owner->GetNodeFromIndex(Index)->SetData(AObject);
}

INT CTreeStrings::GetCount(){
	return Owner->GetCount();
}

void CTreeStrings::Clear(){
	Owner->Clear();
}

void CTreeStrings::Delete(INT Index){
	Owner->GetNodeFromIndex(Index)->Delete();
}

void CTreeStrings::SetUpdateState(BOOL Updating){
	SendMessage(Owner->GetHandle(), WM_SETREDRAW, (WPARAM)!Updating, 0);
	if(!Updating)
		Owner->GetOwner()->Refresh();
}

INT CTreeStrings::Add(String& S){
	INT Result = GetCount();
	if(S.Length() == 1 && *(S.GetBuffer()) == TCHAR(0x1A))
		return Result;
	CTreeNode* Node = NULL;
	INT OldLevel = 0;
	INT Level = 0;
	LPTSTR NewStr = GetBufStart(S.GetBuffer(), Level);
	if(Result > 0){
		Node = Owner->GetNodeFromIndex(Result - 1);
		OldLevel = Node->GetLevel();
	}
	if(Level > OldLevel || Node == NULL){
		if(Level - OldLevel > 1)
			throw "treeview error invalid level";//TreeViewError(sInvalidLevel);
	}
	else{
		for(INT I = OldLevel; I >= Level; I--){
			Node = Node->GetParent();
			if(Node == NULL && I - Level > 0)
				throw "treeview error invalid level";//TreeViewError(sInvalidLevel);
		}
	}
	Owner->AddChild(Node, String(NewStr));
	return Result;
}

void CTreeStrings::Insert(INT Index, String& S){
	Owner->Insert(Owner->GetNodeFromIndex(Index), S);
}

void CTreeStrings::LoadTreeFromStream(CStream* Stream){
	CMethodLock exKeeper(Owner->GetOwner(), (TLockMethod)NULL, (TLockMethod)&CTreeView::Invalidate);
	{
		//for object free scope 
		CStringList List;
		CMethodLock lock(Owner, (TLockMethod)&CTreeNodes::BeginUpdate, (TLockMethod)&CTreeNodes::EndUpdate);
		Clear();
		List.LoadFromStream(Stream);
		CTreeNode* ANode = NULL;
		CTreeNode* NextNode = NULL;
		INT ALevel = 0;
		for(INT i = 0; i < List.GetCount(); i++){
			LPTSTR CurrStr = GetBufStart(List.Get(i).GetBuffer(), ALevel);
			if(ANode == NULL)
				ANode = Owner->AddChild(NULL, String(CurrStr));
			else if(ANode->GetLevel() == ALevel)
				ANode = Owner->AddChild(ANode->GetParent(), String(CurrStr));
			else if(ANode->GetLevel() == (ALevel - 1))
				ANode = Owner->AddChild(ANode, String(CurrStr));
			else if(ANode->GetLevel() > ALevel){
				NextNode = ANode->GetParent();
				while(NextNode->GetLevel() > ALevel)
					NextNode = NextNode->GetParent();
				ANode = Owner->AddChild(NextNode->GetParent(), String(CurrStr));
			}
			else throw "treeview error format, invalid level.";//TreeViewErrorFmt(sInvalidLevelEx, [ALevel, CurrStr]);
		}
	}
	exKeeper.SwapUnLock(NULL);//did not repaint while no exception
}

void CTreeStrings::SaveTreeToStream(CStream* Stream){
	TCHAR TabChar = TCHAR(9);
	LPTSTR EndOfLine = TEXT("\r\n");
	if(GetCount() > 0){
		String NodeStr;
		CTreeNode* ANode = Owner->GetNodeFromIndex(0);
		while(ANode != NULL){
			NodeStr = String(TEXT(""));
			for(INT i = 0; i < ANode->GetLevel(); i++)
				NodeStr = NodeStr + TabChar;
			NodeStr = NodeStr + ANode->GetText() + EndOfLine;
			Stream->Write(NodeStr.GetBuffer(), NodeStr.Length() * sizeof(TCHAR));
			ANode = ANode->GetNext();
		}
	}
}

IMPL_DYN_CLASS(CTreeNodes)
CTreeNodes::CTreeNodes(CTreeView* AOwner):Owner(AOwner),
	UpdateCount(0),
	Reading(0){
	NodeCache.CacheNode = NULL;
	NodeCache.CacheIndex = 0;
}

CTreeNodes::~CTreeNodes(){
	Clear();
}

void CTreeNodes::AddedNode(CTreeNode* Value){
	if(Value != NULL){
		Value->SetChildren(TRUE);
		Repaint(Value);
	}
}

HWND CTreeNodes::GetHandle(){
	return GetOwner()->GetHandle();
}

CTreeNode* CTreeNodes::GetNodeFromIndex(INT Index){
	CTreeNode* Result = NULL;
	if(Index < 0)
		throw "tree view error invalid index.";//TreeViewError(sInvalidIndex);
	if(NodeCache.CacheNode != NULL && ::abs(NodeCache.CacheIndex - Index) <= 1){
		if(Index == NodeCache.CacheIndex)
			Result = NodeCache.CacheNode;
		else if(Index < NodeCache.CacheIndex)
			Result = NodeCache.CacheNode->GetPrev();
		else
			Result = NodeCache.CacheNode->GetNext();
	}
	else{
		Result = GetFirstNode();
		INT I = Index;
		while(I != 0 && Result != NULL){
			Result = Result->GetNext();
			I--;
		}
	}
	if(Result == NULL)
		throw "tree view error invalid index.";//TreeViewError(sInvalidIndex);
	NodeCache.CacheNode = Result;
	NodeCache.CacheIndex = Index;
	return Result;
}

void CTreeNodes::ReadData(CStream* Stream){
	BOOL LHandleAllocated = GetOwner()->HandleAllocated();
	if(LHandleAllocated)
		BeginUpdate();
	Reading = TRUE;
	__try{
		ReadDataImpl(Stream);
	}
	__finally{
		Reading = FALSE;
		if(LHandleAllocated)
			EndUpdate();
	}
}
void CTreeNodes::ReadDataImpl(CStream* Stream){
	INT Count = 0;
	Clear();
	Stream->ReadBuffer(&Count, sizeof(Count));
	TTreeNodeInfo NodeInfo;
	ZeroMemory(&NodeInfo, sizeof(NodeInfo));
	CTreeNode* LNode = NULL;
	for(INT I = 0; I < Count; I++){
		LNode = Add(NULL, String(TEXT("")));
		LNode->ReadData(Stream, &NodeInfo);
		GetOwner()->Added(LNode);
	}
}

void CTreeNodes::Repaint(CTreeNode* Node){
	if(UpdateCount < 1){
		while(Node != NULL && !Node->IsNodeVisible())
			Node = Node->GetParent();
		if(Node != NULL){
			TRect R = Node->DisplayRect(FALSE);
			InvalidateRect(GetOwner()->GetHandle(), (const RECT *)&R, TRUE);
		}
	}
}

void CTreeNodes::WriteData(CStream* Stream){
	INT I = 0;
	CTreeNode* Node = GetFirstNode();
	while(Node != NULL){
		I++;
		Node = Node->GetNextSibling();
	}
	Stream->WriteBuffer(&I, sizeof(I));
	TTreeNodeInfo NodeInfo;
	ZeroMemory(&NodeInfo, sizeof(NodeInfo));
	Node = GetFirstNode();
	while(Node != NULL){
		Node->WriteData(Stream, &NodeInfo);
		Node = Node->GetNextSibling();
	}
}

void CTreeNodes::ClearCache(){
	NodeCache.CacheNode = NULL;
}

void CTreeNodes::WriteExpandedState(CStream* Stream){
	INT Size = sizeof(BOOL) * GetCount();
	Stream->WriteBuffer(&Size, sizeof(Size));
	CTreeNode* Node = GetFirstNode();
	while(Node != NULL){
		BOOL NodeExpanded = Node->GetExpanded();
		Stream->WriteBuffer(&NodeExpanded, sizeof(BOOL));
		Node = Node->GetNext();
	}
}

void CTreeNodes::ReadExpandedState(CStream* Stream){
	INT ItemCount = 0;
	if(Stream->GetPosition() < Stream->GetSize())
		Stream->ReadBuffer(&ItemCount, sizeof(ItemCount));
	else return ;
	INT Index = 0;
	CTreeNode* Node = GetFirstNode();
	BOOL NodeExpanded = FALSE;
	while(Index < ItemCount && Node != NULL){
		Stream->ReadBuffer(&NodeExpanded, sizeof(NodeExpanded));
		Node->SetExpanded(NodeExpanded);
		Index++;
		Node = Node->GetNext();
	}
}

HTREEITEM CTreeNodes::AddItem(HTREEITEM Parent, HTREEITEM Target, TTVItem Item, TAddMode AddMode){
	ClearCache();
	TTVInsertStruct InsertStruct;
	ZeroMemory(&InsertStruct, sizeof(InsertStruct));
	InsertStruct.hParent = Parent;
	if(AddMode == taAddFirst)
		InsertStruct.hInsertAfter = TVI_FIRST;
	else if(AddMode == taAdd)
		InsertStruct.hInsertAfter = TVI_LAST;
	else if(AddMode == taInsert)
		InsertStruct.hInsertAfter = Target;
	InsertStruct.item = Item;
	Owner->ChangeTimer->SetEnabled(FALSE);
	return TreeView_InsertItem(GetHandle(), &InsertStruct);
}

//TODO void DefineProperties(TFiler Filer) override;
TTVItem CTreeNodes::CreateItem(CTreeNode* Node){
	TTVItem Result;
	ZeroMemory(&Result, sizeof(Result));
	Node->InTree = TRUE;
	Result.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	Result.lParam = (LPARAM)Node;
	Result.pszText = LPSTR_TEXTCALLBACK;
	Result.iImage = I_IMAGECALLBACK;
	Result.iSelectedImage = I_IMAGECALLBACK;
	return Result;
}

INT CTreeNodes::GetCount(){
	if(GetOwner()->HandleAllocated())
		return TreeView_GetCount(GetHandle());
	else
		return 0;
}

void CTreeNodes::SetItem(INT Index, CTreeNode* Value){
	GetNodeFromIndex(Index)->Assign(Value);
}

void CTreeNodes::SetUpdateState(BOOL Updating){
	SendMessage(GetHandle(), WM_SETREDRAW, (WPARAM)(!Updating), 0);
	if(Updating)
		GetOwner()->Refresh();
}

CTreeNode* CTreeNodes::AddChildFirst(CTreeNode* Parent, String& S){
	return AddNode(NULL, Parent, S, NULL, naAddChildFirst);
}

CTreeNode* CTreeNodes::AddChild(CTreeNode* Parent, String& S){
	return AddNode(NULL, Parent, S, NULL, naAddChild);;
}

CTreeNode* CTreeNodes::AddChildObjectFirst(CTreeNode* Parent, String& S, LPVOID Ptr){
	return AddNode(NULL, Parent, S, Ptr, naAddChildFirst);
}

CTreeNode* CTreeNodes::AddChildObject(CTreeNode* Parent, String& S, LPVOID Ptr){
	return AddNode(NULL, Parent, S, Ptr, naAddChild);;
}

CTreeNode* CTreeNodes::AddFirst(CTreeNode* Sibling, String& S){
	return AddNode(NULL, Sibling, S, NULL, naAddFirst);
}

CTreeNode* CTreeNodes::Add(CTreeNode* Sibling, String& S){
	return AddNode(NULL, Sibling, S, NULL, naAdd);
}

CTreeNode* CTreeNodes::AddObjectFirst(CTreeNode* Sibling, String& S, LPVOID Ptr){
	return AddNode(NULL, Sibling, S, Ptr, naAddFirst);
}

CTreeNode* CTreeNodes::AddObject(CTreeNode* Sibling, String& S, LPVOID Ptr){
	return AddNode(NULL, Sibling, S, Ptr, naAdd);
}

CTreeNode* CTreeNodes::AddNode(CTreeNode* Node, CTreeNode* Relative, String& S,
	LPVOID Ptr, TNodeAttachMode Method){
	CTreeNode* Result = NULL;
	TAddMode cAddMode[] = {taAdd, taAddFirst, taAdd, taAddFirst, taInsert};
	if(Node == NULL)
		Result = GetOwner()->CreateNode();
	else
		Result = Node;
	CObjectHolder ResultHolder(Result);
	HTREEITEM Item = 0;
	HTREEITEM ItemId = 0;
	CTreeNode* Parent = NULL;
	TAddMode AddMode = cAddMode[Method];
	if(Relative != NULL){
		if(Method == naAdd || Method == naAddFirst){
			Parent = Relative->GetParent();
			if(Parent != NULL)
				Item = Parent->GetItemId();
		}
		else if(Method == naAddChild || Method == naAddChildFirst){
			Parent = Relative;
			Item = Parent->GetItemId();
		}
		else if(Method == naInsert){
			Parent = Relative->GetParent();
			if(Parent != NULL)
				Item = Parent->GetItemId();
			Relative = Relative->GetPrevSibling();
			if(Relative != NULL)
				ItemId = Relative->GetItemId();
			else
				AddMode = taAddFirst;
		}
	}
	Result->SetData(Ptr);
	Result->SetText(S);
	Item = AddItem(Item, ItemId, CreateItem(Result), AddMode);
	if(Item == NULL)
		throw "out of resource, insert error.";//raise EOutOfResources.Create(sInsertError);
	Result->ItemId = Item;
	if(UpdateCount == 0 && Result->IsFirstNode())
		SendMessage(GetHandle(), WM_SETREDRAW, 1, 0);
	AddedNode(Parent);
	if(!Reading)
		Owner->Added(Result);
	ResultHolder.SwapObject(NULL);
	return Result;
}

BOOL CTreeNodes::AlphaSort(BOOL ARecurse){
	return Owner->AlphaSort(ARecurse);
}

void CTreeNodes::Assign(CObject* Source){
	ClearCache();
	if(Source->InstanceOf(CTreeNodes::_Class)){
		CTreeNodes* TreeNodes = (CTreeNodes *)Source;
		Clear();
		CMemoryStream* MemStream = new CMemoryStream();
		CObjectHolder MemSHolder(MemStream);
		TreeNodes->WriteData(MemStream);
		MemStream->SetPosition(0);
		ReadData(MemStream);

	}
	else __super::Assign(Source);
}

void CTreeNodes::BeginUpdate(){
	if(UpdateCount == 0)
		SetUpdateState(TRUE);
	UpdateCount++;
}

void CTreeNodes::Clear(){
	NodeCache.CacheNode = NULL;
}

BOOL CTreeNodes::CustomSort(TTVCompare SortProc, LONG_PTR Data, BOOL ARecurse){
	return Owner->CustomSort(SortProc, Data, ARecurse);
}

void CTreeNodes::Delete(CTreeNode* Node){
	Node->Delete();
}

void CTreeNodes::EndUpdate(){
	UpdateCount--;
	if(UpdateCount == 0)
		SetUpdateState(FALSE);
}

CTreeNode* CTreeNodes::GetFirstNode(){
	return GetNode(TreeView_GetRoot(GetHandle()));
}

CTreeNode* CTreeNodes::GetNode(HTREEITEM ItemId){
	TTVItem Item;
	ZeroMemory(&Item, sizeof(Item));
	Item.hItem = ItemId;
	Item.mask = TVIF_PARAM;
	if(TreeView_GetItem(GetHandle(), &Item))
		return (CTreeNode *)Item.lParam;
	else
		return NULL;
}

CTreeNode* CTreeNodes::Insert(CTreeNode* Sibling, String& S){
	return AddNode(NULL, Sibling, S, NULL, naInsert);
}

CTreeNode* CTreeNodes::InsertObject(CTreeNode* Sibling, String& S, LPVOID Ptr){
	return AddNode(NULL, Sibling, S, Ptr, naInsert);;
}

CTreeNode* CTreeNodes::InsertNode(CTreeNode* Node, CTreeNode* Sibling, String& S, 
	LPVOID Ptr){
	return AddNode(Node, Sibling, S, Ptr, naInsert);
}

IMPL_DYN_CLASS(CTreeView)
CTreeView::CTreeView(CComponent* AOwner) : CWinControl(AOwner),
	BorderStyle(bsSingle),
	ShowButtons(TRUE),
	ShowRoot(TRUE),
	ShowLines(TRUE),
	HideSelection(TRUE),
	SaveIndent(-1),
	MultiSelect(FALSE),
	MultiSelectStyle(msControlSelect),
	CreateWndRestores(TRUE),
	ToolTips(TRUE),
	AutoExpand(FALSE),
	CanvasChanged(FALSE),
	DefEditProc(NULL),
	Dragged(FALSE),
	DragNode(NULL),
	EditHandle(0),
	HotTrack(FALSE),
	Images(NULL),
	LastDropTarget(NULL),
	MemStream(NULL),
	RClickNode(NULL),
	RightClickSelect(FALSE),
	ManualNotify(FALSE),
	ReadOnly(FALSE),
	RowSelect(FALSE),
	SaveIndex(0),
	SaveItems(NULL),
	SaveTopIndex(0),
	SortType(stNone),
	StateChanging(FALSE),
	StateImages(NULL),
	WideText(NULL),
	SaveIndexes(NULL),
	ShiftAnchor(NULL),
	Selecting(FALSE),
	SelectChanged(FALSE),
	OurFont(0),
	StockFont(0),
	INIT_EVENT(AdvancedCustomDraw),
	INIT_EVENT(AdvancedCustomDrawItem),
	INIT_EVENT(CancelEdit),
	INIT_EVENT(Change),
	INIT_EVENT(Changing),
	INIT_EVENT(Collapsed),
	INIT_EVENT(Collapsing),
	INIT_EVENT(Compare),
	INIT_EVENT(CustomDraw),
	INIT_EVENT(CustomDrawItem),
	INIT_EVENT(Deletion),
	INIT_EVENT(Addition),
	INIT_EVENT(Editing),
	INIT_EVENT(Edited),
    INIT_EVENT(Expanded),
    INIT_EVENT(Expanding),
    INIT_EVENT(GetImageIndex),
    INIT_EVENT(GetSelectedIndex),
	INIT_EVENT(CreateNodeClass)
	{
	SetControlStyle(GetControlStyle() & (~csCaptureMouse) | csDisplayDragImage | csReflector);
	SetBounds(GetLeft(), GetTop(), 121, 97);
	SetTabStop(TRUE);
	SetParentColor(FALSE);
	Canvas = new CControlCanvas();
	((CControlCanvas *)Canvas)->SetControl(this);
	TreeNodes = CreateNodes();
	DragImage = new CDragImageList(32, 32);
	Selections = new CList();
	ChangeTimer = new CTimer(this);
	ChangeTimer->SetEnabled(FALSE);
	ChangeTimer->SetInterval(0);
	ChangeTimer->SetOnTimer(this, (TNotifyEvent)&CTreeView::OnChangeTimer);
	EditInstance = MakeObjectWndProc(this, (TObjectWndProc)&CTreeView::EditWndProc);
	ImageChangeLink = new CChangeLink();
	ImageChangeLink->SetOnChange(this, (TNotifyEvent)&CTreeView::ImageListChange);
	StateChangeLink = new CChangeLink();
	StateChangeLink->SetOnChange(this, (TNotifyEvent)&CTreeView::ImageListChange);
}

CTreeView::~CTreeView(){
	delete TreeNodes;
	TreeNodes = NULL;
	if(SaveIndexes == NULL)
		delete SaveIndexes;
	delete Selections;
	delete ChangeTimer;
	if(SaveItems != NULL)
		delete SaveItems;
	delete DragImage;
	if(MemStream != NULL)
		delete MemStream;
	FreeObjectWndProc(EditInstance);
	delete ImageChangeLink;
	delete StateChangeLink;
	delete Canvas;
	Canvas = NULL;
	if(WideText != NULL)
		delete WideText;
}

void CTreeView::MarkCanvasChanged(CObject* Sender){
	CanvasChanged = TRUE;
}

void CTreeView::CMColorChanged(TMessage& Message){
	INHERITED_MSG(Message);
	TreeView_SetBkColor(GetHandle(), ColorToRGB(GetColor()));
}

void CTreeView::CMCtl3DChanged(TMessage& Message){
	INHERITED_MSG(Message);
	if(BorderStyle == bsSingle)
		RecreateWnd();
}

void CTreeView::CMFontChanged(TMessage& Message){
	INHERITED_MSG(Message);
	TreeView_SetTextColor(GetHandle(), ColorToRGB(GetFont()->GetColor()));
}

void CTreeView::CMDrag(TCMDrag& Message){
	INHERITED_MSG(Message);
	/* TODO
	if(Message.DragMessage == dmDragMove){
		TPoint pt = ScreenToClient(Message.DragRec->Pos);
		DoDragOver(Source, pt.x, pt.y, Message.Result != 0);
	}
	else if(Message.DragMessage == dmDragLeave){
		((CDragObject *)Source)->HideDragImage();
		LastDropTarget = DropTarget;
		DropTarget = NULL;
		((CDragObject *)Source)->ShowDragImage();
	}
	else if(Message.DragMessage == dmDragDrop){
		LastDropTarget = NULL;
	}//*/
}

void CTreeView::CNNotify(TWMNotify& Message){
	switch(Message.NMHdr->code){
		case NM_CUSTOMDRAW:{
			if(Canvas != NULL){
				LPNMCUSTOMDRAW PCustomDraw = (LPNMCUSTOMDRAW)Message.NMHdr;
				CMethodLock Lock(Canvas, (TLockMethod)&CCanvas::Lock, (TLockMethod)&CCanvas::UnLock);
				Message.Result = CDRF_DODEFAULT;
				if((PCustomDraw->dwDrawStage & CDDS_ITEM) == 0){
					TRect R = GetClientRect();
					switch(PCustomDraw->dwDrawStage){
						case CDDS_PREPAINT:{
							if(IsCustomDrawn(dtControl, cdPrePaint)){
								BOOL DefaultDraw = FALSE;
								CanvasHandleCleaner Cleaner(Canvas);
								Canvas->SetHandle(PCustomDraw->hdc);
								Canvas->SetFont(GetFont());
								Canvas->SetBrush(GetBrush());
								DefaultDraw = CustomDraw(R, cdPrePaint);
								if(!DefaultDraw){
									Message.Result = CDRF_SKIPDEFAULT;
									return;
								}
							}
							if(IsCustomDrawn(dtItem, cdPrePaint) || IsCustomDrawn(dtItem, cdPreErase))
								Message.Result |= CDRF_NOTIFYITEMDRAW;
							if(IsCustomDrawn(dtItem, cdPostPaint))
								Message.Result |= CDRF_NOTIFYPOSTPAINT;
							if(IsCustomDrawn(dtItem, cdPostErase))
								Message.Result |= CDRF_NOTIFYPOSTERASE;
						}
						break;
						case CDDS_POSTPAINT:{
							if(IsCustomDrawn(dtControl, cdPostPaint))
								CustomDraw(R, cdPostPaint);
						}
						break;
						case CDDS_PREERASE:{
							if(IsCustomDrawn(dtControl, cdPreErase))
								CustomDraw(R, cdPreErase);
						}
						break;
						case CDDS_POSTERASE:{
							if(IsCustomDrawn(dtControl, cdPostErase))
								CustomDraw(R, cdPostErase);
						}
						break;
					}
				}
				else{
					TTVItem TmpItem;
					ZeroMemory(&TmpItem, sizeof(TmpItem));
					TmpItem.hItem = HTREEITEM(PCustomDraw->dwItemSpec);
					CTreeNode* Node = GetNodeFromItem(TmpItem);
					if(Node == NULL)
						return ;
					switch(PCustomDraw->dwDrawStage){
						case CDDS_ITEMPREPAINT:{
							// release the font we may have loaned during item drawing.
							if((PCustomDraw->dwDrawStage & CDDS_ITEMPOSTPAINT) != 0 && 
								OurFont != 0 && StockFont != 0){
								SelectObject(PCustomDraw->hdc, StockFont);
								DeleteObject(OurFont);
								OurFont = 0;
								StockFont = 0;
							}
							CanvasHandleCleaner Cleaner(Canvas);
							Canvas->SetHandle(PCustomDraw->hdc);
							Canvas->SetFont(GetFont());
							Canvas->SetBrush(GetBrush());
							// Unlike the list view, the tree view doesn't override the text
							//  foreground and background colors of selected items.
							if((PCustomDraw->uItemState & CDIS_SELECTED) != 0){
								Canvas->GetFont()->SetColor(clHighlightText);
								Canvas->GetBrush()->SetColor(clHighlight);
							}
							Canvas->GetFont()->SetOnChange(this, (TNotifyEvent)&CTreeView::MarkCanvasChanged);
							Canvas->GetBrush()->SetOnChange(this, (TNotifyEvent)&CTreeView::MarkCanvasChanged);
							CanvasChanged = FALSE;
							BOOL PaintImages = FALSE;
							BOOL DefaultDraw = CustomDrawItem(Node,
								(TCustomDrawState)PCustomDraw->uItemState, cdPrePaint, PaintImages);
							if(!PaintImages)
								Message.Result |= TVCDRF_NOIMAGES;
							if(!DefaultDraw)
								Message.Result |= CDRF_SKIPDEFAULT;
							else if(CanvasChanged){
								CanvasChanged = FALSE;
								Canvas->GetFont()->SetOnChange(NULL, NULL);
								Canvas->GetBrush()->SetOnChange(NULL, NULL);
								LPNMTVCUSTOMDRAW PTVCustomDraw = (LPNMTVCUSTOMDRAW)Message.NMHdr;
								PTVCustomDraw->clrText = ColorToRGB(Canvas->GetFont()->GetColor());
								PTVCustomDraw->clrTextBk = ColorToRGB(Canvas->GetBrush()->GetColor());
								LOGFONT LogFont;
								ZeroMemory(&LogFont, sizeof(LogFont));
								if(GetObject(Canvas->GetFont()->GetHandle(), sizeof(LogFont), &LogFont) != 0){
									Canvas->SetHandle(0);  // disconnect from hdc
									// don't delete the stock font
									OurFont = CreateFontIndirect(&LogFont);
									StockFont = (HFONT)SelectObject(PCustomDraw->hdc, OurFont);
									Message.Result |= CDRF_NEWFONT;
								}
							}
							if(IsCustomDrawn(dtItem, cdPostPaint))
								Message.Result |= CDRF_NOTIFYPOSTPAINT;
						}
						break;
						case CDDS_ITEMPOSTPAINT:{
							BOOL PaintImages = FALSE;
							if(IsCustomDrawn(dtItem, cdPostPaint))
								CustomDrawItem(Node, (TCustomDrawState)PCustomDraw->uItemState, cdPostPaint, PaintImages);
						}
						break;
						case CDDS_ITEMPREERASE:{
							BOOL PaintImages = FALSE;
							if(IsCustomDrawn(dtItem, cdPreErase))
								CustomDrawItem(Node, (TCustomDrawState)PCustomDraw->uItemState, cdPreErase, PaintImages);
						}
						break;
						case CDDS_ITEMPOSTERASE:{
							BOOL PaintImages = FALSE;
							if(IsCustomDrawn(dtItem, cdPostErase))
								CustomDrawItem(Node, (TCustomDrawState)PCustomDraw->uItemState, cdPostErase, PaintImages);
						}
						break;
					}
				}
			}
		}
		break;
		case TVN_BEGINDRAG:{
			Dragged = TRUE;
			DragNode = GetNodeFromItem(((LPNMTREEVIEW)Message.NMHdr)->itemNew);
		}
		break;
		case TVN_BEGINLABELEDIT:{
			LPNMTVDISPINFO DispInfo = (LPNMTVDISPINFO)Message.NMHdr;
			if(/*TODO Dragging() || */!CanEdit(GetNodeFromItem(DispInfo->item)))
				Message.Result = 1;
			if(Message.Result == 0){
				EditHandle = TreeView_GetEditControl(GetHandle());
				DefEditProc = (LPVOID)GetWindowLongPtr(EditHandle, GWLP_WNDPROC);
				SetWindowLongPtr(EditHandle, GWLP_WNDPROC, (LONG_PTR)EditInstance);
			}
		}
		break;
		case TVN_ENDLABELEDIT:{
			Edit(((LPNMTVDISPINFO)Message.NMHdr)->item);
		}
		break;
		case TVN_ITEMEXPANDING:{
			if(!ManualNotify){
				LPNMTREEVIEW PTreeView = (LPNMTREEVIEW)Message.NMHdr;
				CTreeNode* Node = GetNodeFromItem(PTreeView->itemNew);
				if(PTreeView->action == TVE_EXPAND && !CanExpand(Node))
					Message.Result = 1;
				else if(PTreeView->action == TVE_COLLAPSE && !CanCollapse(Node))
					Message.Result = 1;
			}
		}
		break;
		case TVN_ITEMEXPANDED:{
			if(!ManualNotify){
				LPNMTREEVIEW PTreeView = (LPNMTREEVIEW)Message.NMHdr;
				CTreeNode* Node = GetNodeFromItem(PTreeView->itemNew);
				if(PTreeView->action == TVE_EXPAND)
					Expand(Node);
				else
					if(PTreeView->action == TVE_COLLAPSE)
						Collapse(Node);
			}
		}
		break;
		case TVN_SELCHANGINGA:
		case TVN_SELCHANGINGW:{
			if(!CanChange(GetNodeFromItem(((LPNMTREEVIEW)Message.NMHdr)->itemNew)))
				Message.Result = 1;
		}
		break;
		case TVN_SELCHANGEDA:
		case TVN_SELCHANGEDW:{
			LPNMTREEVIEW PTreeView = (LPNMTREEVIEW)Message.NMHdr;
			if(ChangeTimer->GetInterval() > 0){
				ChangeTimer->SetEnabled(FALSE);
				ChangeTimer->SetTag(GetNodeFromItem(PTreeView->itemNew));
				ChangeTimer->SetEnabled(TRUE);
			}
			else
				Change(GetNodeFromItem(PTreeView->itemNew));
		}
		break;
		case TVN_DELETEITEM:{
			CTreeNode* Node = GetNodeFromItem(((LPNMTREEVIEW)Message.NMHdr)->itemOld);
			if(Node != NULL){
				Node->ItemId = NULL;
				ChangeTimer->SetEnabled(FALSE);
				if(StateChanging)
					Node->Delete();
				else
					GetTreeNodes()->Delete(Node);
			}
		}
		break;
		case TVN_SETDISPINFO:{
			LPNMTVDISPINFO DispInfo = (LPNMTVDISPINFO)Message.NMHdr;
			CTreeNode* Node = GetNodeFromItem(DispInfo->item);
			if(Node != NULL && (DispInfo->item.mask & TVIF_TEXT) != 0)
				Node->SetText(String(DispInfo->item.pszText));
		}
		break;
		case TVN_GETDISPINFO:{
			LPNMTVDISPINFO DispInfo = (LPNMTVDISPINFO)Message.NMHdr;
			CTreeNode* Node = GetNodeFromItem(DispInfo->item);
			if(Node != NULL){
				if((DispInfo->item.mask & TVIF_TEXT) != 0)
					lstrcpyn(DispInfo->item.pszText, Node->GetText().GetBuffer(), DispInfo->item.cchTextMax - 1);
				if((DispInfo->item.mask & TVIF_IMAGE) != 0){
					GetImageIndex(Node);
					DispInfo->item.iImage = Node->GetImageIndex();
				}
				if((DispInfo->item.mask & TVIF_SELECTEDIMAGE) != 0){
					GetSelectedIndex(Node);
					DispInfo->item.iSelectedImage = Node->GetSelectedIndex();
				}
			}
		}
		break;
		case NM_RCLICK:{
			RClickNode = NULL;
			TPoint MousePos = {0, 0};
			GetCursorPos(&MousePos);
			if(RightClickSelect){
				TPoint pt = ScreenToClient(MousePos);
				RClickNode = GetNodeAt(pt.x, pt.y);
				Perform(WM_CONTEXTMENU, (WPARAM)GetHandle(), (LPARAM)PointToSmallPoint(MousePos));
				RClickNode = NULL;
			}
			else{
				//Win95/98 eat WM_CONTEXTMENU when posted to the message queue  
				PostMessage(GetHandle(), CN_BASE+WM_CONTEXTMENU, (WPARAM)GetHandle(), (LPARAM)PointToSmallPoint(MousePos));
			}
			Message.Result = 1;  // tell treeview not to perform default response
		}
		break;
	}
}

void CTreeView::WMLButtonDown(TWMLButtonDown& Message){
	Dragged = FALSE;
	DragNode = NULL;
	__try{
		INHERITED_MSG(Message);
		/* TODO
		if(DragMode == dmAutomatic && DragKind == dkDrag){
			SetFocus();
			if(!Dragged){
				TPoint MousePos = {0, 0};
				GetCursorPos(&MousePos);
				TPoint pt = ScreenToClient(MousePos);
				Perform(WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
			}
			else {
				CTreeNode* Node = GetNodeAt(Message.XPos, Message.YPos);
				if(Node != NULL){
					Node->SetFocused(TRUE);
					Node->SetSelected(TRUE);
					BeginDrag(FALSE);
				}
			}
		}//*/
	}
	__finally{
		DragNode = NULL;
	}
}

void CTreeView::WMNotify(TWMNotify& Message){
	if(Message.NMHdr->code == TTN_NEEDTEXTW){
		// Work around NT COMCTL32 problem with tool tips >= 80 characters
		LPTOOLTIPTEXT lpTip = (LPTOOLTIPTEXT)Message.NMHdr;
		TPoint Pt = {0, 0};
		GetCursorPos(&Pt);
		Pt = ScreenToClient(Pt);
		CTreeNode* Node = GetNodeAt(Pt.x, Pt.y);
		if(Node == NULL || Node->GetText().Length() == 0 || 
			(lpTip->uFlags & TTF_IDISHWND) == 0)
			return;
		if(GetComCtlVersion() >= ComCtlVersionIE4 && Node->GetText().Length() < 80){
			INHERITED_MSG(Message);
			return;
		}
		WideText = new String(Node->GetText());
		INT szTextSize = sizeof(lpTip->szText);
		INT MaxTextLen = szTextSize / sizeof(TCHAR);
		if((INT)(WideText->Length()) >= MaxTextLen)
			WideText->Delete(MaxTextLen - 1, WideText->Length() - MaxTextLen + 1);
		lpTip->lpszText = WideText->GetBuffer();
		ZeroMemory(lpTip->szText, szTextSize);
		lstrcpyn(lpTip->szText, WideText->GetBuffer(), MaxTextLen);
		lpTip->hinst = 0;
		SetWindowPos(Message.NMHdr->hwndFrom, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE |
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER);
		Message.Result = 1;
	}
	else INHERITED_MSG(Message);
}

void CTreeView::WMContextMenu(TWMContextMenu& Message){
	TPoint pt = {0, 0};
	if(!IN_TEST(csDesigning, GetComponentState()))
		SetFocus();
	if(InvalidPoint(Message.Pos.x, Message.Pos.y) && (GetSelected() != NULL)){
		Message.Pos = PointToSmallPoint1(ClientToScreen(CenterPoint(GetSelected()->DisplayRect(TRUE))));
	}
	INHERITED_MSG(Message);
	if(IN_TEST(csDestroying, GetComponentState()))
		return;
	TPoint P = ScreenToClient(Point(Message.Pos.x, Message.Pos.y));
	MouseUp(mbRight, KeyboardStateToShiftState(), P.x, P.y);
}

void CTreeView::CMSysColorChange(TMessage& Message){
	INHERITED_MSG(Message);
	if(!IN_TEST(csLoading, GetComponentState())){
		Message.Msg = WM_SYSCOLORCHANGE;
		DefaultHandler(Message);
	}
}

LRESULT CTreeView::EditWndProc(UINT Message, WPARAM wParam, LPARAM lParam){
	TMessage Msg;
	Msg.Msg = Message;
	Msg.wParam = wParam;
	Msg.lParam = lParam;
	Msg.Result = 0;
	PWMKey pKeyMsg = (PWMKey)&Msg;
	__try{
		switch(Message){
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if(DoKeyDown(*pKeyMsg))
				return Msg.Result;
			break;
		case WM_CHAR:
			if(DoKeyPress(*pKeyMsg))
				return Msg.Result;
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if(DoKeyUp(*pKeyMsg))
				return Msg.Result;
			break;
		case CN_KEYDOWN:
		case CN_CHAR:
		case CN_SYSKEYDOWN:
		case CN_SYSCHAR:
			WndProc(Msg);
			return Msg.Result;
		}
		return CallWindowProc((WNDPROC)DefEditProc, EditHandle, Message, wParam, lParam);
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		GetGlobal().CallHandleException(this);
	}
	return Msg.Result;
}

//TODO void DoDragOver(CDragObject* Source, INT X, INT Y, BOOL CanDrop);
void CTreeView::NodeDeselect(INT Index){
	GetSelection(Index)->SetSelectedBit(FALSE);
	Selections->Delete(Index);
}

void CTreeView::NodeSelect(CTreeNode* Node, INT At){
	if(Node != NULL && !(Node->GetDeleting())){
		Selections->Insert(At, Node);
		Node->SetSelectedBit(TRUE);
	}
}

void CTreeView::FinishSelection(CTreeNode* Node, TShiftState ShiftState){
	if(!Selecting && IN_TEST(ssLeft, ShiftState)){
		Selecting = TRUE;
		__try{
			if(!IN_TEST(ssShift, ShiftState))
				ShiftAnchor = NULL;
			// what to do?
			if(GetMultiSelect() && (Node != NULL)){
				if(IN_TEST(ssShift, ShiftState)){
					// figure out the shift anchor
					if (ShiftAnchor == NULL && Selections->GetCount() > 0)
						ShiftAnchor = GetSelection(0);
					if (ShiftAnchor == NULL){
						CTreeNode* LNode = GetTreeNodes()->GetFirstNode();
						if(!LNode->IsNodeVisible())
							LNode = LNode->GetNextVisible();
						while(LNode != NULL){
							if(LNode->GetFocused()){
								ShiftAnchor = LNode;
								break;
							}
							LNode = LNode->GetNextVisible();
						}
					}
					BOOL LBackward = ShiftAnchor != NULL && 
						(Node->GetAbsoluteIndex() < ShiftAnchor->GetAbsoluteIndex());
				
					// which way do we go?
					if(IN_TEST(ssCtrl, ShiftState))
						ControlShiftSelectNode(Node, LBackward);
					else
						ShiftSelectNode(Node, LBackward);
				}
				else{
					// no shift, no problem
					if(IN_TEST(ssCtrl, ShiftState))
						ControlSelectNode(Node);
					else if(Selections->IndexOf(Node) != -1){
						if(Selections->Get(0) != Node){
							Selections->Remove(Node);
							NodeSelect(Node, 0);
						}
					}
					else
						SelectNode(Node);
				}
			}
			else
				SelectNode(Node);
			// all is swell?
			ValidateSelection();
		}
		__finally{
			Selecting = FALSE;
		}
	}
}

void CTreeView::ControlSelectNode(CTreeNode* Node){
	if(IN_TEST(msControlSelect, MultiSelectStyle)){
		if(Node != GetSelected() && (Node == NULL || !Node->GetDeleting()))
			SetSelected(Node);
		INT I = Selections->IndexOf(Node);
		if(I != -1)
			NodeDeselect(I);
		else 
			NodeSelect(Node);
	}
	else
		SelectNode(Node);
}

void CTreeView::ShiftSelectNode(CTreeNode* Node, BOOL Backward, BOOL Deselect){
	if(Node != NULL && !Node->GetDeleting() && IN_TEST(msShiftSelect, MultiSelectStyle)){
		CList LSelect;
		CList LDeselect;
		CTreeNode* LNode = ShiftAnchor;
		if(LNode != Node)
			while(LNode != NULL){
				LSelect.Add(LNode);
				if(Backward)
					if(IN_TEST(msVisibleOnly, MultiSelectStyle))
						LNode = LNode->GetPrevVisible();
					else
						LNode = LNode->GetPrev();
				else
					if(IN_TEST(msVisibleOnly, MultiSelectStyle))
						LNode = LNode->GetNextVisible();
					else 
						LNode = LNode->GetNext();
				if(LNode == Node){
					LSelect.Add(LNode);
					break;
				}
			}

		if(Deselect){
			LDeselect.Assign(Selections, laSrcUnique, &LSelect);
			if(LDeselect.GetCount() > 0)
				for(INT I = Selections->GetCount() - 1; I >= 0; I--)
					if(LDeselect.IndexOf(Selections->Get(I)) != -1)
						NodeDeselect(I);
		}
		LSelect.Assign(Selections, laSrcUnique);
		for(INT I = 0; I < LSelect.GetCount(); I++)
			NodeSelect((CTreeNode*)LSelect.Get(I));
		
		INT I = Selections->IndexOf(Node);
		if(I > 0){
			Selections->Delete(I);
			Selections->Insert(0, Node);
		}
		else if(I == -1)
			NodeSelect(Node);
	}
	else
		SelectNode(Node);
}

void CTreeView::ControlShiftSelectNode(CTreeNode* Node, BOOL Backward){
	ShiftSelectNode(Node, Backward, !IN_TEST(msControlSelect, MultiSelectStyle));
}

void CTreeView::SelectNode(CTreeNode* Node){
	for(INT I = Selections->GetCount() - 1; I >= 0; I--)
		if(Selections->Get(I) != Node)
			NodeDeselect(I);
	if(Node != GetSelected() && (Node == NULL || !Node->GetDeleting()))
		SetSelected(Node);
	if(Node != NULL && !Node->GetDeleting() && Selections->GetCount() == 0)
		NodeSelect(Node);
}

INT CTreeView::GetChangeDelay(){
	return ChangeTimer->GetInterval();
}

HTREEITEM TreeView_GetDropHilite(HWND hwnd){
	return TreeView_GetNextItem(hwnd, NULL, TVGN_DROPHILITE);
}

CTreeNode* CTreeView::GetDropTarget(){
	CTreeNode* Result = NULL;
	if(HandleAllocated()){
		Result = GetTreeNodes()->GetNode(TreeView_GetDropHilite(GetHandle()));
		if(Result == NULL)
			Result = LastDropTarget;
	}
	return Result;
}

INT CTreeView::GetIndent(){
	return TreeView_GetIndent(GetHandle());
}

CTreeNode* CTreeView::GetNodeFromItem(TTVItem& Item){
	if(GetTreeNodes() != NULL)
		if((Item.state & TVIF_PARAM) != 0)
			return (CTreeNode*)Item.lParam;
		else
			return GetTreeNodes()->GetNode(Item.hItem);
	return NULL;
}

CTreeNode* CTreeView::GetSelected(){
	if(HandleAllocated()){
		if(RightClickSelect && RClickNode != NULL)
			return RClickNode;
		else
			return GetTreeNodes()->GetNode(TreeView_GetSelection(GetHandle()));
	}
	else
		return NULL;
}

UINT CTreeView::GetSelectionCount(){
	return Selections->GetCount();
}

CTreeNode* CTreeView::GetSelection(INT Index){
	return ((CTreeNode *)Selections->Get(Index));
}

CTreeNode* CTreeView::GetTopItem(){
	if(HandleAllocated())
		return GetTreeNodes()->GetNode(TreeView_GetFirstVisible(GetHandle()));
	else return NULL;
}

void CTreeView::ImageListChange(CObject* Sender){
	if(HandleAllocated()){
		HIMAGELIST ImageHandle = 0;
		if(((CCustomImageList *)Sender)->HandleAllocated())
			ImageHandle = ((CCustomImageList *)Sender)->GetHandle();
		if(Sender == Images)
			SetImageList(ImageHandle, TVSIL_NORMAL);
		else if(Sender == StateImages)
			SetImageList(ImageHandle, TVSIL_STATE);
	}
}

void CTreeView::SetAutoExpand(BOOL Value){
	if(AutoExpand != Value){
		AutoExpand = Value;
		SetComCtlStyle(this, TVS_SINGLEEXPAND, Value);
	}
}

void CTreeView::SetBorderStyle(TBorderStyle Value){
	if(BorderStyle != Value){
		BorderStyle = Value;
		RecreateWnd();
	}
}

void CTreeView::SetButtonStyle(BOOL Value){
	if(ShowButtons != Value){
		ShowButtons = Value;
		SetComCtlStyle(this, TVS_HASBUTTONS, Value);
	}
}

void CTreeView::SetChangeDelay(INT Value){
	ChangeTimer->SetInterval(Value);
}

void CTreeView::SetDropTarget(CTreeNode* Value){
	if(HandleAllocated())
		if(Value != NULL)
			Value->SetDropTarget(TRUE);
		else
			TreeView_SelectDropTarget(GetHandle(), 0);
}

void CTreeView::SetHideSelection(BOOL Value){
	if(HideSelection != Value){
		HideSelection = Value;
		SetComCtlStyle(this, TVS_SHOWSELALWAYS, !Value);
		Invalidate();
	}
}

void CTreeView::SetHotTrack(BOOL Value){
	if(HotTrack != Value){
		HotTrack = Value;
		SetComCtlStyle(this, TVS_TRACKSELECT, Value);
	}
}

void CTreeView::SetImageList(HIMAGELIST Value, INT Flags){
	if(HandleAllocated())
		TreeView_SetImageList(GetHandle(), Value, Flags);
}

void CTreeView::SetIndent(INT Value){
	if(Value != GetIndent())
		TreeView_SetIndent(GetHandle(), Value);
}

void CTreeView::SetImages(CImageList* Value){
	if(Images != NULL)
		Images->UnRegisterChanges(ImageChangeLink);
	Images = Value;
	if(Images != NULL){
		Images->RegisterChanges(ImageChangeLink);
		Images->FreeNotification(this);
		SetImageList(Images->GetHandle(), TVSIL_NORMAL);
	}
	else
		SetImageList(0, TVSIL_NORMAL);
}

void CTreeView::SetLineStyle(BOOL Value){
	if(ShowLines != Value){
		ShowLines = Value;
		SetComCtlStyle(this, TVS_HASLINES, Value);
	}
}

void CTreeView::SetMultiSelect(BOOL Value){
	if(Value != MultiSelect){
		if(!Value)
			SelectNode(GetSelected());
		MultiSelect = Value;
		ValidateSelection();
	}
}

void CTreeView::SetMultiSelectStyle(TMultiSelectStyle Value){
	if(Value != MultiSelectStyle){
		MultiSelectStyle = Value;
		ValidateSelection();
	}
}

void CTreeView::SetReadOnly(BOOL Value){
	if(ReadOnly != Value){
		ReadOnly = Value;
		SetComCtlStyle(this, TVS_EDITLABELS, !Value);
	}
}

void CTreeView::SetRootStyle(BOOL Value){
	if(ShowRoot != Value){
		ShowRoot = Value;
		SetComCtlStyle(this, TVS_LINESATROOT, Value);
	}
}

void CTreeView::SetRowSelect(BOOL Value){
	if(RowSelect != Value){
		RowSelect = Value;
		SetComCtlStyle(this, TVS_FULLROWSELECT, Value);
	}
}

void CTreeView::SetSelected(CTreeNode* Value){
	if(Value != NULL)
		Value->SetSelected(TRUE);
	else
		TreeView_SelectItem(GetHandle(), NULL);
}

void CTreeView::SetSortType(TSortType Value){
	if(SortType != Value){
		SortType = Value;
		if(((SortType == stData || SortType == stBoth) && OnCompare != NULL) ||
			(SortType == stText || SortType == stBoth))
			AlphaSort();
	}
}

void CTreeView::SetStateImages(CImageList* Value){
	if(StateImages != NULL)
		StateImages->UnRegisterChanges(StateChangeLink);
	StateImages = Value;
	if(StateImages != NULL){
		StateImages->RegisterChanges(StateChangeLink);
		StateImages->FreeNotification(this);
		SetImageList(StateImages->GetHandle(), TVSIL_STATE);
	}
	else 
		SetImageList(0, TVSIL_STATE);
}

void CTreeView::SetToolTips(BOOL Value){
	if(ToolTips != Value){
		ToolTips = Value;
		SetComCtlStyle(this, TVS_NOTOOLTIPS, !Value);
	}
}

void CTreeView::SetTreeNodes(CTreeNodes* Value){
	TreeNodes->Assign(Value);
}

void CTreeView::SetTopItem(CTreeNode* Value){
	if(HandleAllocated() && Value != NULL)
		TreeView_SelectSetFirstVisible(GetHandle(), Value->GetItemId());
}

void CTreeView::OnChangeTimer(CObject* Sender){
	ChangeTimer->SetEnabled(FALSE);
	Change((CTreeNode*)ChangeTimer->GetTag());
}

BOOL CTreeView::CanEdit(CTreeNode* Node){
	BOOL Result = TRUE;
	if(OnEditing != NULL)
		CALL_EVENT(Editing)(this, Node, Result);
	return Result;
}

BOOL CTreeView::CanChange(CTreeNode* Node){
	BOOL Result = TRUE;
	if(OnChanging != NULL)
		CALL_EVENT(Changing)(this, Node, Result);
	return Result;
}

BOOL CTreeView::CanCollapse(CTreeNode* Node){
	BOOL Result = TRUE;
	if(OnCollapsing != NULL)
		CALL_EVENT(Collapsing)(this, Node, Result);
	return Result;
}

BOOL CTreeView::CanExpand(CTreeNode* Node){
	BOOL Result = TRUE;
	if(OnExpanding != NULL)
		CALL_EVENT(Expanding)(this, Node, Result);
	return Result;
}

void CTreeView::Change(CTreeNode* Node){
	SelectChanged = TRUE;
	FinishSelection(GetSelected(), KeyDataToShiftState(0) | ssLeft);
	if(OnChange != NULL)
		CALL_EVENT(Change)(this, Node);
}

void CTreeView::Collapse(CTreeNode* Node){
	if(OnCollapsed != NULL)
		CALL_EVENT(Collapsed)(this, Node);
}

CTreeNode* CTreeView::CreateNode(){
	CTreeNodeClass* LClass = (CTreeNodeClass*)CTreeNode::_Class;
	if(OnCreateNodeClass != NULL)
		CALL_EVENT(CreateNodeClass)(this, &LClass);
	CTreeNode* Result = (CTreeNode*)LClass->NewInstance();
	Result->Owner = GetTreeNodes();
	return Result;
}

CTreeNodes* CTreeView::CreateNodes(){
	return new CTreeNodes(this);
}

void CTreeView::CreateParams(TCreateParams& Params){
	DWORD BorderStyles[] = {0, WS_BORDER};
	DWORD LineStyles[] = {0, TVS_HASLINES};
	DWORD RootStyles[] = {0, TVS_LINESATROOT};
	DWORD ButtonStyles[] = {0, TVS_HASBUTTONS};
	DWORD EditStyles[] = {TVS_EDITLABELS, 0};
	DWORD HideSelections[] = {TVS_SHOWSELALWAYS, 0};
	DWORD DragStyles[] = {TVS_DISABLEDRAGDROP, 0};
	DWORD RTLStyles[] = {0, TVS_RTLREADING};
	DWORD ToolTipStyles[] = {TVS_NOTOOLTIPS, 0};
	DWORD AutoExpandStyles[] = {0, TVS_SINGLEEXPAND};
	DWORD HotTrackStyles[] = {0, TVS_TRACKSELECT};
	DWORD RowSelectStyles[] = {0, TVS_FULLROWSELECT};

	InitCommonControl(ICC_TREEVIEW_CLASSES);
	__super::CreateParams(Params);
	CreateSubClass(Params, WC_TREEVIEW);
	Params.Style |= LineStyles[ShowLines] | BorderStyles[BorderStyle] |
		RootStyles[ShowRoot] | ButtonStyles[ShowButtons] |
		EditStyles[ReadOnly] | HideSelections[HideSelection] |
		//DragStyles[GetDragMode()] | 
		RTLStyles[UseRightToLeftReading()] |
		ToolTipStyles[ToolTips] | AutoExpandStyles[AutoExpand] |
		HotTrackStyles[HotTrack] | RowSelectStyles[RowSelect];
	if(GetCtl3D() && GetGlobal().GetNewStyleControls() && BorderStyle == bsSingle){
		Params.Style &= ~WS_BORDER;
		Params.ExStyle |= WS_EX_CLIENTEDGE;
	}
	Params.WinClass.style &= ~(CS_HREDRAW | CS_VREDRAW);
}

void CTreeView::CreateWnd(){
	StateChanging = FALSE;
	__super::CreateWnd();
	TreeView_SetBkColor(GetHandle(), ColorToRGB(GetColor()));
	TreeView_SetTextColor(GetHandle(), ColorToRGB(GetFont()->GetColor()));
	if (MemStream != NULL){
		CTreeNodes* Items = GetTreeNodes();
		CMethodLock lock(Items, (TLockMethod)&CTreeNodes::BeginUpdate, (TLockMethod)&CTreeNodes::EndUpdate);
		Items->ReadData(MemStream);
		Items->ReadExpandedState(MemStream);
		FreeAndNil((CObject**)&MemStream);
		SetTopItem(Items->GetNodeFromIndex(SaveTopIndex));
		SaveTopIndex = 0;
		if(SaveIndexes != NULL){
			for(INT I = 0; SaveIndexes->GetCount(); I++)
				Selections->Add(Items->GetNodeFromIndex((INT)SaveIndexes->Get(I)));
			FreeAndNil((CObject**)&SaveIndexes);
			ValidateSelection();
			SetSelected(GetSelection(0));
		}
		else
			SetSelected(Items->GetNodeFromIndex(SaveIndex));
		SaveIndex = 0;
	}
	if(SaveIndent != -1)
		SetIndent(SaveIndent);
	if(Images != NULL && Images->HandleAllocated())
		SetImageList(Images->GetHandle(), TVSIL_NORMAL);
	if(StateImages != NULL && StateImages->HandleAllocated())
		SetImageList(StateImages->GetHandle(), TVSIL_STATE);
}

BOOL CTreeView::CustomDraw(TRect& ARect, TCustomDrawStage Stage){
	BOOL Result = TRUE;
	if(Stage == cdPrePaint && OnCustomDraw != NULL)
		CALL_EVENT(CustomDraw)(this, ARect, Result);
	if(OnAdvancedCustomDraw != NULL)
		CALL_EVENT(AdvancedCustomDraw)(this, ARect, Stage, Result);
	return Result;
}

BOOL CTreeView::CustomDrawItem(CTreeNode* Node, TCustomDrawState State,
	TCustomDrawStage Stage, BOOL& PaintImages){
	BOOL Result = TRUE;
	PaintImages = TRUE;
	if(Stage == cdPrePaint && OnCustomDrawItem != NULL)
		CALL_EVENT(CustomDrawItem)(this, Node, State, Result);
	if(OnAdvancedCustomDrawItem != NULL)
		CALL_EVENT(AdvancedCustomDrawItem)(this, Node, State, Stage, PaintImages, Result);
	return Result;
}

void CTreeView::Delete(CTreeNode* Node){
	if(OnDeletion != NULL)
		CALL_EVENT(Deletion)(this, Node);
}

void CTreeView::Added(CTreeNode* Node){
	if(OnAddition != NULL)
		CALL_EVENT(Addition)(this, Node);
}

void CTreeView::DestroyWnd(){
	StateChanging = TRUE;
	RClickNode = NULL;
	CTreeNodes* Items = GetTreeNodes();
	if(CreateWndRestores && Items->GetCount() > 0){
		MemStream = new CMemoryStream();
		Items->WriteData(MemStream);
		Items->WriteExpandedState(MemStream);
		MemStream->SetPosition(0);
		SaveTopIndex = 0;
		SaveIndex = 0;
		CTreeNode* Node = GetTopItem();
		if(Node != NULL)
			SaveTopIndex = Node->GetAbsoluteIndex();
		CMethodLock lock(Items, (TLockMethod)&CTreeNodes::BeginUpdate, (TLockMethod)&CTreeNodes::EndUpdate);
		if(MultiSelect && Selections->GetCount() > 1){
			SaveIndexes = new CList();
			for(INT I = 0; I < Selections->GetCount(); I++)
				SaveIndexes->Add((LPVOID)((CTreeNode *)Selections->Get(I))->GetAbsoluteIndex());
			Selections->Clear();
		}
		else{
			Node = GetSelected();
			if(Node != NULL)
				SaveIndex = Node->GetAbsoluteIndex();
		}
		Items->Clear();
	}
	SaveIndent = GetIndent();
	__super::DestroyWnd();
}

//TODO void DoEndDrag(CObject* Target, INT X, INT Y) override;
//TODO void DoStartDrag(TDragObject& DragObject) override;
void CTreeView::Edit(TTVItem& Item){
	CTreeNode* Node = GetNodeFromItem(Item);
	if(Item.pszText != NULL){
		String S(Item.pszText);
		if(OnEdited != NULL)
			CALL_EVENT(Edited)(this, Node, S);
		if(Node != NULL)
			Node->SetText(S);
	}
	else if(OnCancelEdit != NULL)
		CALL_EVENT(CancelEdit)(this, Node);
}

void CTreeView::Expand(CTreeNode* Node){
	if(OnExpanded != NULL)
		CALL_EVENT(Expanded)(this, Node);
}

//TODO CDragImageList* GetDragImages() override;
void CTreeView::GetImageIndex(CTreeNode* Node){
	if(OnGetImageIndex != NULL)
		CALL_EVENT(GetImageIndex)(this, Node);
}

void CTreeView::GetSelectedIndex(CTreeNode* Node){
	if(OnGetSelectedIndex != NULL)
		CALL_EVENT(GetSelectedIndex)(this, Node);
}

BOOL CTreeView::IsCustomDrawn(TCustomDrawTarget Target, TCustomDrawStage Stage){
	BOOL Result = FALSE;
	//Tree view doesn't support erase notifications
	if(Stage == cdPrePaint){
		if(Target == dtItem)
			Result = OnCustomDrawItem != NULL || OnAdvancedCustomDrawItem != NULL;
		else if(Target == dtControl)
			Result = OnCustomDraw != NULL || OnAdvancedCustomDraw != NULL
				|| OnCustomDrawItem != NULL || OnAdvancedCustomDrawItem != NULL;
	}
	else{
		if(Target == dtItem)
			Result = OnAdvancedCustomDrawItem != NULL;
		else if(Target == dtControl)
			Result = OnAdvancedCustomDraw != NULL || OnAdvancedCustomDrawItem != NULL;
	}
	return Result;
}

void CTreeView::Loaded(){
	__super::Loaded();
	if(IN_TEST(csDesigning, GetComponentState()))
		FullExpand();
}

void CTreeView::Notification(CComponent* AComponent, TOperation Operation){
	__super::Notification(AComponent, Operation);
	if(Operation == opRemove){
		if(AComponent == Images)
			Images = NULL;
		if(AComponent == StateImages)
			StateImages = NULL;
	}
}

//TODO void SetDragMode(TDragMode Value) override;
void CTreeView::WndProc(TMessage& Message){
	if(!IN_TEST(csDesigning, GetComponentState()) 
		&& (Message.Msg == WM_LBUTTONDOWN || Message.Msg == WM_LBUTTONDBLCLK) 
		){//TODO && !GetDragging() && (GetDragMode() == dmAutomatic) && (GetDragKind() == dkDrag){
		if(!IsControlMouseMsg(*((PWMMouse)&Message))){
			SetControlState(GetControlState() | csLButtonDown);
			Dispatch(Message);
		}
	}
	else if(Message.Msg == CN_BASE + WM_CONTEXTMENU)
		Message.Result = Perform(WM_CONTEXTMENU, Message.wParam, Message.lParam);
	else __super::WndProc(Message);
}

void CTreeView::ValidateSelection(){
	if(Selections->GetCount() > 0){
		CTreeNode* LPrimary = (CTreeNode*)Selections->Get(0);
		for(INT I = Selections->GetCount() - 1; I >= 0; I--){
			CTreeNode* LNode = (CTreeNode*)Selections->Get(I);
			if(LNode->Deleting || (
				(I != 0 && !MultiSelect) ||
				(IN_TEST(msVisibleOnly, MultiSelectStyle) && !LNode->IsNodeVisible()) ||
				(IN_TEST(msSiblingOnly, MultiSelectStyle) && LNode->GetParent() != LPrimary->GetParent())
				))
				NodeDeselect(I);
			else if(!LNode->GetSelected())
				LNode->SetSelectedBit(TRUE);
		}
	}
}

void CTreeView::InvalidateSelectionsRects(){
	for(INT I = 0; I < Selections->GetCount(); I++){
		TRect LRect = ((CTreeNode*)Selections->Get(I))->DisplayRect(FALSE);
		InvalidateRect(GetHandle(), (const RECT *)&LRect, FALSE);
	}
}

void CTreeView::MouseDown(TMouseButton Button, TShiftState Shift, INT X, INT Y){
	if(Button == mbLeft){
		if(SelectChanged)
			SelectChanged = FALSE;
		else if(IN_TEST(htOnItem, GetHitTestInfoAt(X, Y)))
			FinishSelection(GetSelected(), Shift);
		else
			ValidateSelection();
	}
	__super::MouseDown(Button, Shift, X, Y);
}

void CTreeView::DoEnter() {
	InvalidateSelectionsRects();
	__super::DoEnter();
}

void CTreeView::DoExit(){
	__super::DoExit();
	InvalidateSelectionsRects();
}

BOOL CTreeView::AlphaSort(BOOL ARecurse){
	return CustomSort(NULL, 0, ARecurse);
}

BOOL CTreeView::CustomSort(TTVCompare SortProc, LONG_PTR Data, BOOL ARecurse){
	BOOL Result = FALSE;
	if(HandleAllocated()){
		TTVSortCB SortCB;
		ZeroMemory(&SortCB, sizeof(SortCB));
		if(SortProc == NULL)
			SortCB.lpfnCompare = (PFNTVCOMPARE)&DefaultTreeViewSort;
		else 
			SortCB.lpfnCompare = SortProc;
		SortCB.hParent = TVI_ROOT;
		SortCB.lParam = Data;
		Result = TreeView_SortChildrenCB(GetHandle(), &SortCB, 0);
		if(ARecurse){
			CTreeNode* Node = TreeNodes->GetFirstNode();
			while(Node != NULL){
				if(Node->GetChildren())
					Node->CustomSort(SortProc, Data, TRUE);
				Node = Node->GetNextSibling();
			}
		}
		GetTreeNodes()->ClearCache();
	}
	return Result;
}

void CTreeView::FullCollapse(){
	CTreeNode* Node = GetTreeNodes()->GetFirstNode();
	while(Node != NULL){
		Node->Collapse(TRUE);
		Node = Node->GetNextSibling();
	}
}

void CTreeView::FullExpand(){
	CTreeNode* Node = GetTreeNodes()->GetFirstNode();
	while(Node != NULL){
		Node->Expand(TRUE);
		Node = Node->GetNextSibling();
	}
}

THitTests CTreeView::GetHitTestInfoAt(INT X, INT Y){
	THitTests Result = 0;
	TVHITTESTINFO HitTest;
	ZeroMemory(&HitTest, sizeof(HitTest));
	HitTest.pt.x = X;
	HitTest.pt.y = Y;
	TreeView_HitTest(GetHandle(), &HitTest);
	if((HitTest.flags & TVHT_ABOVE) != 0) Result |= htAbove;
    if((HitTest.flags & TVHT_BELOW) != 0) Result |= htBelow;
	if((HitTest.flags & TVHT_NOWHERE) != 0) Result |= htNowhere;
    if((HitTest.flags & TVHT_ONITEM) == TVHT_ONITEM)
		Result |= htOnItem;
	else {
		if((HitTest.flags & TVHT_ONITEM) != 0) Result |= htOnItem;
		if((HitTest.flags & TVHT_ONITEMICON) != 0) Result |= htOnIcon;
		if((HitTest.flags & TVHT_ONITEMLABEL) != 0) Result |= htOnLabel;
		if((HitTest.flags & TVHT_ONITEMSTATEICON) != 0) Result |= htOnStateIcon;
	}
    if((HitTest.flags & TVHT_ONITEMBUTTON) != 0) Result |= htOnButton;
    if((HitTest.flags & TVHT_ONITEMINDENT) != 0) Result |= htOnIndent;
    if((HitTest.flags & TVHT_ONITEMRIGHT) != 0) Result |= htOnRight;
    if((HitTest.flags & TVHT_TOLEFT) != 0) Result |= htToLeft;
    if((HitTest.flags & TVHT_TORIGHT) != 0) Result |= htToRight;
	return Result;
}

CTreeNode* CTreeView::GetNodeAt(INT X, INT Y){
	TVHITTESTINFO HitTest;
	ZeroMemory(&HitTest, sizeof(HitTest));
	HitTest.pt.x = X;
	HitTest.pt.y = Y;
	if(TreeView_HitTest(GetHandle(), &HitTest) != NULL)
		return GetTreeNodes()->GetNode(HitTest.hItem);
	return NULL;
}

BOOL CTreeView::IsEditing(){
	HWND ControlHand = TreeView_GetEditControl(GetHandle());
	return (ControlHand != 0) && IsWindowVisible(ControlHand);
}

void CTreeView::LoadFromFile(String& FileName){
	CFileStream Stream(FileName.GetBuffer(), fmOpenRead);
	LoadFromStream(&Stream);
}

void CTreeView::LoadFromStream(CStream* Stream){
	CTreeStrings Strs(GetTreeNodes());
	Strs.LoadTreeFromStream(Stream);
}

void CTreeView::SaveToFile(String& FileName){
	CFileStream Stream(FileName.GetBuffer(), fmCreate);
	SaveToStream(&Stream);
}

void CTreeView::SaveToStream(CStream* Stream){
	CTreeStrings Strs(GetTreeNodes());
	Strs.SaveTreeToStream(Stream);
}

void CTreeView::Select(CTreeNode* Node, TShiftState ShiftState){
	FinishSelection(Node, ShiftState | ssLeft);
}

void CTreeView::Select(CTreeNode* Nodes[], INT Len){
	CList LList;
	for(INT I = 0; I < Len; I++)
		if(!Nodes[I]->Deleting)
			LList.Add(Nodes[I]);
	Select(&LList);
}

void CTreeView::Select(CList* Nodes){
	if(Nodes->GetCount() == 0)
		ClearSelection();
	else{
		CList LSelect;
		CList LDeselect;

		//remove any nodes that are about to not be
		for(INT I = Nodes->GetCount() - 1; I >= 0; I--)
			if(((CTreeNode *)Nodes->Get(I))->Deleting)
				Nodes->Delete(I);

		// make sure the Selected item is the first item
		if(Nodes->GetCount() > 0)
			if(GetSelected() != ((CTreeNode*)Nodes->Get(0)))
				SetSelected((CTreeNode*)Nodes->Get(0));

		// what needs to be deselected?
		LDeselect.Assign(Selections, laSrcUnique, Nodes);
		if(LDeselect.GetCount() > 0)
			for(INT I = Selections->GetCount() - 1; I >= 0; I--)
				if(LDeselect.IndexOf(Selections->Get(I)) != -1)
					NodeDeselect(I);
		
		// what needs to be selected?
		LSelect.Assign(Nodes, laSrcUnique, Selections);
		for(INT I = 0; I < LSelect.GetCount() - 1; I++)
			NodeSelect(((CTreeNode *)LSelect.Get(I)));
		
		// ok lets get the order right!
		Selections->Assign(Nodes);
	}
	// show the world what we just did
	ValidateSelection();
}

void CTreeView::Deselect(CTreeNode* Node){
	if(Selections->IndexOf(Node) != -1)
		ControlSelectNode(Node);
}

void CTreeView::Subselect(CTreeNode* Node, BOOL Validate){
	if(!MultiSelect)
		throw "treeview error multi select required."; //raise ETreeViewError.Create(SMultiSelectRequired);
	if(Node != NULL && !Node->Deleting){
		if(Selections->IndexOf(Node) == -1)
			NodeSelect(Node, 1);
		if(Validate)
			ValidateSelection();
	}
}

void CTreeView::ClearSelection(BOOL KeepPrimary){
	for(INT I = Selections->GetCount() - 1; I >= 0; I--)
		NodeDeselect(I);
	if(!KeepPrimary)
		SetSelected(NULL);
}

CTreeNode* CTreeView::GetSelections(CList* AList){
	AList->Clear();
	INT Count = (INT)GetSelectionCount();
	for(INT I = 0; I < Count; I++)
		AList->Add(Selections->Get(I));
	return GetSelected();
}

BOOL CTreeView::NodeInList(CList* LNodes, CTreeNode* ANode){
	// return true only if node (or one of its parents) is not in the list
	while(ANode != NULL)
		if(LNodes->IndexOf(ANode) == -1)
			ANode = ANode->GetParent();
		else
			break;
	return ANode != NULL;
}

CTreeNode* CTreeView::SiblingNotInList(CList* LNodes, CTreeNode* ANode){
	// if a succeeding sibling of the primary is not selected then select it
	CTreeNode* Result = ANode->GetNextSibling();
	while(Result != NULL)
		if(NodeInList(LNodes, Result))
			Result = Result->GetNextSibling();
		else
			break;
	
	// next didn't work, try previous instead
	if(Result == NULL){
		Result = ANode->GetPrevSibling();
		while(Result != NULL)
			if(NodeInList(LNodes, Result))
				Result = Result->GetPrevSibling();
			else
				break;
	}
	return Result;
}

CTreeNode* CTreeView::FindNextToSelect(){
	CList LNodes;
	// what is selected?
    CTreeNode* LSelected = GetSelections(&LNodes);
    CTreeNode* Result = LSelected;
	
	// if the selected one is really selected then continue, otherwise return it
    while(LSelected != NULL && LSelected->GetSelected()){
		Result = SiblingNotInList(&LNodes, LSelected);
		if(Result == NULL)
			LSelected = LSelected->GetParent();
		else
			break;
		Result = LSelected;
	}
	return Result;
}
