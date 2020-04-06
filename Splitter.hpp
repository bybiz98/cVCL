#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"

typedef INT NaturalNumber;

typedef void (CObject::*TSplitterCanResizeEvent)(CObject* Sender, INT& NewSize, BOOL& Accept);

typedef BYTE TResizeStyle;
#define rsNone		0x0
#define rsLine		0x1
#define rsUpdate	0x2
#define rsPattern   0x3

class cVCL_API CSplitter : public CGraphicControl{
private:
    CWinControl* ActiveControl;
	BOOL AutoSnap;
	BOOL Beveled;
	CBrush* Brush;
	CControl* Control;
	TPoint DownPos;
	HDC LineDC;
	BOOL LineVisible;
	NaturalNumber MinSize;
	INT MaxSize;
	INT NewSize;
	INT OldSize;
	HBRUSH PrevBrush;
	TResizeStyle ResizeStyle;
	INT Split;
private:
	void AllocateLineDC();
	void CalcSplitSize(INT X, INT Y, INT& NewSize, INT& Split);
	void DrawLine();
	CControl* FindControl();
	void FocusKeyDown(CObject* Sender, WORD& Key, TShiftState Shift);
	void ReleaseLineDC();
	void UpdateControlSize();
	void UpdateSize(INT X, INT Y);
protected:
	virtual BOOL CanResize(INT& NewSize);
	virtual BOOL DoCanResize(INT& NewSize);
	void MouseDown(TMouseButton Button, TShiftState Shift, INT X, INT Y) override;
	void MouseMove(TShiftState Shift, INT X, INT Y) override;
	void MouseUp(TMouseButton Button, TShiftState Shift, INT X, INT Y) override;
	void Paint() override;
	void RequestAlign() override;
	virtual void StopSizing();
public:
	DECLARE_TYPE_EVENT(TKeyEvent, OldKeyDown)
	DECLARE_TYPE_EVENT(TSplitterCanResizeEvent, CanResize)
	DECLARE_TYPE_EVENT(TNotifyEvent, Moved)
	DECLARE_TYPE_EVENT(TNotifyEvent, Paint)
public:
    CSplitter(CComponent* AOwner = NULL);
	virtual ~CSplitter();
	
	void SetBeveled(BOOL Value);

	DEFINE_ACCESSOR(BOOL, AutoSnap)
	DEFINE_GETTER(BOOL, Beveled)
	DEFINE_ACCESSOR(NaturalNumber, MinSize)
	DEFINE_ACCESSOR(TResizeStyle, ResizeStyle)

	REF_DYN_CLASS(CSplitter)
};
DECLARE_DYN_CLASS(CSplitter, CGraphicControl)