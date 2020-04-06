#pragma once

#include "stdinc.h"
#include "Types.hpp"


#define CM_BASE					 0xB000
#define CM_ACTIVATE              CM_BASE + 0
#define CM_DEACTIVATE            CM_BASE + 1
#define CM_GOTFOCUS              CM_BASE + 2
#define CM_LOSTFOCUS             CM_BASE + 3
#define CM_CANCELMODE            CM_BASE + 4
#define CM_DIALOGKEY             CM_BASE + 5
#define CM_DIALOGCHAR            CM_BASE + 6
#define CM_FOCUSCHANGED          CM_BASE + 7
#define CM_PARENTFONTCHANGED     CM_BASE + 8
#define CM_PARENTCOLORCHANGED    CM_BASE + 9
#define CM_HITTEST               CM_BASE + 10
#define CM_VISIBLECHANGED        CM_BASE + 11
#define CM_ENABLEDCHANGED        CM_BASE + 12
#define CM_COLORCHANGED          CM_BASE + 13
#define CM_FONTCHANGED           CM_BASE + 14
#define CM_CURSORCHANGED         CM_BASE + 15
#define CM_CTL3DCHANGED          CM_BASE + 16
#define CM_PARENTCTL3DCHANGED    CM_BASE + 17
#define CM_TEXTCHANGED           CM_BASE + 18
#define CM_MOUSEENTER            CM_BASE + 19
#define CM_MOUSELEAVE            CM_BASE + 20
#define CM_MENUCHANGED           CM_BASE + 21
#define CM_APPKEYDOWN            CM_BASE + 22
#define CM_APPSYSCOMMAND         CM_BASE + 23
#define CM_BUTTONPRESSED         CM_BASE + 24
#define CM_SHOWINGCHANGED        CM_BASE + 25
#define CM_ENTER                 CM_BASE + 26
#define CM_EXIT                  CM_BASE + 27
#define CM_DESIGNHITTEST         CM_BASE + 28
#define CM_ICONCHANGED           CM_BASE + 29
#define CM_WANTSPECIALKEY        CM_BASE + 30
#define CM_INVOKEHELP            CM_BASE + 31
#define CM_WINDOWHOOK            CM_BASE + 32
#define CM_RELEASE               CM_BASE + 33
#define CM_SHOWHINTCHANGED       CM_BASE + 34
#define CM_PARENTSHOWHINTCHANGED CM_BASE + 35
#define CM_SYSCOLORCHANGE        CM_BASE + 36
#define CM_WININICHANGE          CM_BASE + 37
#define CM_FONTCHANGE            CM_BASE + 38
#define CM_TIMECHANGE            CM_BASE + 39
#define CM_TABSTOPCHANGED        CM_BASE + 40
#define CM_UIACTIVATE            CM_BASE + 41
#define CM_UIDEACTIVATE          CM_BASE + 42
#define CM_DOCWINDOWACTIVATE     CM_BASE + 43
#define CM_CONTROLLISTCHANGE     CM_BASE + 44
#define CM_GETDATALINK           CM_BASE + 45
#define CM_CHILDKEY              CM_BASE + 46
#define CM_DRAG                  CM_BASE + 47
#define CM_HINTSHOW              CM_BASE + 48
#define CM_DIALOGHANDLE          CM_BASE + 49
#define CM_ISTOOLCONTROL         CM_BASE + 50
#define CM_RECREATEWND           CM_BASE + 51
#define CM_INVALIDATE            CM_BASE + 52
#define CM_SYSFONTCHANGED        CM_BASE + 53
#define CM_CONTROLCHANGE         CM_BASE + 54
#define CM_CHANGED               CM_BASE + 55
#define CM_DOCKCLIENT            CM_BASE + 56
#define CM_UNDOCKCLIENT          CM_BASE + 57
#define CM_FLOAT                 CM_BASE + 58
#define CM_BORDERCHANGED         CM_BASE + 59
#define CM_BIDIMODECHANGED       CM_BASE + 60
#define CM_PARENTBIDIMODECHANGED CM_BASE + 61
#define CM_ALLCHILDRENFLIPPED    CM_BASE + 62
#define CM_ACTIONUPDATE          CM_BASE + 63
#define CM_ACTIONEXECUTE         CM_BASE + 64
#define CM_HINTSHOWPAUSE         CM_BASE + 65
#define CM_DOCKNOTIFICATION      CM_BASE + 66
#define CM_MOUSEWHEEL            CM_BASE + 67
#define CM_ISSHORTCUT            CM_BASE + 68

