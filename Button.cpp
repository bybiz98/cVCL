#include "stdinc.h"
#include "Button.hpp"
#include "ImgList.hpp"
#include "SysInit.hpp"
#include "WinUtils.hpp"
#include "Form.hpp"

IMPL_DYN_CLASS(CButton)
CButton::CButton(CComponent* AOwner):CButtonControl(AOwner),
	Default(FALSE),
	Cancel(FALSE),
	Active(FALSE),
	ModalResult(mrNone){
	SetControlStyle(csSetCaption | csDoubleClicks);
	SetBounds(GetLeft(), GetTop(), 75, 25);
	SetTabStop(TRUE);
}
CButton::~CButton(){
}

void CButton::CMDialogKey(TCMDialogKey& Message){
	if(((Message.CharCode == VK_RETURN && Active) ||
		(Message.CharCode == VK_ESCAPE && Cancel)) &&
		(KeyDataToShiftState((LONG)Message.KeyData) == 0) && CanFocus()){
		Click();
		Message.Result = 1;
	}
	else
		INHERITED_MSG(Message);

}

void CButton::CMDialogChar(TCMDialogChar& Message){
	String Text = this->GetTextString();
	if(IsAccel(Message.CharCode, Text.GetBuffer()) && CanFocus()){
		Click();
		Message.Result = 1;
	}
	else
		INHERITED_MSG(Message);

}

void CButton::CMFocusChanged(TCMFocusChanged& Message){
	if(Message.Sender != NULL && Message.Sender->InstanceOf(CButton::_Class))
		Active = Message.Sender == this;
	else Active = Default;
	SetButtonStyle(Active);
	INHERITED_MSG(Message);

}

void CButton::CNCommand(TWMCommand& Message){
	if(Message.NotifyCode == BN_CLICKED)
		Click();
}

void CButton::CNCtlColorBtn(TWMCtlColorBtn& Message){
	/*TODOif(ThemeServices.ThemesEnabled){
		DrawParentBackground(GetHandle(), Message.ChildDC, NULL, FALSE);
		//{ Return an empty brush to prevent Windows from overpainting we just have created. }
		Message.Result := GetStockObject(NULL_BRUSH);
	}
	else//*/
		INHERITED_MSG(Message);
}

void CButton::WMEraseBkgnd(TWMEraseBkgnd& Message){
	/* TODO
	if(ThemeServices.ThemesEnabled)
		Message.Result = 1;
	else//*/
		DefaultHandler(*((PMessage)&Message));
}

void CButton::CreateParams(TCreateParams& Params){
	DWORD ButtonStyles[] = {BS_PUSHBUTTON, BS_DEFPUSHBUTTON};
	__super::CreateParams(Params);
	CreateSubClass(Params, TEXT("BUTTON"));
	Params.Style |= ButtonStyles[Default ? 1 : 0];
}

void CButton::CreateWnd(){
	__super::CreateWnd();
	Active = Default;
}

void CButton::Click(){
	CForm* Form = GetParentForm(this);
	if(Form != NULL)
		Form->SetModalResult(ModalResult);
	__super::Click();
}

BOOL CButton::UseRightToLeftAlignment(){
	return FALSE;
}

#define BS_MASK 0x000F
void CButton::SetButtonStyle(BOOL ADefault){
	if(HandleAllocated()){
		DWORD Style = 0;
		if(ADefault)
			Style = BS_DEFPUSHBUTTON;
		else 
			Style = BS_PUSHBUTTON;
		if((GetWindowLongPtr(GetHandle(), GWL_STYLE) & BS_MASK) != Style)
			SendMessage(GetHandle(), BM_SETSTYLE, Style, 1);
	}
}
void CButton::SetDefault(BOOL Value){
	Default = Value;
	if(HandleAllocated()){
		CForm* Form = GetParentForm(this);
		if(Form != NULL)
			Form->Perform(CM_FOCUSCHANGED, 0, (LPARAM)Form->GetActiveControl());
	}
}

class CGlyphList : public CImageList{
private:
    CBits* Used;
    INT Count;
	INT AllocateIndex();
public:
	CGlyphList(CComponent* AOwner = NULL);
	CGlyphList(INT AWidth, INT AHeight);
	virtual ~CGlyphList();
	INT AddMasked(CBitmap* Image, TColor MaskColor);
	void Delete(INT Index);
	DEFINE_GETTER(INT, Count)

	REF_DYN_CLASS(CGlyphList)
};
DECLARE_DYN_CLASS(CGlyphList, CImageList)

class CGlyphCache : public CObject{
private:
	CList* GlyphLists;
public:
	CGlyphCache();
	virtual ~CGlyphCache();
	CGlyphList* GetList(INT AWidth, INT AHeight);
	void ReturnList(CGlyphList* List);
    BOOL Empty();

	REF_DYN_CLASS(CGlyphCache)
};
DECLARE_DYN_CLASS(CGlyphCache, CObject)

class CButtonGlyph : public CObject{
private:
    CBitmap *Glyph;
	CGlyphList *GlyphList;
	INT Indexs[4];
	TColor TransparentColor;
	TNumGlyphs NumGlyphs;

	void GlyphChanged(CObject* Sender);
public:
	DECLARE_TYPE_EVENT(TNotifyEvent, Change)

public:
	CButtonGlyph();
	virtual ~CButtonGlyph();
	void SetGlyph(CBitmap* Value);
	void SetNumGlyphs(TNumGlyphs Value);
	void Invalidate();
	INT CreateButtonGlyph(TButtonState State);
	void DrawButtonGlyph(CCanvas* Canvas, TPoint& GlyphPos,
		TButtonState State, BOOL Transparent);
	void DrawButtonText(CCanvas* Canvas, LPTSTR Caption,
		TRect& TextBounds, TButtonState State, LONG BiDiFlags);
	void CalcButtonLayout(CCanvas* Canvas, TRect& Client,
		TPoint& Offset, LPTSTR Caption, TButtonLayout Layout,
		INT Margin, INT Spacing, TPoint& GlyphPos, TRect& TextBounds,
		LONG BiDiFlags);
	//return the text rectangle 
	TRect Draw(CCanvas* Canvas, TRect& Client, TPoint& Offset,
		LPTSTR Caption, TButtonLayout Layout, INT Margin, INT Spacing,
		TButtonState State, BOOL Transparent, LONG BiDiFlags);
	DEFINE_GETTER(CBitmap*, Glyph)
	DEFINE_GETTER(TNumGlyphs, NumGlyphs)

	REF_DYN_CLASS(CButtonGlyph)
};
DECLARE_DYN_CLASS(CButtonGlyph, CObject)

IMPL_DYN_CLASS(CGlyphList)
CGlyphList::CGlyphList(CComponent* AOwner):CImageList(AOwner),Count(0){
	Used = new CBits();
}

