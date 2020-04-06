
#include "stdinc.h"
#include "MsgTarget.hpp"

IMPL_DYN_CLASS(CMsgTarget)
CMsgTarget::CMsgTarget(CComponent *AOwner) : CComponent(AOwner){
}

CMsgTarget::~CMsgTarget(){
}

void CMsgTarget::Dispatch(TMessage& Msg){
	DefaultHandler(Msg);
}

BOOL CMsgTarget::_Dispatch(TMessage& Msg, MsgMapping* _MsgMapping){
	BOOL Dispatched = FALSE;
	if(_MsgMapping != NULL){
		for(;!IS_LAST_MSG_MAP(_MsgMapping);_MsgMapping++){
			if(_MsgMapping->Msg == Msg.Msg && _MsgMapping->pfHandler != NULL){
				(this->*(_MsgMapping->pfHandler))(Msg);
				Dispatched = TRUE;
				break;
			}
		}
	}
	return Dispatched;
}

void CMsgTarget::DefaultHandler(TMessage& Message){
}
