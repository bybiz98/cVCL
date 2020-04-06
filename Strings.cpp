#include "stdinc.h"
#include "Strings.hpp"
#include "WinUtils.hpp"
#include "SysInit.hpp"
#include "String.hpp"

IMPL_DYN_CLASS(CStrings)
CStrings::CStrings():Defined(0), 
	Delimiter(TCHAR('\n')),
	QuoteChar(TCHAR('"')),
	NameValueSeparator(TCHAR('=')),
	UpdateCount(0){
}
CStrings::~CStrings(){
}

void CStrings::Error(LPTSTR Msg, INT Data){
	throw Msg;
}

void CStrings::Error(PResStringRec Msg, INT Data){
	throw "";
}

String CStrings::ExtractName(String& S){
	INT P = S.IndexOf(NameValueSeparator);
	if(P >= 0)
		return S.SubString(0, P);
	else return String();
}

void CStrings::SetUpdateState(BOOL Updating){
}

INT CStrings::CompareStrings(String& S1, String& S2){
	return S1.Compare(S2);
}

INT CStrings::Add(String& S){
	INT Result = GetCount();
	Insert(Result, S);
	return Result;
}

INT CStrings::AddObject(String& S, CObject* AObject){
	INT Result = Add(S);
	PutObject(Result, AObject);
	return Result;
}

void CStrings::Append(String& S){
	Add(S);
}

void CStrings::AddStrings(CStrings& Strings){
	CMethodLock UpdateLock(this, (TLockMethod)&CStrings::BeginUpdate, (TLockMethod)&CStrings::EndUpdate);
	INT Count = Strings.GetCount();
	for(INT I = 0; I < Count; I++)
		AddObject(Strings.Get(I), Strings.GetObject(I));
}

void CStrings::Assign(CStrings& Strings){
	CMethodLock UpdateLock(this, (TLockMethod)&CStrings::BeginUpdate, (TLockMethod)&CStrings::EndUpdate);
	Clear();
	Defined = Strings.Defined;
	NameValueSeparator = Strings.NameValueSeparator;
	QuoteChar = Strings.QuoteChar;
	Delimiter = Strings.Delimiter;
	AddStrings(Strings);
}

void CStrings::BeginUpdate(){
	if(UpdateCount == 0)
		SetUpdateState(TRUE);
	UpdateCount++;
}

void CStrings::EndUpdate(){
	UpdateCount--;
	if(UpdateCount == 0)
		SetUpdateState(FALSE);
}

BOOL CStrings::Equals(CStrings& Strings){
	INT Count = GetCount();
	if(Count != Strings.GetCount())
		return FALSE;
	for(INT I = 0; I < Count; I++)
		if(Get(I) != Strings.Get(I))
			return FALSE;
	return TRUE;
}

void CStrings::Exchange(INT Index1, INT Index2){
	CMethodLock UpdateLock(this, (TLockMethod)&CStrings::BeginUpdate, (TLockMethod)&CStrings::EndUpdate);
	String TempString = Get(Index1);
	CObject* TempObject = GetObject(Index1);
	Put(Index1, Get(Index2));
	PutObject(Index1, GetObject(Index2));
	Put(Index2, TempString);
	PutObject(Index2, TempObject);
}

LPTSTR CStrings::GetText(){
	return NULL;
}

INT CStrings::IndexOf(String& S){
	INT Count = GetCount();
	for(INT I = 0; I < Count; I++)
		if(CompareStrings(Get(I), S) == 0)
			return I;
	return -1;
}

INT CStrings::IndexOfName(String& Name){
	INT Count = GetCount();
	String S;
	for(INT I = 0; I < Count; I++){
		S = Get(I);
		INT P = S.IndexOf(NameValueSeparator);
		if(P > 0 && CompareStrings(S.SubString(0, P), Name) == 0)
			return I;
	}
	return -1;
}

INT CStrings::IndexOfObject(CObject* AObject){
	INT Count = GetCount();
	for(INT Result = 0; Result < Count; Result++)
		if(GetObject(Result) == AObject)
			return Result;
	return -1;
}

void CStrings::InsertObject(INT Index, String& S, CObject* AObject){
	Insert(Index, S);
	PutObject(Index, AObject);
}

