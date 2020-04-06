#include "stdinc.h"
#include "String.hpp"


ReferedStr::ReferedStr(LPTSTR Text,INT AStart, INT AEnd):Buf(NULL),RefCount(0),Length(0), Hash(0){
	if(Text != NULL){
		ULONG Start = AStart < 0 ? 0 : AStart;
		ULONG End = AEnd < 0 ? lstrlen(Text) : AEnd;
		Length = lstrlen(Text);
		if((Start != 0 && Start >= Length) || Start > End || End > Length)
			throw "invalid string start/end/length.";
		Length = End - Start;
		INT Size = (Length + 1)* sizeof(TCHAR);
		Buf = (LPTSTR)malloc(Size);
		if(lstrcpyn(Buf, Text + Start, Length + 1) == NULL)
			throw "invalid string allocation.";
		*(Buf + Length) = TCHAR('\0');
		Hash = GetHashCode(Buf, Size);
	}
	Reference();
}

ReferedStr::~ReferedStr(){
	if(Buf != NULL){
		free(Buf);
		Buf = NULL;
	}
}

ULONG ReferedStr::GetReference(){
	return RefCount;
}
ULONG ReferedStr::Reference(){
	return ::InterlockedIncrement(&RefCount);
}
ULONG ReferedStr::Release(){
	ULONG Ret = ::InterlockedDecrement(&RefCount);
	if(Ret == 0)
		delete this;
	return Ret;
}

INT ReferedStr::IndexOf(TCHAR Value){
	INT Result = -1;
	if(Buf != NULL){
		LPTSTR Ret = _tcschr(Buf, Value);
		if(Ret != NULL){
			Result = (INT)(Ret - Buf);
		}
	}
	return Result;
}

INT ReferedStr::Compare(ReferedStr& Other, BOOL caseSensitive){
	INT Result = -1;
	if(this == (&Other) || (Length == Other.Length && Length == 0)){
		Result = 0;
	}
	else if(Buf == NULL && Other.Buf != NULL){
		Result = -1;
	}
	else if(Buf != NULL && Other.Buf == NULL){
		Result = 1;
	}
	else {
		return caseSensitive ? lstrcmp(Buf, Other.Buf) : lstrcmpi(Buf, Other.Buf);
		/*
		ULONG L1 = Length;
		ULONG L2 = Length;
		LPTSTR _S = Buf;
		LPTSTR _S1 = Other.Buf;
		ULONG LEN = L1 > L2 ? L2 : L1;
		Result = 0;
		while(Result == 0 && LEN > 0){
			Result = (*_S) - (*_S1);
			LEN--;
		}
		if(Result == 0)
			Result = L1 - L2;
		//*/
	}
	return Result;
}

String String::CreateFor(LPTSTR Buf){
	String S(NULL, -1, -1);
	S._Val->Buf = Buf;
	S._Val->Length = lstrlen(Buf);
	S._Val->Hash = GetHashCode(S._Val->Buf, (S._Val->Length + 1) * sizeof(TCHAR));
	return S;
}

String::String(LPTSTR Text, INT Start, INT End){
	_Val = new ReferedStr(Text, Start, End);
}

String::String(String& Source, INT Start, INT End){
	if(Start < 0 && End < 0){
		_Val = Source._Val;
		if(_Val != NULL){
			_Val->Reference();
		}
	}
	else {
		_Val = new ReferedStr(Source.GetBuffer(), Start, End);
	}
}

String::~String(){
	if(_Val != NULL){
		_Val->Release();
	}
}

String& String::operator=(String& Source){
	if(_Val != NULL){
		_Val->Release();
	}
	_Val = Source._Val;
	if(_Val != NULL){
		_Val->Reference();
	}
	return *this;
}

BOOL String::operator==(String& Other){
	return this == &Other && Compare(Other) == 0;
}

BOOL String::operator!=(String& Other){
	return this != &Other && Compare(Other) != 0;
}

BOOL String::operator==(LPTSTR Other){
	BOOL ThisEmpty = _Val == NULL || _Val->Buf == NULL || _Val->Length == 0;
	BOOL OtherEmpty = Other == NULL || lstrlen(Other) == 0;
	if(ThisEmpty && OtherEmpty)
		return TRUE;
	else if(ThisEmpty && !OtherEmpty || !ThisEmpty && OtherEmpty)
		return FALSE;
	else{
		return lstrcmp(_Val->Buf, Other) == 0;
	}
}
BOOL String::operator!=(LPTSTR Other){
	return !(*this == Other);
}