#define CN_BASE                  0xBC00
#define CN_CHARTOITEM            CN_BASE + WM_CHARTOITEM
#define CN_COMMAND               CN_BASE + WM_COMMAND
#define CN_COMPAREITEM           CN_BASE + WM_COMPAREITEM
#define CN_CTLCOLORBTN           CN_BASE + WM_CTLCOLORBTN
#define CN_CTLCOLORDLG           CN_BASE + WM_CTLCOLORDLG
#define CN_CTLCOLOREDIT          CN_BASE + WM_CTLCOLOREDIT
#define CN_CTLCOLORLISTBOX       CN_BASE + WM_CTLCOLORLISTBOX
#define CN_CTLCOLORMSGBOX        CN_BASE + WM_CTLCOLORMSGBOX
#define CN_CTLCOLORSCROLLBAR     CN_BASE + WM_CTLCOLORSCROLLBAR
#define CN_CTLCOLORSTATIC        CN_BASE + WM_CTLCOLORSTATIC
#define CN_DELETEITEM            CN_BASE + WM_DELETEITEM
#define CN_DRAWITEM              CN_BASE + WM_DRAWITEM
#define CN_HSCROLL               CN_BASE + WM_HSCROLL
#define CN_MEASUREITEM           CN_BASE + WM_MEASUREITEM
#define CN_PARENTNOTIFY          CN_BASE + WM_PARENTNOTIFY
#define CN_VKEYTOITEM            CN_BASE + WM_VKEYTOITEM
#define CN_VSCROLL               CN_BASE + WM_VSCROLL
#define CN_KEYDOWN               CN_BASE + WM_KEYDOWN
#define CN_KEYUP                 CN_BASE + WM_KEYUP
#define CN_CHAR                  CN_BASE + WM_CHAR
#define CN_SYSKEYDOWN            CN_BASE + WM_SYSKEYDOWN
#define CN_SYSCHAR               CN_BASE + WM_SYSCHAR
#define CN_NOTIFY                CN_BASE + WM_NOTIFY

class CControl;
class CWinControl;
#pragma pack (1)
typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		struct{
			WORD wParamLo;
			WORD wParamHi;
		};
	};
	union{
		LPARAM lParam;
		struct{
			WORD lParamLo;
			WORD lParamHi;
		};
	};
	union{
		LRESULT Result;
		struct{
			WORD ResultLo;
			WORD ResultHi;
		};
	};
}TMessage, *PMessage;

typedef struct{
	UINT Msg;
	WPARAM Unused;
	PWINDOWPOS WindowPos;
	LPARAM Result;
}TWMWindowPosMsg;

typedef TWMWindowPosMsg TWMWindowPosChanged;
typedef TWMWindowPosMsg TWMWindowPosChanging;

typedef struct{
	UINT Msg;
    WPARAM Keys;
	union{
	  LPARAM lParam;
	  struct{
        SHORT XPos;
        SHORT YPos;
	  };
	  struct{
        TSmallPoint Pos;
	  };
	};
	LRESULT Result;
}TWMMouse, *PWMMouse;

typedef TWMMouse TWMLButtonDblClk;
typedef TWMMouse TWMLButtonDown;
typedef TWMMouse TWMLButtonUp;
typedef TWMMouse TWMMButtonDblClk;
typedef TWMMouse TWMMButtonDown;
typedef TWMMouse TWMMButtonUp;
typedef TWMMouse TWMMouseMove;
typedef TWMMouse TWMRButtonDblClk;
typedef TWMMouse TWMRButtonDown;
typedef TWMMouse TWMRButtonUp, *PWMRButtonUp;
typedef TWMMouse TCMDesignHitTest;


typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		struct{
			SHORT Keys;
			SHORT WheelDelta;
		};
	};
    union{
		LPARAM lPARAM;
		struct{
			SHORT XPos;
			SHORT YPos;
		};
		struct{
			TSmallPoint Pos;
		};
	};
	LRESULT Result;
}TWMMouseWheel, *PWMMouseWheel;

typedef struct{
	UINT Msg;
	WPARAM WheelDelta;
	union{
		LPARAM lParam;
		struct{
			SHORT XPos;
			SHORT YPos;
		};
		struct{
			TSmallPoint Pos;
		};
	};
	LRESULT Result;
}TMSHMouseWheel;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		struct {
			BYTE ShiftState;
			BYTE Unused;
			SHORT WheelDelta;
		};
	};
	union{
		LPARAM lParam;
		struct{
			SHORT XPos;
			SHORT YPos;
		};
		TSmallPoint Pos;
	};
	LRESULT Result;
} TCMMouseWheel, *PCMMouseWheel;