void CStrings::LoadFromFile(LPTSTR FileName){
	CFileStream Stream(FileName, fmOpenRead | fmShareDenyWrite);
	LoadFromStream(&Stream);
}

void CStrings::LoadFromStream(CStream* Stream){
	CMethodLock UpdateLock(this, (TLockMethod)&CStrings::BeginUpdate, (TLockMethod)&CStrings::EndUpdate);
	size_t Size = Stream->GetSize() - Stream->GetPosition();
	LPTSTR Buf = (LPTSTR)malloc(Size + sizeof(TCHAR));
	*(Buf + Size) = TCHAR('\0');
    Stream->Read(Buf, Size);
	SetTextStr(String::CreateFor(Buf));
}

void CStrings::Move(INT CurIndex, INT NewIndex){
	if(CurIndex != NewIndex){
		CMethodLock UpdateLock(this, (TLockMethod)&CStrings::BeginUpdate, (TLockMethod)&CStrings::EndUpdate);
		String TempString = Get(CurIndex);
		CObject* TempObject = GetObject(CurIndex);
		Delete(CurIndex);
		InsertObject(NewIndex, TempString, TempObject);
	}
}

void CStrings::SaveToFile(LPTSTR FileName){
	CFileStream Stream(FileName, fmCreate);
    SaveToStream(&Stream);
}

void CStrings::SaveToStream(CStream* Stream){
	String S = GetTextStr();
	Stream->WriteBuffer(S.GetBuffer(), S.Length() * sizeof(TCHAR));
}

void CStrings::SetText(LPTSTR Text){
	SetTextStr(String::CreateFor(Text));
}

INT CStrings::GetCapacity(){
	return GetCount();
}

void CStrings::SetCapacity(INT NewCapacity){
	// do nothing - descendents may optionally implement this method
}

String CStrings::GetCommaText(){
	TStringsDefined OldDefined = Defined;
	TCHAR OldDelimiter = Delimiter;
	TCHAR OldQuoteChar = QuoteChar;
	Delimiter = TCHAR(',');
	QuoteChar = TCHAR('"');
    String S = GetDelimitedText();
    Delimiter = OldDelimiter;
    QuoteChar = OldQuoteChar;
	Defined = OldDefined;
	return S;
}

void CStrings::SetCommaText(String& Value){
	Delimiter = TCHAR(',');
	QuoteChar = TCHAR('"');
	SetDelimitedText(Value);
}

TCHAR CStrings::GetDelimiter(){
	if(!IN_TEST(sdDelimiter, Defined))
		SetDelimiter(TCHAR(','));
	return Delimiter;
}

void CStrings::SetDelimiter(TCHAR Value){
	if(Delimiter != Value || !IN_TEST(sdDelimiter, Defined)){
		Defined |= sdDelimiter;
		Delimiter = Value;
	}
}

INT QuotedStr(LPTSTR Des, LPTSTR Src, TCHAR Quote){
	INT AddCount = 0;
	LPTSTR P = _tcschr(Src, Quote);
	while(P != NULL){
		P++;
		AddCount++;
		P = _tcschr(Src, Quote);
	}
	INT Len = lstrlen(Src);
	if(AddCount == 0){
		if(Des != NULL){
			*Des = Quote;
			Des++;
			lstrcpyn(Des, Src, Len + 1);
			Des += Len;
			*Des = Quote;
		}
		return Len + 2;
	}
	if(Des != NULL){
		*Des = Quote;
		Des++;
		LPTSTR P = _tcschr(Src, Quote);
		do{
			P++;
			lstrcpyn(Des, Src, (INT)(P - Src) + 1);
			Des += (INT)(P - Src);
			*Des = Quote;
			Des++;
			Src = P;
			P = _tcschr(Src, Quote);
		}while(P != NULL);
		P = Src + lstrlen(Src);
		lstrcpyn(Des, Src, (INT)(P - Src) + 1);
		Des += (INT)(P - Src);
		*Des = Quote;
	}
	return Len + AddCount + 2;
}

