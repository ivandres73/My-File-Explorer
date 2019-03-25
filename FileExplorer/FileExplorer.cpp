
#include "stdafx.h"
#include "FileExplorer.h"

HINSTANCE hInst;							// current instance
TCHAR szTitle[STRING_SIZE];					// The title bar text
TCHAR szWindowClass[STRING_SIZE];			// the main window class name
HWND g_listView;							//global list handle
HWND g_treeView;							//global tree handle
HWND g_hWnd;
RECT rcClient;
vector<TCHAR*>g_nameFile;
vector<TCHAR*>g_nameFolder;
LPCWSTR currentPath;
short create_a;
bool moveActivate, copyActivate;
LPCWSTR tempPath;//used for move and copy functions
LPCWSTR folderPath;// used for copy folder
TCHAR nameFile[100];//used for the name of the file in movefile()

bool createMainWindow(HINSTANCE, int, WNDCLASS);
void createTreeView(int x, int y, int height, int width, HWND hWnd);
void createListView(HWND hWnd);
void loadOrExecSelected(HWND g_listView);
LPWSTR getDateModified(FILETIME ftLastWrite);
LPCWSTR getPathForListView(HWND m_hListView, int iItem);
LPCWSTR getPathForTreeView(HTREEITEM hItem);
void loadChildForListView(LPCWSTR path, Drive *drive);
char *convertByteToStringSize(DWORD fileSizeLow);
bool findNameItem(vector<TCHAR*>g_nameFile, char *name);
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
void PathtoFile(LPCWSTR path, TCHAR*);
void loadDirItemToLisview(HWND hWnd, HWND g_listView, LPCWSTR path);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	copyActivate = false;
	moveActivate = false;
	MSG msg;
	HACCEL hAccelTable;

	WNDCLASS wc = { };

	createMainWindow(hInstance, nCmdShow, wc);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYFILEEXPLORER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

