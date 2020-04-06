
#include "stdinc.h"
#include "Object.hpp"

CObjectClass* CObject::GetClass(){
	return &CObjectClass::_CObjectClass;
}

void CObject::Assign(CObject* Source){

}

BOOL CObject::InstanceOf(CObjectClass* Class){
	CObjectClass* _thisClass = GetClass();
	while(_thisClass != NULL && _thisClass != Class){
		_thisClass = _thisClass->GetParent();
	}
	return _thisClass == Class;
}

CObjectClass CObjectClass::_CObjectClass;
CObjectClass::CObjectClass(){
	Name = TEXT("CObject");
	Parent = NULL;
	InstanceSize = sizeof(CObject);
}

CObjectClass::~CObjectClass(){
}

CObjectClass* CObjectClass::GetParent(){
	return Parent;
}

LPTSTR CObjectClass::GetName(){
	return Name;
}

size_t CObjectClass::GetInstanceSize(){
	return InstanceSize;
}

CObject* CObjectClass::NewInstance(){
	return new CObject();
}

BOOL CObjectClass::operator==(const CObjectClass* pClass){
	return this == pClass;
}

BOOL CObjectClass::operator!=(const CObjectClass* pClass){
	return this != pClass;
}

void FreeAndNil(CObject** obj){
	delete *obj;
	*obj = NULL;
}

WORD GetHashCode(LPVOID Buffer, INT Count){
	WORD Result = 0;
	LPBYTE buf = (LPBYTE)Buffer;
	for(; Count > 0; Count--, buf++){
		Result = (Result & 0xFF00) | (((BYTE)Result) ^ *buf);
		Result = (Result << 5) | (Result >> 11);
	}
	return Result;
}

/*
template <typename InterfaceType>
CInterfaceObject<InterfaceType>::CInterfaceObject(REFIID AIID):_IID(AIID),RefCount(0){
}
template <typename InterfaceType>
CInterfaceObject<InterfaceType>::~CInterfaceObject(){
}
template <typename InterfaceType>
HRESULT STDMETHODCALLTYPE CInterfaceObject<InterfaceType>::QueryInterface(REFIID riid, void **ppvObject){
	if(ppv == 0)
        return E_POINTER;

    *ppv = NULL;
	if (IsEqualIID(riid, IID_IUnknown)){
        *ppv = static_cast<IUnknown *>(this);
    }
	else if (IsEqualIID(riid, _IID)){
		*ppv = static_cast<InterfaceType *>(this);
	}
	else {
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}
template <typename InterfaceType>
ULONG STDMETHODCALLTYPE CInterfaceObject<InterfaceType>::AddRef(){
	return InterlockedIncrement(&RefCount);
}
template <typename InterfaceType>
ULONG STDMETHODCALLTYPE CInterfaceObject<InterfaceType>::Release(){
	ULONG r = InterlockedDecrement(&RefCount);
	if(r > 0)
		return r;
	delete this;
	return 0;
}
//*/