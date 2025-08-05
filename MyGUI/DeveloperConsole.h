#pragma once
#include <string>
#include "imgui.h"
#include "imgui_stdlib.h"
#include <windows.h> 
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include <tchar.h>
#include <vector>



namespace MyGUI 
{
    class DeveloperConsole
    {
    public:
        DeveloperConsole();
        ~DeveloperConsole();

        void Initialize(std::string consoleName, float width, float height);
        void Process();
        void Reset();

        void InputCallback(std::string input);


    private:
        std::string _consoleName;
        float _consoleWidth;
        float _consoleHeight;

    private:
		std::vector<std::string> _consoleMessages; // 콘솔 메시지 저장용 벡터

    private:
        std::string _currentInputText;
    };
}