bool createMainWindow(HINSTANCE hInstance, int nCmdShow, WNDCLASS wc)
{
	LoadString(hInstance, IDS_APP_TITLE, szTitle, STRING_SIZE);
	LoadString(hInstance, IDC_MYFILEEXPLORER, szWindowClass, STRING_SIZE);

	//wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FILE_EXPLORER));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDC_MYFILEEXPLORER);
	wc.lpszClassName = szWindowClass;
	//wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_FILE_EXPLORER));

	RegisterClass(&wc);
	hInst = hInstance;

	HWND hWnd = CreateWindowEx(
		0,                              // Optional window styles.
		szWindowClass,                  // Window class
		L"Windows Explorer",			// Window text
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (!hWnd)
	{
		return false;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return true;
}

void loadMyComputerToTreeView(HWND g_treeView, Drive *drive)
{
	TV_INSERTSTRUCT tvInsert;

	tvInsert.hParent = nullptr;
	tvInsert.hInsertAfter = TVI_ROOT;
	tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	//tvInsert.item.iImage = IDI_COMPUTER;
	//tvInsert.item.iSelectedImage = IDI_COMPUTER;
	tvInsert.item.pszText = L"My Computer";
	tvInsert.item.lParam = (LPARAM)_T("My Computer");
	HTREEITEM hMyComputer = TreeView_InsertItem(g_treeView, &tvInsert);

	for (int i = 0; i < drive->_numberOfDrive; i++)
	{
		tvInsert.hParent = hMyComputer; //Them
		tvInsert.item.iImage = drive->icon[i];
		tvInsert.item.iSelectedImage = drive->icon[i];
		tvInsert.item.pszText = drive->volumeName[i];
		tvInsert.item.lParam = (LPARAM)drive->driveLetter[i];
		HTREEITEM hDrive = TreeView_InsertItem(g_treeView, &tvInsert);

		if (drive->icon[i] == IDI_DATA || drive->icon[i] == IDI_OS)
		{
			tvInsert.hParent = hDrive; //Them
			tvInsert.item.pszText = _T("PreLoad"); //Them
			tvInsert.item.lParam = (LPARAM)_T("PreLoad");
			TreeView_InsertItem(g_treeView, &tvInsert);
			MessageBox(0, L"icon", L"icon if inside", 0);
		}
		TreeView_Expand(g_treeView, hMyComputer, TVE_EXPAND);
		TreeView_SelectItem(g_treeView, hMyComputer);
	}

}

void loadMyComputerToListView(HWND g_listView, Drive *drive)
{
	LV_ITEM lv;
	for (int i = 0; i < drive->_numberOfDrive; i++)
	{
		lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		lv.iItem = i;
		lv.iImage = 0;
		lv.iSubItem = 0;
		lv.pszText = drive->volumeName[i];
		lv.lParam = (LPARAM)drive->driveLetter[i];
		ListView_InsertItem(g_listView, &lv);
		
		lv.mask = LVIF_TEXT;

		lv.iSubItem = 1;
		lv.pszText = drive->driveType[i];
		ListView_SetItem(g_listView, &lv);
	}
}

void createTreeView(int x, int y, int height, int width, HWND hWnd)
{

	g_treeView = CreateWindowEx(0, WC_TREEVIEW, L"TREE VIEW", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_SIZEBOX | WS_VSCROLL | WS_TABSTOP | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, x, y, width, height, hWnd, (HMENU)IDM_TREE_VIEW, hInst, nullptr);

}

void createListView(HWND hWnd)
{
	g_listView = CreateWindow(WC_LISTVIEWW, L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | WS_HSCROLL | WS_VSCROLL, rcClient.left + LEFT_WINDOW_WIDTH + SPLITTER_BAR_WIDTH,
		0,
		rcClient.right - (rcClient.left + LEFT_WINDOW_WIDTH + SPLITTER_BAR_WIDTH),
		(rcClient.bottom - rcClient.top) - (TOP_POS + BOTTOM_POS),
		hWnd,
		NULL, hInst, NULL);
	LVCOLUMN lvCol1;

	lvCol1.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvCol1.fmt = LVCFMT_LEFT;

	lvCol1.cx = 150;
	lvCol1.pszText = _T("Name");
	ListView_InsertColumn(g_listView, 0, &lvCol1);

	LVCOLUMN lvCol2;
	lvCol2.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvCol2.fmt = LVCFMT_LEFT | LVCF_WIDTH;
	lvCol2.cx = 100;
	lvCol2.pszText = _T("Size");
	ListView_InsertColumn(g_listView, 1, &lvCol2);

	LVCOLUMN lvCol3;
	lvCol3.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvCol3.fmt = LVCFMT_CENTER;
	lvCol3.cx = 100;
	lvCol3.pszText = _T("Type");
	ListView_InsertColumn(g_listView, 2, &lvCol3);

	LVCOLUMN lvCol4;
	lvCol4.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvCol4.fmt = LVCFMT_CENTER;
	lvCol4.cx = 300;
	lvCol4.pszText = _T("Data modified");
	ListView_InsertColumn(g_listView, 3, &lvCol4);
}

BOOL CALLBACK DialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				WCHAR nameFile[40];
				GetDlgItemText(hDlg, IDC_EDIT1, (LPWSTR)nameFile, 40 - 1);
				WCHAR fullPath[1024];
				PathCombineW(fullPath, currentPath, nameFile);
				switch (create_a)
				{
				case 0:
					{
						HANDLE hfile = CreateFile(
							fullPath,               // Notice the L for a wide char literal
							GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							CREATE_NEW,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
						CloseHandle(hfile);
					}
					break;
				case 1:
					{
						CreateDirectory(fullPath, 0);
					}
					break;
				case 5:
					{
						LPCWSTR Oldpath;
						int iPos = ListView_GetNextItem(g_listView, -1, LVNI_SELECTED);
						Oldpath = getPathForListView(g_listView, iPos);
						MoveFile(Oldpath, fullPath);
					}
					break;
				case 6:
					{
						CopyFile(tempPath, fullPath, FALSE);
						copyActivate = false;
						moveActivate = false;
					}
					break;
				case 7:
					{
						SHFILEOPSTRUCT s = { 0 };
						s.hwnd = g_hWnd;
						s.wFunc = FO_COPY;
						s.fFlags = FOF_SILENT;

						WCHAR NewTempPath[1024];
						WCHAR* asterisc = L"\*";
						PathCombineW(NewTempPath, folderPath, asterisc);
						s.pTo = fullPath;
						s.pFrom = NewTempPath;
						SHFileOperation(&s);
						copyActivate = false;
						moveActivate = false;
					}
					break;
				default:
					break;
				}
				EndDialog(hDlg, 0);
			}
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return FALSE;
		}
		break;
	}
	return FALSE;
}

