#pragma once

#include "Object.hpp"
#include "Component.hpp"

typedef INT (CObject::*TThreadMethod)(CObject *Sender);

class cVCL_API CThread : public CComponent{
private:
	DWORD ThreadId;
	HANDLE ThreadHandle;
	LPVOID Data;
	BOOL FreeOnTerminate;
	static INT CALLBACK ThreadWrapper(LPVOID Parameter);
protected:
	virtual INT Run();
public:
	DECLARE_TYPE_EVENT(TThreadMethod, Run)
public:
	CThread(CComponent *AOwner = NULL);
	virtual ~CThread();
	void Start();
	void Stop();

	DEFINE_GETTER(DWORD, ThreadId)
	DEFINE_GETTER(HANDLE, ThreadHandle)
	DEFINE_ACCESSOR(LPVOID, Data)
	DEFINE_ACCESSOR(BOOL, FreeOnTerminate)

	REF_DYN_CLASS(CThread)
};
DECLARE_DYN_CLASS(CThread, CComponent)