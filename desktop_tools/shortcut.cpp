#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

//for vim style.
#define VK_H 0x48
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C

ATOM UpKeyId;
ATOM DownKeyId;
ATOM LeftKeyId;
ATOM RightKeyId;

static void GetWorkArea(PRECT rect)
{
	ZeroMemory(rect, sizeof(*rect));
	SystemParametersInfo(SPI_GETWORKAREA, 0, rect, 0);
}

static void GetLeftWorkArea(PRECT rect)
{
	GetWorkArea(rect);
	rect->right = rect->left + (rect->right - rect->left) / 2;
}

static void GetRightWorkArea(PRECT rect)
{
	GetWorkArea(rect);
	rect->left = rect->right - (rect->right - rect->left) / 2;
}

static void ForceWindowForeground(HWND hwnd)
{
	return;

	DWORD LockTimeOut = 0;
	HWND  ForegroundWnd = GetForegroundWindow();
	DWORD CurrentId = GetCurrentThreadId();
	DWORD ForegroundId = GetWindowThreadProcessId(ForegroundWnd, 0);
	BOOL Attach = (CurrentId != ForegroundId);
	BOOL TopMost = (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST);

	if (!TopMost)
		SetWindowPos(hwnd, HWND_TOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	if (Attach) {
		AttachThreadInput(CurrentId, ForegroundId, TRUE);
		SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT,
			0, &LockTimeOut, 0);
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0,
			SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
		AllowSetForegroundWindow(ASFW_ANY);
	}

	SetForegroundWindow(hwnd);

	if (Attach) {
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,
			0, (PVOID)LockTimeOut,
			SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
		AttachThreadInput(CurrentId, ForegroundId, FALSE);
	}

	if (!TopMost)
		SetWindowPos(hwnd, HWND_NOTOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

static HWND GetCurrentWindow(void)
{
	return GetForegroundWindow();
}

static void moveWindow(HWND hwnd, PRECT rect)
{
	LONG width = rect->right - rect->left;
	LONG height = rect->bottom - rect->top;
	if (width && height) {
		ShowWindow(hwnd, SW_SHOWNA);
		MoveWindow(hwnd, rect->left, rect->top, width, height, TRUE);
		ForceWindowForeground(hwnd);
	}
}

static void moveLeft(HWND hwnd)
{
	RECT rect;
	GetLeftWorkArea(&rect);
	moveWindow(hwnd, &rect);
}

static void moveRight(HWND hwnd)
{
	RECT rect;
	GetRightWorkArea(&rect);
	moveWindow(hwnd, &rect);
}

static void moveUp(HWND hwnd)
{
	ForceWindowForeground(hwnd);
	ShowWindow(hwnd, SW_SHOWMAXIMIZED);
}

static void moveDown(HWND hwnd)
{
	ShowWindow(hwnd, SW_SHOWMINIMIZED);
}

void DispatchKey(UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;

	if (message == WM_HOTKEY &&
		(hwnd = GetCurrentWindow()))
	{
		if (wParam == UpKeyId)
			moveUp(hwnd);
		else if (wParam == DownKeyId)
			moveDown(hwnd);
		else if (wParam == LeftKeyId)
			moveLeft(hwnd);
		else if (wParam == RightKeyId)
			moveRight(hwnd);
	}
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   BOOL ret;

   if (HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL))
   {
		ret = RegisterHotKey(hwnd,
			UpKeyId = GlobalAddAtom(_T("Welfear::Hotkey::UP")),
			MOD_WIN, VK_UP);
		if (!ret)
			goto error;

		ret = RegisterHotKey(hwnd,
		DownKeyId = GlobalAddAtom(_T("Welfear::Hotkey::down")),
			MOD_WIN, VK_DOWN);
		if (!ret)
			goto error;

		ret = RegisterHotKey(hwnd,
			LeftKeyId = GlobalAddAtom(_T("Welfear::Hotkey::left")),
			MOD_WIN, VK_LEFT);
		if (!ret)
			goto error;

		ret = RegisterHotKey(hwnd,
			RightKeyId = GlobalAddAtom(_T("Welfear::Hotkey::right")),
			MOD_WIN, VK_RIGHT);
		if (!ret)
			goto error;

		//ShowWindow(hWnd, nCmdShow);
		//UpdateWindow(hWnd);

		return ret;
   }

error:
   *(PCHAR)NULL = 0;
}