CGlyphList::CGlyphList(INT AWidth, INT AHeight):CImageList(AWidth, AHeight),Count(0){
	Used = new CBits();
}

CGlyphList::~CGlyphList(){
	delete Used;
}

INT CGlyphList::AllocateIndex(){
	INT Result = Used->OpenBit();
	if(Result >= Used->GetSize()){
		Result = __super::Add(NULL, NULL);
		Used->SetSize(Result + 1);
	}
	Used->SetBit(Result, TRUE);
	return Result;
}

INT CGlyphList::AddMasked(CBitmap* Image, TColor MaskColor){
	INT Result = AllocateIndex();
	ReplaceMasked(Result, Image, MaskColor);
	Count++;
	return Result;
}

void CGlyphList::Delete(INT Index){
	if(Used->GetBit(Index)){
		Count++;
		Used->SetBit(Index, FALSE);
	}
}

IMPL_DYN_CLASS(CGlyphCache)
CGlyphCache::CGlyphCache(){
	GlyphLists = new CList();
}

CGlyphCache::~CGlyphCache(){
	delete GlyphLists;
}

CGlyphList* CGlyphCache::GetList(INT AWidth, INT AHeight){
	CGlyphList* Result = NULL;
	INT Count = GlyphLists->GetCount();
	for(INT I = Count - 1; I >= 0; I--){
		Result = (CGlyphList*)GlyphLists->Get(I);
		if(AWidth == Result->GetWidth() && AHeight == Result->GetHeight())
			return Result;
	}
	Result = new CGlyphList(AWidth, AHeight);
	GlyphLists->Add(Result);
	return Result;
}

void CGlyphCache::ReturnList(CGlyphList* List){
	if(List == NULL)
		return;
	if(List->GetCount() == 0){
		GlyphLists->Remove(List);
		delete List;
	}
}

BOOL CGlyphCache::Empty(){
	return GlyphLists->GetCount() == 0;
}

INT ButtonCount = 0;
CGlyphCache* GlyphCache = NULL;

IMPL_DYN_CLASS(CButtonGlyph)
CButtonGlyph::CButtonGlyph():GlyphList(NULL),TransparentColor(clOlive),NumGlyphs(1){
	Glyph = new CBitmap();
	Glyph->SetOnChange(this, (TNotifyEvent)&CButtonGlyph::GlyphChanged);
	for(INT I = bsUp; I <= bsExclusive; I++)
		Indexs[I] = -1;
	if(GlyphCache == NULL)
		GlyphCache = new CGlyphCache();
}
CButtonGlyph::~CButtonGlyph(){
	delete Glyph;
	Invalidate();
	if(GlyphCache != NULL && GlyphCache->Empty()){
		delete GlyphCache;
		GlyphCache = NULL;
	}
}

void CButtonGlyph::GlyphChanged(CObject* Sender){
	if(Sender == Glyph){
		TransparentColor = Glyph->GetTransparentColor();
		Invalidate();
		if(OnChange != NULL)
			CALL_EVENT(Change)(this);
	}
}
	
void CButtonGlyph::SetGlyph(CBitmap* Value){
	Invalidate();
	Glyph->Assign(Value);
	if(Value != NULL && Value->GetHeight() > 0){
		TransparentColor = Value->GetTransparentColor();
		if(Value->GetWidth() % Value->GetHeight() == 0){
			INT Glyphs = Value->GetWidth() / Value->GetHeight();
			if(Glyphs > 4)
				Glyphs = 1;
			SetNumGlyphs(Glyphs);
		}
	}
}

void CButtonGlyph::SetNumGlyphs(TNumGlyphs Value){
	if(Value != NumGlyphs && Value > 0){
		Invalidate();
		NumGlyphs = Value;
		GlyphChanged(Glyph);
	}
}

void CButtonGlyph::Invalidate(){
	for(INT I = bsUp; I <= bsExclusive; I++){
		if(Indexs[I] != -1)
			GlyphList->Delete(Indexs[I]);
		Indexs[I] = -1;
	}
	GlyphCache->ReturnList(GlyphList);
	GlyphList = NULL;
}

