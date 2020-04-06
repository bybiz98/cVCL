#pragma once

#include "stdinc.h"
#include "Object.hpp"

class ReferedStr{
private:
	friend class String;
	LPTSTR Buf;
	ULONG RefCount;
	ULONG Length;
	WORD Hash;
private:
	ReferedStr(LPTSTR Text = NULL, INT Start = -1, INT End = -1);
	virtual ~ReferedStr();
	ULONG GetReference();
	ULONG Reference();
	ULONG Release();
	INT Compare(ReferedStr& Other, BOOL caseSensitive = FALSE);
	INT IndexOf(TCHAR Value);
};

class cVCL_API String{
private:
	ReferedStr* _Val;
public:
	String(LPTSTR Text = NULL, INT Start = -1, INT End = -1);
	String(String& Source, INT Start = -1, INT End = -1);
	virtual ~String();
	static String CreateFor(LPTSTR Buf);
	String& operator=(String& Source);
	BOOL operator==(String& Other);
	BOOL operator==(LPTSTR Other);
	BOOL operator!=(String& Other);
	BOOL operator!=(LPTSTR Other);
	friend String operator+(String &a1, String &a2);
	friend String operator+(String &a1, LPTSTR a2);
	friend String operator+(LPTSTR a1, String &a2);
	friend String operator+(String &a1, TCHAR a2);
	friend String operator+(TCHAR a1, String &a2);
	BOOL Assign(LPTSTR Text = NULL, INT Start = -1, INT End = -1);
	INT Compare(String& Other, BOOL caseSensitive = FALSE);
	INT IndexOf(TCHAR Value);
	String SubString(INT Start, INT End = -1);
	void Delete(INT Start, INT Len);
	void Insert(String& Str, INT Index);
	ULONG Length();
	const LPTSTR GetBuffer();
	TCHAR CharAt(INT Offset);
	TCHAR SetCharAt(INT Offset, TCHAR C);
};