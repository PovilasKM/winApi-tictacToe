// winapi.cpp : Defines the entry point for the application.
//

#include <windowsx.h>
#include "framework.h"
#include "winapi.h"
#include <stdio.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

const int CELL_SIZE = 100;
HBRUSH hbr1, hbr2;
HICON hIcon1, hIcon2;
int playerTurn = 1;
int gameBoard[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int winner = 0;
int wins[3];
int fancyIcons = 0;
HWND exportButton, inportButton;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Export(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Inport(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINAPI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPI));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
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
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(GetStockObject(GRAY_BRUSH));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINAPI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//

BOOL GetGameBoardRect(HWND hWnd, RECT* pRect)
{
	RECT rc;
	if (GetClientRect(hWnd, &rc)) {
		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;

		pRect->left = (width - CELL_SIZE * 3) / 2;
		pRect->top = (height - CELL_SIZE * 3) / 2;

		pRect->right = pRect->left + CELL_SIZE * 3;
		pRect->bottom = pRect->top + CELL_SIZE * 3;

		return TRUE;
	}

	SetRectEmpty(pRect);
	return FALSE;
}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}

int GetCellNumberFromPoint(HWND hWnd, int x, int y)
{
	POINT pt = { x, y };
	RECT rc;

	if (GetGameBoardRect(hWnd, &rc))
	{
		if (PtInRect(&rc, pt)) // click is inside
		{
			//normalize (0 to 3*CELL_SIZE)
			x = pt.x - rc.left;
			y = pt.y - rc.top;

			int column = x / CELL_SIZE;
			int row = y / CELL_SIZE;

			return column + row * 3;
		}
		//click is outside
		return -1;
	}
	return -1;
}

BOOL GetCellRect(HWND hWnd, int index, RECT* pRect)
{
	RECT rcBoard;

	SetRectEmpty(pRect);
	if (index < 0 || index > 8)
	{
		return FALSE;
	}

	if (GetGameBoardRect(hWnd, &rcBoard))
	{
		int y = index / 3; // row
		int x = index % 3; // column

		pRect->left = rcBoard.left + x * CELL_SIZE +1;
		pRect->top = rcBoard.top + y * CELL_SIZE + 1;

		pRect->right = pRect->left + CELL_SIZE - 1;
		pRect->bottom = pRect->top + CELL_SIZE - 1;
	}
	return TRUE;
}

/*
	Returns 0 if no winner yet
			1 if player 1 wins
			2 if player 2 wins
			3 tie
*/
int GetWinner(int wins[3])
{
	int cells[] = {0,1,2,  3,4,5,  6,7,8,  0,3,6,  1,4,7,  2,5,8,  0,4,8,  2,4,6 };

	for (int i = 0; i < ARRAYSIZE(cells); i+=3) 
	{
		if (gameBoard[cells[i]] != 0 && gameBoard[cells[i]] == gameBoard[cells[i+1]] && gameBoard[cells[i]] == gameBoard[cells[i+2]])
		{
			wins[0] = cells[i];
			wins[1] = cells[i + 1];
			wins[2] = cells[i + 2];

			return gameBoard[cells[i]];
		}
	}

	for (int i = 0; i < 9; ++i)
	{
		if (gameBoard[i] == 0) 
		{
			return 0;
		}
	}

	return 3;
}

void showTurn(HWND hWnd, HDC hdc)
{
	RECT rcClient;

	if (GetClientRect(hWnd, &rcClient))
	{
		rcClient .top = rcClient.bottom - 48;
		FillRect(hdc, &rcClient, (HBRUSH)GetStockObject(GRAY_BRUSH));
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, playerTurn == 0 ? RGB(255, 255, 255) : playerTurn == 1 ? RGB(255, 0, 0) : RGB(0, 0, 255));
		DrawText(hdc, 
				playerTurn == 0 ? L"Game Over" : playerTurn == 1 ? L"Player 1 turn" : L"Player 2 turn", 
				playerTurn == 0 ? 9 : 13, 
				&rcClient, DT_CENTER);
	}
}

void DrawIconCentered(HDC hdc, RECT* pRect, HICON hIcon)
{
	const int ICON_WIDTH = GetSystemMetrics(SM_CXICON);
	const int ICON_HEIGHT = GetSystemMetrics(SM_CYICON);
	if (NULL != pRect)
	{
		int left = pRect->left + ((pRect->right - pRect->left) - ICON_WIDTH) / 2;
		int top = pRect->top + ((pRect->bottom - pRect->top) - ICON_HEIGHT) / 2;
		DrawIcon(hdc, left, top, hIcon);
	}
}

