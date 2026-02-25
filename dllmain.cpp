// dllmain.cpp : Defines the entry point for the DLL application.

#include <windows.h>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <vector>
#include <iostream>


struct Settings 
{
    uint32_t LoggingLevel = 2; 
    int FrameRateType = 1;
    uint32_t FrameRateLimit = 60; 
    bool ShowFPS = true; 
    bool NoCursor = false;
    bool AdjustCursorSpeed = true; 
    bool DisableLowLatency = false; 
    bool DebugBreakpointMode = false; 
    bool DisableWindowResizing = false; 
    bool DisableDirectFlip = false;
    bool ConfineCursorIn3DGames = false;
    bool CropTitleBarOfUWP = false;
    bool DebugDisableEffectCache = false;
    bool SimulateExclusiveFullscreen = false;
    bool CustomCropping = false;
    uint32_t CaptureMode = 2; 
    float CursorZoomFactor = 1.0f;
    uint32_t CursorInterpolationMode = 1;
    uint32_t AdapterIdx = 0;
    uint32_t MultiMonitorUsage = 0;
    uint32_t CropLeft = 0;
    uint32_t CropTop = 0;
    uint32_t CropRight = 0;
    uint32_t CropBottom = 0;
    uint32_t EffectType = 0;
    uint32_t HotKeyHold = 164;
    uint32_t HotKeyPress = 122;

} g_Settings;


enum FlagMasks : uint32_t {
    NoCursor = 0x1,
    AdjustCursorSpeed = 0x2,
    ShowFPS = 0x4,
    SimulateExclusiveFullscreen = 0x8,
    DisableLowLatency = 0x10,
    BreakpointMode = 0x20,
    DisableWindowResizing = 0x40,
    DisableDirectFlip = 0x80,
    ConfineCursorIn3DGames = 0x100,
    CropTitleBarOfUWP = 0x200,
    DisableEffectCache = 0x400
};


HMODULE m_hRuntime = NULL;

typedef bool(__stdcall* FnInitialize)(
    UINT logLevel,
    const char* logFileName,
    int logArchiveAboveSize,
    int logMaxArchiveFiles
    );
typedef const char* (__stdcall* FnRun)(
    HWND hwndSrc,
    const char* effectsJson,
    UINT flags,
    UINT captureMode,
    int frameRate,	
    float cursorZoomFactor,	
    UINT cursorInterpolationMode,
    UINT adapterIdx,
    UINT multiMonitorUsage,	
    UINT cropLeft,
    UINT cropTop,
    UINT cropRight,
    UINT cropBottom
    );

FnInitialize pInitialize = nullptr;
FnRun pRun = nullptr;

void LoadMagpieSettings()
{
    const char* iniFile = "./Magpie_Hook.ini";
    if (GetFileAttributesA(iniFile) != INVALID_FILE_ATTRIBUTES)
    {
        char tmpdata[32];
        g_Settings.HotKeyHold = GetPrivateProfileIntA("Settings", "HotKeyHold", 164, iniFile);
        g_Settings.HotKeyPress = GetPrivateProfileIntA("Settings", "HotKeyPress", 122, iniFile);
        g_Settings.EffectType = GetPrivateProfileIntA("Settings", "EffectType", 0, iniFile);
        g_Settings.CaptureMode = GetPrivateProfileIntA("Settings", "CaptureMode", 2, iniFile);
        g_Settings.NoCursor = GetPrivateProfileIntA("Settings", "NoCursor", 0, iniFile);
        g_Settings.CursorZoomFactor = (GetPrivateProfileStringA("Settings", "CursorZoomFactor", "1.0", tmpdata, 32, iniFile), (float)atof(tmpdata));
        g_Settings.AdjustCursorSpeed = GetPrivateProfileIntA("Settings", "AdjustCursorSpeed", 1, iniFile);
        g_Settings.ConfineCursorIn3DGames = GetPrivateProfileIntA("Settings", "ConfineCursorIn3DGames", 0, iniFile);
        g_Settings.CursorInterpolationMode = GetPrivateProfileIntA("Settings", "CursorInterpolationMode", 0, iniFile);

        g_Settings.ShowFPS = GetPrivateProfileIntA("Settings", "ShowFPS", 1, iniFile);
        g_Settings.FrameRateType = GetPrivateProfileIntA("Settings", "FrameRateType", 1, iniFile);
        g_Settings.FrameRateLimit = GetPrivateProfileIntA("Settings", "FrameRateLimit", 60, iniFile);
        g_Settings.DisableLowLatency = GetPrivateProfileIntA("Settings", "DisableLowLatency", 0, iniFile);
        g_Settings.DisableDirectFlip = GetPrivateProfileIntA("Settings", "DisableDirectFlip", 0, iniFile);
        g_Settings.DisableWindowResizing = GetPrivateProfileIntA("Settings", "DisableWindowResizing", 0, iniFile);
        g_Settings.SimulateExclusiveFullscreen = GetPrivateProfileIntA("Settings", "SimulateExclusiveFullscreen", 0, iniFile);

        g_Settings.MultiMonitorUsage = GetPrivateProfileIntA("Settings", "MultiMonitorUsage", 0, iniFile);
        g_Settings.AdapterIdx = GetPrivateProfileIntA("Settings", "AdapterIdx", 0, iniFile);

        g_Settings.CropTitleBarOfUWP = GetPrivateProfileIntA("Settings", "CropTitleBarOfUWP", 0, iniFile);
        g_Settings.CustomCropping = GetPrivateProfileIntA("Settings", "CustomCropping", 0, iniFile);
        g_Settings.CropLeft = GetPrivateProfileIntA("Settings", "CropLeft", 0, iniFile);
        g_Settings.CropTop = GetPrivateProfileIntA("Settings", "CropTop", 0, iniFile);
        g_Settings.CropRight = GetPrivateProfileIntA("Settings", "CropRight", 0, iniFile);
        g_Settings.CropBottom = GetPrivateProfileIntA("Settings", "CropBottom", 0, iniFile);


    }
}

