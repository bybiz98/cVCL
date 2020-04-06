#include "stdinc.h"
#include "WinUtils.hpp"
#include "Stream.hpp"

IMPL_DYN_CLASS(CStream)
CStream::CStream(){
}
CStream::~CStream(){
}

void CStream::ReadBuffer(LPVOID Buffer, size_t Count){
	if(Count != 0 && Read(Buffer, Count) != Count)
		throw "Stream read error";
}

void CStream::WriteBuffer(const LPVOID Buffer, size_t Count){
	if(Count != 0 && Write(Buffer, Count) != Count)
		throw "Stream write error";
}

size_t CStream::CopyFrom(CStream* Source, size_t Count){
	if(Count == 0){
		Source->SetPosition(0);
		Count = Source->GetSize();
	}
	size_t N = 0;
	size_t BufSize = Count;
	size_t Result = Count;
	if(Count > 0xF000)
		BufSize = 0xF000;
	LPVOID Buffer = malloc(BufSize);
	__try{
		while(Count != 0){
			if(Count > BufSize)
				N = BufSize;
			else 
				N = Count;
			Source->ReadBuffer(Buffer, N);
			WriteBuffer(Buffer, N);
			Count -= N;
		}
	}
	__finally{
		free(Buffer);
	}
	return Result;
}

void CStream::WriteResourceHeader(const LPTSTR ResName, size_t FixupInfo){
	size_t HeaderSize = 0;
	BYTE Header[160] = {0};
	Header[0] = 0xFF;
	*((WORD *)&Header[1]) = 10;
	lstrcpyn((LPTSTR)(&Header[3]), ResName, 63);
	CharUpperBuff((LPTSTR)(&Header[3]), 63);
	HeaderSize = lstrlen((LPTSTR)(&Header[3])) * sizeof(TCHAR) + sizeof(TCHAR) + 9;
	*((WORD *)&Header[HeaderSize - 6]) = 0x1030;
	*((DWORD *)&Header[HeaderSize - 4]) = 0;
	WriteBuffer(Header, HeaderSize);
	FixupInfo = GetPosition();
}

void CStream::FixupResourceHeader(size_t FixupInfo){
	DWORD ImageSize = (DWORD)(GetPosition() - FixupInfo);
	SetPosition(FixupInfo - 4);
	WriteBuffer(&ImageSize, 4);
	SetPosition(FixupInfo + ImageSize);
}

void CStream::ReadResHeader(){
	BYTE Header[160] = {0};
	size_t BufSize = sizeof(Header) - sizeof(TCHAR);
	size_t ReadCount = Read(Header, BufSize);
	if ((Header[0] == 0xFF) && (*((WORD *)&Header[1]) == 10)){
		size_t len = lstrlen((LPTSTR)&Header[3]) * sizeof(TCHAR) + sizeof(TCHAR) + 9 - ReadCount;
		Seek(len, soFromCurrent);
	}
	else
		throw "Invalid stream format";
}

size_t CStream::GetPosition(){
	return Seek(0, soCurrent);
}

void CStream::SetPosition(size_t Pos){
	Seek(Pos, soBeginning);
}

size_t CStream::GetSize(){
	size_t Pos = Seek(0, soCurrent);
	size_t Result = Seek(0, soEnd);
	Seek(Pos, soBeginning);
	return Result;
}

void CStream::SetSize(size_t NewSize){
}

IMPL_DYN_CLASS(CHandleStream)
CHandleStream::CHandleStream(HANDLE AHandle):
	Handle(AHandle){
}
CHandleStream::~CHandleStream(){
}

size_t CHandleStream::Read(LPVOID Buffer, size_t Count){
	DWORD Result = 0;
	if(!ReadFile(Handle, Buffer, (DWORD)Count, &Result, NULL))
		Result = -1;
	return Result == -1 ? 0 : Result;
}

size_t CHandleStream::Write(const LPVOID Buffer, size_t Count){
	DWORD Result = 0;
	if(!WriteFile(Handle, Buffer, (DWORD)Count, &Result, NULL))
		Result = -1;
	return Result == -1 ? 0 : Result;
}