void changeIcons(HWND hWnd)
{
	DestroyIcon(hIcon1);
	DestroyIcon(hIcon2);
	if (fancyIcons == 0)
	{
		hIcon1 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PLAYER1));
		hIcon2 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PLAYER2));
	}
	else
	{
		hIcon1 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PLAYER1_2));
		hIcon2 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PLAYER2_2));
	}
	InvalidateRect(hWnd, NULL, TRUE); // adds WM_PAINT to msg queue
	UpdateWindow(hWnd); // forces WM_PAINT
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE :
		{
			hbr1 = CreateSolidBrush(RGB(255, 0, 0));
			hbr2 = CreateSolidBrush(RGB(0, 0, 255));
			changeIcons(hWnd);
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case ID_FILE_EXPORT:
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_FILE_NAME), hWnd, Export);
			}
			break;
			case ID_FILE_INPORT:
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_FILE_NAME), hWnd, Inport);
				showTurn(hWnd, GetDC(hWnd));
				InvalidateRect(hWnd, NULL, TRUE); // adds WM_PAINT to msg queue
				UpdateWindow(hWnd); // forces WM_PAINT
			}
			break;
			case ID_FILE_NEWGAME:
			{
				int result = MessageBox(hWnd, L"Do you want to start a new game?", L"New Game", MB_YESNO | MB_ICONQUESTION);
				if (result == IDYES)
				{
					playerTurn = 1;
					winner = 0;
					ZeroMemory(gameBoard, sizeof(gameBoard));
					ZeroMemory(wins, sizeof(wins));
					InvalidateRect(hWnd, NULL, TRUE); // adds WM_PAINT to msg queue
					UpdateWindow(hWnd); // forces WM_PAINT
				}
			}
			break;
			case ID_CHANGEICONS_REGULAR:
			{
				fancyIcons = 0;
				changeIcons(hWnd);
			}
			break;
			case ID_CHANGEICONS_FANCY:
			{
				fancyIcons = 1;
				changeIcons(hWnd);
			}
			break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_LBUTTONDOWN:
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);

			if (playerTurn == 0) //game over
			{
				break;
			}

			int index = GetCellNumberFromPoint(hWnd, xPos, yPos);

			HDC hdc = GetDC(hWnd);
			if (NULL != hdc)
			{
				//get cell
				if (index != -1)
				{
					RECT rcCell;
					if (gameBoard[index] == 0 && GetCellRect(hWnd, index, &rcCell))
					{
						//FillRect(hdc, &rcCell, playerTurn == 2 ? hbr2 : hbr1);
						gameBoard[index] = playerTurn;

						//draw icon
						DrawIconCentered(hdc, &rcCell, playerTurn == 1 ? hIcon1 : hIcon2);

						//check for end condition
						winner = GetWinner(wins);
						if (winner == 1 || winner == 2)
						{
							RECT rcWin;
							for (int i = 0; i < 3; ++i)
							{
								if (GetCellRect(hWnd, wins[i], &rcWin)) 
								{
									FillRect(hdc, &rcWin, winner == 1 ? hbr1 : hbr2);
									//DrawIconCentered(hdc, &rcWin, winner == 1 ? hIcon1 : hIcon2);
								}
							}
							MessageBox(hWnd, winner == 1 ? L"Player 1 won" : L"Player 2 won", L"I'm a caption", MB_OK | MB_ICONINFORMATION);
							playerTurn = 0;
						}
						else if (winner == 3)
						{
							MessageBox(hWnd, L"It's a tie", L"I'm a caption", MB_OK | MB_ICONEXCLAMATION);
							playerTurn = 0;
						}
						else
						{
							playerTurn = playerTurn == 2 ? 1 : 2;
						}
						showTurn(hWnd, hdc);
					}

				}

				ReleaseDC(hWnd, hdc);
			}
		}
		break;
	case WM_GETMINMAXINFO: 
		{
			MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;

			pMinMax->ptMinTrackSize.x = CELL_SIZE * 8;
			pMinMax->ptMinTrackSize.y = CELL_SIZE * 8;

		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

			RECT rc;
			if (GetGameBoardRect(hWnd, &rc)) 
			{
				RECT rcClient;

				if (GetClientRect(hWnd, &rcClient))
				{
					SetBkMode(hdc, TRANSPARENT);
					//draw player turn text
					SetTextColor(hdc, RGB(255, 0, 0));
					TextOut(hdc, 16, 16, L"Player 1", 8);
					DrawIcon(hdc, 24, 32, hIcon1);

					SetTextColor(hdc, RGB(0, 0, 255));
					TextOut(hdc, rcClient.right - 72, 16, L"Player 2", 8);
					DrawIcon(hdc, rcClient.right - 64, 32, hIcon2);

					showTurn(hWnd, hdc);
					
					DestroyWindow(exportButton);
					exportButton = CreateWindowW(
						L"BUTTON",  // Predefined class; Unicode assumed 
						L"Export results",      // Button text 
						WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
						rcClient.right / 6 - 140,         // x position 
						rcClient.bottom - 350,         // y position 
						200,        // Button width
						35,        // Button height
						hWnd,     // Parent window
						(HMENU)ID_FILE_EXPORT,      
						(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
						NULL);      // Pointer not needed. 
					DestroyWindow(inportButton);
					inportButton = CreateWindowW(
						L"BUTTON",  // Predefined class; Unicode assumed 
						L"Import results",      // Button text 
						WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
						rcClient.right / 6 - 140,         // x position 
						rcClient.bottom - 300,         // y position 
						200,        // Button width
						35,        // Button height
						hWnd,     // Parent window
						(HMENU)ID_FILE_INPORT,
						(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
						NULL);      // Pointer not needed. 
					
				}

				FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));  //no borders
				//Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom); //borders
			}
			
			for (int i = 0; i < 4; ++i) 
			{
				//vertical lines
				DrawLine(hdc, rc.left + CELL_SIZE * i, rc.top, rc.left + CELL_SIZE * i, rc.bottom);
				//horizontal lines
				DrawLine(hdc, rc.left, rc.top + CELL_SIZE * i, rc.right, rc.top + CELL_SIZE * i);
			}

			//repaint the board
			RECT rcCell;
			for (int i = 0; i < 9; ++i)
			{
				if (gameBoard[i] != 0 && GetCellRect(hWnd, i, &rcCell)) 
				{
					DrawIconCentered(hdc, &rcCell, gameBoard[i] == 1 ? hIcon1 : hIcon2);
				}
			}

			//paint winner move
			if (winner == 1 || winner == 2)
			{
				RECT rcWin;
				for (int i = 0; i < 3; ++i)
				{
					if (GetCellRect(hWnd, wins[i], &rcWin))
					{
						FillRect(hdc, &rcWin, winner == 1 ? hbr1 : hbr2);
						//DrawIconCentered(hdc, &rcWin, winner == 1 ? hIcon1 : hIcon2);
					}
				}
			}


            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		DeleteObject(hbr1);
		DeleteObject(hbr2);
		DestroyIcon(hIcon1);
		DestroyIcon(hIcon2);
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

void WriteToFile(TCHAR lpszFileName[]) 
{
	FILE* fp;

	char output[10];
	output[0] = playerTurn + '0';
	for (int i = 0; i < 9; i++)
	{
		output[i + 1] = gameBoard[i] + '0';
	}

	fp = _tfopen(lpszFileName, TEXT("wb"));
	fwrite(output, sizeof(char), sizeof(output), fp);
	fclose(fp);
}

BOOL fileExists(TCHAR* file)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE handle = FindFirstFile(file, &FindFileData);
	BOOL found = handle != INVALID_HANDLE_VALUE;
	if (found)
	{
		FindClose(handle);
	}
	return found;
}

