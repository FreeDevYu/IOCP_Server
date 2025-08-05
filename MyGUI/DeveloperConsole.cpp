#include "DeveloperConsole.h"

namespace MyGUI 
{
    DeveloperConsole::DeveloperConsole() 
    {
    }

    DeveloperConsole::~DeveloperConsole()
    {
    }

    void DeveloperConsole::Initialize(std::string consoleName, float width, float height)
    {
        _consoleName = consoleName;
        _currentInputText = "";

        _consoleWidth = width;
        _consoleHeight = height;
        // �ʱ� �޽��� �߰� (����)
        _consoleMessages.push_back("Console Initialized");
		_consoleMessages.push_back("Type your commands below.");
    }


    void DeveloperConsole::Process()
    {
        ImGui::SetNextWindowSize(ImVec2(_consoleWidth, _consoleHeight), ImGuiCond_Once);
        ImGui::Begin(_consoleName.c_str(), nullptr, ImGuiWindowFlags_NoMove);
       // ImGui::Text("Console Initialized");

        ImGui::InputText("Command", &_currentInputText);
        ImGui::SameLine();  // �ٷ� �����ʿ� ���� ���� ��ġ
        if (ImGui::Button("Submit"))
        {
            // �Էµ� �ؽ�Ʈ ó��
            InputCallback(_currentInputText);
            _currentInputText = "";
        }

        if (ImGui::Button("Reset"))
        {
            Reset();
        }

        // �޽��� ��� ����
        ImGui::BeginChild("ConsoleOutput", ImVec2(0, 0), true);  // ��ũ�� ������ ����
        for (const auto& message : _consoleMessages)
        {
            ImGui::Text("You entered: %s", message.c_str());
        }

        // �ڵ� ��ũ�� �Ʒ���
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();

        ImGui::End();

    }

    void DeveloperConsole::Reset()
    {
        _consoleMessages.clear();
        _currentInputText = "";
		_consoleMessages.push_back("Console Reset");
    }

    void DeveloperConsole::InputCallback(std::string input)
    {
		_consoleMessages.push_back(input);
        // �ݹ� �κ�
    }
}