#define ROP_DSPDxax		0x00E20746
INT CButtonGlyph::CreateButtonGlyph(TButtonState State){
	if(State == bsDown && NumGlyphs < 3)
		State = bsUp;
	INT Result = Indexs[State];
	if(Result != -1)
		return Result;
	if((Glyph->GetWidth() | Glyph->GetHeight()) == 0)
		return Result;
	INT IWidth = Glyph->GetWidth() / NumGlyphs;
	INT IHeight = Glyph->GetHeight();
	if(GlyphList == NULL){
		if(GlyphCache == NULL)
			GlyphCache = new CGlyphCache();
		GlyphList = GlyphCache->GetList(IWidth, IHeight);
	}
	CBitmap*  TmpImage = new CBitmap();
	CObjectHolder timgHolder(TmpImage);
	TmpImage->SetWidth(IWidth);
    TmpImage->SetHeight(IHeight);
    TRect IRect = Rect(0, 0, IWidth, IHeight);
    TmpImage->GetCanvas()->GetBrush()->SetColor(clBtnFace);
    TmpImage->SetPalette(CopyPalette(Glyph->GetPalette()));
	TButtonState BS = State;
	if(BS >= NumGlyphs)
		BS = bsUp;
    TRect ORect = Rect(BS * IWidth, 0, (BS + 1) * IWidth, IHeight);
	if(State == bsUp || State == bsDown || State == bsExclusive){
		TmpImage->GetCanvas()->CopyRect(IRect, Glyph->GetCanvas(), ORect);
		if(Glyph->GetTransparentMode() == tmFixed)
			Indexs[State] = GlyphList->AddMasked(TmpImage, TransparentColor);
		else 
			Indexs[State] = GlyphList->AddMasked(TmpImage, clDefault);
	}
	else if(State == bsDisabled){
		CBitmap* MonoBmp = new CBitmap();
		CObjectHolder mbmpHolder(MonoBmp);
		CBitmap* DDB = new CBitmap();
		CObjectHolder dbbHolder(DDB);
		DDB->Assign(Glyph);
		DDB->SetHandleType(bmDDB);
		if(NumGlyphs > 1){
			//Change white & gray to clBtnHighlight and clBtnShadow
			TmpImage->GetCanvas()->CopyRect(IRect, DDB->GetCanvas(), ORect);
			MonoBmp->SetMonochrome(TRUE);
			MonoBmp->SetWidth(IWidth);
			MonoBmp->SetHeight(IHeight);
			
			//Convert white to clBtnHighlight
			DDB->GetCanvas()->GetBrush()->SetColor(clWhite);
			MonoBmp->GetCanvas()->CopyRect(IRect, DDB->GetCanvas(), ORect);
			TmpImage->GetCanvas()->GetBrush()->SetColor(clBtnHighlight);
			HDC DestDC = TmpImage->GetCanvas()->GetHandle();
			::SetTextColor(DestDC, clBlack);
            ::SetBkColor(DestDC, clWhite);
			BitBlt(DestDC, 0, 0, IWidth, IHeight,
				MonoBmp->GetCanvas()->GetHandle(), 0, 0, ROP_DSPDxax);

			//Convert gray to clBtnShadow
			DDB->GetCanvas()->GetBrush()->SetColor(clGray);
			MonoBmp->GetCanvas()->CopyRect(IRect, DDB->GetCanvas(), ORect);
			TmpImage->GetCanvas()->GetBrush()->SetColor(clBtnShadow);
			DestDC = TmpImage->GetCanvas()->GetHandle();
			::SetTextColor(DestDC, clBlack);
			::SetBkColor(DestDC, clWhite);
			BitBlt(DestDC, 0, 0, IWidth, IHeight,
				MonoBmp->GetCanvas()->GetHandle(), 0, 0, ROP_DSPDxax);
			
			//Convert transparent color to clBtnFace
			DDB->GetCanvas()->GetBrush()->SetColor(ColorToRGB(TransparentColor));
			MonoBmp->GetCanvas()->CopyRect(IRect, DDB->GetCanvas(), ORect);
			TmpImage->GetCanvas()->GetBrush()->SetColor(clBtnFace);
			DestDC = TmpImage->GetCanvas()->GetHandle();
			::SetTextColor(DestDC, clBlack);
			::SetBkColor(DestDC, clWhite);
			BitBlt(DestDC, 0, 0, IWidth, IHeight,
				MonoBmp->GetCanvas()->GetHandle(), 0, 0, ROP_DSPDxax);
		}
		else{
			//Create a disabled version
			MonoBmp->Assign(Glyph);
			MonoBmp->SetHandleType(bmDDB);
			MonoBmp->GetCanvas()->GetBrush()->SetColor(clBlack);
			MonoBmp->SetWidth(IWidth);
			if(MonoBmp->GetMonochrome()){
				MonoBmp->GetCanvas()->GetFont()->SetColor(clWhite);
				MonoBmp->SetMonochrome(FALSE);
				MonoBmp->GetCanvas()->GetBrush()->SetColor(clWhite);
			}
			MonoBmp->SetMonochrome(TRUE);
			TmpImage->GetCanvas()->GetBrush()->SetColor(clBtnFace);
			TmpImage->GetCanvas()->FillRect(IRect);
			TmpImage->GetCanvas()->GetBrush()->SetColor(clBtnHighlight);
			::SetTextColor(TmpImage->GetCanvas()->GetHandle(), clBlack);
			::SetBkColor(TmpImage->GetCanvas()->GetHandle(), clWhite);
			BitBlt(TmpImage->GetCanvas()->GetHandle(), 1, 1, IWidth, IHeight,
                  MonoBmp->GetCanvas()->GetHandle(), 0, 0, ROP_DSPDxax);
			TmpImage->GetCanvas()->GetBrush()->SetColor(clBtnShadow);
			::SetTextColor(TmpImage->GetCanvas()->GetHandle(), clBlack);
			::SetBkColor(TmpImage->GetCanvas()->GetHandle(), clWhite);
			BitBlt(TmpImage->GetCanvas()->GetHandle(), 0, 0, IWidth, IHeight,
                  MonoBmp->GetCanvas()->GetHandle(), 0, 0, ROP_DSPDxax);
		}
		Indexs[State] = GlyphList->AddMasked(TmpImage, clDefault);
	}
	Result = Indexs[State];
	Glyph->Dormant();
	return Result;
}

void CButtonGlyph::DrawButtonGlyph(CCanvas* Canvas, TPoint& GlyphPos,
	TButtonState State, BOOL Transparent){
	if(Glyph == NULL)
		return;
	if(Glyph->GetWidth() == 0 || Glyph->GetHeight() == 0)
		return;
	//Canvas->Draw(GlyphPos.x, GlyphPos.y, Glyph);
	//return;
	INT Index = CreateButtonGlyph(State);
	/*
	TRect R;
	if(ThemeServices.ThemesEnabled){
		R.TopLeft = GlyphPos;
		R.right = R.left + Glyph->GetWidth() / NumGlyphs;
		R.bottom = R.top + Glyph->GetHeight();
		if(State == bsDisabled)
			Button = tbPushButtonDisabled;
		else if(State == bsDown || State == bsExclusive)
			Button = tbPushButtonPressed;
		else// bsUp
			Button = tbPushButtonNormal;
		Details = ThemeServices.GetElementDetails(Button);
		ThemeServices.DrawIcon(Canvas.Handle, Details, R, FGlyphList.Handle, Index);
	}
    else //*/
	if(Transparent || (State == bsExclusive)){
		ImageList_DrawEx(GlyphList->GetHandle(), Index, Canvas->GetHandle(), GlyphPos.x, GlyphPos.y, 0, 0,
			clNone, clNone, ILD_TRANSPARENT);
	}
	else
		ImageList_DrawEx(GlyphList->GetHandle(), Index, Canvas->GetHandle(), GlyphPos.x, GlyphPos.y, 0, 0,
			ColorToRGB(clBtnFace), clNone, ILD_NORMAL);
}

void CButtonGlyph::DrawButtonText(CCanvas* Canvas, LPTSTR Caption,
		TRect& TextBounds, TButtonState State, LONG BiDiFlags){
	Canvas->GetBrush()->SetStyle(bsClear);
	if(State == bsDisabled){
		OffsetRect((LPRECT)&TextBounds, 1, 1);
		Canvas->GetFont()->SetColor(clBtnHighlight);
		DrawText(Canvas->GetHandle(), Caption, lstrlen(Caption), (LPRECT)&TextBounds,
			DT_CENTER | DT_VCENTER | BiDiFlags);
		OffsetRect((LPRECT)&TextBounds, -1, -1);
		Canvas->GetFont()->SetColor(clBtnShadow);
		DrawText(Canvas->GetHandle(), Caption, lstrlen(Caption), (LPRECT)&TextBounds,
			DT_CENTER | DT_VCENTER | BiDiFlags);
	}
	else
		DrawText(Canvas->GetHandle(), Caption, lstrlen(Caption), (LPRECT)&TextBounds,
			DT_CENTER | DT_VCENTER | BiDiFlags);
}

