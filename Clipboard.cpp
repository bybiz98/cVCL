#include "stdinc.h"
/*
#include "Clipboard.h"

CClipboard::CClipboard(CComponent* AOwner):CMsgTarget(AOwner),
	OpenRefCount(0),
	ClipboardWindow(0),
	Allocated(FALSE),
	Emptied(FALSE){
}

CClipboard::~CClipboard(){
}

void CClipboard::Adding(){
}

void CClipboard::AssignGraphic(CGraphic* Source){
}

//TODO void CClipboard::AssignPicture(CPicture* Source);
void CClipboard::AssignToBitmap(CBitmap* Dest){
}

void CClipboard::AssignToMetafile(CMetafile* Dest){
}

//TODO void CClipboard::AssignToPicture(CPicture* Dest);
void CClipboard::AssignTo(CClipboard* Dest){
}

void CClipboard::SetBuffer(WORD Format, LPVOID Buffer, INT Size){
}

void CClipboard::WndProc(TMessage& Message){
}

void CClipboard::MainWndProc(TMessage& Message){
}

HWND CClipboard::GetClipboardWindow(){
}

void CClipboard::Assign(CObject* Source){
}

void CClipboard::Clear(){
}

void CClipboard::Close(){
}

CComponent* CClipboard::GetComponent(CComponent* Owner, CComponent* Parent){
}

HANDLE CClipboard::GetAsHandle(WORD Format){
}

INT CClipboard::GetTextBuf(LPTSTR Buffer, INT BufSize){
}

BOOL CClipboard::HasFormat(WORD Format){
}

void CClipboard::Open(){
}

void CClipboard::SetComponent(CComponent* Component){
}

void CClipboard::SetAsHandle(WORD Format, HANDLE Value){
}

void CClipboard::SetTextBuf(LPTSTR Buffer){
}

String CClipboard::GetAsText(){
}

void CClipboard::SetAsText(String& Value){
}

INT CClipboard::GetFormatCount(){
}

WORD CClipboard::GetFormats(INT Index){
}

//*/