String CStrings::GetDelimitedText(){
	INT Count = GetCount();
	if(Count == 0 || Count == 1 && Get(0) == TEXT("")){
		TCHAR aa[2] = {QuoteChar, QuoteChar};
		return String(aa);
	}
	else{
		INT Len = 0;
		for(INT I = 0; I < Count; I++){
			String S = Get(I);
			LPTSTR Buf = S.GetBuffer();
			LPTSTR P = Buf;
			while(!(*P >= TCHAR('\0') && *P < TCHAR(' ') || *P == QuoteChar || *P == Delimiter))
				P++;
			if(*P != TCHAR('\0'))
				Len += QuotedStr(NULL, Buf, QuoteChar);
			else
				Len += S.Length();
			Len += 1;//for Delimiter
		}
		//Len--;
		INT Size = (Len + 1) * sizeof(TCHAR);
		LPTSTR RetBuf = (LPTSTR)malloc(Size);
		LPTSTR Des = RetBuf;
		for(INT I = 0; I < Count; I++){
			String S = Get(I);
			LPTSTR Buf = S.GetBuffer();
			LPTSTR P = Buf;
			while(!(*P >= TCHAR('\0') && *P < TCHAR(' ') || *P == QuoteChar || *P == Delimiter))
				P++;
			if(*P != TCHAR('\0')){
				INT L = QuotedStr(Des, Buf, QuoteChar);
				Des += L;
			}
			else {
				lstrcpyn(Des, Buf, S.Length() + 1);
				Des += S.Length();
			}
			*Des = Delimiter;
			Des++;
		}
		/*
		Result := '';
    for I := 0 to Count - 1 do
    begin
      S := Get(I);
      P := PChar(S);
      while not (P^ in [#0..' ', QuoteChar, Delimiter]) do
      {$IFDEF MSWINDOWS}
        P := CharNext(P);
      {$ELSE}
        Inc(P);
      {$ENDIF}
      if (P^ <> #0) then S := AnsiQuotedStr(S, QuoteChar);
      Result := Result + S + Delimiter;
    end;
    System.Delete(Result, Length(Result), 1);
	//*/
		return String(NULL);
	}
}

String ExtractQuotedStr(LPTSTR Src, TCHAR Quote){
	if(Src == NULL || *Src != Quote)
		return String();
	Src++;
	INT DropCount = 0;
	LPTSTR P = Src;
	Src = _tcschr(Src, Quote);
	while(Src != NULL){   // count adjacent pairs of quote chars
		Src++;
		if(*Src != Quote)
			break;
		Src++;
		DropCount++;
		Src = _tcschr(Src, Quote);;
	}
	if(Src == NULL)
		Src = P + lstrlen(P);
	if((Src - P) <= 0)
		return String();
	if(DropCount == 0)
		return String(P, 0, (INT)(Src - P));
	else{
		INT Len = INT(Src - P) - DropCount;
		INT Size = (Len + 1) * sizeof(TCHAR);
		LPTSTR Buf = (LPTSTR)malloc(Size);
		LPTSTR Dest = Buf;
		Src = _tcschr(P, Quote);
		while(Src != NULL){
			Src++;
			if(*Src != Quote)
				break;
			lstrcpyn(Dest, P, (INT)(Src - P) + 1);
			Dest += Src - P;
			Src++;
			P = Src;
			Src = _tcschr(Src, Quote);
		}
		if(Src == NULL)
			Src = P + lstrlen(P);
		lstrcpyn(Dest, P, (INT)(Src - P) + 1);
		return String::CreateFor(Buf);
	}
}

void CStrings::SetDelimitedText(String& Value){
	CMethodLock UpdateLock(this, (TLockMethod)&CStrings::BeginUpdate, (TLockMethod)&CStrings::EndUpdate);
    Clear();
	LPTSTR P = Value.GetBuffer();
	if(P == NULL)
		return;
	while(*P >= TCHAR('\1') && *P <= TCHAR(' '))
		P++;

	while(*P != TCHAR('\0')){
		if(*P == QuoteChar)
			Add(ExtractQuotedStr(P, QuoteChar));
		else {
			LPTSTR P1 = P;
			while(*P > TCHAR(' ') && *P != Delimiter)
				P++;
			Add(String(P1, 0, (INT)(P - P1)));
		}
		while(*P >= TCHAR('\1') && *P <= TCHAR(' '))
			P++;
		if(*P == Delimiter){
			LPTSTR P1 = P + 1;
			if(*P1 == TCHAR('\0'))
				Add(String(TEXT("")));
			do{
				P++;
			}while(*P >= TCHAR('\1') && *P <= TCHAR(' '));
		}
	}
}

