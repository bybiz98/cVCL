#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "Edit.hpp"

typedef BYTE TMaskedState;
#define msMasked		0x1
#define msReEnter		0x2
#define msDBSetText		0x4

typedef BYTE TMaskCharType;
#define mcNone				0x0
#define mcLiteral			0x1
#define mcIntlLiteral		0x2
#define mcDirective			0x3
#define mcMask				0x4
#define mcMaskOpt			0x5
#define mcFieldSeparator	0x6
#define mcField				0x7

typedef BYTE TMaskDirectives;
#define mdReverseDir		0x1
#define	mdUpperCase			0x2
#define mdLowerCase			0x4
#define mdLiteralChar		0x8

//removes leading blanks if true, else trailing blanks
#define mDirReverse			TCHAR('!') 
//all chars that follow to upper case
#define mDirUpperCase		TCHAR('>') 
//all chars that follow to lower case
#define mDirLowerCase		TCHAR('<') 
// '<>' means remove casing directive  char that immediately follows is a literal 
#define mDirLiteral			TCHAR('\\') 
//in US = A-Z,a-z
#define mMskAlpha			TCHAR('L')
#define mMskAlphaOpt		TCHAR('l')
//in US = A-Z,a-z,0-9 
#define mMskAlphaNum		TCHAR('A')
#define mMskAlphaNumOpt		TCHAR('a')
// any character
#define mMskCharacter		TCHAR('C')
#define mMskCharacterOpt	TCHAR('c')
//0-9, no plus or minus 
#define mMskNumeric			TCHAR('0')
#define mMskNumericOpt		TCHAR('9')
//0-9, plus and minus 
#define mMskNumSymOpt		TCHAR('#')

//intl literals
#define mMskTimeSeparator	TCHAR(':')
#define mMskDateSeparator	TCHAR('/')

#define DefaultBlank		TCHAR('_')
#define MaskFieldSeparator	TCHAR(';')
#define MaskNoSave			TCHAR('0')

#define KEY_CODE_CTRL_C		3
#define KEY_CODE_CTRL_V		22
#define KEY_CODE_CTRL_X		24

typedef String TEditMask;
typedef String TMaskedText;

#define IS_SURROGATE(wch) (IS_HIGH_SURROGATE(wch) || IS_LOW_SURROGATE(wch))

TMaskCharType MaskGetCharType(const String& EditMask, INT MaskOffset);
TCHAR MaskIntlLiteralToChar(TCHAR IChar);
TMaskDirectives MaskGetCurrentDirectives(const String& EditMask, INT MaskOffset);
INT MaskOffsetToOffset(const String& EditMask, INT MaskOffset);
INT OffsetToMaskOffset(const String& EditMask, INT Offset);
String MaskDoFormatText(const String& EditMask, const String& Value, TCHAR Blank);
BOOL IsLiteralChar(const String& EditMask, INT Offset);
BOOL MaskGetMaskSave(const String& EditMask);
TCHAR MaskGetMaskBlank(const String& EditMask);
String PadInputLiterals(const String& EditMask, const String& Value, TCHAR Blank);

class cVCL_API CMaskEdit : public CEdit{
private:
	TEditMask* EditMask;
	TCHAR MaskBlank;
	INT MaxChars;
	BOOL MaskSave;
    TMaskedState MaskState;
	INT CaretPos;
	INT BtnDownX;
	String OldValue;
	BOOL SettingCursor;
	BOOL DoInputChar(TCHAR& NewChar, INT MaskOffset);
	BOOL InputChar(TCHAR& NewChar, INT Offset);
	BOOL DeleteSelection(String& Value, INT Offset, INT Len);
	INT InputString(String& Value, String& NewValue, INT Offset);
	String AddEditFormat(String& Value, BOOL Active);
	String RemoveEditFormat(String& Value);
	INT FindLiteralChar(INT MaskOffset, TCHAR InChar);
	BOOL CharKeys(TCHAR CharCode);
	void DeleteKeys(WORD CharCode);
	void HomeEndKeys(WORD CharCode, TShiftState Shift);
	void CursorInc(INT CursorPos, INT Incr);
	void CursorDec(INT CursorPos);
	void ArrowKeys(WORD CharCode, TShiftState Shift);
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(WM_LBUTTONDOWN, CMaskEdit::WMLButtonDown)
		MSG_MAP_ENTRY(WM_LBUTTONUP, CMaskEdit::WMLButtonUp)
		MSG_MAP_ENTRY(WM_SETFOCUS, CMaskEdit::WMSetFocus)
		MSG_MAP_ENTRY(WM_CUT, CMaskEdit::WMCut)
		MSG_MAP_ENTRY(WM_PASTE, CMaskEdit::WMPaste)
		MSG_MAP_ENTRY(CM_ENTER, CMaskEdit::CMEnter)
		MSG_MAP_ENTRY(CM_EXIT, CMaskEdit::CMExit)
		MSG_MAP_ENTRY(CM_TEXTCHANGED, CMaskEdit::CMTextChanged)
		MSG_MAP_ENTRY(CM_WANTSPECIALKEY, CMaskEdit::CMWantSpecialKey)
	MSG_MAP_END()
	void WMLButtonDown(TWMLButtonDown& Message);
	void WMLButtonUp(TWMLButtonUp& Message);
	void WMSetFocus(TWMSetFocus& Message);
	void WMCut(TMessage& Message);
	void WMPaste(TMessage& Message);
	void CMEnter(TCMEnter& Message);
	void CMExit(TCMExit& Message);
	void CMTextChanged(TMessage& Message);
	void CMWantSpecialKey(TCMWantSpecialKey& Message);
protected:
	void ReformatText(String& NewMask);
	void SetCursor(INT Pos);
	void DoSetCursor();
	void KeyDown(WORD& Key, TShiftState Shift) override;
	void KeyUp(WORD& Key, TShiftState Shift) override;
	void KeyPress(TCHAR& Key) override;
	virtual BOOL EditCanModify();
	virtual void Reset();
	INT GetFirstEditChar();
	INT GetLastEditChar();
	INT GetNextEditChar(INT Offset);
	INT GetPriorEditChar(INT Offset);
	INT GetMaxChars();
	virtual BOOL Validate(String& Value, INT& Pos);
	virtual void ValidateError();
	void CheckCursor();
public:
	CMaskEdit(CComponent* AOwner = NULL);
	virtual ~CMaskEdit();
	virtual void ValidateEdit();
	void Clear() override;
	INT GetTextLen();
	BOOL GetMasked();
	String GetEditText();
	void SetEditText(String& Value);
	TMaskedText GetText();
	void SetText(TMaskedText& Value);
	void SetEditMask(TEditMask& Value);
	INT GetMaxLength();
	void SetMaxLength(INT Value);
	DEFINE_GETTER(TEditMask*, EditMask)
	DEFINE_ACCESSOR(TMaskedState, MaskState)

	REF_DYN_CLASS(CMaskEdit)
};
DECLARE_DYN_CLASS(CMaskEdit, CEdit)