bool LoadMagpieRT()
{
    m_hRuntime = LoadLibraryA("MagpieRT.dll");

    if (!m_hRuntime) 
    {
        MessageBoxA(0, "Failed to Load Magpie Runtime DLL", "Fatal Error", 0);
        return false;
    }

    pInitialize = (FnInitialize)GetProcAddress(m_hRuntime, "Initialize");
    pRun = (FnRun)GetProcAddress(m_hRuntime, "Run");

    if (!pInitialize || !pRun) 
    {
       //Maybe 32 bit dll
       pInitialize = (FnInitialize)GetProcAddress(m_hRuntime, "_Initialize@16");
       pRun = (FnRun)GetProcAddress(m_hRuntime, "_Run@52");
       if (!pInitialize || !pRun)
       {
           char msg[256];
           sprintf(msg, "Failed to Load Magpie Runtime Functions\n pRun : %x pInitialize : %x", (DWORD)pRun, (DWORD)pInitialize);
           MessageBoxA(0, msg, "Fatal Error", 0);
           return false;
       }

    }

    return true;

}

std::string GetAppDir() 
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos + 1);
}

std::string SelectEffectType(UINT type)
{

    //I'm too lazy to download a json lib and parse json for a simple string
    switch (type)
    {
        // Case 0: Lanczos + AdaptiveSharpen
        case 0:return R"([{"effect":"Lanczos","scale":[-1,-1]},{"effect":"AdaptiveSharpen","curveHeight":0.3}])";

        // Case 1: FSR (FidelityFX Super Resolution)
        case 1: return R"([{"effect":"FSR_EASU","scale":[-1,-1]},{"effect":"FSR_RCAS","sharpness":0.87}])";

        // Case 2: FSRCNNX + Bicubic
        case 2:return R"([{"effect":"FSRCNNX"},{"effect":"Bicubic","scale":[-1,-1],"paramB":0.0,"paramC":0.5}])";

        // Case 3: ACNet + Bicubic
        case 3:return R"([{"effect":"ACNet"},{"effect":"Bicubic","scale":[-1,-1],"paramB":0.0,"paramC":0.5}])";

        // Case 4: Anime4K + Bicubic
        case 4:return R"([{"effect":"Anime4K_Upscale_Denoise_L"},{"effect":"Bicubic","scale":[-1,-1],"paramB":0.0,"paramC":0.5}])";

        // Case 5: RAVU Zoom
        case 5:return R"([{"effect":"RAVU_Zoom_R3","scale":[-1,-1]}])";

        // Case 6: CRT Geometry
        case 6:return R"([{"effect":"CRT_Geom","scale":[-1,-1],"curvature":0,"cornerSize":0.001,"CRTGamma":1.5,"monitorGamma":2.2}])";

        // Case 7: Nearest Neighbor (2x)
        case 7:return R"([{"effect":"Nearest","scale":[2,2]}])";

        // Case 8: Nearest Neighbor (3x)
        case 8:return R"([{"effect":"Nearest","scale":[3,3]}])";

        //if all else fails, use Lanczos
        default:return R"([{"effect":"Lanczos","scale":[-1,-1]},{"effect":"AdaptiveSharpen","curveHeight":0.3}])";

    }
}