String operator+(String &a1, String &a2){
	INT Len = a1.Length() + a2.Length();
	if(Len == 0)
		return String(NULL);
	else if(a1.Length() == 0){
		return String(a2);
	}
	else if(a2.Length() == 0){
		return String(a1);
	}
	else{
		INT Size = (Len + 1) * sizeof(TCHAR);
		LPTSTR Buf = (LPTSTR)malloc(Size);
		lstrcpyn(Buf, a1.GetBuffer(), a1.Length() + 1);
		lstrcpyn(Buf + a1.Length(), a2.GetBuffer(), a2.Length() + 1);
		*(Buf + Len) = TCHAR('\0');
		return String::CreateFor(Buf);
	}
}

String operator+(String &a1, LPTSTR a2){
	return a1 + String(a2);
}

String operator+(LPTSTR a1, String &a2){
	return String(a1) + a2;
}

String operator+(String &a1, TCHAR a2){
	INT Len = a1.Length() + 1;
	INT Size = (Len + 1) * sizeof(TCHAR);
	LPTSTR Buf = (LPTSTR)malloc(Size);
	if(a1.Length() > 0)
		lstrcpyn(Buf, a1.GetBuffer(), a1.Length() + 1);
	*(Buf + Len - 1) = a2;
	*(Buf + Len) = TCHAR('\0');
	return String::CreateFor(Buf);
}

String operator+(TCHAR a1, String &a2){
	INT Len = a2.Length() + 1;
	INT Size = (Len + 1) * sizeof(TCHAR);
	LPTSTR Buf = (LPTSTR)malloc(Size);
	if(a2.Length() > 0)
		lstrcpyn(Buf + 1, a2.GetBuffer(), a2.Length() + 1);
	*Buf = a1;
	*(Buf + Len) = TCHAR('\0');
	return String::CreateFor(Buf);
}

BOOL String::Assign(LPTSTR Text, INT Start, INT End){
	if(_Val != NULL){
		_Val->Release();
	}
	_Val = new ReferedStr(Text, Start, End);
	return TRUE;
}

INT String::Compare(String& Other, BOOL caseSensitive){
	INT Result = -1;
	if(this == (&Other) || _Val == NULL && Other._Val == NULL || Length() == 0  && Other.Length() == 0){
		Result = 0;
	}
	else if(_Val == NULL && Other._Val != NULL){
		Result = -1;
	}
	else if(_Val != NULL && Other._Val == NULL){
		Result = 1;
	}
	else {
		Result = _Val->Compare(*(Other._Val), caseSensitive);
	}
	return Result;
}

INT String::IndexOf(TCHAR Value){
	return _Val == NULL ? -1 : _Val->IndexOf(Value);
}

String String::SubString(INT Start, INT End){
	return String(*this, Start, End);
}

void String::Delete(INT Start, INT Len){
	if(_Val != NULL && Start >= 0 && Len > 0 && _Val->Length > 0 && Start < (INT)_Val->Length){
		/*TODO
		if(_Val->GetReference() <= 1){
			lstrcpyn(
		}
		else if(_Val->Length > Len)//*/
		{
			INT NLen = _Val->Length - Len + 1;
			INT Size = NLen * sizeof(TCHAR);
			LPTSTR NBuf = (LPTSTR)malloc(Size);
			ZeroMemory(NBuf, Size);
			lstrcpyn(NBuf, _Val->Buf, Start + 1);
			lstrcpyn(NBuf + Start, _Val->Buf + Start + Len, NLen - Start);
			_Val->Release();
			_Val = new ReferedStr(NBuf);
		}
	}
}

void String::Insert(String& Str, INT Index){
	if(_Val != NULL && Index >= 0 && Index <= (INT)_Val->Length){
		//if(_Val->GetReference() <= 1){
		//
		//}
		//else{
			INT NLen = _Val->Length + Str.Length() + 1;
			INT Size = NLen * sizeof(TCHAR);
			LPTSTR NBuf = (LPTSTR)malloc(Size);
			ZeroMemory(NBuf, Size);
			lstrcpyn(NBuf, _Val->Buf, Index + 1);
			lstrcpyn(NBuf + Index, Str.GetBuffer(), Str.Length() + 1);
			lstrcpyn(NBuf + Index + Str.Length(), _Val->Buf + Index, _Val->Length - Index + 1);
			_Val->Release();
			_Val = new ReferedStr(NBuf);
		//}
	}
	else
		throw "invalid index.";//
}

ULONG String::Length(){
	return _Val == NULL ? 0 : _Val->Length;
}

const LPTSTR String::GetBuffer(){
	return _Val == NULL ? NULL : _Val->Buf;
}
TCHAR String::CharAt(INT Offset){
	if(_Val != NULL && Offset >= 0 && Offset < (INT)_Val->Length)
		return *(_Val->Buf + Offset);
	else throw "invalid offset";
}

TCHAR String::SetCharAt(INT Offset, TCHAR C){
	if(_Val != NULL && Offset >= 0 && Offset < (INT)_Val->Length){
		TCHAR OC = *(_Val->Buf + Offset);
		*(_Val->Buf + Offset) = C;
		return OC;
	}
	else throw "invalid offset";
}
