#pragma once

#include "Object.hpp"
#include "List.hpp"


//Component State
#define csLoading			0x001
#define csReading			0x002
#define csWriting			0x004
#define csDestroying		0x008
#define csDesigning			0x010
#define csAncestor			0x020
#define csUpdating			0x040
#define csFixups			0x080
#define csFreeNotification	0x100
#define	csInline			0x200
#define csDesignInstance	0x400

//Component Style
#define csInheritable		0x01
#define csCheckPropAvail	0x02
#define csSubComponent		0x04
#define csTransient			0x08

//operation code
typedef BYTE TOperation;
#define opInsert			0x1
#define opRemove			0x2

class cVCL_API CComponent: public CObject{
private:
	CComponent* Owner;
	CList* Components;
	CList* FreeNotifies;
	UINT ComponentState;
	LPTSTR Name;
	LPVOID Tag;

	void Remove(CComponent* AComponent);
	void Insert(CComponent* AComponent);
	void RemoveNotification(CComponent* AComponent);
	void SetReference(BOOL Enable);
protected:
	virtual void Notification(CComponent* AComponent, TOperation Operation);
	virtual void Loaded();
	virtual void ValidateRename(CComponent* AComponent, const LPTSTR CurName, LPTSTR NewName);
	virtual void ValidateContainer(CComponent* AComponent);
    virtual void ValidateInsert(CComponent* AComponent);
	virtual void SetChildOrder(CComponent* Child, INT Order);
public:
    CComponent(CComponent* AOwner = NULL);
	virtual ~CComponent();
	virtual CComponent* GetOwner();
	UINT GetComponentState();

	void DestroyComponents();
    void Destroying();
	void FreeNotification(CComponent* AComponent);
	void RemoveFreeNotification(CComponent* AComponent);

	void InsertComponent(CComponent* AComponent);
    void RemoveComponent(CComponent* AComponent);
	CComponent* FindComponent(const LPTSTR AName);

	DEFINE_ACCESSOR(LPVOID, Tag)

	CComponent* GetComponent(INT Index);
	INT GetComponentCount();

	REF_DYN_CLASS(CComponent)
};
DECLARE_DYN_CLASS(CComponent, CObject)

class cVCL_API CBits : public CObject{
private:
	INT Size;
	LPVOID Bits;
	void Error();
public:
	CBits();
	virtual ~CBits();
    INT OpenBit();
	DEFINE_GETTER(INT, Size)
	void SetSize(INT Value);
	void SetBit(INT Index, BOOL Value);
	BOOL GetBit(INT Index);

	REF_DYN_CLASS(CBits)
};
DECLARE_DYN_CLASS(CBits, CObject)