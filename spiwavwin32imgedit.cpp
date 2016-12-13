/*
 * Copyright (c) 2010-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// spiwavwin32imgedit.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "spiwavwin32imgedit.h"
#include "FreeImage.h"
#include <shellapi.h> //for CommandLineToArgW()
#include <mmsystem.h> //for timeSetEvent()
#include <stdio.h> //for swprintf()
#include <assert.h>

// Global Variables:

#define MAX_LOADSTRING 100
FIBITMAP* global_dib;
HFONT global_hFont;
HWND global_hwnd=NULL;
MMRESULT global_timer=0;
#define MAX_GLOBALTEXT	4096
WCHAR global_text[MAX_GLOBALTEXT+1];
int global_x=100;
int global_y=200;
int global_xwidth=400;
int global_yheight=400;
BYTE global_alpha=200;
int global_fontheight=24;
int global_fontwidth=-1; //will be computed within WM_PAINT handler
int global_staticheight=-1; //will be computed within WM_SIZE handler
int global_staticwidth=-1; //will be computed within WM_SIZE handler 
WNDPROC global_pfnDefEditProc = NULL;
#define IDC_MAIN_EDIT			100
#define IDC_MAIN_STATIC			101
#define WM_APP_INVALIDATECONTROLCLIENTRECT	(WM_APP+1)

HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void InvalidateControlClientRect(HWND hwnd);


LRESULT CALLBACK EditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch(uMsg)
   {
   case WM_VSCROLL:
      ::PostMessage(hWnd, WM_APP_INVALIDATECONTROLCLIENTRECT, (WPARAM)hWnd, 0);
      break;
   case WM_HSCROLL:
      ::PostMessage(hWnd, WM_APP_INVALIDATECONTROLCLIENTRECT, (WPARAM)hWnd, 0);
      break;
   case WM_MOUSEWHEEL:
      ::PostMessage(hWnd, WM_APP_INVALIDATECONTROLCLIENTRECT, (WPARAM)hWnd, 0);
      break;
   //case WM_PAINT:
   //   ::PostMessage(hWnd, WM_APP_INVALIDATECONTROLCLIENTRECT, (WPARAM)hWnd, 0);
	//   break;
	  /*
   case EM_SCROLLCARET:
      ::PostMessage(hWnd, WM_APP_INVALIDATECONTROLCLIENTRECT, (WPARAM)hWnd, 0);
	   break;
   case EM_LINESCROLL:
	   ::PostMessage(hWnd, WM_APP_INVALIDATECONTROLCLIENTRECT, (WPARAM)hWnd, 0);
	   break;
   case EM_SCROLL:
	   ::PostMessage(hWnd, WM_APP_INVALIDATECONTROLCLIENTRECT, (WPARAM)hWnd, 0);
	   break;
   case SB_LINEDOWN:
	   ::PostMessage(hWnd, WM_APP_INVALIDATECONTROLCLIENTRECT, (WPARAM)hWnd, 0);
	   break;
	   */
   //case WM_ERASEBKGND:
	//   ::PostMessage(hWnd, WM_APP_INVALIDATECONTROLCLIENTRECT, (WPARAM)hWnd, 0);
	//   break;
   case WM_APP_INVALIDATECONTROLCLIENTRECT:
      //return TransparentEdit_OnRedrawWindow(wParam, lParam);
      InvalidateControlClientRect((HWND)wParam);
	  break;
   }
   return ::CallWindowProc(global_pfnDefEditProc, hWnd, uMsg, wParam, lParam);
}

void InvalidateControlClientRect(HWND hwnd)
{
	RECT myScreenRECT;
	GetWindowRect(hwnd, &myScreenRECT);
	POINT myTopLeftPOINT;
	myTopLeftPOINT.x = myScreenRECT.left;
	myTopLeftPOINT.y = myScreenRECT.top;
	POINT myBottomRightPOINT;
	myBottomRightPOINT.x = myScreenRECT.right;
	myBottomRightPOINT.y = myScreenRECT.bottom;
	ScreenToClient(global_hwnd, &myTopLeftPOINT);
	ScreenToClient(global_hwnd, &myBottomRightPOINT);
	RECT myClientRECT;
	myClientRECT.left = myTopLeftPOINT.x;
	myClientRECT.top = myTopLeftPOINT.y;
	myClientRECT.right = myBottomRightPOINT.x;
	myClientRECT.bottom = myBottomRightPOINT.y;
	InvalidateRect(global_hwnd, &myClientRECT, FALSE);
}

