#pragma once

#include "stdinc.h"
#include "Object.hpp"

typedef LRESULT (CObject::*TObjectWndProc)(UINT Msg, WPARAM wParam, LPARAM lParam);

LPVOID MakeObjectWndProc(LPVOID Object, TObjectWndProc ObjectWndProcAddr);

VOID FreeObjectWndProc(LPVOID ObjectWndProc);

HWND AllocateHWnd(HINSTANCE HInstance, LPVOID Object, TObjectWndProc ObjectWndProcAddr);

void DeallocateHWnd(HWND Wnd);