BOOL CALLBACK DialogProcLink(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		{
			WCHAR Origin[40];
			WCHAR Output[40];
			GetDlgItemText(hDlg, IDC_EDITORIGIN, (LPWSTR)Origin, 40 - 1);
			GetDlgItemText(hDlg, IDC_EDITOUTPUT, (LPWSTR)Output, 40 - 1);
			WCHAR fullPathOrigin[1024];
			WCHAR fullPathOutput[1024];
			PathCombineW(fullPathOrigin, currentPath, Origin);
			PathCombineW(fullPathOutput, currentPath, Output);
			switch (create_a)
			{
			case 2:
				CreateSymbolicLink(fullPathOutput, fullPathOrigin, 0);//0 for symlink to files
				break;
			case 3:
				CreateSymbolicLink(fullPathOutput, fullPathOrigin, 1);//1 for symlink to files
				break;
			case 4:
			{
				BOOL si = CreateHardLink(fullPathOutput, fullPathOrigin, NULL);
				EndDialog(hDlg, 0);
				if (!si)
					MessageBox(0, L"hard link", L"hard link not created", 0);
				return FALSE;
			}
			default:
				break;
			}
			EndDialog(hDlg, 0);
		}
		return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return FALSE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	
	Drive *drive = new Drive();
	NMHDR* notifyMess = nullptr;
	drive->getSystemDrives();
	LPNMTREEVIEW lpnmTree;
	LPCWSTR path;
	switch (message)
	{
	case WM_CREATE:
		INITCOMMONCONTROLSEX icex;

		// Ensure that the common control DLL is loaded. 
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES;
		InitCommonControlsEx(&icex);

		GetClientRect(hWnd, &rcClient);

		createTreeView(0, 0, 530, 200, hWnd);
		createListView(hWnd);

		loadMyComputerToListView(g_listView, drive);
		loadMyComputerToTreeView(g_treeView, drive);
		SetFocus(g_treeView);

		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			delete(drive);
			DestroyWindow(hWnd);
			break;
		case IDM_CREATEFILE:
			create_a = 0;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hWnd, DialogProc);
			break;
		case IDM_CREATEDIR:
			create_a = 1;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hWnd, DialogProc);
			break;
		case IDM_CREATELINKFILE:
			create_a = 2;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMLINK), hWnd, DialogProcLink);
			break;
		case IDM_CREATELINKDIR:
			create_a = 3;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMLINK), hWnd, DialogProcLink);
			break;
		case IDM_CREATEHARD:
			create_a = 4;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMLINK), hWnd, DialogProcLink);
			break;
		case IDM_RENAME:
			create_a = 5;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hWnd, DialogProc);
			break;
		case IDM_COPY:
			{
				moveActivate = false;
				copyActivate = true;
				int iPos = ListView_GetNextItem(g_listView, -1, LVNI_SELECTED);
				tempPath = getPathForListView(g_listView, iPos);

				LPCWSTR filePath = getPathForListView(g_listView, ListView_GetSelectionMark(g_listView));
				WIN32_FIND_DATA fd;

				GetFileAttributesEx(filePath, GetFileExInfoStandard, &fd);
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//if is dir
				{
					create_a = 7;
					folderPath = getPathForListView(g_listView, iPos);
				}
				else//if is file
				{
					create_a = 6;;
				}
			}
			break;
		case IDM_COPYHERE:
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hWnd, DialogProc);
			}
			break;
		case IDM_MOVE:
			{
				copyActivate = false;
				moveActivate = true;
				int iPos = ListView_GetNextItem(g_listView, -1, LVNI_SELECTED);
				tempPath = getPathForListView(g_listView, iPos);
				PathtoFile(tempPath, nameFile);
			}
			break;
		case IDM_MOVEHERE:
			{
				WCHAR absPath[100];
				PathCombineW(absPath, currentPath, nameFile);
				//MessageBox(0, tempPath, L"Origin", 0);
				//MessageBox(0, absPath, L"destination", 0);
				MoveFile(tempPath, absPath);
				copyActivate = false;
				moveActivate = false;
			}
			break;
		case IDM_DELETE:
			{
				int msgboxID = MessageBox(hWnd, L"Are you sure you want to delete this file ?", L"Delete", MB_ICONEXCLAMATION | MB_YESNO);
				if (msgboxID == IDYES)
				{
					int iPos = ListView_GetNextItem(g_listView, -1, LVNI_SELECTED);
					while (iPos != -1) {
						path = getPathForListView(g_listView, iPos);
						ListView_DeleteItem(g_listView, iPos);
						wstring wStr((wstring)path);
						const wchar_t *str = wStr.c_str();
						size_t size = wcslen(str) * 2 + 2;
						char * StartPoint = new char[size];
						size_t c_size;
						wcstombs_s(&c_size, StartPoint, size, str, size);
						if (findNameItem(g_nameFile, StartPoint))
							remove(StartPoint);
						else
							RemoveDirectory(path);
						iPos = ListView_GetNextItem(g_listView, iPos, LVNI_SELECTED);
					}
				}
			}
			break;
		case IDM_OPEN:
			{
				loadOrExecSelected(g_listView);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_NOTIFY:
		notifyMess = (NMHDR*)lParam;
		lpnmTree = (LPNMTREEVIEW)notifyMess;
		switch (notifyMess->code)
		{
		case NM_DBLCLK:
			loadOrExecSelected(g_listView);
			break;
		case NM_RCLICK:
			{
				HMENU hMenu;
				POINT cursor; // Getting the cursor position
				GetCursorPos(&cursor);
				if (copyActivate)
					TrackPopupMenu((HMENU)GetSubMenu(LoadMenu(hInst, MAKEINTRESOURCE(IDR_POPUPCOPY)), 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, hWnd, NULL);
				else if (moveActivate)
					TrackPopupMenu((HMENU)GetSubMenu(LoadMenu(hInst, MAKEINTRESOURCE(IDR_POPUPMOVE)), 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, hWnd, NULL);
				else
					TrackPopupMenu((HMENU)GetSubMenu(LoadMenu(hInst, MAKEINTRESOURCE(IDR_POPUPMENU)), 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, hWnd, NULL);
			}
			break;
		case TVN_SELCHANGED:
			ListView_DeleteAllItems(g_listView);
			loadChildForListView(getPathForTreeView(TreeView_GetNextItem(g_treeView, NULL, TVGN_CARET)), drive);
		default:
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LPWSTR getDateModified(FILETIME ftLastWrite)
{
	SYSTEMTIME stUTC, stLocal;
	FileTimeToSystemTime(&ftLastWrite, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	TCHAR *buffer = new TCHAR[50];
	wsprintf(buffer, _T("%02d/%02d/%04d %02d:%02d %s"),
		stLocal.wDay, stLocal.wMonth, stLocal.wYear,
		(stLocal.wHour>12) ? (stLocal.wHour / 12) : (stLocal.wHour),
		stLocal.wMinute,
		(stLocal.wHour>12) ? (_T("PM")) : (_T("AM")));

	return buffer;
}

void loadDirItemToLisview(HWND hWnd, HWND g_listView, LPCWSTR path)
{
	TCHAR buffer[10240];
	StrCpy(buffer, path);

	if (wcslen(path) == 3)
		StrCat(buffer, _T("*"));
	else
		StrCat(buffer, _T("\\*"));
	WIN32_FIND_DATA fd;
	HANDLE hFile;
	BOOL bFound = true;
	LV_ITEM lv, lv2;
	TCHAR * folder_path = nullptr;
	int itemIndexFolder = 0;
	int fileSizeCount = 0;
	hFile = FindFirstFileW(buffer, &fd);
	bFound = TRUE;
	TCHAR * file_path = nullptr;
	int itemIndexFile = 0;

	if (hFile == INVALID_HANDLE_VALUE)
		bFound = FALSE;

	while (bFound)
	{
		if (((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) &&
			((fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != FILE_ATTRIBUTE_SYSTEM) &&
			((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != FILE_ATTRIBUTE_HIDDEN))
		{
			file_path = new TCHAR[wcslen(path) + wcslen(fd.cFileName) + 2];
			StrCpy(file_path, path);

			if (wcslen(path) != 3)
				StrCat(file_path, _T("\\"));

			StrCat(file_path, fd.cFileName);

			lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			lv.iItem = itemIndexFile;
			lv.iSubItem = 0;
			lv.pszText = fd.cFileName;
			lv.lParam = (LPARAM)file_path;
			lv.iImage = 0;
			ListView_InsertItem(g_listView, &lv);

			DWORD fileSizeLow = fd.nFileSizeLow; //The low-order DWORD value of the file size, in bytes
			ListView_SetItemText(g_listView, itemIndexFile, 1, (TCHAR*)convertByteToStringSize(fileSizeLow));
			ListView_SetItemText(g_listView, itemIndexFile, 2, L"File");
			ListView_SetItemText(g_listView, itemIndexFile, 3, getDateModified(fd.ftLastWriteTime));
			g_nameFile.push_back(file_path);
			++itemIndexFile;
		}
		else
		{
			folder_path = new TCHAR[wcslen(path) + wcslen(fd.cFileName) + 2];
			StrCpy(folder_path, path);

			if (wcslen(path) != 3)
				StrCat(folder_path, _T("\\"));

			StrCat(folder_path, fd.cFileName);
			lv2.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			lv2.iItem = itemIndexFolder;
			lv2.iSubItem = 0;
			lv2.pszText = fd.cFileName;
			lv2.lParam = (LPARAM)folder_path;
			lv2.iImage = 0;
			ListView_InsertItem(g_listView, &lv2);

			ListView_SetItemText(g_listView, itemIndexFolder, 2, _T("Folder"));

			ListView_SetItemText(g_listView, itemIndexFolder, 3, getDateModified(fd.ftLastWriteTime));
			++itemIndexFolder;
			g_nameFolder.push_back(folder_path);
		}

		bFound = FindNextFileW(hFile, &fd);
	}//end of while
}

void loadOrExecSelected(HWND g_listView)
{
	LPCWSTR filePath = getPathForListView(g_listView, ListView_GetSelectionMark(g_listView));
	currentPath = filePath;
	WIN32_FIND_DATA fd;

	//Retrieves attributes for a specified file or directory.
	if (GetFileAttributesEx(filePath, GetFileExInfoStandard, &fd) != 0)
	{
		//Check whether it's folder or directory
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//Delete and reload item in Listview
			ListView_DeleteAllItems(g_listView);
			loadDirItemToLisview(g_hWnd, g_listView, filePath);
		}
		else
		{
			ShellExecute(NULL, _T("open"), filePath, NULL, NULL, SW_SHOWNORMAL);
		}
	}
}

HTREEITEM getMyComputer(HWND g_treeView)
{
	return TreeView_GetChild(g_treeView, TreeView_GetRoot(g_treeView));
}

LPCWSTR getPathForTreeView(HTREEITEM hItem)
{
	TVITEMEX tv;
	tv.mask = TVIF_PARAM;
	tv.hItem = hItem;
	TreeView_GetItem(g_treeView, &tv);
	return (LPCWSTR)tv.lParam;
}

LPCWSTR getPathForListView(HWND m_hListView, int iItem)
{
	LVITEM lv;
	lv.mask = LVIF_PARAM;
	lv.iItem = iItem;
	lv.iSubItem = 0;
	ListView_GetItem(m_hListView, &lv);
	return (LPCWSTR)lv.lParam;
}

void loadChildForListView(LPCWSTR path, Drive *drive)
{
	if (path == nullptr)
		return;
	
	if (!StrCmp(path, _T("MyComputer")))
		loadMyComputerToListView(g_listView, drive);
	else
		loadDirItemToLisview(g_hWnd, g_listView, path);
}

char* convertByteToStringSize(DWORD fileSizeLow)
{
	int nType = 0; //Bytes
	int nRight;

	if (fileSizeLow >= 1024)
	{

		nRight = fileSizeLow % 1024;

		while (nRight > 99)
			nRight /= 10;

		fileSizeLow /= 1024;
		++nType;
	}
	else
		nRight = 0;

	TCHAR *buffer = new TCHAR[11];
	TCHAR *temp = new TCHAR[15];
	_itow_s(fileSizeLow, buffer, 11, 10);
	int k = 0;
	int j = 0;
	bool check = false;
	if (_tcslen(buffer) > 3)
	{
		check = true;

		for (int i = _tcslen(buffer) - 1; i >= 0; i--)
		{
			if (k == 3)
			{
				temp[j++] = ',';
				k = 0;
			}

			temp[j++] = buffer[i];
			k++;
		}
	}
	if (!check)
	{
		if (nType > 0)
		{
			_itow_s(fileSizeLow, buffer, 11, 10);
			StrCat(buffer, _T("   KB"));
		}
		else
		{
			_itow_s(1, buffer, 11, 10);
			StrCat(buffer, _T("   KB"));
		}
	}
	else
	{
		temp[j] = '\0';
		StrCat(temp, _T("   KB"));
	}
	if (!check)
		return (char*)buffer;
	return _strrev((char*)temp);
}

void convertWStringToCharPtr(_In_ std::wstring input, _Out_ char * outputString)
{
	size_t outputSize = input.length() + 1; // +1 for null terminator
	outputString = new char[outputSize];
	size_t charsConverted = 0;
	const wchar_t * inputW = input.c_str();
	wcstombs_s(&charsConverted, outputString, outputSize, inputW, input.length());
}

bool findNameItem(vector<TCHAR*>g_nameFile, char *name)
{
	for (int i = 0; i < g_nameFile.size(); i++)
	{
		char * temp = new char[_tcslen((g_nameFile.at(i)))];
		for (int j = 0; j < _tcslen((g_nameFile.at(i))); j++)
		{
			temp[j] = g_nameFile.at(i)[j];
		}
		temp[_tcslen((g_nameFile.at(i)))] = '\0';
		if (strcmp(name, temp) == 0)
			return true;
	}
	return false;
}

void PathtoFile(LPCWSTR path, TCHAR* nf)
{
	int len = wcslen(path);
	len--;
	int len2 = 0;
	TCHAR temp[100];
	while (path[len] != L'\\')
	{
		nf[len2] = path[len];
		temp[len2++] = path[len--];
	}
	nf[len2] = L'\0';
	temp[len2] = L'\0';
	len = wcslen(temp);
	len--;
	len2 = 0;
	while (len != -1)
	{
		nf[len2++] = temp[len--];
	}
	nf[len2] = L'\0';
}