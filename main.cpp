#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include "resource.h"
#include <fstream>
#include <algorithm>
#include <tchar.h>
#include <windows.h>

#define VICTORY 10001

typedef int (__cdecl *func)();

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DestoryChildCallback(HWND hwnd, LPARAM lParam){
  if (hwnd != NULL) {
    DestroyWindow(hwnd);
  }
  return TRUE;
}

struct Position {
     int x;
     int y;
};

int tileNumbers[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
int solution[6][6] = {{0,0,0,0,0},{0,1,2,3,4,0},{0,5,6,7,8,0},{0,9,10,11,12,0},{0,13,14,15,16,0},{0,0,0,0,0,0}}; //solved puzzle
int grid[6][6] = {0};
HWND tiles[17] = {NULL}; //tile index represents tile number

void setupGame(HWND hwnd);
void generateGrid(bool load);
void moveTile(int x);
Position getTilePositionInGrid(int number);
void saveGame();
void loadGame();

HWND hwnd; //main window
HWND saveButton;
HWND loadButton;
HWND shuffleButton;

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("SlidingPuzzle");

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
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
           szClassName,       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           370,                 /* The programs width */
           400,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
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

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        case WM_CREATE:
            generateGrid(false);
            setupGame(hwnd);
            break;
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                    //Save game button pressed
                    if((HWND)lParam == saveButton){
                        saveGame();
                    }
                    //Load game button pressed
                    if((HWND)lParam == loadButton){
                        /*Destroying all child windows and recreating them.
                          Probably not the best solution but it works for this small game.*/
                        EnumChildWindows(hwnd, DestoryChildCallback, NULL);
                        loadGame();
                        generateGrid(true);
                        setupGame(hwnd);
                    }
                    //Shuffle game button pressed
                    if((HWND)lParam == shuffleButton){
                        EnumChildWindows(hwnd, DestoryChildCallback, NULL);
                        generateGrid(false);
                        setupGame(hwnd);
                    }
                    //Tile pressed
                    for(int x = 1; x <= 16; x++){
                        if((HWND)lParam == tiles[x]){
                            moveTile(x);
                        }
                    }
            }
            break;
        case VICTORY: {
            HINSTANCE lib = LoadLibrary("DLLProject.dll");
            func f = (func)GetProcAddress(lib, "VictoryMessage");
            f();
            FreeLibrary(lib);
            }
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

//buttons
void setupGame(HWND hwnd){

    saveButton = CreateWindow(
                _T("button"),  // Predefined class; Unicode assumed
                _T("Save"),      // Button text
                WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
                50,         // x position
                10,         // y position
                64,        // Button width
                30,        // Button height
                hwnd,     // Parent window
                NULL,       // No menu.
                NULL,
                NULL);      // Pointer not needed.

    loadButton = CreateWindow(
                _T("button"),  // Predefined class; Unicode assumed
                _T("Load"),      // Button text
                WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
                146,         // x position
                10,         // y position
                64,        // Button width
                30,        // Button height
                hwnd,     // Parent window
                NULL,       // No menu.
                NULL,
                NULL);      // Pointer not needed.

    shuffleButton = CreateWindow(
                _T("button"),  // Predefined class; Unicode assumed
                _T("Shuffle"),      // Button text
                WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
                242,         // x position
                10,         // y position
                64,        // Button width
                30,        // Button height
                hwnd,     // Parent window
                NULL,       // No menu.
                NULL,
                NULL);      // Pointer not needed.

    //create tile windows(buttons)
    int n = 0;
    for(int x = 0; x < 4; x++){
        for(int y = 0; y < 4; y++){
            HWND button = CreateWindow(
                _T("button"),  // Predefined class; Unicode assumed
                NULL,      // Button text
                WS_VISIBLE | WS_CHILD | BS_TEXT | BS_BITMAP,  // Styles
                50+64*y,         // x position
                50+64*x,         // y position
                64,        // Button width
                64,        // Button height
                hwnd,     // Parent window
                NULL,       // No menu.
                NULL,
                NULL);      // Pointer not needed.

                //put button handles into array where array index represents button number
                //tileNumbers[n] gives random index for tiles array because tileNumbers[n] is shuffled
                tiles[tileNumbers[n]] = button;

                //put image of index number on button --> tile[5] has image of 5
                HBITMAP hTile = (HBITMAP)LoadBitmap((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),  MAKEINTRESOURCE(1000 + tileNumbers[n]));
                n++;
                SendMessage(button, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hTile);
        }
    }
}

