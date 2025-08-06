#include "NetworkConfig.h"


namespace Utility
{
	void CreateConfig(std::string fileName)
	{
		// JSON ��ü ����
		// nlohmann::json ���̺귯���� ����Ͽ� JSON ������ �����մϴ�.
		// JSON ������ ������ �����մϴ�.

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
		file << config.dump(4);  // 4�� �鿩���� ����

		std::string log = "[Utility] [JsonCreator] File CreateSuccess !";
		std::cout << log << std::endl;
	}

	nlohmann::json LoadSettingFiles(std::string fileName)
	{
		std::ifstream file(fileName);  // JSON ���� ����
		if (!file)
		{
			std::string log = "[Utility] [JsonCreator] File Find Fail !!";
			std::cout << log << std::endl;

			return NULL;
		}

		nlohmann::json config;
		file >> config;  // ���Ͽ��� JSON ��ü �б�

		return config;
	}
}