typedef struct{
    UINT Msg;
    HDC DC;
	LPARAM Unused;
	LRESULT Result;
}TWMPaint;

typedef struct{
    UINT Msg;
    WPARAM Unused;
	union{
		LPARAM lPARAM;
		struct{
			SHORT XPos;
			SHORT YPos;
		};
		TSmallPoint Pos;
	};
	LRESULT Result;
}TWMNCHitTest, TCMHitTest;

typedef struct{
    UINT Msg;
	union{
		WPARAM wPARAM;
		struct{
			WORD CharCode;
			WORD Unused;
		};
	};
    LPARAM KeyData;
    LRESULT Result;
} *PWMKey, TWMKey, TWMKeyUp, TWMKeyDown, TWMKey, TWMChar, TWMSysChar, 
	TWMSysKeyDown, TWMSysKeyUp, TCMDialogKey, TCMDialogChar, 
	TCMWantSpecialKey;

typedef TMessage TWMNoParams, TWMClear, TWMClose;

typedef struct{
	UINT Msg;
	WPARAM Unused;
	LPCREATESTRUCT CreateStruct;
    LRESULT Result;
}TWMNCCreate;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HWND CtlFocus;
	};
	union{
		LPARAM lParam;
		struct{
			WORD Handle;
			WORD Unused;
		};
	};
	LRESULT Result;
} TWMNextDlgCtl;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HFONT Font;
	};
	union{
		LPARAM lParam;
		struct{
			WORD Redraw;
			WORD Unused;
		};
	};
	LRESULT Result;
}TWMSetFont;

typedef TWMNoParams TCMGotFocus;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		struct{
			WORD ItemID;
			WORD NotifyCode;
		};
	};
	union{
		LPARAM lParam;
		HWND Ctl;
	};
	LRESULT Result;
}TWMCommand;

typedef struct{
    UINT Msg;
	union{
		WPARAM wParam;
		HWND hWnd;
	};
	union{
		LPARAM lParam;
		struct{
			SHORT XPos;
			SHORT YPos;
		};
		TSmallPoint Pos;
	};
	LRESULT Result;
}TWMContextMenu;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HWND FocusedWnd;
	};
	union{
		LPARAM lParam;
		LPARAM Unused;
	};
	LRESULT Result;
}TWMSetFocus, *PWMSetFocus;

typedef TWMNoParams TWMGetDlgCode;
typedef TWMNoParams TWMNCDestroy;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HDC ChildDC;
	};
	union{
		LPARAM lParam;
		HWND ChildWnd;
	};
	LRESULT Result;
}TWMCtlColor, TWMCtlColorBtn, TWMCtlColorDlg, TWMCtlColorEdit,
	TWMCtlColorListbox, TWMCtlColorMsgbox, TWMCtlColorScrollbar,
	TWMCtlColorStatic;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HDC DC;
	};
	union{
		LPARAM lParam;
		LPARAM Unused;
	};
	LRESULT Result;
} TWMEraseBkgnd;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		WPARAM Unused;
	};
	union{
		LPARAM lParam;
		CWinControl* Sender;
	};
	LRESULT Result;
}TCMFocusChanged;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		LONG HitTest;
	};
	union{
		LPARAM lParam;
		struct{
			SHORT XCursor;
			SHORT YCursor;
		};
	};
	LRESULT Result;
} TWMNCHitMessage, TWMNCLButtonDblClk, TWMNCLButtonDown, TWMNCLButtonUp,
	TWMNCMButtonDblClk, TWMNCMButtonDown, TWMNCMButtonUp, TWMNCMouseMove;