void CButtonGlyph::CalcButtonLayout(CCanvas* Canvas, TRect& Client,
		TPoint& Offset, LPTSTR Caption, TButtonLayout Layout,
		INT Margin, INT Spacing, TPoint& GlyphPos, TRect& TextBounds,
		LONG BiDiFlags){
	if((BiDiFlags & DT_RIGHT) == DT_RIGHT)
	if(Layout == blGlyphLeft)
		Layout = blGlyphRight;
	else 
		if(Layout == blGlyphRight)
			Layout = blGlyphLeft;
	//calculate the item sizes 
	TPoint ClientSize = {Client.right - Client.left, 
		Client.bottom - Client.top};
	TPoint GlyphSize = {0, 0};
	if(Glyph != NULL){
		GlyphSize.x = Glyph->GetWidth() / NumGlyphs;
		GlyphSize.y = Glyph->GetHeight();
	}
	TPoint TextSize= {0,0};
	if(lstrlen(Caption) > 0){
		TextBounds = Rect(0, 0, Client.right - Client.left, 0);
		DrawText(Canvas->GetHandle(), Caption, lstrlen(Caption), (LPRECT)&TextBounds,
			DT_CALCRECT | BiDiFlags);
		TextSize.x = TextBounds.right - TextBounds.left;
		TextSize.y = TextBounds.bottom - TextBounds.top;
	}
	else{
		TextBounds = Rect(0, 0, 0, 0);
	}
	//If the layout has the glyph on the right or the left, then both the
    //text and the glyph are centered vertically.  If the glyph is on the top
    //or the bottom, then both the text and the glyph are centered horizontally
	TPoint TextPos = {0, 0};
	if(IN_TEST(Layout, (blGlyphLeft | blGlyphRight))){
		GlyphPos.y = (ClientSize.y - GlyphSize.y + 1) / 2;
		TextPos.y = (ClientSize.y - TextSize.y + 1) / 2;
	}
	else{
		GlyphPos.y = (ClientSize.x - GlyphSize.x + 1) / 2;
		TextPos.x = (ClientSize.x - TextSize.x + 1) / 2;
	}
	
	//if there is no text or no bitmap, then Spacing is irrelevant
	if(TextSize.x == 0 || GlyphSize.x == 0)
		Spacing = 0;
	
	//adjust Margin and Spacing
	TPoint TotalSize = {0, 0};
	if(Margin == -1){
		if(Spacing == -1){
			TotalSize.x = GlyphSize.x + TextSize.x;
			TotalSize.y = GlyphSize.y + TextSize.y;
			if(IN_TEST(Layout, (blGlyphLeft | blGlyphRight)))
				Margin = (ClientSize.x - TotalSize.x) / 3;
			else
				Margin = (ClientSize.y - TotalSize.y) / 3;
			Spacing = Margin;
		}
		else {
			TotalSize.x = GlyphSize.x + Spacing + TextSize.x;
			TotalSize.y = GlyphSize.y + Spacing + TextSize.y;
			if(IN_TEST(Layout, (blGlyphLeft | blGlyphRight)))
				Margin = (ClientSize.x - TotalSize.x + 1) / 2;
			else
				Margin = (ClientSize.y - TotalSize.y + 1) / 2;
		}
	}
	else {
		if(Spacing == -1){
			TotalSize.x = ClientSize.x - (Margin + GlyphSize.x);
			TotalSize.y = ClientSize.y - (Margin + GlyphSize.y);
			if(IN_TEST(Layout, (blGlyphLeft | blGlyphRight)))
				Spacing = (TotalSize.x - TextSize.x) / 2;
			else
				Spacing = (TotalSize.y - TextSize.y) / 2;
		}
	}
	switch(Layout){
	case blGlyphLeft:
		GlyphPos.x = Margin;
		TextPos.x = GlyphPos.x + GlyphSize.x + Spacing;
		break;
    case blGlyphRight:
		GlyphPos.x = ClientSize.x - Margin - GlyphSize.x;
		TextPos.x = GlyphPos.x - Spacing - TextSize.x;
		break;
    case blGlyphTop:
		GlyphPos.y = Margin;
        TextPos.y = GlyphPos.y + GlyphSize.y + Spacing;
		break;
    case blGlyphBottom:
		GlyphPos.y = ClientSize.y - Margin - GlyphSize.y;
        TextPos.y = GlyphPos.y - Spacing - TextSize.y;
		break;
	}
	
	//fixup the result variables
	GlyphPos.x += Client.left + Offset.x;
	GlyphPos.y += Client.top + Offset.y;

	//Themed text is not shifted, but gets a different color.
	/*
	if(ThemeServices.ThemesEnabled)
		OffsetRect(TextBounds, TextPos.X + Client.Left, TextPos.Y + Client.Top);
	else //*/
		OffsetRect((LPRECT)&TextBounds, TextPos.x + Client.left + Offset.x, TextPos.y + Client.top + Offset.y);

}

//return the text rectangle 
TRect CButtonGlyph::Draw(CCanvas* Canvas, TRect& Client, TPoint& Offset,
		LPTSTR Caption, TButtonLayout Layout, INT Margin, INT Spacing,
		TButtonState State, BOOL Transparent, LONG BiDiFlags){
	TRect Result = {0,0,0,0};
	TPoint GlyphPos = {0, 0};
	CalcButtonLayout(Canvas, Client, Offset, Caption, Layout, Margin, Spacing,
		GlyphPos, Result, BiDiFlags);
	DrawButtonGlyph(Canvas, GlyphPos, State, Transparent);
	DrawButtonText(Canvas, Caption, Result, State, BiDiFlags);
	return Result;
}

IMPL_DYN_CLASS(CSpeedButton)
CSpeedButton::CSpeedButton(CComponent* AOwner):CGraphicControl(AOwner),
	GroupIndex(0),
	Down(FALSE),
	Dragging(FALSE),
	AllowAllUp(FALSE),
	Layout(blGlyphLeft),
	Spacing(4),
	Transparent(TRUE),
	Margin(-1),
	Flat(FALSE),
	MouseInControl(FALSE),
	State(bsUp){
	Glyph = new CButtonGlyph();
	((CButtonGlyph *)Glyph)->SetOnChange(this, (TNotifyEvent)&CSpeedButton::GlyphChanged);
	SetBounds(0, 0, 23, 22);
	SetControlStyle(csCaptureMouse | csDoubleClicks);
	SetParentFont(TRUE);
	SetColor(clBtnFace);
	ButtonCount++;
}
CSpeedButton::~CSpeedButton(){
	ButtonCount--;
	delete (CButtonGlyph *)Glyph;
}

void CSpeedButton::GlyphChanged(CObject* Sender){
	Invalidate();
}

void CSpeedButton::UpdateExclusive(){
	TMessage Msg;
	if(GroupIndex != 0 && GetParent() != NULL){
		Msg.Msg = CM_BUTTONPRESSED;
		Msg.wParam = GroupIndex;
		Msg.lParam = (LPARAM)this;
		Msg.Result = 0;
		GetParent()->Broadcast(Msg);
	}
}

