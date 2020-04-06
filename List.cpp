
#include "stdinc.h"
#include "List.hpp"
#include "WinUtils.hpp"


IMPL_DYN_CLASS(CList)
CList::CList():
	List(NULL),
	Count(0),
	Capacity(0){
}

void CList::Grow(){
	INT Delta;
	if(Capacity > 64)
		Delta = Capacity / 4;
	else if(Capacity > 8)
		Delta = 16;
	else Delta = 4;
	SetCapacity(Capacity + Delta);
}

void CList::Notify(LPVOID Ptr, TListNotification Action){

}

CList::~CList(){
	Clear();
}

INT CList::Add(LPVOID Item){
	INT Result = Count;
	if(Result == Capacity)
		Grow();
	(*List)[Result] = Item;
	Count++;
	if(Item != NULL)
		Notify(Item, lnAdded);
	return Result;
}

void CList::Clear(){
	SetCount(0);
	SetCapacity(0);
}

LPVOID CList::Delete(INT Index){
	LPVOID Temp;
	if((Index < 0) || (Index >= Count))
		Error(TEXT("List index out of bounds (%d))"), Index);
	Temp = Get(Index);
	Count--;
	if(Index < Count)
		CopyMemory((LPVOID *)List + Index, (LPVOID *)List + Index + 1, (Count - Index) * sizeof(LPVOID));
	if(Temp != NULL)
		Notify(Temp, lnDeleted);
	return Temp;
}

void CList::Error(const LPTSTR Msg, INT Data){
	throw Msg;//std::runtime_error(CUnicodeUtils::StdGetAnsi(Msg));
}

void CList::Exchange(INT Index1, INT Index2){
	LPVOID Item;
	if((Index1 < 0) || (Index1 >= Count))
		Error(TEXT("List index out of bounds (%d)"), Index1);
	if((Index2 < 0) || (Index2 >= Count))
		Error(TEXT("List index out of bounds (%d)"), Index2);
	Item = (*List)[Index1];
	(*List)[Index1] = (*List)[Index2];
	(*List)[Index2] = Item;
}

CList* CList::Expand(){
	if(Count == Capacity)
		Grow();
	return this;
}

LPVOID CList::Extract(LPVOID Item){
	LPVOID Result = NULL;
	INT I = IndexOf(Item);
	if (I >= 0){
		Result = Item;
		(*List)[I] = NULL;
		Delete(I);
		Notify(Result, lnExtracted);
	}
	return Result;
}

LPVOID CList::First(){
	return Get(0);
}

INT CList::IndexOf(LPVOID Item){
	INT Result = 0;
	while((Result < Count) && ((*List)[Result] != Item))
		Result++;
	if(Result == Count)
		Result = -1;
	return Result;
}

void CList::Insert(INT Index, LPVOID Item){
	if((Index < 0) || (Index > Count))
		Error(TEXT("List index out of bounds (%d)"), Index);
	if(Count == Capacity)
		Grow();
	if(Index < Count)
		CopyMemory((LPVOID *)List + Index + 1, (LPVOID *)List + Index, (Count - Index) * sizeof(LPVOID));
	(*List)[Index] = Item;
	Count++;
	if(Item != NULL)
		Notify(Item, lnAdded);
}

LPVOID CList::Last(){
	return Get(Count - 1);
}

void CList::Move(INT CurIndex, INT NewIndex){
	LPVOID Item;
	if(CurIndex != NewIndex){
		if((NewIndex < 0) || (NewIndex >= Count))
			Error(TEXT("List index out of bounds (%d)"), NewIndex);
		Item = Get(CurIndex);
		(*List)[CurIndex] = NULL;
		Delete(CurIndex);
		Insert(NewIndex, NULL);
		(*List)[NewIndex] = Item;
	}
}

INT CList::Remove(LPVOID Item){
	INT Result = IndexOf(Item);
	if (Result >= 0)
		Delete(Result);
	return Result;
}

void CList::Pack(){
	for (INT i = Count - 1; i >= 0; i--)
		if((*List)[i] == NULL)
			Delete(i);
}

void QuickSort(PPointerList SortList, INT L, INT R, TListSortCompare SCompare){
	INT i, j;
	LPVOID p, t;
	do{
		i = L;
		j = R;
		p = (*SortList)[(L + R) >> 1];
		do{
			while (SCompare((*SortList)[i], p) < 0)
				i++;
			while (SCompare((*SortList)[j], p) > 0)
				j++;
			if(i <= j){
				t = (*SortList)[i];
				(*SortList)[i] = (*SortList)[j];
				(*SortList)[j] = t;
			}
			i++;
			j++;
		}while(i <= j);
		if(L < j)
			QuickSort(SortList, L, j, SCompare);
		L = i;
	}while(i < R);
}

void CList::Sort(TListSortCompare Compare){
	if ((List != NULL) && (Count > 0))
		QuickSort(List, 0, Count - 1, Compare);
}

