#include "stdinc.h"
#include "ObjWndProc.hpp"

#if defined(_WIN64)
EXTERN_C LONG_PTR objWndProcX64StartAddr();
EXTERN_C LONG_PTR objWndProcX64Length();
EXTERN_C LONG_PTR objWndProcX64ThisAddr();
EXTERN_C LONG_PTR objWndProcX64OffsetAddr();
#define objWndProcStartAddr objWndProcX64StartAddr
#define objWndProcLength objWndProcX64Length
#define objWndProcThisAddr objWndProcX64ThisAddr
#define objWndProcOffsetAddr objWndProcX64OffsetAddr
#else
EXTERN_C LONG_PTR objWndProcX86StartAddr();
EXTERN_C LONG_PTR objWndProcX86Length();
EXTERN_C LONG_PTR objWndProcX86ThisAddr();
EXTERN_C LONG_PTR objWndProcX86OffsetAddr();
#define objWndProcStartAddr objWndProcX86StartAddr
#define objWndProcLength objWndProcX86Length
#define objWndProcThisAddr objWndProcX86ThisAddr
#define objWndProcOffsetAddr objWndProcX86OffsetAddr
#endif

#pragma pack (1)
	typedef struct ObjectWndMethod *PObjectWndMethod;
	typedef struct ObjectWndMethod{
		PObjectWndMethod Next;
		BYTE WndProc[1];//
	} TObjectWndMethod, *PObjectWndMethod;

	typedef struct MemPool *PMemPool;
	typedef struct MemPool{
		PMemPool Next;
	} TMemPool, *PMemPool;
#pragma pack ()

#define PAGE_SIZE 4096

PObjectWndMethod pFreeList = NULL;
PObjectWndMethod pAllocList = NULL;
PMemPool pMemPool = NULL;


LPVOID handler_cast(TObjectWndProc objWndProc){
	LPVOID *Addr = (LPVOID *)&objWndProc;
	return *Addr;
}

VOID FillObjectWndProc(LPVOID StartAddr, LPVOID Object, TObjectWndProc ObjectWndProcAddr){
	LPVOID addr = (LPVOID)objWndProcStartAddr();
	UINT len = (UINT)objWndProcLength();
	CopyMemory(StartAddr, addr, len);
	LONG_PTR thisAddr = (LONG_PTR)objWndProcThisAddr();
	*((LPVOID *)((LONG_PTR)StartAddr + (thisAddr - (LONG_PTR)addr))) = Object;
	LONG_PTR offsetAddr = (LONG_PTR)objWndProcOffsetAddr();
	*((LPVOID *)((LONG_PTR)StartAddr + (offsetAddr - (LONG_PTR)addr))) = handler_cast(ObjectWndProcAddr);
}

LPVOID MakeObjectWndProc(LPVOID Object, TObjectWndProc ObjectWndProcAddr){
	if(pFreeList == NULL){
		LPVOID pMem = VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		PMemPool pPool = (PMemPool)pMem;
		pPool->Next= pMemPool;
		pMemPool = pPool;
		pMem = LPVOID((LONG_PTR)pMem + sizeof(PMemPool));

		UINT Size = (UINT)objWndProcLength() + sizeof(PObjectWndMethod);
		UINT Count = PAGE_SIZE - sizeof(PMemPool);
		LPVOID pNext = pFreeList;
		while(Count > Size){
			((PObjectWndMethod)pMem)->Next = (PObjectWndMethod)pNext;
			pNext = pMem;
			pMem = LPVOID((LONG_PTR)pMem + Size);
			Count = Count - Size;
		}
		pFreeList = (PObjectWndMethod)pNext;
	}
	PObjectWndMethod pWndProc = pFreeList;
	pFreeList = pWndProc->Next;
	pWndProc->Next = pAllocList;
	pAllocList = pWndProc;
	FillObjectWndProc(&(pWndProc->WndProc), Object, ObjectWndProcAddr);
	return &(pWndProc->WndProc);
}

VOID FreeObjectWndProc(LPVOID ObjectWndProc){
	PObjectWndMethod pWndProc = (PObjectWndMethod)((LONG_PTR)ObjectWndProc - sizeof(PObjectWndMethod));
	if(pAllocList == pWndProc){
		pAllocList = pWndProc->Next;
	}
	else{
		PObjectWndMethod pFind = pAllocList;
		while(pFind != NULL && pFind->Next != pWndProc){
			pFind = pFind->Next;
		}
		if(pFind != NULL && pFind->Next == pWndProc){
			pFind->Next = pWndProc->Next;
		}
		else{
			//not found, do nothing
			return ;
		}
	}
	pWndProc->Next = pFreeList;
	pFreeList = pWndProc;
}


WNDCLASS UtilWindowClass = {
    0,//style
    &DefWindowProc, //lpfnWndProc
    0, //cbClsExtra
    0, //cbWndExtra
    0, //hInstance
    0, //hIcon
    0, //hCursor
    0, //hbrBackground
	NULL, //lpszMenuName
	TEXT("CVHideWindow") //lpszClassName
};

HWND AllocateHWnd(HINSTANCE HInstance, LPVOID Object, TObjectWndProc ObjectWndProcAddr){
	WNDCLASS TempClass;
	UtilWindowClass.hInstance = HInstance;
	BOOL ClassRegistered = GetClassInfo(HInstance, UtilWindowClass.lpszClassName,
		&TempClass);
	if(!ClassRegistered || TempClass.lpfnWndProc != &DefWindowProc){
		if(ClassRegistered)
			::UnregisterClass(UtilWindowClass.lpszClassName, HInstance);
		::RegisterClass(&UtilWindowClass);
	}
	HWND Result = CreateWindowEx(WS_EX_TOOLWINDOW, UtilWindowClass.lpszClassName,
		TEXT(""), WS_POPUP /*!0*/, 0, 0, 0, 0, (HWND)0, (HMENU)0, HInstance, NULL);
	if(Object != NULL && ObjectWndProcAddr != NULL){
		SetWindowLongPtr(Result, GWLP_WNDPROC, (LONG_PTR)MakeObjectWndProc(Object, ObjectWndProcAddr));
	}
	return Result;
}

void DeallocateHWnd(HWND Wnd){
	LPVOID Instance = (LPVOID)GetWindowLongPtr(Wnd, GWLP_WNDPROC);
	DestroyWindow(Wnd);
	if(Instance != &DefWindowProc)
		FreeObjectWndProc(Instance);
}