CBitmap* CSpeedButton::GetGlyph(){
	return ((CButtonGlyph *)Glyph)->GetGlyph();
}

void CSpeedButton::SetGlyph(CBitmap* Value){
	((CButtonGlyph *)Glyph)->SetGlyph(Value);
	Invalidate();
}

TNumGlyphs CSpeedButton::GetNumGlyphs(){
	return ((CButtonGlyph *)Glyph)->GetNumGlyphs();
}

void CSpeedButton::SetNumGlyphs(TNumGlyphs Value){
	if(Value < 0)
		Value = 1;
	else if(Value > 4)
		Value = 4;
	if(Value != ((CButtonGlyph *)Glyph)->GetNumGlyphs()){
		((CButtonGlyph *)Glyph)->SetNumGlyphs(Value);
		Invalidate();
	}
}

void CSpeedButton::SetDown(BOOL Value){
	if(GroupIndex == 0)
		Value = FALSE;
	if(Value != Down){
		if(Down && (!AllowAllUp))
			return;
		Down = Value;
		if(Value){
			if(State == bsUp)
				Invalidate();
			State = bsExclusive;
		}
		else {
			State = bsUp;
			Repaint();
		}
		if(Value)
			UpdateExclusive();
	}
}

void CSpeedButton::SetFlat(BOOL Value){
	if(Value != Flat){
		Flat = Value;
		Invalidate();
	}
}

void CSpeedButton::SetAllowAllUp(BOOL Value){
	if(AllowAllUp != Value){
		AllowAllUp = Value;
		UpdateExclusive();
	}
}

void CSpeedButton::SetGroupIndex(INT Value){
	if(GroupIndex != Value){
		GroupIndex = Value;
		UpdateExclusive();
	}
}

void CSpeedButton::SetLayout(TButtonLayout Value){
	if(Layout != Value){
		Layout = Value;
		Invalidate();
	}
}

void CSpeedButton::SetSpacing(INT Value){
	if(Value != Spacing){
		Spacing = Value;
		Invalidate();
	}
}

void CSpeedButton::SetTransparent(BOOL Value){
	if(Value != Transparent){
		Transparent = Value;
		if(Value)
			SetControlStyle(GetControlStyle() & (~csOpaque));
		else
			SetControlStyle(GetControlStyle() | csOpaque);
		Invalidate();
	}
}

void CSpeedButton::SetMargin(INT Value){
	if(Value != Margin && Value >= -1){
		Margin = Value;
		Invalidate();
	}
}

void CSpeedButton::UpdateTracking(){
	if(Flat){
		if(GetEnabled()){
			TPoint P;
			GetCursorPos(&P);
			MouseInControl = !(FindDragTarget(P, TRUE) == this);
			if(MouseInControl)
				Perform(CM_MOUSELEAVE, 0, 0);
			else
				Perform(CM_MOUSEENTER, 0, 0);
		}
	}
}

void CSpeedButton::WMLButtonDblClk(TWMLButtonDown& Message){
	INHERITED_MSG(Message);
	if(Down)
		DblClick();
}

void CSpeedButton::CMEnabledChanged(TMessage& Message){
	TButtonState NewState[] = {bsDisabled, bsUp};
	((CButtonGlyph *)Glyph)->CreateButtonGlyph(NewState[GetEnabled()]);
	UpdateTracking();
	Repaint();
}

void CSpeedButton::CMButtonPressed(TMessage& Message){
	if(Message.wParam == GroupIndex){
		CSpeedButton* Sender = (CSpeedButton *)Message.lParam;
		if(Sender != this){
			if(Sender->GetDown() && Down){
				Down = FALSE;
				State = bsUp;
				//TODO if(GetAction()->InstanceOf(CCustomAction::_Class))
				//	((CCustomAction *)GetAction())->SetChecked(FALSE);
				Invalidate();
			}
			AllowAllUp = Sender->GetAllowAllUp();
		}
	}
}

void CSpeedButton::CMDialogChar(TCMDialogChar& Message){
	if(IsAccel(Message.CharCode, GetText()) && GetEnabled() && GetVisible() && 
		(GetParent() != NULL) && GetParent()->GetShowing()){
		Click();
		Message.Result = 1;
	}
	else
		INHERITED_MSG(Message);
}

void CSpeedButton::CMFontChanged(TMessage& Message){
	Invalidate();
}

void CSpeedButton::CMTextChanged(TMessage& Message){
	Invalidate();
}

void CSpeedButton::CMSysColorChange(TMessage& Message){
	((CButtonGlyph *)Glyph)->Invalidate();
	((CButtonGlyph *)Glyph)->CreateButtonGlyph(State);
}

void CSpeedButton::CMMouseEnter(TMessage& Message){
	INHERITED_MSG(Message);
	//Don't draw a border if DragMode <> dmAutomatic since this button is meant to 
	//be used as a dock client.
	BOOL NeedRepaint = Flat && !MouseInControl && GetEnabled()
		/*&& (GetDragMode() != dmAutomatic)//*/ 
		&& (GetCapture() == 0);
	//Windows XP introduced hot states also for non-flat buttons. }
	if((NeedRepaint /* || ThemeServices.ThemesEnabled//*/) && !IN_TEST(csDesigning, GetComponentState())){
		MouseInControl = TRUE;
		if(GetEnabled())
			Repaint();
	}
}

void CSpeedButton::CMMouseLeave(TMessage& Message){
	INHERITED_MSG(Message);
	BOOL NeedRepaint = Flat && MouseInControl && GetEnabled() && !Dragging;
	//Windows XP introduced hot states also for non-flat buttons.
	if(NeedRepaint /* || ThemeServices.ThemesEnabled//*/){
		MouseInControl = FALSE;
		if(GetEnabled())
			Repaint();
	}
}

/*TODO void CSpeedButton::ActionChange(CObject* Sender, BOOL CheckDefaults){
procedure CopyImage(ImageList: TCustomImageList; Index: Integer);
  begin
    with Glyph do
    begin
      Width := ImageList.Width;
      Height := ImageList.Height;
      Canvas.Brush.Color := clFuchsia;//! for lack of a better color
      Canvas.FillRect(Rect(0,0, Width, Height));
      ImageList.Draw(Canvas, 0, 0, Index);
    end;
  end;
	__super::ActionChange(Sender, CheckDefaults);
	if(Sender is TCustomAction then
    with TCustomAction(Sender) do
    begin
      if CheckDefaults or (Self.GroupIndex = 0) then
        Self.GroupIndex := GroupIndex;
      { Copy image from action's imagelist }
      if (Glyph.Empty) and (ActionList <> nil) and (ActionList.Images <> nil) and
        (ImageIndex >= 0) and (ImageIndex < ActionList.Images.Count) then
        CopyImage(ActionList.Images, ImageIndex);
    end;

}
//*/

