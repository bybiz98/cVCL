
#include "stdinc.h"
#include "Collection.hpp"
#include "WinUtils.hpp"

IMPL_DYN_CLASS(CCollectionItem)
CCollectionItem::CCollectionItem(CCollection* Collection):CComponent(NULL),
	Collection(NULL),
	ID(0){
	SetCollection(Collection);
}

CCollectionItem::~CCollectionItem(){
	SetCollection(NULL);
}

void CCollectionItem::Changed(BOOL AllItems){
	CCollectionItem* Item;
	if(Collection != NULL && Collection->UpdateCount == 0){
		if(AllItems)
			Item = NULL;
		else 
			Item = this;
		Collection->Update(Item);
	}
}

CComponent* CCollectionItem::GetOwner(){
	return Collection;
}

String CCollectionItem::GetNamePath(){
	if(Collection != NULL)
		return String(NULL);//TODO Format('%s[%d]',[FCollection.GetNamePath, Index]);
	else
		return String(GetClass()->GetName());
}// override;

void CCollectionItem::SetCollection(CCollection* Value){
	if(Collection != Value){
		if(Collection != NULL)
			Collection->RemoveItem(this);
		if(Value != NULL)
			Value->InsertItem(this);
	}
}

INT CCollectionItem::GetIndex(){
	if(Collection != NULL)
		return Collection->Items->IndexOf(this);
	else
		return -1;
}

void CCollectionItem::SetIndex(INT Value){
	INT CurIndex = GetIndex();
	if(CurIndex >= 0 && CurIndex != Value){
		Collection->Items->Move(CurIndex, Value);
		Changed(TRUE);
	}
}

String CCollectionItem::GetDisplayName(){
	return String(GetClass()->GetName());
}

void CCollectionItem::SetDisplayName(String& Value){
	Changed(FALSE);
}

IMPL_DYN_CLASS(CCollection)
CCollection::CCollection(CCollectionItemClass* ItemClass) : CComponent(NULL),
	UpdateCount(0),
	NextID(0),
	PropName(NULL){
	this->ItemClass = ItemClass;
	Items = new CList();
	//TODO NotifyDesigner(this, this, opInsert);
}

CCollection::~CCollection(){
	UpdateCount = 1;
	if(Items != NULL)
		Clear();
	//TODO NotifyDesigner(this, this, opRemove);
	delete Items;
}
	
String CCollection::GetPropName(){
	return String(NULL); //TODO
}

void CCollection::InsertItem(CCollectionItem* Item){
	if(!(Item != NULL && Item->InstanceOf(ItemClass)))
		throw "invalid property.";//TList.Error(@SInvalidProperty, 0);
	Items->Add(Item);
	Item->Collection = this;
	Item->ID = NextID;
	NextID++;
	SetItemName(Item);
	Notify(Item, cnAdded);
	Changed();
	//TODO NotifyDesigner(this, Item, opInsert);
}

void CCollection::RemoveItem(CCollectionItem* Item){
	Notify(Item, cnExtracting);
	if(Item == Items->Last())
		Items->Delete(Items->GetCount() - 1);
	else
		Items->Remove(Item);
	Item->Collection = NULL;
	//TODO NotifyDesigner(this, Item, opRemove);
	Changed();
}

void CCollection::Added(CCollectionItem* Item){
}

void CCollection::Deleting(CCollectionItem* Item){
}

void CCollection::Notify(CCollectionItem* Item, TCollectionNotification Action){\
	if(Action == cnAdded)
		Added(Item);
	else if(Action == cnDeleting)
		Deleting(Item);
}

//Design-time editor support
INT CCollection::GetAttrCount(){
	return 0;
}

String CCollection::GetAttr(INT Index){
	return String(TEXT(""));
}

String CCollection::GetItemAttr(INT Index, INT ItemIndex){
	return ((CCollectionItem *)Items->Get(ItemIndex))->GetDisplayName();
}

void CCollection::Changed(){
	if(UpdateCount == 0)
		Update(NULL);
}

CCollectionItem* CCollection::GetItem(INT Index){
	return (CCollectionItem*)Items->Get(Index);
}

void CCollection::SetItem(INT Index, CCollectionItem* Value){
	((CCollectionItem *)Items->Get(Index))->Assign(Value);
}

void CCollection::SetItemName(CCollectionItem* Item){
}

void CCollection::Update(CCollectionItem* Item){
}

void CCollection::SetPropName(String& Value){
	
}

CComponent* CCollection::Owner(){
	return GetOwner();
}

CCollectionItem* CCollection::Add(){
	CCollectionItem* Result = (CCollectionItem*)ItemClass->NewInstance();
	Result->SetCollection(this);
	Added(Result);
	return Result;
}

void CCollection::Assign(CCollection* Source){
	if(Source != NULL && Source->InstanceOf(CCollection::_Class)){
		CMethodLock lock(this, (TLockMethod)&CCollection::BeginUpdate, (TLockMethod)&CCollection::EndUpdate);
		Clear();
		for(INT I = 0; I < ((CCollection *)Source)->GetCount(); I++)
			Add()->Assign(((CCollection *)Source)->GetItem(I));
		return ;
	}
	//TODO__super::Assign(Source);
}

void CCollection::BeginUpdate(){
	UpdateCount++;
}

void CCollection::Clear(){
	if(Items->GetCount() > 0){
		CMethodLock lock(this, (TLockMethod)&CCollection::BeginUpdate,(TLockMethod)&CCollection::EndUpdate);
		while(Items->GetCount() > 0)
			delete ((CCollectionItem *)Items->Last());
	}
}

void CCollection::Delete(INT Index){
	Notify(((CCollectionItem *)Items->Get(Index)), cnDeleting);
	delete ((CCollectionItem *)Items->Get(Index));
}

void CCollection::EndUpdate(){
	UpdateCount--;
	Changed();
}

CCollectionItem* CCollection::FindItemID(INT ID){
	CCollectionItem* Result = NULL;
	for(INT I = 0; I < Items->GetCount(); I++){
		Result = ((CCollectionItem *)Items->Get(I));
		if(Result->GetID() == ID)
			return Result;
	}
	return NULL;
}

String CCollection::GetNamePath(){
	String Result = String(GetClass()->GetName());
	if(GetOwner() == NULL) 
		return Result;
	String S = String(NULL);//TODO GetOwner()->GetNamePath();
	if(S.Length()==0)
		return Result;
	String P = GetPropName();
	if(P.Length()==0)
		return Result;
	return S + '.' + P;
}// override;

CCollectionItem* CCollection::Insert(INT Index){
	CCollectionItem* Result = Add();
	Result->SetIndex(Index);
	return Result;
}

INT CCollection::GetCount(){
	return Items->GetCount();
}