UINT SelectFrameRate(UINT type)
{
    int frameRate = 0;
    switch (type) 
    {
    case 1: frameRate = -1; break; // Uncapped
    case 2: frameRate = (int)g_Settings.FrameRateLimit; break;
    default: break; // VSync
    }
    return frameRate;
}

void MainThread()
{
    //AllocConsole();
    //wchar_t ConsoleFont[256];
    //swprintf(ConsoleFont, L"SimSun");  //Chinese
    //CONSOLE_FONT_INFOEX cfi;
    //cfi.cbSize = sizeof(cfi);
    //cfi.nFont = 0;
    //cfi.dwFontSize.X = 0;
    //cfi.dwFontSize.Y = 16;
    //cfi.FontFamily = FF_DONTCARE;
    //cfi.FontWeight = FW_NORMAL;
    //std::wcscpy(cfi.FaceName, ConsoleFont);
    //SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    //SetConsoleOutputCP(936);
    //freopen("CONOUT$", "w", stdout);


    if (!LoadMagpieRT())
    {
        exit(0);
    }

   
    std::string logFileName = GetAppDir() + "Runtime.log";
    const int logArchiveAboveSize = 100000;
    const int logMaxArchiveFiles = 1;

    if (pInitialize(0, logFileName.c_str(), logArchiveAboveSize, logMaxArchiveFiles))
    {
        std::string effects = SelectEffectType(g_Settings.EffectType);
        UINT framerate = SelectFrameRate(g_Settings.FrameRateType);
        uint32_t flags =
        (g_Settings.ShowFPS ? ShowFPS : 0) |
        (g_Settings.NoCursor ? NoCursor : 0) |
        (g_Settings.AdjustCursorSpeed ? AdjustCursorSpeed : 0) |
        (g_Settings.DisableLowLatency ? DisableLowLatency : 0) |
        (g_Settings.DebugBreakpointMode ? BreakpointMode : 0) |
        (g_Settings.DisableWindowResizing ? DisableWindowResizing : 0) |
        (g_Settings.DisableDirectFlip ? DisableDirectFlip : 0) |
        (g_Settings.ConfineCursorIn3DGames ? ConfineCursorIn3DGames : 0) |
        (g_Settings.CropTitleBarOfUWP ? CropTitleBarOfUWP : 0) |
        (g_Settings.DebugDisableEffectCache ? DisableEffectCache : 0) |
        (g_Settings.SimulateExclusiveFullscreen ? SimulateExclusiveFullscreen : 0);
        bool isUpscale = false;
        bool customCropping = g_Settings.CustomCropping;


        while (true)
        {
            // Check keys
            bool keyHold = (GetAsyncKeyState(g_Settings.HotKeyHold) & 0x8000) != 0;
            bool keyPress = (GetAsyncKeyState(g_Settings.HotKeyPress) & 0x8000) != 0;

            // Toggle on press
            if (keyHold && keyPress)
            {
                HWND hwnd = GetForegroundWindow();

                if (!isUpscale)
                {
                    pRun(hwnd, effects.c_str(), flags,
                    g_Settings.CaptureMode,
                    framerate,
                    g_Settings.CursorZoomFactor,
                    g_Settings.CursorInterpolationMode,
                    g_Settings.AdapterIdx,
                    g_Settings.MultiMonitorUsage,
                    customCropping ? g_Settings.CropLeft : 0,
                    customCropping ? g_Settings.CropTop : 0,
                    customCropping ? g_Settings.CropRight : 0,
                    customCropping ? g_Settings.CropBottom : 0
                    );
                    isUpscale = true;
                 }
                 else
                 {
                    //BUG : Pressing the hotkey again does not revert back to the window 
                    printf("now destory magpie window\n");
                    const int MAGPIE_WM_DESTROYHOST = RegisterWindowMessageW(L"MAGPIE_WM_DESTORYHOST");
                    if (PostMessage(HWND_BROADCAST, MAGPIE_WM_DESTROYHOST, 0, 0))
                    {
                        printf("destory magpie window sent\n");
                        isUpscale = false;
                    }
                     
                 }
            }
        }
    }
    else
    {
        MessageBoxA(0, "Can't Initialize MagpieRT", "Fatal Error", 0);
    }
}



BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