/*
CControlActionLinkClass* CSpeedButton::GetActionLinkClass(){
  Result := TSpeedButtonActionLink;
}
//*/

HPALETTE CSpeedButton::GetPalette(){
	return GetGlyph()->GetPalette();
}

void CSpeedButton::Loaded(){
	__super::Loaded();
	TButtonState State;
	if(GetEnabled())
		State = bsUp;
	else
		State = bsDisabled;
	((CButtonGlyph *)Glyph)->CreateButtonGlyph(State);
}

void CSpeedButton::MouseDown(TMouseButton Button, TShiftState Shift, INT X, INT Y){
	__super::MouseDown(Button, Shift, X, Y);
	if(Button == mbLeft && GetEnabled()){
		if(!Down){
			State = bsDown;
			Invalidate();
		}
		Dragging = TRUE;
	}
}

void CSpeedButton::MouseMove(TShiftState Shift, INT X, INT Y){
	__super::MouseMove(Shift, X, Y);
	if(Dragging){
		TButtonState NewState = bsExclusive;
		if(!Down)
			NewState = bsUp;
		if(X >= 0 && X < GetClientWidth() && Y >= 0 && Y <= GetClientHeight())
			if(Down)
				NewState = bsExclusive;
			else
				NewState = bsDown;
		if(NewState != State){
			State = NewState;
			Invalidate();
		}
	}
	else if(!MouseInControl)
		UpdateTracking();
}

void CSpeedButton::MouseUp(TMouseButton Button, TShiftState Shift, INT X, INT Y){
	__super::MouseUp(Button, Shift, X, Y);
	if(Dragging){
		Dragging = FALSE;
		BOOL DoClick = (X >= 0 && X < GetClientWidth() && Y >= 0 && Y <= GetClientHeight());
		if(GroupIndex == 0){
			//Redraw face in-case mouse is captured
			State = bsUp;
			MouseInControl = FALSE;
			if(DoClick && State != bsExclusive && State != bsDown)
				Invalidate();
		}
		else if(DoClick){
			SetDown(!Down);
			if(Down)
				Repaint();
		}
		else {
			if(Down)
				State = bsExclusive;
			Repaint();
		}
		if(DoClick)
			Click();
		UpdateTracking();
	}
}

void CSpeedButton::Paint(){
	INT DownStyles[] = {BDR_RAISEDINNER, BDR_SUNKENOUTER};
	INT FillStyles[] = {BF_MIDDLE, 0};
	if(!GetEnabled()){
		State = bsDisabled;
		Dragging = FALSE;
	}
	else if(State == bsDisabled)
		if(Down && GetGroupIndex() != 0)
			State = bsExclusive;
		else 
			State = bsUp;
	GetCanvas()->SetFont(GetFont());
	/*
	if(ThemeServices.ThemesEnabled){
		PerformEraseBackground(this, GetCanvas()->GetHandle());
		if(!GetEnabled())
			Button = tbPushButtonDisabled;
		else
			if(State == bsDown || State == bsExclusive)
				Button = tbPushButtonPressed;
			else
				if(GetMouseInControl())
					Button = tbPushButtonHot;
				else
					Button = tbPushButtonNormal;
		ToolButton = ttbToolbarDontCare;
		if(Flat){
			switch(Button){
			case tbPushButtonDisabled:
				Toolbutton = ttbButtonDisabled;
				break;
			case tbPushButtonPressed:
				Toolbutton = ttbButtonPressed;
				break;
			case tbPushButtonHot:
				Toolbutton = ttbButtonHot;
				break;
			case tbPushButtonNormal:
				Toolbutton = ttbButtonNormal;
				break;
			}
		}
		TRect PaintRect = GetClientRect();
		if(ToolButton == ttbToolbarDontCare){
			Details = ThemeServices.GetElementDetails(Button);
			ThemeServices.DrawElement(Canvas.Handle, Details, PaintRect);
			PaintRect = ThemeServices.ContentRect(Canvas.Handle, Details, PaintRect);
		}
		else {
			Details = ThemeServices.GetElementDetails(ToolButton);
			ThemeServices.DrawElement(Canvas.Handle, Details, PaintRect);
			PaintRect = ThemeServices.ContentRect(Canvas.Handle, Details, PaintRect);
		}
		if(Button == tbPushButtonPressed){
			// A pressed speed button has a white text. This applies however only to flat buttons.
			if(ToolButton != ttbToolbarDontCare)
				GetCanvas->GetFont()->SetColor(clHighlightText);
				Offset = Point(1, 0);
		}
		else 
			Offset = Point(0, 0);
		((CButtonGlyph *)Glyph)->Draw(GetCanvas(), PaintRect, Offset, Caption, Layout, Margin, Spacing, State, GetTransparent(),
			DrawTextBiDiModeFlags(0));
	}
	else
		//*/
	{
		TRect PaintRect = Rect(0, 0, GetWidth(), GetHeight());
		INT DrawFlags = 0;
		if(!Flat){
			DrawFlags = DFCS_BUTTONPUSH | DFCS_ADJUSTRECT;
			if(State == bsDown || State == bsExclusive)
				DrawFlags |= DFCS_PUSHED;
			DrawFrameControl(GetCanvas()->GetHandle(), (LPRECT)&PaintRect, DFC_BUTTON, DrawFlags);
		}
		else{
			if((State == bsDown || State == bsExclusive) ||
				(MouseInControl && (State != bsDisabled)) ||
				IN_TEST(csDesigning, GetComponentState()))
				DrawEdge(GetCanvas()->GetHandle(), (LPRECT)&PaintRect, DownStyles[State == bsDown || State == bsExclusive],
					FillStyles[GetTransparent()] | BF_RECT);
			else {
				if(!GetTransparent()){
					GetCanvas()->GetBrush()->SetColor(GetColor());
					GetCanvas()->FillRect(PaintRect);
				}
			}
			InflateRect((LPRECT)&PaintRect, -1, -1);
		}
		TPoint Offset = {0,0};
		if(State == bsDown || State == bsExclusive){
			if(State == bsExclusive && (!Flat || !MouseInControl)){
				GetCanvas()->GetBrush()->SetBitmap(AllocPatternBitmap(clBtnFace, clBtnHighlight));
				GetCanvas()->FillRect(PaintRect);
			}
			Offset.x = 1;
			Offset.y = 1;
		}
		((CButtonGlyph *)Glyph)->Draw(GetCanvas(), PaintRect, Offset, GetText(), Layout, Margin,
			Spacing, State, GetTransparent(), DrawTextBiDiModeFlags(0));
	}
}

