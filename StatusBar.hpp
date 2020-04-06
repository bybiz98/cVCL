#pragma once

#include "Object.hpp"
#include "List.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "Control.hpp"
#include "WinControl.hpp"
#include "Graphics.hpp"
#include "Collection.hpp"

class CStatusBar;
class CStatusPanels;

typedef BYTE TStatusPanelStyle;
#define psText			0x0
#define psOwnerDraw		0x1

typedef BYTE TStatusPanelBevel;
#define pbNone			0x0
#define pbLowered		0x1
#define pbRaised		0x2

class cVCL_API CStatusPanel : public CCollectionItem{
private:
	friend class CStatusBar;
	String* Text;
	INT Width;
	TAlignment Alignment;
    TStatusPanelBevel Bevel;
	TBiDiMode BiDiMode;
	BOOL ParentBiDiMode;
	TStatusPanelStyle Style;
	BOOL UpdateNeeded;

	BOOL IsBiDiModeStored();
protected:
	String GetDisplayName() override;
public:
	CStatusPanel(CCollection* Collection = NULL);
	virtual ~CStatusPanel();
	void Assign(CObject* Source) override;
	void ParentBiDiModeChanged();
	BOOL UseRightToLeftAlignment();
	BOOL UseRightToLeftReading();

	void SetAlignment(TAlignment Value);
	void SetBevel(TStatusPanelBevel Value);
	void SetBiDiMode(TBiDiMode Value);
	void SetParentBiDiMode(BOOL Value);
	void SetStyle(TStatusPanelStyle Value);
	String GetText();
	void SetText(String& Value);
	void SetWidth(INT Value);

	DEFINE_GETTER(TAlignment, Alignment)
	DEFINE_GETTER(TStatusPanelBevel, Bevel)
	DEFINE_GETTER(TBiDiMode, BiDiMode)
	DEFINE_GETTER(BOOL, ParentBiDiMode)
	DEFINE_GETTER(TStatusPanelStyle, Style)
	DEFINE_GETTER(INT, Width)

	REF_DYN_CLASS(CStatusPanel)
};
DECLARE_DYN_CLASS(CStatusPanel, CCollectionItem)

class cVCL_API CStatusPanels : public CCollection{
private:
	friend class CStatusPanel;
    CStatusBar* StatusBar;
protected:
	CComponent* GetOwner() override;
	void Update(CCollectionItem* Item) override;
public:
	CStatusPanels(CStatusBar* StatusBar = NULL);
	virtual ~CStatusPanels();
	CStatusPanel* Add();
	CStatusPanel* AddItem(CStatusPanel* Item, INT Index);
	CStatusPanel* Insert(INT Index);
	
	CStatusPanel* GetItem(INT Index);
	void SetItem(INT Index, CStatusPanel* Value);

	REF_DYN_CLASS(CStatusPanels)
};
DECLARE_DYN_CLASS(CStatusPanels, CCollection)

typedef void (CObject::*TCustomDrawPanelEvent)(CStatusBar* StatusBar, CStatusPanel* Panel, TRect& Rect);
typedef void (CObject::*TSBCreatePanelClassEvent)(CStatusBar* Sender, CStatusPanelClass& PanelClass);

