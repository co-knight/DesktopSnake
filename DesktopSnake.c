/*********************************************************************************
 *
 * Desktop Snake Game (C/WinAPI)
 *
 *
 * Features:
 * - Uses existing desktop icons instead of deleting them.
 * - Automatically detects desktop icon grid size.
 * - Disables and re-enables "Snap to Grid" for smooth animation.
 * - Elegant input handling to prevent 180-degree turns.
 * - Clear victory condition when all icons are "eaten".
 *
 * Controls:
 * - W / Arrow Up:     Move Up
 * - S / Arrow Down:   Move Down
 * - A / Arrow Left:   Move Left
 * - D / Arrow Right:  Move Right
 * - ESC:              Exit Game
 *
 * How to Compile (using the provided Makefile):
 * make
 *
 *********************************************************************************/

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>

#ifndef SPI_GETICONSPACING
#define SPI_GETICONSPACING 0x0047
#endif

#ifndef SPI_GETICONVERTICALSPACING
#define SPI_GETICONVERTICALSPACING 0x0048
#endif

// --- Global Handles & Settings ---
HWND g_hDesktopListView = NULL; // Desktop's ListView control.
int g_screenWidth;              // Width
int g_screenHeight;             // Height
int g_gridStepX;                // Detected horizontal grid size.
int g_gridStepY;                // Detected vertical grid size.
int g_totalIcons;               // Total number of icons on the desktop.

// --- Game State Flags ---
BOOL g_isFoodEaten = TRUE;
POINT g_foodPos;

// --- Structures ---

// Controllable direction
typedef struct TDirection {
    char key;                  // Character('w', 'a', 's', 'd')
    int vkCode;                // Virtual-Key Code
    struct TDirection* against; // Opposite direction
} Direction;

// Icon on the desktop
typedef struct {
    int order; // The original index of the icon in the ListView
    POINT pos; // Current position on the screen
} Icon;

// Snake
typedef struct {
    BOOL isLiving;
    BOOL hasWon;
    char currentDirKey;
    int speed;
    int length;
    Icon body[1024]; // Max snake length is 1024 (or total icons)
} Snake;


// --- Functions ---
void InitializeGame(Snake* snake);
void CleanupGame();
void ToggleSnapToGrid(BOOL enable);
void ClearDesktop();
void SpawnFood(Snake* snake);
void HandleInput(Snake* snake, Direction directions[]);
void UpdateGame(Snake* snake);