size_t CHandleStream::Seek(size_t Offset, WORD Origin){
	return SetFilePointer(Handle, (LONG)Offset, NULL, Origin);
}

void CHandleStream::SetSize(size_t NewSize){
	Seek(NewSize, soBeginning);
	Win32Check(SetEndOfFile(Handle));
}

HANDLE CHandleStream::GetHandle(){
	return Handle;
}

IMPL_DYN_CLASS(CFileStream)
CFileStream::CFileStream():CHandleStream(0){
}

CFileStream::CFileStream(const LPTSTR FileName, WORD Mode, DWORD Rights){
	if(Mode == fmCreate){
		Handle = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (Handle <= 0)
			throw "Cannot create file ";
	}
	else{
		DWORD AccessMode[] = {GENERIC_READ, GENERIC_WRITE, GENERIC_READ | GENERIC_WRITE};
		DWORD ShareMode[] = {0, 0, FILE_SHARE_READ, FILE_SHARE_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE};
		if((Mode & 3) <= fmOpenReadWrite && (Mode & 0xF0) <= fmShareDenyNone)
			Handle = CreateFile(FileName, AccessMode[Mode & 3], ShareMode[(Mode & 0xF0) >> 4], NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, 0);
		if(Handle <= 0)
			throw "Cannot open file ";
	}
}

CFileStream::~CFileStream(){
	if(Handle > 0)
		CloseHandle(Handle);
}

IMPL_DYN_CLASS(CCustomMemoryStream)
CCustomMemoryStream::CCustomMemoryStream():
	Memory(NULL),
	Position(0),
	Size(0){
}

CCustomMemoryStream::~CCustomMemoryStream(){
}

void CCustomMemoryStream::SetPointer(LPVOID Ptr, size_t Size){
	Memory = Ptr;
	this->Size = Size;
}

size_t CCustomMemoryStream::Read(LPVOID Buffer, size_t Count){
	size_t Result = 0;
	if(Position >= 0 && Count >= 0){
		Result = Size - Position;
		if(Result > 0){
			if(Result > Count) 
				Result = Count;
			CopyMemory(Buffer, (BYTE *)Memory + Position, Result);
			Position += Result;
		}
	}
	return Result;
}

size_t CCustomMemoryStream::Seek(size_t Offset, WORD Origin){
	switch(Origin){
		case soFromBeginning: Position = Offset; break;
		case soFromCurrent: Position += Offset; break;
		case soFromEnd: Position = Size + Offset; break;
	}
	return Position;
}

void CCustomMemoryStream::SaveToStream(CStream* Stream){
	if(Size != 0)
		Stream->WriteBuffer(Memory, Size);
}

void CCustomMemoryStream::SaveToFile(const LPTSTR FileName){
	CFileStream Stream(FileName, fmCreate);
	SaveToStream(&Stream);
}

LPVOID CCustomMemoryStream::GetMemory(){
	return Memory;
}

IMPL_DYN_CLASS(CMemoryStream)
CMemoryStream::CMemoryStream():
	Capacity(0){
}
CMemoryStream::~CMemoryStream(){
	Clear();
}

LPVOID CMemoryStream::Realloc(size_t NewCapacity){
	if ((NewCapacity > 0 && NewCapacity != Size)){
		NewCapacity = (NewCapacity + (MemoryDelta - 1)) & ~(MemoryDelta - 1);
	}
	LPVOID Result = GetMemory();
	if (NewCapacity != Capacity){
		if (NewCapacity == 0){
			GlobalFreePtr(Result);
			Result = NULL;
		}
		else{
			if (GetCapacity() == 0)
				Result = GlobalAllocPtr(HeapAllocFlags, NewCapacity);
			else
				Result = GlobalReAllocPtr(GetMemory(), NewCapacity, HeapAllocFlags);
			if (Result == NULL) 
				throw "Out of memory while expanding memory stream";
		}
	}
	return Result;
}

