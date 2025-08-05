#pragma once
#include "ServerManager.h"
#include "../Network/NetworkConfig.h"
#include "../MyGUI/DeveloperConsole.h"

//#define CONFGINTEST

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ������ �޽��� ó�� �Լ�
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
     if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
         return true;

    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

static MyGUI::DeveloperConsole console;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) 
{
#ifdef CONFGINTEST
    Utility::CreateConfig("servermanager_config.json");
#endif

    auto config = Utility::LoadSettingFiles("servermanager_config.json");

    Manager::ServerManager serverManager;
    serverManager.Initialize(
        new Network::ClientManager(),
        new Network::OverlappedManager(),
        config["SERVER_PORT"].get<int>(),
        config["IP"].get<std::string>(),
        config["OVERLAPPED_COUNT_MAX"].get<int>(),
        config["CLIENT_CAPACITY"].get<int>()
    );

    serverManager.StartIOCP();
    serverManager.StartWorkThreads();
    serverManager.StartListenThread();
    serverManager.SetUpdateFrame(60); // FPS ����
    serverManager.StartUpdateThread();


    ////////////////////////////////////////////////////////////////////////////////
    
    // 1. ������ Ŭ���� ���
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("MyImGuiWindow");
    RegisterClass(&wc);

    // 2. ������ ���� �� HWND ���
    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        _T("ImGui + Win32 + OpenGL"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 720,
        NULL, NULL, hInstance, NULL
    );

    HDC hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0, 0,
        0, 0, 0, 0, 0,
        24, 8,
        0,
        PFD_MAIN_PLANE,
        0, 0, 0, 0
    };

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    HGLRC hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);

    // 3. ������ ǥ��
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 4. ImGui �ʱ�ȭ
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);            
    ImGui_ImplOpenGL3_Init("#version 130"); 

	console.Initialize("Developer Console");

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        console.Process();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SwapBuffers(hdc); 
    }

    // ���� ó��
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // OpenGL ���ؽ�Ʈ ����
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);

    return 0;
}


namespace Debug
{
    enum DebugType
    {
        DEBUG_LOG,
        DEBUG_NETWORK,
        DEBUG_WARNING,
        DEBUG_ERROR,

        MAX
    };;

    inline const char* const* EnumNamesDebugType()
    {
        static const char* const names[DebugType::MAX + 1] = {
          "DEBUG_LOG",
          "DEBUG_NETWORK",
          "DEBUG_WARNING",
          "DEBUG_ERROR",
          "MAX"
        };

        return names;
    }


    static void LogMessage(DebugType type, const std::string& message)
    {
        std::string typeName = EnumNamesDebugType()[type];

        console.AddMessage(typeName, message);
    }
}