String CStrings::GetName(INT Index){
	return ExtractName(Get(Index));
}

CObject* CStrings::GetObject(INT Index){
	return NULL;
}

void CStrings::PutObject(INT Index, CObject* AObject){
}

TCHAR CStrings::GetQuoteChar(){
	if(IN_TEST(sdQuoteChar, Defined))
		QuoteChar = TCHAR('"');
	return QuoteChar;
}

void CStrings::SetQuoteChar(TCHAR Value){
	if(QuoteChar != Value || !IN_TEST(sdQuoteChar, Defined)){
		Defined |= sdQuoteChar;
		QuoteChar = Value;
	}
}

String CStrings::GetValue(String& Name){
	INT I = IndexOfName(Name);
	if(I >= 0)
		return String(Get(I), Name.Length() + 1, -1);
	else return String();
}

void CStrings::SetValue(String& Name, String& Value){
	INT I = IndexOfName(Name);
	if(Value != TEXT("")){
		if(I < 0)
			Add(String());
		Put(I, Name + NameValueSeparator + Value);
	}
	else {
		if(I >= 0)
			Delete(I);
	}
}

String CStrings::GetValueFromIndex(INT Index){
	if(Index >= 0)
		return String(Get(Index), GetName(Index).Length() + 1, -1);
	else return String();
}

void CStrings::SetValueFromIndex(INT Index, String& Value){
	if(Value != TEXT("")){
		if(Index < 0)
			Index = Add(String());
		Put(Index, GetName(Index) + NameValueSeparator + Value);
	}
	else 
		if(Index >= 0)
			Delete(Index);
}

TCHAR CStrings::GetNameValueSeparator(){
	if(!IN_TEST(sdNameValueSeparator, Defined))
		NameValueSeparator = TCHAR('=');
	return NameValueSeparator;
}

void CStrings::SetNameValueSeparator(TCHAR Value){
	if(NameValueSeparator != Value || !IN_TEST(sdNameValueSeparator, Defined)){
		Defined |= sdNameValueSeparator;
		NameValueSeparator = Value;
	}
}

void CStrings::Put(INT Index, String& S){
	CObject* TempObject = GetObject(Index);
	Delete(Index);
	InsertObject(Index, S, TempObject);
}

String CStrings::GetTextStr(){
	INT Count = GetCount();
	INT Len = 0;
	LPTSTR LB = GetGlobal().GetLineBreak();
	INT LBLen = lstrlen(LB);
	for(INT I = 0; I < Count; I++)
		Len += Get(I).Length() + LBLen;
	INT Size = (Len + 1) * sizeof(TCHAR);
	LPTSTR Buf = (LPTSTR)malloc(Size);
	LPTSTR P = Buf;
	for(INT I = 0; I < Count; I++){
		String S = Get(I);
		INT L = S.Length();
		if(L != 0){
			lstrcpyn(P, S.GetBuffer(), L + 1);
			P += L;
		}
		if(LBLen != 0){
			lstrcpyn(P, LB, LBLen + 1);
			P += LBLen;
		}
	}
	*(Buf + Len) = TCHAR('\0');
	return String::CreateFor(Buf);
}

void CStrings::SetTextStr(String& Value){
	CMethodLock UpdateLock(this, (TLockMethod)&CStrings::BeginUpdate, (TLockMethod)&CStrings::EndUpdate);
    Clear();
	LPTSTR P = Value.GetBuffer();
	if(P != NULL){
		while(*P != TCHAR('\0')){
			LPTSTR Start = P;
			while(*P != TCHAR('\0') && *P != TCHAR('\n') && *P != TCHAR('\r'))
				P++;
			Add(String(Start, 0, (INT)(P - Start)));
			if(*P == TCHAR('\r'))
				P++;
			if(*P == TCHAR('\n'))
				P++;
		}
	}
}