size_t CMemoryStream::GetCapacity(){
	return Capacity;
}

void CMemoryStream::SetCapacity(size_t NewCapacity){
	SetPointer(Realloc(NewCapacity), Size);
	Capacity = NewCapacity;
}

void CMemoryStream::Clear(){
	SetCapacity(0);
	Size = 0;
	Position = 0;
}

void CMemoryStream::LoadFromStream(CStream* Stream){
	Stream->SetPosition(0);
	size_t Count = Stream->GetSize();
	SetSize(Count);
	if (Count != 0)
		Stream->ReadBuffer(GetMemory(), Count);
}

void CMemoryStream::LoadFromFile(const LPTSTR FileName){
	CFileStream Stream(FileName, fmOpenRead | fmShareDenyWrite);
	LoadFromStream(&Stream);
}

void CMemoryStream::SetSize(size_t NewSize){
	size_t OldPosition = Position;
	SetCapacity(NewSize);
	Size = NewSize;
	if(OldPosition > NewSize)
		Seek(0, soFromEnd);
}

size_t CMemoryStream::Write(const LPVOID Buffer, size_t Count){
	size_t Result = 0;
	if (Position >= 0 && Count >= 0){
		size_t Pos = Position + Count;
		if(Pos > 0){
			if (Pos > Size){
				if (Pos > Capacity)
					SetCapacity(Pos);
				Size = Pos;
			}
			LPVOID P = GetMemory();
			CopyMemory((BYTE *)P + Position, Buffer, Count);
			Position = Pos;
			Result = Count;
		}
	}
	return Result;
}

IMPL_DYN_CLASS(CResourceStream)
CResourceStream::CResourceStream(){
}

CResourceStream::CResourceStream(HANDLE Instance, LPTSTR ResName, LPTSTR ResType){
	Initialize(Instance, ResName, ResType);
}

CResourceStream::CResourceStream(HANDLE Instance, LONG_PTR ResId, LPTSTR ResType){
	Initialize(Instance, (LPTSTR)ResId, ResType);
}

CResourceStream::~CResourceStream(){
	UnlockResource(HGlobal);
	FreeResource(HGlobal);
}

void CResourceStream::Initialize(HANDLE Instance, LPTSTR Name, LPTSTR ResType){
	ResInfo = FindResource((HMODULE)Instance, Name, ResType);
	if(ResInfo == 0)
		throw "resource not found";//
	HGlobal = LoadResource((HMODULE)Instance, (HRSRC)ResInfo);
	if(HGlobal == 0)
		throw "resource not found.";
	SetPointer(LockResource(HGlobal), SizeofResource((HMODULE)Instance, (HRSRC)ResInfo));
}

size_t CResourceStream::Write(const LPVOID Buffer, size_t Count){
	throw "cannot write resource stream .";//raise EStreamError.CreateRes(@SCantWriteResourceStreamError);
}
  
CStreamAdapter::CStreamAdapter(CStream* Stream, TStreamOwnership AOwnership):CInterfaceObject(IID_IStream),
	Stream(NULL),
	Ownership(AOwnership){
}
CStreamAdapter::~CStreamAdapter(){
	if(Ownership == soOwned){
		delete Stream;
		Stream = NULL;
	}
}

HRESULT CStreamAdapter::Read(void *pv, ULONG cb, ULONG *pcbRead){
	if(pv == NULL)
		return STG_E_INVALIDPOINTER;
	__try{
		ULONG NumRead = (ULONG)Stream->Read(pv, cb);
		if(pcbRead != NULL)
			*pcbRead = NumRead;
		return S_OK;
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		return S_FALSE;
	}
}

HRESULT CStreamAdapter::Write(const void *pv, ULONG cb, ULONG *pcbWritten){
	if(pv == NULL)
		return STG_E_INVALIDPOINTER;
	__try{
		ULONG NumWritten = (ULONG)Stream->Write((LPVOID)pv, cb);
		if(pcbWritten != NULL)
			*pcbWritten = NumWritten;
		return S_OK;
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		return STG_E_CANTSAVE;
	}
}