BOOL ReadFromFile(TCHAR lpszFileName[], char* output) 
{
	if (!fileExists(lpszFileName))
	{
		return FALSE;
	}

	FILE* fp;

	fp = _tfopen(lpszFileName, TEXT("rb"));
	fgets(output, 11, fp);
	fclose(fp);

	return TRUE;
}

void ResetGame(char* data)
{
	winner = 0;
	ZeroMemory(wins, sizeof(wins));
	playerTurn = data[0] - '0';
	for (int i = 0; i < 9; i++)
	{
		gameBoard[i] = data[i + 1] - '0';
	}
}

INT_PTR CALLBACK Export(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR lpszFileName[32];
	WORD cchFileName;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		switch (wParam)
		{
			case IDOK:
			{
				cchFileName = (WORD)SendDlgItemMessage(hDlg,
					IDC_EDIT1,
					EM_LINELENGTH,
					(WPARAM)0,
					(LPARAM)0);

				// Put the number of characters into first word of buffer. 
				*((LPWORD)lpszFileName) = cchFileName;

				// Get the characters. 
				SendDlgItemMessage(hDlg,
					IDC_EDIT1,
					EM_GETLINE,
					(WPARAM)0,       // line 0 
					(LPARAM)lpszFileName);

				// Null-terminate the string. 
				lpszFileName[cchFileName] = 0;

				WriteToFile(lpszFileName);

				MessageBox(hDlg,
					L"Success",
					L"Success",
					MB_OK);

				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Inport(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR lpszFileName[32];
	WORD cchFileName;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		switch (wParam)
		{
		case IDOK:
		{
			cchFileName = (WORD)SendDlgItemMessage(hDlg,
				IDC_EDIT1,
				EM_LINELENGTH,
				(WPARAM)0,
				(LPARAM)0);

			// Put the number of characters into first word of buffer. 
			*((LPWORD)lpszFileName) = cchFileName;

			// Get the characters. 
			SendDlgItemMessage(hDlg,
				IDC_EDIT1,
				EM_GETLINE,
				(WPARAM)0,       // line 0 
				(LPARAM)lpszFileName);

			// Null-terminate the string. 
			lpszFileName[cchFileName] = 0;

			char data[11]; 
			if (ReadFromFile(lpszFileName, data))
			{
				MessageBox(hDlg,
					L"Success",
					L"Success",
					MB_OK);
				ResetGame(data);
			}
			else
			{
				MessageBox(hDlg,
					L"Error reading from file",
					L"Error reading from file",
					MB_OK);
			}

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

