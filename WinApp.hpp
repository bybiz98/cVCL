#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Control.hpp"

typedef void (CObject::*TMessageEvent)(MSG& Msg, BOOL& Handled);
typedef void (CObject::*TIdleEvent)(CObject* Sender, BOOL& Done);

class cVCL_API CWinApp: public CComponent{
private:
	CControl* MouseControl;
	BOOL Running;
	BOOL Terminated;

	BOOL ProcessMessage(MSG& Msg);
	CControl* DoMouseIdle();
	void DoActionIdle();
protected:
	void Idle(const MSG& Msg);
	BOOL IsKeyMsg(MSG& Msg);
public:
	DECLARE_TYPE_EVENT(TMessageEvent, Message)
	DECLARE_TYPE_EVENT(TIdleEvent, Idle)
public:
    CWinApp(CComponent* AOwner = NULL);
	virtual ~CWinApp();

	void Run();
	void HandleException(CObject* Sender);
	void HandleMessage();
	
	REF_DYN_CLASS(CWinApp)
};
DECLARE_DYN_CLASS(CWinApp, CComponent)