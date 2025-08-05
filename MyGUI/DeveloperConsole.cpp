#include "DeveloperConsole.h"

namespace MyGUI 
{
    DeveloperConsole::DeveloperConsole() 
    {
    }

    DeveloperConsole::~DeveloperConsole()
    {
    }

    void DeveloperConsole::Initialize(std::string consoleName)
    {
        _consoleName = consoleName;
        _currentInputText = "";

        _consoleMessages.push_back("Console Initialized");
		_consoleMessages.push_back("Type your commands below.");
    }


    void DeveloperConsole::Process()
    {
        auto consoleSize = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowSize(consoleSize, ImGuiCond_Once);
        ImGui::Begin(_consoleName.c_str(), nullptr, ImGuiWindowFlags_NoMove);

        ImGui::Text("Command:");
        ImGui::SameLine();
        ImGui::InputText("##CommandInput", &_currentInputText);
        ImGui::SameLine();  
        if (ImGui::Button("Submit"))
        {
            // 입력된 텍스트 처리
            CommandCallback(_currentInputText);
            _currentInputText = "";
        }

        if (ImGui::Button("Reset"))
        {
            Reset();
        }

        // 메시지 출력 영역
        ImGui::BeginChild("ConsoleOutput", ImVec2(0, 0), true);  // 스크롤 가능한 영역
        for (const auto& message : _consoleMessages)
        {
            ImGui::Text(message.c_str());
        }

        // 자동 스크롤 아래로
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

    void DeveloperConsole::SetCommandCallback(std::function<void(const std::string&)> callback)
    {
        _requestCommand = callback;
	}

    void DeveloperConsole::CommandCallback(std::string command)
    {
        std::string result = "You entered: " + command;
		_consoleMessages.push_back(result);
        _requestCommand(command);
        // 콜백 부분
    }


    void DeveloperConsole::AddMessage(const std::string& type, const std::string& message)
    {
		//추후 타입별로 필터링 가능하도록 수정 필요
        _consoleMessages.push_back(message);
	}
}