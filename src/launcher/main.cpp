
//HMODULE hDLL = LoadLibrary("ImGuiDLL.dll");
// cl main.cpp /I imgui /link d3d11.lib user32.lib
// launcher.exe

/*
typedef void (*InitFunc)();
typedef void (*RenderFunc)();
typedef void (*ShutdownFunc)();
typedef bool (*VisibleFunc)();

InitFunc IntMainClient = (InitFunc)GetProcAddress(hDLL, "InitIntMainClient");
RenderFunc RenderUI = (RenderFunc)GetProcAddress(hDLL, "RenderUI");
ShutdownFunc ShutdownIntMainClient = (ShutdownFunc)GetProcAddress(hDLL, "ShutdownIntMainClient");
VisibleFunc IsDemoWindowVisible = (VisibleFunc)GetProcAddress(hDLL, "IsDemoWindowVisible");

// Initialization
IntMainClient();

// Render loop
while (running) {
    // Your window/render setup...
    
    RenderUI();
    
    // Your window/render cleanup...
}

// Cleanup
ShutdownIntMainClient();
FreeLibrary(hDLL);
*/

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>

// DLL function typedefs
typedef void (*InitFunc)();
typedef void (*RenderFunc)();
typedef void (*ShutdownFunc)();
typedef bool (*VisibleFunc)();

// Structure for launcher configuration
typedef struct {
    int window_width;
    int window_height;
    bool fullscreen;
    char log_file[MAX_PATH];
    char dll_path[MAX_PATH];
} LauncherConfig;

// Global variables
static HMODULE g_hDLL = NULL;
static InitFunc IntMainClient = NULL;
static RenderFunc RenderUI = NULL;
static ShutdownFunc ShutdownIntMainClient = NULL;
static VisibleFunc IsDemoWindowVisible = NULL;
static LauncherConfig g_config = { 1280, 800, false, "launcher.log", "IntMainClientDLL.dll" };
static FILE* g_logFile = NULL;
static HINSTANCE g_hInstance = NULL;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool LoadConfig(const char* filename);
void LogMessage(const char* format, ...);
void Cleanup();

// WinMain entry point
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    g_hInstance = hInstance;

    // Initialize logging
    g_logFile = fopen(g_config.log_file, "a");
    if (!g_logFile) {
        MessageBox(NULL, _T("Failed to open log file"), _T("Error"), MB_OK | MB_ICONERROR);
    }
    LogMessage("Launcher starting at %s", ctime(&(time_t){time(NULL)}));

    // Load configuration
    if (!LoadConfig("launcher.ini")) {
        LogMessage("Warning: Using default configuration");
    }

    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("IntMainLauncher");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassEx(&wc)) {
        LogMessage("Error: Failed to register window class");
        Cleanup();
        return 1;
    }

    // Adjust window size for client area
    RECT rc = { 0, 0, g_config.window_width, g_config.window_height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    int adjustedWidth = rc.right - rc.left;
    int adjustedHeight = rc.bottom - rc.top;

    // Create window
    DWORD style = WS_OVERLAPPEDWINDOW;
    if (g_config.fullscreen) {
        style = WS_POPUP;
    }

    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        _T("IntMain Client Launcher"),
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        adjustedWidth,
        adjustedHeight,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        LogMessage("Error: Failed to create window");
        Cleanup();
        return 1;
    }

    // Load DLL
    g_hDLL = LoadLibraryA(g_config.dll_path);
    if (!g_hDLL) {
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg), "Failed to load %s", g_config.dll_path);
        LogMessage("Error: %s", errorMsg);
        MessageBox(NULL, _T(errorMsg), _T("Error"), MB_OK | MB_ICONERROR);
        Cleanup();
        return 1;
    }

    // Get function pointers
    IntMainClient = (InitFunc)GetProcAddress(g_hDLL, "InitIntMainClient");
    RenderUI = (RenderFunc)GetProcAddress(g_hDLL, "RenderUI");
    ShutdownIntMainClient = (ShutdownFunc)GetProcAddress(g_hDLL, "ShutdownIntMainClient");
    IsDemoWindowVisible = (VisibleFunc)GetProcAddress(g_hDLL, "IsDemoWindowVisible");

    if (!IntMainClient || !RenderUI || !ShutdownIntMainClient || !IsDemoWindowVisible) {
        LogMessage("Error: Failed to get DLL function pointers");
        MessageBox(NULL, _T("Failed to get DLL function pointers"), _T("Error"), MB_OK | MB_ICONERROR);
        Cleanup();
        return 1;
    }

    // Initialize DLL
    LogMessage("Initializing DLL: %s", g_config.dll_path);
    IntMainClient();

    // Show window
    ShowWindow(hwnd, g_config.fullscreen ? SW_SHOWMAXIMIZED : nCmdShow);
    UpdateWindow(hwnd);

    // Main message loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    bool running = true;

    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                running = false;
            }
        }

        if (!running) break;

        // Call DLL render function
        RenderUI();

        // Prevent CPU hogging
        Sleep(16); // ~60 FPS
    }

    // Cleanup
    LogMessage("Shutting down");
    ShutdownIntMainClient();
    Cleanup();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, hInstance);

    return 0;
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
            return 0;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        g_config.window_width = LOWORD(lParam);
        g_config.window_height = HIWORD(lParam);
        LogMessage("Window resized to %dx%d", g_config.window_width, g_config.window_height);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable F10 menu activation
            return 0;
        break;
    case WM_CLOSE:
        LogMessage("Window close requested");
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Configuration loading
bool LoadConfig(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "width=%d", &g_config.window_width) == 1) continue;
        if (sscanf(line, "height=%d", &g_config.window_height) == 1) continue;
        if (sscanf(line, "fullscreen=%d", &g_config.fullscreen) == 1) continue;
        if (sscanf(line, "log_file=%s", g_config.log_file) == 1) continue;
        if (sscanf(line, "dll_path=%s", g_config.dll_path) == 1) continue;
    }

    fclose(file);
    return true;
}

// Logging function
void LogMessage(const char* format, ...) {
    if (!g_logFile) return;

    va_list args;
    va_start(args, format);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    // Remove newline from ctime output
    char* timeStr = ctime(&(time_t){time(NULL}));
    if (timeStr[strlen(timeStr) - 1] == '\n') timeStr[strlen(timeStr) - 1] = '\0';
    
    fprintf(g_logFile, "[%s] %s\n", timeStr, buffer);
    fflush(g_logFile);
    
    va_end(args);
}

// Cleanup function
void Cleanup() {
    if (g_hDLL) {
        FreeLibrary(g_hDLL);
        g_hDLL = NULL;
    }
    if (g_logFile) {
        fclose(g_logFile);
        g_logFile = NULL;
    }
}