// --- Main ---
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    srand(time(NULL));

    // Find the handle to the desktop's icon list view
    g_hDesktopListView = FindWindowExW(FindWindowExW(GetShellWindow(), NULL, L"SHELLDLL_DefView", NULL), NULL, L"SysListView32", NULL);
    if (g_hDesktopListView == NULL) {
        MessageBoxW(NULL, L"无法找到桌面图标句柄！程序即将退出。", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Controllable directions and their opposites
    Direction dir_w = {'w', 'W', NULL};
    Direction dir_a = {'a', 'A', NULL};
    Direction dir_s = {'s', 'S', NULL};
    Direction dir_d = {'d', 'D', NULL};
    Direction dir_up = {'w', VK_UP, &dir_s};
    Direction dir_left = {'a', VK_LEFT, &dir_d};
    Direction dir_down = {'s', VK_DOWN, &dir_up};
    Direction dir_right = {'d', VK_RIGHT, &dir_left};
    dir_w.against = &dir_s;
    dir_a.against = &dir_d;
    dir_s.against = &dir_w;
    dir_d.against = &dir_a;
    Direction directions[] = {dir_up, dir_down, dir_left, dir_right, dir_w, dir_a, dir_s, dir_d};

    Snake snake = {0};
    InitializeGame(&snake);

    // --- Main Game Loop ---
    while (snake.isLiving && !snake.hasWon && !GetAsyncKeyState(VK_ESCAPE)) {
        HandleInput(&snake, directions);
        UpdateGame(&snake);
        if (g_isFoodEaten) {
            SpawnFood(&snake);
        }
        Sleep(snake.speed);
    }

    CleanupGame();

    // Final message
    if (snake.hasWon) {
        MessageBoxW(NULL, L"恭喜！你吃掉了所有图标，获得了胜利！\n桌面即将恢复。", L"胜利！", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(NULL, L"游戏结束。\n桌面即将恢复。", L"游戏结束", MB_OK | MB_ICONINFORMATION);
    }

    return 0;
}

// --- Initialize ---
void InitializeGame(Snake* snake) {
    // Get screen dimensions
    g_screenWidth = GetSystemMetrics(SM_CXSCREEN);
    g_screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Dynamically get icon grid size
    SystemParametersInfoW(SPI_GETICONSPACING, 0, &g_gridStepX, 0);
    SystemParametersInfoW(SPI_GETICONVERTICALSPACING, 0, &g_gridStepY, 0);
    if (g_gridStepX <= 0) g_gridStepX = 75;
    if (g_gridStepY <= 0) g_gridStepY = 75;

    g_totalIcons = ListView_GetItemCount(g_hDesktopListView);
    if (g_totalIcons == 0) {
       MessageBoxW(NULL, L"桌面上没有图标，无法开始游戏。", L"错误", MB_OK | MB_ICONERROR);
       exit(1);
    }

    ToggleSnapToGrid(FALSE); // Disable snap-to-grid for smooth movement
    ClearDesktop();          // Move all icons off-screen

    // Initialize snake
    snake->isLiving = TRUE;
    snake->hasWon = FALSE;
    snake->speed = 200;
    snake->currentDirKey = 'd'; // Start moving right
    snake->length = 1;

    // Create the snake's head
    snake->body[0].order = 0;
    snake->body[0].pos.x = (rand() % (g_screenWidth / g_gridStepX)) * g_gridStepX;
    snake->body[0].pos.y = (rand() % (g_screenHeight / g_gridStepY)) * g_gridStepY;
    ListView_SetItemPosition(g_hDesktopListView, snake->body[0].order, snake->body[0].pos.x, snake->body[0].pos.y);
}

// --- Cleanup ---
void CleanupGame() {
    ToggleSnapToGrid(TRUE); // Re-enable snap-to-grid
    // Refresh the desktop to make icons reappear correctly
    SendMessageW(g_hDesktopListView, LVM_ARRANGE, LVA_DEFAULT, 0);
}

// --- Toggle Snap to Grid ---
void ToggleSnapToGrid(BOOL enable) {
    DWORD dwStyle = ListView_GetExtendedListViewStyle(g_hDesktopListView);
    if (enable) {
        if ((dwStyle & LVS_EX_SNAPTOGRID) == 0) {
            ListView_SetExtendedListViewStyle(g_hDesktopListView, dwStyle | LVS_EX_SNAPTOGRID);
        }
    } else {
        if ((dwStyle & LVS_EX_SNAPTOGRID) != 0) {
            ListView_SetExtendedListViewStyle(g_hDesktopListView, dwStyle & ~LVS_EX_SNAPTOGRID);
        }
    }
}

// --- Clear Desktop ---
void ClearDesktop() {
    for (int i = 0; i < g_totalIcons; ++i) {
        // Move icons to a far-away place
        ListView_SetItemPosition(g_hDesktopListView, i, -20000, -20000);
    }
}

// --- Spawn Food ---
void SpawnFood(Snake* snake) {
    // Check for victory first
    if (snake->length >= g_totalIcons) {
        snake->hasWon = TRUE;
        return;
    }

    BOOL onSnake;
    do {
        onSnake = FALSE;
        g_foodPos.x = (rand() % (g_screenWidth / g_gridStepX)) * g_gridStepX;
        g_foodPos.y = (rand() % (g_screenHeight / g_gridStepY)) * g_gridStepY;

        // Check if food spawned on the snake
        for (int i = 0; i < snake->length; ++i) {
            if (snake->body[i].pos.x == g_foodPos.x && snake->body[i].pos.y == g_foodPos.y) {
                onSnake = TRUE;
                break;
            }
        }
    } while (onSnake);

    // The food is the next available icon after the snake's body
    int foodIconOrder = snake->length;
    ListView_SetItemPosition(g_hDesktopListView, foodIconOrder, g_foodPos.x, g_foodPos.y);
    g_isFoodEaten = FALSE;
}

// --- Handle Input ---
void HandleInput(Snake* snake, Direction directions[]) {
    for (int i = 0; i < 8; ++i) {
        // Check the direction key is not the opposite direction
        if (GetAsyncKeyState(directions[i].vkCode) && snake->currentDirKey != directions[i].against->key) {
            snake->currentDirKey = directions[i].key;
            break; // Process only one key press per frame
        }
    }
}

// --- Update Game ---
void UpdateGame(Snake* snake) {
    // Store the last position of the tail before moving
    POINT lastPos = snake->body[snake->length - 1].pos;

    // Move body segments
    for (int i = snake->length - 1; i > 0; --i) {
        snake->body[i].pos = snake->body[i - 1].pos;
    }

    // Move the head based on the current direction
    switch (snake->currentDirKey) {
        case 'w': snake->body[0].pos.y -= g_gridStepY; break;
        case 's': snake->body[0].pos.y += g_gridStepY; break;
        case 'a': snake->body[0].pos.x -= g_gridStepX; break;
        case 'd': snake->body[0].pos.x += g_gridStepX; break;
    }

    // --- Collision Detection ---
    // Wall collision
    if (snake->body[0].pos.x < 0 || snake->body[0].pos.x >= g_screenWidth ||
        snake->body[0].pos.y < 0 || snake->body[0].pos.y >= g_screenHeight) {
        snake->isLiving = FALSE;
        return;
    }

    // Self collision
    for (int i = 1; i < snake->length; ++i) {
        if (snake->body[0].pos.x == snake->body[i].pos.x && snake->body[0].pos.y == snake->body[i].pos.y) {
            snake->isLiving = FALSE;
            return;
        }
    }

    // --- Food Detection ---
    if (snake->body[0].pos.x == g_foodPos.x && snake->body[0].pos.y == g_foodPos.y) {
        // Grow the snake
        if (snake->length < 1024 && snake->length < g_totalIcons) {
            snake->length++;
            snake->body[snake->length - 1].order = snake->length - 1;
            snake->body[snake->length - 1].pos = lastPos;
        }
        g_isFoodEaten = TRUE;
    }

    // --- Render ---
    // Update the positions of all snake icons
    for (int i = 0; i < snake->length; ++i) {
        ListView_SetItemPosition(g_hDesktopListView, snake->body[i].order, snake->body[i].pos.x, snake->body[i].pos.y);
    }
}
