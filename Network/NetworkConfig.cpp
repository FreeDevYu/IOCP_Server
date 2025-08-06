#include "NetworkConfig.h"


namespace Utility
{
	void CreateConfig(std::string fileName)
	{
		// JSON 객체 생성
		// nlohmann::json 라이브러리를 사용하여 JSON 파일을 생성합니다.
		// JSON 파일의 구조를 정의합니다.

		nlohmann::json config;

		config["IP"] = "127.0.0.1";
		config["SERVER_PORT"] = 9090;
		config["CLIENT_CAPACITY"] = 10;
		config["OVERLAPPED_COUNT_MAX"] = 50;

		config["HEARTBEAT_INTERVAL"] = 60;
		config["HEARTBEAT_TIMEOUT"] = 30;
		config["HEARTBEAT_TIMEOUT_CHANCE"] = 3;

		config["REGISTER_TIMEOUT"] = 60;

		std::ofstream file(fileName);
		file << config.dump(4);  // 4는 들여쓰기 수준

		std::string log = "[Utility] [JsonCreator] File CreateSuccess !";
		std::cout << log << std::endl;
	}

	nlohmann::json LoadSettingFiles(std::string fileName)
	{
		std::ifstream file(fileName);  // JSON 파일 열기
		if (!file)
		{
			std::string log = "[Utility] [JsonCreator] File Find Fail !!";
			std::cout << log << std::endl;

			return NULL;
		}

		nlohmann::json config;
		file >> config;  // 파일에서 JSON 객체 읽기

		return config;
	}
}