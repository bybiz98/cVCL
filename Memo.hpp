#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "Strings.hpp"
#include "Edit.hpp"
#include "Strings.hpp"

typedef BYTE TScrollStyle;
#define ssNone			0x0
#define ssHorizontal	0x1
#define ssVertical		0x2
#define ssBoth			0x3;

class CMemo;
class cVCL_API CMemoStrings : public CStrings{
private:
	friend class CMemo;
	CMemo* Memo;
protected:
    String Get(INT Index) override;
	INT GetCount() override;
	String GetTextStr() override;
	void Put(INT Index, String& S) override;
	void SetTextStr(String& Value) override;
	void SetUpdateState(BOOL Updating) override;
public:
	CMemoStrings();
	virtual ~CMemoStrings();
    void Clear() override;
	void Delete(INT Index) override;
	void Insert(INT Index, String& S) override;

	REF_DYN_CLASS(CMemoStrings)
};
DECLARE_DYN_CLASS(CMemoStrings, CStrings)

class cVCL_API CMemo : public CEdit{
private:
	CStrings* Lines;
	TAlignment Alignment;
	TScrollStyle ScrollBars;
	BOOL WordWrap;
	BOOL WantReturns;
	BOOL WantTabs;
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_GETDLGCODE, CMemo::WMGetDlgCode)
		MSG_MAP_ENTRY(WM_NCDESTROY, CMemo::WMNCDestroy)
	MSG_MAP_END()
	void WMGetDlgCode(TWMGetDlgCode& Message);
	void WMNCDestroy(TWMNCDestroy& Message);
protected:
    void CreateParams(TCreateParams& Params) override;
	void CreateWindowHandle(const TCreateParams& Params) override;
	void KeyPress(TCHAR& Key) override;
	void Loaded() override;
public:
	CMemo(CComponent* AOwner = NULL);
	virtual ~CMemo();
	TAlignment GetControlsAlignment() override;
	virtual TPoint GetCaretPos();
	virtual void SetCaretPos(TPoint& Value);
	DEFINE_GETTER(CStrings*, Lines)
	DEFINE_GETTER(TAlignment, Alignment)
	DEFINE_GETTER(TScrollStyle, ScrollBars)
	DEFINE_ACCESSOR(BOOL, WantReturns)
	DEFINE_ACCESSOR(BOOL, WantTabs)
	DEFINE_GETTER(BOOL, WordWrap)
    void SetLines(CStrings* Value);
	void SetAlignment(TAlignment Value);
	void SetScrollBars(TScrollStyle Value);
	void SetWordWrap(BOOL Value);

	REF_DYN_CLASS(CMemo)
};
DECLARE_DYN_CLASS(CMemo, CEdit)