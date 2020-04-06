#pragma once

#include "Component.hpp"
#include "List.hpp"
#include "Messages.hpp"

typedef void (CObject::*MsgHandler)(TMessage &);   

typedef struct{
	UINT Msg;
	MsgHandler pfHandler;
} MsgMapping;

#define MSG_MAP_BEGIN() \
	MsgMapping* GetMsgMapping(){\
	static MsgMapping _MsgMapping[] = {

#define MSG_MAP_ENTRY(Msg, Handler) {Msg, (MsgHandler)&Handler},

#define MSG_MAP_END() \
	{WM_NULL, NULL}\
    };\
    return &_MsgMapping[0];\
    };\
	void Dispatch(TMessage& Msg) override {\
		BOOL Dispatched = _Dispatch(Msg, GetMsgMapping());\
		if(!Dispatched) __super::Dispatch(Msg);\
    };\
	void InheritedMsg(TMessage& Msg){\
		__super::Dispatch(Msg);\
    };
#define IS_LAST_MSG_MAP(_Mapping) (_Mapping->Msg == WM_NULL && _Mapping->pfHandler == NULL)
#define INHERITED_MSG(Msg) InheritedMsg(*((PMessage)&(Msg)))

class cVCL_API CMsgTarget : public CComponent{
protected:
	virtual void Dispatch(TMessage& Msg);
	BOOL _Dispatch(TMessage& Msg, MsgMapping* _MsgMapping);
	virtual void DefaultHandler(TMessage& Message);
public:
	CMsgTarget(CComponent *AOwner = NULL);
	virtual ~CMsgTarget();
	REF_DYN_CLASS(CMsgTarget)
};
DECLARE_DYN_CLASS(CMsgTarget, CComponent)