IMPL_DYN_CLASS(CStringList)
CStringList::CStringList():List(NULL),
	Count(0),
	Capacity(0),
	Sorted(FALSE),
	Duplicates(dupIgnore),
	CaseSensitive(FALSE),
	INIT_EVENT(Change),
	INIT_EVENT(Changing){
}

CStringList::~CStringList(){
	SetOnChange(NULL, NULL);
	SetOnChanging(NULL, NULL);
	if(Count != 0)
		Finalize(0, Count);
	Count = 0;
	SetCapacity(0);
}

void CStringList::ExchangeItems(INT Index1, INT Index2){
	PStringItem Item1 = &(*List)[Index1];
	PStringItem Item2 = &(*List)[Index2];
	String* Temp = Item1->string;
	Item1->string = Item2->string;
	Item2->string = Temp;
	CObject* Obj = Item1->Object;
	Item1->Object = Item2->Object;
	Item2->Object = Obj;
}

void CStringList::Grow(){
	INT Delta = 0;
	if(Capacity > 64)
		Delta = Capacity / 4;
	else
		if(Capacity > 8)
			Delta = 16;
		else
			Delta = 4;
	SetCapacity(Capacity + Delta);
}

void CStringList::QuickSort(INT L, INT R, TStringListSortCompare SCompare){
	INT I = L;
	INT J = R;
	do{
		INT P = (L + R) >> 1;
		do{
			while(SCompare(this, I, P) < 0)
				I++;
			while(SCompare(this, J, P) > 0)
				J--;
			if(I <= J){
				ExchangeItems(I, J);
				if(P == I)
					P = J;
				else if(P == J)
					P = I;
				I++;
				J--;
			}
		}while(I <= J);
		if(L < J)
			QuickSort(L, J, SCompare);
		L = I;
	}while(I < R);
}

void CStringList::Finalize(INT StartIndex, INT EndIndex){
	for(INT I = StartIndex; I < EndIndex; I++){
		if((*List)[I].string != NULL)
			delete (*List)[I].string;
		(*List)[I].string = NULL;
		(*List)[I].Object = NULL;
	}
}

void CStringList::Changed(){
	if(GetUpdateCount() == 0 && (OnChange != NULL))
		CALL_EVENT(Change)(this);
}

void CStringList::Changing(){
	if(GetUpdateCount() == 0 && OnChanging != NULL)
		CALL_EVENT(Changing)(this);
}

String CStringList::Get(INT Index){
	if (Index < 0 && Index >= Count)
		throw "list index error.";//Error(@SListIndexError, Index);
	return *((*List)[Index].string);
}

INT CStringList::GetCapacity(){
	return Capacity;
}

INT CStringList::GetCount(){
	return Count;
}

CObject* CStringList::GetObject(INT Index){
	if(Index < 0 && Index >= Count)
		throw "list index error.";//(@SListIndexError, Index);
	return (*List)[Index].Object;
}

void CStringList::Put(INT Index, String& S){
	if(GetSorted())
		throw "sorted list error."; //Error(@SSortedListError, 0);
	if(Index < 0 && Index >= Count)
		throw "list index error."; //Error(@SListIndexError, Index);
	Changing();
	*((*List)[Index].string) = S;
	Changed();
}

void CStringList::PutObject(INT Index, CObject* AObject){
	if(Index < 0 && Index >= Count)
		throw "list index error."; //Error(@SListIndexError, Index);
	Changing();
	(*List)[Index].Object = AObject;
	Changed();
}

void CStringList::SetCapacity(INT NewCapacity){
	if(List == NULL){
		INT Size = NewCapacity * sizeof(TStringItem);
		List = (PStringItemList)malloc(Size);
		ZeroMemory(List, Size);
	}
	else{
		INT OldSize = Capacity * sizeof(TStringItem);
		INT NewSize = NewCapacity * sizeof(TStringItem);
		if(Count > NewCapacity){
			Finalize(NewCapacity, Count);
		}
		List = (PStringItemList)realloc(List, NewSize);
		if(OldSize < NewSize)
			ZeroMemory((LPBYTE)List + OldSize, NewSize - OldSize);
	}
	Capacity = NewCapacity;
}