typedef TWMNoParams TWMCancelMode;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		WPARAM IDCtrl;
	};
	union{
		LPARAM lParam;
		LPNMHDR NMHdr;
	};
	LRESULT Result;
} TWMNotify;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		WPARAM SizeType; //SIZE_MAXIMIZED, SIZE_MINIMIZED, SIZE_RESTORED, SIZE_MAXHIDE, SIZE_MAXSHOW
	};
	union{
		LPARAM lParam;
		struct{
			WORD Width;
			WORD Height;
		};
	};
	LRESULT Result;
} TWMSize;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HWND CursorWnd;
	};
	union{
		LPARAM lParam;
		struct{
			WORD HitTest;
			WORD MouseMsg;
		};
	};
	LRESULT Result;
} TWMSetCursor;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HWND IDCtl;
	};
	union{
		LPARAM lParam;
		LPMEASUREITEMSTRUCT MeasureItemStruct;
	};
	LRESULT Result;
} TWMMeasureItem;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HWND Ctl;
	};
	union{
		LPARAM lParam;
		LPDRAWITEMSTRUCT DrawItemStruct;
	};
	LRESULT Result;
} TWMDrawItem;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		struct{
			SHORT ScrollCode;//SB_xxxx 
			SHORT Pos;
		};
	};
	union{
		LPARAM lParam;
		HWND ScrollBar;
	};
	LRESULT Result;
} TWMScroll, TWMHScroll, TWMVScroll;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HWND Ctl;
	};
	union{
		LPARAM lParam;
		LPCOMPAREITEMSTRUCT CompareItemStruct;
	};
	LRESULT Result;
} TWMCompareItem;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HWND Ctl;
	};
	union{
		LPARAM lParam;
		LPDELETEITEMSTRUCT DeleteItemStruct;
	};
	LRESULT Result;
} TWMDeleteItem;

typedef TWMNoParams TCMActivate;
typedef TWMNoParams TCMDeactivate;
typedef TWMNoParams TCMEnter;
typedef TWMNoParams TCMExit;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		struct{
			WORD Active; //WA_INACTIVE, WA_ACTIVE, WA_CLICKACTIVE
			WORD Minimized;
		};
	};
	union{
		LPARAM lParam;
		HWND ActiveWindow;
	};
	LRESULT Result;
} TWMActivate;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		WPARAM Unused;
	};
	union{
		LPARAM lParam;
		LPCREATESTRUCT CreateStruct;
	};
	LRESULT Result;
} TWMCreate;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		WPARAM Unused;
	};
	union{
		LPARAM lParam;
		CControl* Sender;
	};
	LRESULT Result;
} TCMCancelMode;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		BOOL CalcValidRects;
	};
	union{
		LPARAM lParam;
		LPNCCALCSIZE_PARAMS CalcSizeParams;
	};
	LRESULT Result;
}TWMNCCalcSize;

typedef TWMNoParams TWMDestroy;

class CDragDockObject;
typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		CDragDockObject* DockSource;
	};
	union{
		LPARAM lParam;
		TSmallPoint MousePos;
	};
	LRESULT Result;
} TCMDockClient;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HRGN RGN;
	};
	union{
		LPARAM lParam;
		LPARAM Unused;
	};
	LRESULT Result;
} TWMNCPaint;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		HDC DC;
	};
	union{
		LPARAM lParam;
		UINT Flags;
	};
	LRESULT Result;
} TWMPrint, TWMPrintClient;

typedef struct _DockNotifyRec *PDockNotifyRec;
typedef struct _DockNotifyRec{
	UINT ClientMsg;
	WPARAM MsgWParam;
	LPARAM MsgLParam;
} TDockNotifyRec;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		CControl* Client;
	};
	union{
		LPARAM lParam;
		PDockNotifyRec NotifyRec;
	};
	LRESULT Result;
} TCMDockNotification;

typedef struct{
	UINT Msg;
	union{
		WPARAM wParam;
		CControl* NewTarget;
	};
	union{
		LPARAM lParam;
		CControl* Client;
	};
	LRESULT Result;
} TCMUnDockClient;

typedef struct{
    UINT Msg;
	union{
		WPARAM wParam;
		WPARAM Unused;
	};
	union{
		LPARAM lParam;
		struct{
			SHORT XPos;
			SHORT YPos;
		};
		struct{
			TSmallPoint Pos;
		};
	};
	LRESULT Result;
}TWMMove;

typedef TWMNoParams TWMGetTextLength;

typedef BYTE TDragMessage;
#define dmDragEnter		0x0
#define dmDragLeave		0x1
#define dmDragMove		0x2
#define dmDragDrop		0x3
#define dmDragCancel	0x4
#define dmFindTarget	0x5

typedef struct _DragRec{ 
	TPoint Pos;
	CObject* Source;//TODO CDragObject* Source;
	LPVOID Target; 
	BOOL Docking;
} *PDragRec, TDragRec;

typedef struct{
    UINT Msg;
	union{
		WPARAM wParam;
		struct{
			TDragMessage DragMessage;
			BYTE Reserved1;
			WORD Reserved2;
		};
	};
	union{
		LPARAM lParam;
		PDragRec DragRec;
	};
	LRESULT Result;
}TCMDrag;
#pragma pack ()