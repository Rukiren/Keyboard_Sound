#include <iostream>
#include <string>
#include <thread>  
#include <windows.h>
#include <mciapi.h>
#include "resource.h"

#pragma comment(lib, "winmm.lib")

class SoundPlayer {
public:
    static void PlaySound() {
        // 從資源載入音效
        HMODULE hModule = GetModuleHandle(NULL);
        HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDR_SOUND1), RT_RCDATA);
        
        if (hResource) {
            HGLOBAL hMemory = LoadResource(hModule, hResource);
            LPVOID lpData = LockResource(hMemory);
            DWORD dwSize = SizeofResource(hModule, hResource);

            // 建立臨時檔案
            char tempPath[MAX_PATH];
            GetTempPathA(MAX_PATH, tempPath);
            
            char tempFile[MAX_PATH];
            GetTempFileNameA(tempPath, "SND", 0, tempFile);

            // 寫入臨時檔案
            FILE* fp = fopen(tempFile, "wb");
            if (fp) {
                fwrite(lpData, 1, dwSize, fp);
                fclose(fp);

                // 播放音效
                std::thread soundThread([tempFile]() {
                    std::string uniqueAlias = "sound_" + std::to_string(GetTickCount());
                    std::string openCmd = "open \"" + std::string(tempFile) + "\" type mpegvideo alias " + uniqueAlias;
                    std::string playCmd = "play " + uniqueAlias;
                    
                    mciSendString(openCmd.c_str(), NULL, 0, NULL);
                    mciSendString(playCmd.c_str(), NULL, 0, NULL);
                    
                    Sleep(2000);
                    
                    std::string closeCmd = "close " + uniqueAlias;
                    mciSendString(closeCmd.c_str(), NULL, 0, NULL);
                    
                    // 刪除臨時檔案
                    DeleteFileA(tempFile);
                });
                soundThread.detach();
            }
        }
    }
};

HHOOK g_hKeyboardHook = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        SoundPlayer::PlaySound();
    }
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "全域音效監控已啟動...\n";
    std::cout << "按任意鍵可以同時播放音效，支援音效疊加\n";

    g_hKeyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,     
        LowLevelKeyboardProc, 
        GetModuleHandle(NULL), 
        0
    );

    if (g_hKeyboardHook == NULL) {
        std::cerr << "無法設置鍵盤攔截器！\n";
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(g_hKeyboardHook);
    return 0;
}