void hedit_AddText(const WCHAR* lpszText)
{
	int llength = wcslen(lpszText);

	HWND hEdit = GetDlgItem(global_hwnd, IDC_MAIN_EDIT);
	//int length = GetWindowTextLength(hEdit);
	int glength = GetWindowTextW(hEdit, global_text, MAX_GLOBALTEXT);

	if((llength+glength+2)>=MAX_GLOBALTEXT)
	{
		//erase previous global_text
		//InvalidateControlClientRect(hEdit);
		SetWindowTextW(hEdit, lpszText);
	}
	else
	{
		//keep previous global_text
		wcscat(global_text, lpszText);
		//InvalidateClientRect(hEdit);
		SetWindowTextW(hEdit, global_text);
	}

	/*
	SendMessage(hEdit, EM_SETSEL, 0, MAKELPARAM(-1, 0));
	SendMessage(hEdit, EM_SCROLLCARET, 0, MAKELPARAM(FALSE, 0));
	*/
	//SendMessage(hEdit, EM_LINESCROLL, 0, MAKELPARAM(4, 0));
	
	
}

int CountSubstring(const WCHAR* str, const WCHAR* sub)
{
    int length = wcslen(sub);
    if (length == 0) return 0;
    int count = 0;
    for (str = wcsstr(str, sub); str; str = wcsstr(str + length, sub))
        ++count;
    return count;
}

void hstatic_ReplaceText(const WCHAR* lpszText)
{
	HWND hStatic = GetDlgItem(global_hwnd, IDC_MAIN_STATIC);
	InvalidateControlClientRect(hStatic);
	SetWindowTextW(hStatic, lpszText);
}

void hstatic_AddText(const WCHAR* lpszText)
{
	int maxnumberofchar = global_staticwidth*global_staticheight/(global_fontwidth*global_fontheight);  
	int maxnumberofline = global_staticheight/global_fontheight;  

	int llength = wcslen(lpszText);

	HWND hStatic = GetDlgItem(global_hwnd, IDC_MAIN_STATIC);
	//int length = GetWindowTextLength(hEdit);
	int glength = GetWindowTextW(hStatic, global_text, MAX_GLOBALTEXT);
	int numberofline = CountSubstring(global_text, L"\r\n");
	if((llength+glength+2)>=MAX_GLOBALTEXT || (llength+glength+2)>=maxnumberofchar || (numberofline+1)>=maxnumberofline)
	{
		//erase previous global_text
		hstatic_ReplaceText(lpszText);
	}
	else
	{
		//keep previous global_text
		wcscat(global_text, lpszText);
		SetWindowTextW(hStatic, global_text);
	}	
}
void CALLBACK StartGlobalProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	WCHAR buffer[1024];
	int pause_ms = 1000;
	//for(int ii=0; ii<30; ii++)
	for(int ii=0; ii<1000; ii++)
	{
		Sleep(pause_ms);
		swprintf(buffer, L"%i, some long chain of text, presumed to eventually go behond borders.\r\n", ii);
		hedit_AddText(buffer);
		//hstatic_ReplaceText(buffer);
		hstatic_AddText(buffer);
		swprintf(buffer, L"%i, some extra text added.\r\n", ii);
		hstatic_AddText(buffer);
		swprintf(buffer, L"%i, more extra text added.\r\n", ii);
		hstatic_AddText(buffer);
		swprintf(buffer, L"%i, even more extra text added.\r\n", ii);
		hstatic_AddText(buffer);
		swprintf(buffer, L"%i, even even more extra text added.\r\n", ii);
		hstatic_AddText(buffer);
	}
	PostMessage(global_hwnd, WM_DESTROY, 0, 0);
}