HRESULT CStreamAdapter::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
	ULARGE_INTEGER *libNewPosition){
	if(dwOrigin < STREAM_SEEK_SET || dwOrigin > STREAM_SEEK_END)
		return STG_E_INVALIDFUNCTION;
	__try{
		size_t P = Stream->Seek((size_t)dlibMove.QuadPart, (WORD)dwOrigin);
		if(libNewPosition != NULL)
			(*libNewPosition).QuadPart = P;
		return S_OK;
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		return STG_E_INVALIDPOINTER;
	}
}

HRESULT CStreamAdapter::SetSize(ULARGE_INTEGER libNewSize){
	__try{
		Stream->SetSize((size_t)libNewSize.QuadPart);
		if(libNewSize.QuadPart != Stream->GetSize())
			return E_FAIL;
		else
			return S_OK;
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		return E_UNEXPECTED;
	}
}

#define MaxBufSize	(1024 * 1024)
HRESULT CStreamAdapter::CopyTo(IStream *stm, ULARGE_INTEGER cb, 
	ULARGE_INTEGER *cbRead, ULARGE_INTEGER *cbWritten){
	HRESULT Result = S_OK;
	ULARGE_INTEGER BytesRead = {0};
	ULARGE_INTEGER BytesWritten = {0};
	LPVOID Buffer = NULL;
	INT BufSize;
	__try{
		if(cb.QuadPart > MaxBufSize)
			BufSize = MaxBufSize;
		else
			BufSize = (INT)cb.QuadPart;
		Buffer = malloc(BufSize);
		INT I, N, R;
		ULONG W;
		while(cb.QuadPart > 0){
			if(cb.QuadPart > MAXINT)
				I = MAXINT;
			else
				I = (INT)cb.QuadPart;
			while(I > 0){
				if(I > BufSize)
					N = BufSize;
				else 
					N = I;
				R = (INT)Stream->Read(Buffer, N);
				if(R == 0)
					return Result; // The end of the stream was hit.
				BytesRead.QuadPart += R;
				W = 0;
				Result = stm->Write(Buffer, R, &W);
				BytesWritten.QuadPart += W;
				if(Result == S_OK && W != R)
					Result = E_FAIL;
				if(Result != S_OK)
					return Result;
				I -= R;
				cb.QuadPart -= R;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		Result = E_UNEXPECTED;
	}
	if(Buffer != NULL)
		free(Buffer);
	if(cbWritten != NULL)
		*cbWritten = BytesWritten;
	if(cbRead != NULL) 
		*cbRead = BytesRead;
	return Result;

}

HRESULT CStreamAdapter::Commit(DWORD grfCommitFlags){
	return S_OK;
}

HRESULT CStreamAdapter::Revert(){
	return STG_E_REVERTED;
}

HRESULT CStreamAdapter::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
	DWORD dwLockType){
	return STG_E_INVALIDFUNCTION;
}

HRESULT CStreamAdapter::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
	DWORD dwLockType){
	return STG_E_INVALIDFUNCTION;
}

HRESULT CStreamAdapter::Stat(STATSTG *statstg, DWORD grfStatFlag){
	HRESULT Result = S_OK;
	__try{
		if(statstg != NULL){
			(*statstg).type = STGTY_STREAM;
			(*statstg).cbSize.QuadPart = Stream->GetSize();
			(*statstg).mtime.dwLowDateTime = 0;
			(*statstg).mtime.dwHighDateTime = 0;
			(*statstg).ctime.dwLowDateTime = 0;
			(*statstg).ctime.dwHighDateTime = 0;
			(*statstg).atime.dwLowDateTime = 0;
			(*statstg).atime.dwHighDateTime = 0;
			(*statstg).grfLocksSupported = LOCK_WRITE;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		Result = E_UNEXPECTED;
	}
	return Result;
}

HRESULT CStreamAdapter::Clone(IStream **ppstm){
	return E_NOTIMPL;
}

	