void CSpeedButton::Click(){
	__super::Click();
}

LPTSTR BitBtnResNames[] = {NULL,
	TEXT("BBOK"), TEXT("BBCANCEL"), TEXT("BBHELP"), 
	TEXT("BBYES"), TEXT("BBNO"), TEXT("BBCLOSE"),
	TEXT("BBABORT"), TEXT("BBRETRY"), TEXT("BBIGNORE")
	TEXT("BBALL")};

LPTSTR BitBtnCaptions[] = {NULL,
	TEXT("OK"), TEXT("Cancel"), TEXT("&Yes"),
	TEXT("&No"), TEXT("&Help"), TEXT("&Close"),
	TEXT("&Ignore"), TEXT("&Retry"), TEXT("Abort"),
	TEXT("&All")};

TModalResult BitBtnModalResults[] = {
	0, mrOk, mrCancel, 0, mrYes, mrNo, 
	0, mrAbort, mrRetry, mrIgnore, mrAll};

class CBitBtnGlyphs{
private:
	CBitmap* Glyphs[11];
public:
	CBitBtnGlyphs(){
		ZeroMemory(Glyphs, sizeof(Glyphs));
	};
	virtual ~CBitBtnGlyphs(){
		for(INT I = 0; I < 11; I++){
			if(Glyphs[I] != NULL){
				//delete Glyphs[I];
				Glyphs[I] = NULL;
			}
		}
	};
	CBitmap* GetBitBtnGlyph(TBitBtnKind Kind){
		if(Glyphs[Kind] == NULL){
			Glyphs[Kind] = new CBitmap();
			Glyphs[Kind]->LoadFromResourceName(GetGlobal().GetHInstance(), BitBtnResNames[Kind]);
		}
		return Glyphs[Kind];
	};
};
CBitBtnGlyphs BitBtnGlyphs;

IMPL_DYN_CLASS(CBitBtn)
CBitBtn::CBitBtn(CComponent* AOwner):CButton(AOwner),
	Style(bsAutoDetect),
	Kind(bkCustom),
	Layout(blGlyphLeft),
	Spacing(4),
	Margin(-1),
	IsFocused(FALSE),
	ModifiedGlyph(FALSE),
	MouseInControl(FALSE){
	Glyph = new CButtonGlyph();
	((CButtonGlyph *)Glyph)->SetOnChange(this, (TNotifyEvent)&CBitBtn::GlyphChanged);
	Canvas = new CCanvas();
	SetControlStyle(GetControlStyle() | csReflector);
	SetDoubleBuffered(TRUE);
}

CBitBtn::~CBitBtn(){
	delete (CButtonGlyph *)Glyph;
	delete Canvas;
}

void CBitBtn::DrawItem(DRAWITEMSTRUCT& DrawItemStruct){
	Canvas->SetHandle(DrawItemStruct.hDC);
	TRect R = GetClientRect();
	Canvas->SetFont(GetFont());
	BOOL IsDown = (DrawItemStruct.itemState & ODS_SELECTED) != 0;
	BOOL IsDefault = (DrawItemStruct.itemState & ODS_FOCUS) != 0;
	TButtonState State = 0;
	if(!GetEnabled())
		State = bsDisabled;
	else if(IsDown)
		State = bsDown;
	else State = bsUp;
	/*
	if(ThemeServices.ThemesEnabled){
		if(!GetEnabled())
			Button = tbPushButtonDisabled;
		else if(IsDown)
			Button = tbPushButtonPressed;
		else if(MouseInControl)
			Button = tbPushButtonHot;
		else
			if(IsFocused || IsDefault)
				Button = tbPushButtonDefaulted;
			else
				Button = tbPushButtonNormal;

		Details = ThemeServices.GetElementDetails(Button);
		// Parent background.
		ThemeServices.DrawParentBackground(GetHandle(), DrawItemStruct.hDC, &Details, TRUE);
		// Button shape.
		ThemeServices.DrawElement(DrawItemStruct.hDC, Details, DrawItemStruct.rcItem);
		R = ThemeServices.ContentRect(Canvas->GetHandle(), Details, DrawItemStruct.rcItem);
		if(Button == tbPushButtonPressed)
			Offset = Point(1, 0);
		else
			Offset = Point(0, 0);
		((CButtonGlyph *)Glyph)->Draw(Canvas, R, Offset, Caption, Layout, Margin, Spacing, State, FALSE,
			DrawTextBiDiModeFlags(0));
		if(IsFocused && IsDefault){
			Canvas->GetPen()->SetColor(clWindowFrame);
			Canvas->GetBrush()->SetColor(clBtnFace);
			DrawFocusRect(Canvas->GetHandle(), R);
		}
	}
	else//*/
	{
		R = GetClientRect();
		DWORD Flags = DFCS_BUTTONPUSH | DFCS_ADJUSTRECT;
		if(IsDown)
			Flags |= DFCS_PUSHED;
		if((DrawItemStruct.itemState & ODS_DISABLED)!= 0)
			Flags |= DFCS_INACTIVE;
		//DrawFrameControl doesn't allow for drawing a button as the
		//default button, so it must be done here.
		if(IsFocused || IsDefault){
			Canvas->GetPen()->SetColor(clWindowFrame);
			Canvas->GetPen()->SetWidth(1);
			Canvas->GetBrush()->SetStyle(bsClear);
			Canvas->Rectangle(R.left, R.top, R.right, R.bottom);
			//DrawFrameControl must draw within this border 
			InflateRect((LPRECT)&R, -1, -1);
		}
		//DrawFrameControl does not draw a pressed button correctly 
		if(IsDown){
			Canvas->GetPen()->SetColor(clBtnShadow);
			Canvas->GetPen()->SetWidth(1);
			Canvas->GetBrush()->SetColor(clBtnFace);
			Canvas->Rectangle(R.left, R.top, R.right, R.bottom);
			InflateRect((LPRECT)&R, -1, -1);
		}
		else
			DrawFrameControl(DrawItemStruct.hDC, (LPRECT)&R, DFC_BUTTON, Flags);

		if(IsFocused){
			R = GetClientRect();
			InflateRect((LPRECT)&R, -1, -1);
		}
		
		Canvas->SetFont(GetFont());
		if(IsDown)
			OffsetRect((LPRECT)&R, 1, 1);
		String Text = GetTextString();
		((CButtonGlyph *)Glyph)->Draw(Canvas, R, Point(0,0), Text.GetBuffer(), Layout, Margin,
			Spacing, State, FALSE, DrawTextBiDiModeFlags(0));
		
		if(IsFocused && IsDefault){
			R = GetClientRect();
			InflateRect((LPRECT)&R, -4, -4);
			Canvas->GetPen()->SetColor(clWindowFrame);
			Canvas->GetBrush()->SetColor(clBtnFace);
			DrawFocusRect(Canvas->GetHandle(), (const RECT *)&R);
		}
	}
	Canvas->SetHandle(0);
}

