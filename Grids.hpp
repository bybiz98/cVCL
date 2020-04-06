#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "MaskEdit.hpp"

#define MaxCustomExtents	(INT_MAX/16)
#define MaxShortInt			65535

//Internal grid types
typedef INT (CObject::*TGetExtentsFunc)(LONG Index);

typedef struct _GridAxisDrawInfo{
	INT EffectiveLineWidth;
	INT FixedBoundary;
	INT GridBoundary;
	INT GridExtent;
	LONG LastFullVisibleCell;
	INT FullVisBoundary;
	INT FixedCellCount;
	INT FirstGridCell;
	INT GridCellCount;
	TGetExtentsFunc GetExtent;
} TGridAxisDrawInfo;

typedef struct _GridDrawInfo{
	TGridAxisDrawInfo Horz;
	TGridAxisDrawInfo Vert;
} TGridDrawInfo;

typedef BYTE TGridState;
#define gsNormal		0x0
#define gsSelecting		0x1
#define gsRowSizing		0x2
#define gsColSizing		0x3
#define gsRowMoving		0x4
#define gsColMoving		0x5

typedef BYTE TGridMovement; //gsRowMoving..gsColMoving;

//TInplaceEdit
//The inplace editor is not intended to be used outside the grid

class CCustomGrid;

class cVCL_API CInplaceEdit : public CMaskEdit{
private:
	CCustomGrid* Grid;
	LONG ClickTime;
	void InternalMove(const TRect& Loc, BOOL Redraw);
	void SetGrid(CCustomGrid* Value);
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_SHOWINGCHANGED, CInplaceEdit::CMShowingChanged)
		MSG_MAP_ENTRY(WM_GETDLGCODE, CInplaceEdit::WMGetDlgCode)
		MSG_MAP_ENTRY(WM_PASTE, CInplaceEdit::WMPaste)
		MSG_MAP_ENTRY(WM_CUT, CInplaceEdit::WMCut)
		MSG_MAP_ENTRY(WM_CLEAR, CInplaceEdit::WMClear)
	MSG_MAP_END()
	void CMShowingChanged(TMessage& Message);
	void WMGetDlgCode(TWMGetDlgCode& Message);
	void WMPaste(TMessage& Message);
	void WMCut(TMessage& Message);
	void WMClear(TMessage& Message);
protected:
    void CreateParams(TCreateParams& Params) override;
	void DblClick() override;
	BOOL DoMouseWheel(TShiftState Shift, INT WheelDelta, TPoint& MousePos) override;
	BOOL EditCanModify() override;
	void KeyDown(WORD& Key, TShiftState Shift) override;
	void KeyPress(TCHAR& Key) override;
	void KeyUp(WORD& Key, TShiftState Shift) override;
	virtual void BoundsChanged();
	virtual void UpdateContents();
	void WndProc(TMessage& Message) override;
	DEFINE_GETTER(CCustomGrid*, Grid)
public:
	CInplaceEdit(CComponent* AOwner = NULL);
	virtual ~CInplaceEdit();
	void Deselect();
	void Hide();
	void Invalidate() override;
	void Move(const TRect& Loc);
	BOOL PosEqual(const TRect& Rect);
	void SetFocus() override;
	void UpdateLoc(const TRect& Loc);
	BOOL Visible();
};
