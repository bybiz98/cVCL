#pragma once
#include "stdinc.h"
#include "Object.hpp"
#include "Types.hpp"

//TSeekOrigin
#define soBeginning				0x00
#define soCurrent				0x01
#define soEnd					0x02

#define soFromBeginning			0x00
#define soFromCurrent			0x01
#define soFromEnd				0x02

class cVCL_API CStream : public CObject{
public:
	CStream();
	virtual ~CStream();
	virtual size_t Read(LPVOID Buffer, size_t Count) = 0;
	virtual size_t Write(const LPVOID Buffer, size_t Count) = 0;
	virtual size_t Seek(size_t Offset, WORD Origin) = 0;
	void ReadBuffer(LPVOID Buffer, size_t Count);
	void WriteBuffer(const LPVOID Buffer, size_t Count);
	size_t CopyFrom(CStream* Source, size_t Count);
	void WriteResourceHeader(const LPTSTR ResName, size_t FixupInfo);
	void FixupResourceHeader(size_t FixupInfo);
	void ReadResHeader();
	size_t GetPosition();
	void SetPosition(size_t Pos);
    virtual size_t GetSize();
	virtual void SetSize(size_t NewSize);

	REF_DYN_CLASS(CStream)
};
DECLARE_DYN_CLASS_ABSTRACT(CStream, CObject)


class cVCL_API CHandleStream : public CStream{
protected:
    HANDLE Handle;
public:
    CHandleStream(HANDLE AHandle = 0);
	virtual ~CHandleStream();
	size_t Read(LPVOID Buffer, size_t Count) override;
	size_t Write(const LPVOID Buffer, size_t Count) override;
	size_t Seek(size_t Offset, WORD Origin) override;
	void SetSize(size_t NewSize) override;
	HANDLE GetHandle();

	REF_DYN_CLASS(CHandleStream)
};
DECLARE_DYN_CLASS(CHandleStream, CStream)



#define fmCreate			0xFFFF
#define fmOpenRead			0x0000
#define fmOpenWrite			0x0001
#define fmOpenReadWrite		0x0002

// DOS compatibility mode is not portable
#define fmShareCompat		0x0000
#define fmShareExclusive	0x0010
#define fmShareDenyWrite	0x0020
// write-only not supported on all platforms
#define fmShareDenyRead		0x0030
#define fmShareDenyNone		0x0040

class cVCL_API CFileStream : public CHandleStream{
public:
	CFileStream();
	CFileStream(const LPTSTR FileName, WORD Mode, DWORD Rights = 0);
    virtual ~CFileStream();

	REF_DYN_CLASS(CFileStream)
};
DECLARE_DYN_CLASS(CFileStream, CHandleStream)

class cVCL_API CCustomMemoryStream : public CStream{
private:
    LPVOID Memory;
protected:
	size_t Size;
	size_t Position;
    void SetPointer(LPVOID Ptr, size_t Size);
public:
	CCustomMemoryStream();
	virtual ~CCustomMemoryStream();
	size_t Read(LPVOID Buffer, size_t Count) override;
	size_t Seek(size_t Offset, WORD Origin) override;
	void SaveToStream(CStream* Stream);
	void SaveToFile(const LPTSTR FileName);
	LPVOID GetMemory();

	REF_DYN_CLASS(CCustomMemoryStream)
};
DECLARE_DYN_CLASS_ABSTRACT(CCustomMemoryStream, CStream)

// Must be a power of 2
#define MemoryDelta				0x2000 
//Heap allocation flags, gmem_Moveable
#define HeapAllocFlags			2      

class cVCL_API CMemoryStream : public CCustomMemoryStream{
private:
	size_t Capacity;
protected:
    virtual LPVOID Realloc(size_t NewCapacity);
    size_t GetCapacity();
	void SetCapacity(size_t NewCapacity);
public:
	CMemoryStream();
	virtual ~CMemoryStream();
	void Clear();
	void LoadFromStream(CStream* Stream);
	void LoadFromFile(const LPTSTR FileName);
	void SetSize(size_t NewSize) override;
	size_t Write(const LPVOID Buffer, size_t Count) override;

	REF_DYN_CLASS(CMemoryStream)
};
DECLARE_DYN_CLASS(CMemoryStream, CCustomMemoryStream)

class cVCL_API CResourceStream : public CCustomMemoryStream{
private:
	HANDLE ResInfo;
	HANDLE HGlobal;
	void Initialize(HANDLE Instance, LPTSTR Name, LPTSTR ResType);
public:
	CResourceStream();
	CResourceStream(HANDLE Instance, LPTSTR ResName, LPTSTR ResType);
	CResourceStream(HANDLE Instance, LONG_PTR ResId, LPTSTR ResType);
	virtual ~CResourceStream();
	size_t Write(const LPVOID Buffer, size_t Count) override;
  
	REF_DYN_CLASS(CResourceStream)
};
DECLARE_DYN_CLASS(CResourceStream, CCustomMemoryStream)


typedef BYTE TStreamOwnership;
#define soReference		0x0
#define soOwned			0x1

class cVCL_API CStreamAdapter : public CInterfaceObject<IStream>{
private:
	CStream* Stream;
	TStreamOwnership Ownership;
public:
	CStreamAdapter(CStream* Stream, TStreamOwnership AOwnership = soReference);
	virtual ~CStreamAdapter();
	virtual HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead);
	virtual HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten);
	virtual HRESULT __stdcall Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
		ULARGE_INTEGER *libNewPosition);
	virtual HRESULT __stdcall SetSize(ULARGE_INTEGER libNewSize);
	virtual HRESULT __stdcall CopyTo(IStream *stm, ULARGE_INTEGER cb, 
		ULARGE_INTEGER *cbRead, ULARGE_INTEGER *cbWritten);
	virtual HRESULT __stdcall Commit(DWORD grfCommitFlags);
	virtual HRESULT __stdcall Revert();
	virtual HRESULT __stdcall LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
		DWORD dwLockType);
	virtual HRESULT __stdcall UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
		DWORD dwLockType);
	virtual HRESULT __stdcall Stat(STATSTG *statstg, DWORD grfStatFlag);
	virtual HRESULT __stdcall Clone(IStream **ppstm);
	
	DEFINE_GETTER(CStream*, Stream)
	DEFINE_ACCESSOR(TStreamOwnership, Ownership)

};
