#pragma once
#include <string>
#include <functional>
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

        void Initialize(std::string consoleName);
        void Process();
        void Reset();

        void AddMessage(const std::string& type, const std::string& message);

    private:
        std::string _consoleName;

    private:
		std::vector<std::string> _consoleMessages; // �ܼ� �޽��� ����� ����

    public:
        void SetCommandCallback(std::function<void(const std::string&)> callback);
        void CommandCallback(std::string command);

    private:
        std::string _currentInputText;
		std::function<void(const std::string&)> _requestCommand; // ��ɾ� �ݹ� �Լ�
    };
}