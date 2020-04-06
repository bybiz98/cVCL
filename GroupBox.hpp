#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "Radio.hpp"

class cVCL_API CGroupBox : public CCustomControl{
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_DIALOGCHAR, CGroupBox::WMPaint)
		MSG_MAP_ENTRY(CM_TEXTCHANGED, CGroupBox::WMPaint)
		MSG_MAP_ENTRY(CM_CTL3DCHANGED, CGroupBox::WMPaint)
		MSG_MAP_ENTRY(WM_SIZE, CGroupBox::WMPaint)
	MSG_MAP_END()
	void CMDialogChar(TCMDialogChar& Message);
	void CMTextChanged(TMessage& Message);
	void CMCtl3DChanged(TMessage& Message);
	void WMSize(TMessage& Message);
protected:
	void AdjustClientRect(TRect& Rect) override;
	void CreateParams(TCreateParams& Params) override;
	void Paint() override;
public:
	CGroupBox(CComponent* AOwner = NULL);
	virtual ~CGroupBox();

	REF_DYN_CLASS(CGroupBox)
};
DECLARE_DYN_CLASS(CGroupBox, CCustomControl)

class cVCL_API CRadioGroup : public CGroupBox{
private:
	CList* Buttons;
	CStrings* Items;
	INT ItemIndex;
	INT Columns;
	BOOL Reading;
	BOOL Updating;

	void ArrangeButtons();
	void ItemsChange(CObject* Sender);
	void SetButtonCount(INT Value);
	void UpdateButtons();
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_ENABLEDCHANGED, CRadioGroup::CMEnabledChanged)
		MSG_MAP_ENTRY(CM_FONTCHANGED, CRadioGroup::CMFontChanged)
		MSG_MAP_ENTRY(WM_SIZE, CRadioGroup::WMSize)
	MSG_MAP_END()
	void CMEnabledChanged(TMessage& Message);
	void CMFontChanged(TMessage& Message);
	void WMSize(TWMSize& Message);
protected:
	void Loaded() override;
	//TODO void ReadState(CReader* Reader) override;
	//TODO void GetChildren(TGetChildProc Proc, CComponent* Root) override;
public:
	CRadioGroup(CComponent* AOwner = NULL);
	virtual ~CRadioGroup();
	//TODO void FlipChildren(BOOL AllLevels) override;
	DEFINE_GETTER(INT, Columns)
	DEFINE_GETTER(INT, ItemIndex)
	DEFINE_GETTER(CStrings*, Items)
	DEFINE_GETTER(CList*, Buttons)
	
	void ButtonClick(CObject* Sender);
	virtual BOOL CanModify();

	CRadio* GetButton(INT Index);
	void SetColumns(INT Value);
	void SetItemIndex(INT Value);
	void SetItems(CStrings* Value);

	REF_DYN_CLASS(CRadioGroup)
};
DECLARE_DYN_CLASS(CRadioGroup, CGroupBox)