PCHAR*
    CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
    {
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
            i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( a = CmdLine[i] ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
    }

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//LPWSTR *szArgList;
	LPSTR *szArgList;
	int nArgs;
	int i;

	//szArgList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	if(nArgs>1)
	{
		global_x = atoi(szArgList[1]);
	}
	if(nArgs>2)
	{
		global_y = atoi(szArgList[2]);
	}
	if(nArgs>3)
	{
		global_xwidth = atoi(szArgList[3]);
	}
	if(nArgs>4)
	{
		global_yheight = atoi(szArgList[4]);
	}
	if(nArgs>5)
	{
		global_alpha = atoi(szArgList[5]);
	}
	LocalFree(szArgList);
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SPIWAVWIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPIWAVWIN32));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPIWAVWIN32));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	//wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SPIWAVWIN32); //original with menu
	wcex.lpszMenuName	= NULL; //no menu
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   //global_dib = FreeImage_Load(FIF_PNG, "test.png", PNG_DEFAULT);
   global_dib = FreeImage_Load(FIF_JPEG, "background.jpg", JPEG_DEFAULT);

   //global_hFont=CreateFontW(32,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
   global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");

   //hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, //original with WS_CAPTION etc.
   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_VISIBLE, //no WS_CAPTION etc.
      //CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
      //100, 200, 400, 400, NULL, NULL, hInstance, NULL);
      global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   global_hwnd = hWnd;

	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HGDIOBJ hOldBrush;
	HGDIOBJ hOldPen;
	int iOldMixMode;
	COLORREF crOldBkColor;
	COLORREF crOldTextColor;
	int iOldBkMode;
	HFONT hOldFont, hFont;
	TEXTMETRIC myTEXTMETRIC;

	switch (message)
	{
	case WM_CREATE:
		{
			
			//HWND hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", 
			HWND hEdit = CreateWindowEx(WS_EX_TRANSPARENT, L"EDIT", L"", 
				//WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 
				WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, //this one 
				//WS_CHILD | WS_VISIBLE | ES_MULTILINE, //this one 
				//WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 
				0, 0, 100, 100, hWnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);
			if(hEdit == NULL)
				MessageBox(hWnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);
			//HFONT hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			//SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			SendMessage(hEdit, WM_SETFONT, (WPARAM)global_hFont, MAKELPARAM(FALSE, 0));
			//SendMessage(hEdit, EM_SETREADONLY, (WPARAM)TRUE, MAKELPARAM(FALSE, 0));

			// Subclass edit control
			global_pfnDefEditProc = (WNDPROC)SetWindowLong(hEdit, GWL_WNDPROC, (LONG)EditProc);


			HWND hStatic = CreateWindowEx(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER,  
				0, 100, 100, 100, hWnd, (HMENU)IDC_MAIN_STATIC, GetModuleHandle(NULL), NULL);
			if(hStatic == NULL)
				MessageBox(hWnd, L"Could not create static text.", L"Error", MB_OK | MB_ICONERROR);
			SendMessage(hStatic, WM_SETFONT, (WPARAM)global_hFont, MAKELPARAM(FALSE, 0));



			global_timer=timeSetEvent(2000,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
		}
		break;
	case WM_SIZE:
		{
			RECT rcClient;

			GetClientRect(hWnd, &rcClient);
			
			HWND hEdit = GetDlgItem(hWnd, IDC_MAIN_EDIT);
			SetWindowPos(hEdit, NULL, 0, 0, rcClient.right/2, rcClient.bottom/2, SWP_NOZORDER);
			
			HWND hStatic = GetDlgItem(hWnd, IDC_MAIN_STATIC);
			global_staticwidth = rcClient.right - 0;
			global_staticheight = rcClient.bottom-(rcClient.bottom/2);
			//global_staticheight = rcClient.bottom-0;
			SetWindowPos(hStatic, NULL, 0, rcClient.bottom/2, global_staticwidth, global_staticheight, SWP_NOZORDER);
			//SetWindowPos(hStatic, NULL, 0, 0, global_staticwidth, global_staticheight, SWP_NOZORDER);
		}
		break;
	//case WM_ERASEBKGND:
	//	break;
	case WM_CTLCOLOREDIT:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(0xFF, 0xFF, 0xFF));
			return (INT_PTR)::GetStockObject(NULL_PEN);
		}
		break;
	case WM_CTLCOLORSTATIC:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(0xFF, 0xFF, 0xFF));
			return (INT_PTR)::GetStockObject(NULL_PEN);
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		SetStretchBltMode(hdc, COLORONCOLOR);
		StretchDIBits(hdc, 0, 0, global_xwidth, global_yheight,
						0, 0, FreeImage_GetWidth(global_dib), FreeImage_GetHeight(global_dib),
						FreeImage_GetBits(global_dib), FreeImage_GetInfo(global_dib), DIB_RGB_COLORS, SRCCOPY);

		hOldBrush = SelectObject(hdc, (HBRUSH)GetStockObject(GRAY_BRUSH));
		hOldPen = SelectObject(hdc, (HPEN)GetStockObject(WHITE_PEN));
		//iOldMixMode = SetROP2(hdc, R2_MASKPEN);
		iOldMixMode = SetROP2(hdc, R2_MERGEPEN);
		//Rectangle(hdc, 100, 100, 200, 200);

		crOldBkColor = SetBkColor(hdc, RGB(0xFF, 0x00, 0x00));
		crOldTextColor = SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
		iOldBkMode = SetBkMode(hdc, TRANSPARENT);
		//hFont=CreateFontW(70,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
		//hOldFont=(HFONT)SelectObject(hdc,global_hFont);
		hOldFont=(HFONT)SelectObject(hdc,global_hFont);
		GetTextMetrics(hdc, &myTEXTMETRIC);
		global_fontwidth = myTEXTMETRIC.tmAveCharWidth;
		//TextOutW(hdc, 100, 100, L"test string", 11);

		SelectObject(hdc, hOldBrush);
		SelectObject(hdc, hOldPen);
		SetROP2(hdc, iOldMixMode);
		SetBkColor(hdc, crOldBkColor);
		SetTextColor(hdc, crOldTextColor);
		SetBkMode(hdc, iOldBkMode);
		SelectObject(hdc,hOldFont);
		//DeleteObject(hFont);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		if (global_timer) timeKillEvent(global_timer);
		FreeImage_Unload(global_dib);
		DeleteObject(global_hFont);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