class cVCL_API CStatusBar : public CWinControl{
private:
	friend class CStatusPanels;
    CStatusPanels* Panels;
	CCanvas* Canvas;
	String* SimpleText;
	BOOL SimplePanel;
	BOOL SizeGrip;
	BOOL SizeGripValid;
	BOOL UseSystemFont;
	BOOL AutoHint;
public:
	DECLARE_TYPE_EVENT(TCustomDrawPanelEvent, DrawPanel)
	DECLARE_TYPE_EVENT(TNotifyEvent, Hint)
	DECLARE_TYPE_EVENT(TSBCreatePanelClassEvent, CreatePanelClass)
private:
	void DoRightToLeftAlignment(String& Str, TAlignment AAlignment, BOOL ARTLAlignment);
	void SetPanels(CStatusPanels* Value);
	void UpdateSimpleText();
	void SyncToSystemFont();
protected:
	MSG_MAP_BEGIN()
		MSG_MAP_ENTRY(CM_BIDIMODECHANGED, CStatusBar::CMBiDiModeChanged)
		MSG_MAP_ENTRY(CM_COLORCHANGED, CStatusBar::CMColorChanged)
		MSG_MAP_ENTRY(CM_PARENTFONTCHANGED, CStatusBar::CMParentFontChanged)
		MSG_MAP_ENTRY(CM_SYSCOLORCHANGE, CStatusBar::CMSysColorChange)
		MSG_MAP_ENTRY(CM_SYSFONTCHANGED, CStatusBar::CMSysFontChanged)
		MSG_MAP_ENTRY(CN_DRAWITEM, CStatusBar::CNDrawItem)
		MSG_MAP_ENTRY(WM_ERASEBKGND, CStatusBar::WMEraseBkGnd)
		MSG_MAP_ENTRY(WM_GETTEXTLENGTH, CStatusBar::WMGetTextLength)
		MSG_MAP_ENTRY(WM_PAINT, CStatusBar::WMPaint)
		MSG_MAP_ENTRY(WM_SIZE, CStatusBar::WMSize)
	MSG_MAP_END()
	void CMBiDiModeChanged(TMessage& Message);
    void CMColorChanged(TMessage& Message);
    void CMParentFontChanged(TMessage& Message);
    void CMSysColorChange(TMessage& Message);
    void CMWinIniChange(TMessage& Message);
    void CMSysFontChanged(TMessage& Message);
    void CNDrawItem(TWMDrawItem& Message);
    void WMEraseBkGnd(TWMEraseBkgnd& Message);
    void WMGetTextLength(TWMGetTextLength& Message);
    void WMPaint(TWMPaint& Message);
    void WMSize(TWMSize& Message);
protected:
	void SetUseSystemFont(BOOL Value);
	void ValidateSizeGrip(BOOL ARecreate);
protected:
	void ChangeScale(INT M, INT D);//TODO override;
	virtual CStatusPanels* CreatePanels();
	void CreateParams(TCreateParams& Params);
	void CreateWnd() override;
	virtual BOOL DoHint();
	virtual void DrawPanel(CStatusPanel* Panel, TRect& Rect);
	virtual CStatusPanelClass* GetPanelClass();
	BOOL IsFontStored();
public:
    CStatusBar(CComponent* AOwner = NULL);
	virtual ~CStatusBar();
	//TODO BOOL ExecuteAction(CBasicAction* Action) override;
	//TODO void FlipChildren(BOOL AllLevels) override;
	void SetBounds(INT ALeft, INT ATop, INT AWidth, INT AHeight) override;
	void SetSimplePanel(BOOL Value);
	void SetSimpleText(String& Value);
	void SetSizeGrip(BOOL Value);
	void SetParent(CWinControl* AParent) override;
	
	virtual CStatusPanel* CreatePanel();
	void UpdatePanel(INT Index, BOOL Repaint);
	void UpdatePanels(BOOL UpdateRects, BOOL UpdateText);

	DEFINE_GETTER(CCanvas*, Canvas)
	DEFINE_ACCESSOR(BOOL, AutoHint)
	DEFINE_GETTER(CStatusPanels*, Panels) //SetPanels;
	DEFINE_GETTER(BOOL, SimplePanel) //SetSimplePanel;
	//TODO property SimpleText: string read FSimpleText write SetSimpleText;
    DEFINE_GETTER(BOOL, SizeGrip) //SetSizeGrip;
	DEFINE_GETTER(BOOL, UseSystemFont) //SetUseSystemFont;

	REF_DYN_CLASS(CStatusBar)
};
DECLARE_DYN_CLASS(CStatusBar, CWinControl)