void CStringList::SetUpdateState(BOOL Updating){
	if(Updating)
		Changing();
	else 
		Changed();
}

INT CStringList::CompareStrings(String& S1, String& S2){
	return S1.Compare(S2, GetCaseSensitive());
}

void CStringList::InsertItem(INT Index, String& S, CObject* AObject){
	Changing();
	if(Count == Capacity)
		Grow();
	if(Index < Count)
		MoveMemory(&(*List)[Index + 1], &(*List)[Index], (Count - Index) * sizeof(TStringItem));
	PStringItem Item = &(*List)[Index];
	Item->string = new String(S);
	Item->Object = AObject;
	Count++;
	Changed();
}

INT CStringList::Add(String& S) {
	return AddObject(S, NULL);
}

INT CStringList::AddObject(String& S, CObject* AObject) {
	INT Result = 0;
	if(!GetSorted())
		Result = Count;
	else {
		if(Find(S, Result)){
			if(GetDuplicates() == dupIgnore)
				return Result;
			else if(GetDuplicates() == dupError)
				throw "duplicate string";//Error(@SDuplicateString, 0);
		}
	}
	InsertItem(Result, S, AObject);
	return Result;
}

void CStringList::Clear() {
	if(Count != 0){
		Changing();
		Finalize(0, Count);
		Count = 0;
		SetCapacity(0);
		Changed();
	}
}

void CStringList::Delete(INT Index) {
	if(Index < 0 || Index >= Count)
		throw "list index error."; // Error(@SListIndexError, Index);
	Changing();
	Finalize(Index, Index + 1);
	Count--;
	if(Index < Count)
		MoveMemory(&(*List)[Index], &(*List)[Index + 1], (Count - Index) * sizeof(TStringItem));
	Changed();
}

void CStringList::Exchange(INT Index1, INT Index2) {
	if(Index1 < 0 && Index1 >= Count)
		throw "list index error."; //Error(@SListIndexError, Index1);
	if(Index2 < 0 || Index2 >= Count)
		throw "list index error"; //Error(@SListIndexError, Index2);
	Changing();
	ExchangeItems(Index1, Index2);
	Changed();
}

BOOL CStringList::Find(String& S, INT& Index){
	BOOL Result = FALSE;
	INT L = 0;
	INT H = Count - 1;
	while(L <= H){
		INT I = (L + H) >> 1;
		INT C = CompareStrings(*((*List)[I].string), S);
		if(C < 0)
			L = I + 1;
		else{
			H = I - 1;
			if(C == 0){
				Result = TRUE;
				if(GetDuplicates() != dupAccept)
					L = I;
			}
		}
	}
	Index = L;
	return Result;
}

INT CStringList::IndexOf(String& S) {
	if(!GetSorted())
		return __super::IndexOf(S);
	else{
		INT Result = 0;
		if(!Find(S, Result))
			Result = -1;
		return Result;
	}
}

void CStringList::Insert(INT Index, String& S) {
	InsertObject(Index, S, NULL);
}

void CStringList::InsertObject(INT Index, String& S, CObject* AObject) {
	if(GetSorted())
		throw "sorted list error.";//Error(@SSortedListError, 0);
	if(Index < 0 || Index > Count)
		throw "list index error."; //Error(@SListIndexError, Index);
	InsertItem(Index, S, AObject);
}

INT StringListCompareStrings(CStringList* List, INT Index1, INT Index2){
	return List->CompareStrings(*((*(List->List))[Index1].string),
                                *((*(List->List))[Index2].string));
}

void CStringList::Sort(){
	CustomSort(StringListCompareStrings);
}

void CStringList::CustomSort(TStringListSortCompare Compare){
	if(!GetSorted() && Count > 1){
		Changing();
		QuickSort(0, Count - 1, Compare);
		Changed();
	}
}

void CStringList::SetSorted(BOOL Value){
	if(Sorted != Value){
		if(Value)
			Sort();
		Sorted = Value;
	}
}

void CStringList::SetCaseSensitive(BOOL Value){
	if(Value != CaseSensitive){
		CaseSensitive = Value;
		if(GetSorted())
			Sort();
	}
}
