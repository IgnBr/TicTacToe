#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include "menu.h"
#include "dllheader.h"
#include <iostream>

using namespace std;

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");
HINSTANCE hInstance;
HANDLE hFile;


int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    hInstance = hThisInstance;
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("Code::Blocks Template Windows App"),       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           LoadMenu(NULL, MAKEINTRESOURCE(IDR_MYMENU)),                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

// Globalus kintamieji
const int CELL_SIZE = 100;
HBRUSH hbr1, hbr2;
HICON hIcon1, hIcon2;
int playerTurn = 1;
int gameBoard[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int winner = 0;
int wins[3];

// Gauname zaidimo lentos dydi
BOOL GetGameBoardRect(HWND hwnd, RECT * pRect)
{
    RECT rc;
    if(GetClientRect(hwnd, &rc)){
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;

        pRect -> left = ( w - CELL_SIZE * 3 ) / 2;
        pRect -> top = ( h - CELL_SIZE * 3 ) / 2;
        pRect -> right = pRect -> left + CELL_SIZE * 3;
        pRect -> bottom = pRect -> top + CELL_SIZE * 3;

        return TRUE;
    }

    SetRectEmpty(pRect);
    return FALSE;
}

// Funkcija piesianti linijas, naudojama lentai braizyti
void DrawLine (HDC hdc, int x1, int y1, int x2, int y2)
{
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
}

// Funkcija gaunanti lengelio skaiciu pagal koordinates
int GetCellNumberFromPoint(HWND hwnd, int x, int y)
{
    POINT pt = {x, y};
    RECT rc;

    if(GetGameBoardRect(hwnd, &rc)){
        if(PtInRect(&rc, pt))
        {
            // Pasidarom, kad koordinates zaidimo lentos viduj butu nuo 0 iki 3 * CELL_SIZE
            x = pt.x - rc.left;
            y = pt.y - rc.top;

            int col = x / CELL_SIZE;
            int row = y / CELL_SIZE;
            // Gauname viena is devyniu langeliu
            return col + row * 3;
        }
    }

    return -1;
}

// Funkcija gauti langelio koordinatems
BOOL GetCellRect(HWND hwnd, int index, RECT * pRect)
{
    RECT rcBoard;
    SetRectEmpty(pRect);

    if(index < 0 || index > 8)
    {
        return FALSE;
    }

    if(GetGameBoardRect(hwnd, &rcBoard))
    {
        int y = index / 3; // Eile
        int x = index % 3; // Stulpelis

        pRect -> left = rcBoard.left + x * CELL_SIZE + 1;
        pRect -> top = rcBoard.top + y * CELL_SIZE + 1;
        pRect -> right = pRect -> left + CELL_SIZE - 1;
        pRect -> bottom = pRect -> top + CELL_SIZE - 1;

        return TRUE;
    }
    return FALSE;
}

// Funkcija nuskaicius zaidimo busena is failo, sudeti i globalu kintamaji
void AssignGameBoard(char buffer[])
{
    if(strlen(buffer)==9)
    {
        for (int i = 0; i < 9; ++i)
        {
            gameBoard[i]=(int)buffer[i] - 48;
        }
    }
}

// Funkcija irasanti zaidimo busena i faila
void WriteGameState()
{
    char buffer[10];
    dllspec::dllclass::ConvertIntArrayToString(gameBoard, 9, buffer);
    hFile = CreateFile("gameState.txt",                // name of the write
           GENERIC_WRITE,          // open for writing
           FILE_SHARE_WRITE,                      // do not share
           NULL,                   // default security
           OPEN_EXISTING,             // create new file only
           FILE_ATTRIBUTE_NORMAL,  // normal file
           NULL);                  // no attr. template

    WriteFile(
        hFile,           // open file handle
        buffer,      // start of data to write
        (DWORD)strlen(buffer),  // number of bytes to write
        NULL, // number of bytes that were written
        NULL);

    CloseHandle(hFile);
}

// Funkcija skaitanti zaidimo busena is failo
void ReadGameState()
{
    OVERLAPPED ol = {0};
    char buffer[10] = {0};
    hFile = CreateFile("gameState.txt",                // name of the write
           GENERIC_READ,          // open for writing
           FILE_SHARE_READ,
           NULL,                   // default security
           OPEN_EXISTING,             // create new file only
           FILE_ATTRIBUTE_NORMAL,  // normal file
           NULL);                  // no attr. template

    ReadFileEx(hFile, buffer, 9, &ol, NULL);
    AssignGameBoard(buffer);
    CloseHandle(hFile);
}

// Funkcija nupiesianti lange, kurio zaidejo yra ejimas
void ShowTurn(HWND hwnd, HDC hdc)
{
    RECT rc;

    if(GetClientRect(hwnd, &rc))
    {
        rc.top = rc.bottom - 20;
        FillRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
        SetBkMode(hdc, TRANSPARENT);

        if(winner != 1 && winner != 2 && winner != 3)
        {
            DrawText(hdc, playerTurn == 1 ? "TURN: Player 1" : "TURN: Player 2", 14, &rc, DT_CENTER);
        }

        if(winner == 3){
            DrawText(hdc, "DRAW!", 5, &rc, DT_CENTER);
        }
    }
}

// Funkcija piesianti kryziuku arba nuliuku ikonas langelio centre
void DrawIconCentered(HDC hdc, RECT *pRect, HICON hIcon)
{
    const int ICON_WIDTH = GetSystemMetrics(SM_CXICON);
    const int ICON_HEIGHT = GetSystemMetrics(SM_CYICON);

    if(NULL != pRect)
    {
        int left = pRect->left + ((pRect -> right - pRect->left) - ICON_WIDTH)/2;
        int top = pRect->top + ((pRect -> bottom - pRect->top) - ICON_HEIGHT)/2;
        DrawIcon(hdc, left, top, hIcon);
    }
}


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch(wmId)
            {
                // Meniu juostoje paspaudus new game galime atstatyti zaidimo busena i pradine ir zaisti is naujo
                case ID_NEW_GAME:
                    {
                        int answer = MessageBox(hwnd, "Are you sure you want to start a new game?", "New Game", MB_YESNO | MB_ICONQUESTION);
                        // Jei paspaudziama yes, isvaloma buvusio zaidimo busena
                        if(IDYES == answer)
                        {
                            playerTurn = 1;
                            ZeroMemory(gameBoard, sizeof(gameBoard));
                            ZeroMemory(wins, sizeof(wins));
                            winner = 0;
                            WriteGameState();

                            InvalidateRect(hwnd, NULL, TRUE);
                            UpdateWindow(hwnd);
                        }
                    }
                break;
            }
        }
        break;
        case WM_DESTROY:
            {
                // Sunaikinami sukurti objektai isjungus zaidima
                DeleteObject(hbr1);
                DeleteObject(hbr2);
                DestroyIcon(hIcon1);
                DestroyIcon(hIcon2);
                PostQuitMessage (0);
            }
            break;
        case WM_CREATE:
            {
                // Sukuriami brushai ir ikonos
                hbr1 = CreateSolidBrush(RGB(255, 0, 0));
                hbr2 = CreateSolidBrush(RGB(0, 0, 255));
                hIcon1 = LoadIcon(hInstance, MAKEINTRESOURCE(ID_X_ICON));
                hIcon2 = LoadIcon(hInstance, MAKEINTRESOURCE(ID_ZERO_ICON));
            }
        case WM_GETMINMAXINFO:
            {
                // Sukuriami lango dydzio apribojimai
                MINMAXINFO * pMinMax = (MINMAXINFO*) lParam;
                pMinMax -> ptMinTrackSize.x = CELL_SIZE * 5;
                pMinMax -> ptMinTrackSize.y = CELL_SIZE * 5;
            }
            break;
        case WM_LBUTTONDOWN:
            {
                // Gaunamos paspaudimo koordinates
                int xPos = GET_X_LPARAM(lParam);
                int yPos = GET_Y_LPARAM(lParam);

                // Gaunamas paspaustas langelis
                int index = GetCellNumberFromPoint(hwnd, xPos, yPos);

                HDC hdc = GetDC(hwnd);
                if(NULL != hdc)
                {
                    if(index != -1)
                    {
                        RECT rcCell;
                        // Jei paspaustas laisvas langelis
                        if(gameBoard[index] == 0 && GetCellRect(hwnd, index, &rcCell))
                        {
                            // Pridedam zaidejo ejima, irasom zaidimo busena i faila, nupiesiam zaidejo ikona ir patikrinam ar yra laimetojas
                            gameBoard[index] = playerTurn;
                            WriteGameState();
                            DrawIconCentered(hdc, &rcCell, playerTurn == 1 ? hIcon1 : hIcon2);
                            winner = dllspec::dllclass::GetWinner(wins, gameBoard);

                            // Jei yra laimetojas nuspalvinam laimejusio zaidejo laimincias pozicijas ir isvedam message box, kad yra laimetojas
                            if(winner == 1 || winner == 2){
                                RECT rcWin;
                                for(int i = 0; i<3;++i)
                                {
                                    if(GetCellRect(hwnd, wins[i], &rcWin))
                                    {
                                        FillRect(hdc, &rcWin, winner == 1 ? hbr1 : hbr2);
                                        DrawIconCentered(hdc, &rcWin, winner == 1 ? hIcon1 : hIcon2);
                                    }
                                }

                                MessageBox(hwnd, winner == 1 ? "Player 1 is the winner!": "Player 2 is the winner!", "You Win", MB_OK | MB_ICONINFORMATION);
                                playerTurn = 0;
                            }
                            // Jei lygiosios isvedam message box i ekrana
                            else if (winner == 3)
                            {
                                MessageBox(hwnd, "Draw", "Its A Draw!", MB_OK | MB_ICONEXCLAMATION);
                                playerTurn = 0;
                            }
                            // Jei zaidimas nesibaige, pakeiciam zaidejo ejima
                            else if (winner == 0)
                            {
                                playerTurn = playerTurn == 1 ? 2 : 1;
                            }

                            // Atvaizduojam, kad kito zaidejo eile
                            ShowTurn(hwnd, hdc);

                        }
                    }

                    ReleaseDC(hwnd, hdc);
                }
            }
            break;
        case WM_PAINT:
            {
                // Perziurim irasyta zaidimo busena, jei zaidimas nesibaige - tesiam, jei yra laimetojas, isvalom lenta
                ReadGameState();
                int currentWinner;
                currentWinner = dllspec::dllclass::GetWinner(wins, gameBoard);
                if(currentWinner != 0){
                    playerTurn = 1;
                    ZeroMemory(gameBoard, sizeof(gameBoard));
                    ZeroMemory(wins, sizeof(wins));
                    winner = 0;
                    WriteGameState();
                }

                // Piesiam zaidimo lenta
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                RECT rc;
                if(GetGameBoardRect(hwnd, &rc)){
                    RECT rcClient;

                    // Piesiam zaideju pavadinimus lentos virsuje
                    if(GetClientRect(hwnd, &rcClient))
                    {
                        SetBkMode(hdc, TRANSPARENT);
                        TextOut(hdc, 16, 16, "Player 1 (X)", 12);
                        TextOut(hdc, rcClient.right - 90, 16, "Player 2 (O)", 12);

                        ShowTurn(hwnd, hdc);
                    }

                    // Nuspalvinam lenta baltai
                    FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));


                    for(int i = 0; i < 4; ++i)
                    {
                        // Piesiam vertikalias linijas
                        DrawLine(hdc, rc.left + CELL_SIZE * i, rc.top, rc.left + CELL_SIZE * i, rc.bottom);

                        // Piesiam horizontalias linijas
                        DrawLine(hdc, rc.left, rc.top + CELL_SIZE * i, rc.right , rc.top + CELL_SIZE * i);
                    }

                    // Piesiam uzimtu langeliu ikonas
                    RECT rcCell;

                    for(int i = 0; i < 9; ++i)
                    {
                        if(gameBoard[i] != 0 && GetCellRect(hwnd, i, &rcCell))
                        {
                            DrawIconCentered(hdc, &rcCell, gameBoard[i] == 1 ? hIcon1 : hIcon2);
                        }
                    }
                }
                EndPaint(hwnd, &ps);
            }
            break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
