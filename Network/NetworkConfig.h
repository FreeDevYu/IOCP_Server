#pragma once
#include <iostream>
#include <fstream>
#include <../nlohmann/json.hpp>

                                                                                                                                                                         
namespace Utility
{
	void CreateConfig(std::string fileName);
	nlohmann::json LoadSettingFiles(std::string fileName);
	
}