void CList::Assign(CList* ListA, TListAssignOp AOperator, CList* ListB){
	INT i;
	CList *LSource;
	// ListB given?
	if(ListB != NULL){
		LSource = ListB;
		Assign(ListA);
	}
	else
		LSource = ListA;
	// on with the show
	switch(AOperator){
		case laCopy: {// 12345, 346 = 346 : only those in the new list
			Clear();
			SetCapacity(LSource->GetCapacity());
			for(i = 0; i < LSource->GetCount(); i++){
				Add(LSource->Get(i));
			}
			break;
		};
		case laAnd: {// 12345, 346 = 34 : intersection of the two lists
			for (i = GetCount() - 1; i >= 0; i--)
				if(LSource->IndexOf(Get(i)) == -1)
					Delete(i);
			break;
		};
		case laOr: {// 12345, 346 = 123456 : union of the two lists
			for (i = 0; i < LSource->GetCount(); i++)
				if(IndexOf(LSource->Get(i)) == -1)
					Add(LSource->Get(i));
			break;
		};
		case laXor: {// 12345, 346 = 1256 : only those not in both lists
		    CList LTemp; // Temp holder of 4 byte values
			LTemp.SetCapacity(LSource->GetCount());
			for(i = 0; i < LSource->GetCount(); i++)
				if (IndexOf(LSource->Get(i)) == -1)
					LTemp.Add(LSource->Get(i));
			for(i = GetCount() - 1; i >= 0; i--)
				if(LSource->IndexOf(Get(i)) != -1)
					Delete(i);
			i = GetCount() + LTemp.GetCount();
			if(GetCapacity() < i)
				SetCapacity(i);
			for(i = 0; i < LTemp.GetCount(); i++)
				Add(LTemp.Get(i));
			break;
		};
		case laSrcUnique: {   // 12345, 346 = 125 : only those unique to source
			for(i = GetCount() - 1; i >= 0; i--)
				if (LSource->IndexOf(Get(i)) != -1)
					Delete(i);
			break;
		};
		case laDestUnique: {// 12345, 346 = 6 : only those unique to dest
			CList LTemp;
			LTemp.SetCapacity(LSource->GetCount());
			for(i = LSource->GetCount(); i >= 0; i++)
				if (IndexOf(LSource->Get(i)) == -1)
					LTemp.Add(LSource->Get(i));
			Assign(&LTemp);
			break;
		};
	}
}

INT CList::GetCapacity(){
	return Capacity;
}

void CList::SetCapacity(INT NewCapacity){
	if((NewCapacity < Count) || (NewCapacity > MAX_LIST_SIZE))
		Error(TEXT("List capacity out of bounds (%d)"), NewCapacity);
	if(NewCapacity != Capacity){
		if(NewCapacity == 0 && List != NULL){
			LPVOID p = List;
			List = NULL;
			free(p);
		}
		else{
			LPVOID ret = List == NULL 
				? malloc(NewCapacity * sizeof(LPVOID))
				: realloc(List, NewCapacity * sizeof(LPVOID));
			if(ret == NULL){
				Error(TEXT("error while alloc memory"), 0);
			}
			List = (PPointerList)ret;
		}
		Capacity = NewCapacity;
	}
}

INT CList::GetCount(){
	return Count;
}

void CList::SetCount(INT NewCount){
	if((NewCount < 0) || (NewCount > MAX_LIST_SIZE))
		Error(TEXT("List count out of bounds (%d)"), NewCount);
	if (NewCount > Capacity)
		SetCapacity(NewCount);
	if(NewCount > Count)
		FillMemory((LPVOID *)List + Count, (NewCount - Count) * sizeof(LPVOID), 0);
	else
		for(INT i = Count - 1; i >= NewCount; i--)
			Delete(i);
	Count = NewCount;
}

LPVOID CList::Get(INT Index){
	if ((Index < 0) || (Index >= Count))
		Error(TEXT("List index out of bounds (%d)"), Index);
	return (*List)[Index];
}

void CList::Put(INT Index, LPVOID Item){
	if((Index < 0) || (Index >= Count))
		Error(TEXT("List index out of bounds (%d)"), Index);
	if(Item != (*List)[Index]){
		LPVOID Temp = (*List)[Index];
		(*List)[Index] = Item;
		if(Temp != NULL)
			Notify(Temp, lnDeleted);
		if(Item != NULL)
			Notify(Item, lnAdded);
	}
}

PPointerList CList::GetList(){
	return List;
}


IMPL_DYN_CLASS(CThreadList)
CThreadList::CThreadList(){
	InitializeCriticalSection(&Lock);
	List = new CList();
	Duplicates = dupIgnore;
}


CThreadList::~CThreadList(){
	{
		CMethodLock ListLock(this, (TLockMethodResult)&CThreadList::LockList, (TLockMethod)&CThreadList::UnlockList);
		delete List;
	}
	DeleteCriticalSection(&Lock);
}

void CThreadList::Add(LPVOID Item){
	CMethodLock ListLock(this, (TLockMethodResult)&CThreadList::LockList, (TLockMethod)&CThreadList::UnlockList);
	if((Duplicates == dupAccept) || (List->IndexOf(Item) == -1))
		List->Add(Item);
	else if(Duplicates == dupError)
		List->Error(TEXT("List does not allow duplicates ($0%x)"), (INT)Item);
}

void CThreadList::Clear(){
	CMethodLock ListLock(this, (TLockMethodResult)&CThreadList::LockList, (TLockMethod)&CThreadList::UnlockList);
	List->Clear();
}

CList* CThreadList::LockList(){
	EnterCriticalSection(&Lock);
	return List;
}

void CThreadList::Remove(LPVOID Item){
	CMethodLock ListLock(this, (TLockMethodResult)&CThreadList::LockList, (TLockMethod)&CThreadList::UnlockList);
	List->Remove(Item);
}

void CThreadList::UnlockList(){
	LeaveCriticalSection(&Lock);
}