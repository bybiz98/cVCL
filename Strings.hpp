#pragma once

#include "Component.hpp"
#include "List.hpp"
#include "Messages.hpp"
#include "Stream.hpp"
#include "String.hpp"

typedef BYTE TStringsDefined;
#define sdDelimiter				0x1
#define sdQuoteChar				0x2
#define sdNameValueSeparator	0x4

class cVCL_API CStrings: public CObject{
private:
	TStringsDefined Defined;
	TCHAR Delimiter;
	TCHAR QuoteChar;
	TCHAR NameValueSeparator;
	INT UpdateCount;
	//IStringsAdapter Adapter;
protected:
	void Error(LPTSTR Msg, INT Data);
	void Error(PResStringRec Msg, INT Data);
	String ExtractName(String& S);
	virtual void SetUpdateState(BOOL Updating);
	//property UpdateCount: Integer read FUpdateCount;
	DEFINE_GETTER(INT, UpdateCount)
    virtual INT CompareStrings(String& S1, String& S2);
public:
    CStrings();
	virtual ~CStrings();

	virtual INT Add(String& S);
	virtual INT AddObject(String& S, CObject* AObject);
	void Append(String& S);
	virtual void AddStrings(CStrings& Strings);
	void Assign(CStrings& Strings);
	void BeginUpdate();
	virtual void Clear() = 0;
	virtual void Delete(INT Index) = 0;
	void EndUpdate();
	BOOL Equals(CStrings& Strings);
	virtual void Exchange(INT Index1, INT Index2);
	virtual LPTSTR GetText();
	virtual INT IndexOf(String& S);
	virtual INT IndexOfName(String& Name);
	virtual INT IndexOfObject(CObject* AObject);
	virtual void Insert(INT Index, String& S) = 0;
	virtual void InsertObject(INT Index, String& S, CObject* AObject);
	virtual void LoadFromFile(LPTSTR FileName);
	virtual void LoadFromStream(CStream* Stream);
	virtual void Move(INT CurIndex, INT NewIndex);
	virtual void SaveToFile(LPTSTR FileName);
	virtual void SaveToStream(CStream* Stream);
	virtual void SetText(LPTSTR Text);
	virtual INT GetCapacity();
    virtual void SetCapacity(INT NewCapacity);
	String GetCommaText();
	void SetCommaText(String& Value);
	virtual INT GetCount() = 0;
	TCHAR GetDelimiter();
	void SetDelimiter(TCHAR Value);
	String GetDelimitedText();
	void SetDelimitedText(String& Value);
	String GetName(INT Index);
	virtual CObject* GetObject(INT Index);
	virtual void PutObject(INT Index, CObject* AObject);
	TCHAR GetQuoteChar();
	void SetQuoteChar(TCHAR Value);
	String GetValue(String& Name);
	void SetValue(String& Name, String& Value);
	String GetValueFromIndex(INT Index);
	void SetValueFromIndex(INT Index, String& Value);
	TCHAR GetNameValueSeparator();
	void SetNameValueSeparator(TCHAR Value);
	virtual String Get(INT Index) = 0;
    virtual void Put(INT Index, String& S);
	virtual String GetTextStr();
	virtual void SetTextStr(String& Value);

	//DEFINE_GETTER(IStringsAdapter, Adapter)
	//void SetStringsAdapter(IStringsAdapter Value);

	REF_DYN_CLASS(CStrings)
};
DECLARE_DYN_CLASS_ABSTRACT(CStrings, CObject)

class CStringList;
typedef struct{
	String* string;
    CObject* Object;
} *PStringItem, TStringItem;
typedef TStringItem TStringItemList[];
typedef TStringItemList *PStringItemList;
typedef INT TStringListSortCompare(CStringList* List, INT Index1, INT Index2);
//INT StringListCompareStrings(CStringList* List, INT Index1, INT Index2);

class cVCL_API CStringList : public CStrings{
private:
	friend INT StringListCompareStrings(CStringList* List, INT Index1, INT Index2);
	PStringItemList List;
	INT Count;
	INT Capacity;
	BOOL Sorted;
	TDuplicates Duplicates;
	BOOL CaseSensitive;
	void ExchangeItems(INT Index1, INT Index2);
	void Grow();
	void QuickSort(INT L, INT R, TStringListSortCompare SCompare);
	void Finalize(INT StartIndex, INT EndIndex);
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Change)
	DECLARE_TYPE_EVENT(TNotifyEvent, Changing)
protected:
    virtual void Changed();
    virtual void Changing();
	INT GetCapacity() override;
	void SetCapacity(INT NewCapacity) override;
	void SetUpdateState(BOOL Updating) override;
	INT CompareStrings(String& S1, String& S2) override;
	virtual void InsertItem(INT Index, String& S, CObject* AObject);
public:
	CStringList();
	virtual ~CStringList();
	INT Add(String& S) override;
	INT AddObject(String& S, CObject* AObject) override;
	void Clear() override;
	void Delete(INT Index) override;
	void Exchange(INT Index1, INT Index2) override;
	virtual BOOL Find(String& S, INT& Index);
	INT IndexOf(String& S) override;
	void Insert(INT Index, String& S) override;
	void InsertObject(INT Index, String& S, CObject* AObject) override;
	virtual void Sort();
	virtual void CustomSort(TStringListSortCompare Compare);
	String Get(INT Index) override;
	INT GetCount() override;
	CObject* GetObject(INT Index) override;
	void Put(INT Index, String& S) override;
	void PutObject(INT Index, CObject* AObject) override;

	DEFINE_ACCESSOR(TDuplicates, Duplicates)
	DEFINE_GETTER(BOOL, Sorted)
	DEFINE_GETTER(BOOL, CaseSensitive)
	void SetSorted(BOOL Value);
	void SetCaseSensitive(BOOL Value);

	REF_DYN_CLASS(CStringList)
};
DECLARE_DYN_CLASS_ABSTRACT(CStringList, CStrings)
