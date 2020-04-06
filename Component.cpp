
#include "stdinc.h"
#include "Component.hpp"

IMPL_DYN_CLASS(CComponent)

CComponent::CComponent(CComponent* AOwner):Owner(AOwner),
	Components(NULL),
	FreeNotifies(NULL),
	ComponentState(0),
	Name(NULL){
  //ComponentStyle = csInheritable;
  if(AOwner != NULL)
	  AOwner->InsertComponent(this);
}

CComponent::~CComponent(){
	Destroying();
	if(FreeNotifies != NULL){
		while(FreeNotifies->GetCount() > 0){
			((CComponent *)(FreeNotifies->Get(FreeNotifies->GetCount()-1)))->Notification(this, opRemove);
		}
		FreeAndNil((CObject **)&FreeNotifies);
	}
	DestroyComponents();
	if(Owner != NULL)
		Owner->RemoveComponent(this);
}

void CComponent::Remove(CComponent* AComponent){
	AComponent->Owner = NULL;
	Components->Remove(AComponent);
	if(Components->GetCount() == 0){
		delete Components;
		Components = NULL;
	}
}

void CComponent::Insert(CComponent* AComponent){
	if(Components == NULL)
		Components = new CList();
	Components->Add(AComponent);
	AComponent->Owner = this;
}

void CComponent::DestroyComponents(){
  while(Components != NULL){
		CComponent* Instance = (CComponent*)Components->Last();
		if(IN_TEST(csFreeNotification, Instance->ComponentState) ||
			IN_TEST((csDesigning | csInline), ComponentState))
			RemoveComponent(Instance);
		else
			Remove(Instance);
		delete Instance;
  }
}

void CComponent::Destroying(){
	if(!IN_TEST(csDestroying, ComponentState)){
		ComponentState |= csDestroying;
		if(Components != NULL){
			INT Count = Components->GetCount();
			for(INT I = 0; I < Count; I++)
				((CComponent *)Components->Get(I))->Destroying();
		}
	}
}

void CComponent::Notification(CComponent* AComponent, TOperation Operation){
	if(Operation == opRemove && AComponent != NULL)
		RemoveFreeNotification(AComponent);
	if(Components != NULL){
		INT I = Components->GetCount() - 1;
		while(I >= 0){
			((CComponent *)(Components->Get(I)))->Notification(AComponent, Operation);
			I--;
			if(I >= Components->GetCount())
				I = Components->GetCount() - 1;
		}
	}
}

void CComponent::Loaded(){
	ComponentState &= ~csLoading;
}

void CComponent::SetChildOrder(CComponent* Child, INT Order){
}

CComponent* CComponent::GetOwner(){
	return Owner;
}

UINT CComponent::GetComponentState(){
	return ComponentState;
}

void CComponent::FreeNotification(CComponent* AComponent){
	if(Owner == NULL || AComponent->GetOwner() != Owner){
		// Never acquire a reference to a component that is being deleted.
		//assert(not (csDestroying in (ComponentState + AComponent.ComponentState)));
		if (FreeNotifies == NULL)
			FreeNotifies = new CList();
		if (FreeNotifies->IndexOf(AComponent) < 0){
			FreeNotifies->Add(AComponent);
			AComponent->FreeNotification(this);
		}
	}
	ComponentState |= csFreeNotification;
}

void CComponent::RemoveNotification(CComponent* AComponent){
	if(FreeNotifies != NULL){
		FreeNotifies->Remove(AComponent);
		if(FreeNotifies->GetCount() == 0){
			delete FreeNotifies;
			FreeNotifies = NULL;
		}
	}
}

void CComponent::RemoveFreeNotification(CComponent* AComponent){
	RemoveNotification(AComponent);
	AComponent->RemoveNotification(this);
}

void CComponent::SetReference(BOOL Enable){
}

void CComponent::ValidateContainer(CComponent* AComponent){
	AComponent->ValidateInsert(this);
}

void CComponent::ValidateInsert(CComponent* AComponent){
}

void CComponent::InsertComponent(CComponent* AComponent){
	AComponent->ValidateContainer(this);
	ValidateRename(AComponent, TEXT(""), AComponent->Name);
	Insert(AComponent);
	AComponent->SetReference(TRUE);
	//if(IN_TEST(csDesigning, ComponentState))
	//	AComponent->SetDesigning(TRUE);
	Notification(AComponent, opInsert);
}

