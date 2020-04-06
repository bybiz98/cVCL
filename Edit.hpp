#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"

typedef BYTE TEditCharCase;
#define ecNormal			0x0
#define	ecUpperCase			0x1
#define ecLowerCase			0x2

class cVCL_API CEdit : public CWinControl{
private:
	INT MaxLength;
	TBorderStyle BorderStyle;
	TCHAR PasswordChar;
	BOOL ReadOnly;
	BOOL AutoSize;
	BOOL AutoSelect;
	BOOL HideSelection;
	BOOL OEMConvert;
	TEditCharCase CharCase;
	BOOL Creating;
	BOOL Modified;

	void AdjustHeight();
	void UpdateHeight();
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Change)
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_SETFONT, CEdit::WMSetFont)
		MSG_MAP_ENTRY(CM_CTL3DCHANGED, CEdit::CMCtl3DChanged)
		MSG_MAP_ENTRY(CM_ENTER, CEdit::CMEnter)
		MSG_MAP_ENTRY(CM_FONTCHANGED, CEdit::CMFontChanged)
		MSG_MAP_ENTRY(CN_COMMAND, CEdit::CNCommand)
		MSG_MAP_ENTRY(CM_TEXTCHANGED, CEdit::CMTextChanged)
		MSG_MAP_ENTRY(WM_CONTEXTMENU, CEdit::WMContextMenu)
	MSG_MAP_END()
	void WMSetFont(TWMSetFont& Message);
	void CMCtl3DChanged(TMessage& Message);
	void CMEnter(TCMGotFocus& Message);
	void CMFontChanged(TCMGotFocus& Message);
	void CNCommand(TWMCommand& Message);
	void CMTextChanged(TMessage& Message);
	void WMContextMenu(TWMContextMenu& Message);
protected:
	void CreateParams(TCreateParams& Params) override;
	void CreateWindowHandle(const TCreateParams& Params) override;
	void CreateWnd() override;
	void DestroyWnd() override;
	void DefaultHandler(TMessage& Message) override;

	virtual void DoSetMaxLength(INT Value);
	virtual void Change();
public:
	CEdit(CComponent* AOwner = NULL);
	virtual ~CEdit();

	DEFINE_GETTER(TBorderStyle, BorderStyle)

	void SetBorderStyle(TBorderStyle Value);
	void SetCharCase(TEditCharCase Value);
	void SetHideSelection(BOOL Value);
	void SetMaxLength(INT Value);
	void SetOEMConvert(BOOL Value);
	void SetAutoSize(BOOL Value) override;
	BOOL GetModified();
	void SetModified(BOOL Value);
	void SetPasswordChar(TCHAR Value);
	void SetReadOnly(BOOL Value);

	BOOL GetCanUndo();
	INT GetSelStart();
	void SetSelStart(INT Value);
	INT GetSelLength();
	void SetSelLength(INT Value);

	virtual void Clear();
	void ClearSelection();
	void CopyToClipboard();
	void CutToClipboard();
	void PasteFromClipboard();
	void Undo();
	void ClearUndo();
	void SelectAll();
	INT GetSelTextBuf(LPVOID Buffer, INT BufSize);
	void SetSelTextBuf(LPTSTR Buffer);

	REF_DYN_CLASS(CEdit)
};
DECLARE_DYN_CLASS(CEdit, CWinControl)
