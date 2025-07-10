#define _WIN32_WINNT 0x0601  // Targeting Windows 7 or higher
#include <sdkddkver.h>       // Must be included to properly apply _WIN32_WINNT

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_DEPRECATION_OF_GetVersionExA

#include <windows.h>
#include "Resource.h"
#include "Assistant.h"
#include <sapi.h>
#include <sphelper.h>
#include <atlbase.h>  // For CComPtr
#pragma comment(lib, "sapi.lib")

#include <string>
#include <sstream>


LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

HWND hInput, hOutput, hButton;
std::stringstream chatHistory;

// SAPI global interfaces
CComPtr<ISpRecognizer> cpRecognizer;
CComPtr<ISpRecoContext> cpRecoContext;
CComPtr<ISpRecoGrammar> cpGrammar;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    CoInitialize(NULL);  // COM Init

    const char CLASS_NAME[] = "CLIAssistWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "Nova - AI Assistant",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 650, 500,
        nullptr, nullptr, hInstance, nullptr
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();  // Clean up COM
    return 0;
}

void UpdateChatWindow() {
    SetWindowText(hOutput, chatHistory.str().c_str());
    SendMessage(hOutput, EM_SETSEL, -1, -1);
    SendMessage(hOutput, EM_SCROLLCARET, 0, 0);
}

void Speak(const std::string& text) {
    ISpVoice* pVoice = nullptr;
    if (SUCCEEDED(::CoInitialize(NULL))) {
        HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
        if (SUCCEEDED(hr)) {
            std::wstring wtext(text.begin(), text.end());
            pVoice->Speak(wtext.c_str(), SPF_IS_NOT_XML, NULL);
            pVoice->Release();
        }
        CoUninitialize();
    }
}

std::string ListenFromMic() {
    WCHAR* recognizedText = nullptr;

    if (!cpRecognizer) {
        cpRecognizer.CoCreateInstance(CLSID_SpSharedRecognizer);
        cpRecognizer->CreateRecoContext(&cpRecoContext);
        cpRecoContext->SetNotifyWin32Event();
        cpRecoContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
        cpRecoContext->CreateGrammar(1, &cpGrammar);
        cpGrammar->LoadDictation(NULL, SPLO_STATIC);
        cpGrammar->SetDictationState(SPRS_ACTIVE);
    }

    HANDLE hEvent = cpRecoContext->GetNotifyEventHandle();
    if (WaitForSingleObject(hEvent, 3000) == WAIT_OBJECT_0) {
        CSpEvent event;
        while (event.GetFrom(cpRecoContext) == S_OK) {
            if (event.eEventId == SPEI_RECOGNITION) {
                ISpRecoResult* pResult = event.RecoResult();
                if (pResult) {
                    pResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &recognizedText, NULL);
                    break;
                }
            }
        }
    }

    if (recognizedText) {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, recognizedText, -1, NULL, 0, NULL, NULL);
        std::string result(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, recognizedText, -1, &result[0], size_needed, NULL, NULL);
        CoTaskMemFree(recognizedText);
        return result;
    }

    return "";
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
    {
        CreateWindow("STATIC", "Type your command or use mic:", WS_CHILD | WS_VISIBLE,
            20, 20, 250, 20, hwnd, NULL, NULL, NULL);

        hInput = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
            20, 45, 460, 25, hwnd, (HMENU)IDC_INPUT, NULL, NULL);

        hButton = CreateWindow("BUTTON", "Send", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            490, 45, 100, 25, hwnd, (HMENU)IDC_SEND_BUTTON, NULL, NULL);

        hOutput = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
            20, 85, 570, 340, hwnd, (HMENU)IDC_OUTPUT, NULL, NULL);

        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hOutput, WM_SETFONT, (WPARAM)hFont, TRUE);

        SetTimer(hwnd, 1, 6000, NULL);  // Voice input every 6s
        return 0;
    }

    case WM_TIMER:
    {
        std::string voiceText = ListenFromMic();
        if (!voiceText.empty()) {
            std::string reply = ProcessCommand(voiceText);
            chatHistory << "You (Mic): " << voiceText << "\r\n";
            chatHistory << "Nova: " << reply << "\r\n\r\n";
            UpdateChatWindow();
            Speak(reply);
        }
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_SEND_BUTTON) {
            char buffer[512];
            GetWindowText(hInput, buffer, sizeof(buffer));
            std::string userInput(buffer);

            if (userInput.empty()) return 0;

            std::string reply = ProcessCommand(userInput);
            chatHistory << "You: " << userInput << "\r\n";
            chatHistory << "Nova: " << reply << "\r\n\r\n";

            UpdateChatWindow();
            Speak(reply);
            SetWindowText(hInput, "");
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