void CComponent::RemoveComponent(CComponent* AComponent){
	ValidateRename(AComponent, AComponent->Name, TEXT(""));
	Notification(AComponent, opRemove);
	AComponent->SetReference(FALSE);
	Remove(AComponent);
}

void CComponent::ValidateRename(CComponent* AComponent, const LPTSTR CurName, LPTSTR NewName){
	if(AComponent != NULL && lstrcmpi(CurName, NewName) != 0 &&
		AComponent->Owner == this && FindComponent(NewName) != NULL)
		throw "component name duplication.";//EComponentError.CreateResFmt(@SDuplicateName, [NewName]);
	if(IN_TEST(csDesigning, ComponentState) && Owner != NULL)
		Owner->ValidateRename(AComponent, CurName, NewName);
}

CComponent* CComponent::FindComponent(const LPTSTR AName){
	CComponent* Result = NULL;
	if(AName != NULL && Components != NULL){
		INT Count = Components->GetCount();
		for(INT I = 0; I < Count; I++){
			Result = (CComponent*)Components->Get(I);
			if(lstrcmpi(Result->Name, AName) == 0)
				break;
			Result = NULL;
		}
	}
	return Result;
}

CComponent* CComponent::GetComponent(INT Index){
	if(Components == NULL)
		CList::Error(TEXT("list index error."), Index);
	return (CComponent *)Components->Get(Index);
}

INT CComponent::GetComponentCount(){
	return Components != NULL ? Components->GetCount() : 0;
}


#define BitsPerInt	(sizeof(DWORD) * 8)
typedef DWORD TBitArray[4096];
typedef TBitArray *PBitArray;

IMPL_DYN_CLASS(CBits)
CBits::CBits():Size(0), Bits(NULL){
}
CBits::~CBits(){
	SetSize(0);
}

void CBits::Error(){
	throw "bits index error.";//raise EBitsError.CreateRes(@SBitsIndexError);
}

INT CBits::OpenBit(){
	INT E = (GetSize() + BitsPerInt - 1) / BitsPerInt;
	for(INT I = 0; I < E; I++){
		if((*((PBitArray)Bits))[I] != 0xffffffff){
			DWORD B = (*((PBitArray)Bits))[I];
			for(INT J = 0; J < BitsPerInt; J++){
				if(!IN_TEST(1 << J, B)){
					INT Result = I * BitsPerInt + J;
					if(Result >= GetSize())
						Result = GetSize();
					return Result;
				}
			}
		}
	}
	return GetSize();
}

void CBits::SetSize(INT Value){
	if(Value != GetSize()){
		if(Value < 0)
			Error();
		INT NewMemSize = ((Value + BitsPerInt - 1) / BitsPerInt) * sizeof(DWORD);
		INT OldMemSize = ((GetSize() + BitsPerInt - 1) / BitsPerInt) * sizeof(DWORD);
		if(NewMemSize != OldMemSize){
			LPVOID NewMem = NULL;
			if(NewMemSize != 0){
				NewMem = malloc(NewMemSize);
				ZeroMemory(NewMem, NewMemSize);
			}
			if(OldMemSize != 0){
				if(NewMem != NULL)
					CopyMemory(NewMem, Bits, OldMemSize > NewMemSize ? NewMemSize : OldMemSize);
				free(Bits);
			}
			Bits = NewMem;
		}
		Size = Value;
	}
}

void CBits::SetBit(INT Index, BOOL Value){
	if(Index >= Size || Index < 0)
		Error();
	else{
		INT I = Index / BitsPerInt;
		INT J = Index % BitsPerInt;
		DWORD *pB = ((DWORD *)Bits) + I;
		if(Value)
			*pB = *pB | (1 << J);
		else
			*pB = *pB & (~(1 << J));
	}
}

BOOL CBits::GetBit(INT Index){
	if(Index >= Size || Index < 0)
		Error();
	else{
		INT I = Index / BitsPerInt;
		INT J = Index % BitsPerInt;
		DWORD B = (*((PBitArray)Bits))[I];
		return IN_TEST(1 << J, B);
	}
	return FALSE;
}
