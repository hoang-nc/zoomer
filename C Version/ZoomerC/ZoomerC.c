#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
#include "resource.h"

#define SYS_ICON_ID 100
#define SWM_TRAYMSG	WM_APP /* the message ID sent to our window */
#define SWM_INFO	WM_APP + 1 /* show the window */
#define SWM_EXIT	WM_APP + 2 /* close the window */

// Global Variables:
static MSG msg;
static HACCEL hAccelTable;
static HINSTANCE ghInstance;


static HHOOK s_MouseHookHandle = 0;
static void InstallMouseHook(_In_ HINSTANCE hInstance);

static UINT_PTR gTimerRClickID = 1309;

static BOOL gIsMouseRightDown = FALSE;
static BOOL gIsVirtualControlDown = FALSE;

volatile BOOL gSendingRClick = FALSE;
volatile UINT WM_TASKBARCREATED; 

static char SINGLE_WINDOWS_TITLE[] = "9907567546";
#define START_INFO_URL "start http://nchoang.github.io/zoomer"

static INPUT gKeyboardRCtrlDown = {
	INPUT_KEYBOARD,		/*	type */
	{
		VK_RCONTROL,	/* Keycode & Scan */
			KEYEVENTF_EXTENDEDKEY, /* Flag ExtendedKey and KeyDown  */
	}
};


static INPUT gKeyboardRCtrlUp = {
	INPUT_KEYBOARD,		/*	type */
	{
		VK_RCONTROL,	/* Keycode & Scan */
			KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, /* Flag ExtendedKey and KeyUp  */
	}
};

static INPUT gMouseRDown = {
	INPUT_MOUSE,
	{
		0, 0, /* x, y */
			0,    /* data */
			MOUSEEVENTF_RIGHTDOWN /* Flag */
	}
};

static INPUT gMouseRUp = {
	INPUT_MOUSE,
	{
		0, 0, /* x, y */
			0,    /* data */
			MOUSEEVENTF_RIGHTUP /* Flag */
	}
};

static NOTIFYICONDATA gNotifyIcon = {
	sizeof(NOTIFYICONDATA),
	NULL,
	SYS_ICON_ID,
	NIF_ICON | NIF_MESSAGE | NIF_TIP,
	0,
};

static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static void Init()
{
	gNotifyIcon.hWnd = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DIALOG_DUMP), NULL, &DlgProc);
	SetWindowText(gNotifyIcon.hWnd, SINGLE_WINDOWS_TITLE);

	LoadString(ghInstance, IDS_APP_TITLE, gNotifyIcon.szTip, sizeof(gNotifyIcon.szTip));

	gNotifyIcon.uCallbackMessage = SWM_TRAYMSG;

	gNotifyIcon.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_ZOOMERC));
	Shell_NotifyIcon(NIM_ADD, &gNotifyIcon);
	DestroyIcon(gNotifyIcon.hIcon);

	WM_TASKBARCREATED = RegisterWindowMessage("TaskbarCreated");

	InstallMouseHook(ghInstance);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance,
				   _In_opt_ HINSTANCE hPrevInstance,
				   _In_ LPTSTR    lpCmdLine,
				   _In_ int       nCmdShow)
{
	if (FindWindow(NULL, SINGLE_WINDOWS_TITLE)) return 0;
	ghInstance = hInstance;
	Init();

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDI_ZOOMERC));
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

static __inline void SendCtrlDown()
{
	SendInput(1, &gKeyboardRCtrlDown, sizeof(INPUT));
};

static __inline void SendCtrlUp()
{
	SendInput(1, &gKeyboardRCtrlUp, sizeof(INPUT));
};

static void CALLBACK TimerRClickProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	gSendingRClick = TRUE;
	SendInput(1, &gMouseRDown, sizeof(INPUT));
	SendInput(1, &gMouseRUp, sizeof(INPUT));
	gSendingRClick = FALSE;
}

static __inline void StartSendingRClick()
{
	SetTimer(0, gTimerRClickID, 1, &TimerRClickProc);
}


static LRESULT CALLBACK MouseHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0 && !gSendingRClick) 
	{
		switch (wParam)
		{

		case WM_RBUTTONDOWN:
			gIsMouseRightDown = TRUE;
			return -1;
			break;
		case WM_RBUTTONUP:
			gIsMouseRightDown = FALSE;
			if (gIsVirtualControlDown)
			{
				SendCtrlUp();
				gIsVirtualControlDown = FALSE;
			}
			else
			{
				StartSendingRClick();
			}
			return -1;
			break;
		case WM_MOUSEWHEEL:
			if (gIsMouseRightDown && !gIsVirtualControlDown)
			{
				SendCtrlDown();
				gIsVirtualControlDown = TRUE;
			}
			break;
		}
	}

	return CallNextHookEx(s_MouseHookHandle, code, wParam, lParam);
}

static void InstallMouseHook(_In_ HINSTANCE hInstance)
{
	// install Mouse hook only if it is not installed and must be installed
	if (s_MouseHookHandle == 0)
	{
		//install hook
		s_MouseHookHandle = SetWindowsHookEx(WH_MOUSE_LL, &MouseHookProc, hInstance, 0);
		//If SetWindowsHookEx fails.
		if (s_MouseHookHandle == 0)
		{
			MessageBox(0, "Zoomer fail to run!", "Zoomer", 0);
		}
	}
}

static void ShowContextMenu(HWND hWnd)
{
	static POINT pt;
	static HMENU hMenu = NULL;

	if (hMenu == NULL){
		hMenu = CreatePopupMenu();
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_INFO, "Info");
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, "Exit");
	}
	// note:	must set window to the foreground or the
	//			menu won't disappear when it should
	SetForegroundWindow(hWnd);

	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_BOTTOMALIGN,	pt.x, pt.y, 0, hWnd, NULL );
}

// Message handler for the app
static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case SWM_TRAYMSG:
		switch(lParam)
		{
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			ShowContextMenu(hWnd);
		}
		return 1;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case SWM_INFO:
			system(START_INFO_URL);
			break;
		case SWM_EXIT:
			Shell_NotifyIcon(NIM_DELETE, &gNotifyIcon);
			DestroyIcon(gNotifyIcon.hIcon);
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		}
		return 1;
	default:
		if (WM_TASKBARCREATED == message)
		{
			gNotifyIcon.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_ZOOMERC));
			Shell_NotifyIcon(NIM_ADD, &gNotifyIcon);
			DestroyIcon(gNotifyIcon.hIcon);
		}
	}
	return 0;
}
