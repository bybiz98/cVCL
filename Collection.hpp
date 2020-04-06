#pragma once

#include "Object.hpp"
#include "List.hpp"
#include "Component.hpp"
#include "Strings.hpp"

class CCollection;

class cVCL_API CCollectionItem : public CComponent{
private:
	friend class CCollection;
	CCollection *Collection;
	INT ID;
protected:
	void Changed(BOOL AllItems);
	CComponent* GetOwner() override;
public:
	CCollectionItem(CCollection* Collection = NULL);
	virtual ~CCollectionItem();
	String GetNamePath();// override;

	virtual void SetCollection(CCollection* Value);
	INT GetIndex();
	virtual void SetIndex(INT Value);
    virtual String GetDisplayName();
	virtual void SetDisplayName(String& Value);

	DEFINE_GETTER(CCollection*, Collection)
	DEFINE_GETTER(INT, ID);

	REF_DYN_CLASS(CCollectionItem)
};
DECLARE_DYN_CLASS(CCollectionItem, CComponent)

typedef BYTE TCollectionNotification;
#define cnAdded			0x0
#define cnExtracting	0x1
#define cnDeleting		0x2

class cVCL_API CCollection : public CComponent{
private:
	friend class CCollectionItem;
	CCollectionItemClass* ItemClass;
	CList* Items;
	INT UpdateCount;
	INT NextID;
	String* PropName;
	String GetPropName();
	void InsertItem(CCollectionItem* Item);
	void RemoveItem(CCollectionItem* Item);
protected:
	virtual void Added(CCollectionItem* Item);
	virtual void Deleting(CCollectionItem* Item);

	DEFINE_GETTER(INT, NextID)
	virtual void Notify(CCollectionItem* Item, TCollectionNotification Action);
	//Design-time editor support
	virtual INT GetAttrCount();
	virtual String GetAttr(INT Index);
	virtual String GetItemAttr(INT Index, INT ItemIndex);

	void Changed();
	CCollectionItem* GetItem(INT Index);
	void SetItem(INT Index, CCollectionItem* Value);
	virtual void SetItemName(CCollectionItem* Item);
	virtual void Update(CCollectionItem* Item);
	void SetPropName(String& Value);
	DEFINE_GETTER(INT, UpdateCount);
public:
	CCollection(CCollectionItemClass* ItemClass = NULL);
	virtual ~CCollection();
	CComponent* Owner();
	CCollectionItem* Add();
	void Assign(CCollection* Source);
	virtual void BeginUpdate();
	void Clear();
	void Delete(INT Index);
	virtual void EndUpdate();
	CCollectionItem* FindItemID(INT ID);
	String GetNamePath();// override;
	CCollectionItem* Insert(INT Index);
	INT GetCount();
	
	DEFINE_GETTER(CCollectionItemClass*, ItemClass)

	REF_DYN_CLASS(CCollection)
};
DECLARE_DYN_CLASS(CCollection, CComponent)