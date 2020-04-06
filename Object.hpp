#pragma once

#include "stdinc.h"

class CObjectClass;

class cVCL_API CObject{
public:
	CObject(){};
	virtual ~CObject(){};
	virtual CObjectClass* GetClass();
	virtual void Assign(CObject* Source);
	BOOL InstanceOf(CObjectClass* Class);
};

template <typename InterfaceType>
class cVCL_API CInterfaceObject : public InterfaceType{
private:
	REFIID _IID;
	ULONG RefCount;
public:
	CInterfaceObject(REFIID AIID):_IID(AIID),RefCount(0){
	};
	virtual ~CInterfaceObject(){};
	inline ULONG GetRefCount(){return RefCount;};
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv){
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
	};
	virtual ULONG STDMETHODCALLTYPE AddRef(){
		return InterlockedIncrement(&RefCount);
	};
	virtual ULONG STDMETHODCALLTYPE Release(){
		ULONG r = InterlockedDecrement(&RefCount);
		if(r > 0)
			return r;
		delete this;
		return 0;
	};
};

class cVCL_API CObjectClass{
protected:
	LPTSTR Name;
	CObjectClass* Parent;
	size_t InstanceSize;
public:
	static CObjectClass _CObjectClass;
	CObjectClass();
	virtual ~CObjectClass();
	CObjectClass* GetParent();
	LPTSTR GetName();
	size_t GetInstanceSize();
	virtual CObject* NewInstance();
	BOOL operator==(const CObjectClass* pClass);
	BOOL operator!=(const CObjectClass* pClass);
};

void FreeAndNil(CObject** obj);
WORD GetHashCode(LPVOID Buffer, INT Count);

typedef void (CObject::*TNotifyEvent)(CObject* Sender);

#define IN_TEST(BIT, SET) ((SET & BIT) == BIT)

#define DECLARE_DYN_CLASS_BASE(THIS, BASE) class THIS##Class: public BASE##Class{\
public:\
	THIS##Class(){\
		Name=TEXT(#THIS);\
		Parent = &BASE##Class::_##BASE##Class;\
		InstanceSize = sizeof(THIS);\
	};\
	static THIS##Class _##THIS##Class;

#define DECLARE_DYN_CLASS(THIS, BASE) DECLARE_DYN_CLASS_BASE(THIS, BASE)\
		CObject* NewInstance() override { return new THIS();};\
};

#define DECLARE_DYN_CLASS_ABSTRACT(THIS, BASE) DECLARE_DYN_CLASS_BASE(THIS, BASE)\
		CObject* NewInstance() override { return NULL; };\
};

#define REF_DYN_CLASS(THIS) static CObjectClass* _Class;\
	CObjectClass* GetClass() override{\
		return THIS::_Class; \
	};

#define IMPL_DYN_CLASS(THIS) THIS##Class THIS##Class::_##THIS##Class;\
CObjectClass* THIS::_Class = &THIS##Class::_##THIS##Class;

#define DECLARE_TYPE_EVENT(TYPE, EVENT) private:\
	CObject* _##EVENT##Obj;\
	TYPE On##EVENT##;\
	public:\
	inline VOID SetOn##EVENT##(CObject* _Obj, TYPE _Event){\
		this->_##EVENT##Obj = _Obj;\
		this->On##EVENT## = _Event;\
	};\
	inline TYPE GetOn##EVENT##(){\
		return this->On##EVENT##;\
	};\
	inline CObject* GetOn##EVENT##Obj(){\
		return this->_##EVENT##Obj;\
	};

#define INIT_EVENT(EVENT) _##EVENT##Obj(NULL),On##EVENT##(NULL)

#define DECLARE_EVENT(EVENT) DECLARE_TYPE_EVENT(T##EVENT##Event, EVENT)

#define CALL_EVENT(EVENT) ((this->_##EVENT##Obj)->*(this->On##EVENT##))

#define CALL_EVENT_EXTERNAL(OBJ, EVENT) ((##OBJ##->GetOn##EVENT##Obj())->*(##OBJ##->GetOn##EVENT##()))

#define DEFINE_GETTER(TYPE, FIELD) inline TYPE Get##FIELD##(){\
		return this->##FIELD##;\
	};

#define DEFINE_SETTER(TYPE, FIELD) inline VOID Set##FIELD##(TYPE _Value){\
		this->##FIELD## = _Value;\
	};

#define DEFINE_ACCESSOR(TYPE, FIELD) DEFINE_GETTER(TYPE, FIELD)\
	DEFINE_SETTER(TYPE, FIELD)