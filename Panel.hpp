#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"

void Frame3D(CCanvas* Canvas, TRect& Rect, TColor TopColor, TColor BottomColor, INT Width);


typedef TBevelCut TPanelBevel;
class cVCL_API CPanel : public CCustomControl{
private:
	BOOL AutoSizeDocking;
	TPanelBevel BevelInner;
	TPanelBevel BevelOuter;
	TBevelWidth BevelWidth;
	TBorderWidth BorderWidth;
	TBorderStyle BorderStyle;
	BOOL FullRepaint;
	BOOL Locked;
	BOOL ParentBackgroundSet;
	TAlignment Alignment;
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_BORDERCHANGED, CPanel::CMBorderChanged)
		MSG_MAP_ENTRY(CM_TEXTCHANGED, CPanel::CMTextChanged)
		MSG_MAP_ENTRY(CM_CTL3DCHANGED, CPanel::CMCtl3DChanged)
		MSG_MAP_ENTRY(CM_ISTOOLCONTROL, CPanel::CMIsToolControl)
		MSG_MAP_ENTRY(WM_WINDOWPOSCHANGED, CPanel::WMWindowPosChanged)
		MSG_MAP_ENTRY(CM_DOCKCLIENT, CPanel::CMDockClient)
	MSG_MAP_END()
	void CMBorderChanged(TMessage& Message);
	void CMTextChanged(TMessage& Message);
	void CMCtl3DChanged(TMessage& Message);
	void CMIsToolControl(TMessage& Message);
	void WMWindowPosChanged(TWMWindowPosChanged& Message);
	void CMDockClient(TCMDockClient& Message);
protected:
	void CreateParams(TCreateParams& Params) override;
	void AdjustClientRect(TRect& Rect) override;
	BOOL CanAutoSize(INT& NewWidth, INT& NewHeight) override;
	void Paint() override;
public:
	CPanel(CComponent* AOwner = NULL);
	virtual ~CPanel();
    TAlignment GetControlsAlignment() override;
	
    void SetAlignment(TAlignment Value);
	void SetBevelInner(TPanelBevel Value);
	void SetBevelOuter(TPanelBevel Value);
	void SetBevelWidth(TBevelWidth Value);
	void SetBorderWidth(TBorderWidth Value);
	void SetBorderStyle(TBorderStyle Value);
	void SetParentBackground(BOOL Value) override;

	DEFINE_GETTER(TAlignment, Alignment)
	DEFINE_GETTER(TPanelBevel, BevelInner)
	DEFINE_GETTER(TPanelBevel, BevelOuter)
	DEFINE_GETTER(TBevelWidth, BevelWidth)
	DEFINE_GETTER(TBorderWidth, BorderWidth)
	DEFINE_GETTER(TBorderStyle, BorderStyle)
	DEFINE_GETTER(BOOL, FullRepaint)
	DEFINE_GETTER(BOOL, Locked)

	REF_DYN_CLASS(CPanel)
};
DECLARE_DYN_CLASS(CPanel, CCustomControl)