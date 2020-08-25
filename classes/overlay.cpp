#include <iostream>
#include "overlay.h"

#define OVERLAY_Y_POS 125

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 976

Overlay::Overlay()
    : m_Window(nullptr)
    , m_ScreenSurface(nullptr)
    , m_Image(nullptr)
    , m_StartupInfo()
    , m_ProcessInfo()
{
}

Overlay::~Overlay()
{
    Close();
    CloseHandle(m_ProcessInfo.hProcess);
}

/**
 * Initialize SDL2 for showing the overlay image.
 */
bool Overlay::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "ERROR: Could not initialize SDL2: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cout << "ERROR: Could not initialize SDL2_image: " << IMG_GetError() << std::endl;
        return false;
    }

    m_Window = SDL_CreateWindow(
        "Overlay for SID Factory II",
        SDL_WINDOWPOS_UNDEFINED, OVERLAY_Y_POS,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (m_Window == nullptr) {
        std::cout << "ERROR: Could not create window: " << SDL_GetError() << std::endl;
        return false;
    }
    m_ScreenSurface = SDL_GetWindowSurface(m_Window);

    if (m_ScreenSurface == nullptr) {
        std::cout << "ERROR: Could not get window: " << SDL_GetError() << std::endl;
        return false;
    }

    m_Image = IMG_Load("overlay.png");
    if (m_Image == nullptr) {
        std::cout << "ERROR: Could not load image: " << IMG_GetError() << std::endl;
        return false;
    }

    SDL_Surface* optimized_image = SDL_ConvertSurface(m_Image, m_ScreenSurface->format, 0);
    if (optimized_image == nullptr)
    {
        std::cout << "ERROR: Could not optimize image: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(m_Image);
        return false;
    }
    return true;
}

/**
 * Show the overlay image.
 */
HWND Overlay::Show()
{
    SDL_BlitSurface(m_Image, nullptr, m_ScreenSurface, nullptr);
    SDL_UpdateWindowSurface(m_Window);

    // Return the HWND so we can manipulate the window later
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(m_Window, &wmInfo);
    return wmInfo.info.win.window;
}

/**
 * Close the overlay image.
 */
void Overlay::Close()
{
    SDL_FreeSurface(m_Image);
    m_Image = nullptr;

    SDL_FreeSurface(m_ScreenSurface);
    m_ScreenSurface = nullptr;
    SDL_DestroyWindow(m_Window);
    m_Window = nullptr;
    SDL_Quit();
}

/**
 * Set the position of the the overlay image.
 * NOT CURRENTLY USED.
 */
void Overlay::SetPos(int inX, int inY)
{
    SDL_SetWindowPosition(m_Window, inX, inY);
}

/**
 * Run another program and return its process.
 */
HANDLE Overlay::Run(LPCTSTR inPath, char* inCommandLine)
{
    STARTUPINFO m_StartupInfo;
    PROCESS_INFORMATION m_ProcessInfo;

    ZeroMemory(&m_StartupInfo, sizeof(m_StartupInfo));
    m_StartupInfo.cb = sizeof(m_StartupInfo);
    ZeroMemory(&m_ProcessInfo, sizeof(m_ProcessInfo));

    CreateProcess(
        inPath,         // Path to executable
        inCommandLine,  // Command line parameters
        nullptr,        // Process handle not inheritable
        nullptr,        // Thread handle not inheritable
        false,          // Set handle inheritance to FALSE
        0,              // No creation flags
        nullptr,        // Use parent's environment block
        "..",           // Use parent's starting directory 
        &m_StartupInfo, // Pointer to STARTUPINFO structure
        &m_ProcessInfo  // Pointer to PROCESS_INFORMATION structure
    );

    CloseHandle(m_ProcessInfo.hThread);
    return m_ProcessInfo.hProcess;
}

/**
 * Wait a bit to see if the program is still running.
 */
bool Overlay::IsRunning(HANDLE inProcess, int inTimeout)
{
    return WaitForSingleObject(inProcess, inTimeout) != WAIT_OBJECT_0;
}

/**
 * Get the HWND for a windows process.
 */
HWND Overlay::GetWindow(HANDLE inProcess)
{
    DWORD pid = GetProcessId(inProcess);
    std::pair<HWND, DWORD> params = { 0, pid };

    // Enumerate the windows using a lambda to process each window
    BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
        {
            auto pParams = (std::pair<HWND, DWORD>*)(lParam);

            DWORD processId;
            if (GetWindowThreadProcessId(hwnd, &processId) && processId == pParams->second)
            {
                // Stop enumerating
                SetLastError(-1);
                pParams->first = hwnd;
                return false;
            }

            // Continue enumerating
            return true;
        }, (LPARAM)&params);

    if (!bResult && GetLastError() == -1 && params.first)
        return params.first;

    return 0;
}