void CBitBtn::SetGlyph(CBitmap* Value){
	((CButtonGlyph *)Glyph)->SetGlyph(Value);
	ModifiedGlyph = TRUE;
	Invalidate();
}

CBitmap* CBitBtn::GetGlyph(){
	return ((CButtonGlyph *)Glyph)->GetGlyph();
}

TNumGlyphs CBitBtn::GetNumGlyphs(){
	return ((CButtonGlyph *)Glyph)->GetNumGlyphs();
}

void CBitBtn::SetNumGlyphs(TNumGlyphs Value){
	if(Value < 0)
		Value = 1;
	else if(Value > 4)
		Value = 4;
	if(Value != ((CButtonGlyph *)Glyph)->GetNumGlyphs()){
		((CButtonGlyph *)Glyph)->SetNumGlyphs(Value);
		Invalidate();
	}
}

void CBitBtn::GlyphChanged(CObject* Sender){
	Invalidate();
}

BOOL CBitBtn::IsCustom(){
	return GetKind() == bkCustom;
}

BOOL CBitBtn::IsCustomCaption(){
	return GetTextString() != BitBtnCaptions[Kind];
}

void CBitBtn::SetStyle(TButtonStyle Value){
	if(Value != Style){
		Style = Value;
		Invalidate();
	}
}

void CBitBtn::SetKind(TBitBtnKind Value){
	if(Value != Kind){
		if(Value != bkCustom){
			SetDefault(Value == bkOK || Value == bkYes);
			SetCancel(Value == bkCancel || Value == bkNo);
			String Text = GetTextString();
			if(IN_TEST(csLoading, GetComponentState()) && (Text.GetBuffer() == NULL || lstrlen(Text.GetBuffer()) == 0) ||
				!IN_TEST(csLoading, GetComponentState())){
				if(BitBtnCaptions[Value] != NULL)
					SetText(BitBtnCaptions[Value]);
			}
			SetModalResult(BitBtnModalResults[Value]);
			((CButtonGlyph *)Glyph)->SetGlyph(BitBtnGlyphs.GetBitBtnGlyph(Value));
			SetNumGlyphs(2);
			ModifiedGlyph = FALSE;
		}
		Kind = Value;
		Invalidate();
	}
}

TBitBtnKind CBitBtn::GetKind(){
	if(Kind != bkCustom)
		if(((Kind == bkOK || Kind == bkYes) ^ GetDefault()) ||
			((Kind == bkCancel || Kind == bkNo) ^ GetCancel()) ||
			(GetModalResult() != BitBtnModalResults[Kind]) ||
			ModifiedGlyph)
			Kind = bkCustom;
	return Kind;
}

void CBitBtn::SetLayout(TButtonLayout Value){
	if(Layout != Value){
		Layout = Value;
		Invalidate();
	}
}

void CBitBtn::SetSpacing(INT Value){
	if(Spacing != Value){
		Spacing = Value;
		Invalidate();
	}
}

void CBitBtn::SetMargin(INT Value){
	if(Value != Margin && Value >= - 1){
		Margin = Value;
		Invalidate();
	}
}

void CBitBtn::CNMeasureItem(TWMMeasureItem& Message){
	Message.MeasureItemStruct->itemWidth = GetWidth();
	Message.MeasureItemStruct->itemHeight = GetHeight();
}

void CBitBtn::CNDrawItem(TWMDrawItem& Message){
	DrawItem(*(Message.DrawItemStruct));
}

void CBitBtn::CMFontChanged(TMessage& Message){
	INHERITED_MSG(Message);
	Invalidate();
}

void CBitBtn::CMEnabledChanged(TMessage& Message){
	INHERITED_MSG(Message);
	Invalidate();
}

void CBitBtn::CMMouseEnter(TMessage& Message){
	INHERITED_MSG(Message);
	if(/*ThemeServices.ThemesEnabled && //*/!MouseInControl && !IN_TEST(csDesigning, GetComponentState())){
		MouseInControl = TRUE;
		Repaint();
	}//*/

}

void CBitBtn::CMMouseLeave(TMessage& Message){
	INHERITED_MSG(Message);
	if(/*ThemeServices.ThemesEnabled && //*/MouseInControl){
		MouseInControl = FALSE;
		Repaint();
	}
}

void CBitBtn::WMLButtonDblClk(TWMLButtonDblClk& Message){
	Perform(WM_LBUTTONDOWN, Message.Keys, Message.lParam);
}


void CBitBtn::ActionChange(CObject* Sender, BOOL CheckDefaults){
	/*
  procedure CopyImage(ImageList: TCustomImageList; Index: Integer);
  begin
    with Glyph do
    begin
      Width := ImageList.Width;
      Height := ImageList.Height;
      Canvas.Brush.Color := clFuchsia;//! for lack of a better color
      Canvas.FillRect(Rect(0,0, Width, Height));
      ImageList.Draw(Canvas, 0, 0, Index);
    end;
  end;

begin
  inherited ActionChange(Sender, CheckDefaults);
  if Sender is TCustomAction then
    with TCustomAction(Sender) do
    begin
      { Copy image from action's imagelist }
      if (Glyph.Empty) and (ActionList <> nil) and (ActionList.Images <> nil) and
        (ImageIndex >= 0) and (ImageIndex < ActionList.Images.Count) then
        CopyImage(ActionList.Images, ImageIndex);
    end;
//*/
}

void CBitBtn::CreateHandle(){
	TButtonState State;
	if(GetEnabled())
		State = bsUp;
	else
		State = bsDisabled;
	__super::CreateHandle();
	((CButtonGlyph *)Glyph)->CreateButtonGlyph(State);
}

void CBitBtn::CreateParams(TCreateParams& Params){
	__super::CreateParams(Params);
	Params.Style |= BS_OWNERDRAW;
}

HPALETTE CBitBtn::GetPalette(){
	return GetGlyph()->GetPalette();
}

void CBitBtn::SetButtonStyle(BOOL ADefault){
	if(ADefault != IsFocused){
		IsFocused = ADefault;
		Refresh();
	}
}

void CBitBtn::Click(){
	if(Kind == bkClose){
		CForm* Form = GetParentForm(this);
		if(Form != NULL)
			Form->Close();
		else __super::Click();
	}
	else if(Kind == bkHelp){
		/*TODO
		CWinControl* Control = this;
        while(Control != NULL && Control->GetHelpContext() == 0)
			Control = Control->GetParent();
		if(Control != NULL)
			Application->HelpContext(Control->GetHelpContext());
		else // */
			__super::Click();
	}
	else
		__super::Click();
}

