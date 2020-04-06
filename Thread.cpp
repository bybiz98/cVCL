
#include "stdinc.h"
#include "Thread.hpp"


INT CALLBACK CThread::ThreadWrapper(LPVOID Parameter){
	CThread* Thread = (CThread *)Parameter;
	return Thread->Run();
}

IMPL_DYN_CLASS(CThread)
CThread::CThread(CComponent* AOwner):CComponent(AOwner),
	ThreadId(0),
	ThreadHandle(0),
	Data(NULL),
	FreeOnTerminate(TRUE),
	INIT_EVENT(Run){
	ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ThreadWrapper, (LPVOID)this, CREATE_SUSPENDED, &ThreadId);
}

CThread::~CThread(){
	Stop();
}

INT CThread::Run(){
	if(OnRun != NULL){
		return CALL_EVENT(Run)(this);
	}
	return 0;
}

void CThread::Start(){
	ResumeThread(ThreadHandle);
}

void CThread::Stop(){
	TerminateThread(ThreadHandle, 1);
}
