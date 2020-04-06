#include "stdinc.h"
#include "Object.hpp"
#include "MsgTarget.hpp"
#include "Graphics.hpp"

class cVCL_API CClipboard : public CMsgTarget{
private:
	INT OpenRefCount;
	HWND ClipboardWindow;
	BOOL Allocated;
	BOOL Emptied;
	void Adding();
	void AssignGraphic(CGraphic* Source);
	//TODO void AssignPicture(CPicture* Source);
	void AssignToBitmap(CBitmap* Dest);
	void AssignToMetafile(CMetafile* Dest);
	//TODO void AssignToPicture(CPicture* Dest);
protected:
	void AssignTo(CClipboard* Dest);//Dest
	void SetBuffer(WORD Format, LPVOID Buffer, INT Size);
	virtual void WndProc(TMessage& Message);
	void MainWndProc(TMessage& Message);
	HWND GetClipboardWindow();
    DEFINE_GETTER(INT, OpenRefCount)
public:
	CClipboard(CComponent* AOwner = NULL);
	virtual ~CClipboard();
	void Assign(CObject* Source) override;
	virtual void Clear();
	virtual void Close();
	CComponent* GetComponent(CComponent* Owner, CComponent* Parent);
	HANDLE GetAsHandle(WORD Format);
	INT GetTextBuf(LPTSTR Buffer, INT BufSize);
	BOOL HasFormat(WORD Format);
	virtual void Open();
	void SetComponent(CComponent* Component);
	void SetAsHandle(WORD Format, HANDLE Value);
	void SetTextBuf(LPTSTR Buffer);
	String GetAsText();
	void SetAsText(String& Value);
	INT GetFormatCount();
	WORD GetFormats(INT Index);

	REF_DYN_CLASS(CClipboard)
};
DECLARE_DYN_CLASS(CClipboard, CMsgTarget)