/*zeros around array border, tiles numbers shuffled in the middle
0 0 0 0 0 0
0 1 2 3 4 0
0 5 6 7 8 0
0 9101112 0
013141516 0
0 0 0 0 0 0
*/
void generateGrid(bool load){
    //shuffle starting array for random start
    if(!load){
        std::random_shuffle(&tileNumbers[0], &tileNumbers[16]);
    }
    //make 2d grid from the starting array
    int n = 0;
    for (int x=0; x<4; x++){
        for (int y=0; y<4; y++){
            grid[x+1][y+1] = tileNumbers[n];
            n++;
        }
    }
}

void moveTile(int number){

    Position pos = getTilePositionInGrid(number);

    int x = pos.x;
    int y = pos.y;
    int dx = 0;
    int dy = 0;

    //find legal move
    if (grid[x+1][y]==16) {dx=1; dy=0;}
    else if (grid[x][y+1]==16) {dx=0; dy=1;}
    else if (grid[x][y-1]==16) {dx=0; dy=-1;}
    else if (grid[x-1][y]==16) {dx=-1;dy=0;}
    else { return;}

    //swap tiles in the grid
    int n = grid[x][y];
    grid[x][y] = 16;
    grid[x+dx][y+dy] = n;

    //swap tiles on the screen

    //tile positions before move
    RECT rectTilePressed, rectTile16;

    HWND hTilePressed = tiles[number];
    GetClientRect(hTilePressed, &rectTilePressed);
    MapWindowPoints(hTilePressed, hwnd, (LPPOINT)&rectTilePressed, 2);

    HWND hTile16 = tiles[16];
    GetClientRect(hTile16, &rectTile16);
    MapWindowPoints(hTile16, hwnd, (LPPOINT)&rectTile16, 2);

    //Tile16 moves to TilePressed location
    SetWindowPos(tiles[16], 0, rectTilePressed.left, rectTilePressed.top, 64, 64, SWP_NOSIZE);

    //TilePressed moves to Tile16 location
    SetWindowPos(tiles[number], 0, rectTile16.left, rectTile16.top, 64, 64, SWP_NOSIZE);

    //check if solved
    if(memcmp(grid, solution, sizeof(grid)) == 0){
        SendMessage(hwnd, VICTORY, NULL, NULL);
    };
}

Position getTilePositionInGrid(int number){
    Position position;
    for (int i=0; i<4; i++){
        for (int j=0; j<4; j++){
            if(grid[i+1][j+1] == number){
                position.x = i+1;
                position.y = j+1;
                return position;
            }
        }
    }
    return position;
}

void saveGame(){
    //put grid into starting array
    int n = 0;
    for (int x=0; x<4; x++){
        for (int y=0; y<4; y++){
            tileNumbers[n] = grid[x+1][y+1];
            n++;
        }
    }

    HANDLE file = CreateFile("save.txt",
                             GENERIC_WRITE,
                             0, NULL,
                             CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
    DWORD b_written = 0;
    WriteFile(file, tileNumbers, sizeof(tileNumbers), &b_written, NULL);

    CloseHandle(file);
}

void loadGame(){
    HANDLE file = CreateFile("save.txt",
                      GENERIC_READ,
                      0,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL);

    DWORD b_read = 0;
    if (file != INVALID_HANDLE_VALUE){
        ReadFile(file, tileNumbers, sizeof(tileNumbers), &b_read, NULL);
    }

    CloseHandle(file);
}
