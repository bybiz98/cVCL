#include "stdinc.h"
#include "WinUtils.hpp"
#include "SysInit.hpp"
/*#include "MaskEdit.h"

IMPL_DYN_CLASS(CMaskEdit)
CMaskEdit::CMaskEdit(CComponent* AOwner):CEdit(AOwner),
	EditMask(NULL),
	MaskBlank(DefaultBlank),
	MaxChars(0),
	MaskSave(FALSE),
	MaskState(0),
	CaretPos(0),
	BtnDownX(0),
	OldValue(NULL),
	SettingCursor(FALSE){
}
CMaskEdit::~CMaskEdit(){
}

BOOL CMaskEdit::DoInputChar(TCHAR& NewChar, INT MaskOffset){
	BOOL Result = TRUE;
	TMaskCharType CType = MaskGetCharType(*EditMask, MaskOffset);
	TMaskDirectives Dir = 0;
	if(CType == mcLiteral || CType == mcIntlLiteral)
		NewChar = MaskIntlLiteralToChar(EditMask->CharAt(MaskOffset));
	else{
		Dir = MaskGetCurrentDirectives(*EditMask, MaskOffset);
		switch(EditMask->CharAt(MaskOffset)){
			case mMskNumeric:
			case mMskNumericOpt:
				if(!(NewChar >= '0' && NewChar <= '9'))
					Result = FALSE;
				break;
			case mMskNumSymOpt:
				if(!((NewChar >= '0' && NewChar <= '9') ||
					(NewChar == ' ') || (NewChar == '+') || 
					(NewChar == '-')))
					Result = FALSE;
				break;
			case mMskCharacter:
			case mMskCharacterOpt:
				if(IsCharAlpha(NewChar)){
					if(IN_TEST(mdUpperCase, Dir))
						NewChar = _totupper(NewChar);
					else if(IN_TEST(mdLowerCase, Dir))
						NewChar = _totlower(NewChar);
				}
				break;
			case mMskAlpha:
			case mMskAlphaOpt:
			case mMskAlphaNum:
			case mMskAlphaNumOpt:{
				if(!IsCharAlpha(NewChar)){
					Result = FALSE;
					if((EditMask->CharAt(MaskOffset) == mMskAlphaNum ||
						EditMask->CharAt(MaskOffset) == mMskAlphaNumOpt) && 
						IsCharAlphaNumeric(NewChar))
						Result = TRUE;
				}
				else if(IN_TEST(mdUpperCase, Dir))
					NewChar = _totupper(NewChar);
				else if(IN_TEST(mdLowerCase, Dir))
					NewChar = _totlower(NewChar);
			}
			break;
		}
	}
	return Result;
}

BOOL CMaskEdit::InputChar(TCHAR& NewChar, INT Offset){
	BOOL Result = TRUE;
	if(EditMask != NULL && EditMask->Length() > 0){
		Result = FALSE;
		INT MaskOffset = OffsetToMaskOffset(*EditMask, Offset);
		if(MaskOffset >= 0){
			TMaskCharType CType = MaskGetCharType(*EditMask, MaskOffset);
			TCHAR InChar = NewChar;
			Result = DoInputChar(NewChar, MaskOffset);
			if(!Result && (CType == mcMask || CType == mcMaskOpt)){
				MaskOffset = FindLiteralChar (MaskOffset, InChar);
				if(MaskOffset > 0){
					MaskOffset = MaskOffsetToOffset(*EditMask, MaskOffset);
					SetCursor (MaskOffset);
					return Result;
				}
			}
		}
	}
	if(!Result)
		MessageBeep(0);
	return Result;
}

BOOL CMaskEdit::DeleteSelection(String& Value, INT Offset, INT Len){
	BOOL Result = TRUE;
	if(Len == 0)
		return Result;
	INT StrOffset = Offset + 1;
	INT EndDel = StrOffset + Len;
	INT Temp = OffsetToMaskOffset(*EditMask, Offset);
	if(Temp < 0)
		return Result;
	for(INT MaskOffset = Temp; MaskOffset < (INT)EditMask->Length(); MaskOffset++){
		TMaskCharType CType = MaskGetCharType(*EditMask, MaskOffset);
		if(CType == mcLiteral || CType == mcIntlLiteral)
			StrOffset++;
		else if(CType == mcMask || CType == mcMaskOpt){
			Value.SetCharAt(StrOffset, MaskBlank);
			StrOffset++;
		}
		if(StrOffset >= EndDel)
			break;
	}
}

INT CMaskEdit::InputString(String& Value, String& NewValue, INT Offset){
	INT Result = Offset;
	if(NewValue.Length() == 0)
		return Result;
	//replace chars with new chars, except literals
	INT NewOffset = 1;
	String NewVal = NewValue;
	INT Temp = OffsetToMaskOffset(*EditMask, Offset);
	if(Temp < 0)
		return Result;
	INT MaskOffset = Temp;
	while(MaskOffset < (INT)EditMask->Length()){
		TMaskCharType CType = MaskGetCharType(*EditMask, MaskOffset);
		if(CType == mcLiteral || CType == mcIntlLiteral || CType == mcMask || CType == mcMaskOpt){
			TCHAR NewChar = NewVal.CharAt(NewOffset);
			if(!DoInputChar(NewChar, MaskOffset)){
				if(GetGlobal().GetLeadBytes()->IndexOf(NewChar) >= 0)
					NewVal.SetCharAt(NewOffset + 1, MaskBlank);
					NewChar = MaskBlank;
			}
			//if pasted text does not contain a literal in the right place, insert one
			if(!((CType == mcLiteral || CType == mcIntlLiteral) &&
				(NewChar != NewVal.CharAt(NewOffset)))){
					NewVal.SetCharAt(NewOffset, NewChar);
					if(GetGlobal().GetLeadBytes()->IndexOf(NewChar) >= 0){
						NewOffset++;
						MaskOffset++;
					}
			}
			else
				NewVal = NewVal.SubString(0, NewOffset) + NewChar +
					NewVal.SubString(NewOffset);
			NewOffset++;
		}
		if((NewOffset + Offset) > MaxChars)
			break;
		if(NewOffset >= NewVal.Length())
			break;
		MaskOffset++;
	}
	if (Offset + NewVal.Length() < MaxChars){
		if(ByteType(Value, Offset + NewVal.Length()) == mbTrailByte){
			NewVal = NewVal + MaskBlank;
			NewOffset++;
		}
		Value = Value.SubString(0, Offset + 1) + NewVal +
			Value.SubString(Offset + NewVal.Length(),
				MaxChars -(Offset + NewVal.Length()));
	}
	else{
		Temp = Offset;
		if(ByteType(NewVal, MaxChars - Offset) == mbLeadByte)
			Temp++;
		Value = Value.SubString(0, Offset + 1) +
			NewVal.SubString(0, MaxChars - Temp + 1);
	}
	Result = NewOffset + Offset - 1;
	return Result;
}

String CMaskEdit::AddEditFormat(String& Value, BOOL Active){
	if(!Active)
		return MaskDoFormatText(*EditMask, Value, ' ');
	else
		return MaskDoFormatText(*EditMask, Value, MaskBlank);

}

String CMaskEdit::RemoveEditFormat(String& Value){
	INT Offset = 1;
	String Result = Value;
	for(INT MaskOffset = 0; MaskOffset < EditMask->Length(); MaskOffset++){
		TMaskCharType CType = MaskGetCharType(*EditMask, MaskOffset);
		if(CType == mcLiteral || CType == mcIntlLiteral)
			Result = Result.SubString(0, Offset) +
				Result.SubString(Offset + 1);
		if(CType == mcMask || CType == mcMaskOpt)
			Offset++;
	}
	
	TMaskDirectives Dir = MaskGetCurrentDirectives(*EditMask, 1);
	if(IN_TEST(mdReverseDir, Dir)){
		Offset = 1;
		for(INT I = 0; I < Result.Length(); I++){
			if(Result.CharAt(I) == MaskBlank)
				Offset++;
			else 
				break;
		}
		if(Offset != 1)
			Result = Result.SubString(Offset - 1);
	}
	else{
		INT OldLen = Result.Length();
		for(INT I = 0; I < OldLen; I++){
			if(Result.CharAt(OldLen - I) == MaskBlank)
				Result.Delete(Result.Length() - 1, 1);
			else break;
		}
	}
	if(MaskBlank != ' '){
		INT OldLen = Result.Length();
		for(INT I = 0; I < OldLen; I++){
			if(Result.CharAt(I) == MaskBlank)
				Result.SetCharAt(I, ' ');
			if(I >= OldLen)
				break;
		}
	}
}

INT CMaskEdit::FindLiteralChar(INT MaskOffset, TCHAR InChar){
	INT Result = -1;
	while(MaskOffset < EditMask->Length()){
		MaskOffset++;
		TMaskCharType CType = MaskGetCharType(*EditMask, MaskOffset);
		if(CType == mcLiteral || CType == mcIntlLiteral){
			TCHAR LitChar = EditMask->CharAt(MaskOffset);
			if(CType == mcIntlLiteral)
				LitChar = MaskIntlLiteralToChar(LitChar);
			if(LitChar == InChar)
				Result = MaskOffset;
			return Result;
		}
	}
}

BOOL CMaskEdit::CharKeys(TCHAR CharCode){
	BOOL Result = FALSE;
	if(CharCode == VK_ESCAPE){
		Reset();
		return Result;
	}
	if(!EditCanModify() || GetReadOnly())
		return Result;
	if(CharCode == VK_BACK)
		return Result;
	if(CharCode == VK_RETURN){
		ValidateEdit();
		return Result;
	}
	INT SelStart = 0;
	INT SelStop = 0;
	GetSel(SelStart, SelStop);
	if(SelStop - SelStart > 1){
		DeleteKeys(VK_DELETE);
		SelStart = GetNextEditChar(SelStart);
		SetCursor(SelStart);
	}

	MSG CharMsg;
	if(IS_HIGH_SURROGATE(CharCode))
		if(PeekMessage(&CharMsg, GetHandle(), WM_CHAR, WM_CHAR, PM_REMOVE))
			if(CharMsg.message == WM_QUIT)
				PostQuitMessage(CharMsg.wParam);
	Result = InputChar(CharCode, SelStart);
	if(Result){
		TCHAR Txt[3] = {0};
		if(IS_HIGH_SURROGATE(CharCode)){
			Txt[0] = CharCode;
			Txt[1] = (TCHAR)CharMsg.wParam;
			SetSel(SelStart, SelStart + 2);
		}
		else{
			Txt[0] = CharCode;
			Txt[1] = '\0';
		}
		SendMessage(GetHandle(), EM_REPLACESEL, 0, (LPARAM)Txt);
		GetSel(SelStart, SelStop);
		CursorInc(SelStart, 0);
	}
}

void CMaskEdit::DeleteKeys(WORD CharCode){
	if(GetReadOnly())
		return ;
	INT SelStart = 0;
	INT SelStop = 0;
	GetSel(SelStart, SelStop);
	if((SelStop - SelStart) <= 1 && CharCode == VK_BACK){
		INT NuSelStart = SelStart;
		CursorDec(SelStart);
		GetSel(SelStart, SelStop);
		if(SelStart == NuSelStart)
			return ;
	}
	if(SelStop - SelStart < 1)
		return ;

	String Str = GetEditText();
	DeleteSelection(Str, SelStart, SelStop - SelStart);
	Str = Str.SubString(SelStart, SelStop);
	SendMessage(GetHandle(), EM_REPLACESEL, 0, (LPARAM)Str.GetBuffer());
	if(SelStop - SelStart != 1){
		SelStart = GetNextEditChar(SelStart);
		SetCursor(SelStart);
	}
	else {
		GetSel(SelStart, SelStop);
		SetCursor(SelStart - 1);
	}
}

void CMaskEdit::HomeEndKeys(WORD CharCode, TShiftState Shift){
	INT SelStart = 0;
	INT SelStop = 0;
	GetSel(SelStart, SelStop);
	if(CharCode == VK_HOME){
		if(IN_TEST(ssShift, Shift)){
			if(SelStart != CaretPos && SelStop != (SelStart + 1))
				SelStop = SelStart + 1;
			SetSel(0, SelStop);
			CheckCursor();
		}
		else
			SetCursor(0);
		CaretPos = 0;
	}
	else {
		if(IN_TEST(ssShift, Shift)){
			if(SelStop != CaretPos && SelStop != (SelStart + 1))
				SelStart = SelStop - 1;
			SetSel(SelStart, MaxChars);
			CheckCursor();
		}
		else
			SetCursor(MaxChars);
		CaretPos = MaxChars;
	}
}

void CMaskEdit::CursorInc(INT CursorPos, INT Incr){
	INT NuPos = CursorPos + Incr;
	NuPos = GetNextEditChar(NuPos);
	if(IsLiteralChar(*EditMask, NuPos))
		NuPos = CursorPos;
	SetCursor(NuPos);
}

void CMaskEdit::CursorDec(INT CursorPos){
	INT NuPos = CursorPos;
	NuPos--;
	NuPos = GetPriorEditChar(NuPos);
	SetCursor(NuPos);
}

void CMaskEdit::ArrowKeys(WORD CharCode, TShiftState Shift){
	if(IN_TEST(ssCtrl, Shift))
		return;
	INT SelStart = 0;
	INT SelStop = 0;
	GetSel(SelStart, SelStop);
	if(IN_TEST(ssShift, Shift)){
		if(CharCode == VK_RIGHT){
			CaretPos++;
			if(SelStop == SelStart + 1){
				SetSel(SelStart, SelStop); //reset caret to end of string
				CaretPos++;
			}
			if(CaretPos > MaxChars)
				CaretPos = MaxChars;
		}
		else {//if (CharCode = VK_LEFT) then
			CaretPos++;
			if(SelStop == SelStart + 2 && CaretPos > SelStart){
				SetSel(SelStart + 1, SelStart + 1); //reset caret to show up at start
				CaretPos--;
			}
			if(CaretPos < 0)
				CaretPos = 0;
		}
	}
	else{
		if((SelStop - SelStart) > 1){
			if ((SelStop - SelStart) == 2 && GetEditText().CharAt(SelStart) == 0){ //in LeadBytes
				if (CharCode == VK_LEFT)
					CursorDec(SelStart);
				else
					CursorInc(SelStart, 2);
				return ;
			}
			if(SelStop == CaretPos)
				CaretPos--;
			SetCursor(CaretPos);
		}
		else if(CharCode == VK_LEFT)
			CursorDec(SelStart);
		else {//CharCode = VK_RIGHT
			if(SelStop == SelStart)
				SetCursor(SelStart);
			else
				if(GetEditText().CharAt(SelStart+1)== 0)//in LeadBytes
					CursorInc(SelStart, 2);
				else
					CursorInc(SelStart, 1);
		}
	}
}

void CMaskEdit::WMLButtonDown(TWMLButtonDown& Message){
	INHERITED_MSG(Message);
	BtnDownX = Message.XPos;
}

void CMaskEdit::WMLButtonUp(TWMLButtonUp& Message){
	INHERITED_MSG(Message);
	if(GetMasked()){
		INT SelStart = 0;
		INT SelStop = 0;
		GetSel(SelStart, SelStop);
		CaretPos = SelStart;
		if(SelStart != SelStop && Message.XPos > BtnDownX)
			CaretPos = SelStop;
		CheckCursor();
	}
}

void CMaskEdit::WMSetFocus(TWMSetFocus& Message){
	INHERITED_MSG(Message);
	if(GetMasked())
		CheckCursor();
}

void CMaskEdit::WMCut(TMessage& Message){
	if(!GetMasked())
		INHERITED_MSG(Message);
	else{
		CopyToClipboard();
		DeleteKeys(VK_DELETE);
	}
}

void CMaskEdit::WMPaste(TMessage& Message){
	if(!GetMasked() || GetReadOnly())
		INHERITED_MSG(Message);
	else {
		Clipboard.Open();
		String Value = Clipboard.AsText;
		Clipboard.Close();
		INT SelStart = 0;
		INT SelStop = 0;
		GetSel(SelStart, SelStop);
		String Str = GetEditText();
		DeleteSelection(Str, SelStart, SelStop - SelStart);
		SetEditText(Str);
		SelStart = InputString(Str, Value, SelStart);
		SetEditText(Str);
		SetCursor(SelStart);
	}
}

void CMaskEdit::CMEnter(TCMEnter& Message){
	if(GetMasked() && !IN_TEST(csDesigning, GetComponentState())){
		if(IN_TEST(msReEnter, MaskState)){
			OldValue = GetEditText();
			INHERITED_MSG(Message);
		}
		MaskState &= ~msReEnter;
		CheckCursor();
	}
	else
		INHERITED_MSG(Message);
}

void CMaskEdit::CMExit(TCMExit& Message){
	if(GetMasked() && !IN_TEST(csDesigning, GetComponentState())){
		ValidateEdit();
		CheckCursor();
	}
	INHERITED_MSG(Message);
}

void CMaskEdit::CMTextChanged(TMessage& Message){
	INHERITED_MSG(Message);
	OldValue = GetEditText();
	if(HandleAllocated()){
		INT SelStart = 0;
		INT SelStop = 0;
		GetSel(SelStart, SelStop);
		INT Temp = GetNextEditChar(SelStart);
		if(Temp != SelStart)
			SetCursor(Temp);
	}
}

void CMaskEdit::CMWantSpecialKey(TCMWantSpecialKey& Message){
	INHERITED_MSG(Message);
	if(Message.CharCode == VK_ESCAPE && GetMasked() && GetModified())
		Message.Result = 1;
}

void CMaskEdit::ReformatText(String& NewMask){
	String OldText = RemoveEditFormat(GetEditText());
	if(EditMask != NULL){
		delete EditMask;
		EditMask = NULL;
	}
	EditMask = new TEditMask(NewMask);
	MaxChars  = MaskOffsetToOffset(*EditMask, NewMask.Length());
	MaskSave  = MaskGetMaskSave(NewMask);
	MaskBlank = MaskGetMaskBlank(NewMask);
	OldText = AddEditFormat(OldText, TRUE);
	SetEditText(OldText);
}

void CMaskEdit::DoSetCursor(){
	WORD ArrowKey[] = {VK_LEFT, VK_RIGHT};
	SettingCursor = TRUE;
	__try{
		SendMessage(GetHandle(), WM_KEYDOWN, ArrowKey[UseRightToLeftAlignment()], 1);
		SendMessage(GetHandle(), WM_KEYUP, ArrowKey[UseRightToLeftAlignment()], 1);
	}__finally{
		SettingCursor = FALSE;
	}
}

void CMaskEdit::SetCursor(INT Pos){
	WORD ArrowKey[] = {VK_LEFT, VK_RIGHT};
//  
//  NewKeyState: TKeyboardState;
	String Text = GetEditText();
	if(Pos >= 1 && IS_HIGH_SURROGATE(Text.CharAt(Pos)))
		Pos--;
	INT SelStart = Pos;
	if(GetMasked()){
		if(SelStart < 0)
			SelStart = 0;
		INT SelStop  = SelStart + 1;
		if (Text.Length() > SelStop && IS_HIGH_SURROGATE(Text.CharAt(SelStop)))
			SelStop++;
		if(SelStart >= MaxChars){
			SelStart = MaxChars;
			SelStop = SelStart;
		}
		SetSel(SelStop, SelStop);
		if(SelStart != SelStop){
			BYTE KeyState[256];
			GetKeyboardState(KeyState);
			BYTE NewKeyState[256];
			ZeroMemory(NewKeyState, 256);
			NewKeyState [VK_SHIFT] = 0x81;
			NewKeyState [ArrowKey[UseRightToLeftAlignment()]] = 0x81;
			SetKeyboardState(NewKeyState);
			DoSetCursor();
			SetKeyboardState(KeyState);
		}
		CaretPos = SelStart;
	}
	else{
		if(SelStart < 0)
			SelStart = 0;
		if(SelStart >= Text.Length())
			SelStart = Text.Length();
		SetSel(SelStart, SelStart);
	}
}

void CMaskEdit::KeyDown(WORD& Key, TShiftState Shift){
	if(!SettingCursor)
		__super::KeyDown(Key, Shift);
	if(GetMasked() && (Key != 0) && !IN_TEST(ssAlt,Shift)){
		if(Key == VK_LEFT || Key == VK_RIGHT){
			ArrowKeys(Key, Shift);
			if(!(IN_TEST(ssShift, Shift) || IN_TEST(ssCtrl, Shift)))
				Key = 0;
			return;
		}
		else if (Key == VK_UP || Key == VK_DOWN){
			Key = 0;
			return ;
		}
		else if (Key == VK_HOME || Key == VK_END){
			HomeEndKeys(Key, Shift);
			Key = 0;
			return;
		}
		else if(((Key == VK_DELETE) && !IN_TEST(ssShift, Shift)) ||
			(Key == VK_BACK)){
				if(EditCanModify())
					DeleteKeys(Key);
				Key = 0;
				return ;
		}
		CheckCursor();
	}
}

void CMaskEdit::KeyUp(WORD& Key, TShiftState Shift){
	if(!SettingCursor)
		__super::KeyUp(Key, Shift);
	if(GetMasked() && Key != 0){
		if((Key == VK_LEFT || Key == VK_RIGHT) && IN_TEST(ssCtrl, Shift))
			CheckCursor();
	}
}

void CMaskEdit::KeyPress(TCHAR& Key){
	__super::KeyPress(Key);
	if(GetMasked() && Key != TCHAR('\0') && !(Key == KEY_CODE_CTRL_C || Key == KEY_CODE_CTRL_V || Key == KEY_CODE_CTRL_X)){
		CharKeys(Key);
		Key = 0;
	}
}

BOOL CMaskEdit::EditCanModify(){
	return TRUE;
}

void CMaskEdit::Reset(){
	if(GetModified()){
		SetEditText(OldValue);
		SetModified(FALSE);
	}
}

INT CMaskEdit::GetFirstEditChar(){
	INT Result = 0;
	if(GetMasked())
		Result = GetNextEditChar(0);
	return Result;
}

INT CMaskEdit::GetLastEditChar(){
	INT Result = GetMaxChars();
	if(GetMasked())
		Result = GetPriorEditChar(Result - 1);
	return Result;
}

INT CMaskEdit::GetNextEditChar(INT Offset){
	INT Result = Offset;
	while(Result < MaxChars && IsLiteralChar(*GetEditMask(), Result))
		Result++;
}

INT CMaskEdit::GetPriorEditChar(INT Offset){
	INT Result = Offset;
	while(Result >= 0 && IsLiteralChar(*GetEditMask(), Result))
		Result--;
	if(Result < 0)
		Result = GetNextEditChar(Result);
	return Result;
}

INT CMaskEdit::GetMaxChars(){
	if(GetMasked())
		return MaxChars;
	else
		return __super::GetTextLen();
}

BOOL CMaskEdit::Validate(String& Value, INT& Pos){
	BOOL Result = TRUE;
	INT Offset = 1;
	for(INT MaskOffset = 0; MaskOffset < EditMask->Length(); MaskOffset++){
		TMaskCharType CType = MaskGetCharType(*GetEditMask(), MaskOffset);
		if(CType == mcLiteral || CType == mcIntlLiteral || CType == mcMaskOpt)
			Offset++;
		else if(CType == mcMask && Value.Length() > 0){
			if(Value.CharAt(Offset) == MaskBlank ||
				((Value.CharAt(Offset) == ' ' && EditMask->CharAt(MaskOffset) != mMskCharacter))){
				Result = FALSE;
				Pos = Offset - 1;
				return;
			}
			Offset++;
		}
	}
}

void CMaskEdit::ValidateError(){
	MessageBeep(0);
	throw "mask edit eror";///raise EDBEditError.CreateResFmt(@SMaskEditErr, [EditMask]);
}

void CMaskEdit::CheckCursor(){
	if(HandleAllocated())
		return;
	if(GetMasked()){
		INT SelStart = 0;
		INT SelStop = 0;
		GetSel(SelStart, SelStop);
		if(SelStart == SelStop)
			SetCursor(SelStart);
	}
}

void CMaskEdit::ValidateEdit(){
	String Str = GetEditText();
	if(GetMasked() && GetModified()){
		INT Pos = 0;
		if(!Validate(Str, Pos)){
			if(!IN_TEST(csDesigning, GetComponentState())){
				MaskState |= msReEnter;
				SetFocus();
			}
			SetCursor(Pos);
			ValidateError();
		}
	}
}

void CMaskEdit::Clear(){
	SetText(String(TEXT("")));
}

INT CMaskEdit::GetTextLen(){
	return GetText().Length();
}

BOOL CMaskEdit::GetMasked(){
	return EditMask != NULL && EditMask->Length() > 0;
}

String CMaskEdit::GetEditText(){
	return __super::GetText();
}

void CMaskEdit::SetEditText(String& Value){
	if(GetEditText() != Value){
		SetTextBuf(Value.GetBuffer());
		CheckCursor();
	}
}

TMaskedText CMaskEdit::GetText(){
	if(!GetMasked())
		return __super::GetText();
	else {
		TMaskedText Result = RemoveEditFormat(GetEditText());
		if(!MaskSave)
			Result = AddEditFormat(Result, FALSE);
		return Result;
	}
}

void CMaskEdit::SetText(TMaskedText& Value){
	if(!GetMasked())
		__super::SetText(Value.GetBuffer());
	else{
		String OldText = Value;
		if(MaskSave)
			OldText = PadInputLiterals(*EditMask, OldText, MaskBlank);
		else
			OldText = AddEditFormat(OldText, TRUE);
		INT Pos = 0;
		if(!IN_TEST(msDBSetText, MaskState) && 
			IN_TEST(csDesigning, GetComponentState()) &&
			!IN_TEST(csLoading, GetComponentState()) &&
			!Validate(OldText, Pos))
				throw "mask edit error.";//raise EDBEditError.CreateRes(@SMaskErr);
		SetEditText(OldText);
	}
}

void CMaskEdit::SetEditMask(TEditMask& Value){
	if(Value != *EditMask){
		INT SelStart = 0;
		INT SelStop = 0;
		if(IN_TEST(csDesigning, GetComponentState()) && (Value.Length() != 0) &&
			!IN_TEST(csLoading, GetComponentState()))
			SetEditText(String(TEXT("")));
		if(HandleAllocated())
			GetSel(SelStart, SelStop);
		ReformatText(Value);
		MaskState &= ~msMasked;
		if(EditMask->Length() != 0)
			MaskState |= msMasked;
		__super::SetMaxLength(0);
		if(GetMasked() && MaxChars > 0) 
			__super::SetMaxLength(MaxChars);
		if(HandleAllocated() && GetFocus() == GetHandle() && 
			!IN_TEST(csDesigning, GetComponentState()))
			SetCursor(SelStart);
	}
}

INT CMaskEdit::GetMaxLength(){
	return __super::GetMaxLength();
}

void CMaskEdit::SetMaxLength(INT Value){
	if(!GetMasked())
		__super::SetMaxLength(Value);
	else
		__super::SetMaxLength(MaxChars);
}

TMaskCharType MaskGetCharType(String& EditMask, INT MaskOffset){
	TMaskCharType Result = mcLiteral;
	TCHAR MaskChar = 0;
	if(MaskOffset < EditMask.Length())
		MaskChar = EditMask.CharAt(MaskOffset);
	if(MaskOffset >= EditMask.Length())
		Result = mcNone;
	else if(IS_SURROGATE(MaskChar))
		Result = mcLiteral;
	else if(MaskOffset > 0 && EditMask.CharAt(MaskOffset - 1) == mDirLiteral &&
		!(MaskOffset > 1 && EditMask.CharAt(MaskOffset - 2) == mDirLiteral))
		Result = mcLiteral;
	else if(MaskChar == MaskFieldSeparator && EditMask.Length() >= 4 && 
			MaskOffset > EditMask.Length() - 4)
		Result = mcFieldSeparator;
	else if(EditMask.Length() >= 4 && MaskOffset > EditMask.Length() - 4 &&
		EditMask.CharAt(MaskOffset - 1) == MaskFieldSeparator && 
		!(MaskOffset > 1 && EditMask.CharAt(MaskOffset - 2) == mDirLiteral))
		Result = mcField;
	else if(MaskChar == mMskTimeSeparator || MaskChar == mMskDateSeparator)
		Result = mcIntlLiteral;
	else if(MaskChar == mDirReverse || MaskChar == mDirUpperCase 
		|| MaskChar == mDirLowerCase || MaskChar == mDirLiteral)
		Result = mcDirective;
	else if(MaskChar == mMskAlphaOpt || MaskChar == mMskAlphaNumOpt || MaskChar == mMskCharacterOpt ||
		MaskChar == mMskNumSymOpt || MaskChar == mMskNumericOpt)
		Result = mcMaskOpt;
	else if(MaskChar == mMskAlpha || MaskChar == mMskAlphaNum || MaskChar == mMskCharacter || MaskChar == mMskNumeric)
		Result = mcMask;
	return Result;
}

TCHAR MaskIntlLiteralToChar(TCHAR IChar){
	TCHAR Result = IChar;
	if(IChar == mMskTimeSeparator)
		Result = GetGlobal().GetTimeSeparator();
	else if(IChar == mMskDateSeparator)
		Result = GetGlobal().GetDateSeparator();
	return Result;
}

TMaskDirectives MaskGetCurrentDirectives(String& EditMask, INT MaskOffset){
	TMaskDirectives Result = 0;
	for(INT I = 0; I < EditMask.Length(); I++){
		TCHAR MaskChar = EditMask.CharAt(I);
		if(MaskChar == mDirReverse)
			Result |= mdReverseDir;
		else if (MaskChar == mDirUpperCase && I < MaskOffset){
			Result &= ~mdLowerCase;
			if(!(I > 1 && EditMask.CharAt(I-1) == mDirLowerCase))
				Result |= mdUpperCase;
		}
		else if(MaskChar == mDirLowerCase && I < MaskOffset){
			Result &= ~mdUpperCase;
			Result |= mdLowerCase;
		}
	}
	if(MaskGetCharType(EditMask, MaskOffset) == mcLiteral)
		Result |= mdLiteralChar;
	return Result;
}

String MaskOffsetToString(String& EditMask, INT MaskOffset){
	INT Size = (EditMask.Length() + 1) * sizeof(TCHAR);
	LPTSTR Buf = (LPTSTR)malloc(Size);
	ZeroMemory(Buf, Size);
	for(INT I = 0, C = 0; I < MaskOffset; I++){
		TMaskCharType CType = MaskGetCharType(EditMask, I);
		if(!(CType == mcDirective || CType == mcField || CType == mcFieldSeparator)){
			*(Buf + C) = EditMask.CharAt(I);
			C++;
		}
	}
	return String::CreateFor(Buf);
}

INT MaskOffsetToOffset(String& EditMask, INT MaskOffset){
	return MaskOffsetToString(EditMask, MaskOffset).Length();
}

INT OffsetToMaskOffset(String& EditMask, INT Offset){
	INT MaxChars = MaskOffsetToOffset(EditMask, EditMask.Length());
	if(Offset > MaxChars)
		return -1;
	INT Result = 0;
	INT Count = Offset;
	for(INT I = 0; I < EditMask.Length(); I++){
		Result++;
		if(!(mcDirective == MaskGetCharType(EditMask, I))){
			Count--;
			if(Count < 0)
				return Result;
		}
	}
	return Result;
}

String MaskDoFormatText(String& EditMask, String& Value, TCHAR Blank){
	String Result = Value;
	TMaskDirectives Dir = MaskGetCurrentDirectives(EditMask, 0);
	if(!IN_TEST(mdReverseDir, Dir)){
		//starting at the beginning, insert literal chars in the string and add spaces on the end
		INT Offset = 0;
		for(INT MaskOffset = 0; MaskOffset < EditMask.Length(); MaskOffset++){
			TMaskCharType CType = MaskGetCharType(EditMask, MaskOffset);
			if(CType == mcLiteral || CType == mcIntlLiteral){
				Result = Result.SubString(0, Offset) +
					MaskIntlLiteralToChar(EditMask.CharAt(MaskOffset)) +
					Result.SubString(Offset + 1);
				Offset++;
			}
			else if(CType == mcMask || CType == mcMaskOpt){
				if(Offset > Result.Length())
					Result = Result + Blank;
				Offset++;
			}
		}
	}
	else{
		//starting at the end, insert literal chars in the string and add spaces at the beginning 
		INT Offset = Result.Length();
		for(INT I = 1; I <= EditMask.Length(); I++){
			INT MaskOffset = EditMask.Length() - I;
			TMaskCharType CType = MaskGetCharType(EditMask, MaskOffset);
			if(CType == mcLiteral || CType == mcIntlLiteral){
				Result = Result.SubString(1, Offset) +
					MaskIntlLiteralToChar(EditMask.CharAt(MaskOffset)) +
					Result.SubString(Offset + 1);
			}
			else if(CType == mcMask || CType == mcMaskOpt){
				if(Offset < 1)
					Result = Blank + Result;
				else
					Offset--;
			}
		}
	}
}

BOOL IsLiteralChar(const String& EditMask, INT Offset){
	BOOL Result = FALSE;
	INT MaskOffset = OffsetToMaskOffset(EditMask, Offset);
	if(MaskOffset >= 0){
		TMaskCharType CType = MaskGetCharType(EditMask, MaskOffset);
		Result = CType == mcLiteral || CType == mcIntlLiteral;
	}
}

BOOL MaskGetMaskSave(String& EditMask){
	BOOL Result = TRUE;
	if(EditMask.Length() >= 4){
		INT Sep1 = -1;
		INT Sep2 = -1;
		INT I = EditMask.Length();
		while(Sep2 < 0){
			if(MaskGetCharType(EditMask, I) ==  mcFieldSeparator){
				if(Sep1 < 0)
					Sep1 = I;
				else
					Sep2 = I;
			}
			I--;
			if(I <= 0 || I < EditMask.Length() - 4)
				break;
		}
		if(Sep2 < 0)
			Sep2 = Sep1;
		if(Sep2 > 0 && Sep2 != EditMask.Length())
			Result = !(EditMask.CharAt(Sep2 + 1) == MaskNoSave);
	}
}

TCHAR MaskGetMaskBlank(String& EditMask){
	TCHAR Result = DefaultBlank;
	if(EditMask.Length() >= 4){
		if(MaskGetCharType(EditMask, EditMask.Length() - 1) == mcFieldSeparator){
			//in order for blank specifier to be valid, there must also be a save specifier 
			if(MaskGetCharType(EditMask, EditMask.Length() - 2) == mcFieldSeparator || 
				MaskGetCharType(EditMask, EditMask.Length() - 3) == mcFieldSeparator){
					Result = EditMask.CharAt(EditMask.Length() - 1);
			}
		}
	}
}

String PadSubField(String& EditMask, String& Value, INT StartFld, 
	INT StopFld, INT Len, TCHAR Blank){
	if(StopFld - StartFld < Len){
		//found literal at position J, now pad it
		TMaskDirectives Dir = MaskGetCurrentDirectives(EditMask, 0);
		INT StartPad = StopFld - 1;
		if(IN_TEST(mdReverseDir, Dir))
			StartPad = StartFld - 1;
		String Result = Value.SubString(0, StartPad + 1);
		for(INT K = 0; K < Len - (StopFld - StartFld); K++)
			Result = Result + Blank;
		return Result + Value.SubString(StartPad + 1);
	}
	else if (StopFld - StartFld > Len){
		TMaskDirectives Dir = MaskGetCurrentDirectives(EditMask, 0);
		if(IN_TEST(mdReverseDir, Dir))
			return Value.SubString(0, StartFld) +
				Value.SubString(StopFld - Len);
		else
			return Value.SubString(0, StartFld + Len) + Value.SubString(StopFld);
	}
	else
		return Value;
}

String PadInputLiterals(String& EditMask, String& Value, TCHAR Blank){
	INT LastLiteral = 0;
	String Result = Value;
	for(INT MaskOffset = 1; MaskOffset < EditMask.Length(); MaskOffset++){
		TMaskCharType CType = MaskGetCharType(EditMask, MaskOffset);
		if(CType == mcLiteral || CType == mcIntlLiteral){
			INT Offset = MaskOffsetToOffset(EditMask, MaskOffset);
			INT EndSubFld = Result.Length() + 1;
			for(INT J = LastLiteral + 1; J < Result.Length(); J++){
				if(Result.CharAt(J) == MaskIntlLiteralToChar(EditMask.CharAt(MaskOffset))){
					EndSubFld = J;
					break;
				}
			}
			//we have found a subfield, ensure that it complies 
			if(EndSubFld > Result.Length())
				Result = Result + MaskIntlLiteralToChar(EditMask.CharAt(MaskOffset));
				Result = PadSubField(EditMask, Result, LastLiteral + 1, EndSubFld,
					Offset - (LastLiteral + 1), Blank);
				LastLiteral = Offset;
		}
	}
	
	//ensure that the remainder complies, too
	INT MaxChars = MaskOffsetToOffset(EditMask, EditMask.Length());
	if(Result.Length() != MaxChars)
		Result = PadSubField(EditMask, Result, LastLiteral + 1, Result.Length() + 1,
			MaxChars - LastLiteral, Blank);
		//replace non-literal blanks with blank char
	for(INT Offset = 1; Offset < Result.Length(); Offset++){
		if(Result.CharAt(Offset) == ' '){
			if (!IsLiteralChar(EditMask, Offset - 1))
				Result.SetCharAt(Offset, Blank);
		}
	}
}
//*/