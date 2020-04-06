#pragma once

#include "Object.hpp"

#define MAX_LIST_SIZE		(INT_MAX/16)
typedef LPVOID TPointerList[MAX_LIST_SIZE];
typedef TPointerList *PPointerList;

typedef INT TListSortCompare(LPVOID Item1, LPVOID Item2);

typedef BYTE TListNotification;
#define lnAdded			0x01
#define lnExtracted		0x02
#define lnDeleted		0x04
// these operators are used in Assign and go beyond simply copying
//   laCopy = dest becomes a copy of the source
//   laAnd  = intersection of the two lists
//   laOr   = union of the two lists
//   laXor  = only those not in both lists
// the last two operators can actually be thought of as binary operators but
// their implementation has been optimized over their binary equivalent.
//   laSrcUnique  = only those unique to source (same as laAnd followed by laXor)
//   laDestUnique = only those unique to dest   (same as laOr followed by laXor)
typedef BYTE TListAssignOp;
#define laCopy			0x01
#define laAnd			0x02
#define laOr			0x04
#define laXor			0x08
#define laSrcUnique		0x10
#define laDestUnique	0x20

class cVCL_API CList : public CObject{
private:
	PPointerList List;
	INT Count;
	INT Capacity;
protected:
    virtual void Grow();
	virtual void Notify(LPVOID Ptr, TListNotification Action);
public:
	CList();
	virtual ~CList();
	INT Add(LPVOID Item);
	virtual void Clear();
	LPVOID Delete(INT Index);
	static void Error(const LPTSTR Msg, INT Data);
	void Exchange(INT Index1, INT Index2);
	CList* Expand();
	LPVOID Extract(LPVOID Item);
	LPVOID First();
	INT IndexOf(LPVOID Item);
	void Insert(INT Index, LPVOID Item);
	LPVOID Last();
	void Move(INT CurIndex, INT NewIndex);
	INT Remove(LPVOID Item);
	void Pack();
	void Sort(TListSortCompare Compare);
	void Assign(CList* ListA, TListAssignOp AOperator = laCopy, CList* ListB = NULL);
	INT GetCapacity();
	void SetCapacity(INT NewCapacity);
	INT GetCount();
	void SetCount(INT NewCount);
    LPVOID Get(INT Index);
	void Put(INT Index, LPVOID Item);
    PPointerList GetList();

	REF_DYN_CLASS(CList)
};
DECLARE_DYN_CLASS(CList, CObject)

typedef BYTE TDuplicates;
#define dupIgnore			0x00
#define dupAccept			0x01
#define dupError			0x02

class cVCL_API CThreadList : public CObject{
private:
    CList *List;
	RTL_CRITICAL_SECTION Lock;
	TDuplicates Duplicates;
public:
    CThreadList();
	virtual ~CThreadList();
	void Add(LPVOID Item);
	void Clear();
	CList* LockList();
	void Remove(LPVOID Item);
	void UnlockList();
	DEFINE_ACCESSOR(TDuplicates, Duplicates);

	REF_DYN_CLASS(CThreadList)
};
DECLARE_DYN_CLASS(CThreadList, CObject)